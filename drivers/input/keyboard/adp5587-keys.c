/*description:  keypad driver for ADP5588 and ADP5587
 *               I2C QWERTY Keypad and IO Expander *
 * 
 * Licensed under the GPL-2 or later.
 */
#include <linux/module.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/workqueue.h>
#include <linux/errno.h>
#include <linux/pm.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/sysfs.h>
#include <linux/i2c/adp5587.h>
#include <linux/earlysuspend.h>
#include <linux/io.h>
#include <linux/regulator/consumer.h>
#include <mach/gpio.h>
#include <mach/irqs.h>
#include <mach/regs-gpio.h>

#include <plat/gpio-cfg.h>
#include <asm/io.h>

#include "../../../arch/arm/mach-msm/proc_comm.h"

#ifdef CONFIG_DEBUG_KEY_ACTIVE
#define LOCKUP_CAPTURE
#endif

 /* Configuration Register1 */
#define AUTO_INC        (1 << 7)
#define GPIEM_CFG       (1 << 6)
#define OVR_FLOW_M      (1 << 5)
#define INT_CFG         (1 << 4)
#define OVR_FLOW_IEN    (1 << 3)
#define K_LCK_IM        (1 << 2)
#define GPI_IEN         (1 << 1)
#define KE_IEN          (1 << 0)

/* Interrupt Status Register */
#define CMP2_INT        (1 << 5)
#define CMP1_INT        (1 << 4)
#define OVR_FLOW_INT    (1 << 3)
#define K_LCK_INT       (1 << 2)
#define GPI_INT         (1 << 1)
#define KE_INT          (1 << 0)

/* Key Lock and Event Counter Register */
#define K_LCK_EN        (1 << 6)
#define LCK21           0x30
#define KEC             0xF

/* Key Event Register xy */
#define KEY_EV_PRESSED          (1 << 7)
#define KEY_EV_MASK             (0x7F)

#define KP_SEL(x)               (0xFFFF >> (16 - x))    /* 2^x-1 */

#define KEYP_MAX_EVENT          10

#define ADP5587_DEVICE_ID_MASK  0xF /* 00001111 */
#define ADP5587_KEYMAPSIZE      80  /* KE[6:0] reflects the value 1 to 80 for key press events */

#define HALL_GPIO_124               124
#define HALL_GPIO_28           28
static int HALL_GPIO;
static int HALL_IRQ;
#if (CONFIG_BOARD_REVISION >= 4) /* Volume up key */
#define VOLUMEUP_GPIO           36
#define VOLUME_UP               61
#define VOLUME_DOWN             67
#endif
#define NUM_CAPTURE_KEYS 3

#define ON        1
#define OFF       0

#define DEFAULT_KEYPADBACKLIGHT_TIMEOUT  3
#define FEATURE_DISABLE_SUBKEY //Quattro use only Mainkey
#define POPUP_DELAY 1000
#define DELAY_JIFFIES   ((HZ / 10) * (POPUP_DELAY / 100)) + (HZ / 100) * ((POPUP_DELAY % 100) / 10)
/*
 * Early pre 4.0 Silicon required to delay readout by at least 25ms,
 * since the Event Counter Register updated 25ms after the interrupt
 * asserted.
 */

#define WA_DELAYED_READOUT_REVID(rev)           ((rev) < 4)

#define IRQ_ADP5587_KEY_INT  IRQ_EINT(24) 

static int slide_on_off=0;

struct early_suspend    early_suspend;
static short int  backlight_flag = 1;
void keypad_early_suspend(struct early_suspend *h);
void keypad_late_resume(struct early_suspend *h);
static void key_led_onoff(bool sw);

struct adp5587_kpad {
	struct i2c_client *client;
	struct input_dev *input;
	struct delayed_work work;
	struct delayed_work backlightoff;
	struct delayed_work hall_ic_work;
	unsigned long delay;
	unsigned int keycode[ADP5587_KEYMAPSIZE];
};
/* Keypad Backlight time */
unsigned int backlight_time = DEFAULT_KEYPADBACKLIGHT_TIMEOUT ;
/* sysfs class registration */
struct class *bl_class;
/* sysfs device registration */
struct device *kpd_dev;

/* sys fs : /sys/devices/virtual/key/key/key */
struct class *key_class;
struct device *key_dev;
#if defined(CONFIG_MACH_AEGIS) || defined(CONFIG_MACH_VIPER) || defined(CONFIG_MACH_CHIEF)
struct regulator *keyled_regulator;
static bool key_led_stat = false;  //status for keypad led
#endif
static int key_pressed;
struct i2c_client *client_clone;

extern unsigned char Quattro_hw_version;

#define HALL_IC_PORT_CHANGE 2

//hall_ic_delay_work queue
static struct workqueue_struct *hall_ic_workqueue = NULL;
static void hall_ic_handler(struct work_struct *unused);
unsigned int hall_ic_workqueue_statue = 0;


static int adp5587_read(struct i2c_client *client, u8 reg)
{
	int ret;
	ret = i2c_smbus_read_byte_data(client, reg);

	if (ret < 0)
	dev_err(&client->dev, "Read Error\n");

	return ret;
}

static int adp5587_write(struct i2c_client *client, u8 reg, u8 val)
{
	return i2c_smbus_write_byte_data(client, reg, val);
}

void popup_switch_on(struct work_struct *ignored)
{
	adp5587_write(client_clone, ADP5587_REG_DAT_OUT3, 0x02); /* Status is W1C */
}

static DECLARE_DELAYED_WORK(popup_on_work, popup_switch_on);

static ssize_t key_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int count,mask;

	mask = gpio_get_value(VOL_UP) & gpio_get_value(VOL_DOWN)& gpio_get_value(GPIO_nPOWER);
		
	if((mask==0)||(key_pressed!=0))
	{
		count = sprintf(buf,"PRESS\n");
		printk("keyshort_test: PRESS 0x%x\n", mask);
	}
	else
	{
		count = sprintf(buf,"RELEASE\n");
		printk("keyshort_test: RELEASE 0x%x\n", mask);
	}	

	return count;
}

static ssize_t key_store(struct device *dev, struct device_attribute *attr, char *buf, size_t count)
{
	char value[120];
	
	sscanf(buf, "%s", value);
	
	if(value[0] =='1')
		schedule_delayed_work(&popup_on_work, DELAY_JIFFIES);
	else
		adp5587_write(client_clone, ADP5587_REG_DAT_OUT3, 0x00); /* Status is W1C */
}
static DEVICE_ATTR(key , 0664, key_show, key_store);

static ssize_t led_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", key_pressed );
}

static ssize_t led_store(struct device *dev, struct device_attribute *attr, char *buf, size_t count)
{
	char value[120];
	
	sscanf(buf, "%s", value);
	
	if(value[0] =='1')
		key_led_onoff(true);
	else
		key_led_onoff(false);
}
static DEVICE_ATTR(led , 0664, led_show, led_store);

static ssize_t slide_on_off_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int lid_status = gpio_get_value(HALL_GPIO);
	// 1 => slide open
	// 0 => slide close

	return sprintf(buf, "%d\n", lid_status );
}

static DEVICE_ATTR(slide_on_off , 0664, slide_on_off_show, NULL);


#ifdef FEATURE_DISABLE_SUBKEY
static int keypad_backlight_power(int mainkey);
#else
static int keypad_backlight_power(int mainkey, int subkey);
#endif

#if (CONFIG_BOARD_REVISION >= 4)
static int convert_keycode(int key_code);
#endif

#ifdef LOCKUP_CAPTURE
//#include "../../../arch/arm/mach-msm/smd_private.h"
//#include "../../../arch/arm/mach-msm/proc_comm.h"
//#include <mach/msm_iomap-7xxx.h>
//#include <mach/msm_iomap.h>

static int lockup_capture_check(u8 keycode, u8 keypress)
{
	static int capture_keys[NUM_CAPTURE_KEYS]={1, 51, 31}; //menu + search + power
	int i;
	int capture_key_cnt = 0;

	for(i=0;i<NUM_CAPTURE_KEYS;i++) {
		if(keycode == (capture_keys[i]&~0x80000000)) {
			if(keypress)
				capture_keys[i]|=0x80000000;
			else
				capture_keys[i]&=~0x80000000;
		}
		if(capture_keys[i]&0x80000000)
			capture_key_cnt++;
	}
}
#else
#define lockup_capture_check(keycode, keypress)
#endif

static void adp5587_work(struct work_struct *work)
{
	struct adp5587_kpad *kpad = container_of(work, struct adp5587_kpad, work.work);
	struct i2c_client *client = kpad->client;
	int i, key, status, ev_cnt, keypress;
	unsigned int keycode;

	//int slide_state = gpio_get_value(HALL_GPIO);

	/* Cancel the scheduled work in case key press are continous */
	cancel_delayed_work_sync(&kpad->backlightoff);

	status = adp5587_read(client, ADP5587_REG_INT_STAT);

	if (status & OVR_FLOW_INT)      /* Unlikely and should never happen */
		dev_err(&client->dev, "Event Overflow Error\n");

	if (status & KE_INT){
		ev_cnt = adp5587_read(client, ADP5587_REG_KEY_LCK_EC_STAT) & KEC;
		if (ev_cnt){
			for (i = 0; i < ev_cnt; i++){
				key = adp5587_read(client, ADP5587_REG_KEY_EVENTA + i);
				keycode = kpad->keycode[(key & KEY_EV_MASK) - 1];
				
		#if defined(CONFIG_MACH_AEGIS) || defined(CONFIG_MACH_VIPER) || defined(CONFIG_MACH_CHIEF)
   				//The below changes are to support the Key code requirements of platform
				//Note: space right, space left and web should come as spaceright
				if((key==144 && keycode==16) ||(key==145 && keycode==150))
				{		
					key = 143;
					keycode = 57;
				}
	
				if((key==16 && keycode==16) ||(key==17 && keycode==150))
				{		
					key = 15;
					keycode = 57;
				}

				// Msg should come as web
				if(key==146 && keycode==155)
				{		
					key = 145;
					keycode = 150;
				}
	
				if(key==18 && keycode==155)
				{		
					key = 17;
					keycode = 150;
				}

				// App should come as Msg
				if(key==147 && keycode==301)
				{		
					key = 146;
					keycode = 155;
				}
	
				if(key==19 && keycode==301)
				{		
					key = 18;
					keycode = 155;
				}
			#endif


				keypress = key & KEY_EV_PRESSED;
				key_pressed = keypress;

				if(slide_on_off==0){  
					printk("slide off qwerty ignore!!!\n");
				}
				else{
					input_report_key(kpad->input, keycode, keypress);
					input_sync(kpad->input);
#ifdef CONFIG_SEC_KEY_DBG					
					printk("[qwerty] key %d, keycode %d, keypress %d\n", key, keycode, keypress);
#else
					printk("[qwerty]\n");
#endif
					lockup_capture_check(keycode, keypress);
				}
			}
		}
	}
	adp5587_write(client, ADP5587_REG_INT_STAT, status); /* Status is W1C */
	//printk(" Scheduling delayed work for backlight off in adp5587_work \n");
}

static irqreturn_t adp5587_irq(int irq, void *handle)
{
	struct adp5587_kpad *kpad = handle;

	/*
	* use keventd context to read the event fifo registers
	* Schedule readout at least 25ms after notification for
	* REVID < 4
	*/
	schedule_delayed_work(&kpad->work, kpad->delay);

	return IRQ_HANDLED;
}

#if defined(CONFIG_MACH_AEGIS) || defined(CONFIG_MACH_VIPER) || defined(CONFIG_MACH_CHIEF)
static void key_led_onoff(bool sw)
{
	int ret=0;
        if(sw)
        {
		if (!regulator_is_enabled(keyled_regulator)) {
	                ret = regulator_enable(keyled_regulator);
	                if (ret < 0)
        	                printk(" failed to enable regulator\n");
                }
                key_led_stat = true;
        }
        else
        {
		if (regulator_is_enabled(keyled_regulator)) {
	                ret = regulator_disable(keyled_regulator);
        	        if (ret < 0)
                	        printk(" failed to disable regulator\n");
		}
                key_led_stat = false;
        }
}
#endif

#if defined(CONFIG_MACH_AEGIS) || defined(CONFIG_MACH_VIPER) || defined(CONFIG_MACH_CHIEF)
static irqreturn_t slide_irq(int irq, void *handle)
{
	printk("slide_irq\n");
	struct adp5587_kpad *kpad = handle;

	if(hall_ic_workqueue_statue == 1) {
		printk("%s : delayed workqueue cancel reqeust\n",__func__);
		cancel_delayed_work(&kpad->hall_ic_work);
		hall_ic_workqueue_statue = 0;
	}

	queue_delayed_work(hall_ic_workqueue,&kpad->hall_ic_work,HZ/10);
	hall_ic_workqueue_statue = 1;

	return IRQ_HANDLED;
}
#endif

static void hall_ic_handler(struct work_struct *work)
{
	printk("%s called\n",__func__);
	struct adp5587_kpad *kpad = container_of(work, struct adp5587_kpad, hall_ic_work.work);
	struct input_dev *input = kpad->input;
	int lid_status = gpio_get_value(HALL_GPIO);

	if (lid_status)
	{
		input->sw[SW_LID] = 1;
		slide_on_off=1; 
#ifdef CONFIG_SEC_KEY_DBG		
		printk("slide On\n");	
#endif
	}
	else{
		input->sw[SW_LID] = 0;
		slide_on_off=0; 	
#ifdef CONFIG_SEC_KEY_DBG		
		printk("slide Off\n");				
#endif
	}

	input_report_switch(input, SW_LID, (lid_status ? 0: 1));
	input_sync(input);
	hall_ic_workqueue_statue = 0;
}

static int __devinit adp5587_setup(struct i2c_client *client)
{
	int i;
	/* Program Row0 thru Row7 as Keypad */
	adp5587_write(client, ADP5587_REG_KP_GPIO1, 0xFF);

	/* Program Column0 thru Column 7 as Keypad */
	adp5587_write(client, ADP5587_REG_KP_GPIO2, 0xFF); // col7 gpio Ÿ  

	adp5587_write(client, ADP5587_REG_KP_GPIO3, 0xFF); // quattro_jiny46kim 0x01->0x00 8,9 use for GPIO

	/* Disable pullup for Row0 thru Row7 */
	adp5587_write(client, ADP5587_REG_GPIO_PULL1, 0xFF);
	/* Disable pullup for Col0 thru Col7 */
	adp5587_write(client, ADP5587_REG_GPIO_PULL2, 0xFF);  // 1111 1111 -> col7 pullup disabled.

	/* Disable pullup for Col8 and Col9 */
	adp5587_write(client, ADP5587_REG_GPIO_PULL3, 0xFF); // quattro_jiny46kim 0x01 -> col8 pullup disabled

	for(i = 0; i < KEYP_MAX_EVENT; i++){
		adp5587_read(client, ADP5587_REG_KEY_EVENTA+i);
	}

	adp5587_write(client, ADP5587_REG_INT_STAT, (OVR_FLOW_INT | K_LCK_INT | GPI_INT | KE_INT));
	adp5587_write(client, ADP5587_REG_CFG, 0x19);

	return 0;
}

static int __devinit adp5587_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct adp5587_kpad *kpad;
	struct adp5587_kpad_platform_data *pdata = client->dev.platform_data;
	struct input_dev *input;
	unsigned int revid;
	int ret, i;
	int error;
	client_clone = client;
	printk("+-------------------------------------------+\n");
	printk("|         ADP5587 Keyboard Probe!!!         |\n");
	printk("+-------------------------------------------+\n");

	gpio_set_value(GPIO_RST, GPIO_LEVEL_LOW);
	udelay(100);
	gpio_set_value(GPIO_RST, GPIO_LEVEL_HIGH);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_err(&client->dev, "SMBUS Byte Data not Supported\n");
		return -EIO;
	}

	if (!pdata) {
		dev_err(&client->dev, "no platform data?\n");
		return -EINVAL;
	}

	if (!pdata->rows || !pdata->cols || !pdata->keymap) {
		dev_err(&client->dev, "no rows, cols or keymap from pdata\n");
		return -EINVAL;
	}

	if (pdata->keymapsize != ADP5587_KEYMAPSIZE) {
		dev_err(&client->dev, "invalid keymapsize\n");
		return -EINVAL;
	}

	kpad = kzalloc(sizeof(*kpad), GFP_KERNEL);
	input = input_allocate_device();

	if (!kpad || !input) {
		dev_err(&client->dev, "invalid kpad or input!\n");
		error = -ENOMEM;
		goto err_free_mem;
	}

	kpad->client = client;
	kpad->input = input;
	
	kpad->client->irq = IRQ_ADP5587_KEY_INT;

	#if defined(CONFIG_MACH_AEGIS) || defined(CONFIG_MACH_VIPER) || defined(CONFIG_MACH_CHIEF)
	keyled_regulator = regulator_get(NULL,"keyled");
        if (!keyled_regulator) {
                dev_err(&client->dev, " failed to get regulator\n");
        }
	#endif
	INIT_DELAYED_WORK(&kpad->work, adp5587_work);
	INIT_DELAYED_WORK(&kpad->hall_ic_work, hall_ic_handler);
	
	hall_ic_workqueue = create_singlethread_workqueue("hall_ic");
	if(hall_ic_workqueue == NULL) {
		printk("[Adp5587-keys:ERROR]:Can't not create workqueue for hall_ic\n");
	}
	
	/* Initializing delayed work for Back light off */
	ret = adp5587_read(client, ADP5587_REG_DEV_ID);
	if (ret < 0) {
		error = ret;
		dev_err(&client->dev, "invalid return of adp5587_read()\n");
		goto err_free_mem;
	}
	printk("AMIT: Keypad Device ID = 0x%0x\n",ret);
	printk("VENU:Keypad Device Conf Register = 0x%02x\n",adp5587_read(client, 0x01));
	printk("VENU:Keypad Device Conf Register = 0x%02x\n",adp5587_read(client, 0x02));
	revid = (u8) ret & ADP5587_DEVICE_ID_MASK;

	if (WA_DELAYED_READOUT_REVID(revid))
		kpad->delay = msecs_to_jiffies(30);
	
	input->name = "sec_keypad";
	input->phys = "adp5587-keys/input0";
	input->dev.parent = &client->dev;

	input_set_drvdata(input, kpad);

	input->id.bustype = BUS_I2C;
	input->id.vendor = 0x0001;
	input->id.product = 0x0001;
	input->id.version = revid;

	input->keycodesize = sizeof(kpad->keycode[0]);
	input->keycodemax = pdata->keymapsize;
	input->keycode = kpad->keycode;

	memcpy(kpad->keycode, pdata->keymap, pdata->keymapsize * input->keycodesize);

	/* setup input device */
	__set_bit(EV_KEY, input->evbit);

	if (pdata->repeat)
	__set_bit(EV_REP, input->evbit);

	//dev_err(&client->dev, "keycodemax %d, keymapsize %d, keycodesize %d\n", input->keycodemax, pdata->keymapsize, input->keycodesize);

	for (i = 0; i < input->keycodemax; i++) {
		//dev_err(&client->dev, "kpad->keycode[%d]=%d\n", i, kpad->keycode[i]);
		__set_bit(kpad->keycode[i] & KEY_MAX, input->keybit);
	}

#if defined(CONFIG_MACH_AEGIS) || defined(CONFIG_MACH_VIPER) || defined(CONFIG_MACH_CHIEF)
	HALL_GPIO = pdata->lid_gpio;
	HALL_IRQ = pdata->lid_irq;

	error = request_irq(HALL_IRQ, slide_irq,
		(IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING|IRQF_DISABLED),
		"slide_irq", kpad);
	if (error) {
		dev_err(&client->dev, "lid irq failed...\n");
		goto err_free_mem;
	}
#endif

	input_set_capability(input, EV_SW, SW_LID);

	if(gpio_get_value(HALL_GPIO)){
		input->sw[SW_LID] = 1;
		slide_on_off=1;
	}
	else{
		input->sw[SW_LID] = 0;
		slide_on_off=0;
	}

	__clear_bit(KEY_RESERVED, input->keybit);

	error = input_register_device(input);
	if (error) {
		dev_err(&client->dev, "unable to register input device\n");
		goto err_free_mem;
	}

	error = request_irq(kpad->client->irq, adp5587_irq,
		IRQF_TRIGGER_FALLING | IRQF_DISABLED,
		client->dev.driver->name, kpad);
	if (error) {
		dev_err(&client->dev, "irq %d busy?\n", client->irq);
		goto err_unreg_dev;
	}
	error = adp5587_setup(client);
	if (error){
		dev_err(&client->dev, "invalid return of adp5587_setup()\n");
		goto err_free_irq;
	}

	set_irq_wake(HALL_IRQ,1);

	device_init_wakeup(&client->dev, 1);
	i2c_set_clientdata(client, kpad);

	early_suspend.suspend = keypad_early_suspend;
	early_suspend.resume = keypad_late_resume;
	register_early_suspend(&early_suspend);

	dev_info(&client->dev, "Rev.%d keypad, irq %d, keypad_backlight ON\n", revid, client->irq);

	return 0;
	
	err_free_irq:
	free_irq(client->irq, kpad);
	err_unreg_dev:
	input_unregister_device(input);
	input = NULL;
	err_free_mem:
	input_free_device(input);
	kfree(kpad);

	return error;
}
static int __devexit adp5587_remove(struct i2c_client *client)
{
	struct adp5587_kpad *kpad = i2c_get_clientdata(client);

	adp5587_write(client, ADP5587_REG_CFG, 0);
	free_irq(client->irq, kpad);
	cancel_delayed_work_sync(&kpad->work);
	flush_workqueue(&hall_ic_workqueue);
	input_unregister_device(kpad->input);
	i2c_set_clientdata(client, NULL);
	kfree(kpad);

	return 0;
}
#ifdef CONFIG_PM
static int adp5587_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct adp5587_kpad *kpad = i2c_get_clientdata(client);
	static unsigned Value = 0;

	dev_err(&client->dev, "adp5587_suspend(), keypad_backlight OFF\n");

	disable_irq(client->irq);
	if(hall_ic_workqueue_statue == 1) {
		printk("%s : delayed workqueue cancel reqeust\n",__func__);
		cancel_delayed_work(&kpad->hall_ic_work);
		hall_ic_workqueue_statue = 0;
	}
	cancel_delayed_work_sync(&kpad->work);

	if (device_may_wakeup(&client->dev))
		enable_irq_wake(client->irq);

	return 0;
}

static int adp5587_resume(struct i2c_client *client)
{
	static unsigned Value = 1;

	dev_err(&client->dev, "adp5587_resume(), keypad_backlight ON\n");

	if (device_may_wakeup(&client->dev))
		disable_irq_wake(client->irq);

	enable_irq(client->irq);
	return 0;
}
#endif

static const struct i2c_device_id adp5587_id[] = {
	{ "adp5587", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, adp5587_id);

static struct i2c_driver adp5587_driver = {
	.driver = {
		.name = "adp5587",
	},
	.probe    = adp5587_probe,
	.remove   = __devexit_p(adp5587_remove),
	.id_table = adp5587_id,
#ifdef CONFIG_PM
	.suspend = adp5587_suspend,
	.resume  = adp5587_resume,
#endif
};

void keypad_early_suspend(struct early_suspend *h)
{
}

void keypad_late_resume(struct early_suspend *h)
{
	int slide_state = gpio_get_value(HALL_GPIO);

	backlight_flag = 1;
}
static int __init adp5587_init(void)
{
	key_class = class_create(THIS_MODULE, "adp5587-key");
	
	if (!key_class)
		printk("Failed to create class(adp5587-key)!\n");

	key_dev = device_create(key_class, NULL, 0, NULL, "adp5587-key");
	if (!key_dev)
		printk("Failed to create device(adp5587-key)!\n");

	if (device_create_file(key_dev, &dev_attr_key) < 0)
		printk("Failed to create device file(%s)!\n", dev_attr_key.attr.name);

	if (device_create_file(key_dev, &dev_attr_led) < 0)
		printk("Failed to create device file(%s)!\n", dev_attr_led.attr.name);
	
	if (device_create_file(key_dev, &dev_attr_slide_on_off) < 0)
		printk("Failed to create device file(%s)!\n", dev_attr_slide_on_off.attr.name);	
	
	return i2c_add_driver(&adp5587_driver);
}

module_init(adp5587_init);

static void __exit adp5587_exit(void)
{
	/* removing device attributes entries */
	// device_remove_file(&kpd_dev, &bl_keypad_attributes[0]); //uday
	/* removing device backlight under keypad backlight class */
	//  device_destroy(bl_class,0);
	/* removing class keypad backlight from sysfs entries */
	// class_destroy(bl_class);

	i2c_del_driver(&adp5587_driver);
}
module_exit(adp5587_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("ADP5588/87 Keypad driver");
MODULE_ALIAS("platform:adp5587-keys");                                                                                                                                              

