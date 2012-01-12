/*
 * Copyright 2006-2010, Cypress Semiconductor Corporation.
 * Copyright (C) 2010, Samsung Electronics Co. Ltd. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor
 * Boston, MA  02110-1301, USA.
 *
 */
#include <mach/gpio.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/earlysuspend.h>
#include <linux/input/cypress-touchkey.h>

#define TOUCH_UPDATE
#if defined(TOUCH_UPDATE)
#include <linux/irq.h>
#include <mach/regs-gpio.h>
#include <plat/gpio-cfg.h>
#include <asm/gpio.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <mach/gpio.h>
#endif
#define SCANCODE_MASK		0x07
#define UPDOWN_EVENT_MASK	0x08
#define ESD_STATE_MASK		0x10

#define BACKLIGHT_ON		0x10
#define BACKLIGHT_OFF		0x20

#define DEVICE_NAME "melfas_touchkey"
#define I2C_M_WR 0              /* for i2c */

struct cypress_touchkey_devdata {
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct touchkey_platform_data *pdata;
	struct early_suspend early_suspend;
	u8 backlight_on;
	u8 backlight_off;
	bool is_dead;
	bool is_powering_on;
	bool has_legacy_keycode;
};

static struct cypress_touchkey_devdata *devdata;
static int touch_key_led_on_off=0;

extern void touch_keypad_gpio_init(void);

static int i2c_touchkey_read_byte(struct cypress_touchkey_devdata *devdata,
					u8 *val)
{
	int ret;
	int retry = 2;

	while (true) {
		ret = i2c_smbus_read_byte(devdata->client);
		if (ret >= 0) {
			*val = ret;
			return 0;
		}

		dev_err(&devdata->client->dev, "i2c read error\n");
		if (!retry--)
			break;
		msleep(10);
	}

	return ret;
}

static int i2c_touchkey_write_byte(struct cypress_touchkey_devdata *devdata,
					u8 val)
{
	int ret;
	int retry = 2;

	while (true) {
		ret = i2c_smbus_write_byte(devdata->client, val);
		if (!ret)
			return 0;

		dev_err(&devdata->client->dev, "i2c write error\n");
		if (!retry--)
			break;
		msleep(10);
	}

	return ret;
}

static void all_keys_up(struct cypress_touchkey_devdata *devdata)
{
	int i;

	for (i = 0; i < devdata->pdata->keycode_cnt; i++)
		input_report_key(devdata->input_dev,
						devdata->pdata->keycode[i], 0);

	input_sync(devdata->input_dev);
}

static int recovery_routine(struct cypress_touchkey_devdata *devdata)
{
	int ret = -1;
	int retry = 10;
	u8 data;
	int irq_eint;

	if (unlikely(devdata->is_dead)) {
		dev_err(&devdata->client->dev, "%s: Device is already dead, "
				"skipping recovery\n", __func__);
		return -ENODEV;
	}

	irq_eint = devdata->client->irq;

	all_keys_up(devdata);

	disable_irq_nosync(irq_eint);
	while (retry--) {
		devdata->pdata->touchkey_onoff(TOUCHKEY_OFF);
		devdata->pdata->touchkey_onoff(TOUCHKEY_ON);
		ret = i2c_touchkey_read_byte(devdata, &data);
		if (!ret) {
			enable_irq(irq_eint);
			goto out;
		}
		dev_err(&devdata->client->dev, "%s: i2c transfer error retry = "
				"%d\n", __func__, retry);
	}
	devdata->is_dead = true;
	devdata->pdata->touchkey_onoff(TOUCHKEY_OFF);
	dev_err(&devdata->client->dev, "%s: touchkey died\n", __func__);
out:
	return ret;
}

extern unsigned int touch_state_val;
extern void TSP_forced_release(void);

static irqreturn_t touchkey_interrupt_thread(int irq, void *touchkey_devdata)
{
	u8 data;
	int i;
	int ret;
	int scancode;
	struct cypress_touchkey_devdata *devdata = touchkey_devdata;

	ret = i2c_touchkey_read_byte(devdata, &data);
	if (ret || (data & ESD_STATE_MASK)) {
		ret = recovery_routine(devdata);
		if (ret) {
			printk("%s: touchkey recovery "
					"failed!\n", __func__);
			goto err;
		}
	}
  
	if (data & UPDOWN_EVENT_MASK) {
		scancode = (data & SCANCODE_MASK) - 1;
		input_report_key(devdata->input_dev,
			devdata->pdata->keycode[scancode], 0);
		input_sync(devdata->input_dev);
#ifdef CONFIG_SEC_KEY_DBG							
		printk("[t_key]R:%d \n", devdata->pdata->keycode[scancode]);
#else
		printk("[t_key] released\n");
#endif
	} else {
		if (!touch_state_val) {
			if (devdata->has_legacy_keycode) {
				scancode = (data & SCANCODE_MASK) - 1;
				if (scancode < 0 || scancode >= devdata->pdata->keycode_cnt) {
					printk("%s: scancode is out of "
						"range\n", __func__);
					goto err;
				}
				if (scancode == 1)
					TSP_forced_release();
				input_report_key(devdata->input_dev,
					devdata->pdata->keycode[scancode], 1);
#ifdef CONFIG_SEC_KEY_DBG									
				printk("[t_key]P:%d\n", devdata->pdata->keycode[scancode]);
#else
				printk("[t_key] pressed\n");
#endif
			} else {
				for (i = 0; i < devdata->pdata->keycode_cnt; i++)
				input_report_key(devdata->input_dev,
					devdata->pdata->keycode[i],
					!!(data & (1U << i)));
			}
			input_sync(devdata->input_dev);
		}
	}
err:
	return IRQ_HANDLED;
}

static irqreturn_t touchkey_interrupt_handler(int irq, void *touchkey_devdata)
{
	struct cypress_touchkey_devdata *devdata = touchkey_devdata;

	if (devdata->is_powering_on) {
		dev_dbg(&devdata->client->dev, "%s: ignoring spurious boot "
					"interrupt\n", __func__);
		return IRQ_HANDLED;
	}

	return IRQ_WAKE_THREAD;
}

static int i2c_touchkey_multi_write(struct cypress_touchkey_devdata *devdata, 
						u8 * val, unsigned int len)
{
	int err;
	struct i2c_msg msg[1];
	int retry = 2;

	while (retry--) {
		msg->addr = devdata->client->addr;
		msg->flags = I2C_M_WR;
		msg->len = len;
		msg->buf = val;
		err = i2c_transfer(devdata->client->adapter, msg, 1);
		if (err >= 0)
			return 0;
		printk(KERN_DEBUG "%s %d i2c transfer error\n", __func__,
		       __LINE__);
		mdelay(10);
	}
	return err;
}

static void touchkey_auto_calibration(int autocal_on_off)
{
	signed char data[4] = {0x50,0x00,0x00,0x01}; 
	printk("enter touchkey_auto_calibration");
		
	i2c_touchkey_multi_write(devdata,data, 4);	
	mdelay(10); 
}

static int i2c_touchkey_read(struct cypress_touchkey_devdata *devdata, 
					u8 reg, u8 * val, unsigned int len)
{
	int err;
	int retry = 10;
	struct i2c_msg msg[1];

	while (retry--) {
		msg->addr = devdata->client->addr;
		msg->flags = I2C_M_RD;
		msg->len = len;
		msg->buf = val;
		err = i2c_transfer(devdata->client->adapter, msg, 1);
		if (err >= 0) {
			return 0;
		}
		printk("%s %d i2c transfer error\n", __func__, __LINE__);	/* add by inter.park */
		mdelay(10);
	}
	return err;

}

static int i2c_touchkey_write(struct cypress_touchkey_devdata *devdata, 
						 u8 * val, unsigned int len)
 {
	 int err;
	 struct i2c_msg msg[1];
	 unsigned char data[2];
	 int retry = 2;
 
	 while (retry--) {
		 data[0] = *val;
		 msg->addr = devdata->client->addr;
		 msg->flags = I2C_M_WR;
		 msg->len = len;
		 msg->buf = data;
		 err = i2c_transfer(devdata->client->adapter, msg, 1);
		 if (err >= 0)
			 return 0;
		 printk(KERN_DEBUG "%s %d i2c transfer error\n", __func__,
				__LINE__);
		 mdelay(10);
	 }
	 return err;
 }

#ifdef CONFIG_HAS_EARLYSUSPEND
static void cypress_touchkey_early_suspend(struct early_suspend *h)
{
	struct cypress_touchkey_devdata *devdata =
		container_of(h, struct cypress_touchkey_devdata, early_suspend);

	printk("%s\n",__FUNCTION__);

	devdata->is_powering_on = true;

	if (unlikely(devdata->is_dead))
		return;

	disable_irq(devdata->client->irq);
	devdata->pdata->touchkey_onoff(TOUCHKEY_OFF);

	all_keys_up(devdata);
}

static void cypress_touchkey_early_resume(struct early_suspend *h)
{
	struct cypress_touchkey_devdata *devdata =
		container_of(h, struct cypress_touchkey_devdata, early_suspend);

	printk("%s\n",__FUNCTION__);
	u8 data[6];

	devdata->pdata->touchkey_onoff(TOUCHKEY_ON);
#if 0
	if (i2c_touchkey_write_byte(devdata, devdata->backlight_on)) {
		devdata->is_dead = true;
		devdata->pdata->touchkey_onoff(TOUCHKEY_OFF);
		dev_err(&devdata->client->dev, "%s: touch keypad not responding"
				" to commands, disabling\n", __func__);
		return;
	}
#endif
	devdata->is_dead = false;
	enable_irq(devdata->client->irq);
	devdata->is_powering_on = false;
	msleep(50);
	touchkey_auto_calibration(1/*on*/);
	msleep(200); 
	if(touch_key_led_on_off== 1)
		i2c_touchkey_write(devdata, &devdata->backlight_on, 1); 
}
#endif

#if defined(TOUCH_UPDATE)
extern int get_touchkey_firmware(char *version);
extern int ISSP_main();
static int touchkey_update_status = 0;
struct work_struct touch_update_work;
struct workqueue_struct *touchkey_wq;
#define IRQ_TOUCH_INT (IRQ_EINT_GROUP18_BASE + 6)
#define KEYCODE_REG 0x00
#define FIRMWARE_VERSION 0x01

static void init_hw(void)
{
#if 0
	gpio_direction_output(_3_GPIO_TOUCH_EN, 1);
	msleep(200);
	s3c_gpio_setpull(_3_GPIO_TOUCH_INT, S3C_GPIO_PULL_NONE);
	set_irq_type(IRQ_TOUCH_INT, IRQF_TRIGGER_FALLING);
	s3c_gpio_cfgpin(_3_GPIO_TOUCH_INT, _3_GPIO_TOUCH_INT_AF);
#endif
}


static int touchkey_update_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t touchkey_update_read(struct file * filp, char *buf, size_t count,
			     loff_t * f_pos)
{
	char data[3] = { 0, };

	get_touchkey_firmware(data);
	put_user(data[1], buf);

	return 1;
}

#if 0
extern int mcsdl_download_binary_file(unsigned char *pData,
				      unsigned short nBinary_length);
ssize_t touchkey_update_write(struct file *filp, const char *buf, size_t count,
			      loff_t * f_pos)
{
	unsigned char *pdata;

	disable_irq(IRQ_TOUCH_INT);
	printk("count = %d\n", count);
	pdata = kzalloc(count, GFP_KERNEL);
	if (pdata == NULL) {
		printk("memory allocate fail \n");
		return 0;
	}
	if (copy_from_user(pdata, buf, count)) {
		printk("copy fail \n");
		kfree(pdata);
		return 0;
	}

	mcsdl_download_binary_file((unsigned char *)pdata,
				   (unsigned short)count);
	kfree(pdata);

	init_hw();
	enable_irq(IRQ_TOUCH_INT);
	return count;
}
#endif

static int touchkey_update_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static const struct file_operations touchkey_update_fops = {
	.owner = THIS_MODULE,
	.read = touchkey_update_read,
	//.write   = touchkey_update_write,
	.open = touchkey_update_open,
	.release = touchkey_update_release,
};

static struct miscdevice touchkey_update_device = {
	.minor = MISC_DYNAMIC_MINOR,
 	.name = DEVICE_NAME,
	.fops = &touchkey_update_fops,
};

static ssize_t touch_version_read(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	char data[16],*write_val=0x40,ret;
	int count,key_sensitivity;
	char i;
	ret = i2c_touchkey_write(devdata, &write_val, 1);
	if (ret){
		for(i=0;i<10;i++){
			msleep(10);
			printk("[t_key] version re-check\n");
			ret = i2c_touchkey_write(devdata, &write_val, 1);
			if(ret==0)
				break;
		}
	}		

	if(ret){
		printk("%s: touchkey i2c failed\n", __func__);		
		count = sprintf(buf, "%s\n", "PBA");		
		return count;
	}	

	i2c_touchkey_read(devdata, KEYCODE_REG, data, 16);

	count = sprintf(buf, "0x%x\n", data[1]);

	printk("[touch key] firmware=0x%x,module=0x%x\n", data[1], data[2]);

	return count;
}


static ssize_t touch_version_write(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t size)
{
	printk("input data --> %s\n", buf);

	return size;
}

static void touchkey_update_func(struct work_struct *p)
{
	int retry = 10;
	touchkey_update_status = 1;
	printk("%s start\n", __FUNCTION__);
	while (retry--) {
		if (ISSP_main() == 0) {
			touchkey_update_status = 0;
			printk("touchkey_update succeeded\n");
			enable_irq(IRQ_TOUCH_INT);
			return;
		}
	}
	touchkey_update_status = -1;
	printk("touchkey_update failed\n");
	return;
}

static ssize_t touch_update_write(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t size)
{
	printk("touchkey firmware update \n");
	if (*buf == 'S') {
		disable_irq(IRQ_TOUCH_INT);
		INIT_WORK(&touch_update_work, touchkey_update_func);
		queue_work(touchkey_wq, &touch_update_work);
	}
	return size;
}

static ssize_t touch_update_read(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	int count = 0;

	printk("touch_update_read: touchkey_update_status %d\n",
	       touchkey_update_status);

	if (touchkey_update_status == 0) {
		count = sprintf(buf, "PASS\n");
	} else if (touchkey_update_status == 1) {
		count = sprintf(buf, "Downloading\n");
	} else if (touchkey_update_status == -1) {
		count = sprintf(buf, "Fail\n");
	}

	return count;
}

static ssize_t touch_led_control(struct device *dev,
				 struct device_attribute *attr, const char *buf,
				 size_t size)
{
	struct cypress_touchkey_devdata *devdata = dev_get_drvdata(dev);
	int ret;

	if (strncmp(buf, "1", 1) == 0)
		touch_key_led_on_off = 1;
	else
		touch_key_led_on_off = 0;

	if (devdata && !devdata->is_powering_on) {
		if (touch_key_led_on_off == 1)
			ret = i2c_touchkey_write(devdata, &devdata->backlight_on, 1);
		else if(touch_key_led_on_off ==0 )
			ret = i2c_touchkey_write(devdata, &devdata->backlight_off, 1);

		if (ret)
			dev_err(dev, "%s: touchkey led i2c failed\n", __func__);
	}
	return size;
}

static ssize_t touchkey_enable_disable(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t size)
{
#if 0
	printk("touchkey_enable_disable %c \n", *buf);
	if (*buf == '0') {
		set_touchkey_debug('d');
		disable_irq(IRQ_TOUCH_INT);
		gpio_direction_output(_3_GPIO_TOUCH_EN, 0);
		touchkey_enable = -2;
	} else if (*buf == '1') {
		if (touchkey_enable == -2) {
			set_touchkey_debug('e');
			gpio_direction_output(_3_GPIO_TOUCH_EN, 1);
			touchkey_enable = 1;
			enable_irq(IRQ_TOUCH_INT);
		}
	} else {
		printk("touchkey_enable_disable: unknown command %c \n", *buf);
	}
#endif
	return size;
}

static ssize_t touchkey_menu_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	char data[16],*write_val=0x40,ret;
	int count,key_sensitivity;
	char i;
	ret = i2c_touchkey_write(devdata, &write_val, 1);
	if (ret)
		printk("%s: touchkey i2c failed\n", __func__);

	i2c_touchkey_read(devdata, KEYCODE_REG, data, 16);

	key_sensitivity=((0x00FF&data[10])<<8)|data[11];
	count = sprintf(buf, "%d\n", key_sensitivity);

	printk(" key_sensitivity_menu=%d\n", key_sensitivity);

	return count;
}

static ssize_t touchkey_home_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
		char data[16],*write_val=0x40,ret;
		int count,key_sensitivity;
		char i;
		ret = i2c_touchkey_write(devdata, &write_val, 1);
		if (ret)
			printk("%s: touchkey i2c failed\n", __func__);
	
		i2c_touchkey_read(devdata, KEYCODE_REG, data, 16);
	
		key_sensitivity=((0x00FF&data[12])<<8)|data[13];
		count = sprintf(buf, "%d\n", key_sensitivity);
	
		printk(" key_sensitivity_home=%d\n", key_sensitivity);

		return count;
}

static ssize_t touchkey_back_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
		char data[18],*write_val=0x40,ret;
		int count,key_sensitivity;
		char i;
		ret = i2c_touchkey_write(devdata, &write_val, 1);
		if (ret)
			printk("%s: touchkey i2c failed\n", __func__);
	
		i2c_touchkey_read(devdata, KEYCODE_REG, data, 18);
	
		key_sensitivity=((0x00FF&data[16])<<8)|data[17];
		count = sprintf(buf, "%d\n", key_sensitivity);
	
		printk(" key_sensitivity_back=%d\n", key_sensitivity);

		return count;
}

static ssize_t touchkey_search_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
		char data[16],*write_val=0x40,ret;
		int count,key_sensitivity;
		char i;
		ret = i2c_touchkey_write(devdata, &write_val, 1);
		if (ret)
			printk("%s: touchkey i2c failed\n", __func__);
	
		i2c_touchkey_read(devdata, KEYCODE_REG, data, 16);
	
		key_sensitivity=((0x00FF&data[14])<<8)|data[15];
		count = sprintf(buf, "%d\n", key_sensitivity);
	
		printk(" key_sensitivity_search=%d\n", key_sensitivity);

		return count;
}

static ssize_t touch_sensitivity_control(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	int ret;
	unsigned char data = 0x40;
	ret = i2c_touchkey_write(devdata, &data, 1);	
	return ret;
}

static ssize_t touchkey_idac0_show(struct device *dev,
	struct device_attribute *attr, char *buf)
	{
		char data[10]={0,0,0,0,0,0,0,0,0,0},*write_val=0x40,ret;
		int count,key_sensitivity;
		
		ret = i2c_touchkey_write(devdata, &write_val, 1);
		if (ret)
			printk("%s: touchkey i2c failed\n", __func__);
	
		i2c_touchkey_read(devdata, KEYCODE_REG, data, 10);
	
		key_sensitivity=data[6];
		count = sprintf(buf, "%d\n", key_sensitivity);
	
		printk(" key_sensitivity=%d\n", key_sensitivity);
		return count;
	}


static ssize_t touchkey_idac1_show(struct device *dev,
	struct device_attribute *attr, char *buf)
	{
		char data[10]={0,0,0,0,0,0,0,0,0,0},*write_val=0x40,ret;
		int count,key_sensitivity;
		
		ret = i2c_touchkey_write(devdata, &write_val, 1);
		if (ret)
			printk("%s: touchkey i2c failed\n", __func__);
	
		i2c_touchkey_read(devdata, KEYCODE_REG, data, 10);
	
		key_sensitivity=data[7];
		count = sprintf(buf, "%d\n", key_sensitivity);
	
		printk(" key_sensitivity=%d\n", key_sensitivity);
		return count;
	}


static ssize_t touchkey_idac2_show(struct device *dev,
	struct device_attribute *attr, char *buf)
	{
		char data[10]={0,0,0,0,0,0,0,0,0,0},*write_val=0x40,ret;
		int count,key_sensitivity;
		
		ret = i2c_touchkey_write(devdata, &write_val, 1);
		if (ret)
			printk("%s: touchkey i2c failed\n", __func__);
	
		i2c_touchkey_read(devdata, KEYCODE_REG, data, 10);
	
		key_sensitivity=data[9];
		count = sprintf(buf, "%d\n", key_sensitivity);
	
		printk(" key_sensitivity=%d\n", key_sensitivity);
		return count;
	}


static ssize_t touchkey_idac3_show(struct device *dev,
	struct device_attribute *attr, char *buf)
	{
		char data[10]={0,0,0,0,0,0,0,0,0,0},*write_val=0x40,ret;
		int count,key_sensitivity;
		
		ret = i2c_touchkey_write(devdata, &write_val, 1);
		if (ret)
			printk("%s: touchkey i2c failed\n", __func__);
	
		i2c_touchkey_read(devdata, KEYCODE_REG, data, 10);
	
		key_sensitivity=data[8];
		count = sprintf(buf, "%d\n", key_sensitivity);
	
		printk(" key_sensitivity=%d\n", key_sensitivity);
		return count;
	}

static DEVICE_ATTR(touch_version, 0664, touch_version_read, touch_version_write);
static DEVICE_ATTR(touch_update, 0664, touch_update_read, touch_update_write);
static DEVICE_ATTR(brightness, 0664, NULL, touch_led_control);
static DEVICE_ATTR(enable_disable, 0664, NULL, touchkey_enable_disable);
static DEVICE_ATTR(touchkey_menu, 0664, touchkey_menu_show, NULL);
static DEVICE_ATTR(touchkey_home, 0664, touchkey_home_show, NULL);
static DEVICE_ATTR(touchkey_back, 0664, touchkey_back_show, NULL);
static DEVICE_ATTR(touchkey_search, 0664, touchkey_search_show, NULL);
static DEVICE_ATTR(touchkey_idac0, 0664, touchkey_idac0_show, NULL);
static DEVICE_ATTR(touchkey_idac1, 0664, touchkey_idac1_show, NULL);
static DEVICE_ATTR(touchkey_idac2, 0664, touchkey_idac2_show, NULL);
static DEVICE_ATTR(touchkey_idac3, 0664, touchkey_idac3_show, NULL);
static DEVICE_ATTR(touch_sensitivity, 0664, NULL, touch_sensitivity_control);
#endif
static int cypress_touchkey_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct input_dev *input_dev;

	u8 data[6];
	int err;
	int cnt;
	int touch_auto_calibration_on_off = 0;
#if defined(TOUCH_UPDATE)
	int ret;
	int retry = 10;
#endif
	printk(" cypress_touchkey_probe\n");

	gpio_set_value(TOUCHKEY_VDD_EN, 1);
	msleep(70);

	touch_keypad_gpio_init();

	if (!dev->platform_data) {
		dev_err(dev, "%s: Platform data is NULL\n", __func__);
		return -EINVAL;
	}

	devdata = kzalloc(sizeof(*devdata), GFP_KERNEL);
	if (devdata == NULL) {
		dev_err(dev, "%s: failed to create our state\n", __func__);
		return -ENODEV;
	}

	devdata->client = client;
	i2c_set_clientdata(client, devdata);

	devdata->pdata = client->dev.platform_data;
	if (!devdata->pdata->keycode) {
		dev_err(dev, "%s: Invalid platform data\n", __func__);
		err = -EINVAL;
		goto err_null_keycodes;
	}

	strlcpy(devdata->client->name, DEVICE_NAME, I2C_NAME_SIZE);

	input_dev = input_allocate_device();
	if (!input_dev) {
		err = -ENOMEM;
		goto err_input_alloc_dev;
	}

	devdata->input_dev = input_dev;
	dev_set_drvdata(&input_dev->dev, devdata);
	input_dev->name = "sec_touchkey";
	input_dev->id.bustype = BUS_HOST;

	for (cnt = 0; cnt < devdata->pdata->keycode_cnt; cnt++)
		input_set_capability(input_dev, EV_KEY,
					devdata->pdata->keycode[cnt]);

	err = input_register_device(input_dev);
	if (err)
		goto err_input_reg_dev;

	devdata->is_powering_on = true;

	devdata->pdata->touchkey_onoff(TOUCHKEY_ON);

	err = i2c_master_recv(client, data, sizeof(data));
	if (err < sizeof(data)) {
		if (err >= 0)
			err = -EIO;
		dev_err(dev, "%s: error reading hardware version\n", __func__);
		goto err_read;
	}

	dev_info(dev, "%s: hardware rev1 = %#02x, rev2 = %#02x\n", __func__,
				data[1], data[2]);

	devdata->backlight_on = BACKLIGHT_ON;
	devdata->backlight_off = BACKLIGHT_OFF;

	devdata->has_legacy_keycode = 1;
#if 0
	err = i2c_touchkey_write_byte(devdata, devdata->backlight_on);
	if (err) {
		dev_err(dev, "%s: touch keypad backlight on failed\n",
				__func__);
		goto err_backlight_on;
	}
#endif
	set_irq_type((IRQ_EINT_GROUP18_BASE + 6), IRQ_TYPE_LEVEL_LOW); // IRQ_TYPE_EDGE_FALLING);
	s3c_gpio_cfgpin(_3_GPIO_TOUCH_INT, S3C_GPIO_SFN(0xf));
	s3c_gpio_setpull(_3_GPIO_TOUCH_INT, S3C_GPIO_PULL_NONE);

	if (request_threaded_irq(client->irq, touchkey_interrupt_handler,
				touchkey_interrupt_thread, IRQF_TRIGGER_FALLING,
				DEVICE_NAME, devdata)) {
		dev_err(dev, "%s: Can't allocate irq.\n", __func__);
		goto err_req_irq;
	}
	mdelay(50); 
	touchkey_auto_calibration(1);
	mdelay(200); 
	i2c_touchkey_read	(devdata,0x00, data, 6);
	touch_auto_calibration_on_off = (data[5] & 0x80)>>7;
	printk("touchkey_auto_calibration=%d,data=%x \n",touch_auto_calibration_on_off,data[5]);

#ifdef CONFIG_HAS_EARLYSUSPEND
	devdata->early_suspend.suspend = cypress_touchkey_early_suspend;
	devdata->early_suspend.resume = cypress_touchkey_early_resume;
#endif
	register_early_suspend(&devdata->early_suspend);

	devdata->is_powering_on = false;
#if defined(TOUCH_UPDATE)
	ret = misc_register(&touchkey_update_device);
	if (ret) {
		printk("%s misc_register fail\n", __FUNCTION__);
		goto err_misc_reg;
	}

	dev_set_drvdata(touchkey_update_device.this_device, devdata);

	if (device_create_file
	    (touchkey_update_device.this_device, &dev_attr_touch_version) < 0) {
		printk("%s device_create_file fail dev_attr_touch_version\n",
		       __FUNCTION__);
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_touch_version.attr.name);
	}

	if (device_create_file
	    (touchkey_update_device.this_device, &dev_attr_touch_update) < 0) {
		printk("%s device_create_file fail dev_attr_touch_update\n",
		       __FUNCTION__);
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_touch_update.attr.name);
	}

	if (device_create_file
	    (touchkey_update_device.this_device, &dev_attr_brightness) < 0) {
		printk("%s device_create_file fail dev_attr_touch_update\n",
		       __FUNCTION__);
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_brightness.attr.name);
	}

	if (device_create_file
	    (touchkey_update_device.this_device,
	     &dev_attr_enable_disable) < 0) {
		printk("%s device_create_file fail dev_attr_touch_update\n",
		       __FUNCTION__);
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_enable_disable.attr.name);
	}

	if (device_create_file
	    (touchkey_update_device.this_device, &dev_attr_touchkey_menu) < 0) {
		printk("%s device_create_file fail dev_attr_touch_version\n",
		       __FUNCTION__);
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_touch_version.attr.name);
	}

	if (device_create_file
	    (touchkey_update_device.this_device, &dev_attr_touchkey_home) < 0) {
		printk("%s device_create_file fail dev_attr_touch_version\n",
		       __FUNCTION__);
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_touch_version.attr.name);
	}

	if (device_create_file
	    (touchkey_update_device.this_device, &dev_attr_touchkey_back) < 0) {
		printk("%s device_create_file fail dev_attr_touch_version\n",
		       __FUNCTION__);
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_touch_version.attr.name);
	}

	if (device_create_file
	    (touchkey_update_device.this_device, &dev_attr_touchkey_search) < 0) {
		printk("%s device_create_file fail dev_attr_touch_version\n",
		       __FUNCTION__);
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_touch_version.attr.name);
	}

	if (device_create_file
	    (touchkey_update_device.this_device, &dev_attr_touch_sensitivity) < 0) {
		printk("%s device_create_file fail dev_attr_touch_version\n",
		       __FUNCTION__);
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_touch_version.attr.name);
	}

	if (device_create_file
	    (touchkey_update_device.this_device, &dev_attr_touchkey_idac0) < 0) {
		printk("%s device_create_file fail dev_attr_touch_version\n",
		       __FUNCTION__);
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_touch_version.attr.name);
	}

	if (device_create_file
	    (touchkey_update_device.this_device, &dev_attr_touchkey_idac1) < 0) {
		printk("%s device_create_file fail dev_attr_touch_version\n",
		       __FUNCTION__);
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_touch_version.attr.name);
	}

	if (device_create_file
	    (touchkey_update_device.this_device, &dev_attr_touchkey_idac2) < 0) {
		printk("%s device_create_file fail dev_attr_touch_version\n",
		       __FUNCTION__);
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_touch_version.attr.name);
	}

	if (device_create_file
	    (touchkey_update_device.this_device, &dev_attr_touchkey_idac3) < 0) {
		printk("%s device_create_file fail dev_attr_touch_version\n",
		       __FUNCTION__);
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_touch_version.attr.name);
	}

	touchkey_wq = create_singlethread_workqueue(DEVICE_NAME);
	if (!touchkey_wq)
		goto err_create_wq;

	while (retry--) {
		if (get_touchkey_firmware(data) == 0)	//melfas need delay for multiple read
			break;
	}
	printk("%s F/W version: 0x%x, Module version:0x%x\n", __FUNCTION__,
	       data[1], data[2]);
#endif

	return 0;

err_create_wq:
#if defined(TOUCH_UPDATE)
	misc_deregister(&touchkey_update_device);
#endif
err_misc_reg:
err_req_irq:
err_backlight_on:
err_read:
	devdata->pdata->touchkey_onoff(TOUCHKEY_OFF);
	input_unregister_device(input_dev);
	goto err_input_alloc_dev;
err_input_reg_dev:
	input_free_device(input_dev);
err_input_alloc_dev:
err_null_keycodes:
	kfree(devdata);
	return err;
}

static int __devexit i2c_touchkey_remove(struct i2c_client *client)
{
	struct cypress_touchkey_devdata *devdata = i2c_get_clientdata(client);

#if defined(TOUCH_UPDATE)
	misc_deregister(&touchkey_update_device);
#endif
	unregister_early_suspend(&devdata->early_suspend);
	/* If the device is dead IRQs are disabled, we need to rebalance them */
	if (unlikely(devdata->is_dead))
		enable_irq(client->irq);
	else
		devdata->pdata->touchkey_onoff(TOUCHKEY_OFF);
	free_irq(client->irq, devdata);
	all_keys_up(devdata);
	input_unregister_device(devdata->input_dev);
	kfree(devdata);
	return 0;
}

static const struct i2c_device_id cypress_touchkey_id[] = {
	{ CYPRESS_TOUCHKEY_DEV_NAME, 0 },
};

MODULE_DEVICE_TABLE(i2c, cypress_touchkey_id);

struct i2c_driver touchkey_i2c_driver = {
	.driver = {
		.name = "cypress_touchkey_driver",
	},
	.id_table = cypress_touchkey_id,
	.probe = cypress_touchkey_probe,
	.remove = __devexit_p(i2c_touchkey_remove),
};

static int __init touchkey_init(void)
{
	int ret = 0;
	int retry = 3;
	printk(" touchkey_init\n");
	#if 0
	//update version "eclair/vendor/samsung/apps/Lcdtest/src/com/sec/android/app/lcdtest/touch_firmware.java"
	//if ((data[1] >= 0xa1) && (data[1] < 0xa9)) {
		//set_touchkey_debug('U');
		while (retry--) {
			if (ISSP_main() == 0) {
				printk("touchkey_update succeeded\n");
				//set_touchkey_debug('C');
				break;
			}
			printk("touchkey_update failed... retry...\n");
			//set_touchkey_debug('f');
		}
		if (retry <= 0) {
			gpio_direction_output(_3_GPIO_TOUCH_EN, 0);
#if !defined(CONFIG_ARIES_NTT)
			gpio_direction_output(_3_GPIO_TOUCH_CE, 0);
#endif
			msleep(300);
		}
		init_hw();	//after update, re initalize.
	//}
#endif
	ret = i2c_add_driver(&touchkey_i2c_driver);
	if (ret)
		pr_err("%s: cypress touch keypad registration failed. (%d)\n",
				__func__, ret);

	return ret;
}

static void __exit touchkey_exit(void)
{
	i2c_del_driver(&touchkey_i2c_driver);
}

late_initcall(touchkey_init);
module_exit(touchkey_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("@@@");
MODULE_DESCRIPTION("cypress touch keypad");
