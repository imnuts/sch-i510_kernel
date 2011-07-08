/**
 * @file       fs/rfs/dir.c
 * @brief      directory handling functions 
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

#include <linux/fs.h>
#include <linux/rfs_fs.h>

#include "rfs.h"

#ifdef CONFIG_GCOV_PROFILE
#define        loff_t          off_t
#endif

/* internal data structure for readdir */
struct rfs_dir_info {
       unsigned int type;
       unsigned long ino;
       char name[NAME_MAX + 1];
};

/**
 *  initialize the dot(.) and dotdot(..) dir entries for new directory
 * @param inode                inode for itself
 * @return             return 0 on success, EIO on failure
 *
 * inode must have start cluster of itself and start cluster of parent dir
 */
int init_new_dir(struct inode *inode)
{
       struct buffer_head *bh = NULL;
               struct rfs_dir_entry *dot_ep = NULL;
               struct rfs_dir_entry *dotdot_ep = NULL;
       unsigned char dummy = 0;
       int err = 0;

       /* initialize .(itself) and ..(parent) */
       dot_ep = get_entry(inode, 0, &bh);
       if (IS_ERR(dot_ep)) {
               brelse(bh);
               return -EIO;
       }

       dotdot_ep = (struct rfs_dir_entry *) (bh->b_data + DENTRY_SIZE);

       init_dir_entry(inode, dot_ep, TYPE_DIR, 
                       RFS_I(inode)->start_clu, DOT, &dummy);
       init_dir_entry(inode, dotdot_ep, TYPE_DIR, 
                       RFS_I(inode)->p_start_clu, DOTDOT, &dummy);

       rfs_mark_buffer_dirty(bh, inode->i_sb);
       brelse(bh);

       return err;
}

/**
 *  check whether directory is emtpy
 * @param dir  inode corresponding to the directory
 * @return     return 0 on success, errno on failure
 *
 * is_dir_empty is usually invoked before removing or renaming directry.
 */
int is_dir_empty(struct inode *dir) {

       struct buffer_head *bh = NULL;
       struct rfs_dir_entry *ep;
       unsigned int cpos = 0;
       int err = 0, count = 0;
       unsigned int type;

       if (dir->i_ino == ROOT_INO)
               return -ENOTEMPTY;

       while (1) {
               ep = get_entry(dir, cpos++, &bh);
               if (IS_ERR(ep)) {
                       err = PTR_ERR(ep);
                       if (err == -EFAULT)
                               err = 0;
                       goto out;
               }

               type = entry_type(ep);
               if ((type == TYPE_FILE) || (type == TYPE_DIR)) {
                       /* check entry index bigger than 
                          entry index of parent directory (..) */
                       if (++count > 2) {
                               err = -ENOTEMPTY;
                               goto out;
                       }
               } else if (type == TYPE_UNUSED) {
                       /* do not need checking anymore */
                       goto out;
               }
       }

out :
       brelse(bh);
       return err; 
}

/**
 *  counts the number of sub-directories in specified directory 
 * @param sb   super block
 * @param clu  start cluster number of specified directory to count
 * @return     return the number of sub-directories on sucess, errno on failure
 */
int count_subdir(struct super_block *sb, unsigned int clu)
{
       struct buffer_head *bh = NULL;
       struct rfs_dir_entry *ep = NULL;
       unsigned int cpos = 0;
       unsigned int type;
       int count = 0, err = 0;

       while (1) { 
               ep = get_entry_with_cluster(sb, clu, cpos++, &bh);
               if (IS_ERR(ep)) {
                       err = PTR_ERR(ep);
                       if (err == -EFAULT) /* end of cluster */
                               break;
                       brelse(bh);
                       return err;
               }

               /* check type of dir entry */
               type = entry_type(ep);
               if (type == TYPE_DIR)
                       count++;
               else if (type == TYPE_UNUSED)
                       break;
       }

       brelse(bh);
       return count;
}

/**
 *  read dir entry in specified directory
 * @param inode                specified directory inode
 * @param bh           buffer head to read dir entries
 * @param ppos         entry position to read
 * @param[out] dir_info        to save dir entry info
 * @return             return 0 on success, errno on failure
 */
static int internal_readdir(struct inode *inode, struct buffer_head **bh, loff_t *ppos, struct rfs_dir_info *dir_info)
{
#ifdef CONFIG_RFS_VFAT
       unsigned short uname[UNICODE_NAME_LENGTH];
#endif
       struct rfs_dir_entry *ep = NULL;
       loff_t index = *ppos;
       unsigned long ino;
       unsigned int type;
       int err;

       while (1) {
               ep = get_entry(inode, (u32) index, bh);
               if (IS_ERR(ep)) 
                       return PTR_ERR(ep);

               index++;

               type = entry_type(ep);

               dir_info->type = type;

               if (type == TYPE_UNUSED) 
                       return -INTERNAL_EOF; /* not error case */

               if ((type == TYPE_DELETED) || (type == TYPE_EXTEND) || 
                               (type == TYPE_VOLUME))
                       continue;

#ifdef CONFIG_RFS_VFAT
               uname[0] = 0x0;
               get_uname_from_entry(inode, index - 1, uname);
               if (uname[0] == 0x0 || !IS_VFAT(RFS_SB(inode->i_sb))) 
                       convert_dosname_to_cstring(dir_info->name, ep->name, ep->sysid);        
               else 
                       convert_uname_to_cstring(dir_info->name, uname, RFS_SB(inode->i_sb)->nls_disk);
#else
               convert_dosname_to_cstring(dir_info->name, ep->name, ep->sysid);
#endif

               err = rfs_iunique(inode, index - 1, &ino);
               if (err)
                       return err;
               dir_info->ino = ino;

               *ppos = index;
                       
               return 0;

       }

       return 0;
}

/**
 *  read all dir entries in specified directory
 * @param filp         file pointer of specified directory to read     
 * @param dirent       buffer pointer
 * @param filldir      function pointer which fills dir info
 * @return             return 0 on success, errno on failure
 */
static int rfs_readdir(struct file *filp, void *dirent, filldir_t filldir)
{
       struct dentry *dentry = filp->f_dentry;
       struct inode *inode = dentry->d_inode;
       struct buffer_head *bh = NULL;
       struct rfs_dir_info dir_info;
       unsigned int type;
       loff_t pos;
       int ret;

       CHECK_RFS_INODE(inode, -ENOENT);

       while (1) {
               pos = filp->f_pos;
       
               ret = internal_readdir(inode, &bh, (loff_t *) &filp->f_pos, &dir_info);
               if (ret < 0) 
                       break;

               if (dir_info.type == TYPE_DIR)
                       type = DT_DIR;
               else
                       type = DT_REG;

               ret = filldir(dirent, dir_info.name, strlen(dir_info.name), 
                               pos, dir_info.ino, type);       
               if (ret < 0) {
                       filp->f_pos = pos; /* rollback */
                       break;
               }
       }
       
       brelse(bh);
       return 0;
}

struct file_operations rfs_dir_operations = {
       .read           = generic_read_dir,
       .readdir        = rfs_readdir,
};
