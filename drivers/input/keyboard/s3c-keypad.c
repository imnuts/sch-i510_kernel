/* drivers/input/keyboard/s3c-keypad.c
 *
 * Driver core for Samsung SoC onboard UARTs.
 *
 * Kim Kyoungil, Copyright (c) 2006-2009 Samsung Electronics
 *      http://www.samsungsemi.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/clk.h>
#include <linux/irq.h>
#include <linux/slab.h>

#include <linux/io.h>
#include <mach/hardware.h>
#include <asm/delay.h>
#include <asm/irq.h>

#include <mach/regs-gpio.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <plat/regs-keypad.h>
#ifdef CONFIG_CPU_FREQ
#include <mach/cpu-freq-v210.h>
#endif

#ifdef CONFIG_MACH_CHIEF
#include <mach/gpio-bank.h>
#endif

 
#include "s3c-keypad.h"

#define USE_PERF_LEVEL_KEYPAD 1 
#undef S3C_KEYPAD_DEBUG 
//#define S3C_KEYPAD_DEBUG 

#ifdef S3C_KEYPAD_DEBUG
#define DPRINTK(x...) printk("S3C-Keypad " x)
#define INPUT_REPORT_KEY(a,b,c) do {				\
		printk(KERN_ERR "%s:%d input_report_key(%x, %d, %d)\n", \
		       __func__, __LINE__, a, b, c);			\
		input_report_key(a,b,c);				\
	} while (0)
#else
#define DPRINTK(x...)		/* !!!! */
#define INPUT_REPORT_KEY	input_report_key
#endif

#define DEVICE_NAME "s3c-keypad"

#define TRUE 1
#define FALSE 0
#define	SUBJECT	"s3c_keypad.c"
#define P(format,...)\
    printk ("[ "SUBJECT " (%s,%d) ] " format "\n", __func__, __LINE__, ## __VA_ARGS__);
#define FI \
    printk ("[ "SUBJECT " (%s,%d) ] " "%s - IN" "\n", __func__, __LINE__, __func__);
#define FO \
    printk ("[ "SUBJECT " (%s,%d) ] " "%s - OUT" "\n", __func__, __LINE__, __func__);

static struct clk *keypad_clock;

static u32 keymask[KEYPAD_COLUMNS];
static u32 prevmask[KEYPAD_COLUMNS];

static int in_sleep = 0;

#ifdef CONFIG_MACH_CHIEF
static struct timer_list keypad_timer;
static int is_timer_on = FALSE;

/* keypad_led */
static struct timer_list keyled_timer;

// active keyled  
int keyLedOn = 0;
int keyLedSigOn = 0;
static void keypad_led_onoff(int OnOff);
void keypad_led_onoff_Ext(int OnOff){ keypad_led_onoff(OnOff); }

/* keypad_led */
int keyled_lcdOn = 0;
static void keyled_timer_start();
static void keyled_timer_stop();

void keyled_lcd_on_Ext(){ keyled_lcdOn = 1;  if(keypad_clock) keyled_timer_start(); }
void keyled_lcd_off_Ext(){ keyled_lcdOn = 0; if(keypad_clock) keyled_timer_stop(); }
EXPORT_SYMBOL(keyled_lcd_on_Ext);
EXPORT_SYMBOL(keyled_lcd_off_Ext);
#endif





#define IRQ_VOL_UP_INT (IRQ_EINT_GROUP1_BASE + 7)
extern unsigned int HWREV;

struct input_dev *g_dev;

#if defined (CONFIG_MACH_S5PC110_P1P2)
static int keypad_scan(void)
{

	u32 col,rval,gpio;

	DPRINTK("H3C %x J2C %x, J3c %x J4c%x \n",readl(S5PV210_GPH3CON),readl(S5PV210_GPJ2CON),
		readl(S5PV210_GPJ3CON), readl(S5PV210_GPJ4CON));
	DPRINTK("keypad_scan() is called\n");

	DPRINTK("row val = %x",readl(key_base + S3C_KEYIFROW));

	for (gpio = S5PV210_GPJ2(7); gpio <= S5PV210_GPJ4(4); gpio++)
		s3c_gpio_setpin(gpio, 1);
		
	for (col=0,gpio = S5PV210_GPJ2(7); col < KEYPAD_COLUMNS; col++,gpio++) {

		if(s3c_gpio_setpin(gpio, 0) < 0)
			s3c_gpio_setpin(++gpio, 0);

		//udelay(KEYPAD_DELAY);
		udelay(100);

		//rval = ~(readl(key_base+S3C_KEYIFROW)) & ((1<<KEYPAD_ROWS)-1) ;
		rval = ~(readl(S5PV210_GPH3DAT)) & ((1<<KEYPAD_ROWS)-1) ;
		

		keymask[col] = rval; 

		s3c_gpio_setpin(gpio,1);
	}

	for (gpio = S5PV210_GPJ2(7); gpio <= S5PV210_GPJ4(4); gpio++)
		s3c_gpio_setpin(gpio, 0);

	return 0;
}
#else

#ifdef CONFIG_MACH_CHIEF  
static void keypad_scan_activate_column(int active_col)
{
    u32     cval ;

    switch (active_col)
    {
        case -1 :   /* Clear All */
            s3c_gpio_cfgpin(GPIO_KEYSCAN6, S3C_GPIO_OUTPUT) ;
            s3c_gpio_cfgpin(GPIO_KEYSCAN7, S3C_GPIO_OUTPUT) ;
            gpio_set_value(GPIO_KEYSCAN6, GPIO_LEVEL_LOW) ;
            gpio_set_value(GPIO_KEYSCAN7, GPIO_LEVEL_LOW) ;
            writel(KEYIFCOL_CLEAR, key_base+S3C_KEYIFCOL);
            break ;

        case 6 :
            s3c_gpio_cfgpin(GPIO_KEYSCAN6, S3C_GPIO_OUTPUT) ;
            gpio_set_value(GPIO_KEYSCAN6, GPIO_LEVEL_LOW) ;
            s3c_gpio_cfgpin(GPIO_KEYSCAN7, S3C_GPIO_INPUT) ;    /* HI-Z */
            cval = KEYCOL_DMASK & 0xFFFF ; /* All port Hi-Z */
            writel(cval, key_base+S3C_KEYIFCOL) ;
            break ;

        case 7 :
            s3c_gpio_cfgpin(GPIO_KEYSCAN7, S3C_GPIO_OUTPUT) ;
            gpio_set_value(GPIO_KEYSCAN7, GPIO_LEVEL_LOW) ;
            s3c_gpio_cfgpin(GPIO_KEYSCAN6, S3C_GPIO_INPUT) ;    /* HI-Z */
            cval = KEYCOL_DMASK & 0xFFFF ; /* All port Hi-Z */
            writel(cval, key_base+S3C_KEYIFCOL) ;
             break ;

        default :
            s3c_gpio_cfgpin(GPIO_KEYSCAN6, S3C_GPIO_INPUT) ;    /* HI-Z */
            s3c_gpio_cfgpin(GPIO_KEYSCAN7, S3C_GPIO_INPUT) ;    /* HI-Z */
            cval = KEYCOL_DMASK & ~(((1<<8) | 1) << active_col) ;
            writel(cval, key_base+S3C_KEYIFCOL) ;
            break ;
    }
}
#endif
static int keypad_scan(void)
{

	u32 col,cval,rval;

	DPRINTK("H3C %x H2C %x \n",readl(S5PV210_GPH3CON),readl(S5PV210_GPH2CON));
	DPRINTK("keypad_scan() is called\n");

	DPRINTK("row val = %x",readl(key_base + S3C_KEYIFROW));

	for (col=0; col < KEYPAD_COLUMNS; col++) {

#ifdef CONFIG_MACH_CHIEF
        keypad_scan_activate_column(col) ;
#else		
		cval = KEYCOL_DMASK & ~((1 << col) | (1 << (col+8))); // clear that column number and 

		writel(cval, key_base+S3C_KEYIFCOL);             // make that Normal output.
								 // others shuld be High-Z output.
								 
		
#endif
		udelay(KEYPAD_DELAY);
		//rval = ~(readl(key_base+S3C_KEYIFROW)) & ((1<<KEYPAD_ROWS)-1) ;
#ifdef CONFIG_MACH_CHIEF    
	    rval = ~(readl(key_base+S3C_KEYIFROW)) & ((1<<KEYPAD_ROWS)-1) ;
#else	
		rval = ~(readl(key_base+S3C_KEYIFROW)) & (0xE /* KBR(0) is not used*/);
#endif
		 
		keymask[col] = rval; 
	}

#ifdef CONFIG_MACH_CHIEF   
    keypad_scan_activate_column(-1) ;   /* All Column Low */
#else	
	writel(KEYIFCOL_CLEAR, key_base+S3C_KEYIFCOL);
#endif

	return 0;
}

#endif

#ifdef CONFIG_MACH_CHIEF
/* keypad_led */
static void keyled_timer_start()
{
   extern unsigned int charging_mode_get(void);
   
   if (charging_mode_get() || (keyled_lcdOn == 0))  // Power-off charging
      return;

    keyled_timer.expires = jiffies + (3*HZ);    // 3sec
    
    if (keyLedOn) {
       mod_timer(&keyled_timer,keyled_timer.expires);
    } else {
       add_timer(&keyled_timer);
       keypad_led_onoff(1); // keyLed_3sec
    }
}

static void keyled_timer_stop()
{
    if(keyLedOn)
    {
       keypad_led_onoff(0); // keyLed_3sec
       del_timer_sync(&keyled_timer);
    }
}

static void keyled_timer_handler(unsigned long data)
{
    keypad_led_onoff(0); // keyLed_3sec
}
#endif

static void s3c_keypad_timer_handler(unsigned long data)
{
	u32 press_mask;
	u32 release_mask;
	u32 restart_timer = 0;
	int i,col;
	struct s3c_keypad *pdata = (struct s3c_keypad *)data;
	struct input_dev *dev = pdata->dev;
	struct timer_list * timer = &pdata->keypad_timer;

	keypad_scan();


	for(col=0; col < KEYPAD_COLUMNS; col++) {
		press_mask = ((keymask[col] ^ prevmask[col]) & keymask[col]); 
		release_mask = ((keymask[col] ^ prevmask[col]) & prevmask[col]); 

#ifdef CONFIG_CPU_FREQ
#if USE_PERF_LEVEL_KEYPAD
//		if (press_mask || release_mask)
//			set_dvfs_target_level(LEV_400MHZ);
#endif
#endif
		i = col * KEYPAD_ROWS;

		while (press_mask) {
			if (press_mask & 1) {
				input_report_key(dev,pdata->keycodes[i],1);
#ifdef CONFIG_MACH_CHIEF				
                keyled_timer_start(); /* keypad_led */
#endif				
/* #ifdef CONFIG_KERNEL_DEBUG_SEC */
#ifdef DEBUG_LOG_ENABLE
				printk(KERN_DEBUG"[KEYPAD] key Pressed  : key %d map %d\n",i, pdata->keycodes[i]);
#else
				printk(KERN_DEBUG"[KEYPAD] key Pressed\n");
#endif
						}
			press_mask >>= 1;
			i++;
		}

		i = col * KEYPAD_ROWS;

		while (release_mask) {
			if (release_mask & 1) {
				input_report_key(dev,pdata->keycodes[i],0);
#ifdef CONFIG_MACH_CHIEF				
                keyled_timer_start(); /* keypad_led */
#endif				
/* #ifdef CONFIG_KERNEL_DEBUG_SEC */
#ifdef DEBUG_LOG_ENABLE
				printk(KERN_DEBUG"[KEYPAD] key Released : %d  map %d\n",i,pdata->keycodes[i]);
#else
				printk(KERN_DEBUG"[KEYPAD] key Released\n");
#endif

            }
			release_mask >>= 1;
			i++;
		}
		prevmask[col] = keymask[col];

		restart_timer |= keymask[col];
	}


	if (restart_timer) {
		mod_timer(timer, timer->expires);
	} else {
		writel(KEYIFCON_INIT, key_base+S3C_KEYIFCON);
		pdata->keypad_timer_on = FALSE;
	}

}

static void s3c_keypad_starttimer(struct s3c_keypad * s3c_keypad)
{
	struct timer_list * timer = &s3c_keypad->keypad_timer;

	if( s3c_keypad->keypad_timer_on ) {
		mod_timer(timer, timer->expires);
	}
	else {
		init_timer(timer);
#ifdef CONFIG_MACH_CHIEF
		timer->expires		= jiffies + (3*HZ/100);	
#else	
		timer->expires		= jiffies + (HZ/100);
#endif		
		timer->data			= (unsigned long)s3c_keypad;
		timer->function		= s3c_keypad_timer_handler;
		add_timer(timer);

		s3c_keypad->keypad_timer_on = TRUE;
	}
}

#ifdef CONFIG_MACH_CHIEF
static irqreturn_t s3c_keygpio_isr(int irq, void *dev_id)
{
	unsigned int key_status;
	static unsigned int prev_key_status = (1 << 6);
	struct s3c_keypad *pdata = (struct s3c_keypad *)dev_id;
	struct input_dev *dev = pdata->dev;
	#define KEY_POWER_EVT       116

	// Beware that we may not obtain exact key up/down status at
	// this point.
	key_status = (readl(S5PV210_GPH2DAT)) & (1 << 6);

	// If ISR is called and up/down status remains the same, we
	// must have lost one and should report that first with
	// upside/down.
	if(in_sleep)
	{
	    if (key_status == prev_key_status)
		{
    		INPUT_REPORT_KEY(dev, KEY_POWER_EVT, key_status ? 1 : 0);
		}
		in_sleep = 0;
	}

	INPUT_REPORT_KEY(dev, KEY_POWER_EVT, key_status ? 0 : 1);
   
   	keyled_timer_start(); /* keypad_led */

	prev_key_status = key_status;
	printk(KERN_DEBUG "s3c_keygpio_isr pwr key_status =%d,\n", key_status);

	return IRQ_HANDLED;
}

static int s3c_keygpio_isr_setup(void *pdev)
{
    int ret;
    //PWR key
    s3c_gpio_setpull(S5PV210_GPH2(6), S3C_GPIO_PULL_NONE);
    set_irq_type(IRQ_EINT(22), IRQ_TYPE_EDGE_BOTH);

    // stk.lim: Add IRQF_DISABLED to eliminate any possible race
    // regarding key status
    ret = request_irq(IRQ_EINT(22), s3c_keygpio_isr, IRQF_DISABLED
                        | IRQF_SAMPLE_RANDOM, "key gpio", (void *)pdev);

    if (ret) {
        printk("request_irq failed (IRQ_KEYPAD (gpio)) !!!\n");
        ret = -EIO;
    }

	ret = enable_irq_wake(IRQ_EINT(22));
	if (ret) {
		printk(KERN_ERR"%s : Failed to enable power key irq wake\n", __func__);
	}

    return ret;
}

#endif


static irqreturn_t s3c_keypad_isr(int irq, void *dev_id)
{
	struct s3c_keypad * s3c_keypad = (struct s3c_keypad *)dev_id;

	/* disable keypad interrupt and schedule for keypad timer handler */
	writel(readl(key_base+S3C_KEYIFCON) & ~(INT_F_EN|INT_R_EN), key_base+S3C_KEYIFCON);

	s3c_keypad_starttimer(s3c_keypad);

	/*Clear the keypad interrupt status*/
	writel(KEYIFSTSCLR_CLEAR, key_base+S3C_KEYIFSTSCLR);
    
	DPRINTK("s3c_keypad_isr \n");
	
	return IRQ_HANDLED;
}
#if 0 /* gpio key use gpio-event.c */
static irqreturn_t s3c_keygpio_isr(int irq, void *dev_id)
{
	unsigned int key_status;
	unsigned int * prev_key_status;
	KEYGPIO_INT_INFO *  int_info = (KEYGPIO_INT_INFO *)dev_id;
	KEYGPIO_INFO *		info = int_info->info;
	struct input_dev *	dev = int_info->dev;

	prev_key_status = &info->prev_key_status;

	// Beware that we may not obtain exact key up/down status at
	// this point.

	key_status = (readl(info->gpio_data) & info->gpio_data_mask);

	// If ISR is called and up/down status remains the same, we
	// must have lost one and should report that first with
	// upside/down.
	if(in_sleep)
	{
		if (key_status == *prev_key_status)
		{
			INPUT_REPORT_KEY(dev, info->keycode, key_status ? 1 : 0);
		}
		in_sleep = 0;
	}

	INPUT_REPORT_KEY(dev, info->keycode, key_status ? 0 : 1);

	*prev_key_status = key_status;

	printk(KERN_DEBUG "s3c_keygpio_isr [%d] key_status =%d\n", int_info->index, key_status);

	return IRQ_HANDLED;
}

static void
s3c_keygpio_init(struct s3c_keypad * s3c_keypad)
{
	KEYGPIO_INFO * info_tbl = (KEYGPIO_INFO *)&s3c_keypad->keygpio_info_tbl[0];
	int i;

	/* set gpio, irq of gpio keys */
	info_tbl[KEYGPIO_VOLUME_UP].irq 		= IRQ_EINT(24);
	info_tbl[KEYGPIO_VOLUME_UP].gpio		= S5PV210_GPH3(0); 
	info_tbl[KEYGPIO_VOLUME_UP].gpio_data	= (S5PV210_GPH3_BASE + 0x04);
//	info_tbl[KEYGPIO_VOLUME_UP].gpio_data	= S5PV210_GPH3DAT;
	info_tbl[KEYGPIO_VOLUME_UP].gpio_data_mask	= (1 << 0);
	info_tbl[KEYGPIO_VOLUME_UP].keycode = 115;

	info_tbl[KEYGPIO_POWER].irq 		= IRQ_EINT(22);
	info_tbl[KEYGPIO_POWER].gpio 		= S5PV210_GPH2(6);
	info_tbl[KEYGPIO_POWER].gpio_data	= (S5PV210_GPH2_BASE + 0x04);
//	info_tbl[KEYGPIO_POWER].gpio_data	= S5PV210_GPH2DAT;
	info_tbl[KEYGPIO_POWER].gpio_data_mask = (1 << 6);
	info_tbl[KEYGPIO_POWER].keycode		= 116;

	for(i=0; i < KEYGPIO_MAX; i++) {
		KEYGPIO_INFO * info = &info_tbl[i];
		int gpio = info->gpio;

		info->prev_key_status = info->gpio_data_mask;

		s3c_gpio_cfgpin(gpio, S3C_GPIO_INPUT);
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_UP);
	}
}

static int 
s3c_keygpio_isr_register(struct s3c_keypad * s3c_keypad)
{
	int ret;
	int i, max_keynum;
	KEYGPIO_INFO * info_tbl = (KEYGPIO_INFO *)&s3c_keypad->keygpio_info_tbl[0];
	
	max_keynum = HWREV == 8 ? KEYGPIO_POWER : KEYGPIO_MAX;
	for(i=0; i < max_keynum; i++) {
		int gpio = info_tbl[i].gpio; 
		int irq = info_tbl[i].irq;
		KEYGPIO_INT_INFO * int_info = &s3c_keypad->keygpio_int_info_tbl[i];

		/* setup keygpio_int_info */
		int_info->index	= i;
		int_info->info	= &info_tbl[i];
		int_info->dev	= s3c_keypad->dev;

		set_irq_type(irq, IRQ_TYPE_EDGE_BOTH);
		ret = request_irq(irq, s3c_keygpio_isr, IRQF_SAMPLE_RANDOM,
				        "keygpio", (void *)int_info);
	    if (ret) {
			printk("request_irq failed (IRQ_KEYPAD (key vol up)) !!!\n");
        	ret = -EIO;
			break;
 	   }
	}

	return ret;
}

static void
s3c_keygpio_isr_unregister(struct s3c_keypad * s3c_keypad)
{
	int ret;
	int i, max_keynum;
	KEYGPIO_INFO * info_tbl = (KEYGPIO_INFO *)&s3c_keypad->keygpio_info_tbl[0];
	
	max_keynum = HWREV == 8 ? KEYGPIO_POWER : KEYGPIO_MAX;
	for(i=0; i < max_keynum; i++) {
		int irq = info_tbl[i].irq;
		KEYGPIO_INT_INFO * int_info = &s3c_keypad->keygpio_int_info_tbl[i];

		free_irq(irq, (void *)int_info); 

	}
}
#endif

#ifdef CONFIG_MACH_CHIEF
static void keypad_led_onoff(int OnOff)
{

   printk(KERN_ERR "\nKeypad_LED_ONOFF(OnOff=%d)\n", OnOff);

   if(OnOff)
   {
        keyLedOn = 1; // active keyled
        /* HW Requeset Fixed keypad led on issue  when AP is sleep wakeup*/
        s3c_gpio_cfgpin(GPIO_MAIN_KEY_LED_NEW, S3C_GPIO_OUTPUT);
        gpio_set_value(GPIO_MAIN_KEY_LED_NEW, GPIO_LEVEL_HIGH);
        /* For backward compatible after HW 0.7 */
        // remove //s3c_gpio_cfgpin(GPIO_MAIN_KEY_LED_EN, S3C_GPIO_OUTPUT);
        // remove //gpio_set_value(GPIO_MAIN_KEY_LED_EN, GPIO_LEVEL_HIGH);
   }
   else
   {
        keyLedOn = 0; // active keyled
        /* HW Requeset Fixed keypad led on issue  when AP is sleep wakeup*/
        s3c_gpio_cfgpin(GPIO_MAIN_KEY_LED_NEW, S3C_GPIO_OUTPUT);
        gpio_set_value(GPIO_MAIN_KEY_LED_NEW, GPIO_LEVEL_LOW);
        /* For backward compatible after HW 0.7 */
        // remove // s3c_gpio_cfgpin(GPIO_MAIN_KEY_LED_EN, S3C_GPIO_OUTPUT);
        // remove // gpio_set_value(GPIO_MAIN_KEY_LED_EN, GPIO_LEVEL_LOW);
   }
}
#endif

static ssize_t keyshort_test(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct s3c_keypad * s3c_keypad = dev_get_drvdata(dev);
	KEYGPIO_INFO * info_tbl = (KEYGPIO_INFO *)&s3c_keypad->keygpio_info_tbl[0];

	int count;
	int mask=0;
    	u32 col=0;//,cval=0,rval=0;
	int i;
	int rval;
	u32 press_mask;

	/* check gpio keys */
	for(i=0; i < KEYGPIO_MAX; i++) {
		if( !gpio_get_value(info_tbl[i].gpio) )
			mask |= (1 << i);
	}

#ifndef CONFIG_MACH_CHIEF
	/* check key scan */
	keypad_scan();

	for(col=0; col < KEYPAD_COLUMNS; col++) {
		press_mask = keymask[col]; 

		printk("[%d] press_mask (0x%x) \n", col, press_mask);

		i = col * KEYPAD_ROWS;

		while (press_mask) {
			if (press_mask & 1) {
				mask |= (1 << (i + KEYGPIO_MAX));
			}
			press_mask >>= 1;
			i++;
		}
	}
#else	
    rval = ~readl(key_base+S3C_KEYIFROW) & 0x01/*0x007F*/ ;     /* SENSE[0-6] ---> SENSE[0] for chief*/
    writel(KEYIFCOL_CLEAR, key_base+S3C_KEYIFCOL) ;
    if (rval)
    {
        mask |= 0x100 ;
    }
#endif	
#ifdef CONFIG_MACH_STEALTHV
	if (gpio_get_value(S5PV210_GPH2(6)) == 0) /* power key */
		mask |= 1;

	if (gpio_get_value(S5PV210_GPH3(0)) == 0) /* volume up key */
		mask |= 1;
#endif
    if(mask)
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
static DEVICE_ATTR(key, 0664, keyshort_test, NULL);

extern void keypad_led_control(bool onOff);
static ssize_t keypad_brightness_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    int value;
  
	
    sscanf(buf, "%d", &value);

#ifndef CONFIG_MACH_CHIEF
    if (value)
        keypad_led_control(1);
    else
        keypad_led_control(0);

	printk(KERN_DEBUG "[%s] brightness : %d \n", __FUNCTION__, value);
#else
        if(value == 1)
        {
           keypad_led_onoff(1);
        }
        else if(value == 2)
        {
           keypad_led_onoff(0);
        }
#endif 

   return size;
}
static DEVICE_ATTR(brightness, 0664, NULL, keypad_brightness_store);

unsigned int keywakeup_is_enabled = 0;
static int keypad_keywakeup_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t len)
{
	int value;
	
	sscanf(buf, "%d", &value);

    if (value == 0)
        keywakeup_is_enabled = 0;
    else if (value == 1)
        keywakeup_is_enabled = 1;

    printk(KERN_DEBUG "[%s] keywakeup_is_enabled : %d\n", __FUNCTION__, keywakeup_is_enabled);

	return len;
}
static int keypad_keywakeup_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	printk(KERN_DEBUG "[%s] keywakeup_is_enabled : %d\n", __FUNCTION__, keywakeup_is_enabled);

	return sprintf(buf,"%u\n", keywakeup_is_enabled);
}
static DEVICE_ATTR(keywakeup, 0664, keypad_keywakeup_show, keypad_keywakeup_store);

static int __init s3c_keypad_probe(struct platform_device *pdev)
{
	struct resource *res, *keypad_mem, *keypad_irq;
	struct input_dev *input_dev;
	struct s3c_keypad *s3c_keypad;
	int ret, size;
	int key, code;
	struct class *key_class;
    	struct device *keypress;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev,"no memory resource specified\n");
		return -ENOENT;
	}

	size = (res->end - res->start) + 1;

	keypad_mem = request_mem_region(res->start, size, pdev->name);
	if (keypad_mem == NULL) {
		dev_err(&pdev->dev, "failed to get memory region\n");
		ret = -ENOENT;
		goto err_req;
	}

	key_base = ioremap(res->start, size);
	if (key_base == NULL) {
		printk(KERN_ERR "Failed to remap register block\n");
		ret = -ENOMEM;
		goto err_map;
	}

	keypad_clock = clk_get(&pdev->dev, "keypad");
	if (IS_ERR(keypad_clock)) {
		dev_err(&pdev->dev, "failed to find keypad clock source\n");
		ret = PTR_ERR(keypad_clock);
		goto err_clk;
	}

	clk_enable(keypad_clock);
	
	s3c_keypad = kzalloc(sizeof(struct s3c_keypad), GFP_KERNEL);
	input_dev = input_allocate_device();

	g_dev = input_dev;
	if (!s3c_keypad || !input_dev) {
		ret = -ENOMEM;
		goto err_alloc;
	}

	platform_set_drvdata(pdev, s3c_keypad);
	s3c_keypad->dev = input_dev;
	
	/* Set GPIO Port for keypad mode and pull-up disable*/
	s3c_setup_keypad_cfg_gpio(KEYPAD_ROWS, KEYPAD_COLUMNS);

	/* STEALTH-V don't use KBR(0)-GPH3- for keypad */
#ifndef CONFIG_MACH_CHIEF	
	s3c_gpio_cfgpin(S5PV210_GPH3(0), S3C_GPIO_INPUT);
	s3c_gpio_setpull(S5PV210_GPH3(0), S3C_GPIO_PULL_UP);
#endif

	writel(KEYIFCON_INIT, key_base+S3C_KEYIFCON);
	writel(KEYIFFC_DIV, key_base+S3C_KEYIFFC);
	writel(KEYIFCOL_CLEAR, key_base+S3C_KEYIFCOL);
	
#ifdef CONFIG_MACH_CHIEF
   /* GPIO_CONTROL */
    gpio_set_value(GPIO_KEYSCAN6, GPIO_LEVEL_LOW) ;
    gpio_set_value(GPIO_KEYSCAN7, GPIO_LEVEL_LOW) ;
#endif

	/* create and register the input driver */
	set_bit(EV_KEY, input_dev->evbit);
	/*Commenting the generation of repeat events*/
	//set_bit(EV_REP, input_dev->evbit);
	s3c_keypad->nr_rows = KEYPAD_ROWS;
	s3c_keypad->no_cols = KEYPAD_COLUMNS;
	s3c_keypad->total_keys = MAX_KEYPAD_NR;

	for(key = 0; key < s3c_keypad->total_keys; key++){
		code = s3c_keypad->keycodes[key] = keypad_keycode[key];
		if(code<=0)
			continue;
		set_bit(code & KEY_MAX, input_dev->keybit);
	}

	//printk("%s, keypad row number is %d, column is %d",__FUNCTION__, s3c_keypad->nr_rows, s3c_keypad->no_cols);

#ifndef CONFIG_MACH_CHIEF
      set_bit(26 & KEY_MAX, input_dev->keybit);
      set_bit(42 & KEY_MAX, input_dev->keybit);
#endif 
      
#ifdef 	CONFIG_MACH_CHIEF  
   input_set_capability(input_dev, EV_SW, SW_LID); // TODO:FORTE
#endif

	input_dev->name = DEVICE_NAME;
	input_dev->phys = "s3c-keypad/input0";
	
	input_dev->id.bustype = BUS_HOST;
	input_dev->id.vendor = 0x0001;
	input_dev->id.product = 0x0001;
	input_dev->id.version = 0x0001;

	input_dev->keycode = keypad_keycode;

	ret = input_register_device(input_dev);
	if (ret) {
		printk("Unable to register s3c-keypad input device!!!\n");
		goto err_regi;
	}
   /* keypad_led timer init */
#ifdef CONFIG_MACH_CHIEF   
   init_timer(&keyled_timer);
   keyled_timer.function = keyled_timer_handler;
   keyled_timer.data = (unsigned long)s3c_keypad;
   s3c_keygpio_isr_setup((void *)s3c_keypad);
#endif
	/* For IRQ_KEYPAD */
	keypad_irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (keypad_irq == NULL) {
		dev_err(&pdev->dev, "no irq resource specified\n");
		ret = -ENOENT;
		goto err_irq;
	}
	ret = request_irq(keypad_irq->start, s3c_keypad_isr, IRQF_SAMPLE_RANDOM,
		DEVICE_NAME, (void *) s3c_keypad);
	if (ret) {
		printk("request_irq failed (IRQ_KEYPAD) !!!\n");
		ret = -EIO;
		goto err_irq;
	}
#if 0 /* gpio key use gpio-event.c */
	s3c_keygpio_init((void *)s3c_keypad);
	s3c_keygpio_isr_register((void *)s3c_keypad);
#endif
	/* for 1-5 applicaion 
	   /sys/devices/virtual/key/key/key 
	 */
	key_class = class_create(THIS_MODULE,"key");
	if(IS_ERR(key_class)){
		printk("failed to keyclass\n");
        goto out;
	}

	keypress = device_create(key_class, NULL, 0 , s3c_keypad, "key");
	if(IS_ERR(keypress))	
		printk("failed to create a keypress device\n");
	else{
		if(device_create_file(keypress, &dev_attr_key)<0)
			printk("failed to create device file key\n");	
	}
	
    if (device_create_file(keypress, &dev_attr_brightness) < 0) {
        printk("fail to create device file brightness\n");
  	}
    
    if (device_create_file(keypress, &dev_attr_keywakeup) < 0) {
        printk("fail to create device file keywakeup\n");
  	}

    printk( DEVICE_NAME " Initialized\n");

	return 0;

out:
	free_irq(keypad_irq->start, input_dev);

err_irq:
err_regi:
	input_free_device(input_dev);
	kfree(s3c_keypad);
	
err_alloc:
	clk_disable(keypad_clock);
	clk_put(keypad_clock);

err_clk:
	iounmap(key_base);

err_map:
	release_resource(keypad_mem);
	kfree(keypad_mem);

err_req:
	return ret;
}

static int s3c_keypad_remove(struct platform_device *pdev)
{
	struct s3c_keypad * s3c_keypad = platform_get_drvdata(pdev);
	struct input_dev * input_dev = s3c_keypad->dev;
	
	writel(KEYIFCON_CLEAR, key_base+S3C_KEYIFCON);

	if(keypad_clock) {
		clk_disable(keypad_clock);
		clk_put(keypad_clock);
		keypad_clock = NULL;
	}

	if( s3c_keypad->keypad_timer_on ) {
		del_timer(&s3c_keypad->keypad_timer);
	}
#if 0 /* gpio key use gpio-event.c */	
	s3c_keygpio_isr_unregister(s3c_keypad);
#endif
	input_unregister_device(input_dev);
	iounmap(key_base);
	kfree(pdev->dev.platform_data);
	free_irq(IRQ_KEYPAD, (void *) pdev);

#ifdef CONFIG_MACH_CHIEF
    del_timer(&keyled_timer); /* keypad_led */
#endif	

	printk(DEVICE_NAME " Removed.\n");
	return 0;
}

#ifdef CONFIG_PM
#include <plat/pm.h>

static struct sleep_save s3c_keypad_save[] = {
	SAVE_ITEM(KEYPAD_ROW_GPIOCON),
	SAVE_ITEM(KEYPAD_COL_GPIOCON),
	SAVE_ITEM(KEYPAD_ROW_GPIOPUD),
	SAVE_ITEM(KEYPAD_COL_GPIOPUD),
};

static unsigned int keyifcon, keyiffc;
static int s3c_keypad_suspend(struct platform_device *dev, pm_message_t state)
{
	keyifcon = readl(key_base+S3C_KEYIFCON);
	keyiffc = readl(key_base+S3C_KEYIFFC);
#ifdef CONFIG_MACH_STEALTHV
	enable_irq_wake(IRQ_KEYPAD);
#else
	s3c_pm_do_save(s3c_keypad_save, ARRAY_SIZE(s3c_keypad_save));
	
	//writel(~(0xfffffff), KEYPAD_ROW_GPIOCON);
	//writel(~(0xfffffff), KEYPAD_COL_GPIOCON);

	disable_irq(IRQ_KEYPAD);

	clk_disable(keypad_clock);

	in_sleep = 1;
#endif
	return 0;
}


static int s3c_keypad_resume(struct platform_device *dev)
{
	//struct s3c_keypad          *s3c_keypad = (struct s3c_keypad *) platform_get_drvdata(dev);
    //struct input_dev           *iDev = s3c_keypad->dev;
	//unsigned int key_temp_data=0;
#ifdef CONFIG_MACH_STEALTHV	
	disable_irq_wake(IRQ_KEYPAD);
#else
	printk(KERN_DEBUG "++++ %s\n", __FUNCTION__ );

	clk_enable(keypad_clock);
#endif
	writel(KEYIFCON_INIT, key_base+S3C_KEYIFCON);
	writel(keyiffc, key_base+S3C_KEYIFFC);
	writel(KEYIFCOL_CLEAR, key_base+S3C_KEYIFCOL);

#if 0
	key_temp_data = readl(key_base+S3C_KEYIFROW) & 0x01;
	if (!key_temp_data){
		input_report_key(iDev, 50, 1);
		printk("key data is %d \n", key_temp_data);		
		input_report_key(iDev, 50, 0);
		}
	else {
		/*send some event to android to start the full resume*/
		input_report_key(iDev, KEYCODE_UNKNOWN, 1);//ENDCALL up event
		udelay(5);
		input_report_key(iDev, KEYCODE_UNKNOWN, 0);//ENDCALL down event
		}

	//printk("H3C %x H2C %x \n",readl(S5PC11X_GPH3CON),readl(S5PC11X_GPH2CON));
#endif
#ifndef CONFIG_MACH_STEALTHV
	s3c_pm_do_restore(s3c_keypad_save, ARRAY_SIZE(s3c_keypad_save));

	enable_irq(IRQ_KEYPAD);
	printk(KERN_DEBUG "---- %s\n", __FUNCTION__ );
#endif
	return 0;
}
#else
#define s3c_keypad_suspend NULL
#define s3c_keypad_resume  NULL
#endif /* CONFIG_PM */

static struct platform_driver s3c_keypad_driver = {
	.probe		= s3c_keypad_probe,
	.remove		= s3c_keypad_remove,
	.suspend	= s3c_keypad_suspend,
	.resume		= s3c_keypad_resume,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "s3c-keypad",
	},
};

static int __init s3c_keypad_init(void)
{
	int ret;

	ret = platform_driver_register(&s3c_keypad_driver);
	
	if(!ret)
	   printk(KERN_INFO "S3C Keypad Driver\n");

	return ret;
}

static void __exit s3c_keypad_exit(void)
{
	platform_driver_unregister(&s3c_keypad_driver);
}

module_init(s3c_keypad_init);
module_exit(s3c_keypad_exit);

MODULE_AUTHOR("Samsung");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("KeyPad interface for Samsung S3C");
