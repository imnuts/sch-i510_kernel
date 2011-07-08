/**
 *  @file       fs/rfs/rfs_26.c
 *  @brief      Kernel version 2.6 specified functions
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
 */

#include <linux/fs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/uio.h>
#include <linux/writeback.h>
#include <linux/rfs_fs.h>

#include "rfs.h"
#include "log.h"


#ifdef RFS_FOR_2_6_17
/*
 * In linux 2.6.17 or more, the callback function in direct io is changed.
 * Now, it is used to single get block instead of multiple get blocks.
 */
#define rfs_get_blocks         rfs_get_block

#else  /* !RFS_FOR_2_6_17 */
/**
 *  Function to translate a logical block into physical block
 *  @param inode       inode
 *  @param iblock      logical block number
 *  @param max_blocks  dummy variable new
 *  @param bh_result   buffer head pointer
 *  @param create      control flag
 *  @return            zero on success, negative value on failure
 *
 *  This function is only invoked by direct IO
 */
static int rfs_get_blocks(struct inode *inode, sector_t iblock, unsigned long max_blocks, struct buffer_head *bh_result, int create)
{
       int ret;

       ret = rfs_get_block(inode, iblock, bh_result, create);
       if (!ret)
               bh_result->b_size = (1 << inode->i_blkbits);
       return ret;
}
#endif /* RFS_FOR_2_6_17 */

#ifndef RFS_FOR_2_6_18
/*
 * In linux 2.6.18 or more, struct writeback_control file is changed
 * from start, end to range_start, range_end
 */
#define range_start            start
#define range_end              end
#endif /* !RFS_FOR_2_6_18 */

/**
 *  Write and wait upon the last page for inode
 * @param inode        inode pointer
 * @return     zero on success, negative value on failure
 *
 * This is a data integrity operation for a combination of 
 * zerofill and direct IO write
 */
static int sync_last_page(struct inode *inode)
{
       loff_t lstart = (i_size_read(inode) - 1) & PAGE_CACHE_MASK;
       struct address_space *mapping = inode->i_mapping;
       struct writeback_control wbc = {
               .sync_mode      = WB_SYNC_ALL,
               .range_start    = lstart,
               .range_end      = lstart + PAGE_CACHE_SIZE - 1,
       };

       /*
        * Note: There's race condition. We don't use page cache operation
        * directly.
        */
       return mapping->a_ops->writepages(mapping, &wbc);
}

#define rfs_flush_cache(iov, nr_segs)  do { } while (0)

/**
 *  RFS function excuting direct I/O operation
 *  @param rw  I/O command 
 *  @param iocb        VFS kiocb pointer 
 *  @param iov VFS iovc pointer
 *  @param offset      I/O offset
 *  @param nr_segs     the number segments
 *  @return    written or read date size on sucess, negative value on failure
 */
ssize_t rfs_direct_IO(int rw, struct kiocb * iocb, const struct iovec *iov,
                               loff_t offset, unsigned long nr_segs)
{
       struct inode *inode = iocb->ki_filp->f_mapping->host;
       struct super_block *sb = inode->i_sb;
       int ret = 0;

#ifdef CONFIG_GCOV_PROFILE
/*
 * Note: We *MUST* use the correct API in direct IO.
 *      It is correct place when use gcov
 */
#define        loff_t                  off_t
#endif
       
       if (rw == WRITE) {
               unsigned int clu_size, clu_bits;
               unsigned int alloc_clus, req_clus, free_clus;
               size_t write_len = iov_length(iov, nr_segs);
               loff_t i_size = i_size_read(inode);

               clu_size = RFS_SB(sb)->cluster_size;
               clu_bits = RFS_SB(sb)->cluster_bits;

               /* compare the number of required clusters with free clusters */
               alloc_clus = (i_size + clu_size - 1) >> clu_bits;
               req_clus = (offset + write_len + clu_size - 1) >> clu_bits;
               if (req_clus > alloc_clus)
                       req_clus -= alloc_clus;
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

               /*
                * lseek case in direct IO
                * Note: We have to cast 'offset' as loff_t
                *       to correct operation in kernel gcov
                *       the loff_t means off_t in gcov mode
                */
               if ((loff_t) offset > i_size) {
                       /*
                        * NOTE: In spite of direc IO,
                        * we use page cache for extend_with_zerofill
                        */
                       ret = extend_with_zerofill(inode, 
                                       (u32) i_size,
                                       (u32) offset);
                       if (ret)
                               goto end_log;

                       i_size_write(inode, offset);
                       set_mmu_private(inode, offset);

                       ret = sync_last_page(inode);
                       if (ret)
                               /*
                                * it is possible that real allocated clusters
                                * and size in dir entry can be different
                                */
                               goto end_log;
               }
       }

       /* flush cache */
       rfs_flush_cache(iov, nr_segs);

       ret = blockdev_direct_IO(rw, iocb, inode, inode->i_sb->s_bdev, iov,
                       offset, nr_segs, rfs_get_blocks, NULL);

       if (rw == WRITE) {
end_log:
               if (rfs_log_end(inode->i_sb, (ret >= 0) ? 0 : -EIO))
                       return -EIO;
       }

       return ret;
}

#ifdef CONFIG_GCOV_PROFILE
#undef loff_t
#endif

/**
 *  Function to build super block structure
 *  @param sb          super block pointer
 *  @param data                pointer for an optional date
 *  @param silent      control flag for error message
 *  @return            zero on success, a negative error code on failure
 *
 *  Initialize the super block, system file such as logfile & poolfile and recovery error by sudden power off
 */
static int rfs_fill_super(struct super_block *sb, void *data, int silent)
{
       unsigned int used_clusters;
       int err = 0;

       sb = rfs_common_read_super(sb, data, silent);
       if (!sb)
               return -EINVAL;

#ifdef CONFIG_RFS_VFAT
       RFS_SB(sb)->options.isvfat = TRUE;      
#else
       if (IS_FAT32(RFS_SB(sb))) {
               DPRINTK("invalid fat type\n");
               return -EINVAL;
       }

       RFS_SB(sb)->options.isvfat = FALSE;

       sb->s_root->d_op = &rfs_dentry_operations;
#endif
       sb->s_flags |= MS_NOATIME | MS_NODIRATIME;

       if (rfs_init_pool(sb)) {
               DPRINTK("fast unlink can not be supported\n");
               return -EINVAL;
       }

       if (rfs_log_init(sb)) {
               DPRINTK("RFS-log : Not supported\n");
               return -EINVAL;
       }

       if (rfs_remove_candidates(sb)) {
               DPRINTK("Can not remove candidate segments in pool file\n");
               return -EIO;
       }
       
       /* update total number of used clusters */
       err = count_used_clusters(sb, &used_clusters);
       if (err) { /* I/O error */
               DPRINTK("FAT has something wrong\n");
               return err;
       }

       RFS_SB(sb)->num_used_clusters = used_clusters 
               - (RFS_POOL_I(sb)->num_clusters - POOL_RESERVED_CLUSTER);

       return err;
}

/* local variable definition */
#ifdef RFS_FOR_2_6_20
static struct kmem_cache *rfs_inode_cachep = NULL;
#else
static kmem_cache_t *rfs_inode_cachep = NULL;
#endif

/* static function definition */
/**
 * Function to initialized a newly create rfs specific inode structure
 * @param foo          memory pointer for new inode structure
 * @param cachep       a pointer for inode cache
 * @param flags                control flag
 */
#ifdef RFS_FOR_2_6_27
static void init_once(void *foo)
#elif defined RFS_FOR_2_6_24
static void init_once(struct kmem_cache *cachep, void *foo)
#elif defined RFS_FOR_2_6_20
static void init_once(void * foo, struct kmem_cache * cachep, unsigned long flags)
#else
static void init_once(void * foo, kmem_cache_t * cachep, unsigned long flags)
#endif
{
       struct rfs_inode_info *ei = (struct rfs_inode_info *) foo;

#ifndef RFS_FOR_2_6_22
       if ((flags & (SLAB_CTOR_VERIFY | SLAB_CTOR_CONSTRUCTOR)) ==
                       SLAB_CTOR_CONSTRUCTOR)
#endif
               inode_init_once(&ei->vfs_inode);
}

/**
 * Function to initialize an inode cache 
 */
static int __init rfs_init_inodecache(void)
{
       rfs_inode_cachep = kmem_cache_create("rfs_inode_cache",
                       sizeof(struct rfs_inode_info),
#ifdef RFS_FOR_2_6_23
                       0, (SLAB_RECLAIM_ACCOUNT | SLAB_MEM_SPREAD),
                       init_once);
#else
                       0, SLAB_RECLAIM_ACCOUNT,
                       init_once, NULL);
#endif

       if (!rfs_inode_cachep)
               return -ENOMEM;

       return 0;
}

/**
 * Function to destroy an inode cache 
 */
static void rfs_destroy_inodecache(void)
{
       /*
        * kmem_cache_destroy return type is changed
        * from 'int' to 'void' after 2.6.19
        */
       kmem_cache_destroy(rfs_inode_cachep);
}

/**
 *  Function to allocate rfs specific inode and associate it with vfs inode
 *  @param sb  super block pointer
 *  @return    a pointer of new inode on success, NULL on failure
 */
struct inode *rfs_alloc_inode(struct super_block *sb)
{
       struct rfs_inode_info *new;

#ifdef RFS_FOR_2_6_20
       new = kmem_cache_alloc(rfs_inode_cachep, GFP_KERNEL);
#else
       new = kmem_cache_alloc(rfs_inode_cachep, SLAB_KERNEL);
#endif
       if (!new)
               return NULL;
               
       /* initialize rfs inode info, if necessary */
       new->i_state = RFS_I_ALLOC;

       return &new->vfs_inode; 
}
/**
 * Function to deallocate rfs specific inode 
 * @param inode        inode pointer
 */
void rfs_destroy_inode(struct inode *inode)
{
       if (!inode)
               printk("inode is NULL \n");

       kmem_cache_free(rfs_inode_cachep, RFS_I(inode));
}

/**
 * Interface function for super block initialization 
 * @param fs_type      filesystem type
 * @param flags                flag
 * @param dev_name     name of file system
 * @param data         private date
 * @return     a pointer of super block on success, negative error code on failure
 */
#ifdef RFS_FOR_2_6_18
static int rfs_get_sb(struct file_system_type *fs_type,
       int flags, const char *dev_name, void *data, struct vfsmount *mnt)
{
       return get_sb_bdev(fs_type, flags, dev_name, data, rfs_fill_super, mnt);
}
#else
static struct super_block *rfs_get_sb(struct file_system_type *fs_type,
                               int flags, const char *dev_name, void *data)
{
       return get_sb_bdev(fs_type, flags, dev_name, data, rfs_fill_super);
}
#endif /* RFS_FOR_2_6_18 */

/* rfs filesystem type defintion */
static struct file_system_type rfs_fs_type = {
       .owner          = THIS_MODULE,
       .name           = "rfs",
       .get_sb         = rfs_get_sb,
       .kill_sb        = kill_block_super,
       .fs_flags       = FS_REQUIRES_DEV,
};

/**
 * Init function for rfs 
 */
static int __init init_rfs_fs(void)
{
       int err = 0;

       /* init inode cache */
       err = rfs_init_inodecache();    
       if (err)
               goto fail_init;
       
       err = register_filesystem(&rfs_fs_type);
       if (err)
               goto fail_register;

       return 0;
       
fail_register:
       rfs_destroy_inodecache();
fail_init:
       return err;
}

/**
 * Exit function for rfs 
 */
static void __exit exit_rfs_fs(void)
{
       rfs_destroy_inodecache();
       unregister_filesystem(&rfs_fs_type);
}

module_init(init_rfs_fs);
module_exit(exit_rfs_fs);

MODULE_LICENSE("Samsung, Proprietary");
