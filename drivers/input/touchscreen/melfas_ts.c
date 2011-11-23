/* drivers/input/touchscreen/melfas_ts.c
 *
 * Copyright (C) 2010 Melfas, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/earlysuspend.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <mach/regs-gpio.h>
#include <plat/gpio-cfg.h>
#include <plat/irqs.h>
#include <mach/hardware.h>
#include <mach/gpio.h>
#include <linux/jiffies.h>
#include <mach/regs-gpio.h>
#include <plat/gpio-cfg.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/earlysuspend.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/melfas_ts.h>
#include <linux/miscdevice.h>
#include "mcs8000_download.h"


#define MELFAS_MAX_TOUCH       5
#define FW_VERSION             0x01

#define TS_MAX_X_COORD         320
#define TS_MAX_Y_COORD         480
#define TS_MAX_Z_TOUCH         255
#define TS_MAX_W_TOUCH         30

#define TS_READ_START_ADDR 	   0x10
#define TS_READ_VERSION_ADDR   0x63
#define TS_READ_REGS_LEN       30

#define I2C_RETRY_CNT			10

#define	SET_DOWNLOAD_BY_GPIO	0

#define PRESS_KEY				1
#define RELEASE_KEY				0

#define DEBUG_PRINT 			0

#define IRQ_TOUCH_INT  (IRQ_EINT_GROUP3_BASE+1) 

#define DEVICE_NAME "touchkey"

#if SET_DOWNLOAD_BY_GPIO
#include <melfas_download.h>
#endif // SET_DOWNLOAD_BY_GPIO

struct muti_touch_info
{
    int state;
    int strength;
    int width;
    int posX;
    int posY;
};

struct melfas_ts_data
{
	uint16_t addr;
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct work_struct  work;
	uint32_t flags;
	int (*power)(int on);
	struct early_suspend early_suspend;
	int hw_rev;
	int fw_ver;	
};

static struct miscdevice touchkey_update_device = {
	.minor = MISC_DYNAMIC_MINOR,
 	.name = DEVICE_NAME,
};

#ifdef CONFIG_HAS_EARLYSUSPEND
static void melfas_ts_early_suspend(struct early_suspend *h);
static void melfas_ts_late_resume(struct early_suspend *h);
#endif

static struct muti_touch_info g_Mtouch_info[MELFAS_MAX_TOUCH];

static struct melfas_ts_data *ts;

static int melfas_init_panel(struct melfas_ts_data *ts)
{
    int ret,buf = 0x00;
    ret = i2c_master_send(ts->client, &buf, 1);

    ret = i2c_master_send(ts->client, &buf, 1);

    if (ret < 0)
    {
        printk(KERN_ERR "melfas_ts_probe: i2c_master_send() failed\n [%d]", ret);
        return 0;
    }


    return true;
}

static void melfas_ts_work_func(struct work_struct *work)
{
    struct melfas_ts_data *ts = container_of(work, struct melfas_ts_data, work);
    int ret = 0, i;
    uint8_t buf[TS_READ_REGS_LEN];
    int touchNumber = 0, touchPosition = 0, posX = 0, posY = 0, width = 0, strength = 0;
    int keyEvent = 0, keyState = 0, keyID = 0, keystrength = 0;

#if DEBUG_PRINT
    printk(KERN_ERR "melfas_ts_work_func\n");

    if (ts == NULL)
        printk(KERN_ERR "melfas_ts_work_func : TS NULL\n");
#endif


    /**
    Simple send transaction:
    	S Addr Wr [A]  Data [A] Data [A] ... [A] Data [A] P
    Simple recv transaction:
    	S Addr Rd [A]  [Data] A [Data] A ... A [Data] NA P
    */

    buf[0] = TS_READ_START_ADDR;
    for (i = 0; i < I2C_RETRY_CNT; i++)
    {
        ret = i2c_master_send(ts->client, buf, 1);
#if DEBUG_PRINT
        printk(KERN_ERR "melfas_ts_work_func : i2c_master_send [%d]\n", ret);
#endif
        if (ret >= 0)
        {
            ret = i2c_master_recv(ts->client, buf, TS_READ_REGS_LEN);
#if DEBUG_PRINT
            printk(KERN_ERR "melfas_ts_work_func : i2c_master_recv [%d]\n", ret);
#endif
            if (ret >= 0)
                break; // i2c success
        }
    }


    if (ret < 0)
    {
        printk(KERN_ERR "melfas_ts_work_func: i2c failed\n");
        enable_irq(ts->client->irq);
        return ;
    }
    else // Five Multi Touch Interface
    {
        touchNumber = buf[0] & 0x0F;
        touchPosition = buf[1] & 0x1F;

        for (i = 0; i < MELFAS_MAX_TOUCH; i++)
        {
            g_Mtouch_info[i].posX = ((buf[2 + 5*i] >> 4)   << 8) + buf[3 + 5*i];
            g_Mtouch_info[i].posY = ((buf[2 + 5*i] & 0x0F) << 8) + buf[4 + 5*i];
            g_Mtouch_info[i].width = buf[5 + 5*i];
            g_Mtouch_info[i].strength = buf[6 + 5*i];

            if (g_Mtouch_info[i].width != 0)
            {
                g_Mtouch_info[i].state = 1;
            }
            else
            {
                g_Mtouch_info[i].state = 0;
            }
        }

        keyID = buf[5*MELFAS_MAX_TOUCH + 2] & 0x07;
        keyState = (buf[5*MELFAS_MAX_TOUCH + 2] >> 3) & 0x01;
        keyEvent = (buf[5*MELFAS_MAX_TOUCH + 2] >> 4) & 0x01;
        keystrength = (buf[5*MELFAS_MAX_TOUCH + 3]);

        if (touchNumber > MELFAS_MAX_TOUCH)
        {
#if DEBUG_PRINT
            printk(KERN_ERR "melfas_ts_work_func: Touch ID: %d\n",  touchID);
#endif
            enable_irq(ts->client->irq);
            return;
        }

        for (i = 0; i < MELFAS_MAX_TOUCH; i++)
        {
            if ((g_Mtouch_info[i].posX == 0) || (g_Mtouch_info[i].posY == 0))
                continue;

		input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, i);
		input_report_abs(ts->input_dev, ABS_MT_POSITION_X, g_Mtouch_info[i].posX);
		input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, g_Mtouch_info[i].posY);
		input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, g_Mtouch_info[i].strength);
		input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, g_Mtouch_info[i].width);
		input_mt_sync(ts->input_dev);

		printk("x=%d,y=%d,z=%d,id=%d\n", g_Mtouch_info[i].posX, g_Mtouch_info[i].posY, g_Mtouch_info[i].strength, i);
        }
        if (keyEvent)
        {
            printk("[T_KEY]ID=%d,state=%d\n",keyID,keyState);
            if (keyID == 0x1)
                input_report_key(ts->input_dev, 139, keyState ? PRESS_KEY : RELEASE_KEY);
            if (keyID == 0x2)
                input_report_key(ts->input_dev, 102, keyState ? PRESS_KEY : RELEASE_KEY);
            if (keyID == 0x3)
                input_report_key(ts->input_dev, 158, keyState ? PRESS_KEY : RELEASE_KEY);
            if (keyID == 0x4)
                input_report_key(ts->input_dev, 217, keyState ? PRESS_KEY : RELEASE_KEY);
#if DEBUG_PRINT
            printk(KERN_ERR "melfas_ts_work_func: keyID : %d, keyState: %d\n", keyID, keySt;;ate);
#endif
        }

        input_sync(ts->input_dev);
    }

    enable_irq(ts->client->irq);
}

extern void keypad_led_control(bool onOff);
static ssize_t keypad_brightness_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    int value;
	
    sscanf(buf, "%d", &value);
	printk("keypad_brightness_store %d \n",value);

    if (value)
        keypad_led_control(1);
    else
        keypad_led_control(0);

	printk(KERN_DEBUG "[%s] brightness : %d \n", __FUNCTION__, value);

    return size;
}
static DEVICE_ATTR(brightness, 0664, NULL, keypad_brightness_store);

static irqreturn_t melfas_ts_irq_handler(int irq, void *handle)
{
    struct melfas_ts_data *ts = (struct melfas_ts_data *)handle;
	

    //printk("melfas_ts_irq_handler\n");

    disable_irq_nosync(ts->client->irq);
    schedule_work(&ts->work);

    return IRQ_HANDLED;
}

static int melfas_mcs7000_i2c_read(struct i2c_client* p_client, u8 reg, u8* data, int len)
{

	struct i2c_msg msg;

	/* set start register for burst read */
	/* send separate i2c msg to give STOP signal after writing. */
	/* Continous start is not allowed for cypress touch sensor. */

	msg.addr = p_client->addr;
	msg.flags = 0;
	msg.len = 1;
	msg.buf = &reg;

	if (1 != i2c_transfer(p_client->adapter, &msg, 1))
	{
		printk("%s set data pointer fail! reg(%x)\n", __func__, reg);
		return -EIO;
	}

	/* begin to read from the starting address */

	msg.addr = p_client->addr;
	msg.flags = I2C_M_RD;
	msg.len = len;
	msg.buf = data;

	if (1 != i2c_transfer(p_client->adapter, &msg, 1))
	{
		printk("%s fail! reg(%x)\n", __func__, reg);
		return -EIO;
	}
	
	return 0;
}

static void melfas_mcs7000_read_version(void)
{
	u8 buf[2] = {0,};
	
	if (0 == melfas_mcs7000_i2c_read(ts->client, MCSTS_MODULE_VER_REG, buf, 2))
	{

		ts->hw_rev = buf[0];
		ts->fw_ver = buf[1];
		
		printk("%s :HW Ver : 0x%02x, FW Ver : 0x%02x\n", __func__, buf[0], buf[1]);
	}
	else
	{
		ts->hw_rev = 0;
		ts->fw_ver = 0;
		
		printk("%s : Can't find HW Ver, FW ver!\n", __func__);
	}
}

void mcsdl_vdd_on(void)
{ 
	gpio_set_value(GPIO_TSP_LDO_ON, 1);
}

void mcsdl_vdd_off(void)
{
	gpio_set_value(GPIO_TSP_LDO_ON, 0);
}

void melfas_mcs7000_upgrade(INT32 hw_ver)
{
	int ret;


	printk("[F/W D/L] Entry gpio_tlmm_config\n");
	printk("[F/W D/L] Entry mcsdl_download_binary_data\n");
	ret = mcsdl_download_binary_data(ts->hw_rev); 
	
	melfas_mcs7000_read_version();
		
	if(ret > 0){
			if (ts->hw_rev < 0) {
				printk(KERN_ERR "i2c_transfer failed\n");;
			}
			
			if (ts->fw_ver < 0) {
				printk(KERN_ERR "i2c_transfer failed\n");
			}
			
			printk("[TOUCH] Firmware update success! [Melfas H/W version: 0x%02x., Current F/W version: 0x%02x.]\n", ts->hw_rev, ts->fw_ver);

	}
	else {
		printk("[TOUCH] Firmware update failed.. RESET!\n");
  		mcsdl_vdd_off();
		mdelay(500);
  		mcsdl_vdd_on();
		mdelay(200);
	}
}

static int melfas_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret = 0, i;

	uint8_t buf[2];

	printk(KERN_DEBUG"+-----------------------------------------+\n");
	printk(KERN_DEBUG "|  Melfas Touch Driver Probe!            |\n");
	printk(KERN_DEBUG"+-----------------------------------------+\n");

	gpio_set_value(GPIO_TSP_LDO_ON, 1);
	msleep(70);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
	{
	    printk(KERN_ERR "melfas_ts_probe: need I2C_FUNC_I2C\n");
	    ret = -ENODEV;
	    goto err_check_functionality_failed;
	}

	ts = kmalloc(sizeof(struct melfas_ts_data), GFP_KERNEL);
	if (ts == NULL)
	{
	    printk(KERN_ERR "melfas_ts_probe: failed to create a state of melfas-ts\n");
	    ret = -ENOMEM;
	    goto err_alloc_data_failed;
	}

	INIT_WORK(&ts->work, melfas_ts_work_func);

	ts->client = client;
	i2c_set_clientdata(client, ts);
	ret = i2c_master_send(ts->client, &buf, 1);


#if DEBUG_PRINT
	printk(KERN_ERR "melfas_ts_probe: i2c_master_send() [%d], Add[%d]\n", ret, ts->client->addr);
#endif

	melfas_mcs7000_read_version(); 

	printk("[TOUCH] Melfas	H/W version: 0x%02x.\n", ts->hw_rev);
	printk("[TOUCH] Current F/W version: 0x%02x.\n", ts->fw_ver);

	if(ts->fw_ver < 0x03)
		melfas_mcs7000_upgrade(ts->hw_rev);

	ts->input_dev = input_allocate_device();
	if (!ts->input_dev)
	{
	    printk(KERN_ERR "melfas_ts_probe: Not enough memory\n");
	    ret = -ENOMEM;
	    goto err_input_dev_alloc_failed;
	}

	ts->input_dev->name = "melfas-ts" ;

	ts->input_dev->evbit[0] = BIT_MASK(EV_ABS) | BIT_MASK(EV_KEY);


	ts->input_dev->keybit[BIT_WORD(KEY_MENU)] |= BIT_MASK(KEY_MENU);
	ts->input_dev->keybit[BIT_WORD(KEY_HOME)] |= BIT_MASK(KEY_HOME);
	ts->input_dev->keybit[BIT_WORD(KEY_BACK)] |= BIT_MASK(KEY_BACK);
	ts->input_dev->keybit[BIT_WORD(KEY_SEARCH)] |= BIT_MASK(KEY_SEARCH);


	//	__set_bit(BTN_TOUCH, ts->input_dev->keybit);
	//	__set_bit(EV_ABS,  ts->input_dev->evbit);
	//	ts->input_dev->evbit[0] =  BIT_MASK(EV_SYN) | BIT_MASK(EV_ABS) | BIT_MASK(EV_KEY);

	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, TS_MAX_X_COORD, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, TS_MAX_Y_COORD, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, TS_MAX_Z_TOUCH, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_TRACKING_ID, 0, MELFAS_MAX_TOUCH - 1, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, TS_MAX_W_TOUCH, 0, 0);
	//	__set_bit(EV_SYN, ts->input_dev->evbit);
	//	__set_bit(EV_KEY, ts->input_dev->evbit);


	ret = input_register_device(ts->input_dev);
	if (ret)
	{
	    printk(KERN_ERR "melfas_ts_probe: Failed to register device\n");
	    ret = -ENOMEM;
	    goto err_input_register_device_failed;
	}

	ts->client->irq = IRQ_TOUCH_INT;

	if (ts->client->irq)
	{
#if DEBUG_PRINT
	    printk(KERN_ERR "melfas_ts_probe: trying to request irq: %s-%d\n", ts->client->name, ts->client->irq);
#endif
	    ret = request_irq(ts->client->irq, melfas_ts_irq_handler, IRQF_TRIGGER_FALLING, ts->client->name, ts);
	    if (ret > 0)
	    {
	        printk(KERN_ERR "melfas_ts_probe: Can't allocate irq %d, ret %d\n", ts->client->irq, ret);
	        ret = -EBUSY;
	        goto err_request_irq;
	    }
	}

	schedule_work(&ts->work);

	ret = misc_register(&touchkey_update_device);
	if (ret) {
		printk("%s misc_register fail\n", __FUNCTION__);
		goto err_misc_reg;
	}

	if (device_create_file(touchkey_update_device.this_device, &dev_attr_brightness) < 0) {
		printk("%s device_create_file fail dev_attr_brightness\n",__FUNCTION__);
		pr_err("Failed to create device file(%s)!\n",dev_attr_brightness.attr.name);
	}

#if DEBUG_PRINT
	printk(KERN_ERR "melfas_ts_probe: succeed to register input device\n");
#endif

#if CONFIG_HAS_EARLYSUSPEND
	ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	ts->early_suspend.suspend = melfas_ts_early_suspend;
	ts->early_suspend.resume = melfas_ts_late_resume;
	register_early_suspend(&ts->early_suspend);
#endif

#if DEBUG_PRINT
	printk(KERN_INFO "melfas_ts_probe: Start touchscreen. name: %s, irq: %d\n", ts->client->name, ts->client->irq);
#endif
	return 0;

	err_request_irq:
	printk(KERN_ERR "melfas-ts: err_request_irq failed\n");
	free_irq(client->irq, ts);
	err_input_register_device_failed:
	printk(KERN_ERR "melfas-ts: err_input_register_device failed\n");
	input_free_device(ts->input_dev);
	err_input_dev_alloc_failed:
	printk(KERN_ERR "melfas-ts: err_input_dev_alloc failed\n");
	err_alloc_data_failed:
	printk(KERN_ERR "melfas-ts: err_alloc_data failed_\n");
	err_detect_failed:
	printk(KERN_ERR "melfas-ts: err_detect failed\n");
	kfree(ts);
	err_check_functionality_failed:
	printk(KERN_ERR "melfas-ts: err_check_functionality failed_\n");
	err_misc_reg:

	return ret;
}

static int melfas_ts_remove(struct i2c_client *client)
{
    struct melfas_ts_data *ts = i2c_get_clientdata(client);

    unregister_early_suspend(&ts->early_suspend);
    free_irq(client->irq, ts);
    input_unregister_device(ts->input_dev);
    kfree(ts);
    return 0;
}

static int melfas_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
    int ret;
    struct melfas_ts_data *ts = i2c_get_clientdata(client);

    disable_irq(client->irq);

    ret = cancel_work_sync(&ts->work);
    if (ret) /* if work was pending disable-count is now 2 */
        enable_irq(client->irq);

    ret = i2c_smbus_write_byte_data(client, 0x01, 0x00); /* deep sleep */

    if (ret < 0)
        printk(KERN_ERR "melfas_ts_suspend: i2c_smbus_write_byte_data failed\n");

    return 0;
}

static int melfas_ts_resume(struct i2c_client *client)
{
    struct melfas_ts_data *ts = i2c_get_clientdata(client);

    melfas_init_panel(ts);
    enable_irq(client->irq); // scl wave

    return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void melfas_ts_early_suspend(struct early_suspend *h)
{
	printk("%s\n",__FUNCTION__);
	disable_irq(ts->client->irq);	 
	gpio_set_value(GPIO_TSP_LDO_ON, 0);
}

static void melfas_ts_late_resume(struct early_suspend *h)
{
	printk("%s\n",__FUNCTION__);
	gpio_set_value(GPIO_TSP_LDO_ON, 1);
	msleep(70);	
	enable_irq(ts->client->irq);		
}
#endif

static const struct i2c_device_id melfas_ts_id[] =
{
    { MELFAS_TS_NAME, 0 },
    { }
};

static struct i2c_driver melfas_ts_driver =
{
    .driver = {
    .name = MELFAS_TS_NAME,
    },
    .id_table = melfas_ts_id,
    .probe = melfas_ts_probe,
    .remove = __devexit_p(melfas_ts_remove),
#ifndef CONFIG_HAS_EARLYSUSPEND
    .suspend = melfas_ts_suspend,
    .resume = melfas_ts_resume,
#endif
};

static int __devinit melfas_ts_init(void)
{
    return i2c_add_driver(&melfas_ts_driver);
}

static void __exit melfas_ts_exit(void)
{
    i2c_del_driver(&melfas_ts_driver);
}

MODULE_DESCRIPTION("Driver for Melfas MTSI Touchscreen Controller");
MODULE_AUTHOR("MinSang, Kim <kimms@melfas.com>");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");

module_init(melfas_ts_init);
module_exit(melfas_ts_exit);
