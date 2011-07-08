/* file for future usage, by arris */
/*
 * linux/fs/rfs/xattr_user.c
 * Handler for extended user attributes.
 *
 */

/* #include <linux/module.h>
#include <linux/string.h>
#include <linux/fs.h> */
#include <linux/xattr.h> 

#include <linux/rfs_fs.h>
#include "rfs.h"
#include "xattr.h"

       static size_t
rfs_xattr_user_list(struct dentry *dentry, char *list, size_t list_size,
               const char *name, size_t name_len, int type)
{
       const size_t prefix_len = XATTR_USER_PREFIX_LEN;
       const size_t total_len = prefix_len + name_len + 1;

       printk(KERN_INFO "[rfs_xattr] %s \n", __FUNCTION__);

       if (!test_opt(dentry->d_sb, XATTR_USER))
               return 0;

       if (list && total_len <= list_size) {
               memcpy(list, XATTR_USER_PREFIX, prefix_len);
               memcpy(list+prefix_len, name, name_len);
               list[prefix_len + name_len] = '\0';
       }
       return total_len;
}

       static int
rfs_xattr_user_get(struct dentry *dentry, const char *name,
               void *buffer, size_t size, int type)
{
       struct rfs_xattr_param *param = kzalloc(sizeof(struct rfs_xattr_param *), GFP_KERNEL);

       printk(KERN_INFO "[rfs_xattr] %s \n", __FUNCTION__);

       if (strcmp(name, "") == 0)
               return -EINVAL;
       if (!test_opt(dentry->d_sb, XATTR_USER))
               return -EOPNOTSUPP; 

       /* memcpy name etc to param? arris */
       return rfs_xattr_get(dentry->d_inode, param, (unsigned int *)type);
}

       static int
rfs_xattr_user_set(struct dentry *dentry, const char *name,
               const void *value, size_t size, int flags, int type)
{
       struct rfs_xattr_param *param = kzalloc(sizeof(struct rfs_xattr_param *), GFP_KERNEL);

       printk(KERN_INFO "[rfs_xattr] %s \n", __FUNCTION__);

       if (strcmp(name, "") == 0)
               return -EINVAL;
       if (!test_opt(dentry->d_sb, XATTR_USER))
               return -EOPNOTSUPP;

       /* memcpy name etc to param? arris */
       return rfs_xattr_set(dentry->d_inode, param);
}

struct xattr_handler rfs_xattr_user_handler = {
       .prefix = XATTR_USER_PREFIX,
       .list   = rfs_xattr_user_list,
       .get    = rfs_xattr_user_get,
       .set    = rfs_xattr_user_set,
};
