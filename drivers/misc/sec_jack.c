/*  drivers/misc/sec_jack.c
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
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/switch.h>
#include <linux/input.h>
#include <linux/timer.h>
#include <linux/wakelock.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/gpio_event.h>
#include <linux/sec_jack.h>
#if defined(CONFIG_MACH_CHIEF)
#include <linux/regulator/consumer.h>
#endif


#define MAX_ZONE_LIMIT		10
/* keep this value if you support double-pressed concept */
#define SEND_KEY_CHECK_TIME_MS	30		/* 30ms */
#ifdef CONFIG_MACH_AEGIS
#define DET_CHECK_TIME_MS	200		/* 200ms */
#else
#define DET_CHECK_TIME_MS	400		/* 400ms */
#endif

#define WAKE_LOCK_TIME		(HZ * 5)	/* 5 sec */

#ifdef CONFIG_MACH_CHIEF
#define CONFIG_DEBUG_SEC_JACK 1
#ifdef CONFIG_DEBUG_SEC_JACK
#define SEC_JACKDEV_DBG(...)\
        printk (__VA_ARGS__);
#else
#define SEC_JACKDEV_DBG(...)
#endif
#define KEYCODE_SENDEND 226     // TODO:CHIEF 248 --> CHANGE sec_jack.kl

#define DETECTION_CHECK_COUNT   2
#define DETECTION_CHECK_TIME    get_jiffies_64() + (HZ/10)// 1000ms / 10 = 100ms
#define SEND_END_ENABLE_TIME    get_jiffies_64() + (HZ*2)// 1000ms * 1 = 1sec


#define SEND_END_CHECK_COUNT    3
#define SEND_END_CHECK_TIME     get_jiffies_64() + (HZ/50) //2000ms
#endif

#define NUM_INPUT_DEVICE_ID	2

static struct class *jack_class;
static struct device *jack_dev;


struct sec_jack_info {
	struct sec_jack_platform_data *pdata;
	#ifndef CONFIG_MACH_CHIEF
	struct delayed_work jack_detect_work;
	#endif
	#ifdef CONFIG_MACH_CHIEF
	struct input_dev *input;
	#endif
	struct work_struct buttons_work;
	struct workqueue_struct *queue;
	struct input_dev *input_dev;
	struct wake_lock det_wake_lock;
	struct sec_jack_zone *zone;
	struct input_handler handler;
	struct input_handle handle;
	struct input_device_id ids[NUM_INPUT_DEVICE_ID];
	int det_irq;
	int dev_id;
	int pressed;
	int pressed_code;
	struct platform_device *send_key_dev;
	unsigned int cur_jack_type;
};

/* with some modifications like moving all the gpio structs inside
 * the platform data and getting the name for the switch and
 * gpio_event from the platform data, the driver could support more than
 * one headset jack, but currently user space is looking only for
 * one key file and switch for a headset so it'd be overkill and
 * untestable so we limit to one instantiation for now.
 */
static atomic_t instantiated = ATOMIC_INIT(0);

/* sysfs name HeadsetObserver.java looks for to track headset state
 */
struct switch_dev switch_jack_detection = {
	.name = "h2w",
};

/* To support AT+FCESTEST=1 */
struct switch_dev switch_sendend = {
		.name = "send_end",
};
#if defined (CONFIG_MACH_CHIEF)
static unsigned int sendend_type; //0=short, 1=open
static unsigned int send_end_key_timer_token;
static unsigned int current_jack_type_status;
static unsigned int jack_detect_timer_token;
static unsigned int send_end_irq_token;

static struct timer_list jack_detect_timer;
static struct timer_list send_end_key_event_timer;
static struct wake_lock jack_sendend_wake_lock;

extern struct regulator *earmic_regulator ; /* LDO6 */
struct sec_jack_info *jack_info;
#endif

static struct gpio_event_direct_entry sec_jack_key_map[] = {
	{
		.code	= KEY_UNKNOWN,
	},
};

static struct gpio_event_input_info sec_jack_key_info = {
	.info.func = gpio_event_input_func,
	.info.no_suspend = true,
	.type = EV_KEY,
	.debounce_time.tv.nsec = SEND_KEY_CHECK_TIME_MS * NSEC_PER_MSEC,
	.keymap = sec_jack_key_map,
	.keymap_size = ARRAY_SIZE(sec_jack_key_map)
};

static struct gpio_event_info *sec_jack_input_info[] = {
	&sec_jack_key_info.info,
};

static struct gpio_event_platform_data sec_jack_input_data = {
	.name = "sec_jack",
	.info = sec_jack_input_info,
	.info_count = ARRAY_SIZE(sec_jack_input_info),
};

/* gpio_input driver does not support to read adc value.
 * We use input filter to support 3-buttons of headset
 * without changing gpio_input driver.
 */
static bool sec_jack_buttons_filter(struct input_handle *handle,
				    unsigned int type, unsigned int code,
				    int value)
{
	struct sec_jack_info *hi = handle->handler->private;

	if (type != EV_KEY || code != KEY_UNKNOWN)
		return false;

	hi->pressed = value;

	/* This is called in timer handler of gpio_input driver.
	 * We use workqueue to read adc value.
	 */
	queue_work(hi->queue, &hi->buttons_work);

	return true;
}

static int sec_jack_buttons_connect(struct input_handler *handler,
				    struct input_dev *dev,
				    const struct input_device_id *id)
{
	struct sec_jack_info *hi;
	struct sec_jack_platform_data *pdata;
	struct sec_jack_buttons_zone *btn_zones;
	int err;
	int i;

	/* bind input_handler to input device related to only sec_jack */
	if (dev->name != sec_jack_input_data.name)
		return -ENODEV;

	hi = handler->private;
	pdata = hi->pdata;
	btn_zones = pdata->buttons_zones;

	hi->input_dev = dev;
	hi->handle.dev = dev;
	hi->handle.handler = handler;
	hi->handle.open = 0;
	hi->handle.name = "sec_jack_buttons";

	err = input_register_handle(&hi->handle);
	if (err) {
		pr_err("%s: Failed to register sec_jack buttons handle, "
			"error %d\n", __func__, err);
		goto err_register_handle;
	}

	err = input_open_device(&hi->handle);
	if (err) {
		pr_err("%s: Failed to open input device, error %d\n",
			__func__, err);
		goto err_open_device;
	}

	for (i = 0; i < pdata->num_buttons_zones; i++)
		input_set_capability(dev, EV_KEY, btn_zones[i].code);

	return 0;

 err_open_device:
	input_unregister_handle(&hi->handle);
 err_register_handle:

	return err;
}

static void sec_jack_buttons_disconnect(struct input_handle *handle)
{
	input_close_device(handle);
	input_unregister_handle(handle);
}

static void sec_jack_set_type(struct sec_jack_info *hi, int jack_type)
{
	struct sec_jack_platform_data *pdata = hi->pdata;

	/* this can happen during slow inserts where we think we identified
	 * the type but then we get another interrupt and do it again
	 */
	if (jack_type == hi->cur_jack_type) {
		if (jack_type != SEC_HEADSET_4POLE)
			pdata->set_micbias_state(false);
		return;
	}

	if (jack_type == SEC_HEADSET_4POLE) {
		/* for a 4 pole headset, enable detection of send/end key */
		if (hi->send_key_dev == NULL)
			/* enable to get events again */
			hi->send_key_dev = platform_device_register_data(NULL,
					GPIO_EVENT_DEV_NAME,
					hi->dev_id,
					&sec_jack_input_data,
					sizeof(sec_jack_input_data));
	} else {
		/* for all other jacks, disable send/end key detection */
		if (hi->send_key_dev != NULL) {
			/* disable to prevent false events on next insert */
			platform_device_unregister(hi->send_key_dev);
			hi->send_key_dev = NULL;
		}
		/* micbias is left enabled for 4pole and disabled otherwise */
		pdata->set_micbias_state(false);
	}

	hi->cur_jack_type = jack_type;
	pr_info("%s : jack_type = %d\n", __func__, jack_type);

	switch_set_state(&switch_jack_detection, jack_type);
}

static void handle_jack_not_inserted(struct sec_jack_info *hi)
{
	sec_jack_set_type(hi, SEC_JACK_NO_DEVICE);
	hi->pdata->set_micbias_state(false);
}

static void determine_jack_type(struct sec_jack_info *hi)
{
	struct sec_jack_platform_data *pdata = hi->pdata;
	struct sec_jack_zone *zones = pdata->zones;
	int size = pdata->num_zones;
	int count[MAX_ZONE_LIMIT] = {0};
	int adc;
	int i;



#if defined(CONFIG_MACH_VIPER) || defined(CONFIG_MACH_CHIEF)
	unsigned npolarity = pdata->det_active_high;
#else 
	unsigned npolarity = !pdata->det_active_high;
#endif

	/* set mic bias to enable adc */
	pdata->set_micbias_state(true);

	while (gpio_get_value(pdata->det_gpio) ^ npolarity) {
		adc = pdata->get_adc_value();
		pr_debug("%s: adc = %d\n", __func__, adc);

		/* determine the type of headset based on the
		 * adc value.  An adc value can fall in various
		 * ranges or zones.  Within some ranges, the type
		 * can be returned immediately.  Within others, the
		 * value is considered unstable and we need to sample
		 * a few more types (up to the limit determined by
		 * the range) before we return the type for that range.
		 */
		for (i = 0; i < size; i++) {
			if (adc <= zones[i].adc_high) {
				if (++count[i] > zones[i].check_count) {
					sec_jack_set_type(hi,
							  zones[i].jack_type);
					return;
				}
				msleep(zones[i].delay_ms);
				break;
			}
		}
	}
	/* jack removed before detection complete */
	pr_debug("%s : jack removed before detection complete\n", __func__);
	handle_jack_not_inserted(hi);
}

#ifdef CONFIG_MACH_CHIEF
static void jack_input_selector(int jack_type_status)
{
        SEC_JACKDEV_DBG("jack_type_status = 0X%x", jack_type_status);
}

static void jack_type_detect_change(struct work_struct *ignored)
{
		int headset_state = gpio_get_value(jack_info->pdata->det_gpio) ^ jack_info->pdata->det_active_high;
        int sendend_short_state,sendend_open_state;
	
		printk(" ********************* LNT DEBUG : jack type detect change *************** \n");
        if(headset_state)
        {
        		//  sendend_state = gpio_get_value(send_end->gpio) ^ send_end->low_active;
            	sendend_short_state = gpio_get_value(jack_info->pdata->short_send_end_gpio) ^ jack_info->pdata->det_active_high;

       			#if 1 //open_send_end do nothing
                if (1) // suik_Fix (HWREV >= 0x01)
                {
                    // sendend_open_state = gpio_get_value(send_end_open->gpio) ^ send_end_open->low_active;
					sendend_open_state = gpio_get_value(jack_info->pdata->open_send_end_gpio) ^ jack_info->pdata->det_active_low;
    				SEC_JACKDEV_DBG("SendEnd state short: %d Sendend open: %d sendend type : %d \n",sendend_short_state,sendend_open_state,sendend_type);
    				//if(sendend_state || sendend_open_state)   //suik_Fix
    				if(!sendend_open_state)
    				{
        				printk("4 pole  headset attached\n");
        				current_jack_type_status = SEC_HEADSET_4POLE;//SEC_HEADSET_4_POLE_DEVICE;
    					#if 1 // !defined(CONFIG_MACH_FORTE) && !defined(CONFIG_MACH_CHIEF) // REV07 Only suik_Check
        				// if(gpio_get_value(send_end->gpio))
        				if(gpio_get_value(jack_info->pdata->short_send_end_gpio))
   						#endif
        				{
            				//  enable_irq (send_end->eint);  //suik_Fix
            				enable_irq (jack_info->pdata->short_send_end_eintr);  //suik_Fix
			  	 			printk("*************** LNT DEBUG *********** Enabled Short and Open Irqs \n");
                        }
                        	// enable_irq (send_end_open->eint);
                            enable_irq (jack_info->pdata->open_send_end_eintr);
                    }else
                    {
                    	printk("3 pole headset attatched\n");
                        current_jack_type_status = SEC_HEADSET_3POLE; //SEC_HEADSET_3_POLE_DEVICE;
                    }

                }else
        		#endif
                {
                	if(sendend_short_state)
                    {
                		printk("4 pole  headset attached\n");
                                current_jack_type_status = SEC_HEADSET_4POLE; //SEC_HEADSET_4_POLE_DEVICE;
                    }else
                    {
                            printk("3 pole headset attatched\n");
                            current_jack_type_status = SEC_HEADSET_3POLE; //SEC_HEADSET_3_POLE_DEVICE;
                    }
                       // enable_irq (send_end->eint);
                        enable_irq (jack_info->pdata->short_send_end_eintr);
                }
                send_end_irq_token++;
                switch_set_state(&switch_jack_detection, current_jack_type_status);
                jack_input_selector(current_jack_type_status);
    	}
    	wake_unlock(&jack_sendend_wake_lock);
}

static DECLARE_DELAYED_WORK(detect_jack_type_work, jack_type_detect_change);

static void jack_detect_change(struct work_struct *ignored)
{
	int headset_state;

	printk(" *************** LNT DEBUG *************** %s \n",__func__);
	// SEC_JACKDEV_DBG("");
	del_timer(&jack_detect_timer);
	cancel_delayed_work_sync(&detect_jack_type_work);

	headset_state = gpio_get_value(jack_info->pdata->det_gpio) ^ jack_info->pdata->det_active_high;
#if 1
	SEC_JACKDEV_DBG("jack_detect_change state %d send_end_irq_token %d", headset_state,send_end_irq_token);
	if (headset_state && !send_end_irq_token)
	{
		SEC_JACKDEV_DBG("************* Locked jack send end wake lock ");	
		wake_lock(&jack_sendend_wake_lock);
		if (jack_info->pdata->set_popup_sw_state)
			jack_info->pdata->set_popup_sw_state(1);
		SEC_JACKDEV_DBG("JACK dev attached timer start\n");
		jack_detect_timer_token = 0;
		jack_detect_timer.expires = DETECTION_CHECK_TIME;
		add_timer(&jack_detect_timer);
		sendend_type =0x00;//short type always
	}
	else if(!headset_state && send_end_irq_token)
	{
		/*  if(!get_recording_status())
		    {
		    gpio_set_value(GPIO_MICBIAS_EN, 0);
		    } */
		current_jack_type_status = SEC_JACK_NO_DEVICE;
		switch_set_state(&switch_jack_detection, current_jack_type_status);
		if (jack_info->pdata->set_popup_sw_state)
			jack_info->pdata->set_popup_sw_state(0);
		printk("JACK dev detached %d \n", send_end_irq_token);
		if(send_end_irq_token > 0)
		{
			//  if (1) //suik_Fix (HWREV >= 0x01)
			disable_irq (jack_info->pdata->open_send_end_eintr);
			disable_irq (jack_info->pdata->short_send_end_eintr);
			send_end_irq_token--;
			sendend_type = 0;
		}
		SEC_JACKDEV_DBG("************* Unlocked jack send end wake lock ");	
		wake_unlock(&jack_sendend_wake_lock);
	}
	else
		SEC_JACKDEV_DBG("Headset state does not valid. or send_end event");
#endif

}

static DECLARE_WORK(jack_detect_work, jack_detect_change);

static void jack_detect_timer_handler(unsigned long arg)
{
        struct sec_jack_platform_data *pdata = jack_info->pdata;
        int headset_state = 0;

        headset_state = gpio_get_value(pdata->det_gpio) ^ pdata->det_active_high;

		printk("****************** LNT DEBUG ********** In Jack detect timer handler: timer token : %d \n", jack_detect_timer_token);

        if(headset_state)
        {
               // SEC_JACKDEV_DBG("jack_detect_timer_token is %d\n", jack_detect_timer_token);
                if(jack_detect_timer_token < DETECTION_CHECK_COUNT)
                {
                        jack_detect_timer.expires = DETECTION_CHECK_TIME;
                        add_timer(&jack_detect_timer);
                        jack_detect_timer_token++;
                        //gpio_set_value(GPIO_MICBIAS_EN, 1); //suik_Fix for saving Sleep current
                }
                else if(jack_detect_timer_token == DETECTION_CHECK_COUNT)
                {
                        jack_detect_timer.expires = SEND_END_ENABLE_TIME;
                        jack_detect_timer_token = 0;
			printk(" ********** LNT DEBUG ********* JACK DETECT TIMER HANDLER : sendend_type : %d \n",sendend_type);
                        schedule_delayed_work(&detect_jack_type_work,50);
                }
                else if(jack_detect_timer_token == 4)
                {
                 //       SEC_JACKDEV_DBG("mic bias enable add work queue \n");
                        jack_detect_timer_token = 0;
                }
                else
                	printk(KERN_ALERT "wrong jack_detect_timer_token count %d", jack_detect_timer_token);
        }
        else
        	printk(KERN_ALERT "headset detach!! %d", jack_detect_timer_token);
}

static void send_end_key_event_timer_handler(unsigned long arg)
{
	struct sec_jack_platform_data *pdata = jack_info->pdata;
	int sendend_state, headset_state = 0;

	headset_state = gpio_get_value(pdata->det_gpio) ^ pdata->det_active_high;

	printk(" ****************** LNT DEBUG ************** In send end key event timer handler \n ");

#if 1 //open_send_end do nothing..//suik_Fix
	if (sendend_type)
	{	
		sendend_state = gpio_get_value(pdata->open_send_end_gpio) ^ pdata->det_active_low;
	}else
#endif
	{
		sendend_state = gpio_get_value(pdata->short_send_end_gpio) ^ pdata->det_active_high;
	}

	printk(" ************** LNT DEBUG ********** In send event timer handler : headset state : %d , sendend_state : %d \n",headset_state,sendend_state);

	if(headset_state && sendend_state)
	{
		if(send_end_key_timer_token < SEND_END_CHECK_COUNT)
		{
			send_end_key_timer_token++;
			send_end_key_event_timer.expires = SEND_END_CHECK_TIME;
			add_timer(&send_end_key_event_timer);
			// SEC_JACKDEV_DBG("SendEnd Timer Restart %d", send_end_key_timer_token);
			printk("SendEnd Timer Restart %d", send_end_key_timer_token);
		}
		else if(send_end_key_timer_token == SEND_END_CHECK_COUNT)
		{
			printk("%s:SEND/END is pressed\n", __func__);
			input_report_key(jack_info->input, KEYCODE_SENDEND, 1); //suik_Fix
			input_sync(jack_info->input);
			send_end_key_timer_token = 0;
		}
		else
			printk(KERN_ALERT "[JACK]wrong timer counter %d\n", send_end_key_timer_token);
	}else
		printk(KERN_ALERT "[JACK]GPIO Error\n");
}
#endif
/* thread run whenever the headset detect state changes (either insertion
 * or removal).
 */
static irqreturn_t sec_jack_detect_irq_thread(int irq, void *dev_id)
{
	struct sec_jack_info *hi = dev_id;
	struct sec_jack_platform_data *pdata = hi->pdata;
	int time_left_ms = DET_CHECK_TIME_MS;


#if defined(CONFIG_MACH_VIPER) || defined(CONFIG_MACH_CHIEF)
	unsigned npolarity = pdata->det_active_high;
#else
	unsigned npolarity = !pdata->det_active_high;
#endif

	/* prevent suspend to allow user space to respond to switch */
	wake_lock_timeout(&hi->det_wake_lock, WAKE_LOCK_TIME);

	/* debounce headset jack.  don't try to determine the type of
	 * headset until the detect state is true for a while.
	 */
	while (time_left_ms > 0) {
		if (!(gpio_get_value(pdata->det_gpio) ^ npolarity)) {
			/* jack not detected. */
			handle_jack_not_inserted(hi);
			return IRQ_HANDLED;
		}
		msleep(10);
		time_left_ms -= 10;
	}
	/* jack presence was detected the whole time, figure out which type */
	determine_jack_type(hi);
#ifdef CONFIG_MACH_CHIEF
	schedule_work(&jack_detect_work);
#endif	
	
	return IRQ_HANDLED;
}

/* thread run whenever the button of headset is pressed or released */
void sec_jack_buttons_work(struct work_struct *work)
{
	struct sec_jack_info *hi =
		container_of(work, struct sec_jack_info, buttons_work);
	struct sec_jack_platform_data *pdata = hi->pdata;
	struct sec_jack_buttons_zone *btn_zones = pdata->buttons_zones;
	int adc;
	int i;

	/* prevent suspend to allow user space to respond to switch */
	wake_lock_timeout(&hi->det_wake_lock, WAKE_LOCK_TIME);

	/* when button is released */
	if (hi->pressed == 0) {
		input_report_key(hi->input_dev, hi->pressed_code, 0);
		switch_set_state(&switch_sendend, 0);
		input_sync(hi->input_dev);
		pr_info("%s: keycode=%d, is released\n", __func__,
			hi->pressed_code);
		return;
	}

	/* when button is pressed */
	adc = pdata->get_adc_value();
	pr_debug("%s: adc = %d\n", __func__, adc);

	for (i = 0; i < pdata->num_buttons_zones; i++)
		if (adc >= btn_zones[i].adc_low &&
		    adc <= btn_zones[i].adc_high) {
			hi->pressed_code = btn_zones[i].code;
			input_report_key(hi->input_dev, btn_zones[i].code, 1);
			switch_set_state(&switch_sendend, 1);
			input_sync(hi->input_dev);
			pr_info("%s: keycode=%d, is pressed\n", __func__,
				btn_zones[i].code);
			return;
		}

	pr_warn("%s: key is skipped. ADC value is %d\n", __func__, adc);
}

#ifdef CONFIG_MACH_CHIEF
static void open_sendend_switch_change(struct work_struct *work)
{

	int sendend_state, headset_state;
	struct sec_jack_platform_data *pdata = jack_info->pdata;
	//        SEC_JACKDEV_DBG("");
	del_timer(&send_end_key_event_timer);
	send_end_key_timer_token = 0;

	printk(" ****************** LNT DEBUG ************** In open send end switch change \n ");

	headset_state = gpio_get_value(pdata->det_gpio) ^ pdata->det_active_high;
	sendend_state = gpio_get_value(pdata->open_send_end_gpio) ^ pdata->det_active_low;

	printk("************* LNT DEBUG *********** : headset state : %d , sendend state : %d \n",headset_state, sendend_state);

	//if(headset_state && send_end_irq_token)//headset connect && send irq enable
	if(headset_state )//headset connect && send irq enable
	{
		printk(" open_sendend_switch_change sendend state %d\n",sendend_state);
		if(!sendend_state)  //suik_Fix sams as Sendend(Short)
		{
			// SEC_JACKDEV_DBG(KERN_ERR "sendend isr work queue\n");
			switch_set_state(&switch_sendend, sendend_state);
			input_report_key(jack_info->input, KEYCODE_SENDEND, 0); //released    //suik_Fix
			printk(" *********** Reported released event to platform *********** \n");
			input_sync(jack_info->input);
			printk("%s:SEND/END %s.\n", __func__, "released");
			wake_unlock(&jack_sendend_wake_lock);
		}else
		{
			wake_lock(&jack_sendend_wake_lock);
			send_end_key_event_timer.expires = SEND_END_CHECK_TIME;
			add_timer(&send_end_key_event_timer);
			printk(" *********** Added timer to check for few milli seconds in open send end switch change  *********** \n");
			switch_set_state(&switch_sendend, sendend_state);
			printk("%s:SEND/END %s.timer start \n", __func__, "pressed");
		}

	}else
	{
		printk("********** In else part of headset state in Bottom Half \n");
		//   SEC_JACKDEV_DBG("SEND/END Button is %s but headset disconnect or irq disable.\n", state?"pressed":"released");
	}
}


static DECLARE_WORK(open_sendend_switch_work, open_sendend_switch_change);

/* IRQ handler for Open SEND END */
static irqreturn_t send_end_open_irq_handler(int irq, void *dev_id)
{
	int headset_state;
	struct sec_jack_info *hi = dev_id;
	struct sec_jack_platform_data *pdata = hi->pdata;

	printk("************ LNT DEBUG *********** ENTERED OPEN SEND END IRQ HANDLER \n");

	/* SEC_JACKDEV_DBG("[OPEN]send_end_open_irq_handler isr"); */
	del_timer(&send_end_key_event_timer);
	headset_state = gpio_get_value(pdata->det_gpio) ^ (pdata->det_active_high);

	printk(" **************** LNT DEBUG *********: headset state : %d \n", headset_state);

	if (headset_state)
	{
		sendend_type = 0x01;
		schedule_work(&open_sendend_switch_work);               //suik_Fix
	}
	return IRQ_HANDLED;
}
#endif

static ssize_t select_jack_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	pr_info("%s : operate nothing\n", __func__);

	return 0;
}

static ssize_t select_jack_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct sec_jack_info *hi = dev_get_drvdata(dev);
	struct sec_jack_platform_data *pdata = hi->pdata;
	int value = 0;


	sscanf(buf, "%d", &value);
	pr_err("%s: User  selection : 0X%x", __func__, value);
	if (value == SEC_HEADSET_4POLE) {
		pdata->set_micbias_state(true);
		msleep(100);
	}

	sec_jack_set_type(hi, value);

	return size;
}

static DEVICE_ATTR(select_jack, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH,
	select_jack_show, select_jack_store);

static int sec_jack_probe(struct platform_device *pdev)
{
	struct sec_jack_info *hi;
	struct sec_jack_platform_data *pdata = pdev->dev.platform_data;
	int ret;

	#ifdef CONFIG_MACH_CHIEF
		struct input_dev *input;
	#endif

	pr_info("%s : Registering jack driver\n", __func__);
	if (!pdata) {
		pr_err("%s : pdata is NULL.\n", __func__);
		return -ENODEV;
	}

	if (!pdata->get_adc_value || !pdata->zones ||
	    !pdata->set_micbias_state || pdata->num_zones > MAX_ZONE_LIMIT) {
		pr_err("%s : need to check pdata\n", __func__);
		return -ENODEV;
	}

	if (atomic_xchg(&instantiated, 1)) {
		pr_err("%s : already instantiated, can only have one\n",
			__func__);
		return -ENODEV;
	}

	sec_jack_key_map[0].gpio = pdata->send_end_gpio;

	hi = kzalloc(sizeof(struct sec_jack_info), GFP_KERNEL);
	
	#ifdef CONFIG_MACH_CHIEF
		jack_info = hi;
	#endif
	
	if (hi == NULL) {
		pr_err("%s : Failed to allocate memory.\n", __func__);
		ret = -ENOMEM;
		goto err_kzalloc;
	}

	hi->pdata = pdata;

	/* make the id of our gpio_event device the same as our platform device,
	 * which makes it the responsiblity of the board file to make sure
	 * it is unique relative to other gpio_event devices
	 */
	hi->dev_id = pdev->id;

	ret = gpio_request(pdata->det_gpio, "ear_jack_detect");
	if (ret) {
		pr_err("%s : gpio_request failed for %d\n",
		       __func__, pdata->det_gpio);
		goto err_gpio_request;
	}

/* Allocate the input device for reporting the sendend events to platform */
#ifdef CONFIG_MACH_CHIEF	
	input = hi->input = input_allocate_device();
	if (!input)
	{
		ret = -ENOMEM;
		printk(KERN_ERR "SEC JACK: Failed to allocate input device.\n");
		goto err_request_input_dev;
	}

	input->name = "sec_jack";
	set_bit(EV_SYN, input->evbit);
	set_bit(EV_KEY, input->evbit);
	set_bit(KEYCODE_SENDEND, input->keybit);
	ret = input_register_device(input); 
	if (ret < 0)
	{
		printk(KERN_ERR "SEC JACK: Failed to register driver\n");
		goto err_register_input_dev;
	}
#endif

	ret = switch_dev_register(&switch_jack_detection);
	if (ret < 0) {
		pr_err("%s : Failed to register switch device\n", __func__);
		goto err_switch_dev_register;
	}
	
#ifdef CONFIG_MACH_CHIEF
	wake_lock_init(&jack_sendend_wake_lock, WAKE_LOCK_SUSPEND, "sec_jack");
	init_timer(&jack_detect_timer);
	jack_detect_timer.function = jack_detect_timer_handler;
	init_timer(&send_end_key_event_timer);
	send_end_key_event_timer.function = send_end_key_event_timer_handler;
        if (IS_ERR_OR_NULL(earmic_regulator)) {
                earmic_regulator = regulator_get(NULL, "earmic");
                if (IS_ERR_OR_NULL(earmic_regulator)) {
                        pr_err("failed to get earmic bias regulator");
                        return -EINVAL;
                }
        }
#endif


	ret = switch_dev_register(&switch_sendend);
	if (ret < 0) {
		printk(KERN_ERR "SEC JACK: Failed to register switch device\n");
		goto err_switch_dev_register_send_end;
	}
	wake_lock_init(&hi->det_wake_lock, WAKE_LOCK_SUSPEND, "sec_jack_det");

	INIT_WORK(&hi->buttons_work, sec_jack_buttons_work);
	hi->queue = create_singlethread_workqueue("sec_jack_wq");
	if (hi->queue == NULL) {
		ret = -ENOMEM;
		pr_err("%s: Failed to create workqueue\n", __func__);
		goto err_create_wq_failed;
	}

	hi->det_irq = gpio_to_irq(pdata->det_gpio);

	jack_class = class_create(THIS_MODULE, "jack");
	if (IS_ERR(jack_class))
		pr_err("Failed to create class(sec_jack)\n");

	/* support PBA function test */
	jack_dev = device_create(jack_class, NULL, 0, hi, "jack_selector");
	if (IS_ERR(jack_dev))
		pr_err("Failed to create device(sec_jack)!= %ld\n",
			IS_ERR(jack_dev));

	if (device_create_file(jack_dev, &dev_attr_select_jack) < 0)
		pr_err("Failed to create device file(%s)!\n",
			dev_attr_select_jack.attr.name);
#ifndef  CONFIG_MACH_CHIEF
	set_bit(EV_KEY, hi->ids[0].evbit);
	hi->ids[0].flags = INPUT_DEVICE_ID_MATCH_EVBIT;
	hi->handler.filter = sec_jack_buttons_filter;
	hi->handler.connect = sec_jack_buttons_connect;
	hi->handler.disconnect = sec_jack_buttons_disconnect;
	hi->handler.name = "sec_jack_buttons";
	hi->handler.id_table = hi->ids;
	hi->handler.private = hi;

	ret = input_register_handler(&hi->handler);
	if (ret) {
		pr_err("%s : Failed to register_handler\n", __func__);
		goto err_register_input_handler;
	}
#endif
	ret = request_threaded_irq(hi->det_irq, NULL,
				   sec_jack_detect_irq_thread,
				   IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING |
				   IRQF_ONESHOT, "sec_headset_detect", hi);
	if (ret) {
		pr_err("%s : Failed to request_irq.\n", __func__);
		goto err_request_detect_irq;
	}

	/* to handle insert/removal when we're sleeping in a call */
	ret = enable_irq_wake(hi->det_irq);
	if (ret) {
		pr_err("%s : Failed to enable_irq_wake.\n", __func__);
		goto err_enable_irq_wake;
	}

#ifdef CONFIG_MACH_CHIEF
	sendend_type = 0; /* By default sendend type 0 always , Short type */
	set_irq_type(pdata->open_send_end_eintr,IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_ONESHOT );
	ret = request_threaded_irq(pdata->open_send_end_eintr,NULL, 
			send_end_open_irq_handler, IRQF_DISABLED, 
			"sec_headset_send_end_open",hi);
	if (ret < 0) {
		printk(KERN_ERR "SEC HEADSET: Failed to register OPEN send/end interrupt.\n");
		goto err_request_open_send_end_irq;
	}
	disable_irq(pdata->open_send_end_eintr);

	printk("******************* LNT DEBUG *********** SEND END OPEN IRQ registration success, gpio-pin : %d \n ",pdata->open_send_end_gpio ); 

	schedule_work(&jack_detect_work);
#endif

	dev_set_drvdata(&pdev->dev, hi);

#if defined(CONFIG_MACH_VIPER)
        sec_jack_detect_irq_thread(hi->det_irq, hi);
#endif

	return 0;

#ifdef CONFIG_MACH_CHIEF
err_request_open_send_end_irq:
err_register_input_dev:
    input_free_device(input);	
#endif
err_enable_irq_wake:
	free_irq(hi->det_irq, hi);
err_request_detect_irq:
	input_unregister_handler(&hi->handler);
err_register_input_handler:
	destroy_workqueue(hi->queue);
err_create_wq_failed:
	wake_lock_destroy(&hi->det_wake_lock);
	switch_dev_unregister(&switch_sendend);
err_switch_dev_register_send_end:
	switch_dev_unregister(&switch_jack_detection);
err_switch_dev_register:
	gpio_free(pdata->det_gpio);

#ifdef CONFIG_MACH_CHIEF
err_request_input_dev:
#endif
err_gpio_request:
	kfree(hi);
#ifdef CONFIG_MACH_CHIEF	
	jack_info=NULL;
#endif	
err_kzalloc:
	atomic_set(&instantiated, 0);
	return ret;
}

static int sec_jack_remove(struct platform_device *pdev)
{

	struct sec_jack_info *hi = dev_get_drvdata(&pdev->dev);

	pr_info("%s :\n", __func__);
#ifdef CONFIG_MACH_CHIEF	
	input_unregister_device(hi->input);
	free_irq(hi->pdata->open_send_end_eintr,hi);
#endif	
	disable_irq_wake(hi->det_irq);
	free_irq(hi->det_irq, hi);
	destroy_workqueue(hi->queue);
	if (hi->send_key_dev) {
		platform_device_unregister(hi->send_key_dev);
		hi->send_key_dev = NULL;
	}
	input_unregister_handler(&hi->handler);
	wake_lock_destroy(&hi->det_wake_lock);
	switch_dev_unregister(&switch_sendend);
	switch_dev_unregister(&switch_jack_detection);
	gpio_free(hi->pdata->det_gpio);
	kfree(hi);
	atomic_set(&instantiated, 0);

	return 0;
}

static struct platform_driver sec_jack_driver = {
	.probe = sec_jack_probe,
	.remove = sec_jack_remove,
	.driver = {
			.name = "sec_jack",
			.owner = THIS_MODULE,
		   },
};
static int __init sec_jack_init(void)
{
	return platform_driver_register(&sec_jack_driver);
}

static void __exit sec_jack_exit(void)
{
	platform_driver_unregister(&sec_jack_driver);
}

module_init(sec_jack_init);
module_exit(sec_jack_exit);

MODULE_AUTHOR("ms17.kim@samsung.com");
MODULE_DESCRIPTION("Samsung Electronics Corp Ear-Jack detection driver");
MODULE_LICENSE("GPL");
