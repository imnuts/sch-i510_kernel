/**
 *
 * @file       fs/rfs/file.c
 * @brief      file and file inode functions
 *
 *---------------------------------------------------------------------------*
 *                                                                           *
 *          COPYRIGHT 2003-2007 SAMSUNG ELECTRONICS CO., LTD.                *
 *                          ALL RIGHTS RESERVED                              *
 *                                                                           *
 *   Permission is hereby granted to licensees of Samsung Electronics        *
 *   Co., Ltd. products to use or abstract this computer program only in     *
 *   accordance with the terms of the NAND FLASH MEMORY SOFTWARE LICENSE     *
 *   AGREEMENT for the sole purpose of implementing a product based on       *
 *   Samsung Electronics Co., Ltd. products. No other rights to reproduce,   *
 *   use, or disseminate this computer program, whether in part or in        *
 *   whole, are granted.                                                     *
 *                                                                           *
 *   Samsung Electronics Co., Ltd. makes no representation or warranties     *
 *   with respect to the performance of this computer program, and           *
 *   specifically disclaims any responsibility for any damages,              *
 *   special or consequential, connected with the use of this program.       *
 *                                                                           *
 *---------------------------------------------------------------------------*
 *
 */

#include <linux/spinlock.h>
#include <linux/smp_lock.h>
#include <linux/fs.h>
#include <linux/rfs_fs.h>

#include "rfs.h"
#include "log.h"

#include "xattr.h"

#ifdef CONFIG_GCOV_PROFILE
#define        loff_t          off_t
#endif

/**
 *  check the hint info of inode whether it is available 
 * @param inode                inode
 * @param num_clusters number of clusters to skip
 * @param start_clu    start cluster number to read after skipping hint
 * @return             return 0 on success, EINVAL on failure
 *
 *  If hint info has availability, rest cluster chain can be read after skipping specified clusters
 */
static inline int rfs_lookup_hint(struct inode *inode, unsigned int *num_clusters, unsigned int *start_clu)
{
       if (RFS_I(inode)->start_clu == CLU_TAIL)
               return -EFAULT;

       if ((*num_clusters > 0) && (RFS_I(inode)->hint_last_offset > 0)
                       && (RFS_I(inode)->hint_last_clu >= VALID_CLU)
                       && (*num_clusters >= RFS_I(inode)->hint_last_offset)) {
               *start_clu = RFS_I(inode)->hint_last_clu;
               *num_clusters -= RFS_I(inode)->hint_last_offset;
       } else {
               *start_clu = RFS_I(inode)->start_clu;
       }

       return 0;
}

/**
 *  invalidate the hint info of inode
 * @param inode        inode
 */ 
void rfs_invalidate_hint(struct inode *inode)
{
       RFS_I(inode)->hint_last_clu = 0;
       RFS_I(inode)->hint_last_offset = 0;
}

/**
 *  update the hint info of inode 
 * @param inode                inode
 * @param cluster      previous last cluster number accessed
 * @param offset       cluster offset into a fat chain
 */
static inline void rfs_update_hint(struct inode *inode, unsigned int cluster, unsigned int offset)
{
       if (cluster < VALID_CLU || !offset) {
               rfs_invalidate_hint(inode);
       } else {
               RFS_I(inode)->hint_last_clu = cluster;
               RFS_I(inode)->hint_last_offset = offset;
       }
}

/**
 *  fill the page with zero
 * @param inode        inode    
 * @param page page pointer
 * @param zerofrom     start offset within page        
 * @param zeroto       last offset within page  
 * @param get_block    get_block funciton       
 * @return     return 0 on success, errno on failure
 *
 * This function is invoked by extend_with_zerofill
 */
static int rfs_page_zerofill(struct inode * inode, struct page * page, unsigned zerofrom, unsigned zeroto, get_block_t *get_block) 
{
       struct buffer_head *bh, *head;
       unsigned long block;
       unsigned block_start, block_end, blocksize, bbits, blk_aligned_zeroto; 
       int err = 0, partial = 0;
       char *kaddr;

       bbits = inode->i_blkbits;
       blocksize = 1 << bbits;

#ifdef RFS_FOR_2_6
       if (!page_has_buffers(page))
               create_empty_buffers(page, blocksize, 0);
       head = page_buffers(page);
#else
       if (!page->buffers) 
               create_empty_buffers(page, inode->i_dev, blocksize);
       head = page->buffers;
#endif

       block = page->index << (PAGE_CACHE_SHIFT - bbits); /* start block # */

       /* In the first phase, we allocate buffers and map them to fill zero */
       for(bh = head, block_start = 0; bh != head || !block_start;
               block++, block_start = block_end + 1, bh = bh->b_this_page) {
               if (!bh) { /* I/O error */
                       err = -EIO;
                       RFS_BUG("can't get buffer head\n");
                       goto out;
               }
               block_end = block_start + blocksize - 1;
               if (block_end < zerofrom) 
                       continue;
               else if (block_start > zeroto) 
                       break;
               clear_bit(BH_New, &bh->b_state);

               /* map new buffer head */       
               err = get_block(inode, block, bh, 1);
               if (err) {
                       DEBUG(DL1, "no block\n");       
                       goto out;
               }
               if (!buffer_uptodate(bh) && (block_start < zerofrom)) {
                       ll_rw_block(READ, 1, &bh);
                       wait_on_buffer(bh);
                       if (!buffer_uptodate(bh)) {
                               err = -EIO;
                               goto out;
                       }
               }
       }

       /* 
        * In the second phase, we memset the page with zero,
        * in the block aligned manner.
        * If memset is not block-aligned, hole may return garbage data.
        */
       blk_aligned_zeroto = zeroto | (blocksize - 1);
       kaddr = kmap_atomic(page, KM_USER0);
       memset(kaddr + zerofrom, 0, blk_aligned_zeroto - zerofrom + 1);

       /* In the third phase, we make the buffers uptodate and dirty */
       for(bh = head, block_start = 0; bh != head || !block_start;
               block_start = block_end + 1, bh = bh->b_this_page) {
               block_end = block_start + blocksize - 1;
               if (block_end < zerofrom) {
                       /* block exists in the front of zerofrom. */
                       if (!buffer_uptodate(bh))
                               partial = 1;
                       continue;
               } else if (block_start > zeroto) {  
                       /* block exists in the back of zeroto. */
                       partial = 1;
                       break;
               }

#ifdef RFS_FOR_2_6
               set_buffer_uptodate(bh);
               mark_buffer_dirty(bh);
#else
               mark_buffer_uptodate(bh, 1);
               mark_buffer_dirty(bh);
               down(&RFS_I(inode)->data_mutex);
               buffer_insert_inode_data_queue(bh, inode);
               up(&RFS_I(inode)->data_mutex);
#endif
       }

       /* if all buffers of a page were filled zero */ 
       if (!partial)
               SetPageUptodate(page);

out:
       flush_dcache_page(page);        
       kunmap_atomic(kaddr, KM_USER0);
       return err;     
}

/**
 *  extend a file with zero-fill
 * @param inode        inode
 * @param origin_size  file size before extend 
 * @param new_size     file size to extend      
 * @return     return 0 on success, errno on failure
 *
 * rfs doesn't allow holes. 
 */
int extend_with_zerofill(struct inode *inode, unsigned int origin_size, unsigned int new_size) 
{
       struct address_space *mapping = inode->i_mapping;
       struct super_block *sb = inode->i_sb;
       struct page *page = NULL;
       unsigned long index, final_index;
       unsigned long next_page_start, offset, next_offset;
       unsigned int origin_clusters, new_clusters, clusters_to_extend;
       unsigned zerofrom, zeroto;  /* offset within page */
       int err = 0;
       
       /* compare the number of required clusters with that of free clusters */
       origin_clusters = (origin_size + RFS_SB(sb)->cluster_size - 1)
                               >> RFS_SB(sb)->cluster_bits;
       new_clusters = (new_size + RFS_SB(sb)->cluster_size - 1)
                               >> RFS_SB(sb)->cluster_bits;
       clusters_to_extend = new_clusters - origin_clusters;

       if (clusters_to_extend && (clusters_to_extend > GET_FREE_CLUS(RFS_SB(sb)))) {
               DEBUG(DL2, "No space \n");
               return -ENOSPC;
       }       

       offset = origin_size;
       final_index = (new_size - 1) >> PAGE_CACHE_SHIFT; /* newsize isn't 0 */
       while ((index = (offset >> PAGE_CACHE_SHIFT)) <= final_index) {
               page = grab_cache_page(mapping, index);
               if (!page) { /* memory error */
                       DEBUG(DL0, "out of memory !!");
                       return -ENOMEM;
               }

               /* calculate zerofrom and zeroto */
               next_page_start = (index + 1) << PAGE_CACHE_SHIFT;
               next_offset = (new_size > next_page_start) ? next_page_start : new_size;

               zerofrom = offset & (PAGE_CACHE_SIZE - 1);
               zeroto = (next_offset - 1) & (PAGE_CACHE_SIZE - 1);

               err = rfs_page_zerofill(inode, page, zerofrom, zeroto, rfs_get_block);
               if (err) {
                       if (unlikely(err == -ENOSPC)) {
                               DEBUG(DL0, "The # of the real free clusters is different from super block.");
                               err = -EIO;
                       }
                       ClearPageUptodate(page);
                       unlock_page(page);
                       page_cache_release(page);       
                       DEBUG(DL1, "zero fill failed (err : %d)\n", err);
                       goto out;
               }

               offset = next_page_start;
               unlock_page(page);
               page_cache_release(page);
       }

out:
       return err;
}

/**
 *  truncate a file to a specified size
 * @param inode        inode
 *
 * support to reduce or enlarge a file 
 */
void rfs_truncate(struct inode *inode)
{
       struct super_block *sb = inode->i_sb;
       unsigned int num_clusters = 0;
       unsigned long origin_size, origin_mmu_private;
       int is_back = 0;
       int err;

       origin_size = RFS_I(inode)->trunc_start;
       origin_mmu_private = RFS_I(inode)->mmu_private; 

       /* check the validity */
       if (IS_RDONLY(inode))
               goto rollback_size; 

       if (!(S_ISREG(inode->i_mode) || S_ISDIR(inode->i_mode) || 
                               S_ISLNK(inode->i_mode)))
               goto rollback_size; 

       if (IS_IMMUTABLE(inode) || IS_APPEND(inode))
               goto rollback_size;

       /* RFS-log : start truncate */
       if ((loff_t) origin_size < (loff_t) inode->i_size) {
               /* if caller is inode_setattr, tr can be nested */
               err = rfs_log_start(sb, RFS_LOG_TRUNCATE_F, inode);
               is_back = 0;
       } else {
               err = rfs_log_start(sb, RFS_LOG_TRUNCATE_B, inode);
               is_back = 1;
       }
       if (err)
               goto rollback_size;

       /* transactin starts from here */
       if (RFS_I(inode)->i_state == RFS_I_FREE)
               /* RFS do not perform truncate of unlinked file */
               goto end_log;

       if (is_back) { /* reduce a file */
               num_clusters = (inode->i_size + RFS_SB(sb)->cluster_size - 1) 
                       >> RFS_SB(sb)->cluster_bits;

               err = dealloc_clusters(inode, num_clusters);
               if (err) {
                       /* 
                        * FIXME: media failure 
                        * Even though this failure may result in serious error,
                        * rfs_truncate can propagate it to the upper layer.
                        * This will make it difficult to debug the error 
                        * caused by media failure.
                        * Should we revive the dealloced clusters?
                        * Or, should we truncate the file anyway?
                        *
                        * We can mark the inode with media fail.
                        */
                       goto invalidate_hint;
               }
               set_mmu_private(inode, inode->i_size);
       } else if (origin_mmu_private < inode->i_size) {
               /* extending file with zero */
               err = extend_with_zerofill(inode,
                               (u32) origin_mmu_private, (u32) inode->i_size);
               if (err == -ENOSPC) {
                       DEBUG(DL2, "failed to enlarge a file");
                       goto end_log;
               } else if (err) {
                       DEBUG(DL2, "failed to enlarge a file");
                       truncate_inode_pages(inode->i_mapping, origin_size);
                       num_clusters = (origin_size + RFS_SB(sb)->cluster_size - 1) 
                               >> RFS_SB(sb)->cluster_bits;

                       err = dealloc_clusters(inode, num_clusters);
                       if (err)
                               DEBUG(DL2, "failed to reduce a file");

                       goto invalidate_hint;
               }
       } else {
               /* truncate forward but already zero filled, so do nothing */
       }

       /* invalidate hint info */
       rfs_invalidate_hint(inode);

       inode->i_blocks = (inode->i_size + SECTOR_SIZE - 1) >> SECTOR_BITS; 
       inode->i_mtime = inode->i_atime = CURRENT_TIME;
       rfs_mark_inode_dirty(inode);

       /* RFS-log : end truncate */
       if (rfs_log_end(sb, 0)) {
               /* should we mark the file with media failure */
               ;
       }

       return;

invalidate_hint:
       rfs_invalidate_hint(inode);
       RFS_I(inode)->mmu_private = origin_mmu_private;
       inode->i_size = (loff_t) origin_size;
       rfs_mark_inode_dirty(inode);
       rfs_log_end(sb, 1);
       return;

end_log:
       inode->i_size = (loff_t) origin_size;
       rfs_mark_inode_dirty(inode);
       rfs_log_end(sb, 1);
       return;

rollback_size:
       inode->i_size = (loff_t) origin_size;
       mark_inode_dirty(inode);
       return;
}

/**
 *  change an attribute of inode
 * @param dentry       dentry
 * @param attr         new attribute to set
 * @return             return 0 on success, errno on failure
 * 
 * it is only used for chmod, especially when read only mode be changed
 */
int rfs_setattr(struct dentry *dentry, struct iattr *attr)
{
       struct inode *inode = dentry->d_inode;
       struct buffer_head *bh = NULL;
       struct rfs_dir_entry *ep = NULL;
       int perm, err = 0;
       int tr_start = 0;

       CHECK_RFS_INODE(inode, -EINVAL);

       if (check_reserved_files(inode, NULL))
               return -EPERM;

       if (attr->ia_valid & ATTR_SIZE) {
               if ((loff_t) inode->i_size < (loff_t) attr->ia_size) {
                       unsigned int end_index = inode->i_size >>
                               PAGE_CACHE_SHIFT;
                       unsigned int mmu_private = RFS_I(inode)->mmu_private;
                       unsigned int next_page_start, to_size;

                       next_page_start = (end_index + 1) << PAGE_CACHE_SHIFT;
                       to_size = ((loff_t) attr->ia_size >
                                       (loff_t) next_page_start) ?
                               next_page_start : (unsigned int) attr->ia_size;
                       if (mmu_private < to_size) {
                               /*
                                * RFS-log : fill last page with zero
                                * to avoid race-condition with writepage
                                * (recursive transaction)
                                */
                               if (rfs_log_start(inode->i_sb,
                                                 RFS_LOG_TRUNCATE_F,
                                                 inode))
                                       return -EIO;
                               
                               tr_start = 1; /* transaction starts */

                               err = extend_with_zerofill(inode,
                                               (u32) mmu_private,
                                               (u32) to_size); 

                               if (err) {
                                       rfs_log_end(inode->i_sb, err);
                                       return err;
                               }

                               /* update zerofilled inode's size */
                               inode->i_size = to_size;
                       }
               }

               /* keep current inode size for truncate operation */
               RFS_I(inode)->trunc_start = inode->i_size;
       }
               
       if (attr->ia_valid & ATTR_MODE) {
               perm = attr->ia_mode & 0777;
               ep = get_entry_with_cluster(inode->i_sb, 
                               RFS_I(inode)->p_start_clu,
                               RFS_I(inode)->index, &bh);
               if (IS_ERR(ep)) {
                       if (PTR_ERR(ep) == -EFAULT)
                               err = -ENOENT;
                       else
                               err = PTR_ERR(ep);
                       goto out;
               }

               if (perm & 0222)
                       ep->attr &= ~ATTR_READONLY;
               else
                       ep->attr |= ATTR_READONLY;

               mark_buffer_dirty(bh);
       }

       err = inode_setattr(inode, attr);

out:
       brelse(bh);

       if (tr_start)
               rfs_log_end(inode->i_sb, err);

       return err;
}

/**
 *  check a permission of inode
 * @param inode                inode
 * @param mode         mode
 * @param nd           nameidata
 * @return             return 0 on success, EPERM on failure
 *
 * System file (log or pool file) can not be accessed 
 */
#ifdef RFS_FOR_2_6
#ifdef RFS_FOR_2_6_27
int rfs_permission(struct inode *inode, int mode)
#else
int rfs_permission(struct inode *inode, int mode, struct nameidata *nd)
#endif
#else
int rfs_permission(struct inode *inode, int mode)
#endif
{
       if (mode & (MAY_WRITE | MAY_READ))
               return check_reserved_files(inode, NULL);

       return 0;    /* all operations are permitted */
}

#ifdef CONFIG_GCOV_PROFILE
#undef loff_t
#endif

/**
 *  write up to count bytes to file at speicified position 
 * @param file         file
 * @param buf          buffer pointer
 * @param count                number of bytes to write
 * @param ppos         offset in file
 * @return             return write bytes on success, errno on failure
 * 
 * use pre-allocation for reducing working logs
 */
static ssize_t rfs_file_write(struct file *file, const char *buf,
               size_t count, loff_t *ppos)
{
       struct inode *inode;
       ssize_t ret;
       int err;

#ifdef RFS_FOR_2_6_19
       ret = do_sync_write(file, buf, count, ppos);
#else
       ret = generic_file_write(file, buf, count, ppos);
#endif
       if (ret <= 0)
               return ret;

       inode = file->f_dentry->d_inode->i_mapping->host;
       if ((file->f_flags & O_SYNC) || IS_SYNC(inode)) {
               err = rfs_log_force_commit(inode->i_sb, inode);
               if (err)
                       return -EIO;
       }

       return ret;
}

/**
 *  flush all dirty buffers of inode include data and meta data
 * @param file         file pointer
 * @param dentry       dentry pointer
 * @param datasync     flag
 * @return return 0 on success, EPERM on failure
 */
static int rfs_file_fsync(struct file * file, struct dentry *dentry, int datasync)
{
       struct inode *inode = dentry->d_inode;
       struct super_block *sb = inode->i_sb;
       int ret = 0, err = 0; 
       
       /* data commit */
       ret = rfs_sync_inode(inode, 1, 1);

       /* meta-commit deferred tr */
       if (tr_deferred_commit(sb) && RFS_LOG_I(sb)->inode &&
                       (RFS_LOG_I(sb)->inode == inode)) {
               err = rfs_log_force_commit(inode->i_sb, inode); 
               if (err && !ret)
                       ret = err;
       }
       
       return ret;
}

#ifdef CONFIG_RFS_SYNC_ON_CLOSE
/**
 *  flush modified data of file object
 * @inode      inode of file object to flush
 * @file       file object to flush
 *
 * It is only called when all files are closed, that is inode is released
 */
static int rfs_file_release(struct inode * inode, struct file * file)
{
#ifdef RFS_FOR_2_6
       filemap_fdatawrite(inode->i_mapping);
       filemap_fdatawait(inode->i_mapping);
#endif
       return rfs_file_fsync(file, file->f_dentry, 0);
}
#endif

struct file_operations rfs_file_operations = {
#ifdef RFS_FOR_2_6_19
       .read           = do_sync_read,
       .aio_read       = generic_file_aio_read,
       .aio_write      = generic_file_aio_write,
#else
       .read           = generic_file_read,
#endif
       .write          = rfs_file_write,
       .mmap           = generic_file_mmap,
       .fsync          = rfs_file_fsync,
#ifdef CONFIG_RFS_SYNC_ON_CLOSE
       .release        = rfs_file_release,
#endif
};

struct inode_operations rfs_file_inode_operations = {
       .truncate       = rfs_truncate,
       .permission     = rfs_permission,
       .setattr        = rfs_setattr,
#ifdef CONFIG_RFS_FS_XATTR
       .setxattr       = rfs_xattr_set,
       .getxattr       = rfs_xattr_get,
       .listxattr      = rfs_xattr_list,
       .removexattr    = rfs_xattr_delete,
#endif
//     .check_acl      = generic_check_acl,
};

/**
 *  translate index into a logical block
 * @param inode                inode
 * @param iblock       index
 * @param bh_result    buffer head pointer
 * @param create       flag whether new block will be allocated
 * @return             returns 0 on success, errno on failure 
 *
 * if there aren't logical block, allocate new cluster and map it
 */
#ifdef RFS_FOR_2_6
int rfs_get_block(struct inode *inode, sector_t iblock, struct buffer_head *bh_result, int create)
#else
int rfs_get_block(struct inode *inode, long iblock, struct buffer_head *bh_result, int create)
#endif
{
       unsigned long phys = 0;
       struct super_block *sb = inode->i_sb;
       unsigned int new_clu;
       int ret = 0;

#ifdef RFS_FOR_2_4
       lock_kernel();
#endif

       ret = rfs_bmap(inode, iblock, &phys);
       if (!ret) {
#ifdef RFS_FOR_2_6
               map_bh(bh_result, sb, phys);
#else          
               bh_result->b_dev = inode->i_dev;
               bh_result->b_blocknr = phys;
               bh_result->b_state |= (1UL << BH_Mapped);
#endif
               goto out;
       }

       ret = -EIO;
       if (!create)
               goto out;

       if (iblock != (RFS_I(inode)->mmu_private >> sb->s_blocksize_bits))
               goto out;

       if (!(iblock & (RFS_SB(sb)->blks_per_clu - 1))) {
               ret = alloc_cluster(inode, &new_clu);
               if (ret)
                       goto out;
       }

       RFS_I(inode)->mmu_private += sb->s_blocksize;
       ret = rfs_bmap(inode, iblock, &phys);
       if (ret) {
               RFS_I(inode)->mmu_private -= sb->s_blocksize;
               RFS_BUG("iblock(%ld) doesn't have a physical mapping", 
                               (long int)iblock);
               goto out;
       }

#ifdef RFS_FOR_2_6
       set_buffer_new(bh_result);
       map_bh(bh_result, sb, phys);
#else          
       bh_result->b_dev = inode->i_dev;
       bh_result->b_blocknr = phys;
       bh_result->b_state |= (1UL << BH_Mapped);
       bh_result->b_state |= (1UL << BH_New);
#endif

out:
#ifdef RFS_FOR_2_4
       unlock_kernel();
#endif
       return ret;
}

/**
 *  translation index into logical block number
 * @param inode                inode   
 * @param index                index number    
 * @param[out] phys    logical block number
 * @return     returns 0 on success, errno on failure  
 * @pre                FAT16 root directory's inode does not invoke this function      
 */
int rfs_bmap(struct inode *inode, long index, unsigned long *phys)
{
       struct super_block *sb = inode->i_sb;
       struct rfs_sb_info *sbi = RFS_SB(sb);
       unsigned int cluster, offset, num_clusters;
       unsigned int last_block;
       unsigned int clu, prev, next; 
       int err = 0;

       fat_lock(sb);

       cluster = index >> sbi->blks_per_clu_bits;
       offset = index & (sbi->blks_per_clu - 1);

       /* check hint info */
       num_clusters = cluster;
       err = rfs_lookup_hint(inode, &num_clusters, &clu);
       if (err)
               goto out;

       last_block = (RFS_I(inode)->mmu_private + (sb->s_blocksize - 1))
                               >> sb->s_blocksize_bits;
       if (index >= last_block) {
               err = -EFAULT;
               goto out;
       }

       err = find_cluster(sb, clu, num_clusters, &prev, &next);
       if (err)
               goto out;

       /* update hint info */
       rfs_update_hint(inode, prev, cluster);

       *phys = START_BLOCK(prev, sb) + offset;
out:
       fat_unlock(sb);

       return err;
}
