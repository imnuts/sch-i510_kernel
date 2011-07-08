/**
 *  @file      fs/rfs/namei.c
 *  @brief     Here is adaptation layer between the VFS and RFS filesystem for inode ops
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
#include <linux/sched.h>
 
#include "rfs.h"
#include "log.h"

#include "xattr.h"

#define SLASH  '/'

#define ROOT_MTIME(sb) (sb->s_root->d_inode->i_mtime)

/* structure for rename */
struct rename_info {
       struct inode *old_dir;
       struct inode *old_inode;
       struct inode *new_dir;
       unsigned int *new_index;
       const char *new_name;
};

/**
 *  Function extending a directory on request of saving larger entries that left entries.
 *  @param dir         inode corresponding to extending directory
 *  @param start_clu   return value to contain a start cluster value when this function is used to create new directory
 *  @return    zero on success, if not, negative error code.
 *
 * extend_dir invokes alloc_cluster in it, and assumes that alloc_cluster
 * takes care of not only allocating new cluster but also updating FAT table.
 * In addition, this function is used to create new directory.
 */
static struct buffer_head *extend_dir(struct inode * dir, unsigned int *start_clu) {

       struct super_block *sb = dir->i_sb;
       struct buffer_head *bh, *ret = NULL;
       unsigned int start_block, last_block, cur_block;
       unsigned int new_clu;
       int err;

       if ((RFS_I(dir)->start_clu == RFS_SB(sb)->root_clu) 
                       && !IS_FAT32(RFS_SB(sb))) 
               return ERR_PTR(-ENOSPC);

       if ((dir->i_size + RFS_SB(sb)->cluster_size) > 
                       (MAX_DIR_DENTRY << DENTRY_SIZE_BITS))
               return ERR_PTR(-ENOSPC);

       /* alloc new cluster  */        
       err = alloc_cluster(dir, &new_clu);
       if (err)
               return ERR_PTR(err);

       if (start_clu)
               *start_clu = new_clu;

       /* reset new cluster with zero to initalize directory entry */
       start_block = START_BLOCK(new_clu, sb);
       last_block = start_block + RFS_SB(sb)->blks_per_clu;
       cur_block = start_block;
       do {
               if ((bh = sb_getblk(sb, cur_block))) {
                       memset(bh->b_data, 0x00 , sb->s_blocksize);

#ifdef RFS_FOR_2_6
                       set_buffer_uptodate(bh);
#else
                       mark_buffer_uptodate(bh, 1);
#endif                 
                       rfs_mark_buffer_dirty(bh, sb);
                       if (!ret)
                               ret = bh;
                       else
                               brelse(bh);
               }
       } while (++cur_block < last_block);

       if (!ret)       /* I/O error */
               return ERR_PTR(-EIO);

       /* new inode is also initialized by 0 */
       dir->i_size += RFS_SB(sb)->cluster_size;
       RFS_I(dir)->mmu_private += RFS_SB(sb)->cluster_size;

       return ret;
}

/** 
 * allocate a cluster for new directory 
 * @param inode                new inode
 * @param start_clu    allocated cluster to inode
 * @return             zero on success, or errno
 */
static inline int init_dir_clu(struct inode *inode, unsigned int *start_clu)
{
       struct buffer_head *bh = NULL;
       int ret;

       /* CLU_TAIL is temporary value for cluster allocation */
       RFS_I(inode)->start_clu = CLU_TAIL;
       RFS_I(inode)->mmu_private = 0; /* set 0 for extend_dir() */

       /*
        * RFS-log : setting CLU_TAIL to inode's p_start_clu informs
        * that inode does not have a entry yet
        */
       RFS_I(inode)->p_start_clu = CLU_TAIL;
       bh = extend_dir(inode, start_clu);
       if (IS_ERR(bh))
               return PTR_ERR(bh);

       ret = init_new_dir(inode);

       brelse(bh);
       return ret;
}

/**
 * Function to find proper postion where new entry is stored 
 * @param dir  inode relating to parent directory
 * @param slots        the number of entries to be saved
 * @param bh   buffer head containing directory entry
 * @return     a offset of empty slot of a current directory on sucess, negative value on falure.
 *
 * This function is invoked by upper functions, upper functions are different
 * according the FAT type of current system.
 */
static int find_empty_entry (struct inode *dir, int slots, struct buffer_head **bh) {
       
       struct super_block *sb = dir->i_sb;
       struct buffer_head *new_bh;
       struct rfs_dir_entry *ep;
       unsigned int cpos = 0, free = 0;
       int nr_clus = 0;
       
       while (1) {
               ep = get_entry(dir, cpos, bh);
               if (IS_ERR(ep)) {
                       if (PTR_ERR(ep) == -EFAULT)
                               break;
                       else
                               return PTR_ERR(ep);
               }

               if (IS_FREE(ep->name)) {
                       if (++free == slots)
                               return cpos;
               } else {
                       free = 0;
               }

               cpos++;
       }       

       /* If fail to find requested slots, decide whether to extend directory */
       if ((RFS_I(dir)->start_clu == RFS_SB(sb)->root_clu) &&
           (!IS_FAT32(RFS_SB(sb)))) {
               return -ENOSPC;
       }

       /* calculate the number of cluster */
       nr_clus = (slots - free + 
                       (RFS_SB(sb)->cluster_size >> DENTRY_SIZE_BITS) - 1) /
               (RFS_SB(sb)->cluster_size >> DENTRY_SIZE_BITS);
       if (nr_clus > GET_FREE_CLUS(RFS_SB(sb)))
               return -ENOSPC;

       /* extend entry */
       while (nr_clus-- > 0) {
               new_bh = extend_dir(dir, NULL); 
               if (IS_ERR(new_bh))
                       return PTR_ERR(new_bh);
       }

       while (1) {
               ep = get_entry(dir, cpos, &new_bh);
               if (IS_ERR(ep)) {
                       if (PTR_ERR(ep) == -EFAULT)
                               return -ENOSPC;
                       else
                               return PTR_ERR(ep);
               }
               if (++free == slots)
                       break;
               cpos++;
       } 

       *bh = new_bh;
       /* return accumulated offset regardless of cluster */
       return cpos;
}

#ifndef CONFIG_RFS_VFAT
/**
 *  compare two dos names 
 * @param dentry       dentry to be computed
 * @param a            file name cached in dentry cache
 * @param b            file name to be compared for corresponding dentry
 * @return             return 0 on success, errno on failure
 *
 * If either of the names are invalid, do the standard name comparison
 */

static int rfs_dentry_cmp(struct dentry *dentry, struct qstr *a, struct qstr *b)
{
       u8 a_dosname[DOS_NAME_LENGTH], b_dosname[DOS_NAME_LENGTH];
       char valid_name[NAME_MAX + 1];
       int err = 0;

       if (a->len > NAME_MAX || b->len > NAME_MAX)     /* out-of-range input */
               return -EINVAL;

       memcpy(valid_name, a->name, a->len);
       valid_name[a->len] = '\0';
       err = convert_cstring_to_dosname(a_dosname, valid_name, NULL, FALSE);
       if (err < 0)
               goto out;

       memcpy(valid_name, b->name, b->len);
       valid_name[b->len] = '\0';
       err = convert_cstring_to_dosname(b_dosname, valid_name, NULL, FALSE);
       if (err < 0)
               goto out;
       err = memcmp(a_dosname, b_dosname, DOS_NAME_LENGTH);

out:
       return err;
}

/**
 *  compute the hash for the dos name corresponding to the dentry
 * @param dentry       dentry to be computed
 * @param qstr         parameter to contain resulting hash value
 * @return             return 0
 */
static int rfs_dentry_hash(struct dentry *dentry, struct qstr *qstr)
{
       unsigned char hash_name[DOS_NAME_LENGTH];
       char valid_name[NAME_MAX + 1];
       int err;

       if (qstr->len > NAME_MAX)       /* out-of-range input */
               return 0;

       memcpy(valid_name, qstr->name, qstr->len);
       valid_name[qstr->len] = '\0';
       err = convert_cstring_to_dosname(hash_name, valid_name, NULL, FALSE);
       if (!err)
               qstr->hash = full_name_hash(hash_name, DOS_NAME_LENGTH);

       return 0;
}

struct dentry_operations rfs_dentry_operations = {
       .d_hash         = rfs_dentry_hash,
       .d_compare      = rfs_dentry_cmp,
};

/**
 *  create a dir entry and fill it
 * @param dir          parent dir inode
 * @param inode                inode of new dir entry
 * @param start_clu    start cluster number for itself
 * @param type         entry type
 * @param name         object name 
 * @return             return index(pointer to save dir entry index) on success, errno on failure
 */
int build_entry_short(struct inode *dir, struct inode *inode, unsigned int start_clu, unsigned int type, const char *name)
{
       struct buffer_head *bh = NULL;
       struct rfs_dir_entry *ep = NULL;
       unsigned char dosname[DOS_NAME_LENGTH];
       unsigned char is_mixed = 0;
       unsigned int index;
       int ret = 0;

       /* convert a cstring into dosname */
       ret = mk_dosname(dir, name, dosname, &is_mixed, NULL);
       if (ret < 0) 
               goto out;

       /* find empty dir entry */
       index = find_empty_entry(dir, 1, &bh);
       if ((int) index < 0) {
               ret = index;
               goto out;
       }

       /* allocate a new cluster for directory */
       if (type == TYPE_DIR && inode)
               if ((ret = init_dir_clu(inode, &start_clu)) < 0) 
                       goto out;

       ret = index;
       /* get dir entry pointer */
       ep = get_entry(dir, index, &bh); 
       if (IS_ERR(ep)) {
               ret = PTR_ERR(ep);
               goto out;
       }
       
       /* RFS-log sub trans */
       if (rfs_log_build_entry(dir->i_sb, RFS_I(dir)->start_clu, index, 1)) {
               ret = -EIO;
               goto out;
       }

       /* init dir entry */
       init_dir_entry(dir, ep, type, start_clu, dosname, &is_mixed);

       rfs_mark_buffer_dirty(bh, dir->i_sb);

out:
       brelse(bh);
       return ret;
}

#else  /* CONFIG_RFS_VFAT */

/**
 *  create a dir entry and extend entries 
 * @param dir          parent dir inode
 * @param inode                inode of new dir entry
 * @param start_clu    start cluster number for itself
 * @param type         entry type
 * @param name         object name
 * @return             return index(pointer to save dir entry index) on success, errno on failure
 */
int build_entry_long(struct inode *dir, struct inode *inode, unsigned int start_clu, unsigned int type, const char *name)
{
       struct buffer_head *bh = NULL;
       struct rfs_dir_entry *ep = NULL, *tmp_ep = NULL;
       struct rfs_ext_entry *extp = NULL;
       unsigned char checksum;
       unsigned short uname[UNICODE_NAME_LENGTH];
       unsigned char dosname[DOS_NAME_LENGTH];
       unsigned int num_entries, i;
       unsigned char is_mixed = 0;
       unsigned int index;
       int ret = 0;

       /* 
        * convert a cstring into dosname & 
        * return the count of needed extend slots 
        */
       if ((ret = mk_dosname(dir, name, dosname, &is_mixed, uname)) < 0)
               return ret;

       /* ret has the number of the extend slot */
       num_entries = ret + 1;

       /* find empty dir entry */
       index = find_empty_entry(dir, num_entries, &bh);
       if ((int) index < 0) {
               ret = index;
               goto out;
       }

       /* allocate a new cluster for directory */
       if (type == TYPE_DIR && inode)
               if ((ret = init_dir_clu(inode, &start_clu)) < 0) 
                       goto out;

       ret = index;
       /* get dir entry pointer */
       ep = get_entry(dir, index, &bh);
       if (IS_ERR(ep)) {
               ret = PTR_ERR(ep);
               goto out;
       }

       /* RFS-log sub trans */
       if (rfs_log_build_entry(dir->i_sb, RFS_I(dir)->start_clu, index,
                             num_entries)) {
               ret = -EIO;
               goto out;
       }

       /* init dir entry */
       init_dir_entry(dir, ep, type, start_clu, dosname, &is_mixed);
       
       rfs_mark_buffer_dirty(bh, dir->i_sb);

       /* only have dos entry */
       if (num_entries == 1)
               goto out;

       checksum = calc_checksum(dosname);

       /* init extend entries */
       for (i = 1; i < num_entries; i++) {
               tmp_ep = get_entry(dir, index - i, &bh);
               if (IS_ERR(tmp_ep)) {
                       ret = PTR_ERR(ep);
                       goto out;
               }
               extp = (struct rfs_ext_entry *)tmp_ep;
               if (init_ext_entry(extp, TYPE_EXTEND, 
                               ((i == (num_entries - 1))? i + EXT_END_MARK: i),
                               &(uname[EXT_UNAME_LENGTH * (i - 1)]), 
                               checksum) < 0) { /* out-of-range input */
                       ret = -EIO;
                       goto out;
               }

               rfs_mark_buffer_dirty(bh, dir->i_sb);
       }

out:
       brelse(bh);

       return ret;     
}

#endif /* !CONFIG_RFS_VFAT */

/**
 * find inode for dir entry and return
 * @param sb           super block
 * @param ino          inode number of dir entry
 * @param p_start_clu  parent's start cluster
 * @param index                dir entry's offset in parent dir
 * @param ep           dir entry
 * @return             inode of dir entry on success or errno on failure
 */
static inline struct inode *rfs_iget(struct super_block *sb, unsigned long ino, unsigned int p_start_clu, unsigned int index, struct rfs_dir_entry *ep)
{
#ifndef CONFIG_RFS_IGET4
       struct inode *inode = iget_locked(sb, ino);

       if (inode && (inode->i_state & I_NEW)) {
               fill_inode(inode, ep, p_start_clu, index);
               unlock_new_inode(inode);
       }
#else
       struct inode *inode;
       struct rfs_iget4_args args;

       args.ep = ep;
       args.p_start_clu = p_start_clu;
       args.index = index;

       /* 
        * Some kernel version(under Linux kernel version 2.4.25) does not 
        * support iget_locked.
        */
       inode = iget4(sb, ino, NULL, (void *) (&args));
#endif

       return inode;
}

/**
 *  find dir entry for inode in parent dir and return a dir entry & entry index
 * @param dir          parent dir inode
 * @param inode                inode for itself
 * @param bh           buffer head included dir entry for inode
 * @param name         object name to search
 * @param type         dir entry type
 * @return             return dir entry on success, errno on failure
 */
static struct rfs_dir_entry *search_entry(struct inode *dir, struct inode *inode, struct buffer_head **bh, const char *name, unsigned char type)
{
       if (!inode)     /* out-of-range input */
               return ERR_PTR(-EINVAL);

       CHECK_RFS_INODE(inode, ERR_PTR(-EINVAL));

       return get_entry(dir, RFS_I(inode)->index, bh);
}

/**
 *  lookup inode associated with dentry
 * @param dir          inode of parent directory
 * @param dentry       dentry for itself
 * @param nd           namei data structure
 * @return             return dentry object on success, errno on failure
 *
 * if inode doesn't exist, allocate new inode, fill it, and associated with dentry
 */
#ifdef RFS_FOR_2_6
static struct dentry *rfs_lookup(struct inode *dir, struct dentry *dentry, struct nameidata *nd)
#else
static struct dentry *rfs_lookup(struct inode *dir, struct dentry *dentry)
#endif 
{
       struct inode *inode = NULL;
       struct buffer_head *bh = NULL;
       struct rfs_dir_entry *ep = NULL;
       unsigned int index;
       unsigned long ino;
       int ret = 0;

       /* check the name length */
       if (dentry->d_name.len > NAME_MAX)
               return ERR_PTR(-ENAMETOOLONG);

#ifndef CONFIG_RFS_VFAT 
       dentry->d_op = &rfs_dentry_operations;
#endif

       /* find dir entry */
       ret = find_entry(dir, dentry->d_name.name, &bh, TYPE_ALL);
       if (ret < 0) {
               if (ret == -ENOENT)
                       goto add;
               if ((ret != -EINVAL) && (ret != -ENAMETOOLONG))
                       ret = -EIO;
               goto out;
       }
       index = ret;

       ep = get_entry(dir, index, &bh);
       if (IS_ERR(ep)) {
               ret = -EIO;
               goto out;
       }

       /* get unique inode number */
       ret = rfs_iunique(dir, index, &ino);
       if (ret)
               goto out;

       inode = rfs_iget(dir->i_sb, ino, RFS_I(dir)->start_clu, index, ep);
       if (!inode) {
               ret = -EACCES;
               goto out;
       }

#ifdef CONFIG_RFS_VFAT 
       do {
               struct dentry *alias = NULL;

               alias = d_find_alias(inode);
               if (alias) {
                       if (d_invalidate(alias) == 0) {
                               dput(alias);
                       } else {
                               iput(inode);
                               brelse(bh);
                               return alias;
                       }
               }
       } while (0);
#endif
add:
       brelse(bh);
#ifdef RFS_FOR_2_6
       return d_splice_alias(inode, dentry);
#else  
       d_add(dentry, inode);
       return NULL;
#endif
out:
       brelse(bh);
       return ERR_PTR(ret);
}

/**
 *  create a new file
 * @param dir          inode of parent directory
 * @param dentry       dentry corresponding with a file will be created
 * @param mode         mode
 * @param nd           namei data structure
 * @return             return 0 on success, errno on failure    
 */
#ifdef RFS_FOR_2_6
static int rfs_create(struct inode *dir, struct dentry *dentry, int mode, struct nameidata *nd)
#else
static int rfs_create(struct inode *dir, struct dentry *dentry, int mode)
#endif
{
       struct inode *inode = NULL;
       int ret;

       /* check the validity */
       if (IS_RDONLY(dir))
               return -EROFS;

       if (rfs_log_start(dir->i_sb, RFS_LOG_CREATE, dir))
               return -EIO;

       /* create a new inode */
       inode = rfs_new_inode(dir, dentry, TYPE_FILE);
       if (IS_ERR(inode))
               ret = PTR_ERR(inode);
       else
               ret = 0;

       /* If failed, RFS-log can't rollback due to media error */
       if (rfs_log_end(dir->i_sb, ret))
               return -EIO; 

       if (!ret) /* attach inode to dentry */
               d_instantiate(dentry, inode); 
                                                                                       return ret;
}

/**
 *  check whether a file of inode has start cluster number of RFS reserved files
 * @param inode        inode of checked file
 * @param name         name of checked file
 * @return     return 0 if inode has different start cluster number, else return errno
 *
 * special files for fast unlink & logging are reserved files
 */
int check_reserved_files(struct inode *inode, const char *name)
{
       if (inode) {
               struct super_block *sb = inode->i_sb;
               unsigned int cluster = RFS_I(inode)->start_clu;

               if (cluster == RFS_POOL_I(sb)->start_cluster)
                       return -EPERM;
               if (cluster == RFS_LOG_I(sb)->start_cluster)
                       return -EPERM;
       } else if (name) {
               int len = strlen(name);

               if ((len != RFS_LOG_FILE_LEN) || (len != RFS_POOL_FILE_LEN))
                       return 0;

               if (!strncmp(name, RFS_LOG_FILE_NAME, RFS_LOG_FILE_LEN))
                       return -EPERM;

               if (!strncmp(name, RFS_POOL_FILE_NAME, RFS_POOL_FILE_LEN))
                       return -EPERM;
       }
       
       return 0;
}

/**
 *  remove a file
 * @param dir          parent directory inode
 * @param dentry       dentry corresponding with a file will be removed
 * @return             return 0 on success, errno on failure    
 */
static int rfs_unlink(struct inode *dir, struct dentry *dentry)
{
       struct inode *inode = dentry->d_inode;
       struct buffer_head *bh = NULL;
       struct rfs_dir_entry *ep = NULL;
       int ret;

       /* check the validity */
       if (IS_RDONLY(dir))
               return -EROFS;

       /* check the name length */
       if (dentry->d_name.len > NAME_MAX)
               return -ENAMETOOLONG;

       /* check the system files */
       if (check_reserved_files(inode, NULL))
               return -EPERM;

       /* find dir entry */
       ep = search_entry(dir, inode, &bh, dentry->d_name.name, TYPE_FILE);
       if (IS_ERR(ep)) {
               brelse(bh);
               return PTR_ERR(ep);
       }

       if ((ep->attr & ATTR_READONLY) && (!capable(CAP_SYS_ADMIN))) {
               brelse(bh);
               return -EPERM;
       }
       brelse(bh);

       /* RFS-log : start unlink */
       if (rfs_log_start(dir->i_sb, RFS_LOG_UNLINK, dir))
               return -EIO;

       /* remove dir entry and dealloc all clusters for inode allocated */
       ret = rfs_delete_entry(dir, inode);

       /* If failed, RFS-log can't rollback due to media error */
       if (rfs_log_end(dir->i_sb, ret))
               return -EIO;

       return ret;
}

/**
 *  create a new directory
 * @param dir          parent directory inode
 * @param dentry       dentry corresponding with a directory will be created
 * @param mode         mode
 * @return             return 0 on success, errno on failure    
 */
static int rfs_mkdir(struct inode *dir, struct dentry *dentry, int mode)
{
       struct inode *inode = NULL;
       int ret;

       /* check the validity */
       if (IS_RDONLY(dir))
               return -EROFS;

       /* RFS-log : start mkdir */
       if (rfs_log_start(dir->i_sb, RFS_LOG_CREATE, dir))
               return -EIO;

       /* create a new inode */
       inode = rfs_new_inode(dir, dentry, TYPE_DIR);
       if (IS_ERR(inode))
               ret = PTR_ERR(inode);
       else
               ret = 0;

       /* If failed, RFS-log can't rollback due to media error */
       if (rfs_log_end(dir->i_sb, ret))
               return -EIO; 

       if (!ret) /* attach inode to dentry */
               d_instantiate(dentry, inode); 

       return ret;
}

/**
 *  remove a directory
 * @param dir          parent directory inode
 * @param dentry       dentry corresponding with a directory will be removed 
 * @return             return 0 on success, errno on failure    
 */
static int rfs_rmdir(struct inode *dir, struct dentry *dentry)
{
       struct inode *inode = dentry->d_inode;
       struct buffer_head *bh = NULL;
       struct rfs_dir_entry *ep = NULL;
       int ret;

       /* check the validity */
       if (IS_RDONLY(dir))
               return -EROFS;

       /* check the name length */
       if (dentry->d_name.len > NAME_MAX)
               return -ENAMETOOLONG;

       /* find dir entry */
       ep = search_entry(dir, inode, &bh, dentry->d_name.name, TYPE_DIR);
       if (IS_ERR(ep)) {
               ret = PTR_ERR(ep);
               brelse(bh);
               return ret;
       }
       brelse(bh);

       if (((dir->i_ino == ROOT_INO) && (RFS_I(inode)->index < 0)) || 
                       ((dir->i_ino != ROOT_INO) && (RFS_I(inode)->index < 2)))
               return -ENOENT;

       /* check whether directory is empty */
       ret = is_dir_empty(inode);
       if (ret)
               return ret;

       /* RFS-log : start rmdir */
       if (rfs_log_start(dir->i_sb, RFS_LOG_UNLINK, dir))
               return -EIO;

       /* remove directory */
       ret = rfs_delete_entry(dir, inode);

       /* If failed, RFS-log can't rollback due to media error */
       if (rfs_log_end(dir->i_sb, ret))
               return -EIO;

       return ret;
}

/**
 *  change a directory
 * @param r_info       argument to move
 * @return             return 0 on success, errno on failure    
 */
static int move_to_dir(struct rename_info *r_info)
{
       struct super_block *sb = r_info->old_dir->i_sb;
       unsigned int old_index = RFS_I(r_info->old_inode)->index;
       unsigned int *new_index = r_info->new_index;
       struct buffer_head *old_bh = NULL;
       struct buffer_head *new_bh = NULL;
       struct rfs_dir_entry *old_ep = NULL;
       struct rfs_dir_entry *new_ep = NULL;
       unsigned int old_type;
       int ret = 0;

       old_ep = get_entry(r_info->old_dir, old_index, &old_bh);
       if (IS_ERR(old_ep)) {
               ret = PTR_ERR(old_ep);
               goto out;
       }

       old_type = entry_type(old_ep);
       if (old_type == TYPE_DIR || old_type == TYPE_FILE ||
                       old_type == TYPE_SYMLINK)
               ret = build_entry(r_info->new_dir, NULL, START_CLUSTER(old_ep), 
                               old_type, r_info->new_name);
       else
               ret = -EINVAL;  /* out-of-range input */
       if (ret < 0) 
               goto out;

       *new_index = (unsigned int) ret;

       /* update mtime of new entry */
       new_ep = get_entry(r_info->new_dir, *new_index, &new_bh);
       if (IS_ERR(new_ep)) {
               ret = PTR_ERR(new_ep);
               goto out;
       }

       SET16(new_ep->mtime, GET16(old_ep->mtime));
       SET16(new_ep->mdate, GET16(old_ep->mdate));
       SET32(new_ep->size, GET32(old_ep->size));

       rfs_mark_buffer_dirty(new_bh, sb);

       if (old_type == TYPE_DIR) {
               /* change pointer of parent dir */
               old_ep = get_entry(r_info->old_inode, 1, &old_bh);
               if (IS_ERR(old_ep)) {
                       ret = PTR_ERR(old_ep);
                       goto out;
               }
               if (ROOT_INO != r_info->new_dir->i_ino) {

                       new_ep = get_entry(r_info->new_dir, 0, &new_bh);
                       if (IS_ERR(new_ep)) {
                               ret = PTR_ERR(new_ep);
                               goto out;
                       }
                       memcpy(old_ep, new_ep, sizeof(struct rfs_dir_entry));
                       memcpy(old_ep->name, DOTDOT, DOS_NAME_LENGTH);
               } else {
                       SET16(old_ep->start_clu_lo, RFS_SB(sb)->root_clu);
                       SET16(old_ep->start_clu_hi, RFS_SB(sb)->root_clu >> 16);
                       set_entry_time(old_ep, ROOT_MTIME(sb));
               }
               rfs_mark_buffer_dirty(old_bh, sb);
       }

       /* delete old entry */
       ret = remove_entry(r_info->old_dir, old_index, &old_bh);
out:
       brelse(new_bh);
       brelse(old_bh);
       return ret;
}

/**
 *  change name or location of a file or directory
 * @param old_dir      old parent directory
 * @param old_dentry   old dentry
 * @param new_dir      new parent directory
 * @param new_dentry   new dentry
 * @return             return 0 on success, errno on failure    
 */
static int rfs_rename(struct inode *old_dir, struct dentry *old_dentry, struct inode *new_dir, struct dentry *new_dentry)
{
       struct inode *old_inode = old_dentry->d_inode;
       struct inode *new_inode = new_dentry->d_inode;
       struct qstr *old_qname = &old_dentry->d_name;
       struct qstr *new_qname = &new_dentry->d_name;
       struct buffer_head *old_bh = NULL, *new_bh = NULL;
       struct rfs_dir_entry *old_ep = NULL, *new_ep = NULL;
       struct rename_info r_info;
       unsigned long new_ino;
       unsigned int new_index = 0;
       int is_exist = FALSE;
       int ret;

       /* check the validity */
       if (IS_RDONLY(old_dir))
               return -EROFS;

       /* check the name length */
       if ((old_qname->len > NAME_MAX) ||
                       (new_qname->len > NAME_MAX))
               return -ENAMETOOLONG;

       /* find old dir entry */
       old_ep = search_entry(old_dir, old_inode, &old_bh, 
                               old_qname->name, TYPE_ALL);
       if (IS_ERR(old_ep)) {
               ret = PTR_ERR(old_ep);
               brelse(old_bh);
               return ret;
       }

       if (old_ep->attr & ATTR_READONLY) {
               brelse(old_bh);
               return -EPERM;
       }

       brelse(old_bh);

       /* check permission */
       if (check_reserved_files(old_inode, NULL))
               return -EPERM;

       /* if new_inode is NULL, compare name */
       if (check_reserved_files(new_inode, new_qname->name))
               return -EPERM;
       
       if ((old_dir->i_ino != ROOT_INO) && (RFS_I(old_inode)->index < 2)) 
               return -EINVAL;

       /* find new dir entry if exists, remove it */
       new_ep = search_entry(new_dir, new_inode, &new_bh, 
                               new_qname->name, TYPE_ALL);
       if (!IS_ERR(new_ep)) {
               if (S_ISDIR(new_inode->i_mode)) {
                       ret = is_dir_empty(new_inode);
                       if (ret) {
                               brelse(new_bh);
                               return ret;
                       }
               }
               /* mark is_exist for later deletion */
               is_exist = TRUE;
       }
       brelse(new_bh);

       /* fill rename info */
       r_info.old_dir = old_dir;
       r_info.old_inode = old_inode;
       r_info.new_dir = new_dir;
       r_info.new_index = &new_index;
       r_info.new_name = new_qname->name;

       /* RFS-log : start rename */
       if (rfs_log_start(old_dir->i_sb, RFS_LOG_RENAME, old_inode))
               return -EIO;

       ret = move_to_dir(&r_info);
       if (ret) 
               goto end_log;

       /* delete destination, if exist */
       if (is_exist == TRUE) {
               ret = rfs_delete_entry(new_dir, new_inode);
               if (ret) 
                       goto end_log;
       }

       /* update inode fields */
       ret = rfs_iunique(new_dir, new_index, &new_ino);
       if (ret)
               goto end_log;

       remove_inode_hash(old_inode);

       /*
        * don't change the order of statements for
        * synchronization with rfs_sync_inode() in locked area
        */
       spin_lock(&RFS_I(old_inode)->write_lock);
       old_inode->i_ino = new_ino;
       RFS_I(old_inode)->p_start_clu = RFS_I(new_dir)->start_clu;
       RFS_I(old_inode)->index = new_index;
       spin_unlock(&RFS_I(old_inode)->write_lock);

       insert_inode_hash(old_inode);
       /* 
        * we already reigsterd the inode to the transaction
        * in the above move_to_dir() 
        */
       mark_inode_dirty(old_inode);

       if (old_dir != new_dir) {
               if (S_ISDIR(old_inode->i_mode)) { 
                       old_dir->i_nlink--;
                       new_dir->i_nlink++;
               }
               new_dir->i_mtime = CURRENT_TIME;
               rfs_mark_inode_dirty(new_dir);
       }

       old_dir->i_mtime = CURRENT_TIME;
       rfs_mark_inode_dirty(old_dir);

end_log:
       /* If failed, RFS-log can't rollback due to media error */
       if (rfs_log_end(old_dir->i_sb, ret))
               return -EIO;

       return ret;
}

/**
 *  generate a symlink file
 * @param dir          parent directory inode
 * @param dentry       dentry corresponding with new symlink file
 * @param target       full link of target
 * @return             return 0 on success, errno on failure    
 */
static int rfs_symlink(struct inode *dir, struct dentry *dentry, const char *target)
{
       struct inode *inode = NULL;
       int len = strlen(target) + 1;
       int ret;
       const char *target_last;

       /* check the validity */
       if (IS_RDONLY(dir))
               return -EROFS;

       /* check the system files */
       target_last = strrchr(target, SLASH);
       if (target_last == NULL)
               target_last = target;
       else
               target_last++;

       if (check_reserved_files(NULL, target_last))
               return -EPERM;

       /* RFS-log : start create-symlink */
       if (rfs_log_start(dir->i_sb, RFS_LOG_SYMLINK, dir))
               return -EIO;

       /* create a symlink file */
       inode = rfs_new_inode(dir, dentry, TYPE_SYMLINK);
       if (IS_ERR(inode)) {
               ret = PTR_ERR(inode);
               goto end_log;
       } 
               
#ifdef RFS_FOR_2_6
       ret = page_symlink(inode, target, len);
#else  
       ret = block_symlink(inode, target, len);
#endif
       if (ret == -ENOSPC) {
               rfs_delete_entry(dir, inode);
               iput(inode);
       }

end_log:
       /* for transaction sync, we remember the inode of symbolic link. */
       RFS_LOG_I(dir->i_sb)->symlink_inode = (ret) ? NULL : inode;

       /* If failed, RFS-log can't rollback due to media error */
       if (rfs_log_end(dir->i_sb, ret))
               return -EIO; 

       if (!ret) /* attach inode to dentry */
               d_instantiate(dentry, inode); 

       return ret;
}

struct inode_operations rfs_dir_inode_operations = {
       .create         = rfs_create,
       .lookup         = rfs_lookup,
       .unlink         = rfs_unlink,
       .symlink        = rfs_symlink,
       .mkdir          = rfs_mkdir,
       .rmdir          = rfs_rmdir,
       .rename         = rfs_rename,
       .permission     = rfs_permission,
       .setattr        = rfs_setattr,
#ifdef CONFIG_RFS_FS_XATTR
       .setxattr       = rfs_xattr_set,
       .getxattr       = rfs_xattr_get,
       .listxattr      = rfs_xattr_list,
       .removexattr    = rfs_xattr_delete,
#endif
//     .check_acl      = generic_check_acl,
};

