/**
 * @file       fs/rfs/cluster.c
 * @brief      FAT cluster & FAT table handling functions
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
#include "log.h"

/*
 * structure for FAT cache (INCORE)
 */
struct rfs_fcache {
       unsigned int blkoff;
       unsigned int f_dirty;
       struct buffer_head *f_bh;
       struct list_head list;
};

/*
 * structure of candidate segment (INCORE)
 */
struct c_segment {
       unsigned int start_cluster;
       unsigned int last_cluster;
       struct list_head list;
};

/*
 * structure for map destroy (INCORE)
 */
struct old_map {
       unsigned int block;
       unsigned int nr_blocks;
       struct list_head list;
};

#define FAT_CACHE_SIZE         128

#define FAT_CACHE_HEAD(sb)     (&(RFS_SB(sb)->fcache_lru_list))
#define FAT_CACHE_ENTRY(p)     list_entry(p, struct rfs_fcache, list)
#define SEGMENT_ENTRY(p)       list_entry(p, struct c_segment, list)

#define IS_CONSECUTION(x, y)   ((x + 1) == y ? 1 : 0)

#define IS_POOL_EMPTY(n)       ((n) == POOL_RESERVED_CLUSTER)

static int rfs_insert_candidate(struct inode *);
static int rfs_put_pool(struct super_block *, unsigned int, unsigned int, unsigned int);

/*
 * FAT table manipulations
 */    

/**
 *  initialize internal fat cache entries and add them into fat cache lru list
 * @param sb   super block
 * @return     return 0 on success, -ENOMEM on failure
 */
int rfs_fcache_init(struct super_block *sb)
{
       struct rfs_fcache *array = NULL;
       int i, len;

       len = sizeof(struct rfs_fcache) * FAT_CACHE_SIZE;
       array = (struct rfs_fcache *) kmalloc(len, GFP_KERNEL);
       if (!array) /* memory error */
               return -ENOMEM;

       INIT_LIST_HEAD(FAT_CACHE_HEAD(sb));

       for (i = 0; i < FAT_CACHE_SIZE; i++) {
               array[i].blkoff = NOT_ASSIGNED;
               array[i].f_dirty = FALSE;
               array[i].f_bh = NULL;
               list_add_tail(&(array[i].list), FAT_CACHE_HEAD(sb));
       }

       RFS_SB(sb)->fcache_array = array;

       return 0;
}

/**
 *  destroy fat cache entries and free memory for them
 * @param sb   super block
 */
void rfs_fcache_release(struct super_block *sb)
{
       struct list_head *p;
       struct rfs_fcache *fcache_p = NULL;

       /* release buffer head */
       list_for_each(p, FAT_CACHE_HEAD(sb)) {
               fcache_p = FAT_CACHE_ENTRY(p);
               brelse(fcache_p->f_bh);
       }

       /* release fcache */
       if (RFS_SB(sb)->fcache_array)
               kfree(RFS_SB(sb)->fcache_array);
}

/**
 *  sync all fat cache entries if dirty flag of them are set
 * @param sb   super block
 * @param flush whether to flush or not
 *
 *  mark dirty flag of buffer head corresponding with all fat cache entries and nullify them
 */ 
void rfs_fcache_sync(struct super_block *sb, int flush)
{
       struct rfs_fcache *fcache_p;
       struct list_head *head, *p;

       head = FAT_CACHE_HEAD(sb);

       list_for_each(p, head) {
               fcache_p = FAT_CACHE_ENTRY(p);
               if (fcache_p->f_dirty) {
                       rfs_mark_buffer_dirty(fcache_p->f_bh, sb);
                       fcache_p->f_dirty = FALSE;

                       if (unlikely(flush)) {
                               ll_rw_block(WRITE, 1, &fcache_p->f_bh);
                               wait_on_buffer(fcache_p->f_bh);
                       }
               }
       }
}

/**
 *  change status of fat cache entry
 * @param sb           super block
 * @param blkoff       block number to modify
 *
 * if blkoff is equal to block number(blkoff) fat cache entry, set a dirty flag
 */
static void rfs_fcache_modified(struct super_block *sb, unsigned int blkoff)
{
       struct rfs_fcache *fcache_p = NULL;
       struct list_head *p;

       list_for_each(p, FAT_CACHE_HEAD(sb)) {
               fcache_p = FAT_CACHE_ENTRY(p);
               if (fcache_p->blkoff == blkoff) {
                       fcache_p->f_dirty = TRUE;
                       return;
               }
       }
}

/**
 *  append fat cache entry to fcache lru list 
 * @param sb           super block
 * @param blkoff       block number of fat cache entry
 * @return             return buffer head corrsponding with fat cache entry on success, NULL on failure
 */
static struct buffer_head *rfs_fcache_add(struct super_block *sb, unsigned int blkoff)
{
       struct rfs_fcache *fcache_p;
       struct buffer_head *bh;
       struct list_head *head, *p;

       head = FAT_CACHE_HEAD(sb);

retry:
       p = head->prev; 
       fcache_p = FAT_CACHE_ENTRY(p);
       while (fcache_p->f_bh && fcache_p->f_dirty && p != head) {
               /* We flush the dirty FAT caches only in fat_sync() */
               p = p->prev;
               fcache_p = FAT_CACHE_ENTRY(p);
       }

       if (unlikely(p == head)) {
               /* there is no clean fat cache. So, perform fat cache flush */
               rfs_fcache_sync(sb, 1);
               goto retry;
       }

       brelse(fcache_p->f_bh);

       /*
        * initialize fat cache with some effort
        * because possible error of following sb_bread() makes it false
        */
       fcache_p->f_bh = NULL;
       fcache_p->blkoff = NOT_ASSIGNED;

       bh = rfs_bread(sb, blkoff, BH_RFS_FAT);
       if (!bh) { /* I/O error */
               DPRINTK("can't get buffer head related with fat block\n");
               return NULL;
       }

       /* fill fcache */
       fcache_p->blkoff = blkoff;
       fcache_p->f_dirty = FALSE; /* just read */
       fcache_p->f_bh = bh;

       list_move(p, head);

       return bh;
}

/**
 *  lookup fat cache entry by using blkoff
 * @param sb           super block
 * @param blkoff       block number
 * @return             buffer head of fat cache entry
 *
 * if fat cache entry doesn't exist, get free fat entry and fill it
 */
static struct buffer_head *rfs_fcache_get(struct super_block *sb, unsigned int blkoff)
{
       struct rfs_fcache *fcache_p = NULL;
       struct list_head *head, *p;

       /* find fcache entry included blkoff */
       head = FAT_CACHE_HEAD(sb);
       list_for_each(p, head) {
               fcache_p = FAT_CACHE_ENTRY(p);
               if (fcache_p->blkoff == blkoff) {
                       /* Update LRU list */
                       if (p != head->next)
                               list_move(p, head);
                       return fcache_p->f_bh; /* found */
               }
       }

       return rfs_fcache_add(sb, blkoff);
}

/**
 *  read a block that contains a fat entry
 * @param sb           super block
 * @param[in, out] index index number of fat entry to calculate block number
 * @param[out] blocknr block number corresponding to index     
 * @return             buffer head of fat cache entry which include a fat entry
 */
static struct buffer_head *fat_read_block(struct super_block *sb, unsigned int *index, unsigned int *blocknr)
{
       unsigned int block, offset = *index;

       if (IS_FAT16(RFS_SB(sb)))
               offset = (offset << 1) + RFS_SB(sb)->fat_start_addr;
       else if (IS_FAT32(RFS_SB(sb)))
               offset = (offset << 2) + RFS_SB(sb)->fat_start_addr;

       block = offset >> sb->s_blocksize_bits;

       *index = offset;
       *blocknr = block;

       return rfs_fcache_get(sb, block);
}

/**
 *  read a fat entry in fat table
 * @param sb           super block
 * @param location     location to read a fat entry 
 * @param[out] content content cluster number which is saved in fat entry
 * @return             return 0 on sucess, EIO on failure
 */
int fat_read(struct super_block *sb, unsigned int location, unsigned int *content)
{
       struct buffer_head *bh = NULL;
       unsigned int index = location, dummy;

       if (IS_INVAL_CLU(RFS_SB(sb), location)) {
               /* out-of-range input */
               RFS_BUG("invalid cluster number(%u)\n", location);
               return -EINVAL;
       }

       if (!(bh = fat_read_block(sb, &index, &dummy))) { /* I/O error */
               RFS_BUG("can't get buffer head related with fat block\n");
               return -EIO; 
       }

       if (IS_FAT16(RFS_SB(sb))) {
               index &= (sb->s_blocksize - 1);
               *content = GET16(((u16 *)bh->b_data)[index >> 1]);

               if (*content >= 0xFFF8)
                       *content = CLU_TAIL; /* last cluster of fat chain */

       } else if (IS_FAT32(RFS_SB(sb))) {
               index &= (sb->s_blocksize - 1);
               *content = GET32(((u32 *)bh->b_data)[index >> 2]);

               if (*content >= 0xFFFFFF8) 
                       *content = CLU_TAIL; /* last cluster of chain */
       }

       /* sanity check */
       if ((*content != CLU_FREE) && (*content != CLU_TAIL)) {
               if (IS_INVAL_CLU(RFS_SB(sb), *content)) {
                       RFS_BUG("invalid contents(%u)\n", *content);
                       return -EIO;
               }
       }

       return 0; 
}

/**
 *  write a fat entry in fat table 1 & 2
 * @param sb           super block
 * @param location     location to write a fat entry 
 * @param content      next cluster number or end mark of fat chain
 * @return             return 0 on sucess, EIO or EINVAL on failure
 * @pre                        content shouldn't have 0 or 1 which are reserved
 */
int fat_write(struct super_block *sb, unsigned int location, unsigned int content)
{
       struct buffer_head *bh;
       unsigned int index = location, block;

       if (IS_INVAL_CLU(RFS_SB(sb), location)) {
               /* out-of-range input */
               RFS_BUG("invalid cluster number(%u)\n", location);
               return -EINVAL;
       }

       /* sanity check */
       if ((content != CLU_FREE) && (content != CLU_TAIL)) {
               if (IS_INVAL_CLU(RFS_SB(sb), content)) {
                       RFS_BUG("invalid contents(%u)\n", content);
                       return -EIO;
               }
       }

       bh = fat_read_block(sb, &index, &block);
       if (!bh) { /* I/O error */
               RFS_BUG("Can't get buffer head related with fat block\n"); 
               return -EIO;  
       }

       if (IS_FAT16(RFS_SB(sb))) {
               index &= sb->s_blocksize - 1;
               content &= 0x0000FFFF;
               SET16(((u16 *)bh->b_data)[index >> 1], content);

       } else if (IS_FAT32(RFS_SB(sb))) {
               index &= sb->s_blocksize - 1;
               content &= 0xFFFFFFFF;
               SET32(((u32 *)bh->b_data)[index >> 2], content);
       } 

       rfs_fcache_modified(sb, block);

       return 0;
}

/*
 * Cluster manipulations
 */

/**
 *  find free cluster in fat table
 * @param inode                        inode
 * @param[out] free_clu                free cluster number found
 * @return                     return 0 on success, errno on failure
 */
int find_free_cluster(struct inode *inode, unsigned int *free_clu)
{
       struct rfs_sb_info *sbi = RFS_SB(inode->i_sb);
       unsigned int i;
       int err;

       for (i = VALID_CLU; i < sbi->num_clusters; i++) { 
               /* search free cluster from hint(search_ptr) */
               if (sbi->search_ptr >= sbi->num_clusters)
                       sbi->search_ptr = VALID_CLU;

               err = fat_read(inode->i_sb, sbi->search_ptr, free_clu);
               if (!err && *free_clu == 0) {
                       *free_clu = sbi->search_ptr++;
                       return 0;
               }
               sbi->search_ptr++;
       }

       return -ENOSPC; 
}

/**
 *  find last cluster and return the number of clusters from specified fat chain of inode
 * @param inode                inode   
 * @param[out] last_clu        last cluster number
 * @return             return the number of clusters on success, errno on failure
 */
int find_last_cluster(struct inode *inode, unsigned int *last_clu)
{
       struct super_block *sb = inode->i_sb;
       unsigned int prev, next;
       int count = 0;
       int err = 0;

       /* set default value */
       if (last_clu)
               *last_clu = CLU_TAIL;

       prev = RFS_I(inode)->start_clu;

       if (prev == CLU_TAIL) /* cluster dose not be allocated */
               return 0;

       fat_lock(sb);

       while (1) {
               err = fat_read(sb, prev, &next);
               if (err) {
                       fat_unlock(sb);
                       return -EIO;
               }

               if (next < VALID_CLU) { /* out-of-range input */
                       fat_unlock(sb);
                       RFS_BUG("fat entry(%u) was corrupted\nTry to repair your partition with: fsck.vfat -a\n", next);
                       return -EIO;
               }

               count++;

               if (next == CLU_TAIL) /* found last clu */
                       break;

               prev = next;
       }

       fat_unlock(sb);

       if (last_clu) {
               *last_clu = prev;
               DEBUG(DL2, "last cluster number = %d \n", *last_clu);
       }


       return count;
}

/**
 *  append new cluster to fat chain of inode
 * @param inode                inode   
 * @param last_clu     current last cluster number of the fat chain
 * @param new_clu      new last cluster number to add
 * @return             return 0 on success, errno on failure
 */
int append_new_cluster(struct inode *inode, unsigned int last_clu, unsigned int new_clu)
{
       int err;

       err = fat_write(inode->i_sb, new_clu, CLU_TAIL);
       if (err) {
               DPRINTK("can't write a fat entry(%u)\n", new_clu);
               return err;
       }

       err = fat_write(inode->i_sb, last_clu, new_clu);
       if (err) {
               DPRINTK("can't write a fat entry(%u). "
                       "cluster(%u) is lost\n", last_clu, new_clu);
               return err;
       }

       return err;
}

/**
 *  allocate a new cluster from pool file or fat table
 * @param inode                inode
 * @param[out] new_clu new clsuter number to be allocated
 * @param last_clu     last clsuter number for logging
 * @return             return 0 on success, errno on failure
 */
static int get_cluster(struct inode *inode, unsigned int *new_clu, unsigned int last_clu)
{
       struct super_block *sb = inode->i_sb;
       unsigned int pool_prev, pool_next;
       unsigned int clu;
       int where, err;

       if (!RFS_SB(sb)->pool_info ||
           IS_POOL_EMPTY(RFS_POOL_I(sb)->num_clusters)) {
               /* alloc-cluster from fat table */
               err = find_free_cluster(inode, &clu);
               if (err)
                       return err;
       
               pool_prev = pool_next = CLU_TAIL;
               where = RFS_FAT_TABLE;
       } else {
               /* alloc-cluster from pool file */
               unsigned int dummy;
       
               err = rfs_shrink_pool_chain(sb, &clu, 
                               1, &dummy, &pool_next);
               if (err)
                       return err;

               pool_prev = RFS_POOL_I(sb)->start_cluster;
               where = RFS_POOL;
       }

       err = rfs_log_alloc_chain(sb, RFS_I(inode)->p_start_clu,
                       RFS_I(inode)->index, pool_prev, pool_next,
                       last_clu, CLU_TAIL, 1, &clu);
       if (err)
               return err;

       if (where == RFS_POOL) {
               err = rfs_get_pool(sb, pool_next, 1);
               if (err)
                       return err;
       }

       *new_clu = clu;
       return 0;
}

/**
 *  allocate a new cluster from pool file or fat table
 * @param inode                inode
 * @param[out] new_clu new clsuter number to be allocated
 * @return             return 0 on success, errno on failure
 *
 * if file write or expand file(truncate), pre-allocation is available
 * if fat table doesn't have a free cluster at normal allocation case, free cluster will be allocated in pool file
 */ 
int alloc_cluster(struct inode *inode, unsigned int *new_clu)
{
       struct super_block *sb = inode->i_sb;
       unsigned int last_clu;
       int is_first = FALSE;
       int err;

       if (RFS_I(inode)->start_clu < VALID_CLU) { /* out-of-range input */
               DPRINTK("inode has invalid start cluster(%u)\n", 
                               RFS_I(inode)->start_clu);
               return -EINVAL;
       }

       fat_lock(sb);

       if (RFS_I(inode)->start_clu != CLU_TAIL)
               last_clu = RFS_I(inode)->last_clu;
       else
               last_clu = CLU_TAIL;

       /* Phase 1 : get one free cluster in source */
       if (tr_pre_alloc(sb)) { /* pre-allocation case */
               err = rfs_log_get_cluster(inode, new_clu);      
               if (err)
                       goto out;
       } else { /* normal allocation case */
               err = get_cluster(inode, new_clu, last_clu);
               if (err)
                       goto out;
       }

       /* Phase 2 : append free cluster to end of fat chain related to inode */
       if (RFS_I(inode)->start_clu == CLU_TAIL)
               err = fat_write(sb, *new_clu, CLU_TAIL); 
       else
               err = append_new_cluster(inode, last_clu, *new_clu);
       if (err)
               goto out;

       /* update start & last cluster */
       if (RFS_I(inode)->start_clu == CLU_TAIL) {
               RFS_I(inode)->start_clu = *new_clu;
               is_first = TRUE;
       }
       RFS_I(inode)->last_clu = *new_clu;
       inode->i_blocks += 1 << (RFS_SB(sb)->cluster_bits - SECTOR_BITS);

       RFS_SB(sb)->num_used_clusters++;

       /* check rfs-specific inode state */
       if (unlikely(RFS_I(inode)->i_state == RFS_I_FREE)) {
               /* RFS-log : have already logged things about following */
               if (is_first)
                       err = rfs_attach_candidate(inode);
               else
                       err = rfs_insert_candidate(inode);
       }

out:
       fat_unlock(sb);

       return err;
}

#ifdef CONFIG_RFS_MAPDESTROY
extern int (*xsr_stl_delete)(dev_t dev, u32 start, u32 nums, u32 b_size);
#endif

/**
 *  destroy stl map corresponding with deallocated clusters
 * @param sb   super block
 * @return     return 0 on success
 *
 * it is usually invoked after unlink & truncate 
 */
int rfs_map_destroy(struct super_block *sb)
{
       struct old_map *old_map;
       struct list_head *this, *next;

       list_for_each_safe(this, next, &RFS_SB(sb)->free_chunks) {
               old_map = list_entry(this, struct old_map, list);
               if (!old_map) {
                       RFS_BUG("Chunk is NULL for fat cleansing\n");
                       return -EIO;
               }
               DEBUG(DL2, "block = %d nr_blocks = %d", 
                               old_map->block, old_map->nr_blocks);
#ifdef CONFIG_RFS_MAPDESTROY
               xsr_stl_delete(sb->s_dev, old_map->block, old_map->nr_blocks,
                               sb->s_blocksize);
#endif
               /* delete chunk element from list and free them */
               list_del(this);
               kfree(old_map);
       }

       /* reinitialize chunk list head */
       list_del_init(&RFS_SB(sb)->free_chunks);

       return 0;
}

/**
 *  make a chunk element and add it to chunk list
 * @param sb           super block
 * @param start                start cluster number to free
 * @param nr_clusters  number of clusters to free      
 * @return             return 0 on success
 */
static int rfs_add_chunk(struct super_block *sb, unsigned int start, unsigned int nr_clusters)
{
       struct old_map *old_map;
       unsigned int start_block = START_BLOCK(start, sb);

       if (!IS_XSR(sb->s_dev))
               return 0;

       if (!nr_clusters)
               return 0;

       old_map = (struct old_map *)
                       kmalloc(sizeof(struct old_map), GFP_KERNEL);
       if (!old_map) { /* memory error */
               DEBUG(DL1, "memory allocation failed\n");
               return -ENOMEM;
       }

       old_map->block = start_block;
       old_map->nr_blocks = nr_clusters << RFS_SB(sb)->blks_per_clu_bits;
       DEBUG(DL2, "start block = %d nr_blocks = %d", 
                       old_map->block, old_map->nr_blocks);

       list_add_tail(&old_map->list, &RFS_SB(sb)->free_chunks);

       return 0;
}

/**
 *  count clusters in fat chain & make a chunk list for stl delete
 * @param sb           super block
 * @param start_clu    start cluster number of fat chain
 * @param[out] last_clu        last cluster number of fat chain
 * @param last_val     content of last cluster
 * @param[out] clusters        number of clusters which may be appended or not
 * @return             return 0 on success, errno on failure
 *
 */
static int rfs_check_size(struct super_block *sb, unsigned int start_clu, unsigned int *last_clu, unsigned int last_val, unsigned int *clusters)
{
       unsigned int prev = start_clu, next;
       unsigned int total_nr_clus = 0, chunk_nr_clus = 0;
       unsigned int chunk_start = start_clu;
       int err;

       while (1) { /* count clusters from start_clu */
               err = fat_read(sb, prev, &next);
               if (err) {
                       DPRINTK("can't read a fat entry(%u)\n", prev);
                       return err;
               }

               if (next < VALID_CLU) { /* out-of-range input */
                       RFS_BUG("fat entry(%u) was corrupted\n", next);
                       return -EIO;
               }

               total_nr_clus++;

               /* make a chunk list for map destroy */
               chunk_nr_clus++;
               if (!IS_CONSECUTION(prev, next)) {
                       rfs_add_chunk(sb, chunk_start, chunk_nr_clus);
                       chunk_start = next;
                       chunk_nr_clus = 0;
               }

               if (prev == last_val || next == CLU_TAIL) {
                       if (IS_CONSECUTION(prev, next))
                               rfs_add_chunk(sb, chunk_start, chunk_nr_clus);
                       break;
               }

               prev = next;
       }

       *last_clu = prev;
       *clusters = total_nr_clus;

       return 0;
}

/**
 *  free fat chain from start_clu to EOC
 * @param inode                inode
 * @param new_last     new last cluster number
 * @param start_clu    start cluster number to free
 * @param[out] count   number of clusters which will be freed
 * @return             return 0 on success, errno on failure
 * @pre                        caller must have a mutex for fat table
 *
 *  new_last is only available if reduce a file especially truncate case
 */
int free_chain(struct inode *inode, unsigned int new_last, unsigned int start_clu, unsigned int *count)
{
       struct super_block *sb = inode->i_sb;
       unsigned int free_clus[2];
       unsigned int free_count;
       unsigned int next;
       int err = 0;

       *count = free_count = 0;

       /* check whether clusters are appended to pool file */
       err = rfs_check_size(sb, start_clu, &next, CLU_TAIL, &free_count);
       if (err)
               return err;

       free_clus[0] = start_clu;
       free_clus[1] = RFS_I(inode)->last_clu;
       if (rfs_log_move_chain(sb, RFS_I(inode)->p_start_clu,
                              RFS_I(inode)->index,
                              RFS_POOL_I(sb)->last_cluster,
                              RFS_POOL_I(sb)->c_start_cluster,
                              new_last, CLU_TAIL, 2, free_clus)) {
               return -EIO;
       }

       err = rfs_put_pool(sb, start_clu, next, free_count);
       if (err) {
               DPRINTK("can't append free clusters into pool\n");
               return err;
       }

       if (new_last != CLU_TAIL) {
               /* mark new last cluster */
               err = fat_write(sb, new_last, CLU_TAIL);
               if (err) {
                       DPRINTK("can't write a fat entry (%u)\n", new_last);
                       return err;
               }

               RFS_I(inode)->last_clu = new_last;
       } else {
               RFS_I(inode)->start_clu = CLU_TAIL;
               RFS_I(inode)->last_clu = CLU_TAIL;
       }

       /* update used clusters */
       RFS_SB(inode->i_sb)->num_used_clusters -= free_count;
       *count = free_count;
       
       return 0;
}

/**
 *  deallocate clusters after skipping some clusters
 * @param inode        inode
 * @param skip the number of clusters to skip
 * @return     return 0 on success, errno on failure
 */
int dealloc_clusters(struct inode *inode, unsigned int skip)
{
       struct super_block *sb = inode->i_sb;
       unsigned int new_last = NOT_ASSIGNED;
       unsigned int prev, next;
       unsigned int off = skip - 1;
       unsigned int count = 0;
       int err = 0;

       fat_lock(sb);

       if (!skip) { /* free all clusters */
               next = RFS_I(inode)->start_clu;
               goto free;
       }

       err = find_cluster(sb, RFS_I(inode)->start_clu, off, &prev, &next);
       if (err)
               goto out;

       /* change last cluster number */
       new_last = prev;

free:  
       if (next == CLU_TAIL) /* do not need free chain */
               goto out;

       err = free_chain(inode, new_last, next, &count);

       
out:
       fat_unlock(sb);

       return err;
}

/**
 *  count all used clusters in file system
 * @param sb super block
 * @param[out] used_clusters the number of used clusters in volume 
 * @return return 0 on success, errno on failure
 *
 * cluster 0 & 1 are reserved according to the fat spec
 */
int count_used_clusters(struct super_block *sb, unsigned int *used_clusters)
{
       unsigned int i, clu;
       unsigned int count = 2; /* clu 0 & 1 are reserved */
       int err;

       fat_lock(sb);

       for (i = VALID_CLU; i < RFS_SB(sb)->num_clusters; i++) {
               err = fat_read(sb, i, &clu);
               if (err) {
                       fat_unlock(sb);
                       DPRINTK("can't read a fat entry (%u)\n", i);
                       return err;
               }

               if (clu)
                       count++;
       }

       *used_clusters = count;

       fat_unlock(sb);

       return 0;
}

/**
 *  find a cluster which has an offset into fat chain
 * @param sb           super block
 * @param start_clu    start cluster number of fat chain
 * @param off          offset within fat chain
 * @param[out] clu     cluster number found 
 * @param[out] value   fat entry value of cluster
 * @return return 0 on success, errno on failure
 * @pre                        caller should get fat lock
 */ 
int find_cluster(struct super_block *sb, unsigned int start_clu, unsigned int off, unsigned int *clu, unsigned int *value)
{
       unsigned int prev, next;
       unsigned int i;
       int err;

       prev = start_clu;
       for (i = 0; i <= off; i++) {
               err = fat_read(sb, prev, &next);
               if (err) {
                       DPRINTK("can't read a fat entry (%u)\n", prev);
                       return err;
               }

               if (next < VALID_CLU) { /* out-of-range input */
                       /*
                        * If reset happened during appending a cluster,
                        * the appending cluster had a free status (0).
                        * EFAULT notifies it to replay method.
                        */
                       if (tr_in_replay(sb))
                               return -EFAULT;
                       else
                               return -EIO;
               }

               if (i == off) /* do not need to change prev to next */
                       break;

               if (next == CLU_TAIL)
                       return -EFAULT; /* over request */

               prev = next;
       }

       *clu = prev;
       *value = next;
       return 0;
}

/*
 * fast unlink
 */

/**
 *  initialize first block for meta data of the pool file
 * @param sb           super block
 * @param index                index of dir entry
 * @param start_clu    start cluster number of the pool file
 * @param blocknr      block number to save the dir entry of pool file
 * @return             return 0 on success, errno on failure
 */
static int rfs_init_pool_block(struct super_block *sb, unsigned int index, unsigned int start_clu, unsigned long blocknr)
{
       struct buffer_head *bh = NULL;
       struct rfs_pool_info *pool_info = NULL;

       bh = sb_getblk(sb, START_BLOCK(start_clu, sb));
       if (!bh) { /* I/O error */
               DPRINTK("can't get buffer head to get dir entry of pool file\n");
               return -EIO;
       }
       memset(bh->b_data, 0x00, sb->s_blocksize);
       
       pool_info = (struct rfs_pool_info *) bh->b_data;
       SET32(pool_info->index, index);
       SET32(pool_info->start_cluster, start_clu);
       SET32(pool_info->last_cluster, start_clu);
       SET32(pool_info->num_clusters, POOL_RESERVED_CLUSTER);
       SET32(pool_info->c_start_cluster, NOT_ASSIGNED);
       SET32(pool_info->c_last_cluster, NOT_ASSIGNED);
       SET32(pool_info->blocknr, blocknr);

#ifdef RFS_FOR_2_6
       set_buffer_uptodate(bh);
#else  
       mark_buffer_uptodate(bh, 1);
#endif
       mark_buffer_dirty(bh);
       brelse(bh);

       return 0;
}

/**
 *  allocate memory and make the in-core meta data of pool file
 * @param sb           super block
 * @param index                index of dir entry
 * @param start_clu    start cluster number of the pool file
 * @param blocknr      block number which saved dir entry
 * @return             return 0 on success, errno on failure
 *
 * hint info are written in start block of start cluster allocated the pool file
 */
static int rfs_alloc_pool(struct super_block *sb, unsigned int index, unsigned int start_clu, unsigned long blocknr)
{
       struct rfs_pool_info *pool_info = NULL, *disk_pool_info = NULL;
       struct buffer_head *bh = NULL;

       /* read hint info */
       bh = rfs_bread(sb, START_BLOCK(start_clu, sb), BH_RFS_POOL_BLOCK);
       if (!bh) { /* I/O error */
               DPRINTK("can't get buffer head to get head of pool file\n");
               return -EIO;
       }

       disk_pool_info = (struct rfs_pool_info *) bh->b_data;

       /* memory allocation */
       pool_info = (struct rfs_pool_info *) 
               kmalloc(sizeof(struct rfs_pool_info), GFP_KERNEL);
       if (!pool_info) { /* memory error */
               DEBUG(DL1, "failed memory allocation for pool info\n");
               brelse(bh);
               return -ENOMEM;
       }

       /* compare disk info and in-core info */
       if ((index != GET32(disk_pool_info->index)) ||
               (start_clu != GET32(disk_pool_info->start_cluster))) {
               kfree(pool_info);
               brelse(bh);
               DPRINTK("disk info and in-core info aren't equal. The file system is corrupted.\n");
               return -EIO;
       }

       /* initialize pool info */
       pool_info->index = GET32(disk_pool_info->index);
//     pool_info->blocknr = GET32(disk_pool_info->blocknr);
       pool_info->start_cluster = GET32(disk_pool_info->start_cluster);
       pool_info->last_cluster = GET32(disk_pool_info->last_cluster);
       pool_info->num_clusters = GET32(disk_pool_info->num_clusters);
       pool_info->c_start_cluster = GET32(disk_pool_info->c_start_cluster);
       pool_info->c_last_cluster = GET32(disk_pool_info->c_last_cluster);
#ifdef RFS_UPDATE_POOLHEADER_AT_MOUNT
       pool_info->blocknr = GET32(disk_pool_info->blocknr);

       if (pool_info->blocknr != blocknr) { /* reinit block number */
               pool_info->blocknr = blocknr;
               SET32(disk_pool_info->blocknr, blocknr);
               mark_buffer_dirty(bh);
       }
#else
       pool_info->blocknr = blocknr;
#endif

       INIT_LIST_HEAD(&pool_info->c_list);

       brelse(bh);

       RFS_SB(sb)->pool_info = (void *) pool_info;

       return 0;
}

/**
 *  open the pool file
 * @param sb   super block
 * @return     return 0 on success, errno on failure
 *
 * the pool file doesn't exist, make the pool file before anything else
 */
static int rfs_open_pool(struct super_block *sb)
{
       struct inode *root_dir = sb->s_root->d_inode;
       struct buffer_head *bh = NULL;
       struct rfs_dir_entry *ep = NULL;
       unsigned int index;
       int ret;

       /* find pool file */
       ret = find_entry(root_dir, RFS_POOL_FILE_NAME, &bh, TYPE_FILE);
       if (ret < 0) {
               brelse(bh);
               return ret; 
       }

       index = ret;
       ep = get_entry(root_dir, index, &bh);
       if (IS_ERR(ep)) {
               brelse(bh);
               return PTR_ERR(ep);
       }

       /* allocate memory for hint & fill hint info */ 
       ret = rfs_alloc_pool(sb, index, START_CLUSTER(ep), bh->b_blocknr); 
       if (ret) {
               brelse(bh);
               return ret;
       }

       brelse(bh);
       return 0;
}

/**
 *  make the pool file
 * @param sb   super block
 * @return     return 0 on success, errno on failure
 */
static int rfs_make_pool(struct super_block *sb)
{
       struct inode *root_dir = sb->s_root->d_inode;
       struct inode *inode = NULL;
       struct buffer_head *bh = NULL;
       struct rfs_dir_entry *ep = NULL;
       unsigned int start_clu, index;
       unsigned long blocknr;
       int err;

       /* get a new inode */
       if (!(inode = new_inode(sb)))
               return -ENOMEM; /* memory error */

       /* allocate start cluster */
       RFS_I(inode)->start_clu = CLU_TAIL;
       err = alloc_cluster(inode, &start_clu);
       if (err) 
               goto remove_inode;

       /* make a dir entry */
       index = build_entry(root_dir, NULL, start_clu, TYPE_FILE, 
                       RFS_POOL_FILE_NAME);
       if ((int) index < 0) {
               err = index;
               goto dealloc_cluster;
       }

       ep = get_entry(root_dir, index, &bh);
       if (IS_ERR(ep)) {
               err = PTR_ERR(ep);
               goto remove_entry;
       }

       blocknr = bh->b_blocknr;

       ep->attr |= (ATTR_READONLY | ATTR_SYSTEM | ATTR_HIDDEN);
       SET32(ep->size, RFS_SB(sb)->cluster_size);

       /* initialize first block of cluster */
       err = rfs_init_pool_block(sb, index, start_clu, blocknr);
       if (err) /* I/O error */
               goto remove_entry;
       
       /* allocate memory for hint & fill hint info */ 
       err = rfs_alloc_pool(sb, index, start_clu, blocknr);
       if (err) 
               goto remove_entry;

       mark_buffer_dirty(bh);
       brelse(bh);

       iput(inode);

       /* flush buffers for internal fcache & dir entry */
       rfs_sync_vol(sb);

       return 0;

remove_entry:
       ep->name[0] = (u8) DELETE_MARK;
       mark_buffer_dirty(bh);
       brelse(bh);
dealloc_cluster:
       fat_write(sb, start_clu, CLU_FREE);
remove_inode:
       inode->i_nlink--;
       RFS_BUG_ON(inode->i_nlink);
       iput(inode);
       return err;
}

/**
 *  initialize the pool file at mount time
 * @param sb   super block
 * @return     return 0 on success, errno on failure
 */
int rfs_init_pool(struct super_block *sb)
{
       int err;

       if (RFS_POOL_I(sb)) /* already initialize it */
               return 0;

       /* check whether pool file exist */
       err = rfs_open_pool(sb);
       if (err == -ENOENT) { 
               /* make pool file */
               err = rfs_make_pool(sb);
               if (err) {
                       DPRINTK("can not create a pool file\n");
                       return err;
               }
       } else if (err) { 
               /* unexpected error case */
               DPRINTK("can not open pool file\n");
               return err;
       }

       return 0;
}

/**
 *  release the pool file at umount time
 *  @param sb  super block
 */
void rfs_release_pool(struct super_block *sb)
{
       if (!RFS_POOL_I(sb))
               return;

       kfree(RFS_POOL_I(sb));
}

#ifdef _RFS_INTERNAL_SANITY_CHECK
int sanity_check_pool(struct super_block *sb)
{
       unsigned int prev = RFS_POOL_I(sb)->start_cluster;
       unsigned int next;
       unsigned int p_last_clu = RFS_POOL_I(sb)->last_cluster;
       unsigned int c_start_clu = RFS_POOL_I(sb)->c_start_cluster;
       unsigned int num_clus = RFS_POOL_I(sb)->num_clusters;
       unsigned int count = 1;
       int err;

       next = prev;

       while (count < num_clus) {
               err = fat_read(sb, prev, &next);
               if (err)
                       return -1;

               if (next == CLU_TAIL)
                       break;

               count++;
               if (next == p_last_clu)
                       break;

               prev = next;
       }

       if (next != p_last_clu || count != RFS_POOL_I(sb)->num_clusters) {
               RFS_BUG("pool corrupted (%d, %d) (%d, %d)\n", next, p_last_clu,
                               count, RFS_POOL_I(sb)->num_clusters);
               return -1;
       }

       prev = next;
       fat_read(sb, prev, &next);

       if (next != c_start_clu) {
               RFS_BUG("candidate corrupted(%d, %d)\n", next, c_start_clu);
               return -1;
       }

       return 0;
}
#endif

/**
 *  update last cluster number and total clusters for the pool file
 * @param sb   super block
 * @return     return 0 on success, errno on failure
 */
int rfs_update_pool_block(struct super_block *sb)
{
       struct buffer_head *bh = NULL;
       struct rfs_pool_info *disk_pool = NULL;
       struct rfs_pool_info *rpi = RFS_POOL_I(sb);
       struct rfs_sb_info *sbi = RFS_SB(sb);
       int bug_on = 0;

       bh = rfs_bread(sb, START_BLOCK(rpi->start_cluster, sb),
                       BH_RFS_POOL_BLOCK);
       if (!bh) { /* I/O error */
               DPRINTK("can't get buffer head to get head of pool file\n");
               return -EIO;
       }

       /* sanity check */
       if (unlikely(rpi->last_cluster != CLU_TAIL &&
                       IS_INVAL_CLU(sbi, rpi->last_cluster)))
               bug_on = 1;
       else if (unlikely(rpi->num_clusters > RFS_SB(sb)->num_clusters - 2))
               bug_on = 1;
       else if (unlikely(rpi->c_start_cluster != CLU_TAIL &&
                       IS_INVAL_CLU(sbi, rpi->c_start_cluster)))
               bug_on = 1;
       else if (unlikely(rpi->c_last_cluster != CLU_TAIL &&
                       IS_INVAL_CLU(sbi, rpi->c_last_cluster)))
               bug_on = 1;
 
       if (unlikely(bug_on)) {
               DPRINTK("in-memory pool is corrupted(%u, %u, %u, %u)\n",
                       rpi->last_cluster, rpi->num_clusters,
                       rpi->c_start_cluster, rpi->c_last_cluster);
               BUG();
       }
 
       
       disk_pool = (struct rfs_pool_info *) bh->b_data;
       SET32(disk_pool->last_cluster, rpi->last_cluster);
       SET32(disk_pool->num_clusters, rpi->num_clusters);
       SET32(disk_pool->c_start_cluster, rpi->c_start_cluster);
       SET32(disk_pool->c_last_cluster, rpi->c_last_cluster);

#ifdef _RFS_INTERNAL_SANITY_CHECK
       sanity_check_pool(sb);
#endif
       rfs_mark_buffer_dirty(bh, sb);
       brelse(bh);

       return 0;
}

/**
 *  update size of dir entry for the pool file
 * @param sb           super block
 * @param clusters     number of clusters to expand or shrink
 * @param resize       new size of pool file
 * @return             return 0 on success, errno on failure
 */
int rfs_update_pool_entry(struct super_block *sb, unsigned int clusters, int resize)
{
       struct buffer_head *bh = NULL;
       struct rfs_dir_entry *ep = NULL;
       struct rfs_pool_info *pool_info = RFS_POOL_I(sb);
       unsigned int size, total_size;
       unsigned long offset;

       bh = rfs_bread(sb, pool_info->blocknr, BH_RFS_ROOT);
       if (!bh) { /* I/O error */
               DPRINTK("can't get buffer head related with pool head\n");
               return -EIO;
       }

       offset = pool_info->index << DENTRY_SIZE_BITS;
       offset += RFS_SB(sb)->root_start_addr;
       offset &= (sb->s_blocksize - 1);
       ep = (struct rfs_dir_entry *) (bh->b_data + offset);

       if (RFS_POOL_I(sb)->start_cluster != START_CLUSTER(ep)) {
               brelse(bh);
               RFS_BUG("mismatch start_cluster in-core & disk(%d != %d)\n",
                       pool_info->start_cluster, START_CLUSTER(ep));
               return -EIO;
       }

       /* update size */
       total_size = GET32(ep->size);
       size = clusters << RFS_SB(sb)->cluster_bits;
       if (resize == EXPAND_POOL_SIZE) {
               /* append case */
               total_size += size;
       } else if (resize == SHRINK_POOL_SIZE) {
               /* shrink case */
               total_size -= size;
       } else {
               /* RFS-log : SET_POOL_SIZE */
               total_size = size;
       }

       
       SET32(ep->size, total_size);
       rfs_mark_buffer_dirty(bh, sb);
       brelse(bh);

       

       return 0;
}

/**
 *  shrink some clusters from the pool chain 
 * @param sb                   super block
 * @param[out] clu_list                cluster list allocated
 * @param num_clusters         number of clusters will be allocated
 * @param[out] alloc_clusters  number of clusters were allocated
 * @param[out] next_clu                next cluster number to connect with start cluster of pool file
 * @return                     return 0 on success, errno on failure
 *
 * when it is invoked, fat chain will not be updated
 */
int rfs_shrink_pool_chain(struct super_block *sb, unsigned int *clu_list, unsigned int num_clusters, unsigned int *alloc_clusters, unsigned int *next_clu)
{
       unsigned int prev, next;
       unsigned int count;
       unsigned int pool_clusters;
       int i, err;

       pool_clusters = RFS_POOL_I(sb)->num_clusters - POOL_RESERVED_CLUSTER;
       *next_clu = CLU_TAIL;

       /* first cluster of pool is reserved for pool meta data */
       if (IS_POOL_EMPTY(RFS_POOL_I(sb)->num_clusters)) {
               /* run out of free clusters */
               *alloc_clusters = 0;
               return 0;
       }

       if (num_clusters > pool_clusters)
               count = pool_clusters;
       else
               count = num_clusters;

       prev = RFS_POOL_I(sb)->start_cluster;

       for (i = count; i >= 0; i--) {
               err = fat_read(sb, prev, &next);
               if (err) {
                       DPRINTK("can't read a fat entry (%u)\n", prev);
                       return -EIO;
               }

               if (next < VALID_CLU) { /* out-of-range input */
                       RFS_BUG("fat entry(%u) was corrupted\n", next);
                       return -EIO;
               }

               /* do not need to update clu_list */
               if (i == 0)
                       break;

               *clu_list++ = next;
               prev = next;
       }

       *next_clu = next;
       *alloc_clusters = count;

       return 0;
}

/**
 *  update the fat chain of pool file
 * @param sb           super block
 * @param next_clu     next cluster number
 * @param last_clu     new last cluster number
 * @param clusters     number of clusters
 * @param is_expand    flag to expand or shrink
 * @return             return 0 on success, errno on failure
 */
static int rfs_move_pool_chain(struct super_block *sb, unsigned int next_clu, unsigned int last_clu, unsigned int clusters, int is_expand) 
{
       struct rfs_pool_info *pool_info = RFS_POOL_I(sb);
       int err;

       if (is_expand == EXPAND_POOL_SIZE) {
               err = fat_write(sb, pool_info->last_cluster, next_clu);
               if (err) {
                       DPRINTK("can't write a fat entry (%u)\n", 
                                       pool_info->last_cluster);
                       return err;
               }

               /* update hint info */
               pool_info->num_clusters += clusters;
               pool_info->last_cluster = last_clu;
       
               /* check whether candidate segments exist */
               if (pool_info->c_start_cluster == NOT_ASSIGNED) /* not exist */
                       return 0;

               /* link start candidate segment with 
                  end of free clusters in pool file */
               err = fat_write(sb, pool_info->last_cluster, 
                                       pool_info->c_start_cluster);
               if (err) {
                       DPRINTK("can't write a fat entry (%u)\n", 
                                       pool_info->last_cluster);
                       return err;
               }
       } else if (is_expand == SHRINK_POOL_SIZE) {
               err = fat_write(sb, pool_info->start_cluster, next_clu);
               if (err) {
                       DPRINTK("can't write a fat entry (%u)\n", 
                                       pool_info->start_cluster);
                       return err;
               }

               /* update hint info */
               pool_info->num_clusters -= clusters;
               if (IS_POOL_EMPTY(pool_info->num_clusters)) {
                       pool_info->last_cluster = 
                               pool_info->start_cluster;
               }
       } else {
               /* RFS-log : SET_POOL_SIZE is never passed in this function */
       }

       return 0;
}

/**
 *  put clusters deallocated in the pool file
 * @param sb           super block
 * @param start_clu    start cluster number
 * @param last_clu     new last cluster number
 * @param clusters     number of clusters
 * @return             return 0 on success, errno on failure
 */
static int rfs_put_pool(struct super_block *sb, unsigned int start_clu, unsigned int last_clu, unsigned int clusters)
{
       int err;

       err = rfs_move_pool_chain(sb, start_clu, last_clu, 
                               clusters, EXPAND_POOL_SIZE);
       if (err)
               return err;

       err = rfs_update_pool_entry(sb, clusters, EXPAND_POOL_SIZE);
       if (err)
               return err;

       err = rfs_update_pool_block(sb);
       if (err) /* I/O error */
               return err;

       

       return 0;
}

/**
 *  get free clusters in the pool file
 * @param sb           super block
 * @param next_clu     next cluster number
 * @param clusters     number of clusters      
 * @return             return 0 on success, errno on failure
 */
int rfs_get_pool(struct super_block *sb, unsigned int next_clu, unsigned int clusters)
{
       int err;
       unsigned int dummy = 0;

       err = rfs_move_pool_chain(sb, next_clu, dummy, 
                               clusters, SHRINK_POOL_SIZE);
       if (err) 
               return err;

       err = rfs_update_pool_entry(sb, clusters, SHRINK_POOL_SIZE);
       if (err)
               return err;

       err = rfs_update_pool_block(sb);
       if (err) /* I/O error */
               return err;

       

       return 0;
}

/**
 *  update candidate segment info and fat chain of pool file 
 * @param sb           super block
 * @param segment      candidate segment
 * @param is_attach    flag to attach or detach
 * @return             return 0 on success, errno on failure
 */
static int update_candidate_segment(struct super_block *sb, struct c_segment *segment, int is_attach)
{
       struct rfs_pool_info *pool_info = RFS_POOL_I(sb);
       unsigned int start, last;
       int err;

       start = pool_info->c_start_cluster;
       last = pool_info->c_last_cluster;

       if (is_attach) { /* attach case */
               if (start == NOT_ASSIGNED) {
                       err = fat_write(sb, pool_info->last_cluster, 
                                       segment->start_cluster);
                       if (err) {
                               DPRINTK("can't write a fat entry (%u)\n", 
                                       pool_info->last_cluster);
                               return err;
                       }
                       pool_info->c_start_cluster = segment->start_cluster;
               } else {
                       err = fat_write(sb, pool_info->c_last_cluster, 
                                       segment->start_cluster);
                       if (err) {
                               DPRINTK("can't write a fat entry (%u)\n", 
                                       pool_info->c_last_cluster);
                               return err;
                       }
               }
               pool_info->c_last_cluster = segment->last_cluster;
       } else { /* detach case */
               struct c_segment *prev_seg, *next_seg;

               prev_seg = SEGMENT_ENTRY(segment->list.prev);
               next_seg = SEGMENT_ENTRY(segment->list.next);

               if (start == segment->start_cluster &&
                               last == segment->last_cluster) { /* only one */
                       pool_info->c_start_cluster = NOT_ASSIGNED;
                       pool_info->c_last_cluster = NOT_ASSIGNED;
               } else if (start == segment->start_cluster) { /* first seg */
                       pool_info->c_start_cluster = next_seg->start_cluster;
               } else if (last == segment->last_cluster) { /* last seg */
                       err = fat_write(sb, pool_info->last_cluster,
                                               segment->start_cluster);
                       if (err) {
                               DPRINTK("can't write a fat entry (%u)\n", 
                                       pool_info->last_cluster);
                               return err;
                       }

                       err = fat_write(sb, segment->last_cluster, start);
                       if (err) {
                               DPRINTK("can't write a fat entry (%u)\n", 
                                       segment->last_cluster);
                               return err;
                       }

                       err = fat_write(sb, prev_seg->last_cluster, CLU_TAIL);
                       if (err) {
                               DPRINTK("can't write a fat entry (%u)\n", 
                                       prev_seg->last_cluster);
                               return err;
                       }
                       pool_info->c_last_cluster = prev_seg->last_cluster;
               } else { /* middle seg */
                       err = fat_write(sb, pool_info->last_cluster,
                                               segment->start_cluster);
                       if (err) {
                               DPRINTK("can't write a fat entry (%u)\n", 
                                       pool_info->last_cluster);
                               return err;
                       }

                       err = fat_write(sb, segment->last_cluster, start);
                       if (err) {
                               DPRINTK("can't write a fat entry (%u)\n", 
                                       segment->last_cluster);
                               return err;
                       }

                       err = fat_write(sb, prev_seg->last_cluster, 
                                               next_seg->start_cluster);
                       if (err) {
                               DPRINTK("can't write a fat entry (%u)\n", 
                                       prev_seg->last_cluster);
                               return err;
                       }
               }
       }

       return 0;
}

/**
 *  attach segment into candidate segment list on pool file
 * @param inode        inode
 * @return     return 0 on success, errno on failure   
 */
int rfs_attach_candidate(struct inode *inode)
{
       struct super_block *sb = inode->i_sb;
       struct c_segment *segment;
       unsigned int move_chain[2];
       unsigned int p_prev_clu;
       int err;

       if (RFS_I(inode)->start_clu == CLU_TAIL) /* not assigned to inode */
               return 0;

       fat_lock(sb);

       /* make a candidate segment */
       segment = (struct c_segment *)
               kmalloc(sizeof(struct c_segment), GFP_KERNEL);
       if (!segment) { /* memory error */
               DEBUG(DL1, "memory allocation failed\n");
               err = -ENOMEM;
               goto out;
       }

       segment->start_cluster = RFS_I(inode)->start_clu;
       segment->last_cluster = RFS_I(inode)->last_clu;

       /* RFS-log : move chain of file to pool's tail */
       p_prev_clu = (RFS_POOL_I(sb)->c_last_cluster != CLU_TAIL) ?
               RFS_POOL_I(sb)->c_last_cluster : RFS_POOL_I(sb)->last_cluster;

       move_chain[0] = segment->start_cluster;
       move_chain[1] = segment->last_cluster;
       if ((err = rfs_log_move_chain(sb, RFS_I(inode)->p_start_clu,
                                     RFS_I(inode)->index,
                                     p_prev_clu, CLU_TAIL,
                                     CLU_TAIL, CLU_TAIL, 2, move_chain))) {
               goto out;
       }

       /* update candidate segment list on pool file */
       err = update_candidate_segment(sb, segment, 1);
       if (err)
               goto out;
               
       list_add_tail(&segment->list, &RFS_POOL_I(sb)->c_list);

       /* update hint info of pool file */
       err = rfs_update_pool_block(sb);

out:
       fat_unlock(sb);
       return err;
}

/**
 *  lookup a candidate segment corresponding with start cluster of inode in list
 * @param inode                inode
 * @return             return segment on success, NULL on failure
 */ 
static struct c_segment *lookup_segment(struct inode *inode)
{
       unsigned int start_clu = RFS_I(inode)->start_clu;
       struct c_segment *segment;
       struct list_head *p;

       list_for_each(p, &RFS_POOL_I(inode->i_sb)->c_list) {
               segment = SEGMENT_ENTRY(p);
               if (segment->start_cluster == start_clu)
                       return segment; /* found */
       }

       return NULL;
}

/**
 *  detach segment from candidate segment list 
 * @param inode        inode
 * @return     return 0 on success, errno on failure   
 *
 * And add it into deleted segment on pool file
 */
int rfs_detach_candidate(struct inode *inode)
{
       struct super_block *sb = inode->i_sb;
       struct c_segment *segment, *s;
       unsigned int nr_clusters, dummy;
       unsigned int t_prev_clu, t_next_clu;
       unsigned int move_chain[2];
       unsigned int err;

       if ((RFS_I(inode)->start_clu == CLU_TAIL) ||
                       (RFS_I(inode)->i_state != RFS_I_FREE))
               return 0;

       fat_lock(sb);

       /* lookup candidate segment in list */
       segment = lookup_segment(inode);
       if (!segment || segment->last_cluster != RFS_I(inode)->last_clu) { 
               /* NOT found */
               fat_unlock(sb);
               RFS_BUG("segment does not exist for %d\n", 
                               RFS_I(inode)->start_clu);
               return -EIO;
       }

       /* count clusters in segment and make chunk list for stl delete */
       err = rfs_check_size(sb, segment->start_cluster, &dummy,
                               segment->last_cluster, &nr_clusters);
       if (err)
               goto out;

       /* RFS-log : get prev and next clu of candidate segment */
       if (segment->start_cluster !=
                       RFS_POOL_I(inode->i_sb)->c_start_cluster) {
               s = SEGMENT_ENTRY(segment->list.prev);
               t_prev_clu = s->last_cluster;
       } else
               t_prev_clu = RFS_POOL_I(sb)->last_cluster;

       if (segment->last_cluster != RFS_POOL_I(inode->i_sb)->c_last_cluster) {
               s = SEGMENT_ENTRY(segment->list.next);
               t_next_clu = s->start_cluster;
       } else
               t_next_clu = CLU_TAIL;

       move_chain[0] = segment->start_cluster;
       move_chain[1] = segment->last_cluster;

       /* RFS-log : move candidate segment to deleted pool */
       /* don't care pdir (CLU_TAIL) and entry (-1) */
       if ((err = rfs_log_move_chain(sb, CLU_TAIL, -1,
                                     RFS_POOL_I(sb)->last_cluster,
                                     RFS_POOL_I(sb)->c_start_cluster,
                                     t_prev_clu, t_next_clu,
                                     2, move_chain))) {
               goto out;
       }

       /* update candidate segment list on pool file */
       err = update_candidate_segment(sb, segment, 0);
       if (err)
               goto out;

       RFS_POOL_I(sb)->last_cluster = segment->last_cluster;
       RFS_POOL_I(sb)->num_clusters += nr_clusters;

       list_del(&segment->list);
       kfree(segment);

       /* update hint info & dir entry of pool file */
       err = rfs_update_pool_entry(sb, nr_clusters, EXPAND_POOL_SIZE);
       if (err)
               goto out;

       err = rfs_update_pool_block(sb);
       if (err) /* I/O error */
               goto out;

       /* update used clusters in volume */
       RFS_SB(sb)->num_used_clusters -= nr_clusters;

       
out:
       fat_unlock(sb);
       return err;
}

/**
 *  insert cluster into candidate segment 
 * @param inode        inode
 * @return     return 0 on success, errno on failure   
 */
static int rfs_insert_candidate(struct inode *inode)
{
       struct super_block *sb = inode->i_sb;
       struct rfs_pool_info *pool_info = RFS_POOL_I(sb);
       struct c_segment *segment, *next_seg;
       int err;

       /* lookup candidate segment in list */
       segment = lookup_segment(inode);
       if (!segment) { 
               /* NOT found */
               RFS_BUG("segment does not exist for %d\n", 
                               RFS_I(inode)->start_clu);
               return -EIO;
       }

       /* update fat chain of pool file */
       if (segment->last_cluster != pool_info->c_last_cluster) {
               next_seg = SEGMENT_ENTRY(segment->list.next);
               err = fat_write(sb, RFS_I(inode)->last_clu, 
                                       next_seg->start_cluster);
               if (err) {
                       DPRINTK("can't write a fat entry (%u)\n", 
                                       RFS_I(inode)->last_clu);
                       return err;
               }
       }

       /* update segment info */
       if (segment->last_cluster == pool_info->c_last_cluster) {
               pool_info->c_last_cluster = RFS_I(inode)->last_clu;
               err = rfs_update_pool_block(sb);
               if (err)
                       return err;
       }

       segment->last_cluster = RFS_I(inode)->last_clu;
       
       return 0;
}

/**
 *  remove all candidate segments in pool file
 * @param sb super block
 * @return return 0 on success, errno on failure
 *
 * It is only invoked at mount time
 */
int rfs_remove_candidates(struct super_block *sb)
{
       struct rfs_pool_info *pool_info = RFS_POOL_I(sb);
       unsigned int nr_clusters, dummy;
       int err = 0;

       if (pool_info->c_start_cluster == NOT_ASSIGNED)
               return 0;

       /* count clusters in candidate segments and make chunk for stl delete */
       err = rfs_check_size(sb, pool_info->c_start_cluster, &dummy,
                               CLU_TAIL, &nr_clusters);
       if (err)
               return err;

       if (rfs_log_start(sb, RFS_LOG_DEL_INODE, NULL))
               return -EIO;

       if ((err = rfs_log_update_pool(sb)))
               goto out;

       /* update hint info & dir entry of pool file */
       pool_info->last_cluster = pool_info->c_last_cluster;
       pool_info->num_clusters += nr_clusters;
       pool_info->c_start_cluster = NOT_ASSIGNED;
       pool_info->c_last_cluster = NOT_ASSIGNED;

       err = rfs_update_pool_entry(sb, nr_clusters, EXPAND_POOL_SIZE);
       if (err)
               goto out;

       err = rfs_update_pool_block(sb);
out:
       
       if (rfs_log_end(sb, err))
               return err;

       return 0;
}
