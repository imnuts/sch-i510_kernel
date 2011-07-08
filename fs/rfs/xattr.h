/*
 *---------------------------------------------------------------------------*
 *                                                                           *
 *          COPYRIGHT 2003-2009 SAMSUNG ELECTRONICS CO., LTD.                *
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
/**
 *  @version   RFS_1.3.1_b070_RTM
 *  @file      fs/rfs/xattr.h
 *  @brief     local header file for xattr
 *
 *
 */
#include "rfs.h"
#ifndef _XATTR_H
#define _XATTR_H

#define RFS_XATTR_HEAD_SIGNATURE       ((__u16)(0x1097))
#define RFS_XATTR_ENTRY_SIGNATURE      ((__u16)(0xA5A5))
#define ENOATTR                ENODATA
#define EFULLATTR      ENOTEMPTY

/****************************************
 * configuration of Extended Attribute 
 ****************************************/
/* the maximum byte size of name */
#define RFS_XATTR_NAME_LENGTH_MAX      XATTR_NAME_MAX
/* the maximum byte size of value */
#define RFS_XATTR_VALUE_LENGTH_MAX     XATTR_SIZE_MAX
/* the maximum number of extended attributes */
#define RFS_XATTR_ENTRY_NUMBER_MAX     128
/* the maximum byte size of total extended attributes */
#define RFS_XATTR_LIST_SIZE_MAX                (64 * 1024)

/* Threshold size of xattr compaction. refer __xattr_compaction().*/
#define XATTR_COMPACTION_THRESHOLD     (RFS_XATTR_LIST_SIZE_MAX << 1)

/****************************************
 * marco define for Extended Attribute 
 ****************************************/
#define IS_XATTR_EXIST(rii)            ((rii)->xattr_start_clus != CLU_TAIL)
#define IS_USED_XATTR_ENTRY(flag)      ((flag) == XATTR_ENTRY_USED)
#define IS_DELETE_XATTR_ENTRY(flag)    ((flag) == XATTR_ENTRY_DELETE)


/****************************************
 * type define for Extended Attribute 
 ****************************************/
extern struct xattr_handler *rfs_xattr_handlers[];

typedef unsigned int   XATTR_NS; /* XATTR_NAMESPACE_ID */
enum _XATTR_NAMESPACE_ID
{
       RFS_XATTR_NS_USER               = 1,
       RFS_XATTR_NS_POSIX_ACL_ACCESS   = 2,
       RFS_XATTR_NS_POSIX_ACL_DEFAULT  = 3,
       RFS_XATTR_NS_TRUSTED            = 4,
       RFS_XATTR_NS_SECURITY           = 5
};

/* Flags for extended attribute set operation */
typedef int    XATTR_SET_FLAG;
enum _XATTR_SET_FLAG
{
/* if ATTR already exists, replace and if doesn't exist, create */
       XATTR_SET_FLAG_NONE             = 0x0000,
/* if ATTR already exists, fail */
       XATTR_SET_FLAG_CREATE           = 0x0001,
/* if ATTR does not exist, fail */
       XATTR_SET_FLAG_REPLACE          = 0x0002,
       XATTR_SET_FLAG_DUMMY            = 0x7FFFFFFF
};

/* flag for xattr entry status */
typedef signed int     XATTR_ENTRY_FLAG;
enum _XATTR_ENTRY_FLAG
{
       XATTR_ENTRY_NONE        = 0x00,         /* no flag */
       XATTR_ENTRY_USED        = 0x01,         /* used mark */
       XATTR_ENTRY_DELETE      = 0x02,         /* delete mark */
       XATTR_ENTRY_DUMMY       = 0x7FFFFFFF
};

/* xattr header struct */
/* [Caution] sizeof(rfs_xattr_header) should be 64byte */
struct rfs_xattr_header 
{
       __u16   signature;      /* signature of header*/
       __u16   ctime;          /* create time of directory entry */
       __u16   cdate;          /* create date of directriy entry */
       __u16   mode;           /* mode */
       __u32   uid;            /* user id */
       __u32   gid;            /* group id */
       __u16   valid_count;    /* valid attribute count */
       __u16   reserv1;        /* reservation field 1 */
       __u32   total_space;    /* used space (including deleted entry) */
       __u32   used_space;     /* used space (except deleted entry) */
       __u32   numof_clus;     /* number of allocated cluster */
       __u32   pdir;           /* parent directory' start cluster */
       __u32   entry;          /* directory index in pdir */
       __u32   reserv2[6];     /* reservation field 2 */
} __attribute__ ((packed));

/* xattr entry struct */
/* [Caution] sizeof(rfs_xattr_entry) should be 32byte */
struct rfs_xattr_entry {
       __u8    type_flag;      /* EA entry type flag */
       __u8    ns_id;          /* EA name space id */
       __u16   name_length;    /* EA name length (byte unit) */ 
       __u32   value_length;   /* EA value length (byte unit) */
       __u32   entry_length;   /* EA entry length (byte unit) 
                                * rfs_xattr_entry + name + value + padding 
                                * entray_length align to 32byte */
       __u16   crc16;          /* crc16 check sum (optional) */
       __u32   reserv[4];      /* reservation field  */
       __u16   signature;      /* signature of entry header */
} __attribute__ ((packed));

/* structure for ea parameter (in-memory struct) */
struct rfs_xattr_param
{
       /* extended attribute name or list of name */
       char            *name;
       /* name length */
       unsigned int    name_length;

       /* extended attribute value */
       void            *value;

       /* buffer size for extended attribute value or list */
       /* if size is 0, size of buffer or list will be returned */
       unsigned int    value_length;

       /* namespace id of extended attribute name */
       XATTR_NS        id;

       /* extended attribute set flag */
       XATTR_SET_FLAG  set_flag;
};

extern struct xattr_handler rfs_xattr_user_handler;
extern struct xattr_handler rfs_xattr_trusted_handler;
#ifdef CONFIG_RFS_FS_POSIX_ACL
extern struct xattr_handler rfs_xattr_acl_access_handler;
extern struct xattr_handler rfs_xattr_acl_default_handler;
#endif
#ifdef CONFIG_RFS_FS_SECURITY
extern struct xattr_handler rfs_xattr_security_handler;
#endif

/*********************************************
 * function prototypes for Extended Attribute 
 *********************************************/
extern int 
rfs_xattr_get(struct inode *, struct rfs_xattr_param *, unsigned int *);

extern int 
rfs_do_xattr_set(struct inode *, struct rfs_xattr_param *);
extern int 
rfs_xattr_set(struct inode *, struct rfs_xattr_param *);

extern int 
rfs_do_xattr_delete(struct inode *, struct rfs_xattr_param *);
extern int 
rfs_xattr_delete(struct inode *, struct rfs_xattr_param *);

extern ssize_t
rfs_xattr_list(struct dentry *, char *, size_t);

extern int 
rfs_xattr_read_header_to_inode(struct inode *, int);

extern int
rfs_xattr_write_header(struct inode *);

extern int
__xattr_io(int, struct inode *, unsigned long, char *, unsigned int);

#ifdef CONFIG_RFS_FS_SECURITY
extern int rfs_init_security(struct inode *inode, struct inode *dir);
#else
static inline int rfs_init_security(struct inode *inode, struct inode *dir)
{
       return 0;
}
#endif
#ifdef CONFIG_RFS_FS_FULL_PERMISSION
extern int 
rfs_do_xattr_set_guidmode(struct inode *, uid_t *, gid_t *, umode_t *);

extern int 
rfs_xattr_set_guidmode(struct inode *, uid_t *, gid_t *, umode_t *);

extern int 
rfs_xattr_get_guidmode(struct inode *, uid_t *, gid_t *, umode_t *);
#endif
#endif /* _XATTR_H */
