/* file for future usage, by arris */
/*
 * linux/fs/rfs/xattr_security.c
 * Handler for storing security labels as extended attributes.
 */

// #include <linux/string.h>
// #include <linux/fs.h>
#include <linux/security.h>
#include <linux/xattr.h>
#include "rfs.h"
#include "xattr.h"

static size_t rfs_xattr_security_list(struct dentry *dentry, char *list, size_t list_size,
               const char *name, size_t name_len, int type)
{
       const size_t prefix_len = sizeof(XATTR_SECURITY_PREFIX)-1;
       const size_t total_len = prefix_len + name_len + 1;

       printk(KERN_INFO "[rfs_xattr] %s \n", __FUNCTION__);

       if (list && total_len <= list_size) {
               memcpy(list, XATTR_SECURITY_PREFIX, prefix_len);
               memcpy(list+prefix_len, name, name_len);
               list[prefix_len + name_len] = '\0';
       }
       return total_len;
}

static int rfs_xattr_security_get(struct dentry *dentry, const char *name,
               void *buffer, size_t size, int type)
{
       struct rfs_xattr_param *param = kzalloc(sizeof(struct rfs_xattr_param *), GFP_KERNEL);

       printk(KERN_INFO "[rfs_xattr] %s \n", __FUNCTION__);

       if (strcmp(name, "") == 0)
               return -EINVAL;

       /* memcpy name etc to param? arris */
       return rfs_xattr_get(dentry->d_inode, param, (unsigned int *)type);
       //  RFS_XATTR_NS_SECURITY
}

static int rfs_xattr_security_set(struct dentry *dentry, const char *name,
               const void *value, size_t size, int flags, int type)
{
       struct rfs_xattr_param *param = kzalloc(sizeof(struct rfs_xattr_param *), GFP_KERNEL);

       printk(KERN_INFO "[rfs_xattr] %s \n", __FUNCTION__);

       if (strcmp(name, "") == 0)
               return -EINVAL;

       /* memcpy name etc to param? arris */
       return rfs_xattr_set(dentry->d_inode, param);
}

int rfs_init_security(struct inode *inode, struct inode *dir)
{
       int err;
       size_t len;
       void *value;
       char *name;

       printk(KERN_INFO "[rfs_xattr] %s \n", __FUNCTION__);

       err = security_inode_init_security(inode, dir, &name, &value, &len);
       if (err) {
               if (err == -EOPNOTSUPP)
                       return 0;
               return err;
       }

       /*      err = rfs_xattr_set_handle(handle, inode, RFS_XATTR_NS_SECURITY,
               name, value, len, 0);
               */
       kfree(name);
       kfree(value);
       return err;
}

struct xattr_handler rfs_xattr_security_handler = {
       .prefix = XATTR_SECURITY_PREFIX,
       .list   = rfs_xattr_security_list,
       .get    = rfs_xattr_security_get,
       .set    = rfs_xattr_security_set,
};
