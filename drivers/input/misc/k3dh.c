/*
 *  STMicroelectronics k3dh acceleration sensor driver
 *
 *  Copyright (C) 2010 Samsung Electronics Co.Ltd
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/uaccess.h>
#include <linux/k3dh.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include "k3dh_reg.h"

#define k3dh_dbgmsg(str, args...) pr_debug("%s: " str, __func__, ##args)

/* The default settings when sensor is on is for all 3 axis to be enabled
 * and output data rate set to 400Hz.  Output is via a ioctl read call.
 */
#define DEFAULT_POWER_ON_SETTING (ODR400 | ENABLE_ALL_AXES)
#define ACC_DEV_NAME "accelerometer"
#define ACC_DEV_MAJOR 241

#define CALIBRATION_FILE_PATH	"/data/system/calibration_data"
#define CALIBRATION_DATA_AMOUNT	20

static const struct odr_delay {
	u8 odr; /* odr reg setting */
	s64 delay_ns; /* odr in ns */
} odr_delay_table[] = {
	{  ODR400,    2500000LL  }, /* 400Hz */
	{  ODR100,   10000000LL  }, /* 100Hz */
	{   ODR50,   20000000LL  }, /*  50Hz */
	{   ODR10,  100000000LL  }, /*  10Hz */
	{    ODR5,  200000000LL  }, /*   5Hz */
	{    ODR2,  500000000LL  }, /*   2Hz */
	{    ODR1, 1000000000LL  }, /*   1Hz */
	{ ODRHALF, 2000000000LL  }, /* 0.5Hz */
};

#ifdef CONFIG_MACH_CHIEF
static const int kr3dh_position_map[][3][3] = {
        {{ 0, -1,  0}, { 1,  0,  0}, { 0,  0,  1}}, /* top/upper-left */
        {{ 1,  0,  0}, { 0,  1,  0}, { 0,  0,  1}}, /* top/upper-right */
        {{ 0,  1,  0}, {-1,  0,  0}, { 0,  0,  1}}, /* top/lower-right */
        {{-1,  0,  0}, { 0, -1,  0}, { 0,  0,  1}}, /* top/lower-left */
        {{ 0,  1,  0}, { 1,  0,  0}, { 0,  0, -1}}, /* bottom/upper-right */
        {{-1,  0,  0}, { 0,  1,  0}, { 0,  0, -1}}, /* bottom/upper-left */
        {{ 0, -1,  0}, {1,  0,  0}, { 0,  0, -1}}, /* bottom/lower-left */
        {{ 1,  0,  0}, { 0, -1,  0}, { 0,  0, -1}}, /* bottom/lower-right */
};
#endif
/* K3DH acceleration data */
struct k3dh_acc {
	s16 x;
	s16 y;
	s16 z;
};

struct k3dh_data {
	struct i2c_client *client;
	struct miscdevice k3dh_device;
	struct k3dh_platform_data *pdata;
	struct mutex read_lock;
	struct mutex write_lock;
	struct completion data_ready;
	struct class *acc_class;
	struct k3dh_acc cal_data;
	u8 ctrl_reg1_shadow;
	atomic_t opened; /* opened implies enabled */
	struct work_struct work;
	struct workqueue_struct *work_queue;
	struct hrtimer timer;
	struct input_dev *input_dev;
	ktime_t poll_delay;
#ifdef CONFIG_MACH_CHIEF
	atomic_t position;
#endif
};

extern struct class *sec_class;

/* Read X,Y and Z-axis acceleration raw data */
static int k3dh_read_accel_raw_xyz(struct k3dh_data *k3dh,
				struct k3dh_acc *acc)
{
	int err;
        s8 reg = AXISDATA_REG | AC;
	u8 acc_data[6];

	err = i2c_smbus_read_i2c_block_data(k3dh->client, reg,
					    sizeof(acc_data), acc_data);
	if (err != sizeof(acc_data)) {
		pr_err("%s : failed to read 6 bytes for getting x/y/z\n",
		       __func__);
		return -EIO;
	}
	
	acc->x = (short)(acc_data[1] << 8) | acc_data[0];
	acc->y = (short)(acc_data[3] << 8) | acc_data[2];
	acc->z = (short)(acc_data[5] << 8) | acc_data[4];

	acc->x = acc->x >> 4;
	acc->y = acc->y >> 4;
	acc->z = acc->z >> 4;

	return 0;
}

static int k3dh_read_accel_xyz(struct k3dh_data *k3dh,
				struct k3dh_acc *acc)
{
	int err = 0;
#ifdef CONFIG_MACH_CHIEF	
        int raw[3], accel_pos_adjusted[3];
	int i, j, value;
	int pos = (int)atomic_read(&k3dh->position);
#endif	
        mutex_lock(&k3dh->read_lock);
	err = k3dh_read_accel_raw_xyz(k3dh, acc);
	mutex_unlock(&k3dh->read_lock);
	if (err < 0) {
		pr_err("%s: k3dh_read_accel_raw_xyz() failed\n", __func__);
		return err;
	}

	acc->x -= k3dh->cal_data.x;
	acc->y -= k3dh->cal_data.y;
	acc->z -= k3dh->cal_data.z;
#ifdef CONFIG_MACH_CHIEF
	raw[0] = acc->x;
	raw[1] = acc->y;
	raw[2] = acc->z;
	for (i = 0; i < 3; i++) {
                value = 0;
                for (j = 0; j < 3; j++) {
                        value += kr3dh_position_map[pos][i][j] * (int)raw[j];
                }
	accel_pos_adjusted[i] = value;
	}
	acc->x = accel_pos_adjusted[0];
	acc->y = accel_pos_adjusted[1];
	acc->z = accel_pos_adjusted[2];
#endif
	return err;
}

static int k3dh_open_calibration(struct k3dh_data *k3dh)
{
	struct file *cal_filp = NULL;
	int err = 0;
	mm_segment_t old_fs;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	cal_filp = filp_open(CALIBRATION_FILE_PATH, O_RDONLY, 0666);
	if (IS_ERR(cal_filp)) {
		pr_err("%s: Can't open calibration file\n", __func__);
		set_fs(old_fs);
		err = PTR_ERR(cal_filp);
		return err;
	}

	err = cal_filp->f_op->read(cal_filp,
		(char *)&k3dh->cal_data, 3 * sizeof(s16), &cal_filp->f_pos);
	if (err != 3 * sizeof(s16)) {
		pr_err("%s: Can't read the cal data from file\n", __func__);
		err = -EIO;
	}

	k3dh_dbgmsg("%s: (%u,%u,%u)\n", __func__,
		k3dh->cal_data.x, k3dh->cal_data.y, k3dh->cal_data.z);

	filp_close(cal_filp, current->files);
	set_fs(old_fs);

	return err;
}

static int k3dh_do_calibrate(struct device *dev)
{
	struct k3dh_data *k3dh = dev_get_drvdata(dev);
	struct k3dh_acc data = { 0, };
	struct file *cal_filp = NULL;
	int sum[3] = { 0, };
	int err = 0;
	int i;
	mm_segment_t old_fs;

	for (i = 0; i < CALIBRATION_DATA_AMOUNT; i++) {
		mutex_lock(&k3dh->read_lock);
		err = k3dh_read_accel_raw_xyz(k3dh, &data);
		mutex_unlock(&k3dh->read_lock);
		if (err < 0) {
			pr_err("%s: k3dh_read_accel_raw_xyz() "
				"failed in the %dth loop\n", __func__, i);
			return err;
		}

		sum[0] += data.x;
		sum[1] += data.y;
		sum[2] += data.z;
	}

	k3dh->cal_data.x = sum[0] / CALIBRATION_DATA_AMOUNT;
	k3dh->cal_data.y = sum[1] / CALIBRATION_DATA_AMOUNT;
	k3dh->cal_data.z = (sum[2] / CALIBRATION_DATA_AMOUNT) - 1024;

	printk(KERN_INFO "%s: cal data (%d,%d,%d)\n", __func__,
			k3dh->cal_data.x, k3dh->cal_data.y, k3dh->cal_data.z);

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	cal_filp = filp_open(CALIBRATION_FILE_PATH,
			O_CREAT | O_TRUNC | O_WRONLY, 0666);
	if (IS_ERR(cal_filp)) {
		pr_err("%s: Can't open calibration file\n", __func__);
		set_fs(old_fs);
		err = PTR_ERR(cal_filp);
		return err;
	}

	err = cal_filp->f_op->write(cal_filp,
		(char *)&k3dh->cal_data, 3 * sizeof(s16), &cal_filp->f_pos);
	if (err != 3 * sizeof(s16)) {
		pr_err("%s: Can't write the cal data to file\n", __func__);
		err = -EIO;
	}

	filp_close(cal_filp, current->files);
	set_fs(old_fs);

	return err;
}

/*  open command for K3DH device file  */
static int k3dh_open(struct inode *inode, struct file *file)
{
	int err = 0;
	struct k3dh_data *k3dh = container_of(file->private_data,
						struct k3dh_data,
						k3dh_device);
	
	printk("%s\n", __func__);
	if (atomic_read(&k3dh->opened) == 0) {
		file->private_data = k3dh;
		err = k3dh_open_calibration(k3dh);
		if (err < 0)
			pr_err("%s: k3dh_open_calibration() failed\n",
				__func__);
		k3dh->ctrl_reg1_shadow = DEFAULT_POWER_ON_SETTING;
		err = i2c_smbus_write_byte_data(k3dh->client, CTRL_REG1,
						DEFAULT_POWER_ON_SETTING);
		if (err)
			pr_err("%s: i2c write ctrl_reg1 failed\n", __func__);
	}

	atomic_add(1, &k3dh->opened);

	return err;
}

/*  release command for K3DH device file */
static int k3dh_close(struct inode *inode, struct file *file)
{
	int err = 0;
	struct k3dh_data *k3dh = file->private_data;
	
	printk("%s\n", __func__);
	atomic_sub(1, &k3dh->opened);
	if (atomic_read(&k3dh->opened) == 0) {
		err = i2c_smbus_write_byte_data(k3dh->client, CTRL_REG1,
								PM_OFF);
		k3dh->ctrl_reg1_shadow = PM_OFF;
	}

	return err;
}

static s64 k3dh_get_delay(struct k3dh_data *k3dh)
{
	int i;
	u8 odr;
	s64 delay = -1;

	odr = k3dh->ctrl_reg1_shadow & ODR_MASK;
	for (i = 0; i < ARRAY_SIZE(odr_delay_table); i++) {
		if (odr == odr_delay_table[i].odr) {
			delay = odr_delay_table[i].delay_ns;
			break;
		}
	}
	return delay;
}

static int k3dh_set_delay(struct k3dh_data *k3dh, s64 delay_ns)
{
	int odr_value = ODRHALF;
	int res = 0;
	int i;
	/* round to the nearest delay that is less than
	 * the requested value (next highest freq)
	 */
	printk(" passed %lldns\n", delay_ns);
	
	for (i = 0; i < ARRAY_SIZE(odr_delay_table); i++) {
		if (delay_ns < odr_delay_table[i].delay_ns)
			break;
	}
	if (i > 0)
		i--;
	printk("matched rate %lldns, odr = 0x%x\n",
			odr_delay_table[i].delay_ns,
			odr_delay_table[i].odr);
	odr_value = odr_delay_table[i].odr;
	delay_ns = odr_delay_table[i].delay_ns;
	
	mutex_lock(&k3dh->write_lock);
	printk("old = %lldns, new = %lldns\n",
		     k3dh_get_delay(k3dh), delay_ns);
	if (odr_value != (k3dh->ctrl_reg1_shadow & ODR_MASK)) {
		u8 ctrl = (k3dh->ctrl_reg1_shadow & ~ODR_MASK);
		ctrl |= odr_value| ENABLE_ALL_AXES;
		k3dh->ctrl_reg1_shadow = ctrl;
		res = i2c_smbus_write_byte_data(k3dh->client, CTRL_REG1, ctrl);
		printk("writing odr value 0x%x\n", odr_value);
	}
	mutex_unlock(&k3dh->write_lock);
	return res;
}

/*  ioctl command for K3DH device file */
static int k3dh_ioctl(struct inode *inode, struct file *file,
		       unsigned int cmd, unsigned long arg)
{
	int err = 0;
	struct k3dh_data *k3dh = file->private_data;
	struct k3dh_acc data;
	s64 delay_ns;

	/* cmd mapping */
	switch (cmd) {
	case K3DH_IOCTL_SET_DELAY:
		if (copy_from_user(&delay_ns, (void __user *)arg,
					sizeof(delay_ns)))
			return -EFAULT;
		err = k3dh_set_delay(k3dh, delay_ns);
		break;
	case K3DH_IOCTL_GET_DELAY:
		delay_ns = k3dh_get_delay(k3dh);
		if (put_user(delay_ns, (s64 __user *)arg))
			return -EFAULT;
		break;
	case K3DH_IOCTL_READ_ACCEL_XYZ:
		err = k3dh_read_accel_xyz(k3dh, &data);
		if (err)
			break;
		if (copy_to_user((void __user *)arg, &data, sizeof(data)))
			return -EFAULT;
		break;
	default:
		err = -EINVAL;
		break;
	}

	return err;
}


static void k3dh_enable(struct k3dh_data *k3dh)
{
	printk("%s\n", __func__);
	i2c_smbus_write_byte_data(k3dh->client, CTRL_REG1,
				  DEFAULT_POWER_ON_SETTING);
	hrtimer_start(&k3dh->timer, k3dh->poll_delay, HRTIMER_MODE_REL);
}

static void k3dh_disable(struct k3dh_data *k3dh)
{
	printk("%s\n", __func__);
	i2c_smbus_write_byte_data(k3dh->client, CTRL_REG1, PM_OFF);
	hrtimer_cancel(&k3dh->timer);
	cancel_work_sync(&k3dh->work);
}

static ssize_t k3dh_delay_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct k3dh_data *k3dh = dev_get_drvdata(dev);

	return sprintf(buf, "%lu\n", ktime_to_ns(k3dh->poll_delay));
}

static ssize_t k3dh_delay_store(struct device *dev,
				 struct device_attribute *attr, const char *buf, size_t count)
{
	int err;
	s64 delay;
	struct k3dh_data *k3dh = dev_get_drvdata(dev);

	err = strict_strtoll(buf, 10, &delay);
	if(err < 0)
		return count;

	hrtimer_cancel(&k3dh->timer);

	printk("k3dh_delay_store old delay : %lld",delay); 
	delay *= NSEC_PER_MSEC;
	printk(" New delay : %lld\n",delay); 
	k3dh->poll_delay = ns_to_ktime(delay);
	
	k3dh_set_delay(k3dh, delay);

	if(atomic_read(&k3dh->opened))
	{
		hrtimer_start(&k3dh->timer, k3dh->poll_delay, HRTIMER_MODE_REL);	
	}

	return count;
}

static ssize_t k3dh_enable_show(struct device *dev,
				 struct device_attribute *attr,
				 char *buf)
{
	struct k3dh_data *k3dh = dev_get_drvdata(dev);
	return sprintf(buf, "%d\n", atomic_read(&k3dh->opened));
}

static ssize_t k3dh_enable_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	struct k3dh_data *k3dh = dev_get_drvdata(dev);
	int enable = simple_strtoul(buf, NULL, 10);
	int enabled = atomic_read(&k3dh->opened);

	if(enable == enabled) {
		return count;
	}

	if(enable) {
		atomic_inc(&k3dh->opened);
		k3dh_enable(k3dh);
	} else {
		if(!atomic_read(&k3dh->opened))
			return count;

		atomic_dec(&k3dh->opened);
		if(!atomic_read(&k3dh->opened)) {
			k3dh_disable(k3dh);
		}
	}
	
	return count;

}

static DEVICE_ATTR(poll_delay, S_IRUGO | S_IWUSR | S_IWGRP| S_IROTH | S_IWOTH, k3dh_delay_show, k3dh_delay_store);
static DEVICE_ATTR(enable, S_IRUGO | S_IWUSR | S_IWGRP| S_IROTH | S_IWOTH, k3dh_enable_show, k3dh_enable_store);

static struct attribute *k3dh_sysfs_attrs[] = {
	&dev_attr_enable.attr,
	&dev_attr_poll_delay.attr,
	NULL
};

static struct attribute_group k3dh_attribute_group = {
	.attrs = k3dh_sysfs_attrs,
};

static enum hrtimer_restart k3dh_timer_func(struct hrtimer *timer)
{
	struct k3dh_data *k3dh
			= container_of(timer, struct k3dh_data, timer);
	queue_work(k3dh->work_queue, &k3dh->work);
	hrtimer_forward_now(&k3dh->timer, k3dh->poll_delay);
	return HRTIMER_RESTART;
}

static void k3dh_work_func(struct work_struct *work)
{
	int err = 0;
	struct k3dh_acc data;
	struct k3dh_data *k3dh = container_of(work, struct k3dh_data, work);

	err = k3dh_read_accel_xyz(k3dh, &data);
	
	#ifdef CONFIG_MACH_AEGIS
		data.x = data.x * X_POSITION;
		data.y = data.y * Y_POSITION;
		data.z = data.z * Z_POSITION;
	#endif	
	input_report_rel(k3dh->input_dev, REL_X, data.x);
	input_report_rel(k3dh->input_dev, REL_Y, data.y);
	input_report_rel(k3dh->input_dev, REL_Z, data.z);
	input_sync(k3dh->input_dev);
}

static int k3dh_suspend(struct device *dev)
{
	int res = 0;
	struct k3dh_data *k3dh = dev_get_drvdata(dev);
	
	printk("%s\n", __func__);
	if (atomic_read(&k3dh->opened) > 0)
		k3dh_disable(k3dh);
		/*res = i2c_smbus_write_byte_data(k3dh->client,
						CTRL_REG1, PM_OFF);*/

	return res;
}

static int k3dh_resume(struct device *dev)
{
	int res = 0;
	struct k3dh_data *k3dh = dev_get_drvdata(dev);

	printk("%s\n", __func__);
	if (atomic_read(&k3dh->opened) > 0)
		k3dh_enable(k3dh);
		/*res = i2c_smbus_write_byte_data(k3dh->client, CTRL_REG1,
						k3dh->ctrl_reg1_shadow);*/

	return res;
}

static const struct dev_pm_ops k3dh_pm_ops = {
	.suspend = k3dh_suspend,
	.resume = k3dh_resume,
};

static const struct file_operations k3dh_fops = {
	.owner = THIS_MODULE,
	.open = k3dh_open,
	.release = k3dh_close,
	.ioctl = k3dh_ioctl,
};

static ssize_t k3dh_fs_read(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct k3dh_data *k3dh = dev_get_drvdata(dev);
	struct k3dh_acc data = { 0, };
	int err = 0;
	int on;

	mutex_lock(&k3dh->write_lock);

	on = atomic_read(&k3dh->opened);
	if (on == 0) {
		err = i2c_smbus_write_byte_data(k3dh->client, CTRL_REG1,
						DEFAULT_POWER_ON_SETTING);
	}

	mutex_unlock(&k3dh->write_lock);

	if (err < 0) {
		pr_err("%s: i2c write ctrl_reg1 failed\n", __func__);
		return err;
	}

	err = k3dh_read_accel_xyz(k3dh, &data);
	if (err < 0) {
		pr_err("%s: k3dh_read_accel_xyz failed\n", __func__);
		return err;
	}

	if (on == 0) {
		mutex_lock(&k3dh->write_lock);
		err = i2c_smbus_write_byte_data(k3dh->client, CTRL_REG1,
								PM_OFF);
		mutex_unlock(&k3dh->write_lock);
		if (err)
			pr_err("%s: i2c write ctrl_reg1 failed\n", __func__);
	}

	return sprintf(buf, "%d,%d,%d\n", data.x, data.y, data.z);
}

static ssize_t k3dh_calibration_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	int err;
	struct k3dh_data *k3dh = dev_get_drvdata(dev);

	err = k3dh_open_calibration(k3dh);
	if (err < 0)
		pr_err("%s: k3dh_open_calibration() failed\n", __func__);

	return sprintf(buf, "%d %d %d\n",
		k3dh->cal_data.x, k3dh->cal_data.y, k3dh->cal_data.z);
}

static ssize_t k3dh_calibration_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	int err;

	err = k3dh_do_calibrate(dev);
	if (err < 0)
		pr_err("%s: k3dh_do_calibrate() failed\n", __func__);

	return count;
}

static DEVICE_ATTR(calibration,
		S_IRUGO | S_IWUSR | S_IWGRP,
		k3dh_calibration_show, k3dh_calibration_store);
static DEVICE_ATTR(acc_file, S_IRUGO | S_IWUSR ,
							k3dh_fs_read, NULL);

#ifdef CONFIG_MACH_CHIEF
static int kr3dh_set_position(struct device *dev, int position)
{
        struct i2c_client *client = to_i2c_client(dev);
        struct k3dh_data *k3dh = i2c_get_clientdata(client);

        return atomic_set(&k3dh->position, position);

}
#endif
static int k3dh_probe(struct i2c_client *client,
		       const struct i2c_device_id *id)
{
	struct k3dh_data *k3dh;
	struct device *dev_t, *dev_cal;
	struct k3dh_platform_data *pdata = client->dev.platform_data;
	struct input_dev *input_dev;
	int err;

	if (!pdata) {
		pr_err("%s: missing pdata!\n", __func__);
		return -ENODEV;
	}

	if (!i2c_check_functionality(client->adapter,
				     I2C_FUNC_SMBUS_WRITE_BYTE_DATA |
				     I2C_FUNC_SMBUS_READ_I2C_BLOCK)) {
		pr_err("%s: i2c functionality check failed!\n", __func__);
		err = -ENODEV;
		goto exit;
	}

	k3dh = kzalloc(sizeof(struct k3dh_data), GFP_KERNEL);
	if (k3dh == NULL) {
		dev_err(&client->dev,
				"failed to allocate memory for module data\n");
		err = -ENOMEM;
		goto exit;
	}

	k3dh->client = client;
	k3dh->pdata = pdata;
	i2c_set_clientdata(client, k3dh);

	init_completion(&k3dh->data_ready);
	mutex_init(&k3dh->read_lock);
	mutex_init(&k3dh->write_lock);
	atomic_set(&k3dh->opened, 0);

	hrtimer_init(&k3dh->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	k3dh->poll_delay = ns_to_ktime(200 * NSEC_PER_MSEC);
	k3dh->timer.function = k3dh_timer_func;
	
	k3dh->work_queue = create_singlethread_workqueue("k3dh_workqueue");
	if(!k3dh->work_queue) {
		err = -ENOMEM;
		pr_err("%s: count not create workqueue\n", __func__);
		goto exit;
	}

	INIT_WORK(&k3dh->work, k3dh_work_func);

	input_dev = input_allocate_device();
	if(!input_dev) {
		pr_err("%s: count not allocate input device\n", __func__);
		err = -ENOMEM;
		goto err_input_allocate;
	}

	input_set_drvdata(input_dev, k3dh);
	input_dev->name = "accelerometer_sensor";
	input_set_capability(input_dev, EV_REL, REL_X);
	input_set_abs_params(input_dev, REL_X, -2048, 2047, 0, 0);	
	input_set_capability(input_dev, EV_REL, REL_Y);
	input_set_abs_params(input_dev, REL_Y, -2048, 2047, 0, 0);
	input_set_capability(input_dev, EV_REL, REL_Z);
	input_set_abs_params(input_dev, REL_Z, -2048, 2047, 0, 0);
	
	err = input_register_device(input_dev);
	if(err < 0) {
		pr_err("%s: could not register input device\n", __func__);
		goto err_input_register;
	}

	k3dh->input_dev = input_dev;
	err = sysfs_create_group(&input_dev->dev.kobj, &k3dh_attribute_group);
	if(err) {
		pr_err("%s: could not create sysfs group\n", __func__);
		goto err_misc_register;
	}

	
	/* sensor HAL expects to find /dev/accelerometer */
	k3dh->k3dh_device.minor = MISC_DYNAMIC_MINOR;
	k3dh->k3dh_device.name = "accelerometer";
	k3dh->k3dh_device.fops = &k3dh_fops;

	err = misc_register(&k3dh->k3dh_device);
	if (err) {
		pr_err("%s: misc_register failed\n", __FILE__);
		goto err_misc_register;
	}

	/* creating class/device for test */
	k3dh->acc_class = class_create(THIS_MODULE, "accelerometer");
	if (IS_ERR(k3dh->acc_class)) {
		pr_err("%s: class create failed(accelerometer)\n", __func__);
		err = PTR_ERR(k3dh->acc_class);
		goto err_class_create;
	}

	dev_t = device_create(k3dh->acc_class, NULL,
				MKDEV(ACC_DEV_MAJOR, 0), "%s", "accelerometer");
	if (IS_ERR(dev_t)) {
		pr_err("%s: class create failed(accelerometer)\n", __func__);
		err = PTR_ERR(dev_t);
		goto err_acc_device_create;
	}

	err = device_create_file(dev_t, &dev_attr_acc_file);
	if (err < 0) {
		pr_err("%s: Failed to create device file(%s)\n",
				__func__, dev_attr_acc_file.attr.name);
		goto err_acc_device_create_file;
	}
	dev_set_drvdata(dev_t, k3dh);

	/* creating device for calibration */
	dev_cal = device_create(sec_class, NULL, 0, NULL, "gsensorcal");
	if (IS_ERR(dev_cal)) {
		pr_err("%s: class create failed(gsensorcal)\n", __func__);
		err = PTR_ERR(dev_cal);
		goto err_cal_device_create;
	}

#ifdef CONFIG_MACH_CHIEF
	kr3dh_set_position(&client->dev, CONFIG_INPUT_KR3DH_POSITION);
#endif
	err = device_create_file(dev_cal, &dev_attr_calibration);
	if (err < 0) {
		pr_err("%s: Failed to create device file(%s)\n",
				__func__, dev_attr_calibration.attr.name);
		goto err_cal_device_create_file;
	}
	dev_set_drvdata(dev_cal, k3dh);

	return 0;

err_cal_device_create_file:
	device_destroy(sec_class, 0);
err_cal_device_create:
	device_remove_file(dev_t, &dev_attr_acc_file);
err_acc_device_create_file:
	device_destroy(k3dh->acc_class, MKDEV(ACC_DEV_MAJOR, 0));
err_acc_device_create:
	class_destroy(k3dh->acc_class);
err_class_create:
	misc_deregister(&k3dh->k3dh_device);
err_misc_register:
	mutex_destroy(&k3dh->read_lock);
	mutex_destroy(&k3dh->write_lock);
err_input_register:
	input_free_device(input_dev);
err_input_allocate:
	destroy_workqueue(k3dh->work_queue);
	kfree(k3dh);
exit:
	return err;
}

static int k3dh_remove(struct i2c_client *client)
{
	struct k3dh_data *k3dh = i2c_get_clientdata(client);

	device_destroy(sec_class, 0);
	device_destroy(k3dh->acc_class, MKDEV(ACC_DEV_MAJOR, 0));
	class_destroy(k3dh->acc_class);
	misc_deregister(&k3dh->k3dh_device);
	mutex_destroy(&k3dh->read_lock);
	mutex_destroy(&k3dh->write_lock);
	kfree(k3dh);

	return 0;
}

static const struct i2c_device_id k3dh_id[] = {
	{ "k3dh", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, k3dh_id);

static struct i2c_driver k3dh_driver = {
	.probe = k3dh_probe,
	.remove = __devexit_p(k3dh_remove),
	.id_table = k3dh_id,
	.driver = {
		.pm = &k3dh_pm_ops,
		.owner = THIS_MODULE,
		.name = "k3dh",
	},
};

static int __init k3dh_init(void)
{
	return i2c_add_driver(&k3dh_driver);
}

static void __exit k3dh_exit(void)
{
	i2c_del_driver(&k3dh_driver);
}

module_init(k3dh_init);
module_exit(k3dh_exit);

MODULE_DESCRIPTION("k3dh accelerometer driver");
MODULE_AUTHOR("tim.sk.lee@samsung.com");
MODULE_LICENSE("GPL");
