/**
 * @file        include/linux/rfs_fs.h
 * @brief       common header file for RFS
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
  
#ifndef _LINUX_RFS_FS_H
#define _LINUX_RFS_FS_H

#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
#include <linux/buffer_head.h>
#include <linux/rfs_fs_i.h>
#include <linux/rfs_fs_sb.h>
#else
#include <linux/locks.h>
#endif
#include <linux/byteorder/generic.h>
#include <linux/xsr_if.h>

/*
 * Constand and Mecro definition
 */
#define ROOT_INO               1

#define RFS_MAGIC               (0x2003BAB1L)

#define        DOS_NAME_LENGTH         11
#define EXT_UNAME_LENGTH       13
#define SHORT_NAME_LENGTH      8
#define UNICODE_NAME_LENGTH    256
#define MAX_NAME_LENGTH                UNICODE_NAME_LENGTH
#define MAX_TOTAL_LENGTH       260

#define DELETE_MARK            ((u8) 0xE5)             
#define EXT_END_MARK           ((u8) 0x40)             

#define KANJI_LEAD              ((u8)(0xE5))
#define REPLACE_KANJI           ((u8)(0x05))

#define SPACE                   ((u8)(0x20))    /* ' ' */
#define PERIOD                  ((u8)(0x2E))    /* '.' */
#define UNDERSCORE              ((u8)(0x5F))    /* '_' */
#define TILDE                  ((u8)(0x7E))    /* '~' */

#define PRIMARY_LOWER           ((u8)(0x08))
#define EXTENSION_LOWER         ((u8)(0x10))
#define PRIMARY_UPPER           ((u8)(0x07))
#define EXTENSION_UPPER         ((u8)(0x0E0))
#define UPPER_N_LOWER           ((u8)(0x0FF))

#define primary_masked(mixed)   (mixed & 0x00F)
#define extension_masked(mixed) (mixed & 0x0F0)

#define SIGNATURE               0xAA55

#define DENTRY_SIZE            32      /* MS-DOS FAT Compatible */
#define DENTRY_SIZE_BITS       5
#define MAX_ROOT_DENTRY                511     /* 0 ~ 511 */
#define MAX_DIR_DENTRY         65536

#define SECTOR_SIZE             512
#define SECTOR_BITS             9
#define SECTOR_MASK             (SECTOR_SIZE - 1)

/* attribute(FAT type) */
#define ATTR_NORMAL            ((u8) 0x00)
#define ATTR_READONLY          ((u8) 0x01)
#define ATTR_HIDDEN            ((u8) 0x02)
#define ATTR_SYSTEM            ((u8) 0x04)
#define ATTR_VOLUME            ((u8) 0x08)
#define ATTR_EXTEND            ((u8) 0x0F)
#define ATTR_SUBDIR            ((u8) 0x10)
#define ATTR_ARCHIVE           ((u8) 0x20)

/* type of directory entry(internal type) */
#define TYPE_UNUSED             ((u32) 0x00)
#define TYPE_DELETED            ((u32) 0x01)
#define TYPE_FILE               ((u32) 0x02)
#define TYPE_DIR                ((u32) 0x03)
#define TYPE_EXTEND             ((u32) 0x04)
#define TYPE_ALL                ((u32) 0x05)
#define TYPE_UNKNOWN            ((u32) 0x06)
#define TYPE_SYMLINK           ((u32) 0x07)
#define TYPE_VOLUME            ((u32) 0x08)

#define SYMLINK_MARK           ((u8) 0xE2)     /* symlink */

/* FAT type */
#define FAT16                  16
#define FAT32                  32

#define IS_FAT16(sbi)          ((sbi)->fat_bits == FAT16)              
#define IS_FAT32(sbi)          ((sbi)->fat_bits == FAT32)              
#define IS_VFAT(sbi)           ((sbi)->options.isvfat == TRUE)

/* threshold value(# of clusters) to determin the FAT type */
#define FAT12_THRESHOLD         4087        /* 4085 + clu 0 + clu 1 */
#define FAT16_THRESHOLD         65527       /* 65525 + clu 0 + clu 1 */
#define FAT32_THRESHOLD         268435447   /* 268435445 + clu 0 + clu 1*/

/* related with cluster */
#define CLU_TAIL               ((unsigned int) (~0))
#define CLU_FREE               ((unsigned int) (0))

#define VALID_CLU              2

/* fast unlink */
#define RFS_POOL_FILE_NAME     "RFS_POOL.SY$"
#define RFS_POOL_FILE_LEN      12
#define POOL_RESERVED_CLUSTER  1

#define SHRINK_POOL_SIZE       0
#define EXPAND_POOL_SIZE       1
#define SET_POOL_SIZE          2

/* Internal error code */
#define INVALID_ENTRY          131
#define INTERNAL_EOF           132

/* Miscellaneous definition */
#define TRUE                   1
#define FALSE                  0
#define DOT                    ".          "
#define DOTDOT                 "..         "

/* REVISIT: It's not fixed since it is changed from int to unsigned int */
#define NOT_ASSIGNED           (~0)

/* macro */
/* REVISIT : We need endian handling */
#define GET16(x)       le16_to_cpu(x)
#define GET32(x)       le32_to_cpu(x)
#define GET64(x)       le64_to_cpu(x)

#define SET64(dst, src)                        \
do {                                   \
       (dst) = cpu_to_le64(src);       \
} while (0)

#define SET32(dst, src)                        \
do {                                   \
       (dst) = cpu_to_le32(src);       \
} while (0)

#define SET16(dst, src)                \
do {                                   \
       (dst) = cpu_to_le16(src);       \
} while (0)

#define GET_FREE_CLUS(sbi)      ((sbi)->num_clusters - (sbi)->num_used_clusters)

#define START_CLUSTER(x)                                               \
       (((GET16((x)->start_clu_hi)) << 16) | GET16((x)->start_clu_lo))
#define START_BLOCK(x, sb)                                             \
       (((x - VALID_CLU) << RFS_SB(sb)->blks_per_clu_bits) +           \
       RFS_SB(sb)->data_start)

#define SET_XATTR_START_CLUSTER(x, xattr_start_clu)                     \
do {                                                                    \
        SET16((x)->ctime, (__u16)(xattr_start_clu & 0x0FFFF));  \
        SET16((x)->cdate, (__u16)(xattr_start_clu >> 16));      \
} while (0)

#define IS_FREE(name)  (((name[0] == DELETE_MARK) || (name[0] == 0x0))? 1 : 0 )

#define IS_XSR(x)              (MAJOR(x) == XSR_BLK_DEVICE_FTL ? 1 : 0)

#ifdef _RFS_INTERNAL_QUOTA
/* Mount flags */
#define RFS_MOUNT_USRQUOTA      0x01
#define RFS_MOUNT_GRPQUOTA      0x02
#endif
#define RFS_MOUNT_EA            0x04
#define RFS_MOUNT_CHECK_NO      0x08
#define RFS_MOUNT_CHECK_STRICT  0x10

/* #define RFS_MOUNT_POSIX_ACL  0x10 */
#define RFS_MOUNT_XATTR_USER            0x04000 /* Extended user attributes */

#define clear_opt(o, opt)               (o &= ~RFS_MOUNT_##opt)
#define set_opt(o, opt)                 (o |= RFS_MOUNT_##opt)
#define test_opt(sb, opt)               (RFS_SB(sb)->options.opts & \
                                                        RFS_MOUNT_##opt)

/* function macro */
#ifdef CONFIG_RFS_VFAT
#define find_entry             find_entry_long
#define build_entry            build_entry_long
#else
#define find_entry             find_entry_short
#define build_entry            build_entry_short
#endif
       
#define rfs_mark_buffer_dirty(x, sb)   mark_buffer_tr_dirty(x, sb)
#define rfs_mark_inode_dirty(x)                mark_inode_tr_dirty(x)

/*
 *  structure of partition entry (DISK)
 */
struct part_entry {
       u8    def_boot;
       u8    bgn_head;
       u8    bgn_sector;
       u8    bgn_cylinder;
       u8    sys_type;
       u8    end_head;
       u8    end_sector;
       u8    end_cylinder;
       u32   start_sector;
       u32   num_sectors;
} __attribute__ ((packed));

/*
 * structure of master boot record (DISK)
 */
struct mbr {
       u8    boot_code[446];
       u8    partition[64];
       u16   signature;
} __attribute__ ((packed));

/* 
 * structure of BIOS Parameter Block (DISK)
 */
struct bpb {
       u16     sector_size;
       u8      sectors_per_clu;
       u16     num_reserved;
       u8      num_fats;
       u16     num_root_entries;
       u16     num_sectors;
       u8      media_type;
       u16     num_fat_sectors;
       u16     sectors_in_track;
       u16     num_heads;
       u32     num_hidden_sectors;
       u32     num_huge_sectors;

       u32     num_fat32_sectors;
       u16     ext_flags;
       u16     version;
       u32     root_cluster;
       u16     fsinfo_sector;
       u16     backup_sector;
       u8      reserved[12];
} __attribute__ ((packed));

/* 
 * structure of additional BPB data (DISK)
 */
struct ext_bpb {
       u8      phy_drv_no;
       u8      reserved;
       u8      ext_signature;
       u8      vol_serial[4];
       u8      vol_label[11];
       u8      vol_type[8];
} __attribute__ ((packed));

/* 
 * structure of Parition Boot Record (DISK)
 */
struct pbr {
       u8      jmp_boot[3];
       u8      oem_name[8];
       u8      bpb[25];
       union {
               struct {
                       u8      ext_bpb[26];
#ifdef CONFIG_RFS_FS_XATTR
                        u8      boot_code[440];
#else
                        u8      boot_code[446];
#endif
               } fat16;
               struct {
                       u8      bpb[28];
                       u8      ext_bpb[26];
#ifdef CONFIG_RFS_FS_XATTR
                       u8      boot_code[412];
#else
                       u8      boot_code[418];
#endif
               } fat32;
       } u;
#ifdef CONFIG_RFS_FS_XATTR
       u16     xattr_root_flag;
       u32     xattr_start_clus;
#endif
       u8      boot_code[2];
       u16     signature;
} __attribute__ ((packed));

/*
 * structure of dir entry data on the disk (DISK)
 */
struct rfs_dir_entry {
       u8      name[DOS_NAME_LENGTH];  /* 8.3 name */
       u8      attr;
       u8      sysid;
       u8      cmsec;          /* create time in milliseconds */
       u16     ctime;          /* create time */
       u16     cdate;          /* create date */
       u16     adate;          /* access date */
       u16     start_clu_hi;   /* high 16-bits of start cluster */
       u16     mtime;          /* modify time */
       u16     mdate;          /* modify date */
       u16     start_clu_lo;   /* low 16-bits of start cluster */      
       u32     size;   
} __attribute__ ((packed));

/*
 * structure of extentry entry data on the disk (DISK)
 * extentry entry is needed for long file name 
 */
struct rfs_ext_entry {
       u8      entry_offset;
       u16     uni_0_4[5];             /* unicode 0 ~ 4 */
       u8      attr;
       u8      sysid;
       u8      checksum;
       u16     uni_5_10[6];            /* unicode 5 ~ 10 */
       u16     start_clu;              /* aligned */
       u16     uni_11_12[2];           /* unicode 11 ~ 12 */
} __attribute__ ((packed));

/* 
 * hint info for fast unlink (DISK/INCORE) 
 */
struct rfs_pool_info {
       u32 index;              /* index of pool file dir entry */
       u32 blocknr;            /* pool file dir entry is saved in this block */
       u32 start_cluster;      /* start cluster number of pool file */
       u32 last_cluster;       /* last cluster number in deleted segment */
       u32 num_clusters;       /* number of clusters except candidate segs */
       u32 c_start_cluster;    /* start clu # in candidate segment list */
       u32 c_last_cluster;     /* last clu # in candidate segment list */
       struct list_head c_list;        /* head for candidate segment list */
};

enum rfs_state_bits {
       BH_RFS_LOG_START = BH_PrivateStart,
       BH_RFS_LOG_COMMIT,
       BH_RFS_LOG,
       BH_RFS_FAT,
       BH_RFS_POOL_BLOCK,
       BH_RFS_ROOT,
       BH_RFS_DIR,
       BH_RFS_MBR,
       BH_RFS_DATA,
};

#ifdef CONFIG_RFS_IGET4
/*
 * structure for read_inode2 (INCORE)
 */
struct rfs_iget4_args {
       struct rfs_dir_entry *ep;
       u32 p_start_clu;
       u32 index;
};
#endif

/* status flag: [3] max n_tail [2] min n_tail [1] lossy, [0] mix */
#define get_lossy(status)      ((status >> 8) & 0xFF)
#define get_mix(status)                (u8) (status & 0xFF)

#define put_lossy(status, lossy)       \
do {   status |= ((lossy & 0xFF) << 8);                \
} while(0)
#define put_mix(status, mix)   \
do {   status |= (mix & 0xFF);         \
} while(0)

/*
 * vector operations
 */

/* inode.c */
int rfs_iunique (struct inode *, unsigned int, unsigned long *);
int fill_inode (struct inode *, struct rfs_dir_entry *, unsigned int, unsigned int);
struct inode *rfs_new_inode (struct inode *, struct dentry *, unsigned int);
void rfs_delete_inode (struct inode *);
int rfs_delete_entry (struct inode *, struct inode *);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
int rfs_write_inode (struct inode *, int);
#else
void rfs_write_inode (struct inode *, int);
#endif
#ifdef CONFIG_RFS_IGET4
void rfs_read_inode2 (struct inode *, void *);
#endif

static inline void set_mmu_private(struct inode *inode, loff_t value)
{
       struct super_block *sb = inode->i_sb;

       RFS_I(inode)->mmu_private = value;
       if (RFS_I(inode)->mmu_private & (sb->s_blocksize - 1)) {
               RFS_I(inode)->mmu_private |= (sb->s_blocksize -1);
               RFS_I(inode)->mmu_private++;
       }
}

/* super.c */
struct super_block *rfs_common_read_super (struct super_block *, void *, int);
int rfs_sync_vol(struct super_block *);

/* file.c */
int extend_with_zerofill (struct inode *, unsigned int, unsigned int); 
int rfs_setattr (struct dentry *, struct iattr *);
int rfs_bmap (struct inode *, long, unsigned long *);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
int rfs_get_block (struct inode *, sector_t, struct buffer_head *, int);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)
int rfs_permission (struct inode *, int);
#else
int rfs_permission (struct inode *, int, struct nameidata *);
#endif
#else
int rfs_get_block (struct inode *, long, struct buffer_head *, int);
int rfs_permission (struct inode *, int);
#endif
void rfs_invalidate_hint(struct inode *);

/* namei.c */
int build_entry_short (struct inode *, struct inode *, unsigned int, unsigned int, const char *);
int build_entry_long (struct inode *, struct inode *, unsigned int, unsigned int, const char *);
int check_reserved_files (struct inode *, const char *);

/* dir.c */
int is_dir_empty (struct inode *);
int init_new_dir (struct inode *);
int count_subdir (struct super_block *, unsigned int);

/* dos.c */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
void set_entry_time (struct rfs_dir_entry *, struct timespec);
#else
void set_entry_time (struct rfs_dir_entry *, time_t);
#endif
unsigned int entry_type (struct rfs_dir_entry *);
unsigned int entry_time (unsigned short, unsigned short);
void init_dir_entry (struct inode *, struct rfs_dir_entry *, unsigned int, unsigned int, const u8 *, unsigned char *); 
int init_ext_entry (struct rfs_ext_entry *, unsigned int, unsigned int, const u16 *, unsigned char);
unsigned char calc_checksum (const u8 *);
int mk_dosname (struct inode *dir, const char *name, u8 *dosname, unsigned char *mixed, u16 *uname);
int get_uname_from_entry (struct inode *, unsigned int , unsigned short *);
struct rfs_dir_entry *get_entry (struct inode *, unsigned int, struct buffer_head **);
struct rfs_dir_entry *get_entry_with_cluster (struct super_block *, unsigned int, unsigned int, struct buffer_head **);
int find_entry_short (struct inode *, const char *, struct buffer_head **, unsigned int);
int find_entry_long (struct inode *, const char *, struct buffer_head **, unsigned int); 
int remove_entry (struct inode *, unsigned int, struct buffer_head **);

/* cluster.c */
int fat_read (struct super_block *, unsigned int, unsigned int *);
int fat_write (struct super_block *, unsigned int, unsigned int);
int alloc_cluster (struct inode *, unsigned int *);
int rfs_map_destroy (struct super_block *);
int free_chain (struct inode *, unsigned int, unsigned int, unsigned int *);
int count_num_clusters (struct inode *);
int dealloc_clusters (struct inode *, unsigned int);
int count_used_clusters (struct super_block *, unsigned int *);
int append_new_cluster(struct inode *, unsigned int, unsigned int);
int find_free_cluster(struct inode *, unsigned int *);
int find_last_cluster(struct inode *, unsigned int *);
int find_cluster(struct super_block *, unsigned int, unsigned int, unsigned int *, unsigned int *);

int rfs_fcache_init (struct super_block *);
void rfs_fcache_release (struct super_block *);
void rfs_fcache_sync (struct super_block *, int);

int rfs_init_pool (struct super_block *);
void rfs_release_pool (struct super_block *);
int rfs_shrink_pool_chain (struct super_block *, unsigned int *, 
                               unsigned int,  unsigned int *, unsigned int *);
int rfs_get_pool (struct super_block *, unsigned int, unsigned int);
int rfs_attach_candidate (struct inode *);
int rfs_detach_candidate (struct inode *);
int rfs_remove_candidates (struct super_block *);
int rfs_update_pool_block (struct super_block *);
int rfs_update_pool_entry (struct super_block *, unsigned int, int);

static inline struct rfs_pool_info *RFS_POOL_I(struct super_block *sb)
{
       return (struct rfs_pool_info *)(RFS_SB(sb)->pool_info);
}

/* code_convert.c  */
void convert_dosname_to_cstring (char *, const u8 *, unsigned char);
int convert_uname_to_cstring (char *, const u16 *, struct nls_table *);
int convert_cstring_to_dosname(u8 *, const char *, unsigned int *, unsigned int);
int create_fatname(const char *, u8 *, u16 *, unsigned int *, struct nls_table *, unsigned int);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
/* rfs_26.c : Linux 2.6 dependent operations */
struct inode *rfs_alloc_inode (struct super_block *);
void rfs_destroy_inode (struct inode *);
#else
/* rfs_24.c : Linux 2.4 dependent operations */
void rfs_log_wakeup (unsigned long __data);
void rfs_data_wakeup (unsigned long __data);
int rfs_block_prepare_write(struct inode *inode, struct page *page, unsigned from, unsigned to, get_block_t *get_block);
int rfs_block_commit_write(struct inode *inode, struct page *page, unsigned from, unsigned to);
#endif

/* dir.c */
extern struct file_operations rfs_dir_operations;

/* file.c */
extern struct inode_operations rfs_file_inode_operations;
extern struct file_operations rfs_file_operations;

/* inode.c */
extern struct address_space_operations rfs_aops;

/* namei.c */
extern struct inode_operations rfs_dir_inode_operations;
extern struct dentry_operations rfs_dentry_operations;

/* symlink.c */
extern struct inode_operations rfs_symlink_inode_operations;

/* for transaction sync */
extern void mark_buffer_tr_dirty(struct buffer_head *bh, struct super_block *sb);
extern void mark_inode_tr_dirty(struct inode *inode);
#endif /* _LINUX_RFS_FS_H */
