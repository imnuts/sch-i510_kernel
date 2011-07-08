/**
 * @file       include/linux/xsr_if.h
 * @brief      XSR interface to export commands and macros to utils, fat
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
#ifndef _XSR_IF_H_
#define _XSR_IF_H_

#ifndef __KERNEL__
/*Warning*/
/*If you modify BML, you must check this definition*/
/*****************************************************************************/
/* Partition Entry ID of BML_LoadPIEntry()                                   */
/* Partition Entry ID from 0 to 0x0FFFFFFF is reserved in BML                */
/* Following ID is the pre-defined value and User can use Partition Entry ID */
/* from PARTITION_USER_DEF_BASE                                              */
/*****************************************************************************/
#define     PARTITION_ID_NBL1               0  /* NAND bootloader stage 1    */
#define     PARTITION_ID_NBL2               1  /* NAND bootloader stage 2    */
#define     PARTITION_ID_NBL3               2  /* NAND bootloader stage 3    */
#define     PARTITION_ID_COPIEDOS           3  /* OS image copied from NAND
                                                  flash memory to RAM        */
#define     PARTITION_ID_DEMANDONOS         4  /* OS image loaded on demand  */

#define     PARTITION_ID_FILESYSTEM         8  /* file system 0              */
#define     PARTITION_ID_FILESYSTEM1        9  /* file system 1              */
#define     PARTITION_ID_FILESYSTEM2        10 /* file system 2              */
#define     PARTITION_ID_FILESYSTEM3        11 /* file system 3              */
#define     PARTITION_ID_FILESYSTEM4        12 /* file system 4              */
#define     PARTITION_ID_FILESYSTEM5        13 /* file system 5              */
#define     PARTITION_ID_FILESYSTEM6        14 /* file system 6              */
#define     PARTITION_ID_FILESYSTEM7        15 /* file system 7              */
#define     PARTITION_ID_FILESYSTEM8        16 /* file system 8              */
#define     PARTITION_ID_FILESYSTEM9        17 /* file system 9              */
#define     PARTITION_ID_FILESYSTEM10       18 /* file system 10             */
#define     PARTITION_ID_FILESYSTEM11       19 /* file system 11             */
#define     PARTITION_ID_FILESYSTEM12       20 /* file system 12             */
#define     PARTITION_ID_FILESYSTEM13       21 /* file system 13             */
#define     PARTITION_ID_FILESYSTEM14       22 /* file system 14             */

#define     PARTITION_USER_DEF_BASE         0x10000000 /* partition id base for
                                                  user definition            */

/*****************************************************************************/
/* value of nAttr of XSRPartEntry structure                                  */
/* nAttr can be 'BML_PI_ATTR_FROZEN + BML_PI_ATTR_RO' or                     */
/*              'BML_PI_ATTR_RO'                      or                     */
/*              'BML_PI_ATTR_RW'.                                            */
/* other value is invalid attribute.                                         */
/*****************************************************************************/
#define     BML_PI_ATTR_FROZEN              0x00000020
#define     BML_PI_ATTR_RO                  0x00000002
#define     BML_PI_ATTR_RW                  0x00000001

#endif


/**
 * This file define some macro and it will shared user and kernel
 */
#ifdef CONFIG_XSR_DUAL_DEVICE
#define XSR_MAX_DEVICES                2
#else
#define XSR_MAX_DEVICES                1
#endif

/* this is support 15 partition*/
#define MASK(x)                        ((1U << (x)) -1)                        
#define PARTN_BITS              4 
#define PARTN_MASK             MASK(PARTN_BITS)
#define MAX_FLASH_PARTITIONS   ((0x1<< PARTN_BITS) - 1)
#define MAX_PAGE_SIZE 2048
#define MAX_OOB_SIZE 64

/* Device major number*/
#define XSR_BLK_DEVICE_RAW     137
#define XSR_BLK_DEVICE_FTL     138
/* distinguish chip and partition during dump and restore */
#define XSR_CHIP               0xaabb
#define XSR_PART               0xaacc
#define MAGIC_STR_SIZE         8
/* BML level ioctl commands */    
#define BML_GET_DEV_INFO     0x8A21 
#define BML_GET_PARTITION    0x8A22
#define BML_SET_PARTITION    0x8A23
#define BML_FORMAT           0x8A24
#define BML_ERASE_ALL        0x8A25
#define BML_ERASE_PARTITION  0x8A26
#define BML_DUMP            0x8A27
#define BML_RESTORE          0x8A28
#define BML_UNLOCK_ALL       0x8A29
#define BML_SET_RW_AREA      0x8A2A

typedef struct {
       unsigned int    offset;
       unsigned char mbuf[MAX_PAGE_SIZE];
       unsigned char sbuf[MAX_OOB_SIZE];
} BML_PAGEINFO_T;

typedef struct {
       int     phy_blk_size;  /* in bytes expect spare*/
       int     num_blocks;
       int     page_msize; /* main size in page */
       int     page_ssize; /* spare size in page */
} BML_DEVINFO_T;

typedef struct {
       int    num_parts;
       int    part_size[MAX_FLASH_PARTITIONS];  /* in number of blocks */
       int    part_id[MAX_FLASH_PARTITIONS]; /* device class */
       int    part_attr[MAX_FLASH_PARTITIONS]; /* device class */
} BML_PARTTAB_T;

/* STL level ioctl commands */
#define STL_FORMAT             0x8A01  /* FTL format     */
#define STL_GET_DEV_INFO       0x8A02  /* FTL stat       */
#define STL_CLEAN              0x8A03  /* FTL clean      */
#define STL_SYNC               0x8A13  /* FTL sync       */
#define STL_MAPDESTROY         0x8A14  /* FTL mapdestroy */

typedef struct {
       unsigned int total_sectors;
       unsigned int page_size;
}stl_info_t;

typedef struct {
       unsigned int fill_factor;
       unsigned int nr_reserved_units;
       unsigned int blocks_per_unit;
}stl_config_t;

#endif /* _XSR_IF_H_ */
