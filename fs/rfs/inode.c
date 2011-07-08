/**
 * @file       fs/rfs/inode.c
 * @brief      common inode operations
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

#include <linux/fs.h>
#include <linux/rfs_fs.h>
#include <linux/smp_lock.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
#include <linux/mpage.h>
#include <linux/backing-dev.h>
#endif

#include "rfs.h"
#include "log.h"

#ifdef RFS_FOR_2_6

ssize_t rfs_direct_IO(int, struct kiocb *, const struct iovec *,
               loff_t, unsigned long);

#define GET_ENTRY_TIME(ep, inode)                                      \
do {                                                                   \
       inode->i_ctime.tv_sec = inode->i_atime.tv_sec =                 \
               inode->i_mtime.tv_sec =                                 \
               entry_time(GET16(ep->mtime), GET16(ep->mdate));         \
       inode->i_ctime.tv_nsec = inode->i_mtime.tv_nsec =               \
               inode->i_atime.tv_nsec = 0;                             \
} while(0);

#else  /* !RFS_FOR_2_6 */

int rfs_direct_IO (int, struct inode *, struct kiobuf *, unsigned long, int);

#define GET_ENTRY_TIME(ep, inode)                                      \
do {                                                                   \
       inode->i_ctime = inode->i_atime= inode->i_mtime =               \
               entry_time(GET16(ep->mtime), GET16(ep->mdate));         \
} while(0);
#endif /* RFS_FOR_2_6 */

/**
 *  read a specified page
 * @param file         file to read
 * @param page         page to read
 * @return             return 0 on success
 */
static int rfs_readpage(struct file *file, struct page *page)
{
#ifdef RFS_FOR_2_6
       return mpage_readpage(page, rfs_get_block);
#else
       return block_read_full_page(page, rfs_get_block);
#endif
}

/**
 *  write a specified page
 * @param page to write page
 * @param wbc  writeback control       
 * @return     return 0 on success, errno on failure
 */
#ifdef RFS_FOR_2_6
static int rfs_writepage(struct page *page, struct writeback_control *wbc)
{
       return block_write_full_page(page, rfs_get_block, wbc);
}
#else
static int rfs_writepage(struct page *page)
{
       struct inode *inode = page->mapping->host;
       int ret;

       down(&RFS_I(inode)->data_mutex);
       ret = block_write_full_page(page, rfs_get_block);
       up(&RFS_I(inode)->data_mutex);

       return ret;
}
#endif

#ifdef RFS_FOR_2_6
/**
 *  read multiple pages
 * @param file         file to read
 * @param mapping       address space to read
 * @param pages                page list to read       
 * @param nr_pages     number of pages 
 * @return             return 0 on success, errno on failure
 */
static int rfs_readpages(struct file *file, struct address_space *mapping,
               struct list_head *pages, unsigned nr_pages)
{
       return mpage_readpages(mapping, pages, nr_pages, rfs_get_block);
}

/**
 *  write multiple pages
 * @param mapping       address space to write
 * @param wbc          writeback_control       
 * @return             return 0 on success, errno on failure
 */
static int rfs_writepages(struct address_space *mapping, struct writeback_control *wbc)
{
       return mpage_writepages(mapping, wbc, rfs_get_block);
}
#endif

/**
 *  read some paritial page to write rest page
 * @param file         to read file
 * @param page         specified page to read
 * @param from         start position in page
 * @param to           bytes counts to prepare in page 
 * @return             return 0 on success, errno on failure
 *
 * This function requires addtional code saving inode->i_size because there is
 * case when inode->i_size is chagned after cont_prepare_write.  
 */
#ifdef RFS_FOR_2_4
static int rfs_prepare_write(struct file *file, struct page *page, unsigned from, unsigned to)
{
       struct inode *inode = page->mapping->host;
       unsigned page_start_offset = page->index << PAGE_CACHE_SHIFT;
       unsigned mmu_private = RFS_I(inode)->mmu_private;
       unsigned mmu_private_in_page = mmu_private & (PAGE_CACHE_SIZE - 1);
       unsigned newfrom;
       char *kaddr;
       int ret = 0;
               
       if (rfs_log_start(inode->i_sb, RFS_LOG_WRITE, inode))
               return -EIO;

       if ((page_start_offset + from) > mmu_private) {
               if (page_start_offset > mmu_private)
                       newfrom = 0;
               else
                       newfrom = mmu_private_in_page;
       } else 
                       newfrom = from;

       if (page_start_offset > mmu_private) {
               /* zerofill the previous hole pages */
               ret = extend_with_zerofill(inode, mmu_private, page_start_offset);
               if (ret)
                       goto out;
       }

       ret = rfs_block_prepare_write(inode, page, newfrom, to-1, rfs_get_block);
       if (ret)
               goto out;

       if (from > newfrom) {
               /* memset & commit the previous hole in page */
               kaddr = page_address(page);
               memset(kaddr+newfrom, 0, from-newfrom);
               flush_dcache_page(page);
               rfs_block_commit_write(inode, page, newfrom, from);
               kunmap(page);
       } 

       return ret;

out:
       RFS_I(inode)->trunc_start = RFS_I(inode)->mmu_private;
       rfs_log_end(inode->i_sb, ret);

       return ret;
}

#else
#ifndef RFS_FOR_2_6_24
static int rfs_prepare_write(struct file *file, struct page *page, unsigned from, unsigned to)
{
       struct inode *inode = page->mapping->host;
       int ret = 0;
               
       if (rfs_log_start(inode->i_sb, RFS_LOG_WRITE, inode))
               return -EIO;

       ret = cont_prepare_write(page, from, to, rfs_get_block,
                       &(RFS_I(inode)->mmu_private));

       if (ret) {
               RFS_I(inode)->trunc_start = RFS_I(inode)->mmu_private;
               rfs_log_end(inode->i_sb, ret);
       }

       return ret;
}
#endif /* ! RFS_FOR_2_6_24 */
#endif

#ifndef RFS_FOR_2_6_24
/**
 *  write a specified page
 * @param file         to write file
 * @param page         page descriptor
 * @param from         start position in page          
 * @param to           end position in page    
 * @return             return 0 on success, errno on failure
 */
static int rfs_commit_write(struct file *file, struct page *page, unsigned from, unsigned to)
{
       struct inode *inode = page->mapping->host;
       int ret;

#ifdef RFS_FOR_2_6
       ret = generic_commit_write(file, page, from, to);
#else
       down(&RFS_I(inode)->data_mutex);
       ret = generic_commit_write(file, page, from, to);
       up(&RFS_I(inode)->data_mutex);
#endif

       if (rfs_log_end(inode->i_sb, ret))
               return -EIO;

       return ret;
}
#endif /* ! RFS_FOR_2_6_24 */

#ifdef RFS_FOR_2_6_24
static int rfs_write_begin(struct file *file, struct address_space *mapping,
               loff_t pos, unsigned len, unsigned flags,
               struct page **pagep, void **fsdata){
       struct inode *inode = mapping->host;
       int ret = 0;

       if (rfs_log_start(inode->i_sb, RFS_LOG_WRITE, inode))
               return -EIO;

       *pagep = NULL;
       ret = cont_write_begin(file, mapping, pos, len, flags, pagep, fsdata,
                       rfs_get_block, &(RFS_I(inode)->mmu_private));

       return ret;
}
static int rfs_write_end(struct file *file, struct address_space *mapping,
               loff_t pos, unsigned len, unsigned copied,
               struct page *page, void *fsdata){
       struct inode *inode = mapping->host;
       int ret = 0;

       ret = generic_write_end(file, mapping, pos, len, copied, page, fsdata);

       if (ret < len)
               RFS_BUG("Failed to write whole data(%u from %u pos: %lu)\n", ret, len, (unsigned long)pos);

       if (ret) {
               RFS_I(inode)->trunc_start = RFS_I(inode)->mmu_private;
               rfs_log_end(inode->i_sb, ret);
       }
       return ret;
}
#endif /* RFS_FOR_2_6_24 */

struct address_space_operations rfs_aops = {
        .readpage      = rfs_readpage,
        .writepage     = rfs_writepage,
       .sync_page      = block_sync_page,
#ifdef RFS_FOR_2_6_24
       .write_begin    = rfs_write_begin,
       .write_end      = rfs_write_end,
#else
       .prepare_write  = rfs_prepare_write,
        .commit_write  = rfs_commit_write,
#endif
       .direct_IO      = rfs_direct_IO,
#ifdef RFS_FOR_2_6
       .readpages      = rfs_readpages,
       .writepages     = rfs_writepages,
#endif
};

/*
 *  get an unique inode number
 * @param dir          parent directory
 * @param index        dir entry's offset in parent dir's cluster(s)
 * @param[out] ino     return ino
 * @return             return 0 on success, errno on failure
 */
int rfs_iunique(struct inode *dir, unsigned int index, unsigned long *ino)
{
       struct super_block *sb = dir->i_sb;
       struct rfs_sb_info *sbi = RFS_SB(sb);

       /*
        * NOTE: RFS uses hash value for iunique
        * which is byte address of index >> bits of dir entry's size.
        * 0 ~ 15th entries are reserved, because reserved area has one sector
        * at least. Especially 1 belongs to root inode
        */
       if ((RFS_I(dir)->start_clu != sbi->root_clu) || IS_FAT32(sbi)) {
               unsigned int offset, cluster_offset;
               unsigned int prev, next;
               int err;

               /* in FAT32 root dir or sub-directories */
               offset = index << DENTRY_SIZE_BITS;
               cluster_offset = offset >> sbi->cluster_bits;
               fat_lock(sb);
               err = find_cluster(sb, RFS_I(dir)->start_clu, 
                               cluster_offset, &prev, &next);
               fat_unlock(sb);
               if (err)
                       return err;

               offset &= sbi->cluster_size - 1;
               offset += (START_BLOCK(prev, sb) << sb->s_blocksize_bits);
               *ino = offset >> DENTRY_SIZE_BITS;
       } else {
               /* in root directory */
               *ino = (sbi->root_start_addr >> DENTRY_SIZE_BITS) + index;
       }

       return 0;
}

/**
 *  fill up the RFS-specific inode
 * @param inode                inode
 * @param ep           dir entry
 * @param p_start_clu  start cluster number of parent inode
 * @param dentry       dir entry index
 * @return             return 0 on success, errno on failure
 *
 * it is only invoked when new inode is allocated
 */
int fill_inode(struct inode *inode, struct rfs_dir_entry *ep, unsigned int p_start_clu, unsigned int dentry)
{
       struct super_block *sb = inode->i_sb;
       unsigned int type;
       unsigned int size;
       int err = 0, link_count = 0;
       int num_clus = 0;
       
       /* fill the RFS-specific inode info */
       RFS_I(inode)->p_start_clu = p_start_clu;
       RFS_I(inode)->index = dentry;
       RFS_I(inode)->hint_last_clu = 0;
       RFS_I(inode)->hint_last_offset = 0;
       RFS_I(inode)->i_state = RFS_I_ALLOC;

       /* sanity code */
       if (START_CLUSTER(ep) && IS_INVAL_CLU(RFS_SB(sb), START_CLUSTER(ep))) {
               err = -EIO;
               RFS_BUG("dir entry(%u, %u) has corrupted start_clu(%u)\n",
                               p_start_clu, dentry, START_CLUSTER(ep));
               goto bad_inode;
       }

       /* set start_clu and last_clu in private */
       if (RFS_POOL_I(sb) &&
                       (START_CLUSTER(ep) == RFS_POOL_I(sb)->start_cluster)) {
               RFS_I(inode)->start_clu = RFS_POOL_I(sb)->start_cluster;
               RFS_I(inode)->last_clu = RFS_POOL_I(sb)->last_cluster;
       } else if (START_CLUSTER(ep)) {
               /* pool dose exits or dir entry is not for pool file */
               unsigned int last_clu;
                       
               RFS_I(inode)->start_clu = START_CLUSTER(ep);
               num_clus = find_last_cluster(inode, &last_clu);
               if (num_clus < 0) {
                       err = num_clus;
                       DEBUG(DL0, "No last cluster (err : %d)\n", err);
                       goto bad_inode;
               }
               RFS_I(inode)->last_clu = last_clu;
       } else {
               /* cluster was not assigned to inode */
               RFS_I(inode)->start_clu = CLU_TAIL;
               RFS_I(inode)->last_clu = CLU_TAIL;
       }

       spin_lock_init(&RFS_I(inode)->write_lock);

       type = entry_type(ep);

       /* set i_size, i_mode, i_op, i_fop, i_mapping, i_nlink */
       if (type == TYPE_DIR) {
               /* directory always has zero-size by fat spec */
               inode->i_size = num_clus << RFS_SB(sb)->cluster_bits;

               inode->i_mode = S_IFDIR;
               inode->i_op = &rfs_dir_inode_operations;
               inode->i_fop = &rfs_dir_operations;
               link_count = count_subdir(sb, RFS_I(inode)->start_clu);
               if (unlikely(link_count < 2)) {
                       err = link_count;
                       goto bad_inode;
               } else {
                       inode->i_nlink = link_count;
               }
       } else if (type == TYPE_FILE) {
               if ((ep->cmsec & SYMLINK_MARK) == SYMLINK_MARK) {
                       inode->i_mode = S_IFLNK;
                       inode->i_op = &page_symlink_inode_operations;
                       inode->i_mapping->a_ops = &rfs_aops;
               } else {
                       inode->i_mode = S_IFREG;
                       inode->i_op = &rfs_file_inode_operations;
                       inode->i_fop = &rfs_file_operations;
                       inode->i_mapping->a_ops = &rfs_aops;
               }
               inode->i_nlink = 1;

               /* set inode->i_size */
               if (RFS_POOL_I(sb) && (START_CLUSTER(ep) ==
                                       RFS_POOL_I(sb)->start_cluster)) {
                       /* just temporary */
                       inode->i_size = RFS_SB(sb)->cluster_size;
               } else {
                       int rcv_clus;

                       size = GET32(ep->size);
                       
                       rcv_clus = (size + RFS_SB(sb)->cluster_size - 1)
                               >> RFS_SB(sb)->cluster_bits;

                       if (rcv_clus < num_clus) {
                               /* RFS-log : data replay */
                               DEBUG(DL0, "data recover(%u, %u, %u):"
                                               " %d != %d",
                                               RFS_I(inode)->p_start_clu,
                                               RFS_I(inode)->index,
                                               RFS_I(inode)->start_clu,
                                               num_clus, rcv_clus);

                               inode->i_size = size;
                               RFS_I(inode)->trunc_start = num_clus <<
                                       RFS_SB(sb)->cluster_bits;
                               rfs_truncate(inode);
                       } else if (rcv_clus > num_clus) {
                               /* just set inode->i_size not modify ep->size */
                               DEBUG(DL0, "size recover (%u, %u, %u):"
                                               "  %d != %d",
                                               RFS_I(inode)->p_start_clu,
                                               RFS_I(inode)->index,
                                               RFS_I(inode)->start_clu,
                                               num_clus, rcv_clus);
                               inode->i_size = num_clus <<
                                       RFS_SB(sb)->cluster_bits;
                       } else
                               inode->i_size = size;
               }
       } else {
               DEBUG(DL0, "dos entry type(%x) is invalid\n", type);
               err = -EINVAL;
               goto bad_inode;
       }

       inode->i_mode |= 0777; 
       if (ep->attr & ATTR_READONLY) /* handle read-only case */
               inode->i_mode &= ~0222;

       inode->i_uid = 0;
       inode->i_gid = 0;
       inode->i_version = 0;
       GET_ENTRY_TIME(ep, inode); 

#ifndef RFS_FOR_2_6_19
       inode->i_blksize = sb->s_blocksize;
#endif

       inode->i_blocks = (inode->i_size + SECTOR_SIZE - 1) >> SECTOR_BITS;

       set_mmu_private(inode, inode->i_size);

#ifdef RFS_FOR_2_4
       init_MUTEX(&RFS_I(inode)->data_mutex);
       init_timer(&RFS_I(inode)->timer);
       RFS_I(inode)->timer.function = rfs_data_wakeup;
       RFS_I(inode)->timer.data = (unsigned long) RFS_I(inode);
#endif

       return 0;

bad_inode:
       make_bad_inode(inode);
       return err;
}

/**
 *  create a new inode
 * @param dir          inode of parent directory
 * @param dentry       dentry associated with inode will be created
 * @param type         inode type
 * @return             return inode pointer on success, erro on failure
 */
struct inode *rfs_new_inode(struct inode *dir, struct dentry *dentry, unsigned int type)
{
       struct inode *inode = NULL;
       struct buffer_head *bh = NULL;
       struct rfs_dir_entry *ep = NULL;
       unsigned long new_ino;
       unsigned int index;
       int ret = 0;

       /* get a new inode */
       if (!(inode = new_inode(dir->i_sb)))
               return ERR_PTR(-ENOMEM); /* memory error */

       /* initialize dir entry and extend entries */
       index = build_entry(dir, inode, 0, type, dentry->d_name.name);
       if ((int) index < 0) {
               ret = index;
               goto alloc_failure;
       }

       ep = get_entry(dir, index, &bh);
       if (IS_ERR(ep)) {
               ret = PTR_ERR(ep);
               goto alloc_failure;
       }

       /* fill inode info */
       ret = fill_inode(inode, ep, RFS_I(dir)->start_clu, index);
       if (ret)
               goto alloc_failure;
                       
       if (type == TYPE_DIR) {
               /* increase parent's i_nlink */
               dir->i_nlink++;
       } else if (type == TYPE_FILE || type == TYPE_SYMLINK) {
               /* initialize it when only create time */
#if defined(CONFIG_PREEMPT) && defined(RFS_FOR_2_6_24) /* kernel_64m is tr patched and inode->i_mapping->nrpages is ok! 
                                                       * but for 2.6.24 (aquila) we need inode->i_mapping ? */

               truncate_inode_pages(inode->i_mapping, 0); /* it works on tv with rt patched 2.6.24 
                                                            * but...
                                                            * unmap_mapping_range(inode->i_mapping, 0, 0, 0);
                                                            * is more correct? */
#else
               inode->i_mapping->nrpages = 0;
#endif
       }

       /* get new inode number */
       ret = rfs_iunique(dir, index, &new_ino);
       if (ret) {
               if (type == TYPE_DIR) {
                       inode->i_nlink--;
                       dir->i_nlink--;
               }
               goto alloc_failure;
       }
       inode->i_ino = new_ino;
       insert_inode_hash(inode);

       /* 
        * we already reigsterd the inode to the transaction
        * in the above build_entry() 
        */
       mark_inode_dirty(inode);

       dir->i_mtime = dir->i_atime = CURRENT_TIME;
       rfs_mark_inode_dirty(dir);

       brelse(bh);
       return inode;

alloc_failure:
       inode->i_nlink--;
       RFS_BUG_ON(inode->i_nlink);
       iput(inode);

       if (!IS_ERR(bh))
               brelse(bh);

       return ERR_PTR(ret);
}

/**
 *  delete blocks of inode and clear inode
 * @param inode        inode will be removed
 * @return     return 0 on success, errno on failure
 *
 * It will be invoked at the last iput if i_nlink is zero
 */
void rfs_delete_inode(struct inode *inode)
{
#ifdef RFS_FOR_2_4
       lock_kernel();
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,14)
       truncate_inode_pages(&inode->i_data, 0);
#endif

       if (!is_bad_inode(inode)) {
               /* RFS-log : start of transaction */
               if (rfs_log_start(inode->i_sb, RFS_LOG_DEL_INODE, inode))
                       goto out;

               /* actual de-allocation of clusters */
               rfs_detach_candidate(inode);

               /* RFS-log : end of transaction */
               rfs_log_end(inode->i_sb, 0);
       }

out:
       clear_inode(inode);

#ifdef RFS_FOR_2_4
       unlock_kernel();
#endif
}

/**
 *  deallocate clusters and remove entries for inode
 * @param dir  parent directory inode  
 * @param inode        inode will be removed
 * @return     return 0 on success, errno on failure
 */
int rfs_delete_entry(struct inode *dir, struct inode *inode)
{
       struct buffer_head *bh = NULL;
       int ret;

       /*
        * free cluster(s) that were allocated to inode 
        * will be reserved such as candidate free clusters 
        */
       ret = rfs_attach_candidate(inode);
       if (ret)
               return ret; 

       /* remove dos & extent entries */
       ret = remove_entry(dir, RFS_I(inode)->index, &bh);
       brelse(bh);
       if (ret)
               return ret;

       /* decrease i_nlink */
       inode->i_nlink--;
       if (S_ISDIR(inode->i_mode)) {
               inode->i_nlink--;
               dir->i_nlink--;
       }
       if (unlikely(inode->i_nlink)) {
               RFS_BUG("nlink of inode is not zero (%u) \n", inode->i_nlink);
               return -EIO;
       }

       inode->i_mtime = inode->i_atime = CURRENT_TIME;
       dir->i_mtime = dir->i_atime = CURRENT_TIME; 

       /* change ino to avoid sharing */
       remove_inode_hash(inode);

       /* do not change the order of statements for sync with write_inode() */
       spin_lock(&RFS_I(inode)->write_lock);
       inode->i_ino = RFS_SB(dir->i_sb)->highest_d_ino;
       RFS_I(inode)->i_state = RFS_I_FREE;
       spin_unlock(&RFS_I(inode)->write_lock);

       insert_inode_hash(inode);

       /* 
        * we already reigsterd the inode to the transaction
        * in the above remove_entry() 
        */
       mark_inode_dirty(inode);
       rfs_mark_inode_dirty(dir);

       return 0;
}

#ifdef CONFIG_RFS_IGET4
/**
 *  fill up the in-core inode 
 *  @param inode       created new inode       
 *  @param private     private argument to fill
 *
 *  it has a same role with fill_inode
 */
void rfs_read_inode2(struct inode *inode, void *private)
{
       struct rfs_iget4_args *args;

       args = (struct rfs_iget4_args *) private;

       fill_inode(inode, args->ep, args->p_start_clu, args->index);
}
#endif

/**
 * write a inode to dir entry
 * @param inode                inode
 * @param data_commit  flush dirty data and update dir entry's size
 * @param wait         flag whether inode info is flushed
 * @return             return 0 on success, errno on failure
 */
int rfs_sync_inode(struct inode *inode, int data_commit, int wait)
{
       struct super_block *sb = inode->i_sb;
       struct buffer_head *bh = NULL;
       struct rfs_dir_entry *ep = NULL;
       unsigned long backup_ino;
       int ret = 0;
       int committed = 0;
#ifdef RFS_FOR_2_6
       int err = 0;
#endif

       if (inode->i_ino == ROOT_INO) 
               return 0;

       CHECK_RFS_INODE(inode, -EINVAL);

       /* update size of directory entry except directory */
       if (!S_ISDIR(inode->i_mode) && data_commit) {
               DEBUG(DL3, "%u, %u, %u, %llu",
                               RFS_I(inode)->p_start_clu,
                               RFS_I(inode)->index,
                               RFS_I(inode)->start_clu,
                               inode->i_size);
                       
#ifdef RFS_FOR_2_6
               ret = filemap_fdatawrite(inode->i_mapping);
               err = filemap_fdatawait(inode->i_mapping);
               if (err && !ret)
                       ret = err;
#else
               rfs_data_down(RFS_I(inode));
               ret = fsync_inode_data_buffers(inode);
               rfs_data_up(RFS_I(inode));
#endif
               if (!ret)
                       committed = 1;
       }

       backup_ino = inode->i_ino;
       ep = get_entry_with_cluster(sb, RFS_I(inode)->p_start_clu, 
                       RFS_I(inode)->index, &bh);
       if (IS_ERR(ep))
               return -EIO;

       spin_lock(&RFS_I(inode)->write_lock);
 
       /*
        * confirm that inode is not changed by rename/unlink
        * during getting entry
        */
       if (backup_ino != inode->i_ino || RFS_I(inode)->i_state == RFS_I_FREE) {
               spin_unlock(&RFS_I(inode)->write_lock);
               brelse(bh);
               return 0;
       }
 
       /* if data are committed, update entry's size */
       if (committed)
               SET32(ep->size, inode->i_size);
 
       /* update directory entry's start cluster number */
       if (RFS_I(inode)->start_clu != CLU_TAIL) {
               /* sanity check : no case of overwriting non empty file */
               if ((START_CLUSTER(ep) != 0) &&
                   (START_CLUSTER(ep) != RFS_I(inode)->start_clu)) {
                       spin_unlock(&RFS_I(inode)->write_lock);
                       brelse(bh);
                       RFS_BUG("inode's start cluster is corrupted (%u, %u)\n",
                                       START_CLUSTER(ep),
                                       RFS_I(inode)->start_clu);
                       return -EIO;
               }

               SET16(ep->start_clu_lo, RFS_I(inode)->start_clu);
               SET16(ep->start_clu_hi, RFS_I(inode)->start_clu >> 16);
       } else {
               SET16(ep->start_clu_lo, 0);
               SET16(ep->start_clu_hi, 0);
       }

       /* 
        * Update directory entry's mtime and mdate into inode's mtime
        * We mistake inode's mtime for current time at RFS-1.2.0 ver 
        * to update directory entry's mtime and mdate
        */
       set_entry_time(ep, inode->i_mtime);

       /*
        * All changes are done. After unlock, create() on the unlinked entry
        * can change buffer as it wants
        */
       spin_unlock(&RFS_I(inode)->write_lock);

       if (wait) {
               mark_buffer_dirty(bh);
#ifdef RFS_FOR_2_6
               err = sync_dirty_buffer(bh);
               if (err && !ret)
                       ret = err;
#else
               ll_rw_block(WRITE, 1, &bh);
               wait_on_buffer(bh);
#endif
       } else
               rfs_mark_buffer_dirty(bh, sb);

       brelse(bh);
       return ret;
}

/**
 * write inode 
 * @param inode                inode
 * @param wait         flag whether inode info is flushed
 * @return             return 0 on success, errno on failure for 2.6   
 */
#ifdef RFS_FOR_2_6
int rfs_write_inode(struct inode *inode, int wait)
#else
void rfs_write_inode(struct inode *inode, int wait)
#endif
{
       int ret = 0;

       if (current->flags & PF_MEMALLOC)
               goto out;

       /* We return 0 despite teh read-only mode. see ext3_write_inode */
       if (IS_RDONLY(inode))
               goto out;

       if (RFS_I(inode)->i_state == RFS_I_FREE)
               goto out;

#ifdef RFS_FOR_2_4
       lock_kernel();
#endif
               
       ret = rfs_sync_inode(inode, 1, wait);

#ifdef RFS_FOR_2_4
       unlock_kernel();
#endif

out:
#ifdef RFS_FOR_2_6
       return ret;
#else
       return;
#endif
}
