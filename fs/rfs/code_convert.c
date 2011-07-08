/**
 * @file       fs/rfs/code_convert.c
 *
 * @brief      Dos name and Unicode name handling operations  
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
#include <linux/types.h>
#include <linux/string.h>
#include <linux/rfs_fs.h>
#include <linux/ctype.h>
#include <linux/nls.h>

#include "rfs.h"

/**
 * check the invalid character in the short entry
 * this character will be changed to underscore when configured RFS_VFAT
 * otherwise, making a entry with this character fails
 * @param c    dosname character to check
 * @return     zero on valid character
 */
static inline int check_invalid_short(u8 c)
{
       if ((c <= 0x20) || (c == '+') || (c == ',') || (c == ';') || 
                       (c == '=') || (c == '[') || (c == ']'))
               return 1;
       else
               return 0;
}

/**
 * check the invalid character for FAT file name
 * @param c    unicode character to check 
 * @return     zero on valid character
 */
static inline int check_invalid(u16 c)
{
#ifndef CONFIG_RFS_VFAT
       if ((c == '\\') || (c == '/') || (c == ':') || (c == '*') || (c == '?')
               || (c == '\"') || (c == '<') || (c == '>') || (c == '|') 
               || (c <= 0x20) || (c == '+') || (c == ',') || (c == ';') 
               || (c == '=') || (c == '[') || (c == ']'))
#else
       if ((c == '\\') || (c == '/') || (c == ':') || (c == '*') || (c == '?')
               || (c == '\"') || (c == '<') || (c == '>') || (c == '|'))
#endif
               return 1;
       else
               return 0;
}

/**
 * Function converting cstring to dosname
 * @param dosname      resulting dosname
 * @param cstring      cstring to be converted
 * @param status       flag indicating the capital combination and whether resulting dosname fits the 8.3 dosname or not
 * @param check                flag indicating whether check invalid characters or not
 * @return             zero on sucess, negative error code on failure. 
 */
int convert_cstring_to_dosname(u8 *dosname, const char* cstring, unsigned int *status, unsigned int check)
{
       const char *end_of_name, *last_period;
       int i, len, lossy = FALSE;
       char mixed = 0;

       /* strip all leading spaces and periods */
       while ((*cstring == SPACE) || (*cstring == PERIOD)) {
               lossy = TRUE;
               cstring++;
       }
       
       len = strlen(cstring);
       if (!len)
               return -EINVAL;

       end_of_name = cstring + len - 1;

       /* check the trailing period & space */
       if (!check && (*end_of_name == PERIOD || *end_of_name == SPACE))
               return -EINVAL;

       /* search for the last embedded period */
       last_period = strrchr(cstring, PERIOD);
       if (last_period == NULL)
               last_period = end_of_name + 1;

       memset(dosname, SPACE, DOS_NAME_LENGTH);

       i = 0;
       for (;(i < DOS_NAME_LENGTH) && (cstring <= end_of_name); cstring++) {
               if (check && check_invalid((u16)(*cstring & 0x00ff))) {
                       DEBUG(DL3, "Invalid character");
                       return -EINVAL;
               }

               if (*cstring == SPACE) {
                       lossy = TRUE;
                       continue;
               } 

               if (*cstring == PERIOD) {
                       if (cstring < last_period)
                               lossy = TRUE;
                       else
                               i = SHORT_NAME_LENGTH;
                       continue;
               } 

#if !defined(CONFIG_RFS_NLS) && defined(CONFIG_RFS_VFAT)
               /* not support non-ASCII */
               if (check && (*cstring & 0x80)) {
                       DEBUG(DL3, "NLS not support");
                       return -EINVAL;
               }
#endif

               /* fill dosname */
               if (check_invalid_short(*cstring)) {
                       dosname[i++] = UNDERSCORE;
                       lossy = TRUE;
               } else if (isascii(*cstring) && islower(*cstring)) {
                       if (i < SHORT_NAME_LENGTH) 
                               mixed |= PRIMARY_LOWER;
                       else 
                               mixed |= EXTENSION_LOWER;
                       dosname[i++] = toupper(*cstring);
               } else if (isascii(*cstring) && isupper(*cstring)) {
                       if (i < SHORT_NAME_LENGTH) 
                               mixed |= PRIMARY_UPPER;
                       else
                               mixed |= EXTENSION_UPPER;
                       dosname[i++] = *cstring;
               } else {
                       dosname[i++] = *cstring;
               }

               if ((i == SHORT_NAME_LENGTH) && ((cstring + 1) < last_period)) {
                       lossy = TRUE;
#ifdef CONFIG_RFS_VFAT
                       if (check) {
                               while(++cstring < last_period) {
                                       if (check_invalid((u16)(*cstring & 0x0ff)))
                                               return -EINVAL;
                               }
                       }
#endif
                       cstring = last_period;
               }
       } /* end of loop */

       if (cstring <= end_of_name) {
               lossy = TRUE;
#ifdef CONFIG_RFS_VFAT
               if (check) {
                       while(cstring <= end_of_name) {
                               if (check_invalid((u16)(*cstring++ & 0x0ff)))
                                       return -EINVAL;
                       }
               }
#endif
       }

       /* post check */
       if (dosname[0] == KANJI_LEAD) 
               dosname[0] = REPLACE_KANJI;

       if (status != NULL) {
               *status = 0;

               if ((primary_masked(mixed) == (PRIMARY_UPPER | PRIMARY_LOWER)) 
                       || (extension_masked(mixed) == 
                               (EXTENSION_UPPER | EXTENSION_LOWER))) {
                       put_mix(*status, UPPER_N_LOWER);
               } else {
                       if (primary_masked(mixed) == PRIMARY_LOWER)
                               put_mix(*status, PRIMARY_LOWER);
                       if (extension_masked(mixed) == EXTENSION_LOWER) 
                               put_mix(*status, EXTENSION_LOWER);
               }

               put_lossy(*status, lossy);
       }

       return 0;
}

/**
 * Function converting dos name to cstring
 * @param cstring      dosname to be converted
 * @param dosname      resulting cstring
 * @param sysid                flag indicating the capital combination
 * @return     none
 */
void convert_dosname_to_cstring(char *cstring, const u8 *dosname, unsigned char sysid)
{
       int i = 0;

       if (dosname[0] == REPLACE_KANJI) {
               *cstring++ = (s8) KANJI_LEAD;
               i = 1;
       }

       for ( ; i < SHORT_NAME_LENGTH; i++) {
               if (dosname[i] == SPACE)
                       break;

               if (sysid & PRIMARY_LOWER && isascii(dosname[i]))
                       *cstring++ = (s8) tolower(dosname[i]);
               else
                       *cstring++ = (s8) dosname[i];
       }

       /* no extension */
       if (dosname[SHORT_NAME_LENGTH] == SPACE) {
               *cstring = '\0';
               return;
       }

       *cstring++ = PERIOD;

       for (i = SHORT_NAME_LENGTH; i < DOS_NAME_LENGTH; i++) {
               if (dosname[i] == SPACE)
                       break;

               if (sysid & EXTENSION_LOWER && isascii(dosname[i]))
                       *cstring++ = (s8) tolower(dosname[i]);
               else
                       *cstring++ = (s8) dosname[i];
       }

       *cstring = '\0';
       return;
}

#ifdef CONFIG_RFS_VFAT

#ifdef CONFIG_RFS_NLS
/**
 * Function to convert encoded character to unicode by NLS table
 * @param nls          NLS table
 * @param chars                encoded characters
 * @param chars_len    the length of character buffer
 * @param uni          the unicode character converted
 * @param lower                flag indicating the case type of unicode character
 * @return             the length of character converted
 */
static int char2uni(struct nls_table *nls, unsigned char *chars, int chars_len, wchar_t *uni)
{
       int clen = 0;
       wchar_t uni_tmp;

       clen = nls->char2uni(chars, chars_len, &uni_tmp);
       if (clen < 0) {
               *uni = UNDERSCORE;
               clen = 1;
       } else if (clen <= 1) {
               *uni = uni_tmp;
               clen = 1;
       } else {
               *uni = uni_tmp;
       }

       return clen;
}
#endif         /* CONFIG_RFS_NLS */

/**
 * Function converting cstring to unicode name
 * @param uname                unicode name to be converted
 * @param cstring      resulting cstring
 * @param nls          Linux NLS object pointer for future enchancement
 * @param check                flag indicating wether check invalid characters or not
 * @return             the length of uname on success, negative error code on failure
 */
static int convert_cstring_to_uname(u16 *uname, const char *cstring, struct nls_table *nls, unsigned int check) 
{
#ifndef CONFIG_RFS_NLS         /* if not support nls */
       int i = 0;

       while (cstring[i] != 0x00) {
               if (cstring[i] & 0x80) {
                       /* cannot convert to unicode without codepage */
                       DEBUG(DL3, "NLS not support");
                       return -EINVAL;
               }

               uname[i] = (u16) (cstring[i] & 0x00ff);
               i++;
       }

       /* add the null */
       uname[i] = 0x0000;
       return i;
#else
       /* support nls codepage to convert character */
       int clen = 0, uni_len = 0, len;
       int i = 0;

       len = strlen(cstring);

       for (i = 0; (cstring[i] != '\0') && (uni_len < MAX_NAME_LENGTH); uni_len++) {

               clen = char2uni(nls, (unsigned char *)&cstring[i],
                               len - i, &uname[uni_len]);
               i += clen;

               if (uname[uni_len] == 0x00)
                       break;

               if (check && check_invalid(uname[uni_len])) {
                       DEBUG(DL3, "Invalid character");
                       return -EINVAL;
               }
       }

       /* the length of unicode name is never over the limitation */
       {
               uname[uni_len] = 0x0000;

               /* return the length of name */
               return uni_len;
       }
#endif /* CONFIG_RFS_NLS */
}

/**
 * Function converting unicode name to cstring
 * @param cstring      resulting converted
 * @param uname                unicode name to be converted
 * @param nls          Linux NLS object pointer for future use
 * @return             zero on success, negative error code on failure
 */
int convert_uname_to_cstring(char *cstring, const u16 *uname, struct nls_table *nls)
{
#ifndef CONFIG_RFS_NLS         /* if not support nls */
       int i = 0;
       for (; i < MAX_NAME_LENGTH; i++) {
               /* cannot convert */
               if (uname[i] & 0xff80)
                       cstring[i] = UNDERSCORE;
               else 
                       cstring[i] = (s8) uname[i];

               if (cstring[i] == '\0')
                       break;
       }

       /* return the length of unicode name */
       return i;
#else
       /* need nls codepage to convert character */
       int clen = 0, stridx = 0, i = 0;
       const u16 *uname_ptr = uname;
       char buf[NLS_MAX_CHARSET_SIZE];

       for (;(i < MAX_NAME_LENGTH) && (stridx <= NAME_MAX); i++, uname_ptr++) {
               /* end of uname is NULL */
               if (*uname_ptr == 0x0000)
                       break;

               clen = nls->uni2char(*uname_ptr, buf, sizeof(buf));
               if (clen <= 0) {
                       cstring[stridx] = UNDERSCORE;
                       clen = 1;
               } else if (clen == 1) {
                       cstring[stridx] = buf[0];
               } else {
                       if (stridx + clen > NAME_MAX)
                               break;
                       memcpy(&cstring[stridx], buf, clen);
               }

               stridx += clen;
       }

       cstring[stridx] = '\0';

       /* return the length of unicode name */
       return i;
#endif /* CONFIG_RFS_NLS */

}
#endif /* CONFIG_RFS_VFAT */

/**
 * translate encoded string from user to 8.3 dosname and unicode
 *
 * @param cstring      encoded string from user IO
 * @param dosname      resulting dosname of encoded string
 * @param unicode      resulting unicode name
 * @param status       flag indicating the capital combination and whether resulting dosname fits the 8.3 dosname or not
 * @param nls          NLS codepage table
 * @param check                flag whether check invalid characters or not
 * @return             zero or length of uname on success, or errno on failure
 */
int create_fatname(const char *cstring, u8 *dosname, u16 *unicode, unsigned int *status, struct nls_table *nls, unsigned int check)
{
       /*
        * support NLS or not
        */
       int ret = 0;

#ifdef CONFIG_RFS_NLS
       if (unlikely(!nls))     /* out-of-range input */
               return -EINVAL;
#else
       nls = NULL;
#endif /* CONFIG_RFS_NLS */

       if (dosname) {
               /* check trailing space, period */
               ret = convert_cstring_to_dosname(dosname, cstring, status, check);
               if (ret < 0)
                       return ret;
       }

#ifdef CONFIG_RFS_VFAT
       if (unicode && status) { 
               /* 
                * don't check the length of unicode name
                * because it's checked at rfs_lookup
                */

               /* make unicode only if condition is satisfied */
               if (get_lossy(*status) || get_mix(*status) == UPPER_N_LOWER) {
                       /* don't check the length of unicode */
                       ret = convert_cstring_to_uname(unicode, cstring, nls, FALSE);
                       if (ret < 0)
                               return ret;
               } else {
                       unicode[0] = 0x00;
                       return 0;
               }
       }
#endif /* CONFIG_RFS_VFAT */

       return ret;
}

