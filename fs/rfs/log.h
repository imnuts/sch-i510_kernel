/**
 * @file       fs/rfs/log.h
 * @brief      header file for log
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

#ifndef _LINUX_RFS_LOG_H
#define _LINUX_RFS_LOG_H

#ifdef __KERNEL__
#include <linux/sched.h>
#include <linux/rfs_fs.h>
#else
#define u8     unsigned char
#define u16    unsigned short
#define u32    unsigned int
#define u64    unsigned long long
#endif

/***************************************************************************/
/* RFS Common definitions                                                  */
/***************************************************************************/
/* max value is set with extra space */
#define RFS_LOG_MAX_ENTRIES             464
#define RFS_LOG_MAX_CLUSTERS            112


#if defined(CONFIG_RFS_PRE_ALLOC) && (CONFIG_RFS_PRE_ALLOC < RFS_LOG_MAX_CLUSTERS)
#define RFS_LOG_PRE_ALLOC              CONFIG_RFS_PRE_ALLOC
#else
#define RFS_LOG_PRE_ALLOC              RFS_LOG_MAX_CLUSTERS
#endif

/* logfile used for unlink */
#define RFS_LOG_FILE_NAME       "$RFS_LOG.LO$"

#define RFS_LOG_FILE_LEN       12

/***************************************************************************/
/* RFS dependant definitions                                               */
/***************************************************************************/
#define RFS_LOG_MAX_COUNT               256 

/* transaction type */
#define RFS_LOG_NONE                   0x0000
#define RFS_LOG_CREATE                 0x0001
#define RFS_LOG_RENAME                  0x0002
#define RFS_LOG_WRITE                   0x0003
#define RFS_LOG_TRUNCATE_F              0x0004
#define RFS_LOG_TRUNCATE_B              0x0005
#define RFS_LOG_UNLINK                  0x0006
#define RFS_LOG_DEL_INODE              0x0007
#define RFS_LOG_SYMLINK                        0x0008
#define RFS_LOG_REPLAY                 0x0010
#define RFS_LOG_INVALID                        0xffff

/* sub transaction type */
#define RFS_SUBLOG_COMMIT               0x0000
#define RFS_SUBLOG_ABORT               0x0001
#define RFS_SUBLOG_START                0x0002
#define RFS_SUBLOG_ALLOC_CHAIN         0x0003
#define RFS_SUBLOG_BUILD_ENTRY          0x0005
#define RFS_SUBLOG_REMOVE_ENTRY         0x0006
#define RFS_SUBLOG_UPDATE_ENTRY         0x0007
#define RFS_SUBLOG_GET_CHAIN           0x0009
#define RFS_SUBLOG_MOVE_CHAIN          0x000B
#define RFS_SUBLOG_UPDATE_POOL         0x000C


/* logtype specific MACRO */
#define MK_LOG_TYPE(X, Y)       (((X) << 16) | (Y))
#define GET_LOG(X)              ((X) >> 16)
#define GET_SUBLOG(X)           ((X) & 0x0000ffff)

/* where pre-alloction happens */
#define RFS_POOL               1
#define RFS_FAT_TABLE          2

#ifdef CONFIG_GCOV_PROFILE
typedef u32    rfs_log_seq_t;
#else
typedef u64    rfs_log_seq_t;
#endif

struct rfs_log_entry {
        u32    pdir;
        u32    entry;
        u32    numof_entries;
        u32    from_size;
        u32    to_size;
        u8     undel_buf[RFS_LOG_MAX_ENTRIES];
};

struct rfs_log_fat {
       /* two fields for minimal fat entry infos */
        u32    pdir;
        u32    entry;

       /* four fields for pool contents */
       u32     p_last_clu;
       u32     p_num_clus;
       u32     p_c_start_clu;
       u32     p_c_last_clu;

       /* two fields for pool chain */
       u32     p_prev_clu;
       u32     p_next_clu;

       /* two fields for target chain */
       u32     t_prev_clu;
       u32     t_next_clu;

       u32     numof_clus;
       u32     clus[RFS_LOG_MAX_CLUSTERS];
};

struct rfs_trans_log {
       u32     type;
       union {
               rfs_log_seq_t sequence;
               u64     dummy;  /* place holder for compatibility */
       };
       union {
               struct rfs_log_fat      log_fat;
               struct rfs_log_entry    log_entry;
       };
} __attribute__ ((packed));

#ifdef __KERNEL__
struct rfs_log_info {
       unsigned long blocks[RFS_LOG_MAX_COUNT];
       unsigned int secs_per_blk;
       unsigned int secs_per_blk_bits;
       unsigned int isec;
       unsigned int type;
       rfs_log_seq_t sequence;
       int dirty;

       void *log_mutex;

       unsigned int numof_pre_alloc;
       unsigned int alloc_index;
       unsigned int pre_alloc_clus[RFS_LOG_MAX_CLUSTERS];
       int where;

       struct inode *inode;            /* target file */
       struct buffer_head *bh;
       struct rfs_trans_log *log;

       unsigned int start_cluster;     /* for logfile itself */
#ifdef RFS_FOR_2_4
       struct inode tr_buf_inode;      /* in order to link transaction dirty buffers */
#endif
       struct inode *symlink_inode;    /* in order to point the symlink inode */
};

/* get rfs log info */
static inline struct rfs_log_info *RFS_LOG_I(struct super_block *sb)
{
       return (struct rfs_log_info *)(RFS_SB(sb)->log_info);
}

/* get rfs log */
static inline struct rfs_trans_log *RFS_LOG(struct super_block *sb)
{
       return ((struct rfs_log_info *)(RFS_SB(sb)->log_info))->log;
}

static inline void rfs_sync_bh(struct buffer_head *bh)
{
       if (!bh)
               return;
       
       if (buffer_dirty(bh)) {
               ll_rw_block(WRITE, 1, &bh);
               wait_on_buffer(bh);
       } else if (buffer_locked(bh))
               wait_on_buffer(bh);
}

static inline int is_empty_tr(struct super_block *sb)
{
       if (!(RFS_LOG_I(sb)->dirty))
               return 1;

       return 0;
}

static inline int tr_in_replay(struct super_block *sb)
{
       if (RFS_LOG_I(sb)->type == RFS_LOG_REPLAY)
               return 1;
       return 0;
}

/* called by FAT */
int rfs_log_init(struct super_block *sb);
int rfs_log_start(struct super_block *sb, unsigned int log_type,
               struct inode *inode);
int rfs_log_end(struct super_block *sb, int result);
int rfs_log_build_entry(struct super_block *sb, unsigned int pdir,
                unsigned int entry, unsigned int numof_entries);
int rfs_log_remove_entry(struct super_block *sb, unsigned int pdir,
                unsigned int entry, unsigned int numof_entries,
                unsigned char *undel_buf);
int rfs_log_update_entry(struct super_block *sb, unsigned int pdir,
                unsigned int entry, unsigned int from_size,
                unsigned int to_size);
int rfs_log_alloc_chain(struct super_block *sb,
               unsigned int pdir, unsigned int entry,
               unsigned int pool_prev, unsigned int pool_next,
               unsigned int target_prev, unsigned int target_next,
               unsigned int numof_clus, unsigned int *clus);
int rfs_log_move_chain(struct super_block *sb,
               unsigned int pdir, unsigned int entry,
               unsigned int pool_prev, unsigned int pool_next,
               unsigned int target_prev, unsigned int target_next,
               unsigned int numof_clus, unsigned int *clus);
int rfs_log_update_pool(struct super_block *sb);
int tr_pre_alloc(struct super_block *sb);
int rfs_log_get_cluster(struct inode *inode, unsigned int *new_clu);
void rfs_log_cleanup(struct super_block *sb);
int rfs_log_force_commit(struct super_block *sb, struct inode *inode);
int rfs_log_read(struct super_block *sb, unsigned int isec);
int rfs_log_write(struct rfs_log_info *log_info);
int rfs_log_mark_end(struct super_block *sb, unsigned int sub_type);
int rfs_meta_commit(struct super_block *);
int tr_deferred_commit(struct super_block *sb);

int rfs_log_replay(struct super_block *sb);
#endif /* __KERNEL__ */

#endif /* _LINUX_RFS_FS_H */

