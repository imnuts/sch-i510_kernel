/**
 *  @file       fs/rfs/super.c
 *  @brief      super block and init functions
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
#include <linux/bitops.h>
#include <linux/blkdev.h>
#include <linux/rfs_fs.h>
#include <linux/nls.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
#include <linux/parser.h>
#include <linux/writeback.h>
#include <linux/statfs.h>
#endif

#include "rfs.h"
#include "log.h"

#ifdef RFS_FOR_2_6
enum {
       opt_codepage, opt_acl, opt_noacl, opt_vfat, opt_xattr, opt_noxattr, opt_err,
};

static match_table_t rfs_tokens = {
       {opt_codepage, "codepage=%s"},
       {opt_acl, "acl"},
       {opt_noacl, "noacl"},
       {opt_vfat, "vfat"},
       {opt_xattr, "xattr"},
       {opt_noxattr, "noxattr"},
       {opt_err, NULL}
};
#endif

#ifdef CONFIG_RFS_NLS
static const char rfs_default_codepage[] = CONFIG_RFS_DEFAULT_CODEPAGE;
#endif

/**
 *  write the super block especially commit the previous transaction and flush the fat cache 
 * @param sb   super block
 */
static void rfs_write_super(struct super_block *sb)
{
       if ((sb->s_flags & MS_RDONLY)) 
               return;

       rfs_log_force_commit(sb, NULL);

       sb->s_dirt = 0;
}

/**
 *  get statistics on a file system 
 * @param sb   super block
 * @param stat structure to fill stat info
 * @return     return 0
 */
#ifdef RFS_FOR_2_6_18
static int rfs_statfs(struct dentry *dentry, struct kstatfs *stat)
#elif RFS_FOR_2_6
static int rfs_statfs(struct super_block *sb, struct kstatfs *stat)
#else
static int rfs_statfs(struct super_block *sb, struct statfs *stat)
#endif
{
#ifdef RFS_FOR_2_6_18
       struct super_block *sb = dentry->d_sb;
#endif
       int used_clusters = RFS_SB(sb)->num_used_clusters;      
       
       stat->f_type = RFS_MAGIC;
       stat->f_bsize = RFS_SB(sb)->cluster_size;
       stat->f_blocks = RFS_SB(sb)->num_clusters;
       stat->f_bfree = RFS_SB(sb)->num_clusters - used_clusters;
       stat->f_bavail = stat->f_bfree;
       stat->f_namelen = MAX_NAME_LENGTH;

       return 0;
}

/**
 * allow to remount to make a writable file system readonly
 * @param sb   super block
 * @param flags        to chang the mount flags
 * @param data private data
 * @return     return 0
 */
static int rfs_remount(struct super_block *sb, int *flags, char *data)
{
       if ((*flags & MS_RDONLY) != (sb->s_flags & MS_RDONLY)) {
               if (*flags & MS_RDONLY) {
                       sb->s_flags |= MS_RDONLY;
                       *flags |= MS_RDONLY;
               }
       } else {
               if (!(sb->s_flags & MS_RDONLY)) {
                       sb->s_flags &= ~MS_RDONLY;
                       *flags &= ~MS_RDONLY;
               }
       }

       *flags |= MS_NOATIME | MS_NODIRATIME;

        return 0;
}

/**
 *  release the super block
 * @param sb   super block
 * 
 * release log, internal fat cache, and the pool file memory
 */
static void rfs_put_super(struct super_block *sb)
{
#ifdef RFS_FOR_2_6
       struct rfs_sb_info *sbi = RFS_SB(sb);
#endif

       /* It precedes rfs_fcache_release because
          it enventually calls rfs_fcache_sync */
       rfs_log_cleanup(sb);

       rfs_fcache_release(sb);

       rfs_release_pool(sb);
       
       kfree(RFS_SB(sb)->fat_mutex);

#ifdef RFS_FOR_2_6
       if (!sbi) {
               RFS_BUG("rfs-specific sb is corrrupted\n");
               return;
       }
               
       sb->s_fs_info = NULL;
       kfree(sbi);
#endif
       return;
}

static struct super_operations rfs_sops = {
#ifdef RFS_FOR_2_6
       .alloc_inode    = rfs_alloc_inode,
       .destroy_inode  = rfs_destroy_inode,
#endif
#ifdef CONFIG_RFS_IGET4
       .read_inode2    = rfs_read_inode2,
#endif
       .write_inode    = rfs_write_inode,
       .delete_inode   = rfs_delete_inode,
       .put_super      = rfs_put_super,
       .write_super    = rfs_write_super,
       .statfs         = rfs_statfs,
       .remount_fs     = rfs_remount,
};

/**
 *  get the partition boot record of the super block 
 * @param sb                   super block
 * @param[out] res             buffer head contains pbr info which will be released by caller
 * @param[out] pbr_sector      number of pbr sector
 * @return                     return 0 on success, errno on failure 
 */
static int get_pbr_info(struct super_block *sb, struct buffer_head **res, unsigned int *pbr_sector)
{
       struct buffer_head *bh;
       struct mbr *mbr_p;
       struct pbr *pbr_p;
       struct part_entry *pte_p;
       unsigned int start_sector;

       /* read MBR sector */
       bh = rfs_bread(sb, 0, BH_RFS_MBR);
       if (!bh) { /* I/O error */
               DPRINTK("unable to read MBR sector \n");
               return -EIO;
       }

       mbr_p = (struct mbr *) bh->b_data;
       
       if ((u16) SIGNATURE != GET16(mbr_p->signature)) {
               DPRINTK("invalid MBR signature (%x != %x)\n",
                               SIGNATURE, GET16(mbr_p->signature));
               brelse(bh);
               return -EINVAL;
       }

       /* get partition entry */
       pte_p = (struct part_entry *) mbr_p->partition;
       start_sector = GET32(pte_p->start_sector);
       brelse(bh);

       /* read PBR sector */
       bh = rfs_bread(sb, start_sector, BH_RFS_MBR);
       if (!bh) { /* I/O error */
               DPRINTK("unable to read PBR sector \n");
               return -EIO;
       }

       pbr_p = (struct pbr *) bh->b_data;
       if ((u16) SIGNATURE != GET16(pbr_p->signature)) {
               DPRINTK("invalid boot sector signature (%x != %x)\n",
                               SIGNATURE, GET16(pbr_p->signature));
               brelse(bh);
               return -EINVAL;
       }

       /* set return value */
       *res = bh;
       *pbr_sector = start_sector;

       return 0;
}

/**
 * parse the mount options
 * @param sb           super block
 * @param options      mount options 
 * @return     zero on success
 */
int parse_option(struct super_block *sb, char *options)
{
#ifdef RFS_FOR_2_6
       substring_t args[MAX_OPT_ARGS];
#endif
       struct rfs_mount_info *opts = &(RFS_SB(sb)->options);
       char *codepage;
       char *p;

       opts->codepage = NULL;

       if (!options)
               goto out;

       /* 
        * in include/linux/rfs_fs.h
        * #define clear_opt(o, opt)               (o &= ~RFS_MOUNT_##opt)
        * #define set_opt(o, opt)                 (o |= RFS_MOUNT_##opt)
        * #define test_opt(sb, opt)               (RFS_SB(sb)->options.opts & \
        *                                                          RFS_MOUNT_##opt) */

#ifdef RFS_FOR_2_6
       while ((p = strsep(&options, ",")) != NULL) {
               int token;
               if (!*p)
                       continue;

               token = match_token(p, rfs_tokens, args);

               switch (token) {
               /* NLS codepage used in disk */
               case opt_codepage:
                       codepage = match_strdup(&args[0]);
                       if (!codepage)
                               return -ENOENT;
                       opts->codepage = codepage;
                       break;
               case opt_noxattr:
                       clear_opt(opts->opts, XATTR_USER);
               case opt_xattr:
                       set_opt(opts->opts, XATTR_USER);
                       break;
               case opt_vfat:
               case opt_acl:
               case opt_noacl:
                       DEBUG(DL0, "Mount option %s is not supported\n",p);
                       break;
               default:
                       return -EINVAL;
               }
       }
#else
       p = strtok(options, ",");
       if (p == NULL)
               goto out;

       do {
               if (!strncmp(p, "codepage=", 9)) {
                       codepage = strchr(p, '=');
                       if (!codepage)
                               return -ENOENT;
                       opts->codepage = codepage + 1;
               } else {
                       return -EINVAL;
               }
       } while((p = strtok(NULL, ",")) != NULL);
#endif

out:
#ifdef CONFIG_RFS_NLS
       if (opts->codepage == NULL) {
               if (strcmp(rfs_default_codepage, "")) {
                       opts->codepage = (char *) rfs_default_codepage;
                       DEBUG(DL0, "Use default codepage %s\n", opts->codepage);
               } else {
                       DPRINTK("If you configure the NLS, you must select codepage\n");
                       return -EINVAL;
               }
       }
#endif
       return 0;
}

/**
 *  fill up the RFS-specific super block
 * @param sb                   super block
 * @param old_blksize          original block size of block device
 * @param[out] new_blksize     new block size of the file system will be set 
 * @return                     return 0 on success, errno on failure
 *
 * choose a minimum value between cluster size and block size of block device
 */
static int rfs_build_sb(struct super_block *sb, unsigned long old_blksize, unsigned long *new_blksize)
{
       struct buffer_head *bh;
       struct pbr *pbr_p;
       struct bpb *bpb_p;
       unsigned int pbr_sector = 0;
       unsigned short sector_size, sector_bits; 
       unsigned int num_sectors, num_reserved, fat_sectors, root_sectors;
       unsigned int fat_start_sector, root_start_sector, data_start_sector;
       unsigned int num_root_entries = MAX_ROOT_DENTRY, root_clu = 0;
       unsigned int sectors_per_blk, sectors_per_blk_bits;
       unsigned int num_blks, block_size;
       loff_t device_size = sb->s_bdev->bd_inode->i_size;

       /* get PBR sector */
       if (get_pbr_info(sb, &bh, &pbr_sector))
               return -EINVAL;

       /* fill private info of sb */
       pbr_p = (struct pbr *) bh->b_data;
       bpb_p = (struct bpb *) pbr_p->bpb;

       /* get logical sector size */
       sector_size = GET16(bpb_p->sector_size); 
       if (!sector_size || ((u32) sector_size > PAGE_CACHE_SIZE)) {
               DPRINTK("invalid logical sector size : %d\n", sector_size);
               brelse(bh);
               return -EINVAL;
       }
       sector_bits = ffs(sector_size) - 1;

       /* get reserved, fat, root sectors */
       num_reserved = GET16(bpb_p->num_reserved);
       fat_sectors = GET16(bpb_p->num_fat_sectors);
       root_sectors = GET16(bpb_p->num_root_entries) << DENTRY_SIZE_BITS;
       root_sectors = ((root_sectors - 1) >> sector_bits) + 1;
       if (!fat_sectors && !GET16(bpb_p->num_root_entries)) {
               /* when fat32 */
               RFS_SB(sb)->fat_bits = FAT32;
               fat_sectors = GET32(bpb_p->num_fat32_sectors);
               root_clu = GET32(bpb_p->root_cluster);
               root_sectors = 0;
               num_root_entries = 0;
       }

       /* get each area's start sector number */
       fat_start_sector = pbr_sector + num_reserved;
       root_start_sector = fat_start_sector + fat_sectors * bpb_p->num_fats; 
       data_start_sector = root_start_sector + root_sectors;

       /* get total number of sectors on volume */
       num_sectors = GET16(bpb_p->num_sectors);
       if (!num_sectors) 
               num_sectors = GET32(bpb_p->num_huge_sectors);
       /* check whether it is bigger than device size or it is not available */
       if (!num_sectors || ((num_sectors << sector_bits) > device_size)) {
               DPRINTK("invalid number of sectors : %u\n", num_sectors);
               brelse(bh);
               return -EINVAL;
       }

       /* set cluster size */
       RFS_SB(sb)->cluster_size = bpb_p->sectors_per_clu << sector_bits;
       RFS_SB(sb)->cluster_bits = ffs(RFS_SB(sb)->cluster_size) - 1;

       /* get new block size */
       if (old_blksize > RFS_SB(sb)->cluster_size)
               block_size = RFS_SB(sb)->cluster_size;
       else
               block_size = old_blksize;

       /* 
        * block size is sector size if block device's block size is not set,
        * logical sector size is bigger than block size to set, 
        * or start sector of data area is not aligned 
        */
       if (!block_size || sector_size > block_size || 
               (data_start_sector << sector_bits) & (block_size - 1))
               block_size = sector_size;

       sectors_per_blk = block_size >> sector_bits;
       sectors_per_blk_bits = ffs(sectors_per_blk) - 1;

       /* set number of blocks per cluster */ 
       RFS_SB(sb)->blks_per_clu = bpb_p->sectors_per_clu >> 
                                       sectors_per_blk_bits;
       RFS_SB(sb)->blks_per_clu_bits = ffs(RFS_SB(sb)->blks_per_clu) - 1;

       /* set start address of fat table area */
       RFS_SB(sb)->fat_start_addr = fat_start_sector << sector_bits;

       /* set start address of root directory */
       RFS_SB(sb)->root_start_addr = root_start_sector << sector_bits;
       /* 
        * NOTE: although total dir entries in root dir are bigger than 512 
        * RFS only used 512 entries in root dir
        */
       RFS_SB(sb)->root_end_addr = RFS_SB(sb)->root_start_addr + 
                               (num_root_entries << DENTRY_SIZE_BITS);

       /* set start block number of data area */
       RFS_SB(sb)->data_start = data_start_sector >> sectors_per_blk_bits; 

       /* set total number of clusters */
       num_blks = (num_sectors - data_start_sector) >> sectors_per_blk_bits;
       RFS_SB(sb)->num_clusters = (num_blks >> RFS_SB(sb)->blks_per_clu_bits) 
                                       + 2; /* clu 0 & clu 1 */

       /* set fat type */
       if (!RFS_SB(sb)->fat_bits) {
               if (RFS_SB(sb)->num_clusters >= FAT12_THRESHOLD &&
                       RFS_SB(sb)->num_clusters < FAT16_THRESHOLD)
                       RFS_SB(sb)->fat_bits = FAT16;
               else {
                       DPRINTK("invalid fat type\n");
                       brelse(bh);
                       return -EINVAL;
               }
       }

       /* set root dir's first cluster number, etc. */
       RFS_SB(sb)->root_clu = root_clu;
       RFS_SB(sb)->search_ptr = VALID_CLU; /* clu 0 & 1 are reserved */

       /* release buffer head contains pbr sector */
       brelse(bh);
               
       /* init semaphore for fat table */
       RFS_SB(sb)->fat_mutex = kmalloc(sizeof(struct rfs_semaphore), GFP_KERNEL);  
       if (!(RFS_SB(sb)->fat_mutex)) { /* memory error */
               DEBUG(DL0, "memory allocation failed");
               return -ENOMEM;
       }

       init_fat_lock(sb);

       /* init list for map destroy */
       INIT_LIST_HEAD(&RFS_SB(sb)->free_chunks);

       /* new block size of block device will be set */
       *new_blksize = block_size;

       /* NLS support */
#ifdef CONFIG_RFS_NLS
       if (RFS_SB(sb)->options.codepage) {
               RFS_SB(sb)->nls_disk = load_nls(RFS_SB(sb)->options.codepage);
               if (!RFS_SB(sb)->nls_disk) {
                       DPRINTK("RFS: %s not found\n", RFS_SB(sb)->options.codepage);
                       return -EINVAL;
               }
       }
#endif

#ifdef RFS_FOR_2_4
       init_timer(&RFS_SB(sb)->timer);
       RFS_SB(sb)->timer.function = rfs_log_wakeup;
       RFS_SB(sb)->timer.data = (unsigned long) sb;
#endif
       RFS_SB(sb)->highest_d_ino = (num_sectors << sector_bits) >> DENTRY_SIZE_BITS;

       return 0;
}

/**
 *  fill up the root inode
 * @param inode        root inode
 * @return return 0 on success, errno on failure
 */
static int fill_root_inode(struct inode *inode)
{
       struct super_block *sb = inode->i_sb;
       unsigned int last_clu;
       int err = 0, num_clusters = 0;

       inode->i_ino = ROOT_INO;
       inode->i_mode = S_IFDIR | 0777;
       inode->i_uid = 0;
       inode->i_gid = 0;

       inode->i_mtime = inode->i_atime = inode->i_ctime = CURRENT_TIME;
#ifndef RFS_FOR_2_6_19
       inode->i_blksize = sb->s_blocksize;
#endif
       inode->i_version = 0;

       insert_inode_hash(inode);

       inode->i_op = &rfs_dir_inode_operations;
       inode->i_fop = &rfs_dir_operations;

       RFS_I(inode)->start_clu = RFS_SB(sb)->root_clu;
       RFS_I(inode)->p_start_clu = RFS_SB(sb)->root_clu;
       RFS_I(inode)->index = 0;

       if (!IS_FAT32(RFS_SB(sb))) {
               inode->i_size = RFS_SB(sb)->cluster_size;
       } else {
               num_clusters = find_last_cluster(inode, &last_clu);
               if (num_clusters <= 0) {
                       err = num_clusters;
                       DPRINTK("No last cluster (err : %d)\n", err);
                       return -EIO;
               }
               inode->i_size = num_clusters 
                               << RFS_SB(sb)->cluster_bits;
               /* update last cluster */
               RFS_I(inode)->last_clu = last_clu;
       }

       inode->i_nlink = count_subdir(sb, RFS_I(inode)->start_clu);
       inode->i_blocks = (inode->i_size + SECTOR_SIZE - 1) >> SECTOR_BITS;

       set_mmu_private(inode, inode->i_size);

       spin_lock_init(&RFS_I(inode)->write_lock);
#ifdef RFS_FOR_2_4
       init_MUTEX(&RFS_I(inode)->data_mutex);
       init_timer(&RFS_I(inode)->timer);
       RFS_I(inode)->timer.function = rfs_data_wakeup;
       RFS_I(inode)->timer.data = (unsigned long) RFS_I(inode);
#endif

       return 0;
}

/**
 *  read the super block (disk such as MBR) of RFS and initialize super block (incore)
 * @param sb           super block
 * @param data         private data
 * @param silent       verbose flag
 * @return             return super block pointer on success, null on failure
 */
struct super_block *rfs_common_read_super(struct super_block *sb,
                                       void *data, int silent)
{
       struct inode *root_inode = NULL;
       unsigned long new_blksize, old_blksize;
       int err;

#ifdef RFS_FOR_2_6
       struct rfs_sb_info *sbi;

       /* initialize sbi with 0x00 */
       /* log_info and pool_info must be initialized with 0 */
       sbi = kzalloc(sizeof(struct rfs_sb_info), GFP_KERNEL);
       if (!sbi) /* memory error */
               goto failed_mount;

       sb->s_fs_info = sbi;

       old_blksize = block_size(sb->s_bdev);
#else
       old_blksize = block_size(sb->s_dev);
#endif
       sb_min_blocksize(sb, 512);

       /* parsing mount options */
       if (parse_option(sb, data) < 0)
               goto failed_mount;

       /* fill the RFS-specific info of sb */
       if (rfs_build_sb(sb, old_blksize, &new_blksize) < 0)
               goto failed_mount;

       

       /* setup the rest superblock info */
       if (!sb_set_blocksize(sb, new_blksize)) {
               DPRINTK("unable to set blocksize\n");
               goto failed_mount;
       }

       sb->s_maxbytes = 0xFFFFFFFF; /* maximum file size */
       sb->s_op = &rfs_sops;
       sb->s_magic = RFS_MAGIC;
       sb->s_dirt = 0;

       RFS_SB(sb)->fcache_array = NULL;
       if (rfs_fcache_init(sb)) { /* memory error */
               DPRINTK("unable to init fat cache\n");
               goto release_fcache;
       }

       /* allocate root inode & fill it */
       if (!(root_inode = new_inode(sb)))
               goto release_fcache;

       err = fill_root_inode(root_inode); 
       if (err) {
               iput(root_inode);
               goto release_fcache;
       }

       if (!(sb->s_root = d_alloc_root(root_inode))) {
               iput(root_inode);
               goto release_fcache;
       }

       return sb;

release_fcache:
       /* release fcache */
       if (RFS_SB(sb)->fcache_array)
               kfree(RFS_SB(sb)->fcache_array);
failed_mount:
       if (RFS_SB(sb)->fat_mutex)
               kfree(RFS_SB(sb)->fat_mutex);
#ifdef RFS_FOR_2_6
       if (sbi)
               kfree(sbi);
#endif 

       return NULL;
}

/**
 *  flush all dirty buffers of the file system include fat cache
 * @param sb   super block
 * @return     return 0
 */
int rfs_sync_vol(struct super_block *sb)
{
       /* fat cache sync */
       fat_lock(sb);
       rfs_fcache_sync(sb, 0);
       fat_unlock(sb);

       /* fat cache is dirty without waiting flush. So sync device */
#ifdef RFS_FOR_2_6
       sync_blockdev(sb->s_bdev);
#else  
       fsync_no_super(sb->s_dev);
#endif 
       return 0;
}

#ifdef _RFS_INTERNAL_UNUSED_LOG
/**
 *  flush all dirty buffers of a transaction
 * @param sb    super block
 * @return      return 0 on success
 */
int rfs_sync_transaction(struct super_block *sb)
{
       int ret = 0;

       if (RFS_LOG_I(sb)->type != RFS_LOG_UNLINK &&
                       RFS_LOG_I(sb)->type != RFS_LOG_DEL_INODE)
               ret = rfs_sync_inode(RFS_LOG_I(sb)->inode, 1, 0);

       ret |= rfs_meta_commit(sb);

       return ret;
}
#endif
