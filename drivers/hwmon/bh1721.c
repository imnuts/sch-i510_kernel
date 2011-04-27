/*
 *  bh1721fvc.c - Ambient Light Sensor IC
 *
 *  Copyright (C) 2009 Samsung Electronics
 *  Donggeun Kim <dg77.kim@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/bh1721.h>

#define NUM_OF_BYTES_WRITE	1
#define NUM_OF_BYTES_READ	2

#define ON              1
#define OFF				0
 
const unsigned char POWER_DOWN = 0x00;
const unsigned char POWER_ON = 0x01;
const unsigned char AUTO_RESOLUTION_1 = 0x10;
const unsigned char AUTO_RESOLUTION_2 = 0x20;
const unsigned char H_RESOLUTION_1 = 0x12;
const unsigned char H_RESOLUTION_2 = 0x22;
const unsigned char L_RESOLUTION_1 = 0x13;
const unsigned char L_RESOLUTION_2 = 0x23;
const unsigned char L_RESOLUTION_3 = 0x16;
const unsigned char L_RESOLUTION_4 = 0x26;


struct class *lightsensor_class;
struct device *switch_cmd_dev;
static bool light_enable = OFF;

struct bh1721_chip {
	struct i2c_client *client;
	struct device *dev;
	void (*reset) (void);

	unsigned char illuminance_data[2];
};
struct bh1721_chip *chip;

static int bh1721_write_command(struct i2c_client *client, const char *command)
{
	return i2c_master_send(client, command, NUM_OF_BYTES_WRITE);
}

static int bh1721_read_value(struct i2c_client *client, char *buf)
{
	return i2c_master_recv(client, buf, NUM_OF_BYTES_READ);
}

static ssize_t bh1721_show_illuminance(struct device *dev,
				struct device_attribute *attr, char *buf)
{	
	unsigned int result;

	/* 
	 * POWER ON command is possible to omit.
	 */
	if((bh1721_write_command(chip->client, &POWER_ON))>0)
		light_enable = ON;
	bh1721_write_command(chip->client, &H_RESOLUTION_2);

	/* Maximum measurement time */
	msleep(180);
	bh1721_read_value(chip->client, chip->illuminance_data);
	if((bh1721_write_command(chip->client, &POWER_DOWN))>0)
		light_enable = OFF;
	result = chip->illuminance_data[0] << 8 | chip->illuminance_data[1];
	result = (result*10)/12;
	return sprintf(buf, "%d\n", result);
}

static SENSOR_DEVICE_ATTR(illuminance, S_IRUGO | S_IWUSR,
			  bh1721_show_illuminance, NULL, 0);

static struct attribute *bh1721_attributes[] = {
	&sensor_dev_attr_illuminance.dev_attr.attr,
	NULL
};

static const struct attribute_group bh1721_group = {
	.attrs = bh1721_attributes,
};

/*For Factory Test Mode*/
static ssize_t lightsensor_file_state_show(struct device *dev, struct device_attribute *attr, char *buf)
{	
	unsigned int result;
	/* 
	 * POWER ON command is possible to omit.
	 */
	if((bh1721_write_command(chip->client, &POWER_ON))>0)
		light_enable = ON;
	bh1721_write_command(chip->client, &H_RESOLUTION_2);

	/* Maximum measurement time */
	msleep(180);
	bh1721_read_value(chip->client, chip->illuminance_data);
	if((bh1721_write_command(chip->client, &POWER_DOWN))>0)
		light_enable = OFF;
	result = chip->illuminance_data[0] << 8 | chip->illuminance_data[1];
	result = (result*10)/12;
	return sprintf(buf, "%d\n", result);
}
static DEVICE_ATTR(lightsensor_file_state, S_IRUGO, lightsensor_file_state_show,NULL);

/* for light sensor on/off control from platform */
static ssize_t lightsensor_file_cmd_show(struct device *dev, struct device_attribute *attr, char *buf)
{		
	return sprintf(buf, "%d\n", light_enable);
}
static ssize_t lightsensor_file_cmd_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int mode,ret;	
	ret=0;
	sscanf(buf,"%d",&mode);	
	if(mode ==0)
	{
		bh1721_write_command(chip->client, &POWER_DOWN);
		light_enable = OFF;
	}
	else if(mode ==1)		
	{	
		bh1721_write_command(chip->client, &POWER_ON);
		light_enable = ON;
	}
	return count;		
}
static DEVICE_ATTR(lightsensor_file_cmd, S_IRUGO | S_IWUGO, lightsensor_file_cmd_show, lightsensor_file_cmd_store);


static int bh1721_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{	
	struct bh1721_platform_data *pdata;
	int ret;
	printk("[%s] bh1721 started...",__func__);
	chip = kzalloc(sizeof(struct bh1721_chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	pdata = client->dev.platform_data;

	chip->reset = pdata->reset;
	chip->client = client;
	i2c_set_clientdata(client, chip);

	if (chip->reset)
		chip->reset();

	chip->dev = hwmon_device_register(&client->dev);
	if (IS_ERR(chip->dev)) {
		ret = PTR_ERR(chip->dev);
		goto error_hwmon;
	}
	
	ret = sysfs_create_group(&client->dev.kobj, &bh1721_group);
	if (ret) {
		dev_err(&client->dev, "Creating bh1721 attribute group failed");
		goto error_device;
	}

	/* set sysfs for light sensor test mode*/
	lightsensor_class = class_create(THIS_MODULE, "lightsensor");
	if (IS_ERR(lightsensor_class))
	{
		printk("Failed to create class(lightsensor)!\n");
		goto error_device;
	}
	switch_cmd_dev = device_create(lightsensor_class, NULL, 0, NULL, "switch_cmd");
	if (IS_ERR(switch_cmd_dev))
	{
		printk("Failed to create device(switch_cmd_dev)!\n");
		goto DESTROY_CLASS;
	}
	if (device_create_file(switch_cmd_dev, &dev_attr_lightsensor_file_cmd) < 0)
	{
		printk("Failed to create device file(%s)!\n", dev_attr_lightsensor_file_cmd.attr.name);
		goto DESTROY_DEVICE;
	}
	if (device_create_file(switch_cmd_dev, &dev_attr_lightsensor_file_state) < 0)
	{
		printk("Failed to create device file(%s)!\n", dev_attr_lightsensor_file_state.attr.name);
		device_remove_file(switch_cmd_dev, &dev_attr_lightsensor_file_cmd);
		goto DESTROY_DEVICE;
	}
	
	printk(KERN_INFO "%s registered\n", id->name);
	printk("%s end",__func__);
	return 0;

DESTROY_DEVICE:
	device_destroy(lightsensor_class,0);
DESTROY_CLASS:
	class_destroy(lightsensor_class);
error_device:
	sysfs_remove_group(&client->dev.kobj, &bh1721_group);
error_hwmon:
	hwmon_device_unregister(chip->dev);
	i2c_set_clientdata(client, NULL);
	kfree(chip);
	return ret;
}

static int __exit bh1721_remove(struct i2c_client *client)
{	
	device_remove_file(switch_cmd_dev, &dev_attr_lightsensor_file_cmd);
	device_remove_file(switch_cmd_dev, &dev_attr_lightsensor_file_state);
	device_destroy(lightsensor_class,0);
	class_destroy(lightsensor_class);
	sysfs_remove_group(&client->dev.kobj, &bh1721_group);
	hwmon_device_unregister(chip->dev);
	i2c_set_clientdata(client, NULL);
	kfree(chip);

	printk("[%s] bh1721 removed...",__func__);
	return 0;
}

static int bh1721_suspend(struct i2c_client *client, pm_message_t mesg)
{	
	bh1721_write_command(chip->client, &POWER_DOWN);
	return 0;
}

static int bh1721_resume(struct i2c_client *client)
{	
	if (chip->reset)
		chip->reset();
	return 0;
}

static const struct i2c_device_id bh1721_id[] = {
	{"bh1721", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, bh1721_id);

static struct i2c_driver bh1721_i2c_driver = {
	.driver = {
		   .name = "bh1721",
		   },
	.probe = bh1721_probe,
	.remove = __exit_p(bh1721_remove),
	.suspend = bh1721_suspend,
	.resume = bh1721_resume,
	.id_table = bh1721_id,
};

static int __init bh1721_init(void)
{
	return i2c_add_driver(&bh1721_i2c_driver);
}

module_init(bh1721_init);

static void __exit bh1721_exit(void)
{	
	i2c_del_driver(&bh1721_i2c_driver);
}

module_exit(bh1721_exit);

MODULE_AUTHOR("Donggeun Kim <dg77.kim@samsung.com>");
MODULE_DESCRIPTION("Linux Device Driver for BH1721");
MODULE_LICENSE("GPL");
