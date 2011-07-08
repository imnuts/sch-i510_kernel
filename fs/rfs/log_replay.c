/**
 * @file       fs/rfs/log_replay.c
 * @brief      functions for replaying log
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

#include <linux/rfs_fs.h>
#include "rfs.h"
#include "log.h"

#ifdef LOG_BINARY_SCAN
#define SEARCH_MAX(sb, from, to, outvar)                               \
               binary_search_max(sb, from, to, outvar)
#else
#define SEARCH_MAX(sb, from, to, outvar)                               \
               sequential_search_max(sb, from, to, outvar)
#endif

/**
 * undo alloc chain from pool or fat table
 * @param sb super block
 * @return 0 on success, errno on failure
 *
 * make target's chain complete and then return allocated chain to pool or
 * fat table
 */
static int rfs_log_undo_alloc_chain(struct super_block *sb)
{
       struct rfs_log_fat *log_fat;
       unsigned int numof_clus;
       unsigned int sub_type;
       unsigned int i;
       int ret = 0;

       sub_type = GET_SUBLOG(GET32(RFS_LOG(sb)->type));
       log_fat = &(RFS_LOG(sb)->log_fat);

       if (GET32(log_fat->t_prev_clu) != CLU_TAIL) {
               /* make target chain complete */
               if (fat_write(sb, GET32(log_fat->t_prev_clu),
                                       GET32(log_fat->t_next_clu)))
                       return -EIO;
               DEBUG(DL1, "make target %u -> %u", GET32(log_fat->t_prev_clu),
                               GET32(log_fat->t_next_clu));
       } else if (GET32(log_fat->pdir) != CLU_TAIL) {
               /*
                * In case of mkdir, allocating cluster precedes
                * building entry, which is remarked
                * by setting CLU_TAIL to pdir
                */
               struct buffer_head *bh = NULL;
               struct rfs_dir_entry *ep;

               ep = get_entry_with_cluster(sb, GET32(log_fat->pdir),
                               GET32(log_fat->entry), &bh);
               if (IS_ERR(ep)) {
                       /*
                        * EFAULT means extension of parent is not flushed and
                        * there is no dir entry for new file.
                        */
                       if (PTR_ERR(ep) != -EFAULT) {
                               brelse(bh);
                               return -EIO;
                       }
               } else {
                       DEBUG(DL1, "ep's start_clu [%u -> 0]",
                                       START_CLUSTER(ep));

                       SET16(ep->start_clu_lo, 0);
                       SET16(ep->start_clu_hi, 0);

                       if (buffer_uptodate(bh))
                               mark_buffer_dirty(bh);
               }

               brelse(bh);
       }

       numof_clus = GET32(log_fat->numof_clus);

       if (sub_type == RFS_SUBLOG_GET_CHAIN) {
               /* link pool prev with head of chain */
               if (fat_write(sb, GET32(log_fat->p_prev_clu),
                                       GET32(log_fat->clus[0])))
                       return -EIO;

               /* make chain from pool complete */
               for (i = 0; i < numof_clus - 1; i++) {
                       if (fat_write(sb, GET32(log_fat->clus[i]),
                                     GET32(log_fat->clus[i + 1]))) {
                               return -EIO;
                       }
               }

               /* link chain with pool next */
               if (fat_write(sb, GET32(log_fat->clus[i]),
                                       GET32(log_fat->p_next_clu)))
                       return -EIO;

               DEBUG(DL1, "make pool %u -> %u, %u -> %u",
                       GET32(log_fat->p_prev_clu), GET32(log_fat->clus[0]),
                       GET32(log_fat->clus[i]), GET32(log_fat->p_next_clu));

               DEBUG(DL1, "make target %u -> %u",
                       GET32(log_fat->t_prev_clu), GET32(log_fat->t_next_clu));
       } else { /* RFS_SUBLOG_ALLOC_CHAIN */
               /* set free allocated chain */
               for (i = 0; i < numof_clus; i++) {
                       if (fat_write(sb, GET32(log_fat->clus[i]), CLU_FREE))
                               return -EIO;
                       DEBUG(DL2, "free %u", GET32(log_fat->clus[i]));
               }
       }

       /* update pool file */
       RFS_POOL_I(sb)->last_cluster = GET32(log_fat->p_last_clu);
       RFS_POOL_I(sb)->num_clusters = GET32(log_fat->p_num_clus);
       RFS_POOL_I(sb)->c_start_cluster = GET32(log_fat->p_c_start_clu);
       RFS_POOL_I(sb)->c_last_cluster = GET32(log_fat->p_c_last_clu);

       /* rewrite pool file */
       ret = rfs_update_pool_block(sb);
       if (ret)
               return ret;

       /* resize entry of pool file */
       ret = rfs_update_pool_entry(sb, GET32(log_fat->p_num_clus),
                       SET_POOL_SIZE);

       return ret;
}

#ifdef _RFS_INTERNAL_UNUSED_LOG
/**
 * undo dealloc chain to fat table
 * @param sb super block
 * @return 0 on success, -EIO on failure
 *
 * make sure the logged chain and link the chain with old prev and next
 */
static int rfs_log_undo_free_chain(struct super_block *sb)
{
       struct rfs_log_fat *log_fat;
       unsigned int numof_clus;
       unsigned int i;

       log_fat = &(RFS_LOG(sb)->log_fat);
       numof_clus = GET32(log_fat->numof_clus);

       /* link target prev with head of free_chain */
       if (GET32(log_fat->t_prev_clu) != CLU_TAIL) { /* if not unlink file */
               if (fat_write(sb, GET32(log_fat->t_prev_clu),
                                       GET32(log_fat->clus[0])))
                       return -EIO;
       } else {
               struct buffer_head *bh = NULL;
               struct rfs_dir_entry *ep;

               if (numof_clus == 0) {
                       RFS_BUG("RFS-log : no deleted chain\n");
                       return -EIO;
               }

               ep = get_entry_with_cluster(sb, GET32(log_fat->pdir),
                               GET32(log_fat->entry), &bh);
               if (IS_ERR(ep)) {
                       brelse(bh);
                       return -EIO;
               }
               DEBUG(DL1, "ep's start_clu [%u -> %u]",
                               START_CLUSTER(ep), GET32(log_fat->clus[0]));

               SET16(ep->start_clu_lo, GET32(log_fat->clus[0]));
               SET16(ep->start_clu_hi, GET32(log_fat->clus[0]) >> 16);

               if (buffer_uptodate(bh))
                       mark_buffer_dirty(bh);

               brelse(bh);
       }

       /* make chain from free clusters */
       for (i = 0; i < numof_clus - 1; i++) {
               if (fat_write(sb, GET32(log_fat->clus[i]),
                                       GET32(log_fat->clus[i + 1]))) {
                       return -EIO;
               }
       }

       /* link free_chain with target next */
       if (fat_write(sb, GET32(log_fat->clus[i]), GET32(log_fat->t_next_clu)))
               return -EIO;

       DEBUG(DL1, "make target %u -> %u, %u -> %u",
                       GET32(log_fat->t_prev_clu), GET32(log_fat->clus[0]),
                       GET32(log_fat->clus[i]), GET32(log_fat->t_next_clu));
       return 0;
}
#endif

/**
 * undo built entries
 * @param sb super block
 * @return 0 on success, errno on failure
 *
 *     for each entry in built entries {
 *             if (entry is built)
 *                     entry->name[0] = DELETION_MARK;
 *     }
 */
static int rfs_log_undo_built_entry(struct super_block *sb)
{
       struct rfs_log_entry *log_entry;
       struct buffer_head *bh = NULL;
       struct rfs_dir_entry *ep;
       unsigned int numof_entries;
       unsigned int i;
       int ret = 0;

       log_entry = &(RFS_LOG(sb)->log_entry);
       numof_entries = GET32(log_entry->numof_entries);

       for (i = 0; i < numof_entries; i++) {
               ep = get_entry_with_cluster(sb, GET32(log_entry->pdir),
                               GET32(log_entry->entry) - i, &bh);
               if (IS_ERR(ep)) {
                       ret = PTR_ERR(ep);
                       if (ret == -EFAULT) {
                               /* beyond file limit */
                               /* the extended cluster was
                                  not flushed, so do nothing
                                  for build-entry on it */
                               ret = 0;
                       }
                       goto rel_bh;
               }

               if (!IS_FREE(ep->name))
                       ep->name[0] = (unsigned char) DELETE_MARK;

               if (buffer_uptodate(bh))
                       mark_buffer_dirty(bh);
       }

rel_bh:
       brelse(bh);

       return ret;
}

/**
 * undo removed entries
 * @param sb super block
 * @return 0 on success, errno on failure
 * @pre the way to delete entries is marking deletion at ep->name[0] 
 *
 *     for each entry in built entries {
 *             entry->name[0] = log_entry->undel_buf[index];
 *     }
 */
static int rfs_log_undo_removed_entry(struct super_block *sb)
{
       struct rfs_log_entry *log_entry;
       struct buffer_head *bh = NULL;
       struct rfs_dir_entry *ep;
       unsigned int numof_entries;
       unsigned int i;
       int ret = 0;

       log_entry = &(RFS_LOG(sb)->log_entry);
       numof_entries = GET32(log_entry->numof_entries);

       for (i = 0; i < numof_entries; i++) {
               ep = get_entry_with_cluster(sb, GET32(log_entry->pdir),
                               GET32(log_entry->entry) - i, &bh);
               if (IS_ERR(ep)) {
                       /*
                        * In rfs_symlink(), removing entry could happen
                        * for recovery of ENOSPC. In this case,
                        * entries can not be found, so returns EFAULT.
                        */
                       if ((ret = PTR_ERR(ep)) == -EFAULT)
                               ret = 0;

                       goto rel_bh;
               }

               ep->name[0] = log_entry->undel_buf[i];

               if (buffer_uptodate(bh))
                       mark_buffer_dirty(bh);
       }

rel_bh:
       brelse(bh);

       return ret;
}

#ifdef _RFS_INTERNAL_UNUSED_LOG
/**
 * undo updated entry
 * @param sb super block
 * @return 0 on success, errno on failure
 *
 * restore old size
 */
static int rfs_log_undo_updated_entry(struct super_block *sb)
{
       struct rfs_log_entry *log_entry;
       struct buffer_head *bh = NULL;
       struct rfs_dir_entry *ep;
       int ret = 0;

       log_entry = &(RFS_LOG(sb)->log_entry);

       ep = get_entry_with_cluster(sb, GET32(log_entry->pdir),
                       GET32(log_entry->entry), &bh);
       if (IS_ERR(ep)) {
               ret = PTR_ERR(ep);
               goto rel_bh;
       }
       SET32(ep->size, GET32(log_entry->from_size));
       mark_buffer_dirty(bh);

rel_bh:
       brelse(bh);

       return ret;
}
#endif

/**
 * undo moving chain (delete chain or delete inode)
 * @param sb super block
 * @return 0 on success, errno on failure
 * @pre logging only head and tail for moved chain
 *
 * link logged chain with old prev and old, and rollback pool chain
 * update pool file
 */
static int rfs_log_undo_move_chain(struct super_block *sb)
{
       struct rfs_log_fat *log_fat;
       int ret;

       log_fat = &(RFS_LOG(sb)->log_fat);

       /* link target prev with head of chain */
       if (GET32(log_fat->t_prev_clu) != CLU_TAIL) {
               /* Shrink file to positive size or delete inode */
               if (fat_write(sb, GET32(log_fat->t_prev_clu),
                                       GET32(log_fat->clus[0])))
                       return -EIO;
       } else if (GET32(log_fat->pdir) != CLU_TAIL) {
               /*
                * Shrink file to 0 or unlink file.
                * In case of run-time rollback of mkdir by -ENOSPC,
                * when dir entry have not been built yet
                * log_fat->pdir has CLU_TAIL.
                */
               struct buffer_head *bh = NULL;
               struct rfs_dir_entry *ep;

               if (GET32(log_fat->numof_clus) == 0) {
                       RFS_BUG("RFS-log : no deleted chain\n");
                       return -EIO;
               }

               ep = get_entry_with_cluster(sb, GET32(log_fat->pdir),
                               GET32(log_fat->entry), &bh);
               if (IS_ERR(ep)) {
                       brelse(bh);
                       return -EIO;
               }
               DEBUG(DL1, "ep's start_clu [%u -> %u]",
                               START_CLUSTER(ep), GET32(log_fat->clus[0]));

               SET16(ep->start_clu_lo, GET32(log_fat->clus[0]));
               SET16(ep->start_clu_hi, GET32(log_fat->clus[0]) >> 16);

               if (buffer_uptodate(bh))
                       mark_buffer_dirty(bh);

               brelse(bh);
       }

       /* link tail of chain with tail of chain */
       if (fat_write(sb, GET32(log_fat->clus[1]), GET32(log_fat->t_next_clu)))
               return -EIO;

       DEBUG(DL1, "make target %u -> %u, %u -> %u",
                       GET32(log_fat->t_prev_clu), GET32(log_fat->clus[0]),
                       GET32(log_fat->clus[1]), GET32(log_fat->t_next_clu));
       
       /* make pool chain complete */
       if (fat_write(sb, GET32(log_fat->p_prev_clu),
                               GET32(log_fat->p_next_clu)))
               return -EIO;

       DEBUG(DL1, "make pool %u -> %u",
                       GET32(log_fat->p_prev_clu), GET32(log_fat->p_next_clu));

       /* update pool file */
       RFS_POOL_I(sb)->last_cluster = GET32(log_fat->p_last_clu);
       RFS_POOL_I(sb)->num_clusters = GET32(log_fat->p_num_clus);
       RFS_POOL_I(sb)->c_start_cluster = GET32(log_fat->p_c_start_clu);
       RFS_POOL_I(sb)->c_last_cluster = GET32(log_fat->p_c_last_clu);

       /* rewrite pool file */
       ret = rfs_update_pool_block(sb);
       if (ret)
               return ret;

       /* resize entry of pool file */
       ret = rfs_update_pool_entry(sb, GET32(log_fat->p_num_clus),
                       SET_POOL_SIZE);
       if (ret)
               return ret;

       return 0;
}

/**
 * undo change of pool file
 * @param sb super block
 * @return 0 on success, errno on failure
 * @pre logging only head and tail for moved chain
 */
static int rfs_log_undo_updated_pool(struct super_block *sb)
{
       struct rfs_log_fat *log_fat;
       int ret;

       log_fat = &(RFS_LOG(sb)->log_fat);

       /* update pool file */
       RFS_POOL_I(sb)->last_cluster = GET32(log_fat->p_last_clu);
       RFS_POOL_I(sb)->num_clusters = GET32(log_fat->p_num_clus);
       RFS_POOL_I(sb)->c_start_cluster = GET32(log_fat->p_c_start_clu);
       RFS_POOL_I(sb)->c_last_cluster = GET32(log_fat->p_c_last_clu);

       DEBUG(DL1, "POOL_INFO : <%u, %u>, <%u, %u>",
                       RFS_POOL_I(sb)->last_cluster,
                       RFS_POOL_I(sb)->num_clusters,
                       RFS_POOL_I(sb)->c_start_cluster,
                       RFS_POOL_I(sb)->c_last_cluster);

       /* rewrite pool file */
       ret = rfs_update_pool_block(sb);
       if (ret)
               return ret;

       /* resize entry of pool file */
       ret = rfs_update_pool_entry(sb, GET32(log_fat->p_num_clus),
                       SET_POOL_SIZE);
       if (ret)
               return ret;

       return 0;
}

/**
 * sequential search for the index of a log with max sequence
 * @param sb super block
 * @param from from index
 * @param to to index
 * @param[out] last_index the index of log with max sequence
 * @return 0 on success, error no on failure.
 */
static int sequential_search_max(struct super_block *sb, unsigned int from,
               unsigned int to, unsigned int *last_index)
{
       rfs_log_seq_t max_seq, seq;
       unsigned int i;

       max_seq = seq = 0;
       *last_index = -1;

       DEBUG(DL3, "from : %d, to : %d\n", from, to);
       for (i = from; i <= to; i++) {
               if (rfs_log_read(sb, i))
                       return -EIO;

               seq = GET64(RFS_LOG(sb)->sequence);
               
               DEBUG(DL3, "%d, max : 0x%0x, cur : 0x%0x\n",
                       i, (unsigned int) max_seq, (unsigned int) seq);
               if (max_seq < seq) {
                       max_seq = seq;
                       *last_index = i;
               }
       }

       DEBUG(DL2, "last tr is : %d, max_seq : 0x%0x\n",
                       *last_index, (unsigned int) max_seq);

       /* notice that it is possible to overflow sequence number */
       if (max_seq == (rfs_log_seq_t) (~0))
               return -EILSEQ;

       return 0;
}

/**
 * get the index of a log with max sequence in log file
 * @param sb super block
 * @param[out] last_index the index of log with max sequence
 * @return 0 or -ENOENT on success, other errnos on failure
 */
static int rfs_log_get_trans(struct super_block *sb, int *last_index)
{
       int ret;

       /* set default value */
       *last_index = -1;

       /* check empty log file */
       if (rfs_log_read(sb, 0))
               return -EIO;

       if (GET32(RFS_LOG(sb)->type) == RFS_LOG_NONE)
               return -ENOENT;

       
       ret = sequential_search_max(sb, 0, RFS_LOG_MAX_COUNT - 1, last_index);

       /*
        * TODO
        * Is it necessary to handle overflowing sequence number? (EILSEQ)
        */
       if (ret < 0)
               return ret;

       return 0;
}

/**
 * replay logs from latest uncommited log
 * @param sb rfs private super block
 * @return 0 on success, errno on failure
 * @pre super block should be initialized
 */
int rfs_log_replay(struct super_block *sb)
{
       rfs_log_seq_t prev_seq = 0;
       /* sub_type should be used as 'int' type */
       int sub_type = RFS_LOG_NONE;
       int last_index = 0, index;
       int ret = 0;

       if ((ret = rfs_log_get_trans(sb, &last_index))) {
               if (ret == -ENOENT) {
                       RFS_LOG_I(sb)->isec = 0;
                       ret = 0;
               }
               goto out;
       }

       ret = rfs_log_read(sb, last_index);
       if (ret)
               goto out;

       index = last_index;
       RFS_LOG_I(sb)->sequence = GET64(RFS_LOG(sb)->sequence) + 1;
       RFS_LOG_I(sb)->type = RFS_LOG_REPLAY;
       sub_type = GET_SUBLOG(GET32(RFS_LOG(sb)->type));

       /* In order to check continuity of sequence number */
       prev_seq = GET64(RFS_LOG(sb)->sequence) + 1;
       while (1) {
               /*
                * Note: It's important use 'int' type in swtich condition.
                *       'unsigned int' condition can't handle it correctly
                *       in gcov mode
                */

               /* Are the sequence numbers continuous? */
               if (prev_seq - 1 != GET64(RFS_LOG(sb)->sequence)) {
                       DPRINTK("Log records are not continuous."
                                       "Log file is broken\n");
                       return -EIO;
               }

               switch (sub_type) {
               case RFS_SUBLOG_ALLOC_CHAIN:
               case RFS_SUBLOG_GET_CHAIN:
                       DEBUG(DL0, "RFS_SUBLOG_ALLOC_CHAIN");
                       ret = rfs_log_undo_alloc_chain(sb);
                       break;
#ifdef _RFS_INTERNAL_UNUSED_LOG
               case RFS_SUBLOG_FREE_CHAIN:
                       DEBUG(DL0, "RFS_SUBLOG_FREE_CHAIN");
                       ret = rfs_log_undo_free_chain(sb);
                       break;
               case RFS_SUBLOG_UPDATE_ENTRY:
                       DEBUG(DL0, "RFS_SUBLOG_UPDATE_ENTRY");
                       ret = rfs_log_undo_updated_entry(sb);
                       break;
               case RFS_SUBLOG_PUT_CHAIN:
#endif
               case RFS_SUBLOG_MOVE_CHAIN:
                       DEBUG(DL0, "RFS_SUBLOG_MOVE_CHAIN");
                       ret = rfs_log_undo_move_chain(sb);
                       break;
               case RFS_SUBLOG_BUILD_ENTRY:
                       DEBUG(DL0, "RFS_SUBLOG_BUILD_ENTRY");
                       ret = rfs_log_undo_built_entry(sb);
                       break;
               case RFS_SUBLOG_REMOVE_ENTRY:
                       DEBUG(DL0, "RFS_SUBLOG_REMOVE_ENTRY");
                       ret = rfs_log_undo_removed_entry(sb);
                       break;
               case RFS_SUBLOG_UPDATE_POOL:
                       DEBUG(DL0, "RFS_SUBLOG_UPDATE_POOL");
                       ret = rfs_log_undo_updated_pool(sb);
                       break;
               case RFS_SUBLOG_ABORT: /* transaction is aborted */
                       DEBUG(DL0, "Latest transaction is aborted."
                               " Strongly recommend to check filesystem\n");
               case RFS_SUBLOG_COMMIT: /* transaction completes */
               case RFS_SUBLOG_START: /* transaction dose nothing */
                       /* do nothing. do not need to commit */
                       DEBUG(DL0, "RFS_SUBLOG_COMMIT");
                       if (last_index == index) {
                               brelse(RFS_LOG_I(sb)->bh);
                               RFS_LOG_I(sb)->bh = NULL;
                               RFS_LOG_I(sb)->log = NULL;
                               goto out;
                       }

                       goto commit;
               default:
                       RFS_BUG("RFS-log : Unsupporting log record\n");
                       ret = -EIO;
               }

               if (ret)
                       goto out;

               if (index == 0)
                       index = RFS_LOG_MAX_COUNT;

               /* the prev seq is greater than current by 1 */
               prev_seq--;

               /* get next log */
               ret = rfs_log_read(sb, --index);
               if (ret)
                       goto out;

               sub_type = GET_SUBLOG(GET32(RFS_LOG(sb)->type));
       }
commit:
       rfs_sync_vol(sb);

       /* commit mark */
       if (last_index == RFS_LOG_MAX_COUNT - 1)
               RFS_LOG_I(sb)->isec = 0;
       else
               RFS_LOG_I(sb)->isec = last_index + 1;

       ret = rfs_log_mark_end(sb, RFS_SUBLOG_COMMIT);

       DEBUG(DL0, "end mark commit");
       return ret;
out:
       RFS_LOG_I(sb)->type = RFS_LOG_NONE;
       
       return ret;
}
