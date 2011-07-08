/**
 * @file       fs/rfs/dos.c
 * @brief      FAT directory entry Manipultion and mangement operations  
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
#include <linux/string.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/rfs_fs.h>

#include "rfs.h"
#include "log.h"

/* for dir entries manipulation */

/* local variable definition */
static const unsigned int days[] = {
       0,  31,  59,  90, 120, 151, 181, 212,
       243, 273, 304, 334,   0,   0,   0,   0
};
extern struct timezone sys_tz;

static const u8 *reserved_names[] = {
       "AUX     ", "CON     ", "NUL     ", "PRN     ",
       "COM1    ", "COM2    ", "COM3    ", "COM4    ",
       "LPT1    ", "LPT2    ", "LPT3    ", "LPT4    ",
       NULL };

#define MAX_DIGIT      4               /* maximum tail: 1024 */
#define MAX_NUMERIC    1024

#define SEC_PER_MIN    (60)            /* 60 secs / min */
#define MIN_PER_HR     (60)            /* 60 mins / hour */
#define SEC_PER_HR     (3600)          /* 3600 secs / hour */
#define HR_PER_DAY     (24)            /* 24 hours / day */
#define DAY_PER_YR     (365)           /* 365 days / year */
#define SEC_PER_DAY    (60 * 60 * 24)  /* 86400 secs / day */
#define DAY_PER_10YR   (365 * 10 + 2)  /* 3650 days / 10years */
#define SEC_PER_10YR   DAY_PER_10YR * SEC_PER_DAY      /* 10 years -> 315532800 secs */
#define MIN_DATE       SEC_PER_10YR

#define leap_days(yr)  ((yr + 3) / 4)  /* leap-year days during yr years */
#define leap_year(yr)  ((yr & 3) == 0) /* true if yr is the leap year */

/* time: [0-4] 2-second counts [5-10] minutes [11-15] hours */
#define FAT_time(sec, min, hour)       ((hour << 11) + (min << 5) + (sec / 2))
/* date: [0-4] day [5-8] month [9-15] years from 1980*/
#define FAT_date(day, month, year)     ((year << 9) + (month << 5) + day)

/**
 * Function converting linux time to FAT time
 * @param linux_date   linux time to be converted
 * @param time return value to contain a time of converted FAT time
 * @param date return value to contain a date of converted FAT time
 * @return     none
 *
 * Linux time starts from 1970.1.1 and FAT time starts from 1980.1.1
 */
static void convert_date_linux2dos(time_t linux_date, unsigned short *time, unsigned short *date)
{
       unsigned int day = 0, year = 0, month = 0;

       /* set to GMT time */
       linux_date -= sys_tz.tz_minuteswest * SEC_PER_MIN;

       /* set the minimum value of date to 1980.1.1 */
       if (linux_date < MIN_DATE)
               linux_date = MIN_DATE;

       /* set the FAT time */
       *time = FAT_time(linux_date % SEC_PER_MIN,
                        (linux_date / SEC_PER_MIN) % MIN_PER_HR,
                        (linux_date / SEC_PER_HR) % HR_PER_DAY);

       /* get the days & years */
       day = (linux_date - MIN_DATE) / SEC_PER_DAY;
       year = day / DAY_PER_YR;

       /* re-organize the year & day by the leap-years */
       if ((leap_days(year) + (year * DAY_PER_YR)) > day)
               year--;
       day -= (leap_days(year) + (year * DAY_PER_YR));

       /* find the month & day */
       if (day == days[2] && leap_year(year)) {
               month = 2;
       } else {
               if (leap_year(year) && (day > days[2]))
                       day--;

               for (month = 0; month < 12; month++)
                       if (days[month] > day)
                               break;
       }

       /* set the FAT date */
       *date = FAT_date(day - days[month - 1] + 1, month, year);
}

#ifdef CONFIG_RFS_VFAT
/**
 * Function setting a given buffer to zero
 * @param bitmap       character buffer to be filled with zero
 * @param mapsize      size of buffer
 * @return     none
 */
static inline void bitmap_clear_all(unsigned char *bitmap, unsigned int mapsize)
{
       memset(bitmap, 0x00, mapsize);
}

/**
 * Function check if certain bit of buffer is set to 1 
 * @param bitmap       character buffer to be tested
 * @param i    offset of target bit
 * @return     return 1 if a bit is 1, if not, 0
 */
static inline int bitmap_test(unsigned char *bitmap, int i)
{
       unsigned char row;

       row = bitmap[i >> 3];
       if ((row >> (i & 0x7)) & 0x1)
               return 1;
       else
               return 0;
}

/**
 * Function set certain bit of buffer to 1 
 * @param bitmap       character buffer to be modified
 * @param i    offset of target bit
 * @return     none
 */
static inline void rfs_bitmap_set(unsigned char *bitmap, int i)
{
       bitmap[i >> 3]  |= (0x01 << (i & 0x7));
}
#endif /* CONFIG_RFS_VFAT */

/**
 * Function checking if given dos file name is one of the reserved names
 * @param dosname      dos name to be tested
 * @return     return zero if dosname is valid, if not, a negative error code
 *
 * This function does validity check of give dos file name not to make dosname
 *  same with reserved name
 */
static int check_reserved(u8 *dosname) 
{
       const u8 **reserved;

       for (reserved = reserved_names; *reserved != NULL; reserved++) {
               if (!strncmp((char *) dosname, (char *)(*reserved), SHORT_NAME_LENGTH))
                       return -EINVAL;
       }

       return 0;
}

/**
 * Function checking if given name is valid
 * @param dosname      name to be tested
 * @param lossy                indicating wether the original string has lossy or not
 * @param isvfat       indicating VFAT if not zero
 * @return             return zero when name is valid if not a negative error code
 *
 * This function does the validity check of given file name. This function is
 * invoked prior to rfs_lookup
 */
static inline int check_name(u8 *dosname, unsigned int lossy, int isvfat)
{
       if (!isvfat || lossy != TRUE) 
               return check_reserved(dosname);
       else
               return 0;
}

/**
 * Function replacing the type of directory entry with give type  
 * @param ep   directory entry to be modified
 * @param type entry type
 * @return     return zero on success, a negative value on failure
 */
static int set_entry_type(struct rfs_dir_entry *ep, unsigned char type)
{
       if (type == TYPE_UNUSED) {
               ep->name[0] = 0x0;
       } else if (type == TYPE_DELETED) {
               ep->name[0] = (u8) DELETE_MARK;
       } else if (type == TYPE_EXTEND) {
               ep->attr = (u8) ATTR_EXTEND;
       } else if (type == TYPE_DIR) {
               ep->attr = (u8) ATTR_SUBDIR;
       } else if (type == TYPE_FILE) {
               ep->attr = (u8) ATTR_ARCHIVE;
       } else if (type == TYPE_SYMLINK) {
               ep->attr = (u8) ATTR_ARCHIVE;
               ep->cmsec = SYMLINK_MARK;
       } else {        /* out-of-range input */
               return -EINVAL;
       }

       return 0;
}

#ifdef CONFIG_RFS_VFAT
/**
 * Function retrieving unicode name from extend directory entry
 * @param extp extend entry from which unicode is extracted
 * @param uname        pointer to array to store a unicode name 
 * @param is_last      condition flag indicating if current extend entry
 *                     is the last one.
 * @return     return zero on success, a negative value on failure
 *                       
 * This function is invokded to extract unicode names scatterd in several
 * directory entries and put them together so that they can be commplete 
 * unicode name.
 */
static int get_uname_from_ext_entry(struct rfs_ext_entry *extp, u16 *uname, unsigned int is_last)
{
       unsigned int i;

       for (i = 0; i < EXT_UNAME_LENGTH; i++) {
               if (i < 5) {
                       uname[i] = GET16(extp->uni_0_4[i]);
               } else if (i < 11) {
                       uname[i] = GET16(extp->uni_5_10[i - 5]);
               } else {
                       uname[i] = GET16(extp->uni_11_12[i - 11]);
               }
               if (uname[i] == 0x0) 
                       return 0;
       }

       if (is_last) 
               uname[i] = 0x0;

       return 0;
}

/**
 * Function searching numeric tail in an effort to avoid redundancy
 * of filename.
 * @param dir          inode corresponding to given dos name
 * @param dos_name     dos_name to which numeric tail is to be appended
 * @return             return zero on success, a negative value on failure
 *
 * This function check if any file having same dos name with given one exists
 * in the directory and calcuate a numeric tail. 
 */
static int get_numeric_tail(struct inode *dir, u8 *dos_name) 
{
       int has_tilde;
       unsigned char bmap[MAX_NUMERIC >> 3]; 
       unsigned int type;
       unsigned int i, j, cpos = 0; 
       unsigned int count = 0;
       struct buffer_head *bh = NULL;
       struct rfs_dir_entry *ep = NULL;
       int err = 0;
               
       bitmap_clear_all(bmap, (MAX_NUMERIC >> 3));
       rfs_bitmap_set(bmap, 0);

       while (1) {
               ep = get_entry(dir, cpos++, &bh);
               if (IS_ERR(ep)) {
                       if (PTR_ERR(ep) == -EFAULT) {   /* end-of-directory */
                               break;
                       }
                       err = PTR_ERR(ep);
                       goto out;
               }

               type = entry_type(ep);
               if (type == TYPE_FILE || type == TYPE_DIR) {
                       for (i = 0; i < SHORT_NAME_LENGTH; i++) {
                               if (ep->name[i] == TILDE)
                                       break;
                       }
                       if (strncmp(ep->name, dos_name, i))
                               continue;
                       
                       has_tilde = FALSE;
                       for (i = 0, count = 0; i < SHORT_NAME_LENGTH; i++) {
                               if (ep->name[i] == TILDE) {
                                       has_tilde = TRUE;
                               } else if (has_tilde) {
                                       if (ep->name[i] >= '0' && ep->name[i] <='9') {
                                               count = count * 10 + (ep->name[i] - '0');
                                       }
                               }
                       }
               } else if (type == TYPE_UNUSED) {
                       break; /* end of valid entry */
               }

               if (count) 
                       rfs_bitmap_set(bmap, count);
       }

       for (count = 0, i = 0; (!count) && (i < (MAX_NUMERIC >> 3)); i++) {
               if (bmap[i] != (u8) 0xff) {
                       for (j = 0; j < SHORT_NAME_LENGTH; j++) {
                               if (bitmap_test(&bmap[i], j) == 0) {
                                       count = (i << 3) + j;
                                       break;
                               }
                       }
               }
       }

       if (count == 0 || count >= MAX_NUMERIC) {
               err = -ENOENT;  /* out-of-range input (numeric tail) */
               goto out;
       } else {
               err = count;
       }
       
out:
       if (!IS_ERR(bh))
               brelse(bh);

       return err;
}

/**
 * Function appending a given numerical tail to given dos name  
 * @param dos_name     directory entry to be modified
 * @param count                numerical tail to be appended
 * @return     none
 *
 * This function does not return error
 */
static void append_tail(u8 *dos_name, unsigned int count)
{
       char str_tail[SHORT_NAME_LENGTH - 1];
       int length, n_tail = 0;

       sprintf(str_tail, "%c%d", TILDE, count);
       length = strlen(str_tail);

       while (n_tail < (SHORT_NAME_LENGTH - length)) {
               if (dos_name[n_tail] == SPACE)
                       break;
               if (dos_name[n_tail] & 0x80) {
                       if (n_tail + 2 < (SHORT_NAME_LENGTH - length))
                               n_tail += 2;
                       else 
                               break;
               } else {
                       n_tail++;
               }
       }

       memcpy(&(dos_name[n_tail]), str_tail, length);

       memset(&(dos_name[n_tail + length]), SPACE, 
                       SHORT_NAME_LENGTH - length - n_tail);
}

/**
 * Function appending numeric tail in an effort to avoid redundancy
 * of filename.
 * @param dir          inode corresponding to given dos name
 * @param dos_name     dos_name to which numeric tail is to be appended
 * @return             return zero on success, a negative value on failure
 *
 * This function check if any file having same dos name with given one exists
 * in the directory and calcuate a numeric tail. 
 */
static int add_numeric_tail(struct inode *dir, u8 *dos_name)
{
       int count = 0;
       
       count = get_numeric_tail(dir, dos_name);
       if (count < 0)
               return count;

       append_tail(dos_name, count);

       return 0;
}
#endif /* CONFIG_RFS_VFAT */

/*
 *  Global function 
 */

/**
 *  Function assigning a FAT time to directory entry by converting a given
 *  linux time.
 *  @param ep  directory entry to be modified
 *  @param cur_time    linux time to be converted
 *  @return    none
 */
#ifdef RFS_FOR_2_6
void set_entry_time(struct rfs_dir_entry *ep, struct timespec cur_time)
{
       unsigned short time = 0x00, date = 0x21;
       struct timespec tmp_time;

       /* set current system time by default */
       tmp_time = (cur_time.tv_sec > 0) ? cur_time : CURRENT_TIME;
       convert_date_linux2dos(tmp_time.tv_sec, &time, &date);
#else
void set_entry_time(struct rfs_dir_entry *ep, time_t cur_time)
{
       unsigned short time = 0x00, date = 0x21;
       time_t tmp_time = 0;

       /* set current system time by default */
       tmp_time = (cur_time > 0) ? cur_time : CURRENT_TIME;
       convert_date_linux2dos(tmp_time, &time, &date);
#endif

       SET16(ep->mtime, time);
       SET16(ep->mdate, date);
}

/**
 * Function retreiving the type of entry and convert it to internal type flag
 * @param ep   directory entry to extract type information
 * @return     internal type flag
 */
unsigned int entry_type(struct rfs_dir_entry *ep)
{
       if (ep->name[0] == (u8) 0x0)
               return TYPE_UNUSED;
       else if (ep->name[0] == (u8) DELETE_MARK)
               return TYPE_DELETED;
       else if (ep->attr == (u8) ATTR_EXTEND)
               return TYPE_EXTEND;
       else if (ep->attr & (u8) ATTR_SUBDIR)
               return TYPE_DIR;
       else if (ep->attr & (u8) ATTR_VOLUME)
               return TYPE_VOLUME;

       return TYPE_FILE;
}

#define DOS_time(sec, min, hour)       ((sec * 2) + (min * SEC_PER_MIN) + (hour * SEC_PER_HR))
#define days_in_years(year)    (year / 4) + (year * DAY_PER_YR)
#define DOS_date(day, month, year)     \
       ((day - 1) + days[month] + days_in_years(year) + \
        ((leap_year(year) && (month < 2))? 0: 1))

/**
 *  Function converting a FAT time to linux time 
 *  @param time        a time value of the FAT time            
 *  @param date        a date value of teh FAT time
 *  @return    converted linux time
 */
unsigned int entry_time(unsigned short time, unsigned short date)
{
       int secs, day;

       /* first subtract and mask after that... Otherwise, if
          date == 0, bad things happen */

       secs = DOS_time((time & 0x1F), ((time >> 5) & 0x3F), (time >> 11));

       /* days since 1.1.70 plus 80's leap day */
       day = DOS_date((date & 0x1F), (((date >> 5) - 1) & 0xF), (date >> 9)) +
               DAY_PER_10YR;

       secs += day * SEC_PER_DAY;

       /* reflect timezone */
       secs += sys_tz.tz_minuteswest * 60;

       return secs;
}

#ifdef CONFIG_RFS_VFAT
/**
 * Function calculating checksum for given dos name
 * @param dos_name     name to be calcuated
 * @return     checksum value
 *
 * This function calcuate the check of dos name, which is the key 
 * to identify the validity of the dos entry.
 */
unsigned char calc_checksum(const u8 *dos_name) 
{
       unsigned char checksum = 0;
       int i;

       for (i = 0; i < DOS_NAME_LENGTH; i++, dos_name++)
               checksum = (((checksum & 1) << 7) | ((checksum & 0xFE) >> 1)) + *dos_name;

       return checksum;
}
#endif /* CONFIG_RFS_VFAT */

/**
 * Function to create dosname with given cstring
 * @param dir          inode pointer
 * @param name         given cstring
 * @param dosname      empty string buffer
 * @param mixed                return value indicating mixed string
 * @param uname                return unicode name from cstring name
 * @return             the number of the extend slots needed on success, a negative error code on failure
 */
int mk_dosname(struct inode *dir, const char *name, u8 *dosname, unsigned char *mixed, u16 *uname)
{
       struct rfs_sb_info *sbi = RFS_SB(dir->i_sb);
       unsigned int status = 0;
       int ret = 0;
       int uni_len;

       /* make uname and base dosname (without numeric-tail) */
       uni_len = create_fatname(name, dosname, uname, &status, sbi->nls_disk, TRUE);
       if (uni_len < 0) 
               return uni_len;

       /* check reserved names */
       /* if not support long name, 
        * must check reserved names regardless of lossy 
        * if support long name, only check reserved names if lossy is true
        */

       ret = check_name(dosname, get_lossy(status), IS_VFAT(sbi));
       if (ret < 0)
               return ret;

#ifdef CONFIG_RFS_VFAT
       /* when long file name is supported */
       if (get_lossy(status)) {
               ret = add_numeric_tail(dir, dosname);
               if (ret < 0)
                       return ret;

               ret = ((uni_len + (EXT_UNAME_LENGTH - 1)) / EXT_UNAME_LENGTH);
       } else if (get_mix(status) == UPPER_N_LOWER) {
               ret = ((uni_len + (EXT_UNAME_LENGTH - 1)) / EXT_UNAME_LENGTH);
       } else {
               ret = 0;
       }

       if (mixed) {
               /* 
                * if SFN has extra slot, 
                * dosname in SFN has mark of lower case in sysid 
                */
               if (ret)
                       *mixed = (PRIMARY_LOWER | EXTENSION_LOWER);
               else
                       *mixed = get_mix(status);
       }

#else
       /* when long file name isn't supported */
       ret = 0;
       if (mixed) {
               /* dosname has mark of lower case in sysid */
               if (get_mix(status) == UPPER_N_LOWER)
                       *mixed = (PRIMARY_LOWER | EXTENSION_LOWER);
               else
                       *mixed = get_mix(status);
       }
#endif /* CONFIG_RFS_VFAT */

       /* there's no unicode name for the extend slots */
       if ((ret == 0) && uname)
               uname[0] = 0x0000;

       /* return the number of extend slots needed */
       return ret;
}

/**
 * Function initializing given entry
 * @param dir          inode of parent directory
 * @param ep           directory entry to be initalized
 * @param type         entry type
 * @param start_clu    start cluster of file
 * @param dosname      dos name of file
 * @param mixed                return value indicating if a file needs extend entry    
 * @return     none
 */
void init_dir_entry(struct inode *dir, struct rfs_dir_entry *ep, unsigned int type, unsigned int start_clu, const u8 *dosname, unsigned char *mixed) 
{
       ep->cmsec = 0;
       SET16(ep->ctime, 0);
       SET16(ep->cdate, 0);
       SET16(ep->adate, 0);
       set_entry_time(ep, CURRENT_TIME);

       SET16(ep->start_clu_lo, start_clu);
       SET16(ep->start_clu_hi, start_clu >> 16);

       SET32(ep->size, 0);

       set_entry_type(ep, type);
       ep->sysid = *mixed;
       memcpy(ep->name, dosname, DOS_NAME_LENGTH);
}

#ifdef CONFIG_RFS_VFAT
/**
 * Function initializing given extend entry
 * @param extp         extend entry to be initalized
 * @param type         entry type
 * @param entry_offset a sequence number of current extend entry
 * @param uname                unicode name converted from original file name
 * @param checksum     checksum value
 * @return     return zero on success, a negative value on failure
 */
int init_ext_entry(struct rfs_ext_entry *extp, unsigned int type, unsigned int entry_offset, const u16 *uname, unsigned char checksum) 
{
       int i, uname_end = FALSE;
       u16 part;

       if (unlikely(!uname))   /* out-of-range input */
               return -EIO;

       set_entry_type((struct rfs_dir_entry *)extp, type);
       extp->entry_offset = (u8) entry_offset;
       extp->sysid = 0;
       extp->checksum = checksum;
       SET16(extp->start_clu, 0);

       for (i =0; i < EXT_UNAME_LENGTH; i++) {
               if (!uname_end)
                       part = uname[i];
               else
                       part = 0xFFFF;

               if (i < 5)
                       SET16(extp->uni_0_4[i], part);
               else if (i < 11)
                       SET16(extp->uni_5_10[i - 5], part);
               else
                       SET16(extp->uni_11_12[i - 11], part);
               
               if (uname[i] == 0x0) 
                       uname_end = TRUE;
       }

       return 0;       
}
#endif /* CONFIG_RFS_VFAT */

/**
 * get block number related with offset in FAT16 root directory
 * @param sb                   super block pointer
 * @param[in, out] offset      offset of directory entry
 * @param[out] block           block number 
 * @return                     return 0 on success, errno on failure   
 */
static inline int get_root_block(struct super_block *sb, unsigned int *offset, unsigned long *block)
{
       struct rfs_sb_info *sbi = RFS_SB(sb);
       unsigned int off = *offset;

       off += sbi->root_start_addr;
       if (off > sbi->root_end_addr)   /* out-of-range input */
               return -EFAULT;

       *offset = off;
       *block = off >> sb->s_blocksize_bits;
       return 0;
}

/**
 * Function retriving directory entry using a given cluster value
 * @param sb   super block pointer
 * @param clu  cluster value
 * @param entry        entry offset
 * @param bh   buffer head pointer
 * @return     a pointer of directory entry on success, a negative value on failure
 */
struct rfs_dir_entry *get_entry_with_cluster(struct super_block *sb, unsigned int clu, unsigned int entry, struct buffer_head **bh)
{
       struct rfs_sb_info *sbi = RFS_SB(sb);
       struct rfs_dir_entry *ep = NULL;
       unsigned long block;
       unsigned int off;
       int err;

       off = entry << DENTRY_SIZE_BITS;

       if ((clu != sbi->root_clu) || IS_FAT32(sbi)) {
               unsigned int cluster_offset, block_offset;
               unsigned int prev, next;

               /* FAT32 or sub directory */
               cluster_offset = off >> sbi->cluster_bits;
               block_offset = (off >> sb->s_blocksize_bits) 
                               & (sbi->blks_per_clu - 1);
               fat_lock(sb);
               err = find_cluster(sb, clu, cluster_offset, &prev, &next);
               fat_unlock(sb);
               if (err)
                       return ERR_PTR(err);
               block = START_BLOCK(prev, sb) + block_offset;
       } else {
               /* FAT16 root directory */
               err = get_root_block(sb, &off, &block);
               if (err)
                       return ERR_PTR(err);
       }

       if (*bh)
               brelse(*bh);

       *bh = rfs_bread(sb, block, BH_RFS_DIR);
       if (!(*bh)) {   /* I/O error */
               DPRINTK("can't get buffer head\n");
               return ERR_PTR(-EIO);
       }

       off &= sb->s_blocksize - 1;
       ep = (struct rfs_dir_entry *) ((*bh)->b_data + off);

       return ep;
}

/**
 * Function retriving directy entry indicted by given entry num
 * @param dir          inode relating to seeking entry
 * @param entry                entry position to be found
 * @param res_bh       buffer head to contain found directry portion
 * @return     a pointer of directory entry on success, a negative value on failure
 *
 * Because of difference in name handling routine, find_entry_long handles only
 * searching with long file name.
 */
struct rfs_dir_entry *get_entry(struct inode *dir, unsigned int entry, struct buffer_head **res_bh) 
{
       struct super_block *sb = dir->i_sb;
       struct rfs_sb_info *sbi = RFS_SB(sb);
       struct rfs_dir_entry *ep = NULL;
       unsigned long block;
       unsigned int off;
       int err;

       /* 1. get the real offset in the buffer head */
       off = entry << DENTRY_SIZE_BITS;

       if ((RFS_I(dir)->start_clu != sbi->root_clu) || IS_FAT32(sbi)) {
               long iblock;

               /* FAT32 or sub directory */
               iblock = off >> sb->s_blocksize_bits;
               err = rfs_bmap(dir, iblock, &block);
       } else {
               /* FAT16 root directory */
               err = get_root_block(sb, &off, &block);
       }

       if (err)
               return ERR_PTR(err);

       if (*res_bh)
               brelse(*res_bh);
       
       *res_bh = rfs_bread(sb, block, BH_RFS_DIR);
       if (*res_bh == NULL) {  /* I/O error */
               DPRINTK("can't get buffer head\n");
               return ERR_PTR(-EIO);
       }

       off &= sb->s_blocksize - 1;
       ep = (struct rfs_dir_entry *) ((*res_bh)->b_data + off);

       return ep;
}      

#ifdef CONFIG_RFS_VFAT
/**
 * Function retrieving unicode name from directory entry
 * @param dir  inode corresponding to directory entry
 * @param entry        the position of directory entry with short file name
 * @param uname        variable for retrieved unicode name
 * @return     zero on success, a negative error code on failure
 *
 * This function retrieves unicode name by seeking extend directory entry
 * with given a offset of short file name entry.
 */
int get_uname_from_entry (struct inode *dir, unsigned int entry, u16 *uname)
{
       struct rfs_ext_entry *extp = NULL;
       struct rfs_dir_entry *ep = NULL;
       struct buffer_head *bh = NULL;
       int i, err = 0;
       
       if ((int) entry <= 0)           /* out-of-range input */
               return err;

       for (i = 0, entry--; entry >= 0; entry--) {
               ep = get_entry(dir, entry, &bh);
               if (IS_ERR(ep)) {
                       if (PTR_ERR(ep) != -EFAULT)
                               err = PTR_ERR(ep);
                       break;
               }
               extp = (struct rfs_ext_entry *)ep;
               /* boundary check for max length of array  & error handling*/
               if (entry_type(ep) == TYPE_EXTEND) {
                       get_uname_from_ext_entry(extp, &(uname[i]), TRUE);
                       
                       if (i > UNICODE_NAME_LENGTH) {  /* out-of-range input */
                               err = -EIO;
                               break;
                       }

                       if (extp->entry_offset > EXT_END_MARK) 
                               break;
               } else
                       break;

               i += 13;
       }

       brelse(bh);

       return err;
}

/**
 * Function to extract the unicode name from the extend slots
 * @param dir          inode relating to seeking entry
 * @param entry                entry position of the last extend slot
 * @param bh           buffer head pointer
 * @param ep           the last extend slot as input and the SFN slot as output
 * @return             the number of the extend slots  
 */
static int get_long_name(struct inode *dir, unsigned int entry, struct buffer_head **res_bh, struct rfs_dir_entry **ep, u16 *ext_uname)
{
       struct rfs_ext_entry *extp;
       unsigned char checksum; 
       unsigned int type;
       unsigned int cpos = entry;
       int nr_ext, i;

       /* last index */
       extp = (struct rfs_ext_entry *) (*ep);
       nr_ext = (int) (extp->entry_offset - EXT_END_MARK);
       checksum = extp->checksum;

       i = nr_ext;

       while (i > 0) {
               get_uname_from_ext_entry(extp, 
                       ext_uname + (i - 1) * EXT_UNAME_LENGTH, 
                       (extp->entry_offset > EXT_END_MARK)? TRUE: FALSE);

               *ep = get_entry(dir, ++cpos, res_bh);
               if (IS_ERR(*ep)) {
                       if (PTR_ERR(*ep) == -EFAULT)    /* end-of-directory */
                               return -ENOENT;
                       return PTR_ERR(*ep);
               }

               /* last is SFN directory entry */
               if (i == 1)
                       break;

               /* no extend entries */
               if (entry_type(*ep) != TYPE_EXTEND) {
                       ext_uname[0] = 0x0000;
                       return 0;
               }

               /* orphan LFN entry */
               extp = (struct rfs_ext_entry *) *ep;
               if ((extp->entry_offset > EXT_END_MARK) || 
                               (extp->checksum != checksum))
                       return 0; 

               i--;
       }

       /* SFN entry with extend slots */
       type = entry_type(*ep);

       /* orphan LFN entries upto now */
       if (!(type == TYPE_DIR || type == TYPE_FILE))
               return 0;

       if (checksum != calc_checksum((*ep)->name)) {
               /* no extend entries */
               ext_uname[0] = 0x0000;
               return 0;
       }

       return nr_ext;
}

/**
 * Function check if given file name is exist 
 * @param dir          inode relating to seeking entry
 * @param name         name of file to be sought
 * @param bh           buffer head pointer
 * @param seek_type    entry type to be sought
 * @return     a offset of entry if file name exists, a negative value otherwise.
 *
 * Because of difference in name handling routine, find_entry_long handles only
 * searching with long file name
 */
int find_entry_long (struct inode *dir, const char *name, struct buffer_head **bh, unsigned int seek_type) 
{
       struct rfs_dir_entry *ep;
       struct rfs_ext_entry *extp;
       u16 ext_uname[MAX_TOTAL_LENGTH];
       u16 unicode[MAX_TOTAL_LENGTH];
       char dosname[DOS_NAME_LENGTH + 1];
       int cpos = 0;
       int nr_slot = 0;
       int uni_len = 0, uni_slot = 0;
       unsigned int status = 0;
       unsigned int type;

       if (RFS_I(dir)->start_clu == RFS_SB(dir->i_sb)->root_clu) {
               if (!strcmp(name, ".") || !strcmp(name, ".."))
                       return -INVALID_ENTRY; 
       }

       memset(unicode, 0xff, MAX_TOTAL_LENGTH * sizeof(u16));

       /* uni_len has the length of unicode */
       uni_len = create_fatname(name, dosname, unicode, &status, 
                               RFS_SB(dir->i_sb)->nls_disk, FALSE);
       if (uni_len < 0)
               return uni_len;

       uni_slot = ((uni_len + (EXT_UNAME_LENGTH - 1)) / EXT_UNAME_LENGTH);

       /* scan the directory */
       while(1) {
               ep = get_entry(dir, cpos, bh);
               if (IS_ERR(ep)) {
                       if (PTR_ERR(ep) == -EFAULT)     /* end-of-directory */
                               return -ENOENT;
                       return PTR_ERR(ep);
               }

               type = entry_type(ep);
               if (type == TYPE_UNUSED)        /* end-of-directory */ 
                       return -ENOENT;
               if (type == TYPE_DELETED || type == TYPE_VOLUME) {
                       cpos++;
                       continue;       
               } 
       
               if (type == TYPE_EXTEND) {
                       extp = (struct rfs_ext_entry *) ep;
                       if ((extp->entry_offset < EXT_END_MARK) || (uni_len == 0)) {
                               cpos++;
                               continue;
                       }

                       /* get long name from extend slot iff unicode exist */
                       memset(ext_uname, 0xff, MAX_TOTAL_LENGTH * sizeof(u16));
                       nr_slot = get_long_name(dir, cpos, bh, &ep, ext_uname);
                       if (nr_slot > 0) {
                               /* found LFN slot, SFN slot */
                               cpos += nr_slot;
                       } else if (nr_slot == 0) {
                               /* not found LFN slot */
                               cpos++;
                               continue;
                       } else {
                               /* fail */
                               return nr_slot;
                       }

                       /* compare long name if length is same */
                       if (nr_slot == uni_slot) {
                               if (!memcmp(ext_uname, unicode, (nr_slot * 
                                       EXT_UNAME_LENGTH * sizeof(u16)))) {
                                       if ((seek_type == TYPE_ALL) || 
                                                       (seek_type == type))
                                               goto found;
                               }
                       } 
               }

               if (uni_len == 0) {
                       /* always compare short name */
                       if (!strncmp(dosname, ep->name, DOS_NAME_LENGTH)) {
                               if ((seek_type == TYPE_ALL) || (seek_type == type))
                                       goto found;
                       }
               }
               cpos++;
       }

found: 
       return cpos;
}

#else  /* !CONFIG_RFS_VFAT */

/**
 * Function check if given file name is exist 
 * @param dir          inode relating to seeking entry
 * @param name         file name to be sought   
 * @param bh           buffer head to contain found directry portion
 * @param seek_type    entry type to be sought
 * @return     a offset of entry if file name exists, a negative value otherwise.
 *
 * Because of difference in name handling routine, find_entry_long handles only
 * searching with short file name (8.3 dos name)
 */
int find_entry_short(struct inode *dir, const char *name, struct buffer_head **bh, unsigned int seek_type)
{
       struct rfs_dir_entry *ep;
       char dosname[DOS_NAME_LENGTH];
       int cpos = 0;
       unsigned int type;
       
       if (RFS_I(dir)->start_clu == RFS_SB(dir->i_sb)->root_clu) {
               if (!strcmp(name, ".") || !strcmp(name, ".."))
                       return -INVALID_ENTRY; 
       }

       cpos = create_fatname(name, dosname, NULL, NULL, 
                       RFS_SB(dir->i_sb)->nls_disk, FALSE);
       if (cpos < 0)
               return cpos;

       cpos = 0;
       while (1) {
               ep = get_entry(dir, cpos, bh);
               if (IS_ERR(ep)) {
                       if (PTR_ERR(ep) == -EFAULT)     /* end-of-directory */
                               return -ENOENT;
                       return PTR_ERR(ep);
               }

               type = entry_type(ep);
               if (type == TYPE_FILE || type == TYPE_DIR) {
                       if (!strncmp(dosname, ep->name, DOS_NAME_LENGTH)) {
                               if (seek_type == TYPE_ALL) {
                                       goto found;
                               } else if (seek_type == type) {
                                       goto found;
                               }       
                       }       
               } else if (type == TYPE_UNUSED) {       /* end-of-directory */
                       return -ENOENT;
               }
               cpos++;
       }

found: 
       return cpos;
}
#endif /* CONFIG_RFS_VFAT */

/**
 * Function to remove directy entries 
 * @param dir  inode relating to deteted entry
 * @param entry        position of entry deletion will occur
 * @param bh   buffer head containing directory entry
 * @return     zero on success, negative error code on failure 
 */
int remove_entry (struct inode *dir, unsigned int entry, struct buffer_head **bh)
{
       struct rfs_dir_entry *ep;
       unsigned int numof_entries = 0;
       unsigned int i = 0;
       /* 
        * RFS-log : 21 comes from
        * cell(MAX_NAME_LENGTH / EXT_UNAME_LENGTH) + 1.
        * It means max(extended entries) + original entry
        */
       unsigned char undel_buf[21];
       unsigned char ent_off = 0;

       while (1) {
               ep = get_entry(dir, entry - i, bh);
               if (IS_ERR(ep)) 
                       goto error;

               if ((i != 0) && (entry_type(ep) != TYPE_EXTEND))
                       break;

               if (i == 0) {
                       /* sanity check */
                       if (unlikely((entry_type(ep) != TYPE_FILE) &&
                                       (entry_type(ep) != TYPE_DIR))) {
                               RFS_BUG("dir entry (%d, %d) is corrupted\n",
                                               RFS_I(dir)->start_clu,
                                               entry);
                               return -EIO;
                       }
               }
 
               undel_buf[i] = ep->name[0];
               numof_entries++;
               if (entry - i == 0)
                       break;
               if (entry_type(ep) == TYPE_EXTEND) {
                       ent_off = ((struct rfs_ext_entry *) ep)->entry_offset;
                       if (ent_off > EXT_END_MARK) 
                               break;
               }
               i++;
       }

       /* RFS-log : remove entry */
       if (rfs_log_remove_entry(dir->i_sb, RFS_I(dir)->start_clu, entry, 
                               numof_entries, undel_buf)) {
               return -EIO;
       }

       for (i = 0; i < numof_entries; i++) {
               ep = get_entry(dir, entry - i, bh);
               if (IS_ERR(ep))
                       goto error;

               set_entry_type(ep, TYPE_DELETED);

               if (buffer_uptodate(*bh))
                       rfs_mark_buffer_dirty(*bh, dir->i_sb);
       }

       return 0;

error: 
       return PTR_ERR(ep);
}
