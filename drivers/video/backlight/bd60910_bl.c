/*
 *  LCD / Backlight control code for ROHM BD60910
 *
 *  Copyright (c) 2005		Dirk Opfer
 *  Copyright (c) 2007,2008	Dmitry Baryshkov
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/spi/spi.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <plat/gpio-cfg.h>
#include <linux/fb.h>
#include <linux/backlight.h>
#include <linux/earlysuspend.h>

#define TRACE_CALL()        printk(KERN_ERR "%s\n", __func__)

#define BD60910_I2C_ADDR        0xEC    // TODO:
#define BD60910_I2C_BUS_NUM     13

#define COMADJ_DEFAULT	97

#define DAC_CH1		0
#define DAC_CH2		1

#define DIM_BL	20
#define MIN_BL	45
#define MAX_BL	225


#define CONFIG_FORTE_REV01
#define CONFIG_BL_HAS_EARLYSUSPEND


#ifdef  CONFIG_FORTE_REV01
    #define BL_ENABLE   S5PV210_GPG2(2)
    #define BL_SCL      S5PV210_GPJ2(1)
    #define BL_SDA      S5PV210_GPJ2(2)
#else
    #define BL_ENABLE   S5PV210_GPG2(2)
    #define BL_SCL      S5PV210_GPJ3(4)
    #define BL_SDA      S5PV210_GPJ3(5)
#endif

struct bd60910_bl_data {
	struct i2c_client *i2c;
	struct backlight_device *bl;

	int comadj;
};

static struct i2c_driver bd60910_bl_driver ;
static struct i2c_client *bl_i2c_client = NULL;

static int bd_brightness = 0;
static int backlight_suspend = 0 ;
int backlight_level = 0;   // disys_forte

enum {
	BACKLIGHT_LEVEL_OFF		= 0,
	BACKLIGHT_LEVEL_DIMMING	= 1,
	BACKLIGHT_LEVEL_NORMAL	= 6
};

int get_bl_update_status(void)
{
    printk(KERN_ERR "get brightness=%d\n", bd_brightness) ;
	return bd_brightness;
}
EXPORT_SYMBOL(get_bl_update_status);

static void bd60910_bl_set_backlight(struct bd60910_bl_data *data, int brightness)
{
    printk(KERN_INFO "brightness=%d\n", brightness) ;

    if (brightness == bd_brightness)
        return ;

    if (backlight_suspend)
        return ;

    if (brightness == 0)
    {
        extern void keyled_lcd_off_Ext(); /* keypad_led */ //board_bring
        keyled_lcd_off_Ext();             /* keypad_led */
      
        i2c_smbus_write_byte_data(data->i2c, 0x03, 0x00) ;
    	i2c_smbus_write_byte_data(data->i2c, 0x01, 0x00);
        bd_brightness = brightness ;
        return ;
    }

    /* else */
    if (brightness > data->bl->props.max_brightness)
        brightness = data->bl->props.max_brightness ;

    if (bd_brightness == 0)
    {
      extern void keyled_lcd_on_Ext(); /* keypad_led */ //board_bring
      keyled_lcd_on_Ext();             /* keypad_led */

    	i2c_smbus_write_byte_data(data->i2c, 0x01, 0x01);
    	i2c_smbus_write_byte_data(data->i2c, 0x08, 0x10) ;
    }

   // backlight_level update
   if(brightness == 0)
   {
      backlight_level = BACKLIGHT_LEVEL_OFF;	//lcd off
   }
   else if((brightness < MIN_BL) && (brightness > 0))
   {
      backlight_level = BACKLIGHT_LEVEL_DIMMING;	//dimming
   }
   else
   {
      backlight_level = BACKLIGHT_LEVEL_NORMAL;	//normal
   }

    /* change max 160 */
    brightness = (160*brightness)/255;
    //printk(KERN_ERR "brightness change =%d\n", brightness) ;

    i2c_smbus_write_byte_data(data->i2c, 0x03, ((brightness >> 1) & 0x7F)) ;    /* (0~255) --> (0~127) */

    bd_brightness = brightness ;
}

static int bd60910_bl_update_status(struct backlight_device *dev)
{
	struct backlight_properties *props = &dev->props;
	struct bd60910_bl_data *data = dev_get_drvdata(&dev->dev);
	int power = max(props->power, props->fb_blank);
	int brightness = props->brightness;

    //TRACE_CALL() ;
    //printk(KERN_ERR "power=%d, blank=%d, brightness=%d\n", props->power, props->fb_blank, props->brightness) ;
	if (power)
		brightness = 0;

	bd60910_bl_set_backlight(data, brightness);

	return 0;
}

static int bd60910_bl_get_brightness(struct backlight_device *dev)
{
	struct backlight_properties *props = &dev->props;

    TRACE_CALL() ;
	return bd_brightness ;
}

// cmk 2011.04.11 fixed camera & video player black screen issue. Add bd60910_bl_check_fb with return 0.
static int bd60910_bl_check_fb(struct backlight_device *bldev, struct fb_info *fb)
{
	return 0;
}

static struct backlight_ops bl_ops = {
	.get_brightness		= bd60910_bl_get_brightness,
	.update_status		= bd60910_bl_update_status,
	.check_fb		    = bd60910_bl_check_fb,
};
// end cmk

#undef  BD60910_DRIVER_LEGACY
#ifndef BD60910_DRIVER_LEGACY /* New-Style */
#ifdef  CONFIG_BL_HAS_EARLYSUSPEND
static struct early_suspend	bl_early_suspend ;

void bd60910_bl_early_suspend(struct early_suspend *h)
{
	struct bd60910_bl_data *data ;

    TRACE_CALL() ;
    if (bl_i2c_client == NULL)
        return ;

    data = i2c_get_clientdata(bl_i2c_client);
	bd60910_bl_set_backlight(data, 0);
    backlight_suspend = 1 ;
}

void bd60910_bl_late_resume(struct early_suspend *h)
{
	struct bd60910_bl_data *data ;

    TRACE_CALL() ;
    if (bl_i2c_client == NULL)
        return ;

    backlight_suspend = 0 ;
    data = i2c_get_clientdata(bl_i2c_client);
	backlight_update_status(data->bl);
}
#endif

static int __devinit bd60910_bl_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	struct bd60910_bl_data *data = kzalloc(sizeof(struct bd60910_bl_data), GFP_KERNEL);
	int ret = 0;
    TRACE_CALL() ;
	if (!data)
		return -ENOMEM;
    // FORTE	data->comadj = sharpsl_param.comadj == -1 ? COMADJ_DEFAULT : sharpsl_param.comadj;

        ret = gpio_request(BL_ENABLE, "backlight");
	if (ret) {
		dev_dbg(&data->bl->dev, "Unable to request gpio!\n");
		goto err_gpio_bl;
	}
	ret = gpio_direction_output(BL_ENABLE, 1);      // TODO: FORTE Rev0.1 timing ???
	if (ret)
		goto err_gpio_dir;
	i2c_set_clientdata(client, data);
	data->i2c = client;
        bl_i2c_client = client ;
	data->bl = backlight_device_register("s5p_bl" /*"bd60910"*/, &client->dev,
			data, &bl_ops, NULL); /* YUNG@DISYS GB */
	if (IS_ERR(data->bl)) {
		ret = PTR_ERR(data->bl);
		goto err_reg;
	}

	data->bl->props.brightness = 69;
	data->bl->props.max_brightness = 255 ;
	data->bl->props.power = FB_BLANK_UNBLANK;

	backlight_update_status(data->bl);
 
#ifdef CONFIG_BL_HAS_EARLYSUSPEND
	bl_early_suspend.suspend = bd60910_bl_early_suspend;
	bl_early_suspend.resume =  bd60910_bl_late_resume;
	bl_early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
	register_early_suspend(&bl_early_suspend);
#endif

	return 0;

err_reg:
	data->bl = NULL;
	i2c_set_clientdata(client, NULL);
err_gpio_dir:
	gpio_free(BL_ENABLE);
err_gpio_bl:
	kfree(data);
	return ret;
}

static int __devexit bd60910_bl_remove(struct i2c_client *client)
{
	struct bd60910_bl_data *data = i2c_get_clientdata(client);

    TRACE_CALL() ;
	backlight_device_unregister(data->bl);
	data->bl = NULL;
	i2c_set_clientdata(client, NULL);

	gpio_free(BL_ENABLE);

	kfree(data);

	return 0;
}

#if defined(CONFIG_PM) && !defined(CONFIG_BL_HAS_EARLYSUSPEND)
static int bd60910_bl_suspend(struct i2c_client *client, pm_message_t pm)
{
	struct bd60910_bl_data *data = i2c_get_clientdata(client);

    TRACE_CALL() ;
	bd60910_bl_set_backlight(data, 0);

	return 0;
}

static int bd60910_bl_resume(struct i2c_client *client)
{
	struct bd60910_bl_data *data = i2c_get_clientdata(client);

    TRACE_CALL() ;
	backlight_update_status(data->bl);
	return 0;
}
#else
#define bd60910_bl_suspend NULL
#define bd60910_bl_resume NULL
#endif

#else   /* Legacy Style */
static unsigned short   bl_ignore[] = {I2C_CLIENT_END };
static unsigned short   bl_normal[] = {I2C_CLIENT_END };
static unsigned short   bl_probe[]  = {BD60910_I2C_BUS_NUM, (BD60910_I2C_ADDR >> 1), I2C_CLIENT_END };
const static struct i2c_client_address_data bl_addr_data = {
	.normal_i2c	= bl_normal,
	.ignore		= bl_ignore,
	.probe		= bl_probe,
};

static int bd60910_attach(struct i2c_adapter *adap, int addr, int kind)
{
	struct i2c_client *c;
	int ret;

	TRACE_CALL() ;

	c = kmalloc(sizeof(*c), GFP_KERNEL);
	if (!c)
		return -ENOMEM;

	memset(c, 0, sizeof(struct i2c_client));

	strncpy(c->name, bd60910_bl_driver.driver.name, I2C_NAME_SIZE);
	c->addr = addr;
	c->adapter = adap;
	c->driver = &bd60910_bl_driver;

	if ((ret = i2c_attach_client(c)))
		goto error;

	bl_i2c_client = c;

error:
    printk(KERN_ERR "bd60910_attched ret=%d\n", ret) ;
	return ret;
}


static int bd60910_attach_adapter(struct i2c_adapter *adap)
{
	TRACE_CALL() ;
	return i2c_probe(adap, &bl_addr_data, bd60910_attach);
}

static int bd60910_detach_client(struct i2c_client *client)
{
	TRACE_CALL() ;
	i2c_detach_client(client);
	return 0;
}
#endif  /* new-style or legacy-style */

static const struct i2c_device_id bd60910_bl_id[] = {
	{ "bd60910-i2c", 0 },
	{ },
};

static struct i2c_driver bd60910_bl_driver = {
	.driver = {
		.name		= "bd60910-i2c",
		.owner		= THIS_MODULE,
	},
#ifdef BD60910_DRIVER_LEGACY  //  legacy style
	.id 		= 0,
	.attach_adapter	= bd60910_attach_adapter,
	.detach_client	= bd60910_detach_client,
	.command	= NULL,
	.address_data = &bl_addr_data,
#else // new style
	.probe		= bd60910_bl_probe,
	.remove		= __devexit_p(bd60910_bl_remove),
	.suspend	= bd60910_bl_suspend,
	.resume		= bd60910_bl_resume,
	.id_table	= bd60910_bl_id,
#endif
};

static int __init bd60910_bl_init(void)
{
#ifdef BD60910_DRIVER_LEGACY
    TRACE_CALL() ;

	return i2c_add_driver(&bd60910_bl_driver);
#else
    struct i2c_board_info   info ;
    struct i2c_adapter      *adapter ;
    int ret ;

    TRACE_CALL() ;
#if 0
    // backlight_check {{
    s3c_gpio_cfgpin(GPIO_DISPLAY_ID, S3C_GPIO_INPUT);
	s3c_gpio_setpull(GPIO_DISPLAY_ID, S3C_GPIO_PULL_UP);
    ret = gpio_get_value(GPIO_DISPLAY_ID);
    if(0/*ret*/) // for chief
    {
        printk(KERN_ERR "===============================================\n");
        printk(KERN_ERR "             NOT CONNECT BACKLIGHT");
        printk(KERN_ERR "===============================================\n");
        s3c_gpio_setpull(GPIO_DISPLAY_ID, S3C_GPIO_PULL_NONE);

        s3c_gpio_cfgpin(BL_SCL, S3C_GPIO_INPUT);
        s3c_gpio_setpull(BL_SCL, S3C_GPIO_PULL_NONE);

        s3c_gpio_cfgpin(BL_SDA, S3C_GPIO_INPUT);
        s3c_gpio_setpull(BL_SCL, S3C_GPIO_PULL_NONE);

        gpio_direction_output(BL_ENABLE, GPIO_LEVEL_LOW);
        return;
    }
    // backlight_check }}
#endif
    ret = i2c_add_driver(&bd60910_bl_driver) ;
    if (ret != 0)
    {
        printk(KERN_ERR "%s i2c fail %d\n", __FUNCTION__, ret) ;
        return ret ;
    }

    memset(&info, 0, sizeof(info)) ;
    info.addr = BD60910_I2C_ADDR>>1 ;
    strlcpy(info.type, "bd60910-i2c", I2C_NAME_SIZE) ;

    adapter = i2c_get_adapter(BD60910_I2C_BUS_NUM) ; // TODO:
    if (!adapter)
    {
        printk(KERN_ERR "not found apater i2c %d\n", BD60910_I2C_BUS_NUM) ;
        goto err_driver ;
    }

    i2c_new_device(adapter, &info) ;
    // printk(KERN_ERR "info type=%s addr=%02x\n", info.type, info.addr) ;
    i2c_put_adapter(adapter) ;
    return 0 ;

err_driver :
    i2c_del_driver(&bd60910_bl_driver) ;
    return -ENODEV ;
#endif
}

static void __exit bd60910_bl_exit(void)
{
	i2c_del_driver(&bd60910_bl_driver);
}

module_init(bd60910_bl_init);
module_exit(bd60910_bl_exit);

MODULE_AUTHOR("mmm");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("LCD/Backlight control for Rohm BD60910");

