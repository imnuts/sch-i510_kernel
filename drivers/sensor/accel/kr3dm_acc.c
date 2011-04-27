/*  $Date: 2009/11/10 17:37:35 $
 *  Revision: 1.0
 */

/****************************** kr3dm *************************************
Application description	: kr3dm Linux driver
			: STMicroelectronics
Date			: 08/12/2009
Revision			: 1-0-0
Changed Features		: First Release
Bug fixes			: First Release
MEMS platform		: digital output KR3DM_ACC
S/W platform		: gcc 4.2.1
Application Details		: kr3dm Linux driver

Copyright (c) 2009 STMicroelectronics.

THIS PROGRAM IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER
EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTY
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE ENTIRE RISK
AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE
PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,
REPAIR OR CORRECTION.

THIS DOCUMENT CONTAINS PROPRIETARY AND CONFIDENTIAL INFORMATION OF THE
STMICROELECTRONICS GROUP.
INFORMATION FURNISHED IS BELIEVED TO BE ACCURATE AND RELIABLE. HOWEVER,
STMICROELECTRONICS ASSUMES NO RESPONSIBILITY FOR THE CONSEQUENCES OF USE
OF SUCH INFORMATION.
SPECIFICATIONS MENTIONED IN THIS PUBLICATION ARE
SUBJECT TO CHANGE WITHOUT NOTICE.
THIS PUBLICATION SUPERSEDES AND REPLACES ALL INFORMATION PREVIOUSLY SUPPLIED.
STMICROELECTRONICS PRODUCTS ARE NOT AUTHORIZED FOR
USE AS CRITICAL COMPONENTS IN LIFE
SUPPORT DEVICES OR SYSTEMS WITHOUT EXPRESS
WRITTEN APPROVAL OF STMICROELECTRONICS.

STMicroelectronics GROUP OF COMPANIES

Australia - Belgium - Brazil - Canada - China - France - Germany - Italy -
Japan - Korea - Malaysia - Malta - Morocco - The Netherlands - Singapore -
Spain - Sweden - Switzerland - Taiwan - Thailand - United Kingdom - U.S.A.
STMicroelectronics Limited is a member of the STMicroelectronics Group.
********************************************************************************
Version History.

Revision 1-0-0 19/11/09
First Release

Revision 1-0-0 08/12/2009
First Release for KR3DM based on LIS331DLH
*******************************************************************************/


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/uaccess.h>
#include <linux/unistd.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/irq.h>
#include <mach/gpio.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include "kr3dm_acc.h"

#define KR3DM_MSG 0

/** KR3DM acceleration data
\brief Structure containing acceleration values for x,y and z-axis in signed short
*/
struct kr3dm_acc_t {
	signed char x, /* x-axis acc data sign extended. Range -128 to +127. */
		    y, /* y-axis acc data sign extended. Range -128 to +127. */
		    z; /* z-axis acc data sign extended. Range -128 to +127. */

};

struct kr3dm_data{
	struct i2c_client client;
};

static struct class *kr3d_dev_class;


/*************************************************************************/
/* KR3DM Functions							 */
/*************************************************************************/
/* set kr3dm bandwidth */
int kr3dm_set_bandwidth(char bw)
{
	int res = 0;
	unsigned char data =0;
#if KR3DM_MSG
	printk(KERN_INFO, "[KR3DM ACC]%s\n", __func__);
#endif
	res = i2c_acc_kr3dm_read_word(CTRL_REG1);
	if (res < 0)
            data =0;   
        else
           data = res & 0x00e7;

	data = data + bw;
	res = i2c_acc_kr3dm_write(CTRL_REG1, &data);
	return res;
}

/* read selected bandwidth from kr3dm*/
int kr3dm_get_bandwidth(unsigned char *bw)
{
	int res = 1;
#if KR3DM_MSG
	printk(KERN_INFO,  "[KR3DM ACC]%s\n", __func__);
#endif
	/* TO DO */
	return res;
}

int kr3dm_set_enable(char mode)
{
	int res = 0;
	unsigned char data = 0;

#if KR3DM_MSG
	printk(KERN_INFO, "[KR3DM ACC]%s\n", __func__);
#endif
	res = i2c_acc_kr3dm_read_word(CTRL_REG1);
	if (res < 0)
            data =0; 
        else   
            data = res & 0x001f;

	data = mode + data;
	res = i2c_acc_kr3dm_write(CTRL_REG1, &data);
	return res;
}

/* X,Y and Z-axis acceleration data readout
  \param *acc pointer to \ref kr3dm_acc_t structure for x,y,z data readout
  \note data will be read by multi-byte protocol into a 6 byte structure
*/
int kr3dm_read_accel_xyz(struct kr3dm_acc_t *acc)
{
	int res;
	unsigned char acc_data[6];
	res = i2c_acc_kr3dm_read(OUT_X, &acc_data[1], 1);
	res = i2c_acc_kr3dm_read(OUT_Y, &acc_data[3], 1);
	res = i2c_acc_kr3dm_read(OUT_Z, &acc_data[5], 1);
/*
	acc->x = (short) (((acc_data[1]) << 8) | acc_data[0]);
	acc->y = (short) (((acc_data[3]) << 8) | acc_data[2]);
	acc->z = (short) (((acc_data[5]) << 8) | acc_data[4]);
	acc->x = acc->x/16;
	acc->y = acc->y/16;
	acc->z = acc->z/16;
*/
	acc->x = acc_data[1];
	acc->y = acc_data[3];
	acc->z = acc_data[5];
#if KR3DM_MSG
	printk(KERN_INFO, "[KR3DM_ACC] kr3dm_read_accel_xyz, X:%d, Y:%d, Z:%d\n",
		acc_data[1], acc_data[3], acc_data[5]);
#endif

	return res;
}


/* Device Initialization  */
int device_init(void/*kr3dm_t *pLis*/)
{
	int res;
	unsigned char buf[2];
	buf[0] = 0x00;
	buf[1] = 0x27;
#if KR3DM_MSG
	printk(KERN_INFO, "[KR3DM ACC]%s\n", __func__);
#endif
	res = i2c_acc_kr3dm_write(CTRL_REG1, &buf[1]);
	res = i2c_acc_kr3dm_write(CTRL_REG2, &buf[0]);
	res = i2c_acc_kr3dm_write(CTRL_REG3, &buf[0]);
	res = i2c_acc_kr3dm_write(CTRL_REG4, &buf[0]);
	res = i2c_acc_kr3dm_write(CTRL_REG5, &buf[0]);
	res = i2c_acc_kr3dm_write(INT1_THS, &buf[0]);
	res = i2c_acc_kr3dm_write(INT2_THS, &buf[0]);
	res = i2c_acc_kr3dm_write(INT2_DURATION, &buf[0]);
	return res;
}

int kr3dm_set_range(char range)
{
	int err = 0;
	unsigned char buf[2];
#if KR3DM_MSG
	printk(KERN_INFO, "[KR3DM ACC]%s\n", __func__);
#endif
	buf[0] = range;
	err = i2c_acc_kr3dm_write(CTRL_REG4, buf);
	return err;
}

/*************************************************************************/
/* KR3DM Sysfs								*/
/*************************************************************************/
static ssize_t kr3dm_fs_read(struct device *dev, struct device_attribute *attr, char *buf)
{
	int count;
	struct kr3dm_acc_t acc;
	kr3dm_read_accel_xyz(&acc);
	printk(KERN_INFO, "x: %d,y: %d,z: %d\n", acc.x, acc.y, acc.z);
	count = sprintf(buf, "%d,%d,%d\n", acc.x, acc.y, acc.z);
	return count;
}

static ssize_t kr3dm_fs_write(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t size)
{
	/* buf[size]=0; */
	printk(KERN_INFO, "input data --> %s\n", buf);
	return size;
}

static DEVICE_ATTR(acc_file, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH,
	kr3dm_fs_read, kr3dm_fs_write);


/*************************************************************************/
/*		KR3DM operation functions			         */
/*************************************************************************/
/*  read command for KR3DM device file  */
static ssize_t kr3dm_read(struct file *file, char __user *buf,
	size_t count, loff_t *offset)
{
	int ret = 0;
	struct kr3dm_acc_t acc;
#if KR3DM_MSG
	printk(KERN_INFO, "[KR3DM ACC]%s\n", __func__);
#endif
	kr3dm_read_accel_xyz(&acc);
	if (count != sizeof(acc))
		return -1;
	ret = copy_to_user(buf, &acc, sizeof(acc));
	if (ret != 0) {
#if KR3DM_MSG
		printk(KERN_INFO,  "[KR3DM ACC]KR3DM Accel: copy_to_user result: %d\n",
			ret);
#endif
	}
	return sizeof(acc);
}

/*  write command for KR3DM device file */
static ssize_t kr3dm_write(struct file *file, const char __user *buf,
	size_t count, loff_t *offset)
{
#if KR3DM_MSG
	printk(KERN_INFO, "[KR3DM ACC]%s\n", __func__);
	printk(KERN_INFO, "[KR3DM ACC]KR3DM Accel should be accessed with ioctl command\n");
#endif
	return 0;
}

/*  open command for KR3DM device file  */
static int kr3dm_open(struct inode *inode, struct file *file)
{
	int chip_id = 0;
	/* chip_id = i2c_acc_kr3dm_read_word(WHO_AM_I); */
	/* KR3DM_MSG( "[KR3DM ACC]CHIP ID is 0x%x\n",chip_id); */
	device_init();
#if KR3DM_MSG
	printk(KERN_INFO,  "[KR3DM ACC]KR3DM Accel has been opened\n");
#endif
	return 0;
}

/*  release command for KR3DM device file */
static int kr3dm_close(struct inode *inode, struct file *file)
{
#if KR3DM_MSG
	printk(KERN_INFO,  "[KR3DM ACC]KR3DM Accel has been closed\n");
#endif
	return 0;
}

/*  ioctl command for KR3DM device file */
static int kr3dm_ioctl(struct inode *inode, struct file *file,
	unsigned int cmd, unsigned long arg)
{
	int err = 0;
	unsigned char data[6];
	/* cmd mapping */
	switch (cmd) {
	case KR3DM_IOCTL_SET_G_RANGE:
#if KR3DM_MSG
		printk(KERN_INFO, "[KR3DM ACC]%s - KR3DM_IOCTL_SET_G_RANGE\n",
			__func__);
#endif
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
			printk(KERN_INFO, "copy_from_user error\n");
			return -EFAULT;
		}
		err = kr3dm_set_range(*data);
		return err;
	case KR3DM_IOCTL_SET_ENABLE:
#if KR3DM_MSG
		printk(KERN_INFO, "[KR3DM ACC]%s - KR3DM_IOCTL_SET_ENABLE\n",
			__func__);
#endif
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
			printk(KERN_INFO, "copy_to_user error\n");
			return -EFAULT;
		}
		err = kr3dm_set_enable(*data);
		return err;
	case KR3DM_IOCTL_SET_DELAY:
#if KR3DM_MSG
		printk(KERN_INFO, "[KR3DM ACC]%s - KR3DM_IOCTL_SET_DELAY\n",
			__func__);
#endif
		if (copy_from_user(data, (unsigned char *)arg, 1) != 0) {
			printk(KERN_INFO, "copy_from_user error\n");
			return -EFAULT;
		}
		err = kr3dm_set_bandwidth(*data);
		return err;
	case KR3DM_IOCTL_GET_DELAY:
#if KR3DM_MSG
		printk(KERN_INFO, "[KR3DM ACC]%s - KR3DM_IOCTL_GET_DELAY\n",
			__func__);
#endif
		err = kr3dm_get_bandwidth(data);
		if (copy_to_user((unsigned char *)arg, data, 1) != 0) {
			printk(KERN_INFO, "copy_to_user error\n");
			return -EFAULT;
		}
		return err;
	case KR3DM_IOCTL_READ_ACCEL_XYZ:
#if KR3DM_MSG
		printk(KERN_INFO, "[KR3DM ACC]%s - KR3DM_IOCTL_READ_ACCEL_XYZ\n",
			__func__);
#endif
		err = kr3dm_read_accel_xyz((struct kr3dm_acc_t *)data);
		if (copy_to_user((struct kr3dm_acc_t *)arg,
			(struct kr3dm_acc_t *)data, 3) != 0) {
			printk(KERN_INFO, "copy_to error\n");
			return -EFAULT;
		}
		return err;
		/*case KR3DM_SELFTEST:
		TO DO
		return err;*/
	default:
#if KR3DM_MSG
		printk(KERN_INFO, "[KR3DM ACC]%s - INVALID COMMAND\n",
			__func__);
#endif
		return 0;
	}
}

static const struct file_operations kr3dm_fops = {
	.owner = THIS_MODULE,
	.read = kr3dm_read,
	.write = kr3dm_write,
	.open = kr3dm_open,
	.release = kr3dm_close,
	.ioctl = kr3dm_ioctl,
};

static int kr3dm_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	int err = 0;
	struct device *dev_t;
#if KR3DM_MSG
	printk(KERN_INFO, "[KR3DM ACC]%s\n", __func__);
#endif
	err = register_chrdev(ACC_DEV_MAJOR, "accelerometer", &kr3dm_fops);
	if (err < 0)
		return err;
	kr3d_dev_class = class_create(THIS_MODULE, "accelerometer");
	if (IS_ERR(kr3d_dev_class))
		return PTR_ERR(kr3d_dev_class);
	dev_t = device_create(kr3d_dev_class, NULL, MKDEV(ACC_DEV_MAJOR, 0),
		"%s", "accelerometer");
	if (device_create_file(dev_t, &dev_attr_acc_file) < 0)
		printk(KERN_INFO, "Failed to create device file(%s)!\n",
		dev_attr_acc_file.attr.name);
	if (IS_ERR(dev_t))
		return PTR_ERR(dev_t);
	return i2c_acc_kr3dm_init();
}

#ifdef CONFIG_PM
static int kr3dm_suspend(struct i2c_client *client, pm_message_t mesg)
{
	int res = 0;
	unsigned char data;
	res = i2c_acc_kr3dm_read_word(CTRL_REG1);
#if KR3DM_MSG
	printk(KERN_INFO, "[KR3DM ACC][kr3dm_suspend] res = %x\n", res);
#endif
	data = res & 0x001f;
#if KR3DM_MSG
	printk(KERN_INFO, "[KR3DM ACC][kr3dm_suspend] data = %x\n", data);
#endif
	res = i2c_acc_kr3dm_write(CTRL_REG1, &data);
#if KR3DM_MSG
	printk(KERN_INFO, "[KR3DM ACC][%s] KR3DM !!suspend mode!!\n", __func__);
#endif
	return 0;
}

static int kr3dm_resume(struct i2c_client *client)
{
	int res = 0;
	unsigned char data;
	res = i2c_acc_kr3dm_read_word(CTRL_REG1);
#if KR3DM_MSG
	printk(KERN_INFO, "[KR3DM ACC][kr3dm_resume] res = %x\n", res);
#endif
	data = (res & 0x003f)|(0x1<<5);
#if KR3DM_MSG
	printk(KERN_INFO, "[KR3DM ACC][kr3dm_resume] data = %x\n", data);
#endif
	res = i2c_acc_kr3dm_write(CTRL_REG1, &data);
#if KR3DM_MSG
	printk(KERN_INFO, "[KR3DM ACC][%s] KR3DM !!resume mode!!\n", __func__);
#endif
	return 0;
}
#else
#define kr3dm_suspend NULL
#define kr3dm_resume NULL
#endif

static struct platform_device *kr3dm_device;

static struct platform_driver kr3dm_driver = {
	.probe   = kr3dm_probe,
	.suspend = kr3dm_suspend,
	.resume  = kr3dm_resume,
	.driver  = {
		.name = "kr3dm-accelerometer",
	}
};

static int __init kr3dm_init(void)
{
	int result;
#if KR3DM_MSG
	printk(KERN_INFO, "[KR3DM ACC]%s\n", __func__);
#endif
	result = platform_driver_register(&kr3dm_driver);
	if (result)
		return result;
	kr3dm_device  = platform_device_register_simple("kr3dm-accelerometer", \
		-1, NULL, 0);
	if (IS_ERR(kr3dm_device))
		return PTR_ERR(kr3dm_device);
	return 0;
}

static void __exit kr3dm_exit(void)
{
 #if KR3DM_MSG
	printk(KERN_INFO, "[KR3DM ACC]KR3DM_ACCEL exit\n");
#endif
	i2c_acc_kr3dm_exit();
	unregister_chrdev(ACC_DEV_MAJOR, "accelerometer");
	class_destroy(kr3d_dev_class);
	platform_device_unregister(kr3dm_device);
	platform_driver_unregister(&kr3dm_driver);
}

module_init(kr3dm_init);
module_exit(kr3dm_exit);

MODULE_DESCRIPTION("kr3dm accelerometer driver");
MODULE_AUTHOR("STMicroelectronics");
MODULE_LICENSE("GPL");
