/* drivers/input/touchscreen/melfas_ts_i2c_tsi.c
 *
 * Copyright (C) 2007 Google, Inc.
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
#include <linux/hrtimer.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/earlysuspend.h>
#include <mach/gpio.h>
#include <linux/jiffies.h>

#include <asm/io.h>
#include <linux/irq.h>
#include <mach/regs-gpio.h>
#include <plat/gpio-cfg.h>
#include <mach/hardware.h>
#include <asm-generic/gpio.h>
//#include <linux/dprintk.h>
#include "melfas_download.h"

#if CONFIG_MACH_CHIEF
#include <mach/gpio-chief.h>
#include <mach/gpio-chief-settings.h>
#else
#include <mach/forte/gpio-aries.h>
#endif
#include <linux/slab.h>

#define CONFIG_TOUCHSCREEN_MELFAS_FIRMWARE_UPDATE

#define INPUT_INFO_REG 0x10
#define IRQ_TOUCH_INT   (IRQ_EINT_GROUP6_BASE+3)//MSM_GPIO_TO_INT(GPIO_TOUCH_INT)

#define FINGER_NUM	      5 //for multi touch
#define CONFIG_CPU_FREQ
#undef CONFIG_MOUSE_OPTJOY

#ifdef CONFIG_CPU_FREQ
//#include <plat/s3c64xx-dvfs.h>
#include <mach/cpu-freq-v210.h>

#endif

static int debug_level = 5; 
#define debugprintk(level,x...)  if(debug_level>=level) printk(x)

extern int mcsdl_download_binary_data(void);//eunsuk test  [int hw_ver -> void]
#ifdef CONFIG_MOUSE_OPTJOY
extern int get_sending_oj_event();
#endif

extern struct class *sec_class;

struct input_info {
	int max_x;
	int max_y;
	int state;
	int x;
	int y;
	int z;
	int width;
	int finger_id; 
};

struct melfas_ts_driver {
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct work_struct  work;
	int irq;
	int hw_rev;
	int fw_ver;
	struct input_info info[FINGER_NUM];
	int suspended;
	struct early_suspend	early_suspend;
};

struct melfas_ts_driver *melfas_ts = NULL;
struct i2c_driver melfas_ts_i2c;
struct workqueue_struct *melfas_ts_wq;

static struct vreg *vreg_touch;
static struct vreg *vreg_touchio;

#ifdef CONFIG_HAS_EARLYSUSPEND
void melfas_ts_early_suspend(struct early_suspend *h);
void melfas_ts_late_resume(struct early_suspend *h);
#endif	/* CONFIG_HAS_EARLYSUSPEND */

#define TOUCH_HOME	0//KEY_HOME
#define TOUCH_MENU	0//KEY_MENU
#define TOUCH_BACK	0//KEY_BACK
#define TOUCH_SEARCH  0//KEY_SEARCH

int melfas_ts_tk_keycode[] =
{ TOUCH_HOME, TOUCH_MENU, TOUCH_BACK, TOUCH_SEARCH, };

struct device *ts_dev;

void mcsdl_vdd_on(void)
{ 
  gpio_set_value(GPIO_TOUCH_EN,1);
  mdelay(25); //MUST wait for 25ms after vreg_enable() 
}

void mcsdl_vdd_off(void)
{
  gpio_set_value(GPIO_TOUCH_EN,0);
  mdelay(100); //MUST wait for 100ms before vreg_enable() 
}

static int melfas_i2c_read(struct i2c_client* p_client, u8 reg, u8* data, int len)
{

	struct i2c_msg msg;

	/* set start register for burst read */
	/* send separate i2c msg to give STOP signal after writing. */
	/* Continous start is not allowed for cypress touch sensor. */

	msg.addr = p_client->addr;
	msg.flags = 0;
	msg.len = 1;
	msg.buf = &reg;

	
	printk(KERN_ERR "[ %s ] addr [ %d ]\n", __func__, msg.addr);		// heatup - test - remove

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

static void melfas_read_version(void)
{
	u8 buf[2] = {0,};
	
	if (0 == melfas_i2c_read(melfas_ts->client, MCSTS_MODULE_VER_REG, buf, 2))
	{

		melfas_ts->hw_rev = buf[0];
		melfas_ts->fw_ver = buf[1];
		
		printk("%s :HW Ver : 0x%02x, FW Ver : 0x%02x\n", __func__, buf[0], buf[1]);
	}
	else
	{
		melfas_ts->hw_rev = 0;
		melfas_ts->fw_ver = 0;
		
		printk("%s : Can't find HW Ver, FW ver!\n", __func__);
	}
}

static void melfas_read_resolution(void)
{
	
	uint16_t max_x=0, max_y=0;	

	u8 buf[3] = {0,};
	
	if(0 == melfas_i2c_read(melfas_ts->client, MCSTS_RESOL_HIGH_REG , buf, 3)){

		printk("%s :buf[0] : 0x%02x, buf[1] : 0x%02x, buf[2] : 0x%02x\n", __func__,buf[0],buf[1],buf[2]);

		if(buf[0] == 0){
			melfas_ts->info[0].max_x = 320;
			melfas_ts->info[0].max_y = 480;			
			
			printk("%s : Can't find Resolution!\n", __func__);
			}
		
		else{
			max_x = buf[1] | ((uint16_t)(buf[0] & 0x0f) << 8); 
			max_y = buf[2] | (((uint16_t)(buf[0] & 0xf0) >> 4) << 8); 
			melfas_ts->info[0].max_x = max_x;
			melfas_ts->info[0].max_y = max_y;

			printk("%s :max_x: %d, max_y: %d\n", __func__, melfas_ts->info[0].max_x, melfas_ts->info[0].max_y);
			}
		}

	else
	{
		melfas_ts->info[0].max_x = 320;
		melfas_ts->info[0].max_y = 480;
		
		printk("%s : Can't find Resolution!\n", __func__);
	}
}

static ssize_t registers_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int status, mode_ctl, hw_rev, fw_ver;
	
	status  = i2c_smbus_read_byte_data(melfas_ts->client, MCSTS_STATUS_REG);
	if (status < 0) {
		printk(KERN_ERR "i2c_smbus_read_byte_data failed\n");;
	}
	mode_ctl = i2c_smbus_read_byte_data(melfas_ts->client, MCSTS_MODE_CONTROL_REG);
	if (mode_ctl < 0) {
		printk(KERN_ERR "i2c_smbus_read_byte_data failed\n");;
	}
	hw_rev = i2c_smbus_read_byte_data(melfas_ts->client, MCSTS_MODULE_VER_REG);
	if (hw_rev < 0) {
		printk(KERN_ERR "i2c_smbus_read_byte_data failed\n");;
	}
	fw_ver = i2c_smbus_read_byte_data(melfas_ts->client, MCSTS_FIRMWARE_VER_REG);
	if (fw_ver < 0) {
		printk(KERN_ERR "i2c_smbus_read_byte_data failed\n");;
	}
	
	sprintf(buf, "[TOUCH] Melfas Tsp Register Info.\n");
	sprintf(buf, "%sRegister 0x00 (status)  : 0x%08x\n", buf, status);
	sprintf(buf, "%sRegister 0x01 (mode_ctl): 0x%08x\n", buf, mode_ctl);
	sprintf(buf, "%sRegister 0x30 (hw_rev)  : 0x%08x\n", buf, hw_rev);
	sprintf(buf, "%sRegister 0x31 (fw_ver)  : 0x%08x\n", buf, fw_ver);

	return sprintf(buf, "%s", buf);
}

static ssize_t registers_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	int ret;
	if(strncmp(buf, "RESET", 5) == 0 || strncmp(buf, "reset", 5) == 0) {
		
	    ret = i2c_smbus_write_byte_data(melfas_ts->client, 0x01, 0x01);
		if (ret < 0) {
			printk(KERN_ERR "i2c_smbus_write_byte_data failed\n");
		}
		printk("[TOUCH] software reset.\n");
	}
	return size;
}

static ssize_t gpio_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	sprintf(buf, "[TOUCH] Melfas Tsp Gpio Info.\n");
	sprintf(buf, "%sGPIO TOUCH_INT : %s\n", buf, gpio_get_value(GPIO_TOUCH_INT)? "HIGH":"LOW"); 
	return sprintf(buf, "%s", buf);
}

static ssize_t gpio_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	if(strncmp(buf, "ON", 2) == 0 || strncmp(buf, "on", 2) == 0) {
    mcsdl_vdd_on();
		//gpio_set_value(GPIO_TOUCH_EN, GPIO_LEVEL_HIGH);
		printk("[TOUCH] enable.\n");
		mdelay(200);
	}

	if(strncmp(buf, "OFF", 3) == 0 || strncmp(buf, "off", 3) == 0) {
    mcsdl_vdd_off();
		printk("[TOUCH] disable.\n");
	}
	
	if(strncmp(buf, "RESET", 5) == 0 || strncmp(buf, "reset", 5) == 0) {
    mcsdl_vdd_off();
		mdelay(500);
    mcsdl_vdd_on();
		printk("[TOUCH] reset.\n");
		mdelay(200);
	}
	return size;
}


static ssize_t firmware_show(struct device *dev, struct device_attribute *attr, char *buf)
{	
	int hw_rev, fw_ver;
	
	hw_rev = i2c_smbus_read_byte_data(melfas_ts->client, MCSTS_MODULE_VER_REG);
	if (hw_rev < 0) {
		printk(KERN_ERR "i2c_smbus_read_byte_data failed\n");
	}
	fw_ver = i2c_smbus_read_byte_data(melfas_ts->client, MCSTS_FIRMWARE_VER_REG);
	if (fw_ver < 0) {
		printk(KERN_ERR "i2c_smbus_read_byte_data failed\n");
	}

	sprintf(buf, "H/W rev. 0x%x F/W ver. 0x%x\n", hw_rev, fw_ver);
	return sprintf(buf, "%s", buf);
}


static ssize_t firmware_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	#ifdef CONFIG_TOUCHSCREEN_MELFAS_FIRMWARE_UPDATE	
	int ret;
	if(strncmp(buf, "UPDATE", 6) == 0 || strncmp(buf, "update", 6) == 0) {
		printk("[TOUCH] Melfas  H/W version: 0x%02x.\n", melfas_ts->hw_rev);
		printk("[TOUCH] Current F/W version: 0x%02x.\n", melfas_ts->fw_ver);

		disable_irq(melfas_ts->client->irq);

		printk("[F/W D/L] Entry gpio_tlmm_config\n");

		s3c_gpio_cfgpin(GPIO_TOUCH_I2C_SCL, S3C_GPIO_OUTPUT); s3c_gpio_setpull(GPIO_TOUCH_I2C_SCL,S3C_GPIO_PULL_DOWN);
		s3c_gpio_cfgpin(GPIO_TOUCH_I2C_SDA, S3C_GPIO_OUTPUT); s3c_gpio_setpull(GPIO_TOUCH_I2C_SDA,S3C_GPIO_PULL_DOWN);
		s3c_gpio_cfgpin(GPIO_TOUCH_INT, S3C_GPIO_OUTPUT);     s3c_gpio_setpull(GPIO_TOUCH_INT,S3C_GPIO_PULL_DOWN);

//		gpio_tlmm_config(GPIO_CFG(GPIO_I2C0_SCL,  0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
//		gpio_tlmm_config(GPIO_CFG(GPIO_I2C0_SDA,  0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);	
//		gpio_tlmm_config(GPIO_CFG(GPIO_TOUCH_INT, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		
		printk("[F/W D/L] Entry mcsdl_download_binary_data\n");
		ret = mcsdl_download_binary_data(); //eunsuk test [melfas_ts->hw_rev -> ()]
		
		enable_irq(melfas_ts->client->irq);
		
		melfas_read_version(); 
			
		if(ret > 0){
				if (melfas_ts->hw_rev < 0) {
					printk(KERN_ERR "i2c_transfer failed\n");
				}
				
				if (melfas_ts->fw_ver < 0) {
					printk(KERN_ERR "i2c_transfer failed\n");
				}
				
				printk("[TOUCH] Firmware update success! [Melfas H/W version: 0x%02x., Current F/W version: 0x%02x.]\n", melfas_ts->hw_rev, melfas_ts->fw_ver);

		}
		else {
			printk("[TOUCH] Firmware update failed.. RESET!\n");
      mcsdl_vdd_off();
			mdelay(500);
      mcsdl_vdd_on();
			mdelay(200);
		}
	}
#endif

	return size;
}



static ssize_t debug_show(struct device *dev, struct device_attribute *attr, char *buf)
{	
	return sprintf(buf, "%d", debug_level);
}

static ssize_t debug_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	if(buf[0]>'0' && buf[0]<='9') {
		debug_level = buf[0] - '0';
	}

	return size;
}

static DEVICE_ATTR(gpio, S_IRUGO | S_IWUSR, gpio_show, gpio_store);
static DEVICE_ATTR(registers, S_IRUGO | S_IWUSR, registers_show, registers_store);
static DEVICE_ATTR(firmware, S_IRUGO | S_IWUSR, firmware_show, firmware_store);
static DEVICE_ATTR(debug, S_IRUGO | S_IWUSR, debug_show, debug_store);


void melfas_ts_work_func(struct work_struct *work)
{
  int ret;
  int ret1;
  int i = 0;
  u8 id = 0;
  static int PreState = 1;

  struct i2c_msg msg[2];
  
  uint8_t start_reg;
  uint8_t buf1[6]; // 8-> 6 melfas recommand

//  printk(KERN_ERR "==!!== melfas_ts_work_func \n");		// heatup - test
  
  msg[0].addr = melfas_ts->client->addr;
  msg[0].flags = 0; 
  msg[0].len = 1;
  msg[0].buf = &start_reg;
  start_reg = MCSTS_INPUT_INFO_REG;
  msg[1].addr = melfas_ts->client->addr;
  msg[1].flags = I2C_M_RD; 
  msg[1].len = sizeof(buf1);
  msg[1].buf = buf1;
  

  ret  = i2c_transfer(melfas_ts->client->adapter, &msg[0], 1);
  ret1 = i2c_transfer(melfas_ts->client->adapter, &msg[1], 1);
  
//  printk(KERN_ERR "==!!== ret:%d ,ret1:%d\n",ret,ret1);
  if((ret < 0) ||  (ret1 < 0)) 
  {
  	printk(KERN_ERR "==melfas_ts_work_func: i2c_transfer failed!!== ret:%d ,ret1:%d\n",ret,ret1);
  }
  else
  {    
    int x = buf1[2] | ((uint16_t)(buf1[1] & 0x0f) << 8); 
    int y = buf1[3] | (((uint16_t)(buf1[1] & 0xf0) >> 4) << 8); 
    int z = buf1[4];
    int finger = buf1[0] & 0x0f;  //Touch Point ID
    int touchaction = (int)((buf1[0] >> 4) & 0x3); //Touch action
    int touchtype = (int)((buf1[0] >> 6) & 0x3);
#ifdef CONFIG_CPU_FREQ
//    set_dvfs_perf_level();
#endif
//		printk("==!!== touchaction:[%d]..touchtype:[%d]\n",touchaction,touchtype);
    finger = finger -1; // melfas touch  started  touch finger id from the  index 1
    id = finger; // android input id : 0~ 
//	  printk("===touchaction : [%d]=== \n",touchaction);

      switch(touchaction) {
        case 0x0: // Non-touched state (Rlease Event)

         if(melfas_ts->info[id].z == -1)
         {
              enable_irq(melfas_ts->irq);
              return ;
         }
        
			//melfas_ts->info[id].x = -1;
			//melfas_ts->info[id].y = -1;
			//melfas_ts->info[id].z = -1;
			//melfas_ts->info[id].finger_id = finger; 
			//z = 0;
//			debugprintk(5," TOUCH RELEASE\n");			
			melfas_ts->info[id].x = x;
			melfas_ts->info[id].y = y;
			melfas_ts->info[id].z = 0;
			melfas_ts->info[id].finger_id = finger;
			z = 0;
//			s5pc110_unlock_dvfs_high_level(DVFS_LOCK_TOKEN_4);
			
          break;

        case 0x1: //touched state (Press Event)

            if(melfas_ts->info[id].x == x
            && melfas_ts->info[id].y == y
            && melfas_ts->info[id].z == z
            && melfas_ts->info[id].finger_id == finger)
            {
                enable_irq(melfas_ts->irq);
                return ;
            }
        
			melfas_ts->info[id].x = x;
			melfas_ts->info[id].y = y;
			melfas_ts->info[id].z = z;
			melfas_ts->info[id].finger_id = finger; 
//			if(melfas_ts->info[id].z == 0) // generated duplicate event 
//				return;
//			s5pc110_lock_dvfs_high_level(DVFS_LOCK_TOKEN_4,0);
//			 debugprintk(5," TOUCH PRESS\n");		 
          break;

        case 0x2: 

          break;

        case 0x3: // Palm Touch
          printk(KERN_DEBUG "[TOUCH] Palm Touch!\n");
          break;

        case 0x7: // Proximity
          printk(KERN_DEBUG "[TOUCH] Proximity!\n");
          break;
      }
	  
      melfas_ts->info[id].state = touchaction;
//		if(touchaction) // press
//			melfas_ts->info[id].z = z;
//		else // release
//			melfas_ts->info[id].z = 0;
//		if(touchaction == 0x1 && (melfas_ts->info[id].z == 0))// duplicate touch work
//			return;
//		if(nPreID >= id || bChangeUpDn)
		if(((touchaction == 1)&&(z!=0))||((touchaction == 0)&&(z==0)))
		{
			if(((touchaction == 1)&&(z!=0)))
				s5pv210_lock_dvfs_high_level(DVFS_LOCK_TOKEN_4,0);    // heatup - chief
			else
				s5pv210_unlock_dvfs_high_level(DVFS_LOCK_TOKEN_4);    // heatup - chief
			
		  	for ( i= 0; i<FINGER_NUM; ++i ) 
			{
				if(melfas_ts->info[i].z== -1) continue;
				input_report_abs(melfas_ts->input_dev, ABS_MT_POSITION_X, melfas_ts->info[i].x);
				input_report_abs(melfas_ts->input_dev, ABS_MT_POSITION_Y, melfas_ts->info[i].y);
				input_report_abs(melfas_ts->input_dev, ABS_MT_TOUCH_MAJOR, melfas_ts->info[i].z ? 40 : 0);		
//				input_report_abs(melfas_ts->input_dev, ABS_MT_TRACKING_ID, melfas_ts->info[i].finger_id);
//				input_report_abs(melfas_ts->input_dev, ABS_MT_WIDTH_MAJOR, 3/*melfas_ts->info[i].finger_id*/);		
                input_report_abs(melfas_ts->input_dev, ABS_MT_WIDTH_MAJOR, (melfas_ts->info[i].finger_id * 0xff) + 5);

				input_mt_sync(melfas_ts->input_dev);
	//			debugprintk(5,"[TOUCH_MT] x1: %4d, y1: %4d, z1: %4d, finger: %4d, i=[%d] \n", x, y,(melfas_ts->info[id].z), finger,i);			
	//			input_sync(melfas_ts->input_dev);
				if(melfas_ts->info[i].z == 0)
					melfas_ts->info[i].z = -1;
			}
    		input_sync(melfas_ts->input_dev);	
	 }
//		PreState = touchaction;
  }

 // if(readl(gpio_pend_mask_mem)&INT_BIT_MASK)
//		 writel(readl(gpio_pend_mask_mem)|INT_BIT_MASK, gpio_pend_mask_mem);
  
//  s3c_gpio_cfgpin(GPIO_TOUCH_INT, S3C_GPIO_SFN(0xf));

  enable_irq(melfas_ts->irq);
}


irqreturn_t melfas_ts_irq_handler(int irq, void *dev_id)
{
//	s3c_gpio_cfgpin(GPIO_TOUCH_INT, S3C_GPIO_INPUT);
//	printk("*************************TOUCH INTERRUPT****************\n");	
//	disable_irq_nosync(IRQ_TOUCH_INT);
	disable_irq_nosync(melfas_ts->irq);
	//disable_irq(melfas_ts->irq);
	queue_work(melfas_ts_wq, &melfas_ts->work);
	return IRQ_HANDLED;
}

int melfas_ts_probe(void)
{
	int ret = 0;
	uint16_t max_x=0, max_y=0;

	printk("\n====================================================");
	printk("\n=======         [TOUCH SCREEN] PROBE       =========");
	printk("\n====================================================\n");


	if (!i2c_check_functionality(melfas_ts->client->adapter, I2C_FUNC_I2C/*I2C_FUNC_I2C*//*I2C_FUNC_SMBUS_BYTE_DATA*/)) {
		printk(KERN_ERR "melfas_ts_probe: need I2C_FUNC_I2C\n");
		ret = -ENODEV;
		goto err_check_functionality_failed;
	}
//	printk(" melfas_ts_work_func START\n");
	
	INIT_WORK(&melfas_ts->work, melfas_ts_work_func);
	
	melfas_read_version(); 

	printk(KERN_INFO "[TOUCH] Melfas  H/W version: 0x%02x.\n", melfas_ts->hw_rev);
	printk(KERN_INFO "[TOUCH] Current F/W version: 0x%02x.\n", melfas_ts->fw_ver);
	
	melfas_read_resolution();
	max_x = melfas_ts->info[0].max_x ;
	max_y = melfas_ts->info[0].max_y ;
	printk("melfas_ts_probe: max_x: %d, max_y: %d\n", max_x, max_y);

	melfas_ts->input_dev = input_allocate_device();
	if (melfas_ts->input_dev == NULL) {
		ret = -ENOMEM;
		printk(KERN_ERR "melfas_ts_probe: Failed to allocate input device\n");
		goto err_input_dev_alloc_failed;
	}

	melfas_ts->input_dev->name = "melfas_ts_input";
#if 1
	set_bit(EV_SYN, melfas_ts->input_dev->evbit);
	set_bit(EV_KEY, melfas_ts->input_dev->evbit);
	set_bit(BTN_TOUCH, melfas_ts->input_dev->keybit);
	set_bit(EV_ABS, melfas_ts->input_dev->evbit);
#else
	set_bit(EV_SYN, melfas_ts->input_dev->evbit);
	set_bit(EV_KEY, melfas_ts->input_dev->evbit);
	set_bit(TOUCH_HOME, melfas_ts->input_dev->keybit);
	set_bit(TOUCH_MENU, melfas_ts->input_dev->keybit);
	set_bit(TOUCH_BACK, melfas_ts->input_dev->keybit);
	set_bit(TOUCH_SEARCH, melfas_ts->input_dev->keybit);

	melfas_ts->input_dev->keycode = melfas_ts_tk_keycode;	
	
	set_bit(BTN_TOUCH, melfas_ts->input_dev->keybit);
	set_bit(EV_ABS, melfas_ts->input_dev->evbit);
#endif
#if 1
	input_set_abs_params(melfas_ts->input_dev, ABS_X, 0, 319/*479*/, 0, 0);
	input_set_abs_params(melfas_ts->input_dev, ABS_Y, 0, 479/*799*/, 0, 0);
	input_set_abs_params(melfas_ts->input_dev, ABS_MT_POSITION_X, 0, 319/*479*/, 0, 0);
	input_set_abs_params(melfas_ts->input_dev, ABS_MT_POSITION_Y, 0, 479/*799*/, 0, 0);
	input_set_abs_params(melfas_ts->input_dev, ABS_PRESSURE, 0, 255, 0, 0);
	input_set_abs_params(melfas_ts->input_dev, ABS_TOOL_WIDTH, 0, 15, 0, 0);
	input_set_abs_params(melfas_ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(melfas_ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, 30, 0, 0);
#else
	input_set_abs_params(melfas_ts->input_dev, ABS_MT_TRACKING_ID, 0, 10, 0, 0);
	input_set_abs_params(melfas_ts->input_dev, ABS_MT_POSITION_X, 0, max_x, 0, 0);
	input_set_abs_params(melfas_ts->input_dev, ABS_MT_POSITION_Y, 0, max_y, 0, 0);
	input_set_abs_params(melfas_ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
#endif
	printk("melfas_ts_probe: max_x: %d, max_y: %d\n", max_x, max_y);

	ret = input_register_device(melfas_ts->input_dev);
	if (ret) {
		printk(KERN_ERR "melfas_ts_probe: Unable to register %s input device\n", melfas_ts->input_dev->name);
		goto err_input_register_device_failed;
	}

	melfas_ts->client->irq = IRQ_TOUCH_INT;	// heatup - test
	melfas_ts->irq = melfas_ts->client->irq; //add by KJB
	
	ret = request_irq(melfas_ts->client->irq, melfas_ts_irq_handler, IRQF_DISABLED, "melfas_ts irq", 0);
	if(ret == 0) {
		printk(KERN_INFO "melfas_ts_probe: Start touchscreen %s \n", melfas_ts->input_dev->name);
	}
	else {
		printk("request_irq failed\n");
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	melfas_ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	melfas_ts->early_suspend.suspend = melfas_ts_early_suspend;
	melfas_ts->early_suspend.resume = melfas_ts_late_resume;
	register_early_suspend(&melfas_ts->early_suspend);
#endif	/* CONFIG_HAS_EARLYSUSPEND */

	return 0;
err_misc_register_device_failed:
err_input_register_device_failed:
	input_free_device(melfas_ts->input_dev);

err_input_dev_alloc_failed:
err_detect_failed:
	kfree(melfas_ts);
err_alloc_data_failed:
err_check_functionality_failed:
	return ret;

}

int melfas_ts_remove(struct i2c_client *client)
{
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&melfas_ts->early_suspend);
#endif	/* CONFIG_HAS_EARLYSUSPEND */
	free_irq(melfas_ts->irq, 0);
	input_unregister_device(melfas_ts->input_dev);
	return 0;
}

int melfas_ts_gen_touch_up(void)
{
  // report up key if needed
  int i;
  for ( i= 0; i<FINGER_NUM; ++i ){
  	if(melfas_ts->info[i].state == 0x1){ /*down state*/

		melfas_ts->info[i].state = 0x0;
		int finger = melfas_ts->info[i].finger_id;
    	int x = melfas_ts->info[i].x;
    	int y = melfas_ts->info[i].y;
    	int z = melfas_ts->info[i].z;
    	printk("[TOUCH] GENERATE UP KEY x: %4d, y: %4d, z: %4d\n", x, y, z);
		input_report_abs(melfas_ts->input_dev, ABS_MT_TRACKING_ID, finger);

		if (x) 	input_report_abs(melfas_ts->input_dev, ABS_MT_POSITION_X, x);
    	if (y)	input_report_abs(melfas_ts->input_dev, ABS_MT_POSITION_Y, y);

		input_report_abs(melfas_ts->input_dev, ABS_PRESSURE, z);

    input_sync(melfas_ts->input_dev);
		}
  }    
}

int melfas_ts_suspend(pm_message_t mesg)
{
	int i=0;
	
  melfas_ts->suspended = true;
  melfas_ts_gen_touch_up();
  disable_irq(melfas_ts->irq);

#if 0 // heatup
  s3c_gpio_cfgpin(GPIO_TOUCH_I2C_SCL, S3C_GPIO_OUTPUT); s3c_gpio_setpull(GPIO_TOUCH_I2C_SCL,S3C_GPIO_PULL_NONE);
  s3c_gpio_cfgpin(GPIO_TOUCH_I2C_SDA, S3C_GPIO_OUTPUT); s3c_gpio_setpull(GPIO_TOUCH_I2C_SDA,S3C_GPIO_PULL_NONE);
//  gpio_tlmm_config(GPIO_CFG(GPIO_I2C0_SCL, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA),GPIO_CFG_ENABLE);
//  gpio_tlmm_config(GPIO_CFG(GPIO_I2C0_SDA, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA),GPIO_CFG_ENABLE);
#endif

  for(i=0; i<FINGER_NUM; i++)
  {
	  melfas_ts->info[i].z = -1;
  }
//  mcsdl_vdd_off();
// gpio's sleep current
//  s3c_gpio_cfgpin(GPIO_TOUCH_INT,S3C_GPIO_INPUT);
//  s3c_gpio_setpull(GPIO_TOUCH_INT,S3C_GPIO_PULL_DOWN);
  
  gpio_set_value(GPIO_TOUCH_EN,0);
  gpio_set_value(GPIO_TOUCH_I2C_SCL, 0);  // TOUCH SCL DIS
  gpio_set_value(GPIO_TOUCH_I2C_SDA, 0);  // TOUCH SDA DIS
  msleep(1);
  
  printk(KERN_INFO "%s: melfas_ts_suspend!\n", __func__);
  return 0;
}

int melfas_ts_resume(void)
{
#if 0 // heatup
	s3c_gpio_cfgpin(GPIO_TOUCH_I2C_SCL, S3C_GPIO_OUTPUT); s3c_gpio_setpull(GPIO_TOUCH_I2C_SCL,S3C_GPIO_PULL_UP);
	s3c_gpio_cfgpin(GPIO_TOUCH_I2C_SDA, S3C_GPIO_OUTPUT); s3c_gpio_setpull(GPIO_TOUCH_I2C_SDA,S3C_GPIO_PULL_UP);
	
//  gpio_tlmm_config(GPIO_CFG(GPIO_I2C0_SCL, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_16MA),GPIO_CFG_ENABLE);
//  gpio_tlmm_config(GPIO_CFG(GPIO_I2C0_SDA, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_16MA),GPIO_CFG_ENABLE);
#endif

 // s3c_gpio_cfgpin(GPIO_TOUCH_INT,S3C_GPIO_INPUT);
//  s3c_gpio_setpull(GPIO_TOUCH_INT,S3C_GPIO_PULL_UP);
//  s3c_gpio_cfgpin(GPIO_TOUCH_INT, S3C_GPIO_SFN(0xf));
  
//  mcsdl_vdd_on();
  gpio_set_value(GPIO_TOUCH_EN,1);
  msleep(1);
  gpio_set_value(GPIO_TOUCH_I2C_SCL, 1);  // TOUCH SCL EN
  gpio_set_value(GPIO_TOUCH_I2C_SDA, 1);  // TOUCH SDA EN    
  msleep(210);//300-> Minimum stable time 200msec for MMS100 series after power on
  melfas_ts->suspended = false;
  enable_irq(melfas_ts->irq);  
  printk(KERN_INFO "%s: melfas_ts_resume!\n", __func__);
  return 0;
}

int tsp_preprocess_suspend(void)
{
#if 0 // blocked for now.. we will gen touch when suspend func is called
  // this function is called before kernel calls suspend functions
  // so we are going suspended if suspended==false
  if(melfas_ts->suspended == false) {  
    // fake as suspended
    melfas_ts->suspended = true;
    
    //generate and report touch event
    melfas_ts_gen_touch_up();
  }
#endif
  return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
void melfas_ts_early_suspend(struct early_suspend *h)
{
	melfas_ts_suspend(PMSG_SUSPEND);
}

void melfas_ts_late_resume(struct early_suspend *h)
{
	melfas_ts_resume();
}
#endif	/* CONFIG_HAS_EARLYSUSPEND */


int melfas_i2c_probe(struct i2c_client *client,const struct i2c_device_id *id)
{
	melfas_ts->client = client;
	i2c_set_clientdata(client, melfas_ts);
	return 0;
}

static int __devexit melfas_i2c_remove(struct i2c_client *client)
{
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&melfas_ts->early_suspend);
#endif  /* CONFIG_HAS_EARLYSUSPEND */
	free_irq(melfas_ts->client->irq, 0);
	input_unregister_device(melfas_ts->input_dev);
   
	melfas_ts = i2c_get_clientdata(client);
	kfree(melfas_ts);
	return 0;
}

struct i2c_device_id melfas_id[] = {
	{ "melfas_ts_i2c", 0 },
	{ }
};

struct i2c_driver melfas_ts_i2c = {
	.driver = {
		.name	= "melfas_ts_i2c",
		.owner	= THIS_MODULE,
	},
	.probe 		= melfas_i2c_probe,
	.remove		= __devexit_p(melfas_i2c_remove),
	.id_table	= melfas_id,
};


void init_hw_setting(void)
{
	int ret;

#if 0
	vreg_touch = vreg_get(NULL, "wlan2"); /* VTOUCH_2.8V */
	vreg_touchio = vreg_get(NULL, "gp13"); /* VTOUCHIO_1.8V */
	
	ret = vreg_enable(vreg_touch);
#endif	
	if (!(gpio_get_value(GPIO_TOUCH_EN))) 
	{
		gpio_direction_output(GPIO_TOUCH_EN,1);
		printk(KERN_ERR "%s: vreg_touch enable failed (%d)\n", __func__, ret);
//		return -EIO;
	}
	else 
	{
		printk(KERN_INFO "%s: vreg_touch enable success!\n", __func__);
	}
#if 0	
	ret = vreg_enable(vreg_touchio);
	
	if (ret) { 
		printk(KERN_ERR "%s: vreg_touchio enable failed (%d)\n", __func__, ret);
		return -EIO;
	}
	else {
		printk(KERN_INFO "%s: vreg_touchio enable success!\n", __func__);
	}
#endif
	mdelay(100);
	
//	s3c_gpio_cfgpin(GPIO_TOUCH_I2C_SCL, S3C_GPIO_OUTPUT); s3c_gpio_setpull(GPIO_TOUCH_I2C_SCL,S3C_GPIO_PULL_UP);
//	s3c_gpio_cfgpin(GPIO_TOUCH_I2C_SDA, S3C_GPIO_OUTPUT); s3c_gpio_setpull(GPIO_TOUCH_I2C_SDA,S3C_GPIO_PULL_UP);
	s3c_gpio_cfgpin(GPIO_TOUCH_INT, S3C_GPIO_SFN(0xf)/*S3C_GPIO_INPUT*/); 
	s3c_gpio_setpull(GPIO_TOUCH_INT,S3C_GPIO_PULL_UP);
	
//	gpio_tlmm_config(GPIO_CFG(GPIO_I2C0_SCL, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_16MA),GPIO_CFG_ENABLE);
//	gpio_tlmm_config(GPIO_CFG(GPIO_I2C0_SDA, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_16MA),GPIO_CFG_ENABLE);
//	gpio_tlmm_config(GPIO_CFG(GPIO_TOUCH_INT, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_16MA),GPIO_CFG_ENABLE);

	set_irq_type(IRQ_TOUCH_INT, IRQ_TYPE_LEVEL_LOW); //chief.boot.temp changed from edge low to level low VERIFY!!!

	mdelay(10);

}

struct platform_driver melfas_ts_driver =  {
	.probe	= melfas_ts_probe,
	.remove = melfas_ts_remove,
	.driver = {
		.name = "melfas-ts",
		.owner	= THIS_MODULE,
	},
};


int __init melfas_ts_init(void)
{
	int ret;
	printk("\n====================================================");
	printk("\n=======         [TOUCH SCREEN] INIT        =========");
	printk("\n====================================================\n");

	init_hw_setting();

	ts_dev = device_create(sec_class, NULL, 0, NULL, "ts");
	if (IS_ERR(ts_dev))
		pr_err("Failed to create device(ts)!\n");
	if (device_create_file(ts_dev, &dev_attr_gpio) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_gpio.attr.name);
	if (device_create_file(ts_dev, &dev_attr_registers) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_registers.attr.name);
	if (device_create_file(ts_dev, &dev_attr_firmware) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_firmware.attr.name);
	if (device_create_file(ts_dev, &dev_attr_debug) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_debug.attr.name);

	melfas_ts = kzalloc(sizeof(struct melfas_ts_driver), GFP_KERNEL);
	if(melfas_ts == NULL) {
		return -ENOMEM;
	}

	ret = i2c_add_driver(&melfas_ts_i2c);
	if(ret) printk("[%s], i2c_add_driver failed...(%d)\n", __func__, ret);

	printk(KERN_ERR "[HEATUP] ret : %d, melfas_ts->client name : %s\n",ret,melfas_ts->client->name);


	if(!melfas_ts->client) {
		printk("###################################################\n");
		printk("##                                               ##\n");
		printk("##    WARNING! TOUCHSCREEN DRIVER CAN'T WORK.    ##\n");
		printk("##    PLEASE CHECK YOUR TOUCHSCREEN CONNECTOR!   ##\n");
		printk("##                                               ##\n");
		printk("###################################################\n");
		i2c_del_driver(&melfas_ts_i2c);
		return 0;
	}
	melfas_ts_wq = create_singlethread_workqueue("melfas_ts_wq");
	if (!melfas_ts_wq)
		return -ENOMEM;

	return platform_driver_register(&melfas_ts_driver);

}

void __exit melfas_ts_exit(void)
{
	i2c_del_driver(&melfas_ts_i2c);
	if (melfas_ts_wq)
		destroy_workqueue(melfas_ts_wq);
}
late_initcall(melfas_ts_init);
//module_init(melfas_ts_init);
module_exit(melfas_ts_exit);

MODULE_DESCRIPTION("Melfas Touchscreen Driver");
MODULE_LICENSE("GPL");
