/**
 * @file       fs/rfs/log.c
 * @brief      functions for logging
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

#include <linux/bitops.h>
#include <linux/rfs_fs.h>
#include "rfs.h"
#include "log.h"

#ifdef CONFIG_RFS_FAT_DEBUG
#define CHECK_MUTEX(X, Y)                                              \
do {                                                                   \
       if ((X) != (Y)) {                                               \
               RFS_BUG("RFS-log : lock is broken %d "                  \
                       "(should be %d)\n",                             \
                       (int)(X), (int)(Y));                            \
       }                                                               \
} while (0)
#else
#define CHECK_MUTEX(...)       do { } while (0)
#endif

static int rfs_log_open(struct super_block *sb);
static int rfs_make_logfile(struct super_block *sb);
static int rfs_log_start_nolock(struct super_block *sb, unsigned int type,
               struct inode *inode);
static int pre_alloc_clusters(struct inode *inode);
static int commit_deferred_tr(struct super_block *sb, unsigned long ino);

/**
 * mark buffer dirty & register buffer to the transaction if we are inside transaction
 * @param bh   buffer 
 * @param sb   super block 
 */
void mark_buffer_tr_dirty(struct buffer_head *bh, struct super_block *sb)
{
#ifdef RFS_FOR_2_4
       struct inode *tr_inode = &(RFS_LOG_I(sb)->tr_buf_inode);

       mark_buffer_dirty(bh);

       if (RFS_LOG_I(sb) && get_log_lock_owner(sb) == current->pid) {
               /* inside transaction */
               buffer_insert_inode_queue(bh, tr_inode);
       }
#endif
#ifdef RFS_FOR_2_6
       mark_buffer_dirty(bh);
#endif
}

/**
 * mark inode dirty & write inode if we are inside transaction
 * @param inode        inode 
 */
void mark_inode_tr_dirty(struct inode *inode)
{
       struct super_block *sb = inode->i_sb;

       mark_inode_dirty(inode);
       if (RFS_LOG_I(sb) && get_log_lock_owner(sb) == current->pid) {
               /* inside transaction */
               rfs_sync_inode(inode, 0, 0);
       }
}

/**
 * commit rfs meta-data
 * @param sb super block
 */
int rfs_meta_commit(struct super_block *sb)
{
#ifdef RFS_FOR_2_4
       struct inode *tr_inode = &(RFS_LOG_I(sb)->tr_buf_inode);
#endif
       struct inode *inode = RFS_LOG_I(sb)->inode;
       int err = 0, ret = 0;

       /* fat cache sync */
       fat_lock(sb);
       rfs_fcache_sync(sb, 0);
       fat_unlock(sb);

       /* other transactions except write register dir entry */
       if (tr_pre_alloc(sb) && inode)
               ret = rfs_sync_inode(inode, 0, 0);

       /* rfs meta-data include link path */
       if (RFS_LOG_I(sb)->type == RFS_LOG_SYMLINK && 
                       (inode = RFS_LOG_I(sb)->symlink_inode)) {
               err = rfs_sync_inode(inode, 1, 0);
               if (err && !ret)
                       ret = err;
       }


#ifdef RFS_FOR_2_6
       err = sync_blockdev(sb->s_bdev);
#else
       err = fsync_inode_buffers(tr_inode);
#endif
       if (err && !ret)
               ret = err;

       return ret;
}

#ifdef _RFS_INTERNAL_UNUSED_LOG
/**
 * commit rfs data
 * @param sb super block
 * @param wait write and wait for inode?
 */
int rfs_data_commit(struct inode *inode, int wait)
{
       int ret;
       ret = rfs_sync_inode(inode, 1, wait);

       return ret;
}
#endif

/**
 * Does transaction deferredly commit?
 * @param sb super block
 * @return if transaction is feasible for deferred commit, then return TRUE,
 *     otherwise return FALSE.
 */
int tr_deferred_commit(struct super_block *sb)
{
       if (RFS_LOG_I(sb)->type == RFS_LOG_WRITE)
               return TRUE;

       return FALSE;
}

/**
 * Does transaction need pre allocation?
 * @param sb super block
 * @return if transaction is feasible for pre-allocation, then return TRUE,
 *     otherwise return FALSE.
 */
int tr_pre_alloc(struct super_block *sb)
{
       if (!RFS_LOG_I(sb)) {
               /* special case : creation of pool file */
               return FALSE;
       }

       /* must include all tr_deferred_commit() */
       if (RFS_LOG_I(sb)->type == RFS_LOG_WRITE ||
                       RFS_LOG_I(sb)->type == RFS_LOG_TRUNCATE_F)
               return TRUE;

       return FALSE;
}

/*****************************************************************************/
/* log init/exit functions                                                   */
/*****************************************************************************/
/**
 * init logfile
 * @param sb super block
 * @return 0 on success, errno on failure
 */
static int init_logfile(struct super_block *sb)
{
       unsigned int i;

       for (i = 0; i < RFS_LOG_MAX_COUNT; i++) {
               if (rfs_log_read(sb, i))
                       return -EIO;

               memset(RFS_LOG(sb), (unsigned char) RFS_LOG_NONE, SECTOR_SIZE);
               mark_buffer_dirty(RFS_LOG_I(sb)->bh);
       }

       brelse(RFS_LOG_I(sb)->bh);
       RFS_LOG_I(sb)->bh = NULL;
       RFS_LOG_I(sb)->log = NULL;

       rfs_sync_vol(sb);

       return 0;
}

/**
 * initialize log
 * @param sb super block
 * @return 0 on success, errno on failure
 */
int rfs_log_init(struct super_block *sb)
{
       int ret;

       if (sizeof(struct rfs_trans_log) > SECTOR_SIZE) {
               DPRINTK("RFS-log : Current log record is %u bytes."
                      "Log record must be 512 bytes at most\n",
                      sizeof(struct rfs_trans_log));
               return -EINVAL;
       }

       if ((ret = rfs_log_open(sb)) == -ENOENT) { /* there is no logfile */
               if ((ret = rfs_make_logfile(sb))) {
                       DPRINTK("RFS-log(%d) : Couldn't make log file."
                              " RFS cannot mount without logfile\n", ret);
                       if (ret == -ENOSPC) {
                               printk(KERN_WARNING "Please check there is"
                                       " enough space to create logfile"
                                       " (MIN : %u bytes)\n",
                                       RFS_LOG_MAX_COUNT * SECTOR_SIZE);
                       }
                       return ret;
               }

               /* open again */
               if ((ret = rfs_log_open(sb))) {
                       /* I/O error */
                       DPRINTK("RFS-log(%d) : Although it was made"
                                       " can't open log file\n", ret);
                       return -EIO;
               }

               /* initialize log file to prevent confusing garbage value */
               if ((ret = init_logfile(sb))) {
                       /* I/O error */
                       DPRINTK("RFS-log(%d) : Cannot init logfile\n", ret);
                       return -EIO;
               }
       } else if (ret) {
               /* I/O error */
               DPRINTK("RFS-log(%d) : Can't open log file\n", ret);
               return -EIO;
       } else { /* success in openning log */
               ret = rfs_log_replay(sb);
               if (ret) {
                       /* I/O error */
                       DPRINTK("RFS-log(%d) : Fail to replay log\n", ret);
                       return -EIO;
               }
       }

       return 0;
}

/**
 * clean up log
 * @param sb super block
 * cleanup log info at normal umount
 */
void rfs_log_cleanup(struct super_block *sb)
{
       if (!RFS_LOG_I(sb))
               return;

       commit_deferred_tr(sb, 0);
       brelse(RFS_LOG_I(sb)->bh);
       kfree(RFS_LOG_I(sb)->log_mutex);
       kfree(RFS_LOG_I(sb));
       RFS_SB(sb)->log_info = NULL;
}

/*****************************************************************************/
/* log open function                                                         */
/*****************************************************************************/
/**
 * open logfile
 * @param sb super block
 * @return 0 on success, errno on failure
 */
static int rfs_log_open(struct super_block *sb)
{
       struct inode *root_dir;
       struct inode *inode;
       struct dentry *root_dentry;
       struct rfs_dir_entry *ep;
       struct buffer_head *bh = NULL;
       struct rfs_log_info *log_info;
       unsigned int i;
       unsigned int iblock;
       int index;
       int ret = 0;

       root_dentry = sb->s_root;
       root_dir = root_dentry->d_inode;

       index = find_entry(root_dir, RFS_LOG_FILE_NAME, &bh, TYPE_FILE);
       if (index < 0) {
               DEBUG(DL0, "can't find log (%d)", index);
               ret = -ENOENT;
               goto rel_bh;
       }

       ep = get_entry(root_dir, index, &bh);
       if (IS_ERR(ep)) {
               /* I/O error */
               DEBUG(DL0, "can't get entry in root_dir");
               ret = PTR_ERR(ep);
               goto rel_bh;
       }

       /* get a new inode */
       if (!(inode = new_inode(root_dir->i_sb))) {
               /* memory error */
               DEBUG(DL0, "Can not alloc inode for logfile");
               ret = -ENOMEM;
               goto rel_bh;
       }

       /* fill inode info */
       ret = fill_inode(inode, ep, RFS_SB(sb)->root_clu, index);
       if (ret)
               goto rel_bh;

       log_info = kmalloc(sizeof(struct rfs_log_info), GFP_KERNEL);
       if (!log_info) {
               /* memory error */
               DEBUG(DL0, "memory allocation error");
               ret = -ENOMEM;
               goto out;
       }

       /* log size is 9th power of 2 (512B) */
       log_info->secs_per_blk = sb->s_blocksize >> 9;
       log_info->secs_per_blk_bits =
               (unsigned int) ffs(log_info->secs_per_blk) - 1;

       DEBUG(DL3, "sectors per block : %u its bit : %u\n",
                       log_info->secs_per_blk, log_info->secs_per_blk_bits);

       for (i = 0; i < RFS_LOG_MAX_COUNT; i++) {
               iblock = i >> log_info->secs_per_blk_bits;
               ret = rfs_bmap(inode, iblock, &(log_info->blocks[i]));
               if (ret) {
                       kfree(log_info);
                       goto out;
               }
       }
       log_info->type = RFS_LOG_NONE;
       log_info->sequence = 1;
       log_info->bh = NULL;
       log_info->inode = NULL;
       log_info->isec = 0;

       /* init fields for deferred commit */
       log_info->alloc_index = 0;
       log_info->numof_pre_alloc = 0;

       log_info->start_cluster = RFS_I(inode)->start_clu;
       
       log_info->log_mutex = kmalloc(sizeof(struct rfs_semaphore), GFP_KERNEL);
       if (!(log_info->log_mutex)) {
               /* memory error */
               DEBUG(DL0, "memory allocation failed");
               kfree(log_info);
               ret = -ENOMEM;
               goto out;
       }

#ifdef RFS_FOR_2_4
       INIT_LIST_HEAD(&(log_info->tr_buf_inode.i_dirty_buffers));
#endif
       RFS_SB(sb)->log_info = (void *) log_info;

       init_log_lock(sb);
out:
       iput(inode);
rel_bh:
       brelse(bh);
       return ret;
}

/*****************************************************************************/
/* logging functions                                                         */
/*****************************************************************************/
/**
 * force to commit write transaction
 * @param sb super block
 * @param inode inode for target file or NULL for volume sync
 * @return 0 on success, errno on failure
 * @pre It can be called in middle of transaction, so 
 *     it should get a lock for committing transaction
 *
 * Never sleep holding super lock. Log lock can protects RFS key data
 */
int rfs_log_force_commit(struct super_block *sb, struct inode *inode)
{
       int ret;

       if (inode && inode != RFS_LOG_I(sb)->inode)
               return 0;

       lock_log(sb);

       if (!inode) {
               unlock_super(sb);
               ret = commit_deferred_tr(sb, 0);
               unlock_log(sb);
               lock_super(sb);
       } else {
               ret = commit_deferred_tr(sb, inode->i_ino);
               unlock_log(sb);
       }

       return ret;
}

/**
 * release remaining pre-allocations
 * @param sb super block
 * @return 0 on success, errno on failure
 */
static int release_pre_alloc(struct super_block *sb)
{
       unsigned int i;
       int ret = 0;

       if (RFS_LOG_I(sb)->alloc_index >= RFS_LOG_I(sb)->numof_pre_alloc)
               goto out;

       fat_lock(sb);
       if (RFS_LOG_I(sb)->where == RFS_POOL) {
#ifdef _RFS_INTERNAL_SANITY_CHECK
               struct rfs_log_info *rli = RFS_LOG_I(sb);
               unsigned int value;

               for (i = rli->alloc_index;
                               i < rli->numof_pre_alloc - 1; i++) {
                       ret = fat_read(sb, rli->pre_alloc_clus[i],
                                       &value);
                       if (ret) {
                               printk("can not read fat table\n");
                               BUG();
                       }

                       if (value != rli->pre_alloc_clus[i + 1]) {
                               printk("free cluster from fat is not zero\n");
                               BUG();
                       }
               }
#endif

               for (i = RFS_LOG_I(sb)->alloc_index;
                    i < RFS_LOG_I(sb)->numof_pre_alloc; i++) {
                       ret = fat_write(sb, RFS_LOG_I(sb)->pre_alloc_clus[i],
                                       CLU_FREE);
                       if (ret) 
                               break;

                       DEBUG(DL3, "chained cluster %u freed",
                               RFS_LOG_I(sb)->pre_alloc_clus[i]);
               }
       } else { /* RFS_LOG_I(sb)->where == RFS_FAT_TABLE */
               /* pre-allocated clusters had CLU_FREE */
#ifdef _RFS_INTERNAL_SANITY_CHECK
               struct rfs_log_info *rli = RFS_LOG_I(sb);
               unsigned int value;

               for (i = rli->alloc_index;
                               i < rli->numof_pre_alloc; i++) {
                       ret = fat_read(sb, rli->pre_alloc_clus[i],
                                       &value);
                       if (ret) {
                               printk("can not read fat table\n");
                               BUG();
                       }
                       if (value != CLU_FREE) {
                               printk("free cluster from fat is not zero\n");
                               BUG();
                       }
               }
#endif
               i = RFS_LOG_I(sb)->numof_pre_alloc;
               RFS_SB(sb)->search_ptr = RFS_LOG_I(sb)->pre_alloc_clus[
                       RFS_LOG_I(sb)->alloc_index];
       }

       if (i != RFS_LOG_I(sb)->numof_pre_alloc)
               DPRINTK("RFS-log : critical error!! FAT chain is crashed\n");

       fat_unlock(sb);
out:
       RFS_LOG_I(sb)->alloc_index = RFS_LOG_I(sb)->numof_pre_alloc = 0;
       return ret;
}

/**
 * commit deferred transaction for pre-allocation
 * @param sb super block
 * @param ino inode number
 * @return 0 on success, errno on failure
 */
static int commit_deferred_tr(struct super_block *sb, unsigned long ino)
{
       int ret = 0;

       if (tr_deferred_commit(sb) && 
          (!ino || (RFS_LOG_I(sb)->inode && (RFS_LOG_I(sb)->inode->i_ino == ino)))) {
               ret = release_pre_alloc(sb);
               if (ret) /* I/O error */
                       return ret;

               ret = rfs_meta_commit(sb);
               if (ret) 
                       return ret;

               DEBUG(DL3, "where pre clusters are : %d", RFS_LOG_I(sb)->where);
               if (rfs_log_mark_end(sb, RFS_SUBLOG_COMMIT)) {
                       /* I/O error */
                       DPRINTK("RFS-log : Couldn't commit write"
                                      " transaction\n");
                       return -EIO;
               }
               sb->s_dirt = 0;
       }

       return ret;
}

/**
 * get a lock and mark start of transaction
 * @param sb super block
 * @param log_type log type
 * @param inode relative inode
 * @return 0 on success, errno on failure
 * @post locking log has been remained until trasaction ends
 *
 * pre-allocation is commited if there is no further write to the same file
 *     as that of previous transaction
 */
int rfs_log_start(struct super_block *sb, unsigned int log_type,
               struct inode *inode)
{
       int ret;

       /*
        * Exception handling : Inode for root can be clear
        *      if mounting rfs is failed during initialization of log,
        *      and then rfs_delete_inode is called.
        *      So, logging for delete_inode must handle this exceptional case.
        */      
       if (!RFS_LOG_I(sb)) {
               if (log_type == RFS_LOG_DEL_INODE) {
                       return 0;
               } else {
                       RFS_BUG("RFS-log : System crashed\n");
                       return -EIO;
               }
       }

       /* check nested transaction */
       /* Any process cannot set owner to own pid without a mutex */
       if (lock_log(sb) >= 2) {
               /* recursive acquisition */
               return 0;
       }

       if (tr_deferred_commit(sb)) {
               if ((log_type == RFS_LOG_I(sb)->type) &&
                   (RFS_LOG_I(sb)->inode == inode)) {
                       /* write data in the same file */
                       return 0;
               }
               /* different transaction; flush prev write-transaction */
               DEBUG(DL2, "prev : write, cur : %d, ino : %lu", log_type, inode->i_ino);
               ret = commit_deferred_tr(sb, RFS_LOG_I(sb)->inode->i_ino);
               if (ret) /* I/O error */
                       goto err;
       }


       if (rfs_log_start_nolock(sb, log_type, inode)) {
               DPRINTK("RFS-log : Couldn't start log\n");
               ret = -EIO;
               goto err;
       }

       return 0;
err:
       unlock_log(sb);

       return ret;
}

/**
 * mark end of transaction and release lock
 * @param sb super block
 * @param result result of transaction (0 : success, others : failure)
 * @return 0 on success, errno on failure
 * @pre trasaction must have started
 *
 * mark EOT and flush it unless transaction is for pre-allocation.
 */
int rfs_log_end(struct super_block *sb, int result)
{
       unsigned int sub_type = RFS_SUBLOG_COMMIT;
       int ret = 0;

       /*
        * Exception handling : Filtered all faulty transactions which
        *      start without log info except for delete_inode, which
        *      must be handled.
        *      Please reference a comment in rfs_log_start().
        */      
       if (!RFS_LOG_I(sb))
               return 0;

       /* recursive lock */
       /* Any process cannot increase ref_count without a mutex */
       if (get_log_lock_depth(sb) > 1) {
               unlock_log(sb);
               DEBUG(DL2, "release self recursive lock (ref: %d)",
                       get_log_lock_depth(sb));
               return 0;
       }

       /* transaction did not change any meta data */
       if (is_empty_tr(sb)) {
               DEBUG(DL3, "empty transaction");

               brelse(RFS_LOG_I(sb)->bh);
               RFS_LOG_I(sb)->bh = NULL;
               RFS_LOG_I(sb)->log = NULL;
               RFS_LOG_I(sb)->type = RFS_LOG_NONE;

               goto rel_lock;
       }

       DEBUG(DL2, "result : %d, type : %d", result, RFS_LOG_I(sb)->type);
       if (result)
               sub_type = RFS_SUBLOG_ABORT;

       if (tr_deferred_commit(sb) && (sub_type == RFS_SUBLOG_COMMIT)) {
               DEBUG(DL2, "deferred commit");
               ret = 0;
               sb->s_dirt = 1;
               goto rel_lock;
       }

       if (tr_pre_alloc(sb)) {
               ret = release_pre_alloc(sb);
               if (ret) /* I/O error */
                       goto rel_lock;
       }

       ret = rfs_meta_commit(sb);
       if (ret)
               goto rel_lock;

       ret = rfs_log_mark_end(sb, sub_type);

rel_lock:
       CHECK_MUTEX(get_log_lock_depth(sb), 1);

       unlock_log(sb);

       return ret;
}

/**
 * logging for entry to be built
 * @param sb super block
 * @param pdir the start cluster of parent
 * @param entry        entry index
 * @param numof_entries        the number of entries
 * @return 0 on success, errno on failure
 * @pre trasaction must have been started
 */
int rfs_log_build_entry(struct super_block *sb, unsigned int pdir,
               unsigned int entry, unsigned int numof_entries)
{
       if (!RFS_LOG_I(sb)) {
               /* special case : pool file is created at format */
               return 0;
       }

       if (rfs_log_read(sb, RFS_LOG_I(sb)->isec))
               return -EIO;

       SET32(RFS_LOG(sb)->type, MK_LOG_TYPE(RFS_LOG_I(sb)->type,
                               RFS_SUBLOG_BUILD_ENTRY)); 
       SET32(RFS_LOG(sb)->log_entry.pdir, pdir);
       SET32(RFS_LOG(sb)->log_entry.entry, entry);
       SET32(RFS_LOG(sb)->log_entry.numof_entries, numof_entries);

       DEBUG(DL2, "pdir : %u, entry : %u, numof_entries : %u",
                       pdir, entry, numof_entries);

       if (rfs_log_write(RFS_LOG_I(sb)))
               return -EIO;

       return 0;
}

/**
 * logging for entry to be removed
 * @param sb super block
 * @param pdir the start cluster of parent
 * @param entry        entry index
 * @param numof_entries        the number of entries
 * @param undel_buf the original value at position of deletion mark
 * @return 0 on success, errno on failure
 * @pre trasaction must have been started
 */
int rfs_log_remove_entry(struct super_block *sb, unsigned int pdir,
               unsigned int entry, unsigned int numof_entries,
               unsigned char *undel_buf)
{
       unsigned int i;

       if (rfs_log_read(sb, RFS_LOG_I(sb)->isec))
               return -EIO;

       SET32(RFS_LOG(sb)->type, MK_LOG_TYPE(RFS_LOG_I(sb)->type,
                               RFS_SUBLOG_REMOVE_ENTRY)); 
       SET32(RFS_LOG(sb)->log_entry.pdir, pdir);
       SET32(RFS_LOG(sb)->log_entry.entry, entry);
       SET32(RFS_LOG(sb)->log_entry.numof_entries, numof_entries);
       for (i = 0; i < numof_entries; i++)
               RFS_LOG(sb)->log_entry.undel_buf[i] = undel_buf[i];

       DEBUG(DL2, "pdir : %u, entry : %u, numof_entries : %u",
                       pdir, entry, numof_entries);

       if (rfs_log_write(RFS_LOG_I(sb)))
               return -EIO;

       return 0;
}

#ifdef _RFS_INTERNAL_UNUSED_LOG
/**
 * logging for entry to be updated 
 * @param sb super block
 * @param pdir the start cluster of parent
 * @param entry entry index
 * @param from_size the original size used to undo
 * @param to_size the changed size used to redo
 * @return 0 on success, errno on failure
 * @pre trasaction must have been started
 */
int rfs_log_update_entry(struct super_block *sb, unsigned int pdir,
               unsigned int entry, unsigned int from_size,
               unsigned int to_size)
{
       if (rfs_log_read(sb, RFS_LOG_I(sb)->isec))
               return -EIO;

       SET32(RFS_LOG(sb)->type, MK_LOG_TYPE(RFS_LOG_I(sb)->type,
                               RFS_SUBLOG_UPDATE_ENTRY)); 
       SET32(RFS_LOG(sb)->log_entry.pdir, pdir);
       SET32(RFS_LOG(sb)->log_entry.entry, entry);
       SET32(RFS_LOG(sb)->log_entry.from_size, from_size);
       SET32(RFS_LOG(sb)->log_entry.to_size, to_size);

       DEBUG(DL2, "pdir : %u, entry : %u, from : %u, to : %u",
                       pdir, entry, from_size, to_size);

       if (rfs_log_write(RFS_LOG_I(sb)))
               return -EIO;

       return 0;
}
#endif

/**
 * logging for updation of chain
 * @param sb super block
 * @param subtype sub transaction
 * @param pdir the start cluster of parent
 * @param entry entry index
 * @param pool_next next cluster in pool related to chain 
 * @param pool_prev previous cluster in pool related to chain 
 * @param pool_next next cluster in pool related to chain 
 * @param target_prev previous cluster in file or pool related to chain
 * @param target_next next cluster in file or pool related to chain
 * @param numof_clus the number of clusters in chain
 * @param clus the clusters in chain
 */
static void rfs_log_update_chain(struct super_block *sb, unsigned int subtype,
               unsigned int pdir, unsigned int entry,
               unsigned int pool_prev, unsigned int pool_next,
               unsigned int target_prev, unsigned int target_next,
               unsigned int numof_clus, unsigned int *clus)
{
       unsigned int i;

       SET32(RFS_LOG(sb)->type, MK_LOG_TYPE(RFS_LOG_I(sb)->type, subtype));
       DEBUG(DL2, "SUBTYPE : %u", subtype);

       SET32(RFS_LOG(sb)->log_fat.pdir, pdir);
       SET32(RFS_LOG(sb)->log_fat.entry, entry);

       SET32(RFS_LOG(sb)->log_fat.p_last_clu, RFS_POOL_I(sb)->last_cluster);
       SET32(RFS_LOG(sb)->log_fat.p_num_clus, RFS_POOL_I(sb)->num_clusters);
       SET32(RFS_LOG(sb)->log_fat.p_c_start_clu,
                       RFS_POOL_I(sb)->c_start_cluster);
       SET32(RFS_LOG(sb)->log_fat.p_c_last_clu,
                       RFS_POOL_I(sb)->c_last_cluster);
       DEBUG(DL2, "POOL_INFO : <%u, %u>, <%u, %u>",
                       RFS_POOL_I(sb)->last_cluster,
                       RFS_POOL_I(sb)->num_clusters,
                       RFS_POOL_I(sb)->c_start_cluster,
                       RFS_POOL_I(sb)->c_last_cluster);

       SET32(RFS_LOG(sb)->log_fat.p_prev_clu, pool_prev);
       SET32(RFS_LOG(sb)->log_fat.p_next_clu, pool_next);
       DEBUG(DL2, "POOL_CHAIN : <%u, %u>", pool_prev, pool_next);

       SET32(RFS_LOG(sb)->log_fat.t_prev_clu, target_prev);
       SET32(RFS_LOG(sb)->log_fat.t_next_clu, target_next);
       DEBUG(DL2, "TARGET_CHAIN : <%u, %u>", target_prev, target_next);

       SET32(RFS_LOG(sb)->log_fat.numof_clus, numof_clus);
       for (i = 0; i < numof_clus; i++) {
               DEBUG(DL3, "%u, ", clus[i]);
               SET32(RFS_LOG(sb)->log_fat.clus[i], clus[i]);
       }

       DEBUG(DL2, "NUMOF_LOG_CLU : <%u>", numof_clus);
}

/**
 * logging alloc chain from fat table or pool
 * @param sb super block
 * @param pdir the start cluster of parent
 * @param entry entry index
 * @param pool_prev previous cluster of detached chain in pool
 * @param pool_next next cluster of detached chain in pool
 * @param target_prev previous cluster of extented chain in file
 * @param target_next next cluster of extented chain in file
 * @param numof_clus the number of clusters in chain
 * @param clus the clusters in chain
 * @return 0 on success, errno on failure
 */
int rfs_log_alloc_chain(struct super_block *sb,
               unsigned int pdir, unsigned int entry,
               unsigned int pool_prev, unsigned int pool_next,
               unsigned int target_prev, unsigned int target_next,
               unsigned int numof_clus, unsigned int *clus)
{
       unsigned int subtype;
       int ret;

       if (!RFS_LOG_I(sb)) {
               /* special case : poolfile is created
                  before logfile at format */
               return 0;
       }

       if ((!is_empty_tr(sb)) && tr_pre_alloc(sb)) {
               struct inode *inode = RFS_LOG_I(sb)->inode;
               unsigned int type = RFS_LOG_I(sb)->type;

               /* commit pre-allocation windows */
               ret = rfs_meta_commit(sb);
               if (ret)
                       return ret;

               if (rfs_log_mark_end(sb, RFS_SUBLOG_COMMIT))
                       return -EIO;

               if (rfs_log_start_nolock(sb, type, inode))
                       return -EIO;
       }

       if (rfs_log_read(sb, RFS_LOG_I(sb)->isec))
               return -EIO;

       subtype = (pool_prev == CLU_TAIL) ? RFS_SUBLOG_ALLOC_CHAIN :
                                           RFS_SUBLOG_GET_CHAIN;

       rfs_log_update_chain(sb, subtype, pdir, entry, pool_prev, pool_next,
               target_prev, target_next, numof_clus, clus);

       if (rfs_log_write(RFS_LOG_I(sb)))
               return -EIO;

       return 0;
}

#ifdef _RFS_INTERNAL_UNUSED_LOG
/**
 * logging dealloc chain to fat table or pool
 * @param sb super block
 * @param pdir the start cluster of parent
 * @param entry entry index
 * @param pool_prev previous cluster of chain to be attached in pool
 * @param pool_next next cluster of chain to be attached  in pool
 * @param target_prev previous cluster of free-chain in file
 * @param target_next next cluster of free-chain in file
 * @param numof_clus the number of clusters in chain
 * @param clus the clusters in chain
 * @return 0 on success, errno on failure
 */
int rfs_log_free_chain(struct super_block *sb,
               unsigned int pdir, unsigned int entry,
               unsigned int pool_prev, unsigned int pool_next,
               unsigned int target_prev, unsigned int target_next,
               unsigned int numof_clus, unsigned int *clus)
{
       unsigned int subtype;

       if (rfs_log_read(sb, RFS_LOG_I(sb)->isec))
               return -EIO;

       subtype = (pool_prev == CLU_TAIL) ? RFS_SUBLOG_FREE_CHAIN :
                                           RFS_SUBLOG_PUT_CHAIN;

       rfs_log_update_chain(sb, subtype, pdir, entry, pool_prev, pool_next,
               target_prev, target_next, numof_clus, clus);

       if (rfs_log_write(RFS_LOG_I(sb)))
               return -EIO;

       return 0;
}
#endif

/**
 * logging move candidate segment to deleted partition in pool
 * @param sb super block
 * @param pdir the start cluster of parent
 * @param entry entry index
 * @param pool_prev previous cluster of chain to be moved in pool
 * @param pool_next next cluster of chain to be moved in pool
 * @param target_prev previous cluster of segment(chain)
 * @param target_next next cluster of segment(chain)
 * @param numof_clus the number of clusters in chain
 * @param clus the clusters in chain
 * @return 0 on success, errno on failure
 */
int rfs_log_move_chain(struct super_block *sb,
               unsigned int pdir, unsigned int entry,
               unsigned int pool_prev, unsigned int pool_next,
               unsigned int target_prev, unsigned int target_next,
               unsigned int numof_clus, unsigned int *clus)
{
       if (rfs_log_read(sb, RFS_LOG_I(sb)->isec))
               return -EIO;

       rfs_log_update_chain(sb, RFS_SUBLOG_MOVE_CHAIN, pdir, entry,
               pool_prev, pool_next, target_prev, target_next,
               numof_clus, clus);

       if (rfs_log_write(RFS_LOG_I(sb)))
               return -EIO;

       return 0;
}

/**
 * logging updation of pool file
 * @param sb super block
 * @return 0 on success, errno on failure
 */
int rfs_log_update_pool(struct super_block *sb)
{
       if (rfs_log_read(sb, RFS_LOG_I(sb)->isec))
               return -EIO;

       rfs_log_update_chain(sb, RFS_SUBLOG_UPDATE_POOL, CLU_TAIL, -1,
               CLU_TAIL, CLU_TAIL, CLU_TAIL, CLU_TAIL, 0, NULL);

       if (rfs_log_write(RFS_LOG_I(sb)))
               return -EIO;

       return 0;
}

/**
 * logging start of transaction without a lock
 * @param sb super block
 * @param type log type
 * @param inode modified inode
 * @return 0 on success, errno on failure
 */
static int rfs_log_start_nolock(struct super_block *sb, unsigned int type,
               struct inode *inode)
{
       extern int sanity_check_pool(struct super_block *);

#ifdef _RFS_INTERNAL_SANITY_CHECK
       extern int sanity_check_pool(struct super_block *sb);

       sanity_check_pool(sb);
#endif

#ifdef _RFS_INTERNAL_SANITY_CHECK
       sanity_check_pool(sb);
#endif

       RFS_LOG_I(sb)->type = type;
       RFS_LOG_I(sb)->inode = inode;
       RFS_LOG_I(sb)->dirty = FALSE;

       return 0;
}

/**
 * mark an end of transaction
 * @param sb super block
 * @param sub_type commit or abort     
 * @return 0 on success, errno on failure
 */
int rfs_log_mark_end(struct super_block *sb, unsigned int sub_type)
{
       if (rfs_log_read(sb, RFS_LOG_I(sb)->isec))
               return -EIO;

       set_bit(BH_RFS_LOG_COMMIT, &(RFS_LOG_I(sb)->bh->b_state));

       SET32(RFS_LOG(sb)->type, MK_LOG_TYPE(RFS_LOG_I(sb)->type, sub_type));
       if (rfs_log_write(RFS_LOG_I(sb)))
               return -EIO;
               
       RFS_LOG_I(sb)->inode = NULL;
       RFS_LOG_I(sb)->type = RFS_LOG_NONE;

       /* destroy stl mapping */
       if (IS_XSR(sb->s_dev))
               rfs_map_destroy(sb);

       return 0;
}

/*****************************************************************************/
/* log read / write functions                                                */
/*****************************************************************************/
/**
 * read log
 * @param sb super block
 * @param isec logical sector to read
 * @return 0 on success, errno on failure
 */
int rfs_log_read(struct super_block *sb, unsigned int isec)
{
       struct buffer_head *bh;
       int state = BH_RFS_LOG;
       int sec_off;

       if (RFS_LOG_I(sb)->bh)
               brelse(RFS_LOG_I(sb)->bh);

       /* rotational logfile */
       if (isec >= RFS_LOG_MAX_COUNT) {
               RFS_BUG("RFS-log : can't read log record\n");
               return -EIO;
       }

       DEBUG(DL3, "read log [%dth : 0x%x]\n", isec, (unsigned int) RFS_LOG_I(sb)->sequence);

       if (is_empty_tr(sb))
               state = BH_RFS_LOG_START;

       bh = rfs_bread(sb, RFS_LOG_I(sb)->blocks[isec], state); 
       if (!bh) {
               RFS_BUG("RFS-log : Couldn't read log\n");
               return -EIO;
       }

       RFS_LOG_I(sb)->bh = bh;

       /* point next log record */
       if (isec == RFS_LOG_MAX_COUNT - 1)
               RFS_LOG_I(sb)->isec = 0;
       else
               RFS_LOG_I(sb)->isec = isec + 1;

       sec_off = isec & (RFS_LOG_I(sb)->secs_per_blk - 1);
       RFS_LOG_I(sb)->log = (struct rfs_trans_log *) ((bh->b_data) +
                       (sec_off << SECTOR_BITS));

       return 0;
}

/**
 * write log to logfile and release it
 * @param log_info rfs's log structure 
 * @return 0 on success, errno on failure
 */
int rfs_log_write(struct rfs_log_info *log_info)
{
       int ret = 0;

       if (!log_info->bh) {
               RFS_BUG("RFS-log : No buffer head for log\n");
               return -EIO;
       }

       SET64(log_info->log->sequence, (log_info->sequence)++);
       mark_buffer_dirty(log_info->bh);
#ifdef RFS_FOR_2_6
       ret = sync_dirty_buffer(log_info->bh);
#else
       ll_rw_block(WRITE, 1, &(log_info->bh));
       wait_on_buffer(log_info->bh);
#endif

       brelse(log_info->bh);
       log_info->bh = NULL;
       log_info->log = NULL;
       log_info->dirty = TRUE;

       return ret;
}

/*****************************************************************************/
/* misc.                                                                     */
/*****************************************************************************/
/**
 * make logfile
 * @param sb           super block
 * @return 0 on success, errno on failure
 */
static int rfs_make_logfile(struct super_block *sb)
{
       struct inode tmp_inode;
       struct inode *root_dir;
       struct dentry *root_dentry;
       struct rfs_dir_entry *ep;
       struct buffer_head *bh = NULL;
       unsigned int file_size;
       unsigned int numof_clus;
       unsigned int *clus;
       unsigned int clu = 0;
       unsigned int i;
       unsigned int index;
       int ret;

       root_dentry = sb->s_root;
       root_dir = root_dentry->d_inode;

       /* scan enough space to create log file */
       file_size = RFS_LOG_MAX_COUNT * SECTOR_SIZE;
       numof_clus = file_size >> RFS_SB(sb)->cluster_bits;
       if (file_size & (RFS_SB(sb)->cluster_size - 1))
               numof_clus++;

       DEBUG(DL3, "file_size : %u, numof_clus : %u\n", file_size, numof_clus);
       DEBUG(DL3, "sector_size : %lu, cluster_size : %u", sb->s_blocksize,
                       RFS_SB(sb)->cluster_size);

       clus = kmalloc(numof_clus * sizeof(unsigned int), GFP_KERNEL);
       if (!clus)
               return -ENOMEM;

       for (i = 0; i < numof_clus; i++) {
               ret = find_free_cluster(root_dir, &clu);
               if (ret) {
                       kfree(clus);
                       return -EACCES;
               }
               clus[i] = clu;
       }

       if (i < numof_clus) { /* insufficient space */
               kfree(clus);
               return -ENOSPC;
       }

       ret = fat_write(sb, clus[0], CLU_TAIL);
       if (ret) {
               kfree(clus);
               return ret;
       }

       index = build_entry(root_dir, NULL, clus[0], TYPE_FILE,
                       RFS_LOG_FILE_NAME);
       if ((int) index < 0) {
               kfree(clus);
               return index;
       }

       ep = get_entry(root_dir, index, &bh);
       if (IS_ERR(ep)) {
               kfree(clus);
               brelse(bh);
               return PTR_ERR(ep);
       }

       /* set tmp_inode for log file */
       tmp_inode.i_sb = sb;
       RFS_I(&tmp_inode)->start_clu = clus[0];

       /* allocate enough clusters */
       for (i = 1; i < numof_clus; i++) {
               ret = append_new_cluster(&tmp_inode, clus[i - 1], clus[i]);
               if (ret) {
                       kfree(clus);
                       brelse(bh);
                       return ret;
               }
       }

       ep->attr |= (ATTR_READONLY | ATTR_SYSTEM | ATTR_HIDDEN);
       SET32(ep->size, file_size);
       mark_buffer_dirty(bh);
       brelse(bh);

       kfree(clus);
       return 0;
}

/**
 * return a pre-allocated cluster
 * @param inode        inode of file to be extended
 * @param new_clu out-var to save free cluster number
 * @return 0 on success, errno on failure
 */
int rfs_log_get_cluster(struct inode *inode, unsigned int *new_clu)
{
       struct super_block *sb = inode->i_sb;
       int ret;

       /* set default invalid value */
       *new_clu = CLU_TAIL;

       if (RFS_LOG_I(sb)->alloc_index >= RFS_LOG_I(sb)->numof_pre_alloc) {
               ret = pre_alloc_clusters(inode);
               if (ret)
                       return ret;
       }

       if (likely(RFS_LOG_I(sb)->alloc_index <
                               RFS_LOG_I(sb)->numof_pre_alloc)) {
               *new_clu = RFS_LOG_I(sb)->pre_alloc_clus[
                       (RFS_LOG_I(sb)->alloc_index)++];
       } else {
               RFS_BUG("RFS-log : pre-allocation corruption\n");
               return -EIO;
       }

       DEBUG(DL3, "alloc_cluster : %u", *new_clu);
       return 0;
}

/**
 * do pre-allocation
 * @param inode inode of file to be written
 * @return 0 on success, errno on failure
 *
 * pre-allocate clusters and save them in log buffer
 */
static int pre_alloc_clusters(struct inode *inode)
{
       struct super_block *sb = inode->i_sb;
       struct rfs_sb_info *sbi = RFS_SB(sb);
       unsigned int count = 0;
       unsigned int p_prev_clu, p_next_clu;
       unsigned int t_next_clu = NOT_ASSIGNED;
       unsigned int content = NOT_ASSIGNED;
       unsigned int i;
       int err;

       /* first, find free clusters in free chain pool */
       err = rfs_shrink_pool_chain(sb, RFS_LOG_I(sb)->pre_alloc_clus, 
                       RFS_LOG_PRE_ALLOC, &count, &p_next_clu);
       if (err)
               return err;

       p_prev_clu = RFS_POOL_I(sb)->start_cluster;
       RFS_LOG_I(sb)->where = RFS_POOL;

       /* if there are no free clusters in pool file,
          find free clusters in fat table */
       if (!count) {
               for (i = VALID_CLU; i < sbi->num_clusters; i++,
                               sbi->search_ptr++) { 
                       /* search free cluster from hint(search_ptr) */
                       if (sbi->search_ptr >= sbi->num_clusters)
                               sbi->search_ptr = VALID_CLU;
                       
                       err = fat_read(sb, sbi->search_ptr, &content);
                       if (err)
                               return err;

                       if (content == CLU_FREE) {
                               RFS_LOG_I(sb)->pre_alloc_clus[count++] =
                                       sbi->search_ptr;

                               if (count >= RFS_LOG_PRE_ALLOC)
                                       break;
                       }
               }
               if (!count)
                       return -ENOSPC;

               p_prev_clu = p_next_clu = CLU_TAIL;
               RFS_LOG_I(sb)->where = RFS_FAT_TABLE;
       }
       RFS_LOG_I(sb)->alloc_index = 0;
       RFS_LOG_I(sb)->numof_pre_alloc = count;

       /* in case of unlinked inode, t_next_clu is the head of other chain */
       if (RFS_I(inode)->last_clu != CLU_TAIL) {
               err = fat_read(sb, RFS_I(inode)->last_clu, &t_next_clu);
               if (err)
                       return err;
       } else
               t_next_clu = CLU_TAIL;

       if ((err = rfs_log_alloc_chain(sb, RFS_I(inode)->p_start_clu,
                                      RFS_I(inode)->index,
                                      p_prev_clu, p_next_clu,
                                      RFS_I(inode)->last_clu, t_next_clu,
                                      RFS_LOG_I(sb)->numof_pre_alloc,
                                      RFS_LOG_I(sb)->pre_alloc_clus))) {
               return err;
       }

       if (RFS_LOG_I(sb)->where == RFS_POOL) {
               if ((err = rfs_get_pool(sb, p_next_clu, count)))
                       return err;
       }

       return 0; 
}
