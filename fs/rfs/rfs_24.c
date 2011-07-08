/**
 *  @file       fs/rfs/rfs_24.c
 *  @brief      Kernel version 2.4 specified functions
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
#include <linux/module.h>
#include <linux/init.h>
#include <linux/iobuf.h>
#include <linux/rfs_fs.h>

#include "rfs.h"
#include "log.h"

/**
 *  prepare the blocks and map them 
 * @param inode        inode    
 * @param page page pointer
 * @param from start offset within page        
 * @param to   last offset within page  
 * @param get_block    get_block funciton       
 * @return     return 0 on success, errno on failure
 */
int rfs_block_prepare_write(struct inode * inode, struct page * page, unsigned from, unsigned to, get_block_t *get_block) 
{
       struct buffer_head *bh, *head;
       unsigned long block;
       unsigned block_start, block_end, blocksize, bbits; 
       int err = 0;
       char *kaddr = kmap(page);

       bbits = inode->i_blkbits;
       blocksize = 1 << bbits;

       if (!page->buffers) 
               create_empty_buffers(page, inode->i_dev, blocksize);
       head = page->buffers;

       block = page->index << (PAGE_CACHE_SHIFT - bbits); /* start block # */

       /* we allocate buffers and map them */
       for(bh = head, block_start = 0; bh != head || !block_start;
               block++, block_start = block_end + 1, bh = bh->b_this_page) {
               if (!bh) {
                       err = -EIO;
                       RFS_BUG("can't get buffer head\n");
                       goto out;
               }
               block_end = block_start + blocksize - 1;
               if (block_end < from) {
                       continue;
               } else if (block_start > to) { 
                       break;
               }
               clear_bit(BH_New, &bh->b_state);

               /* map new buffer if necessary*/        
               if (!buffer_mapped(bh) || (inode->i_size <= (block<<(inode->i_sb->s_blocksize_bits)))) {
                       err = get_block(inode, block, bh, 1);
                       if (err) {
                               DEBUG(DL1, "no block\n");       
                               goto out;
                       }
                       if (buffer_new(bh) && block_end > to) {
                               memset(kaddr+to+1, 0, block_end-to);
                               continue;
                       }
               }                       
               if (!buffer_uptodate(bh) && 
                       (block_start < from || block_end > to)) {
                       ll_rw_block(READ, 1, &bh);
                       wait_on_buffer(bh);
                       if (!buffer_uptodate(bh)) {
                               err = -EIO;
                               goto out;
                       }
               }
       }
out:
       flush_dcache_page(page);        
       kunmap_atomic(kaddr, KM_USER0);
       return err;     
}

/**
 *  block commit write 
 * @param inode        inode    
 * @param page page pointer
 * @param from start offset within page        
 * @param to   last offset within page  
 * @return     return 0 on success, errno on failure
 */
int rfs_block_commit_write(struct inode *inode, struct page *page,
               unsigned from, unsigned to)
{
       unsigned block_start, block_end;
       unsigned blocksize;
       struct buffer_head *bh, *head;

       blocksize = 1 << inode->i_blkbits;

       for(bh = head = page->buffers, block_start = 0;
           bh != head || !block_start;
           block_start=block_end + 1, bh = bh->b_this_page) {
               block_end = block_start + blocksize - 1;
               if (block_end < from)
                       continue;
               else if (block_start > to) 
                       break;
               else {
                       set_bit(BH_Uptodate, &bh->b_state);
                       __mark_buffer_dirty(bh);
                       down(&RFS_I(inode)->data_mutex);
                       buffer_insert_inode_data_queue(bh, inode);
                       up(&RFS_I(inode)->data_mutex);
               }
       }

       return 0;
}

/**
 * wakeup function of logging process
 * @param __data     : super block
 */
void rfs_log_wakeup(unsigned long __data)
{
       struct super_block *sb = (struct super_block *) __data;
       struct task_struct *p = RFS_SB(sb)->sleep_proc;

       mod_timer(&RFS_SB(sb)->timer, DATA_EXPIRES(jiffies));
       if (p->state == TASK_UNINTERRUPTIBLE)
               wake_up_process(p);
}

/**
 * wakeup function for a process committing data
 * @param __data     : private inode
 */
void rfs_data_wakeup(unsigned long __data)
{
       struct rfs_inode_info *rfsi = (struct rfs_inode_info *) __data;
       struct task_struct *p = rfsi->sleep_proc;

       mod_timer(&rfsi->timer, DATA_EXPIRES(jiffies));
       if (p->state == TASK_UNINTERRUPTIBLE)
               wake_up_process(p);
}

/**
 *  rfs_direct_IO - directly read/write from/to user space buffers without cache
 * @param rw           read/write type
 * @param inode                inode
 * @param iobuf                iobuf
 * @param blocknr      number of blocks
 * @param blocksize    block size
 * @return             write/read size on success, errno on failure
 */
int rfs_direct_IO(int rw, struct inode *inode, struct kiobuf *iobuf, unsigned long blocknr, int blocksize)
{
       struct super_block *sb = inode->i_sb;
       loff_t offset, old_size = inode->i_size;
       unsigned int alloc_clus = 0;
       int zerofilled = FALSE, ret;

       offset = blocknr << sb->s_blocksize_bits;

       if (rw == WRITE) {
               unsigned int clu_size, clu_bits;
               unsigned int req_clus, free_clus;

               clu_size = RFS_SB(sb)->cluster_size;
               clu_bits = RFS_SB(sb)->cluster_bits;

               /* compare the number of required clusters with free clusters */
               alloc_clus = (inode->i_size + clu_size - 1) >> clu_bits;
               req_clus = (offset + iobuf->length + clu_size - 1) >> clu_bits;
               if (req_clus > alloc_clus)
                       req_clus -= alloc_clus; /* required clusters */
               else 
                       req_clus = 0;

               if (rfs_log_start(inode->i_sb, RFS_LOG_WRITE, inode))
                       return -EIO;

               free_clus = GET_FREE_CLUS(RFS_SB(sb));
               if (req_clus > free_clus) {
                       DEBUG(DL1, "req_clus = %d free_clus = %d \n", 
                                       req_clus, free_clus);
                       ret = -ENOSPC;
                       goto end_log;
               }

               /* lseek case in direct IO */
               if (offset > old_size) {
                       /* 
                        * NOTE: In spite of direc IO, 
                        * we use page cache for extend_with_zeorfill 
                        */
                       ret = extend_with_zerofill(inode, 
                                       (u32) old_size, (u32) offset);
                       if (ret)
                               goto end_log;

                       inode->i_size = offset;
                       set_mmu_private(inode, offset);
                       zerofilled = TRUE;
               }
       }

       ret = generic_direct_IO(rw, inode, iobuf, 
                       blocknr, blocksize, rfs_get_block);

       if (rw == WRITE) {
               if (ret == -EINVAL) { 
                       int err;

                       /* 
                        * free some clusters if only unaligned offset & length 
                        * of iobuf exists which were allocated at direct IO op
                        */
                       err = dealloc_clusters(inode, alloc_clus);
                       if (!err) {
                               inode->i_size = old_size;
                               set_mmu_private(inode, old_size);
                       }

                       /* invalidate hint info */
                       rfs_invalidate_hint(inode);
               } 

end_log:
               if (rfs_log_end(inode->i_sb, (ret >= 0) ? 0 : -EIO))
                       return -EIO;

               if (!ret && zerofilled)
                       ret = fsync_inode_data_buffers(inode);
       }

       return ret;
}

/**
 *  build the super block
 * @param sb           super block
 * @param data         private data 
 * @param silent       verbose flag
 * @return             return super block pointer on success, null on failure
 *
 * initialize the super block, system file such as logfile & poolfile and recovery error by sudden power off
 */
static struct super_block *rfs_read_super(struct super_block *sb, void *data, int silent)
{
       struct super_block *res = NULL;
       unsigned int used_clusters;
       int err;

       res = rfs_common_read_super(sb, data, silent);
       if (!res) 
               return NULL;
#ifdef CONFIG_RFS_VFAT
       RFS_SB(sb)->options.isvfat = TRUE;      
#else
       if (IS_FAT32(RFS_SB(res))) {
               DPRINTK("invalid fat type\n");
               return NULL;
       }

       sb->s_root->d_op = &rfs_dentry_operations;
       RFS_SB(sb)->options.isvfat = FALSE;
#endif
       sb->s_flags |= MS_NOATIME | MS_NODIRATIME;

       if (rfs_init_pool(sb)) {
               DPRINTK("fast unlink can not be supported\n");
               return NULL;
       }

       if (rfs_log_init(sb)) {
               DPRINTK("RFS-log : Not supported\n");
               return NULL;
       }

       if (rfs_remove_candidates(sb)) {
               DPRINTK("Can not remove candidate segments in pool file\n");
               return NULL;
       }

       /* update total number of used clusters */
       err = count_used_clusters(sb, &used_clusters);
       if (err) { /* I/O error */
               DPRINTK("FAT has something wrong\n");
               return NULL;
       }

       RFS_SB(sb)->num_used_clusters = used_clusters 
               - (RFS_POOL_I(sb)->num_clusters - POOL_RESERVED_CLUSTER);

       return res;
}

static DECLARE_FSTYPE_DEV(rfs_fs_type, "rfs", rfs_read_super);

/**
 *  register RFS file system
 */
static int __init init_rfs_fs(void)
{
       return register_filesystem(&rfs_fs_type);
}

/**
 *  unregister RFS file system
 */
static void __exit exit_rfs_fs(void)
{
       unregister_filesystem(&rfs_fs_type);
}

module_init(init_rfs_fs);
module_exit(exit_rfs_fs);

MODULE_LICENSE("Samsung, Proprietary");
