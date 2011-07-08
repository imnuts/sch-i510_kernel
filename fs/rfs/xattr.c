/* file for future usage, by arris */

#include <linux/rfs_fs.h>

#include "xattr.h"

static struct xattr_handler *rfs_xattr_handler_map[] = {
       [RFS_XATTR_NS_USER]              = &rfs_xattr_user_handler,
#ifdef CONFIG_RFS_FS_POSIX_ACL
       [RFS_XATTR_NS_POSIX_ACL_ACCESS]  = &rfs_xattr_acl_access_handler,
       [RFS_XATTR_NS_POSIX_ACL_DEFAULT] = &rfs_xattr_acl_default_handler,
#endif
       [RFS_XATTR_NS_TRUSTED]           = &rfs_xattr_trusted_handler,
#ifdef CONFIG_RFS_FS_SECURITY
       [RFS_XATTR_NS_SECURITY]          = &rfs_xattr_security_handler,
#endif
};

struct xattr_handler *rfs_xattr_handlers[] = {
       &rfs_xattr_user_handler,
       &rfs_xattr_trusted_handler,
#ifdef CONFIG_RFS_FS_POSIX_ACL
       &rfs_xattr_acl_access_handler,
       &rfs_xattr_acl_default_handler,
#endif
#ifdef CONFIG_RFS_FS_SECURITY
       &rfs_xattr_security_handler,
#endif
       NULL
};

static inline struct xattr_handler *rfs_xattr_handler(int name_index)
{
       struct xattr_handler *handler = NULL;

       if (name_index > 0 && name_index < ARRAY_SIZE(rfs_xattr_handler_map))
               handler = rfs_xattr_handler_map[name_index];
       return handler;
}

int rfs_xattr_get(struct inode *inode, struct rfs_xattr_param *param, unsigned int *dummy){
       int err = 0;

       printk(KERN_INFO "[rfs_xattr] %s name %s\n", __FUNCTION__,(char *)&param->name);

//     down_read(&RFS_I(dentry->d_inode)->xattr_sem);

//     up_read(&RFS_I(dentry->d_inode)->xattr_sem);

       return err;
}

int rfs_do_xattr_set(struct inode *inode, struct rfs_xattr_param *param){
       printk(KERN_INFO "[rfs_xattr] %s name %s\n", __FUNCTION__,(char *)&param->name);
       return 0;
}

int rfs_xattr_set(struct inode *inode, struct rfs_xattr_param *param){
       int err = 0;

       printk(KERN_INFO "[rfs_xattr] %s name %s\n", __FUNCTION__,(char *)&param->name);

//     down_write(&RFS_I(dentry->d_inode)->xattr_sem);
       err = rfs_do_xattr_set(inode, param);
//     up_write(&RFS_I(dentry->d_inode)->xattr_sem);

       return err;
}

int rfs_do_xattr_delete(struct inode *inode, struct rfs_xattr_param *param){
       printk(KERN_INFO "[rfs_xattr] %s name %s\n", __FUNCTION__,(char *)&param->name);
       return 0;
}

int rfs_xattr_delete(struct inode *inode, struct rfs_xattr_param *param){
       int err = 0;

       printk(KERN_INFO "[rfs_xattr] %s name %s\n", __FUNCTION__,(char *)&param->name);

       err = rfs_do_xattr_delete(inode, param);

       return err;
}

ssize_t rfs_xattr_list(struct dentry *dentry, char *buffer, size_t buffer_size){
       int err = 0;

       printk(KERN_INFO "[rfs_xattr] %s \n", __FUNCTION__);

//     down_read(&RFS_I(dentry->d_inode)->xattr_sem);

//     up_read(&RFS_I(dentry->d_inode)->xattr_sem);

       return err;
}

int rfs_xattr_read_header_to_inode(struct inode *inode, int b){
       printk(KERN_INFO "[rfs_xattr] %s \n", __FUNCTION__);
       return 0;
}

int rfs_xattr_write_header(struct inode *inode){
       printk(KERN_INFO "[rfs_xattr] %s \n", __FUNCTION__);
       return 0;
}

int __xattr_io(int a, struct inode *inode, unsigned long c, char *buffer, unsigned int buffer_size){
       printk(KERN_INFO "[rfs_xattr] %s \n", __FUNCTION__);
       return 0;
}
