/**
 * @file       fs/rfs/rfs.h
 * @brief      local header file for rfs
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

#ifndef _RFS_H
#define _RFS_H

#include <linux/sched.h>
#include <linux/version.h>
#include <linux/buffer_head.h>

/*
 *  kernel version macro
 */
#undef RFS_FOR_2_4
#undef RFS_FOR_2_6
#undef RFS_FOR_2_6_10
#undef RFS_FOR_2_6_16
#undef RFS_FOR_2_6_17
#undef RFS_FOR_2_6_18
#undef RFS_FOR_2_6_19
#undef RFS_FOR_2_6_20
#undef RFS_FOR_2_6_22
#undef RFS_FOR_2_6_23
#undef RFS_FOR_2_6_24
#undef RFS_FOR_2_6_27
#undef RFS_FOR_2_6_29
#undef RFS_FOR_2_6_34

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
#define RFS_FOR_2_6            1
#else
#define RFS_FOR_2_4            1
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34)
#define RFS_FOR_2_6_34          1
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29)
#define RFS_FOR_2_6_29          1
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)
#define RFS_FOR_2_6_27          1
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24)
#define RFS_FOR_2_6_24          1
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 23)
#define RFS_FOR_2_6_23          1
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 22)
#define RFS_FOR_2_6_22          1
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20)
#define RFS_FOR_2_6_20          1
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 19)
#define RFS_FOR_2_6_19          1
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 18)
#define RFS_FOR_2_6_18          1
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 17)
#define RFS_FOR_2_6_17          1
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 16)
#define RFS_FOR_2_6_16          1
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 10)
#define RFS_FOR_2_6_10          1
#endif

#ifdef RFS_FOR_2_6_34
#include <linux/slab.h>
#endif

typedef enum rfs_lock_type {
       RFS_FAT_LOCK,
       RFS_LOG_LOCK,
} rfs_lock_type_t;

/*
 * for locking
 */
#define fat_lock(sb)           rfs_down(RFS_SB(sb)->fat_mutex, RFS_FAT_LOCK)
#define fat_unlock(sb)         rfs_up(RFS_SB(sb)->fat_mutex, RFS_FAT_LOCK)
#define init_fat_lock(sb)      rfs_init_mutex(RFS_SB(sb)->fat_mutex, sb)

#define lock_log(sb)           rfs_down(RFS_LOG_I(sb)->log_mutex, RFS_LOG_LOCK)
#define unlock_log(sb)         rfs_up(RFS_LOG_I(sb)->log_mutex, RFS_LOG_LOCK)
#define init_log_lock(sb)      rfs_init_mutex(RFS_LOG_I(sb)->log_mutex, sb)
#define get_log_lock_depth(sb) rfs_get_lock_depth(RFS_LOG_I(sb)->log_mutex)
#define get_log_lock_owner(sb) rfs_get_lock_owner(RFS_LOG_I(sb)->log_mutex)

#define IS_LOG_LOCK(type)      ((type) == RFS_LOG_LOCK)

/* 
 * for debugging
 */
#define DL0        (0)     /* Quiet   */
#define DL1        (1)     /* Audible */
#define DL2        (2)     /* Loud    */
#define DL3        (3)     /* Noisy   */

#define DPRINTK(format, args...)                                       \
do {                                                                   \
       printk("%s[%d]: " format, __func__ , __LINE__, ##args);         \
} while (0)

#undef DEBUG

#ifdef CONFIG_RFS_FAT_DEBUG

#define DEBUG(n, format, arg...)                                       \
do {                                                                   \
               if (n <= CONFIG_RFS_FAT_DEBUG_VERBOSE) {                        \
               printk("%s[%d]: " format "\n",                          \
                                __func__, __LINE__, ##arg);            \
               }                                                               \
} while(0)

#define RFS_BUG(format, args...)                                       \
do {                                                                   \
       DPRINTK(format, ##args);                                        \
       BUG();                                                          \
} while (0)

#define RFS_BUG_ON(condition)                                          \
do {                                                                   \
       BUG_ON(condition);                                              \
} while (0)

#else

#define DEBUG(n, arg...)       do { } while (0)
#define RFS_BUG(format, args...)       DPRINTK(format, ##args)
#define RFS_BUG_ON(condition)          do { } while (0)        

#endif /* CONFIG_RFS_FAT_DEBUG */

#define DATA_EXPIRES(j)                ((j) + CONFIG_RFS_LOG_WAKEUP_DELAY)

#define IS_INVAL_CLU(sbi, clu)                                         \
       ((clu < VALID_CLU || clu >= (sbi)->num_clusters) ? 1 : 0)

#define CHECK_RFS_INODE(inode, errno)                                  \
do {                                                                   \
       if (inode->i_ino != ROOT_INO) {                                 \
               if (RFS_I(inode)->p_start_clu >=                        \
                               RFS_SB(inode->i_sb)->num_clusters       \
                               || RFS_I(inode)->start_clu < VALID_CLU) { \
                               RFS_BUG("out of range (%u, %u, %u)",    \
                                       RFS_I(inode)->index,            \
                                       RFS_I(inode)->p_start_clu,      \
                                       RFS_I(inode)->start_clu);       \
                               return errno;                           \
               }                                                       \
       }                                                               \
} while (0)

#ifdef RFS_FOR_2_6_27
#include <linux/semaphore.h>
#else
#include <asm/semaphore.h>
#endif

struct rfs_semaphore {
       struct semaphore mutex;
       pid_t owner;
       int depth;
       struct super_block *sb;
};

void rfs_truncate(struct inode *);
int rfs_sync_inode(struct inode *, int, int);

/**
 * down the mutex
 * @param lock    a specific lock structure
 * @param type    lock type
 * @return        return the modified lock depth
*/
static inline int rfs_down(struct rfs_semaphore *lock, rfs_lock_type_t type)
{
       if (lock->owner == current->pid && lock->depth >= 1) {
               /* recursive lock */
               lock->depth++;
               DEBUG(DL3, "owner = %d depth = %d", lock->owner, lock->depth);
               return lock->depth;
       }

       /* first acquire */
       down(&lock->mutex);

#ifdef RFS_FOR_2_4
       if (IS_LOG_LOCK(type)) {
               /* register timer to avoid indefinite hang in wait_on_buffer */
               RFS_SB(lock->sb)->sleep_proc = current;
               RFS_SB(lock->sb)->timer.expires = DATA_EXPIRES(jiffies);
               add_timer(&RFS_SB(lock->sb)->timer);
       }
#endif

       lock->owner = current->pid;
       lock->depth++;

       DEBUG(DL3, "owner = %d depth = %d", lock->owner, lock->depth);

       return lock->depth;
}

/** 
 * up the mutex  
 * @param lock     a specific lock structure 
 * @param type     lock type
 * @return         return the modified lock depth
 */
static inline int rfs_up(struct rfs_semaphore *lock, rfs_lock_type_t type)
{
       if (lock->depth > 1) {
               lock->depth--;
       } else {
               DEBUG(DL3, "owner = %d depth = %d", lock->owner, lock->depth);

               lock->owner = -1;
               lock->depth--;

#ifdef RFS_FOR_2_4
               if (IS_LOG_LOCK(type))
                       del_timer_sync(&RFS_SB(lock->sb)->timer);
#endif

               up(&lock->mutex);
       }
       return lock->depth;
}

#ifdef RFS_FOR_2_4
/** 
 * down data mutex of ordered transaction for kernel2.4
 * @param rfsi     a native inode info
 */
static inline void rfs_data_down(struct rfs_inode_info *rfsi)
{
       down(&rfsi->data_mutex);

       /* register timer to avoid indefinite hang in wait_on_buffer */
       rfsi->sleep_proc = current;
       rfsi->timer.expires = DATA_EXPIRES(jiffies);
       add_timer(&rfsi->timer);
}

/** 
 * up data mutex of ordered transaction for kernel2.4
 * @param rfsi     a private inode
 */
static inline void rfs_data_up(struct rfs_inode_info *rfsi)
{
       del_timer_sync(&rfsi->timer);
       up(&rfsi->data_mutex);
}
#endif

/**
 * init the mutex
 * @param lock a specific lock structure
 * @param sb   super block
*/
static inline void rfs_init_mutex(struct rfs_semaphore *lock,
                                 struct super_block *sb)
{
       init_MUTEX(&lock->mutex);
       lock->owner = -1;
       lock->depth = 0;
       lock->sb = sb;
}

/** 
 * get the current depth  
 * @param lock     a specific lock structure 
 * @return         return the current lock depth
 */
static inline int rfs_get_lock_depth(struct rfs_semaphore *lock)
{
       return lock->depth;
}

/**
 * get the current lock owner
 * @param lock     a specific lock structure
 * @return         return the current lock owner
 */
static inline int rfs_get_lock_owner(struct rfs_semaphore *lock)
{
       return lock->owner;
}

inline static struct buffer_head * rfs_bread(struct super_block *sb,
               int block, int rfs_state)
{
       struct buffer_head *bh;
       bh = sb_bread(sb, block);

       return bh;
}
#endif /* _RFS_H */
