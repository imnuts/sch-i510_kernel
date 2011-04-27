/*
 * qt602240_ts.c - AT42QT602240 Touchscreen driver
 *
 * Copyright (C) 2009 Samsung Electronics Co.Ltd
 * Author: Joonyoung Shim <jy0922.shim@samsung.com>
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

/*
 * TODO:
 * - Multi touch support
 * - Gesture support
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/firmware.h>
#include <linux/i2c.h>
#include <linux/i2c/qt602240_ts.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/gpio.h>
#include <plat/gpio-cfg.h>
#include <linux/leds.h>
#include "qt602240_p1.h"
#include <linux/vmalloc.h>
#include "mach/gpio-p1.h"

/******************************************************************************
* 
*
*       QT602240 Object table init
* 
*                                                             
* *****************************************************************************/
//General Object
gen_powerconfig_t7_config_t power_config = {0};                 //Power config settings.
gen_acquisitionconfig_t8_config_t acquisition_config = {0};     // Acquisition config. 

//Touch Object
touch_multitouchscreen_t9_config_t touchscreen_config = {0};    //Multitouch screen config.
touch_keyarray_t15_config_t keyarray_config = {0};              //Key array config.
touch_proximity_t23_config_t proximity_config = {0};        //Proximity config. 

//Signal Processing Objects
proci_gripfacesuppression_t20_config_t gripfacesuppression_config = {0};    //Grip / face suppression config.
procg_noisesuppression_t22_config_t noise_suppression_config = {0};         //Noise suppression config.
proci_onetouchgestureprocessor_t24_config_t onetouch_gesture_config = {0};  //One-touch gesture config. 
proci_twotouchgestureprocessor_t27_config_t twotouch_gesture_config = {0};  //Two-touch gesture config.

//Support Objects
spt_gpiopwm_t19_config_t  gpiopwm_config = {0};             //GPIO/PWM config
spt_selftest_t25_config_t selftest_config = {0};            //Selftest config.
spt_cteconfig_t28_config_t cte_config = {0};                //Capacitive touch engine config.

spt_comcconfig_t18_config_t   comc_config = {0};            //Communication config settings.
gen_commandprocessor_t6_config_t    command_config = {0};

static report_finger_info_t fingerInfo[MAX_USING_FINGER_NUM];

static int set_mode_for_ta = 0;		// 1: TA or USB, 0: normal
unsigned char maxim_chg_status(void);	// 1: TA or UST, 0: normal

static int set_mode_for_amoled = 0;		//0: TFt-LCD, 1: AMOLED

static int config_mode_val = 0; 	//0: normal 1: stylus

static int gFirmware_Update_State = FW_UPDATE_READY;

//extern symbol from mach-p1.c
extern unsigned int HWREV;
extern void touh_led_on(int type);

#ifdef CONFIG_HAS_EARLYSUSPEND
struct qt602240_data * p_qt602240_data;
static void qt602240_early_suspend(struct early_suspend *);
static void qt602240_late_resume(struct early_suspend *);
#endif	/* CONFIG_HAS_EARLYSUSPEND */

#if defined (KEY_LED_CONTROL)
static struct timer_list  g_led_timer;
void led_control(int data)
{
    int i;

    for(i = 0; i < data; i++)
    {
        gpio_set_value(KEYLED_EN, 1);
        udelay(10);
        gpio_set_value(KEYLED_EN, 0); 
        udelay(10);
    }
    gpio_set_value(KEYLED_EN, 1);
    
    //wait for the mode change
    udelay(500);
}

void init_led(void)
{
    gpio_request(KEYLED_EN, "KEYLED_EN");
    s3c_gpio_cfgpin(KEYLED_EN, GPIO_OUTPUT);
    s3c_gpio_setpull(KEYLED_EN, S3C_GPIO_PULL_NONE);

    //KEYLED is only working in low current mode.
    led_control(15);
    led_control(KEYLED_ADDRESS_MAX);      // [Address ] Max current setting
    led_control(KEYLED_DATA_LOW);           // [Data] Low current mode
    led_control(KEYLED_ADDRESS_LOW);      // [Address] Low current mode
    led_control(2);                                     // [Data] 0.5mA
}

void touh_led_on(int type)
{
    int data =0;
    
    switch(type)
    {
        case 0:
            data = 1;
            break;
            
        case 1:
        case 2:
        case 3:
        case 4:
            /*
            Data      D4      D3      D2      D1
            1           on      on      on      on
            2           on      on      on      off
            3           on      on      off     on
            4           on      on      off     off
            ....
            16          off     off     off     off
            */
            data = ((0x1)<<(type-1)) + 1;
            break;

        default:
            printk("[TSP] %s Unknown data\n", __func__);
            break;
    }

    // [Address ] Led on/off
    led_control(KEYLED_ADDRESS_ONOFF);
    led_control(data);

}

static void TouchKey_Led_TimerProc(void)
{
    touh_led_on(0);
}

#endif

static struct qt602240_object * qt602240_get_object(struct qt602240_data *data, u8 type)
{
	struct qt602240_object *object;
	int i;

	for (i = 0; i < data->info->object_num; i++) 
	{
		object = data->object_table + i;
		if (object->type == type)
		{
		    return object;
		}
	}

	dev_err(&data->client->dev, "invalid type\n");
	return NULL;
}

int qt602240_reg_write(struct qt602240_data *qt602240data,  u8 type, u8 *values)
{
    int i;
    unsigned char data[I2C_MAX_SEND_LENGTH];
    struct i2c_msg wmsg;
    struct qt602240_object *object;

    object = qt602240_get_object(qt602240data, type);
    if (!object)
        printk("[TSP] error : 0x%x\n", object->type);

    if(object->size > ( I2C_MAX_SEND_LENGTH - 2 ))
        printk("[TSP][ERROR] %s() data length error\n", __FUNCTION__);

    wmsg.addr = qt602240data->client->addr;
    wmsg.flags = I2C_M_WR;
    wmsg.len = (object->size + 3);
    wmsg.buf = data;

    data[0] = object->start_address & 0x00ff;
    data[1] = object->start_address >> 8;

    for (i = 0; i < object->size + 1; i++)
    {
        data[i+2] = *(values+i);
    }

    return (i2c_transfer(qt602240data->client->adapter, &wmsg, 1));
}

int QT602240_Command_Config_Init(struct qt602240_data *data)
{
    command_config.reset = 0x0;
    command_config.backupnv = 0x0;
    command_config.calibrate = 0x0;
    command_config.reportall = 0x0;
    command_config.reserve= 0x0;
    command_config.diagnostic = 0x0;
    
    return (qt602240_reg_write(data, GEN_COMMANDPROCESSOR_T6, (void *) &command_config));
}

int QT602240_Powr_Config_Init(struct qt602240_data *data)
{
    power_config.idleacqint = 0x40;//0x20    //64ms in idle status
    power_config.actvacqint = 0xff;           // free run in active status
    power_config.actv2idleto = 0x32;          //10s  
    return (qt602240_reg_write(data, GEN_POWERCONFIG_T7, (void *) &power_config));
}

int QT602240_Acquisition_Config_Init(struct qt602240_data *data)
{
//    int version = data->info->version;

    acquisition_config.chrgtime = 0x6;  //0x8;      //1500ns       Charge-transfer dwell time
    acquisition_config.reserved = 0x00;
    acquisition_config.tchdrift = 0x05;               // 1s              Touch drift time
    acquisition_config.driftst = 0x00;                // 1 cycle        Drift suspend time
    acquisition_config.tchautocal = 0x00;          // infinite        Touch Automatic Calibration
    acquisition_config.sync = 0x00;                  // disabled    
    acquisition_config.atchcalst = 0x09;//0x05;  // 1800ms      Anti-touch calibration time
    acquisition_config.atchcalsthr = 0x0f;//0x14//                  Anti-touch Calibration suspend time

#ifdef OAKWOOD
    acquisition_config.atchcalfrcratio= 0;
    acquisition_config.atchcalfrcthr= 0;
#endif    
    
    return (qt602240_reg_write(data, GEN_ACQUISITIONCONFIG_T8, (void *) &acquisition_config));
}

int QT602240_Multitouch_Config_Init(struct qt602240_data *data)
{
//    int version = data->info->version;

#ifdef OAKWOOD
    touchscreen_config.ctrl = 0x8b;
    touchscreen_config.xorigin = 0x00;
    touchscreen_config.yorigin = 0x00;
    touchscreen_config.xsize = 18;
    touchscreen_config.ysize = 27;
    touchscreen_config.akscfg = 0x00;
    touchscreen_config.blen = 16;         // Gain of the analog circuits in front of the ADC [7:4]
    touchscreen_config.tchthr = 30;       // touch Threshold value
    touchscreen_config.orient = 3;       // 0x4 : Invert Y, 0x2 : Invert X, 0x1 : Switch    
    touchscreen_config.mrgtimeout = 0x00;               
    touchscreen_config.movhysti = 5;   //0x1;    // Move hysteresis, initial
    touchscreen_config.movhystn = 1;  //0x1     // Move hysteresis, next    
    touchscreen_config.movfilter = 0;   // 15              // Filter Limit[6:4] , Adapt threshold [3:0]
    touchscreen_config.numtouch= 0x05;     
    touchscreen_config.tchdi = 0x02;     
    touchscreen_config.mrghyst = 0x0a;               // Merge hysteresis
    touchscreen_config.mrgthr = 10;//0xa            // Merge threshold
    touchscreen_config.amphyst = 0x0a;              // Amplitude hysteresis
    touchscreen_config.xrange1= 0x57;   //2byte
    touchscreen_config.xrange2= 0x02;
    
    touchscreen_config.yrange1 = 0xff;  //2byte
    touchscreen_config.yrange2 = 0x03;    
    touchscreen_config.xloclip = 0x00;
    touchscreen_config.xhiclip = 0x00;
    touchscreen_config.yloclip = 0x00;
    touchscreen_config.yhiclip = 0x00;
    touchscreen_config.xedgectrl = 0x00;
    touchscreen_config.xedgedist = 0x00;
    touchscreen_config.yedgectrl = 0x00;
    touchscreen_config.yedgedist = 0x00;
    touchscreen_config.jumplimit = 15;            //18*8    
#else
    //0x80 :Scan en
    //0x8 : Disable vector change, 0x2: Enable reporting, 0x1 : Enable the multi-touch
    touchscreen_config.ctrl = 0x8b;         //Enable amplitude change : 0x0 << 3
    touchscreen_config.xorigin = 0x00;
    touchscreen_config.yorigin = 0x00;

    if(HWREV >= 0)
    {
        touchscreen_config.xsize = 0x12;
        touchscreen_config.ysize = 0x0b;
    }
    else
    {
        touchscreen_config.xsize = 0x13;
        touchscreen_config.ysize = 0x0b;
    }
    touchscreen_config.akscfg = 0x00;
    touchscreen_config.blen = 0x00;         // Gain of the analog circuits in front of the ADC [7:4]
    touchscreen_config.tchthr = 25;//0x27;       // touch Threshold value
    touchscreen_config.orient = 0x04;       // 0x4 : Invert Y, 0x2 : Invert X, 0x1 : Switch
    
    touchscreen_config.mrgtimeout = 0x00;               
    touchscreen_config.movhysti = 0x0a;   //0x1;    // Move hysteresis, initial
    touchscreen_config.movhystn = 0x0a;  //0x1     // Move hysteresis, next    
    touchscreen_config.movfilter = 0x4b;              // Filter Limit[6:4] , Adapt threshold [3:0]
    touchscreen_config.numtouch= 0x05;     
    touchscreen_config.tchdi = 0x02;     
    touchscreen_config.mrghyst = 0x0a;               // Merge hysteresis
    touchscreen_config.mrgthr = 0x20;//0xa            // Merge threshold
    touchscreen_config.amphyst = 0x0a;              // Amplitude hysteresis
    touchscreen_config.xrange1= 0xff;   //2byte
    touchscreen_config.xrange2= 0x03;
    
    touchscreen_config.yrange1 = 0x57;  //2byte
    touchscreen_config.yrange2 = 0x02;    
    touchscreen_config.xloclip = 0x00;
    touchscreen_config.xhiclip = 0x00;
    touchscreen_config.yloclip = 0x00;
    touchscreen_config.yhiclip = 0x00;
    touchscreen_config.xedgectrl = 0x00;
    touchscreen_config.xedgedist = 0x00;
    touchscreen_config.yedgectrl = 0x00;
    touchscreen_config.yedgedist = 0x00;
    touchscreen_config.jumplimit = 0x12;            //18*8
#endif
   
    return (qt602240_reg_write(data, TOUCH_MULTITOUCHSCREEN_T9, (void *) &touchscreen_config));
}

int QT602240_KeyArrary_Config_Init(struct qt602240_data *data)
{

#ifdef OAKWOOD
    keyarray_config.ctrl = 0x03;
    keyarray_config.xorigin = 14;
    keyarray_config.yorigin = 27;
    keyarray_config.xsize = 0x04;
    keyarray_config.ysize = 0x01;
    keyarray_config.akscfg = 0x00;
    keyarray_config.blen = 0;
    keyarray_config.tchthr = 30;
    keyarray_config.tchdi = 2;
#else
    if(HWREV >= 0x0)
    {
        keyarray_config.ctrl = 0x03;
        keyarray_config.xorigin = 0x00;
        keyarray_config.yorigin = 0x0b;
        keyarray_config.xsize = 0x04;
        keyarray_config.ysize = 0x01;
        keyarray_config.akscfg = 0x00;
        keyarray_config.blen = 0x00;
        keyarray_config.tchthr = 0x20;
        keyarray_config.tchdi = 0x02;
    }
    else
    {
        keyarray_config.ctrl = 0;
        keyarray_config.xorigin = 0;
        keyarray_config.yorigin = 0;
        keyarray_config.xsize = 0;
        keyarray_config.ysize = 0;
        keyarray_config.akscfg = 0;
        keyarray_config.blen = 0;
        keyarray_config.tchthr = 0;
        keyarray_config.tchdi = 0;
    }
#endif
    
    keyarray_config.reserved[0] = 0;
    keyarray_config.reserved[1] = 0;

    return (qt602240_reg_write(data, TOUCH_KEYARRAY_T15, (void *) &keyarray_config));
}

int QT602240_GPIOPWM_Config_Init(struct qt602240_data *data)
{
    gpiopwm_config.ctrl = 0;
    gpiopwm_config.reportmask = 0;
    gpiopwm_config.dir = 0;
    gpiopwm_config.intpullup = 0;
    gpiopwm_config.out = 0;
    gpiopwm_config.wake = 0;
    gpiopwm_config.pwm = 0;
    gpiopwm_config.period = 0;
    gpiopwm_config.duty[0] = 0;
    gpiopwm_config.duty[1] = 0;
    gpiopwm_config.duty[2] = 0;
    gpiopwm_config.duty[3] = 0;

    return (qt602240_reg_write(data, SPT_GPIOPWM_T19, (void *) &gpiopwm_config));
}

int QT602240_Grip_Face_Suppression_Config_Init(struct qt602240_data *data)
{
//    int version = data->info->version;    

 #ifdef OAKWOOD
     gripfacesuppression_config.ctrl = 0x07;//0x4 : Disable Grip suppression, 0x2: Enable report, 0x1 : Enable
     gripfacesuppression_config.xlogrip = 0;
     gripfacesuppression_config.xhigrip = 0;
     gripfacesuppression_config.ylogrip = 0;
     gripfacesuppression_config.yhigrip = 0;
     gripfacesuppression_config.maxtchs = 0;
     gripfacesuppression_config.reserved = 0;
     gripfacesuppression_config.szthr1 = 0;
     gripfacesuppression_config.szthr2 = 0;
     gripfacesuppression_config.shpthr1 = 0;
     gripfacesuppression_config.shpthr2 = 0;
     gripfacesuppression_config.supextto = 0;  
 #else
    gripfacesuppression_config.ctrl = 0x07;//0x4 : Disable Grip suppression, 0x2: Enable report, 0x1 : Enable
    gripfacesuppression_config.xlogrip = 0x00;
    gripfacesuppression_config.xhigrip = 0x00;
    gripfacesuppression_config.ylogrip = 0x00;
    gripfacesuppression_config.yhigrip = 0x00;
    gripfacesuppression_config.maxtchs = 0x00;
    gripfacesuppression_config.reserved = 0x00;
    gripfacesuppression_config.szthr1 = 0x19;
    gripfacesuppression_config.szthr2 = 0x0f;
    gripfacesuppression_config.shpthr1 = 0x02;
    gripfacesuppression_config.shpthr2 = 0x0a;
    gripfacesuppression_config.supextto = 0x32;    
#endif    

    /* Write grip suppression config to chip. */
    return (qt602240_reg_write(data, PROCI_GRIPFACESUPPRESSION_T20, (void *) &gripfacesuppression_config));
}

int QT602240_Noise_Config_Init(struct qt602240_data *data)
{
//    int version = data->info->version;
#ifdef OAKWOOD
    //0x8 : Enable Median filter, 0x4 : Enable Frequency hopping, 0x1 : Enable
    noise_suppression_config.ctrl = 5;    
    noise_suppression_config.reserved = 0;
    noise_suppression_config.reserved1 = 0; 
    noise_suppression_config.gcaful1 = 0;        // Upper limit for the GCAF
    noise_suppression_config.gcaful2 = 0;
    noise_suppression_config.gcafll1 = 0;        // Lower limit for the GCAF
    noise_suppression_config.gcafll2 = 0;
    noise_suppression_config.actvgcafvalid = 0;//0x0f;   //Minium number of samples in active mode
    noise_suppression_config.noisethr = 20;       //0x0f;  // Threshold for the noise signal
    noise_suppression_config.freqhopscale = 1;//0x1e;
    noise_suppression_config.freq[0] = 10;
    noise_suppression_config.freq[1] = 15;
    noise_suppression_config.freq[2] = 20;
    noise_suppression_config.freq[3] = 25;
    noise_suppression_config.freq[4] = 30;
    noise_suppression_config.idlegcafvalid = 0;
#else
    //0x8 : Enable Median filter, 0x4 : Enable Frequency hopping, 0x1 : Enable
    noise_suppression_config.ctrl = 0x0d;    
    noise_suppression_config.reserved = 0;
    noise_suppression_config.reserved1 = 0; 
    noise_suppression_config.gcaful1 = 0x04;        // Upper limit for the GCAF
    noise_suppression_config.gcaful2 = 0x0a;
    noise_suppression_config.gcafll1 = 0x01;        // Lower limit for the GCAF
    noise_suppression_config.gcafll2 = 0x0a;
    noise_suppression_config.actvgcafvalid = 0x04;//0x0f;   //Minium number of samples in active mode
    noise_suppression_config.noisethr = 0x1e;       //0x0f;  // Threshold for the noise signal
    noise_suppression_config.freqhopscale = 0x00;//0x1e;
    noise_suppression_config.freq[0] = 0x06;
    noise_suppression_config.freq[1] = 0x0b;
    noise_suppression_config.freq[2] = 0x0f;
    noise_suppression_config.freq[3] = 0x13;
    noise_suppression_config.freq[4] = 0x15;
    noise_suppression_config.idlegcafvalid = 0x04;
#endif    
 
    /* Write Noise suppression config to chip. */
    return (qt602240_reg_write(data, PROCG_NOISESUPPRESSION_T22, (void *) &noise_suppression_config));
}

int QT602240_One_Touch_Gesture_Config_Init(struct qt602240_data *data)
{
    /* Disable one touch gestures. */
    onetouch_gesture_config.ctrl = 0;
    onetouch_gesture_config.numgest = 0;   
    onetouch_gesture_config.gesten1 = 0;    //2byte
    onetouch_gesture_config.gesten2 = 0;
    onetouch_gesture_config.pressproc = 0;
    onetouch_gesture_config.tapto = 0;
    onetouch_gesture_config.flickto = 0;
    onetouch_gesture_config.dragto = 0;
    onetouch_gesture_config.spressto = 0;
    onetouch_gesture_config.lpressto = 0;
    onetouch_gesture_config.reppressto = 0;
    onetouch_gesture_config.flickthr1 = 0;       //2byte
    onetouch_gesture_config.flickthr2 = 0;
    onetouch_gesture_config.dragthr1 = 0;       //2byte    
    onetouch_gesture_config.dragthr2 = 0;
    onetouch_gesture_config.tapthr1 = 0;        //2byte
    onetouch_gesture_config.tapthr2 = 0;
    onetouch_gesture_config.throwthr1 = 0;     //2byte        
    onetouch_gesture_config.throwthr2 = 0;
    
    return (qt602240_reg_write(data, PROCI_ONETOUCHGESTUREPROCESSOR_T24, (void *) &onetouch_gesture_config));
}

int QT602240_Proximity_Config_Init(struct qt602240_data *data)
{
    /* Disable Proximity. */
    proximity_config.ctrl = 0;
    proximity_config.xorigin = 0;
    proximity_config.xsize = 0;
    proximity_config.ysize = 0;
    proximity_config.reserved_for_future_aks_usage = 0;
    proximity_config.blen = 0;
    proximity_config.tchthr1 = 0;
    proximity_config.tchthr2 = 0;
    proximity_config.tchdi = 0;
    proximity_config.average = 0;
    proximity_config.rate1 = 0;
    proximity_config.rate2 = 0;

    return (qt602240_reg_write(data, TOUCH_PROXIMITY_T23, (void *) &proximity_config));
}

int QT602240_Selftest_Config_Init(struct qt602240_data *data)
{
    selftest_config.ctrl = 0;
    selftest_config.cmd = 0;
    
    return (qt602240_reg_write(data, SPT_SELFTEST_T25, (void *) &selftest_config));
}

int QT602240_Two_touch_Gesture_Config_Init(struct qt602240_data *data)
{
    /* Disable two touch gestures. */
    twotouch_gesture_config.ctrl = 0;
    twotouch_gesture_config.numgest = 0;
    twotouch_gesture_config.reserved2 = 0;
    twotouch_gesture_config.gesten = 0;
    twotouch_gesture_config.rotatethr = 0;
    twotouch_gesture_config.zoomthr1 = 0;       //2byte
    twotouch_gesture_config.zoomthr2 = 0;

    return (qt602240_reg_write(data, PROCI_TWOTOUCHGESTUREPROCESSOR_T27, (void *) &twotouch_gesture_config));
}

int QT602240_CTE_Config_Init(struct qt602240_data *data)
{
//    int version = data->info->version;
    
     /* Set CTE config */
    cte_config.ctrl = 0x00;     //reserved
    cte_config.cmd = 0x00; 

#ifdef OAKWOOD
    cte_config.mode = 0;
    cte_config.actvgcafdepth = 0x10;//0x63   //Size of sampling window in active acquisition mode
    cte_config.voltage = 0x3c;                    // 0.01 * 60 + 2.7
#else
    if(HWREV >= 0x0)
        cte_config.mode = 0x02;
    else
        cte_config.mode = 0x03;
#endif
    cte_config.idlegcafdepth = 0x8;//0x20      //Size of sampling window in idle acquisition mode
    cte_config.actvgcafdepth = 0x20;//0x63   //Size of sampling window in active acquisition mode
    cte_config.voltage = 0x0a;                    // 0.01 * 10 + 2.7
    
     /* Write CTE config to chip. */
    return (qt602240_reg_write(data, SPT_CTECONFIG_T28, (void *) &cte_config));
}

static int qt602240_object_readable(unsigned int type)
{
	switch (type) {
	case QT602240_GEN_MESSAGE:
	case QT602240_GEN_COMMAND:
	case QT602240_GEN_POWER:
	case QT602240_GEN_ACQUIRE:
	case QT602240_TOUCH_MULTI:
	case QT602240_TOUCH_KEYARRAY:
	case QT602240_TOUCH_PROXIMITY:
	case QT602240_PROCI_GRIPFACE:
	case QT602240_PROCG_NOISE:
	case QT602240_PROCI_ONETOUCH:
	case QT602240_PROCI_TWOTOUCH:
	case QT602240_SPT_GPIOPWM:
	case QT602240_SPT_SELFTEST:
	case QT602240_SPT_CTECONFIG:
		return 1;
	default:
		return 0;
	}
}

#if 0
static int qt602240_object_writable(unsigned int type)
{
	switch (type) {
	case QT602240_GEN_COMMAND:
	case QT602240_GEN_POWER:
	case QT602240_GEN_ACQUIRE:
	case QT602240_TOUCH_MULTI:
	case QT602240_TOUCH_KEYARRAY:
	case QT602240_TOUCH_PROXIMITY:
	case QT602240_PROCI_GRIPFACE:
	case QT602240_PROCG_NOISE:
	case QT602240_PROCI_ONETOUCH:
	case QT602240_PROCI_TWOTOUCH:
	case QT602240_SPT_GPIOPWM:
	case QT602240_SPT_SELFTEST:
	case QT602240_SPT_CTECONFIG:
		return 1;
	default:
		return 0;
	}
}

static void write_file(char *filename, char *data)
{
    struct file *file;
    loff_t pos = 0;
    int fd;

    mm_segment_t old_fs = get_fs();
    set_fs(KERNEL_DS);

    fd = sys_open(filename, O_WRONLY|O_CREAT, 0644);
    if (fd >= 0) 
    {
        sys_write(fd, data, strlen(data));
        file = fget(fd);
        if (file) 
        {
            vfs_write(file, data, strlen(data), &pos);
            fput(file);
        }
        sys_close(fd);
    }
    set_fs(old_fs);
}
#endif

static void qt602240_dump_message(struct qt602240_data *data)
{
	struct device *dev = &data->client->dev;
	struct qt602240_message *message = data->object_message;

	dev_dbg(dev, "reportid:\t0x%x\n", message->reportid);
	dev_dbg(dev, "message1:\t0x%x\n", message->message[0]);
	dev_dbg(dev, "message2:\t0x%x\n", message->message[1]);
	dev_dbg(dev, "message3:\t0x%x\n", message->message[2]);
	dev_dbg(dev, "message4:\t0x%x\n", message->message[3]);
	dev_dbg(dev, "message5:\t0x%x\n", message->message[4]);
	dev_dbg(dev, "message6:\t0x%x\n", message->message[5]);
	dev_dbg(dev, "message7:\t0x%x\n", message->message[6]);
	dev_dbg(dev, "checksum:\t0x%x\n", message->checksum);
}

static int qt602240_check_bootloader(struct i2c_client *client,
		unsigned int state)
{
	u8 val;
	static int error_count = 0;

recheck:
	if (i2c_master_recv(client, &val, 1) != 1) 
	{
		dev_err(&client->dev, "i2c recv failed\n");
		return -EIO;
	}

	switch (state) {
	case QT602240_WAITING_BOOTLOAD_CMD:
	case QT602240_WAITING_FRAME_DATA:
		val &= ~QT602240_BOOT_STATUS_MASK;
		break;
	case QT602240_FRAME_CRC_PASS:
		if (val == QT602240_FRAME_CRC_CHECK)
			goto recheck;
		break;
	default:
		return -EINVAL;
	}
	
	if (val != state) 
	{
	    if(error_count>=10)
	    {
		dev_err(&client->dev, "Unvalid bootloader mode state\n");
		return -EINVAL;
           }
           else
           {
		error_count++;
		printk("[TSP] state : 0x%x, return val : 0x%d , error_count : %d\n", state, val, error_count );
		return 1;
	     }
	}

	return 0;
}

static int qt602240_unlock_bootloader(struct i2c_client *client)
{
	u8 buf[2];

	buf[0] = QT602240_UNLOCK_CMD_LSB;
	buf[1] = QT602240_UNLOCK_CMD_MSB;

	if (i2c_master_send(client, buf, 2) != 2) {
		dev_err(&client->dev, "i2c send failed\n");
		return -EIO;
	}

	return 0;
}

static int qt602240_fw_write(struct i2c_client *client, const u8 *data,
		unsigned int frame_size)
{
	if (i2c_master_send(client, data, frame_size) != frame_size) {
		dev_err(&client->dev, "i2c send failed\n");
		return -EIO;
	}

	return 0;
}

static int qt602240_read_reg(struct i2c_client *client, u16 reg)
{
	u8 buf[2];
	u8 val;

	buf[0] = reg & 0xff;
	buf[1] = (reg >> 8) & 0xff;

	if (i2c_master_send(client, buf, 2) != 2) {
		dev_err(&client->dev, "i2c send failed\n");
		return -EIO;
	}

	if (i2c_master_recv(client, &val, 1) != 1) {
		dev_err(&client->dev, "i2c recv failed\n");
		return -EIO;
	}

	return val;
}

static int qt602240_write_reg(struct i2c_client *client, u16 reg, u8 val)
{
	u8 buf[3];

	buf[0] = reg & 0xff;
	buf[1] = (reg >> 8) & 0xff;
	buf[2] = val;

	if (i2c_master_send(client, buf, 3) != 3) {
		dev_err(&client->dev, "i2c send failed\n");
		return -EIO;
	}

	return 0;
}

static int qt602240_read_object_table(struct i2c_client *client, u16 reg,
		u8 *object_buf)
{
	u8 buf[2];

	buf[0] = reg & 0xff;
	buf[1] = (reg >> 8) & 0xff;

	if (i2c_master_send(client, buf, 2) != 2) {
		dev_err(&client->dev, "i2c send failed\n");
		return -EIO;
	}

	if (i2c_master_recv(client, object_buf, 6) != 6) {
		dev_err(&client->dev, "i2c recv failed\n");
		return -EIO;
	}

	return 0;
}

static int qt602240_read_message(struct qt602240_data *data)
{
	struct i2c_client *client = data->client;
	struct qt602240_object *object;
	u16 reg;
	u8 buf[2];

	object = qt602240_get_object(data, QT602240_GEN_MESSAGE);
	if (!object)
		return -EINVAL;

	reg = object->start_address;

	buf[0] = reg & 0xff;
	buf[1] = (reg >> 8) & 0xff;

	if (i2c_master_send(client, buf, 2) != 2) {
		dev_err(&client->dev, "i2c send failed\n");
		return -EIO;
	}

	if (i2c_master_recv(client, (u8 *)data->object_message, 9) != 9) {
		dev_err(&client->dev, "i2c recv failed\n");
		return -EIO;
	}

	return 0;
}

static int qt602240_read_object(struct qt602240_data *data, u8 type, u8 offset)
{
	struct qt602240_object *object;
	u16 reg;

	object = qt602240_get_object(data, type);
	if (!object)
		return -EINVAL;

	reg = object->start_address;

	return qt602240_read_reg(data->client, reg + offset);
}

static int qt602240_write_object(struct qt602240_data *data, u8 type,
		u8 offset, u8 val)
{
	struct qt602240_object *object;
	u16 reg;

	object = qt602240_get_object(data, type);
	if (!object)
	{
	    printk("[TSP] error : 0x%x\n", object->type);
		return -EINVAL;
	}
  
	reg = object->start_address;
	return qt602240_write_reg(data->client, reg + offset, val);
}

static void qt602240_input_read(struct qt602240_data *data)
{
    struct qt602240_message *message = data->object_message;
    struct qt602240_object *object;
    struct input_dev *input_dev = data->input_dev;
    u8 reportid;
    u8 touch_status = 0;
    u8 id, size;
    int i;
    int bChangeUpDn= 0;
    int x = 0, y = 0;
    int iled= 0;
    static int nPrevID= -1;

    do{
    
        bChangeUpDn = 0;
		
        if (qt602240_read_message(data))
        {
            printk("[TSP] Couldn't read message\n");
            break;
        }                                                                                                                                                                              

        reportid = message->reportid;

#ifdef OAKWOOD
        if(( reportid >= 2 )&& ( reportid <= 17))
#else
        if(( reportid >= 2 )&& ( reportid <= 11 ))          //report ID 2~11 : Touch events
#endif  
        {
            id = message->reportid-2;
            
            /* whether reportid is thing of QT602240_TOUCH_MULTI */
            object = qt602240_get_object(data, QT602240_TOUCH_MULTI);            
            if (!object)
            {
                printk("[TSP] Couldn't get the object\n");
                break;
            }

            touch_status = message->message[0];                             //Message[0] : Touch status
            
            x = (message->message[1] << 2) |                                 //Message[1] : x position MSByte
                  ((message->message[3] & ~0x3f) >> 6);                   //Message[3] : x position LSBits , bit4 ~ 7
                            
            y = (message->message[2] << 2) |                                 //Message[2] : y position MSByte
        	     ((message->message[3] & ~0xf3) >> 2);                  //Message[3] : y position LSBits , bit0~3       			

            size = message->message[4];

            if((touch_status & 0xf0 ) == 0xc0)                                  // Detect & Press  : 0x80 | 0x40
            {
                fingerInfo[id].pressure= message->message[5];
                fingerInfo[id].x= (int16_t)x;
                fingerInfo[id].y= (int16_t)y;
                bChangeUpDn= 1;
  //              printk("[TSP] Finger[%d] Down  (%d,%d) size : %d \n", id, fingerInfo[id].x, fingerInfo[id].y, size);
            }
            else if ((touch_status & 0xf0 ) == 0x90 )	                      // Detect & Move : 0x80 | 0x10
            {
                fingerInfo[id].pressure= message->message[5];
                fingerInfo[id].x= (int16_t)x;
                fingerInfo[id].y= (int16_t)y;
                //printk("[TSP] Finger[%d] Move (%d,%d)!\n", id, fingerInfo[id].x, fingerInfo[id].y );
            }
            else if ( touch_status & 0x20 )                                         // Release : 0x20
            {
                fingerInfo[id].pressure= 0;
                bChangeUpDn= 1;
//                printk("[TSP] Finger[%d] Up    (%d,%d) size : %d\n", id, fingerInfo[id].x, fingerInfo[id].y, size);
            }
            else if( touch_status == 0x84)                                         // amplitude chage
            {
                fingerInfo[id].pressure= message->message[5];
                //printk("[TSP] Finger[%d] amplitude is moidifed. AMP : %d\n ", id, fingerInfo[id].pressure );
            }
            else
            {
                printk("[TSP] Unknown state(%x)! \n", touch_status);
            }
            
            fingerInfo[id].id = (id <<8)|size;

            if ( nPrevID >= id || bChangeUpDn )
            {

                for ( i= 0; i<MAX_USING_FINGER_NUM; ++i )
                {
                    if ( fingerInfo[i].pressure == -1 ) 
                        continue;

                    input_report_abs(input_dev, ABS_MT_POSITION_X, fingerInfo[i].x);
                    input_report_abs(input_dev, ABS_MT_POSITION_Y, fingerInfo[i].y);
                    input_report_abs(input_dev, ABS_MT_TOUCH_MAJOR, fingerInfo[i].pressure);
                    input_report_abs(input_dev, ABS_MT_WIDTH_MAJOR, fingerInfo[i].id);
                    input_mt_sync(input_dev);

                    if ( fingerInfo[i].pressure == 0 ) 
                        fingerInfo[i].pressure= -1;
                }

                input_sync(input_dev);
                DEBUG_MSG("[TSP] Multi-Touch Event[%d] Done!\n", id);
            }

            nPrevID= id;

        }
#ifdef OAKWOOD
        else if(reportid == 18)
#else
        else if((reportid == 12)||(reportid == 13))                                                         // Report ID 12,13 : Key event
#endif  
        {

            /* whether reportid is thing of QT602240_TOUCH_KEYARRAY */
            object = qt602240_get_object(data, QT602240_TOUCH_KEYARRAY);            
            if (!object)
            {
                printk("[TSP] Couldn't get the object\n");
                break;
            }

            for(i = 0; i <NUMOFKEYS; i++ )
            {                
                if(tsp_keystatus[i])
                {
#ifdef OAKWOOD                
                    input_report_key(input_dev, tsp_keycodes[3-i], 0);
#else
                    input_report_key(input_dev, tsp_keycodes[i], 0);
#endif                    
                    printk("[TSP] %s key is release. Keycode : %d\n", tsp_keyname[i], tsp_keycodes[i]);
                    tsp_keystatus[i] = KEY_RELEASE;
                }
                else if(message->message[1] & (0x1<<i) )
                {
                    if(message->message[0] & 0x80)                                  // detect
                    {
#ifdef OAKWOOD
                        input_report_key(input_dev, tsp_keycodes[3-i], 1);
                        touh_led_on(i+1);
#else
                        input_report_key(input_dev, tsp_keycodes[i], 1);
                        if(HWREV >= 0x0)
                        {
                            touh_led_on(4-i);
                        }
#endif                
                        printk("[TSP] %s key is pressed. Keycode : %d\n", tsp_keyname[i], tsp_keycodes[i]);
                        tsp_keystatus[i] = KEY_PRESS;
                    }
                }
  
                if(HWREV >= 0x0)
                {
                    if(tsp_keystatus[i] == KEY_RELEASE)
                    {
                        iled++;
                    }
                    if(iled == 4)
                    {
                        mod_timer(&g_led_timer, jiffies + HZ*(2/10));
                    }
                }

            }            
        }
#ifndef OAKWOOD
        else if( reportid == 14 )      	//20102017 julia
        {
            if((touch_status&0x01) == 0x00)   
            { 
                printk("[TSP] Palm Touch! - %d released. Ver=%x\n", touch_status, data->info->version );	
            }
            else
            {
                printk("[TSP] Palm Touch! - %d suppressed\n", touch_status);	
            }
        } 
#endif  

        qt602240_dump_message(data);

    } while (reportid != 0xff);

}

static void qt602240_irq_worker(struct work_struct *work)
{
	struct qt602240_data *data = container_of(work,
			struct qt602240_data, ts_event_work);

	qt602240_input_read(data);
}

static irqreturn_t qt602240_interrupt(int irq, void *dev_id)
{
	struct qt602240_data *data = dev_id;

	if (!work_pending(&data->ts_event_work))
		schedule_work(&data->ts_event_work);

	return IRQ_HANDLED;
}

void qt602240_reg_init(struct qt602240_data *data)
{
    int ret;
    
    ret = QT602240_Command_Config_Init(data);
    if(ret<0)
        printk("[TSP] fail to initialize the Command config\n");
        
    ret = QT602240_Powr_Config_Init(data);
    if(ret<0)
        printk("[TSP] fail to initialize the Power config\n");
        
    ret = QT602240_Acquisition_Config_Init(data);
    if(ret<0)
        printk("[TSP] fail to initialize the Acqusition config\n");
        
    ret = QT602240_Multitouch_Config_Init(data);
    if(ret<0)
        printk("[TSP] fail to initialize the Multi-touch config\n");
        
    ret = QT602240_KeyArrary_Config_Init(data);
    if(ret<0)
        printk("[TSP] fail to initialize the KeyArrary config\n"); 
        
#ifndef OAKWOOD
    ret = QT602240_Grip_Face_Suppression_Config_Init(data);
    if(ret<0)
        printk("[TSP] fail to initialize the suppression config\n");
#endif

    ret = QT602240_Noise_Config_Init(data);
    if(ret<0)
        printk("[TSP] fail to initialize the Noise config\n");
        
#if 0
    ret = QT602240_One_Touch_Gesture_Config_Init(data);
    if(ret<0)
        printk("[TSP] fail to initialize the 1 Gesture config\n");

    ret = QT602240_GPIOPWM_Config_Init(data);
    if(ret<0)
        printk("[TSP] fail to initialize the GPIO/PWM config\n");

    ret = QT602240_Proximity_Config_Init(data);        
    if(ret<0)
        printk("[TSP] fail to initialize the Proximity config\n");
        
    ret = QT602240_Selftest_Config_Init(data);
    if(ret<0)
        printk("[TSP] fail to initialize the selftest config\n");
        
    ret = QT602240_Two_touch_Gesture_Config_Init(data);
    if(ret<0)
        printk("[TSP] fail to initialize the 2 gesture config\n");
#endif

    ret = QT602240_CTE_Config_Init(data);    
    if(ret<0)
        printk("[TSP] fail to initialize the CTE config\n");
        
}

void qt602240_check_matrix_size(struct qt602240_data *data)
{
    u8 mode;
    mode = touchscreen_config.xsize - 16;
    if( mode != cte_config.mode)
        qt602240_write_object(data, QT602240_SPT_CTECONFIG, QT602240_CTE_MODE, mode);
}

static int qt602240_info_get(struct qt602240_data *data)
{
	struct i2c_client *client = data->client;
	struct qt602240_info *info = data->info;
	int val;

	val = qt602240_read_reg(client, QT602240_FAMILY_ID);
	if (val < 0)
		return -EIO;
	info->family_id = val;

	val = qt602240_read_reg(client, QT602240_VARIANT_ID);
	if (val < 0)
		return -EIO;
	info->variant_id = val;

	val = qt602240_read_reg(client, QT602240_VERSION);
	if (val < 0)
		return -EIO;
	info->version = val;

	val = qt602240_read_reg(client, QT602240_BUILD);
	if (val < 0)
		return -EIO;
	info->build = val;

	val = qt602240_read_reg(client, QT602240_MATRIX_X_SIZE);
	if (val < 0)
		return -EIO;
	info->matrix_xsize = val;

	val = qt602240_read_reg(client, QT602240_MATRIX_Y_SIZE);
	if (val < 0)
		return -EIO;
	info->matrix_ysize = val;

	val = qt602240_read_reg(client, QT602240_OBJECT_NUM);
	if (val < 0)
		return -EIO;
	info->object_num = val;

	return 0;
}

static int qt602240_initialize(struct qt602240_data *data)
{
	struct i2c_client *client = data->client;
	struct qt602240_info *info;
	int i;
	int ret;
	u16 reg;
	u8 buf[QT602240_OBJECT_SIZE];
	u8 reportid = 0;

	info = data->info = kzalloc(sizeof(struct qt602240_info), GFP_KERNEL);

	if (!data->info) {
		dev_err(&data->client->dev, "Failed to allocate memory\n");
		return -ENOMEM;
	}

	ret = qt602240_info_get(data);
	if (ret < 0)
	{
	    dev_err( &data->client->dev, "[TSP] Failed to get TSP info. \n");
	    return ret;
	}
	else
	{
            printk( "[TSP] F/W version    : %d\n", data->info->version );
            printk( "[TSP] Family  ID     : %d\n", data->info->family_id);
            printk( "[TSP] Variant ID     : %d\n", data->info->variant_id);            
            printk( "[TSP] Build number   : %d\n", data->info->build);            
            printk( "[TSP] X size         : %d\n", data->info->matrix_xsize);
            printk( "[TSP] Y size         : %d\n", data->info->matrix_ysize);
        }

	data->object_table =
		kzalloc(sizeof(struct qt602240_data) * data->info->object_num,
				GFP_KERNEL);
	data->object_message =
		kzalloc(sizeof(struct qt602240_message), GFP_KERNEL);
	if (!data->object_table || !data->object_message) {
		dev_err(&data->client->dev, "Failed to allocate memory\n");
		return -ENOMEM;
	}

	/* get object table information */
	for (i = 0; i < data->info->object_num; i++) {
		struct qt602240_object *object = data->object_table + i;

		reg = QT602240_OBJECT_START + QT602240_OBJECT_SIZE * i;
		ret = qt602240_read_object_table(client, reg, buf);
		if (ret)
			return ret;

		object->type = buf[0];
		object->start_address = (buf[2] << 8) | buf[1];
		object->size = buf[3];
		object->instances = buf[4];
		object->num_report_ids = buf[5];

		if (object->num_report_ids) {
			reportid += object->num_report_ids *
				(object->instances + 1);
			object->max_reportid = reportid;
		}
	}

        /* Init qt602240 reg*/
        qt602240_reg_init(data);

#ifndef OAKWOOD
	/* check X/Y matrix size */
	qt602240_check_matrix_size(data);
#endif	

	/* read dummy message to make high CHG pin */
	do {
		ret = qt602240_read_object(data, QT602240_GEN_MESSAGE, 0);
		if (ret < 0)
			return ret;
	} while (ret != 0xff);
	

	/* backup to memory */
	qt602240_write_object(data, QT602240_GEN_COMMAND,
			QT602240_COMMAND_BACKUPNV, QT602240_BACKUP_VALUE);
	/* soft reset */
	qt602240_write_object(data, QT602240_GEN_COMMAND,
			QT602240_COMMAND_RESET, 1);

#ifdef OAKWOOD
	mdelay(200);
#else
        mdelay(100);
#endif

	dev_info(&client->dev,
			"Family ID: %d Variant ID: %d Version: %d Build: %d\n",
			info->family_id, info->variant_id, info->version,
			info->build);

	dev_info(&client->dev,
			"Matrix X Size: %d Matrix Y Size: %d Object Num: %d\n",
			info->matrix_xsize, info->matrix_ysize,
			info->object_num);

	return 0;
}

static ssize_t qt602240_info_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct qt602240_data *data = dev_get_drvdata(dev);
	struct qt602240_info *info = data->info;
	int count = 0;

	count += sprintf(buf + count, "Family ID:\t0x%x (%d)\n",
			info->family_id, info->family_id);
	count += sprintf(buf + count, "Variant ID:\t0x%x (%d)\n",
			info->variant_id, info->variant_id);
	count += sprintf(buf + count, "Version:\t0x%x (%d)\n",
			info->version, info->version);
	count += sprintf(buf + count, "Build:\t\t0x%x (%d)\n",
			info->build, info->build);
	count += sprintf(buf + count, "Matrix X Size:\t0x%x (%d)\n",
			info->matrix_xsize, info->matrix_xsize);
	count += sprintf(buf + count, "Matrix Y Size:\t0x%x (%d)\n",
			info->matrix_ysize, info->matrix_ysize);
	count += sprintf(buf + count, "Object Num:\t0x%x (%d)\n",
			info->object_num, info->object_num);

	return count;
}

static ssize_t qt602240_object_table_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct qt602240_data *data = dev_get_drvdata(dev);
	struct qt602240_object *object;
	int count = 0;
	int i;

	for (i = 0; i < data->info->object_num; i++) {
		object = data->object_table + i;

		count += sprintf(buf + count,
				"Object Table Element %d\n", i + 1);
		count += sprintf(buf + count, "  type:\t\t\t0x%x (%d)\n",
				object->type, object->type);
		count += sprintf(buf + count, "  start_address:\t0x%x (%d)\n",
				object->start_address, object->start_address);
		count += sprintf(buf + count, "  size:\t\t\t0x%x (%d)\n",
				object->size, object->size);
		count += sprintf(buf + count, "  instances:\t\t0x%x (%d)\n",
				object->instances, object->instances);
		count += sprintf(buf + count, "  num_report_ids:\t0x%x (%d)\n",
				object->num_report_ids, object->num_report_ids);
		count += sprintf(buf + count, "  max_reportid:\t\t0x%x (%d)\n",
				object->max_reportid, object->max_reportid);
		count += sprintf(buf + count, "\n");
	}

	return count;
}

static ssize_t qt602240_object_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct qt602240_data *data = dev_get_drvdata(dev);
	struct qt602240_object *object;
	int val;
	int count = 0;
	int i, j;

	for (i = 0; i < data->info->object_num; i++) {
		object = data->object_table + i;

		count += sprintf(buf + count,
				"Object Table Element %d(Type %d)\n",
				i + 1, object->type);

		if (!qt602240_object_readable(object->type)) {
			count += sprintf(buf + count, "\n");
			continue;
		}

		for (j = 0; j < object->size + 1; j++) {
			val = qt602240_read_object(data, object->type, j);
			if (val < 0)
				return count;

			count += sprintf(buf + count,
					"  Byte %d: 0x%x (%d)\n", j, val, val);
		}

		count += sprintf(buf + count, "\n");
	}

	return count;
}

static ssize_t qt602240_object_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct qt602240_data *data = dev_get_drvdata(dev);
	struct qt602240_object *object;
	unsigned int type;
	unsigned int offset;
	unsigned int val;
	int ret;

	if (sscanf(buf, "%u %u %u", &type, &offset, &val) != 3) {
		dev_err(dev, "Invalid values\n");
		return -EINVAL;
	}

	object = qt602240_get_object(data, type);
	if (!object || (offset > object->size)) {
		dev_err(dev, "Invalid values\n");
		return -EINVAL;
	}

	ret = qt602240_write_object(data, type, offset, val);
	if (ret < 0)
		return ret;

	return count;
}

static int qt602240_load_fw(struct device *dev, unsigned int version)
{
	struct qt602240_data *data = dev_get_drvdata(dev);
	const struct firmware *fw = NULL;
	unsigned int frame_size;
	unsigned int pos = 0;
	int ret;
	unsigned char *pFW_data;

	if(version == QT602240_VER_22)
	{
        	ret = request_firmware(&fw, QT602240_FW_NAME, dev);
        	if (ret < 0) {
        		dev_err(dev, "Unable to open firmware %s\n", QT602240_FW_NAME);
        		ret = -EIO;
        		goto err_fw;
        	}
	}
	else
	{
            struct file* filp;
            loff_t         fw_size;
            
            filp = filp_open(FW_PATH, O_RDONLY, 0);
            if (IS_ERR(filp))
            {
                printk(KERN_INFO "Unable to load '%s'.\n", FW_PATH);
                ret = -ENOENT;
                goto err_fw;
            }

            fw_size = filp->f_path.dentry->d_inode->i_size;

            pFW_data = vmalloc(fw_size);
            if (pFW_data == NULL)
            {
                printk(KERN_INFO "Out of memory loading '%s'.\n", FW_PATH);
                filp_close(filp, current->files);
                ret = -ENOMEM;
                goto err_fw;
            }

            pos = 0;
            if (vfs_read(filp, pFW_data, fw_size, &pos) != fw_size)
            {
                printk(KERN_INFO "Failed to read '%s'.\n", FW_PATH);
                vfree(pFW_data);
                filp_close(filp, current->files);
                ret = -EIO;
                goto err_fw;
            }
            filp_close(filp, current->files);

	}

	/* change to the bootloader mode */
	qt602240_write_object(data, QT602240_GEN_COMMAND,
			QT602240_COMMAND_RESET, QT602240_BOOT_VALUE);
	mdelay(100);

	/* change to slave address of bootloader */
	if (data->client->addr == QT602240_APP_LOW)
		data->client->addr = QT602240_BOOT_LOW;
	else
		data->client->addr = QT602240_BOOT_HIGH;

	ret = qt602240_check_bootloader(data->client,
			QT602240_WAITING_BOOTLOAD_CMD);
	if (ret < 0)
		goto err_fw;

	/* unlock bootloader */
	qt602240_unlock_bootloader(data->client);
	mdelay(100);

	while (pos < fw->size ) 
	{
		ret = qt602240_check_bootloader(data->client,
				QT602240_WAITING_FRAME_DATA);
		if (ret < 0)
			goto err_fw;

		frame_size = ((*(fw->data + pos) << 8) | *(fw->data + pos + 1));

		/* We should add 2 at frame size as the firmware data is not
		 * included the CRC bytes.
		 */
		frame_size += 2;

		printk("[TSP] frame_size : 0x%x\n", frame_size);	

		/* write one frame to device */
		qt602240_fw_write(data->client, fw->data + pos, frame_size);

		ret = qt602240_check_bootloader(data->client,
				QT602240_FRAME_CRC_PASS);
		if (ret < 0)
			goto err_fw;
		else if( ret == 0)
		    pos += frame_size;

		dev_info(dev, "Updated %zd bytes / %zd bytes\n", pos, fw->size);
	}

err_fw:
	/* change to slave address of application */
	if (data->client->addr == QT602240_BOOT_LOW)
		data->client->addr = QT602240_APP_LOW;
	else
		data->client->addr = QT602240_APP_HIGH;

	return ret;
}

static ssize_t qt602240_update_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{	
	return sprintf(buf, "%d\n", gFirmware_Update_State);
}

static ssize_t qt602240_update_fw_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
    unsigned int version;
    int ret;

    if (sscanf(buf, "%u", &version) != 1) {
    	dev_err(dev, "Invalid values\n");
    	return -EINVAL;
    }

    printk("[TSP] F/W  downloading...\n");
    gFirmware_Update_State = FW_UPDATE_DOWNLOADING;

    ret = qt602240_load_fw(dev, version);
    if (ret < 0)
    {
        gFirmware_Update_State = FW_UPDATE_FAIL;
    	dev_err(dev, "The firmware update fail\n");
    }
    else
    {
        gFirmware_Update_State = FW_UPDATE_DONE;
    	dev_info(dev, "The firmware update success\n");
    }

    return count;
}

void qt602240_set_amoled_display(int mode)
{
	set_mode_for_amoled = mode;
}
EXPORT_SYMBOL(qt602240_set_amoled_display);

// mode 1 = TA
// mdoe 0 = TA
static int qt602240_set_for_TA(int mode, struct qt602240_data *data)
{
	unsigned int type = QT602240_TOUCH_MULTI;
	unsigned int offset = QT602240_TOUCH_TCHTHR;
	unsigned int val;
	int ret;	
	int threshold;

	if(mode)
	{
		threshold = 30;
	}
	else
	{
		threshold = 25;
	}

	val = threshold;

	printk("[TSP] %s threshold:%d\n", __func__, val);

	ret = qt602240_write_object(data, type, offset, val);
	if (ret < 0)
	{
		printk("[TSP] error %s: write_object\n", __func__);
	}
	
	return 0;

}

// mode 1 = Charger connected
// mode 0 = Charger disconnected
void qt602240_inform_charger_connection(int mode)
{
	// should be changed to use workqueue

	printk("[TSP] start %s :%d\n", __func__, mode);
	
	if(mode)
	{
		set_mode_for_ta	= 1;
	}
	else
	{
		set_mode_for_ta = 0;
	}

	if(NULL == p_qt602240_data)
	{
		printk("[TSP] %s touch device not prepared yet\n", __func__);
		return;
	}

	if(config_mode_val)
	{
		// stylus mode
		qt602240_set_for_TA(set_mode_for_ta, p_qt602240_data);
	}

}
EXPORT_SYMBOL(qt602240_inform_charger_connection);

static void qt602240_change_config_for_stylus(struct qt602240_data *data, int mode)
{
    if(mode)
    {
        // stylus mode
        qt602240_write_object(data, 9, 7, 13);		       // threshold
        qt602240_write_object(data, 9, 11, 0x01 );		// movhsti
        qt602240_write_object(data, 9, 13, 0xf);		//movfilter
        qt602240_write_object(data, 22, 0, 0);              //disable
//        qt602240_write_object(data, 28, 3, 8);              //idlegcafdepth
        qt602240_write_object(data, 28, 4, 16);            //activegcafdepth
    }
    else
    {
        // normal mode
        qt602240_write_object(data, 9, 7, 25);		       // threshold
        qt602240_write_object(data, 9, 11, 0xa );		// movhsti
        qt602240_write_object(data, 9, 13, 0x4b);		//movfilter
        qt602240_write_object(data, 22, 0, 0xd);           //Median filter, Frequency hopping
//        qt602240_write_object(data, 28, 3, 8);              //idlegcafdepth
        qt602240_write_object(data, 28, 4, 32);            //activegcafdepth
    }
}

#if 0
static int qt602240_config_settings(unsigned char* setting_list, struct qt602240_data *data, int mode)
{
	struct qt602240_object *object;
	int i,j;
	int ret;
	int index = 0;

	for (i = 0; i < data->info->object_num; i++) {
		object = data->object_table + i;

		if (!qt602240_object_writable(object->type))
			continue;

		for (j = 0; j < object->size + 1; j++)
			qt602240_write_object(data, object->type, j,
					setting_list[index + j]);

		index += object->size + 1;
	}

	/* check X/Y matrix size */
	qt602240_check_matrix_size(data);

	/* read dummy message to make high CHG pin */
	do {
		ret = qt602240_read_object(data, QT602240_GEN_MESSAGE, 0);
		if (ret < 0)
			return ret;
	} while (ret != 0xff);

//	if(mode)
//		qt602240_set_for_TA(set_mode_for_ta, data);

	/* touch enable */
	qt602240_write_object(data, QT602240_TOUCH_MULTI, QT602240_TOUCH_CTRL, 0x8f);

}
#endif

static ssize_t qt602240_config_mode_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
    if(config_mode_val)
    {
        printk("[TSP] config: stylus mode\n");
    }
    else
    {
        printk("[TSP] config: normal mode\n");
    }

    return snprintf(buf, PAGE_SIZE, "%d\n", config_mode_val);
}

static ssize_t qt602240_config_mode_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
    unsigned int mode;
    struct qt602240_data *data = dev_get_drvdata(dev);

    if(*buf == 0x00)
    {
        mode = 0;
    }
    else if(*buf == 0x01)
    {
        mode = 1;
    }
    else if (sscanf(buf, "%u", &mode) != 1) 
    {
        dev_err(dev, "Invalid values(0x%x)\n", *buf);
        return -EINVAL;
    }

    switch(mode)
    {
        case 0:
            printk("[TSP] set normal mode\n");
            
            if(mode != config_mode_val)
            {                
                qt602240_change_config_for_stylus(data, mode);
            }

            config_mode_val = mode;
            break;
        
        case 1:
            printk("[TSP] set stylus mode\n");
            
            if(mode != config_mode_val)
            {
                qt602240_change_config_for_stylus(data, mode);
                qt602240_set_for_TA(set_mode_for_ta, data);
            }
            
            config_mode_val = mode;
            break;
        
        default:
            printk("[TSP] %s : invalid mode\n", __func__);
            break;
    }

    return count;
}

static DEVICE_ATTR(info, 0444, qt602240_info_show, NULL);
static DEVICE_ATTR(object_table, 0444, qt602240_object_table_show, NULL);
static DEVICE_ATTR(object, 0664, qt602240_object_show, qt602240_object_store);
static DEVICE_ATTR(update_fw, 0664, NULL, qt602240_update_fw_store);
static DEVICE_ATTR(update_status, 0664, qt602240_update_status_show, NULL);
static DEVICE_ATTR(config_mode, 0666, qt602240_config_mode_show, qt602240_config_mode_store);

static struct attribute *qt602240_attrs[] = {
	&dev_attr_info.attr,
	&dev_attr_object_table.attr,
	&dev_attr_object.attr,
	&dev_attr_update_fw.attr,
	&dev_attr_update_status.attr,
	&dev_attr_config_mode.attr,
	NULL
};

static const struct attribute_group qt602240_attr_group = {
	.attrs = qt602240_attrs,
};

extern int mDNIe_tuning_load_from_file(void);

static ssize_t firmware1_show(struct device *dev, struct device_attribute *attr, char *buf)
{
#ifndef MDNIE_TUNING
	u8 version = p_qt602240_data->info->version;
	u8 build = p_qt602240_data->info->build;
	printk("[TSP] Firmware %x %x\n", version, build);

	return sprintf(buf, "%x.%x\n", version>>4, version&0xf);
#else
	int ret;

	ret = mDNIe_tuning_load_from_file();
	printk("[TSP] ret = %d\n", ret);

	if(ret<0)
		return sprintf(buf, "FAIL\n");
	else
		return sprintf(buf, "OK (%d data)\n", ret);
#endif
}

static ssize_t firmware1_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	return size;
}

static ssize_t key_threshold_show(struct device *dev, struct device_attribute *attr, char *buf)
{	
	return sprintf(buf, "%d\n", touchscreen_config.tchthr);
}

static ssize_t key_threshold_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
    int i;
    if(sscanf(buf,"%d",&i)==1)
    {
        touchscreen_config.tchthr = i;	//basically,40;
        printk("[TSP] threshold is changed to %d\n",i);
    }
    else
        printk("[TSP] threshold write error\n");

    return size;
}

static DEVICE_ATTR(firmware1, S_IRUGO | S_IWUSR, firmware1_show, firmware1_store);
static DEVICE_ATTR(key_threshold, S_IRUGO | S_IWUSR, key_threshold_show, key_threshold_store);

extern struct class *sec_class;
struct device *ts_dev;

static int __devinit qt602240_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	struct qt602240_data *data;
	struct input_dev *input_dev;
	int ret;
	int i;

	if (!client->dev.platform_data)
		return -EINVAL;

	data = kzalloc(sizeof(struct qt602240_data), GFP_KERNEL);
	input_dev = input_allocate_device();
	if (!data || !input_dev) {
		dev_err(&client->dev, "Failed to allocate memory\n");
		ret = -ENOMEM;
		goto err_free_mem;
	}

	INIT_WORK(&data->ts_event_work, qt602240_irq_worker);
	data->client = client;
	data->input_dev = input_dev;
	data->pdata = client->dev.platform_data;
	data->irq = client->irq;

	input_dev->name = "AT42QT602240 Touchscreen";
	input_dev->id.bustype = BUS_I2C;
	input_dev->dev.parent = &client->dev;

	set_bit(EV_SYN, input_dev->evbit);
	set_bit(EV_KEY, input_dev->evbit);
	set_bit(BTN_TOUCH, input_dev->keybit);
	set_bit(EV_ABS, input_dev->evbit);

	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0, QT602240_MAX_XC, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0, QT602240_MAX_YC, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_WIDTH_MAJOR, 0, 15, 0, 0);

        if(HWREV >= 0)
        {        
            input_dev->keycode = tsp_keycodes;
            
            for(i = 0; i < NUMOFKEYS; i++)
            {
                input_set_capability(input_dev, EV_KEY, ((int*)input_dev->keycode)[i]);
                tsp_keystatus[i] = KEY_RELEASE;
            }            
        }	    	

	input_set_drvdata(input_dev, data);

	/* for multi-touch */
	for (i=0; i<MAX_USING_FINGER_NUM ; i++)		// touchscreen_config.numtouch is 5
		fingerInfo[i].pressure = -1;

	ret = qt602240_initialize(data);
	if (ret < 0)
		goto err_free_object;

	ret = request_irq(IRQ_TOUCH_INT, qt602240_interrupt, IRQF_TRIGGER_FALLING,
			client->dev.driver->name, data);

//	set_irq_type(IRQ_TOUCH_INT, IRQ_TYPE_LEVEL_LOW);
	s3c_gpio_cfgpin(GPIO_TOUCH_INT, S3C_GPIO_SFN(0xf));
	if (ret < 0) {
		dev_err(&client->dev, "Failed to register interrupt\n");
		goto err_free_object;
	}

	ret = input_register_device(input_dev);
	if (ret < 0)
		goto err_free_irq;

	ret = sysfs_create_group(&client->dev.kobj, &qt602240_attr_group);
	if (ret)
		goto err_free_irq;

	i2c_set_clientdata(client, data);

	ts_dev = device_create(sec_class, NULL, 0, NULL, "ts");
	if (IS_ERR(ts_dev))
		pr_err("Failed to create device(ts)!\n");
		
	if (device_create_file(ts_dev, &dev_attr_firmware1) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_firmware1.attr.name);
		
	if (device_create_file(ts_dev, &dev_attr_key_threshold) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_key_threshold.attr.name);

#if 0		// temp. remove
	//moon: for checking Charger
	if(maxim_chg_status())
		set_mode_for_ta = 1;
	else
		set_mode_for_ta = 0;
#endif

        if(HWREV >=0x0)
        {
            init_led();
            init_timer(&g_led_timer);
            g_led_timer.function = TouchKey_Led_TimerProc;
        }

#ifdef CONFIG_HAS_EARLYSUSPEND
	p_qt602240_data = data;
	data->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	data->early_suspend.suspend = qt602240_early_suspend;
	data->early_suspend.resume = qt602240_late_resume;
	register_early_suspend(&data->early_suspend);
#endif	/* CONFIG_HAS_EARLYSUSPEND */

	return 0;

err_free_irq:
	free_irq(client->irq, data);
err_free_object:
	kfree(data->object_message);
	kfree(data->object_table);
	kfree(data->info);
err_free_mem:
	input_free_device(input_dev);
	kfree(data);
	return ret;
}

static int __devexit qt602240_remove(struct i2c_client *client)
{
	struct qt602240_data *data = i2c_get_clientdata(client);

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&data->early_suspend);
#endif	/* CONFIG_HAS_EARLYSUSPEND */

	free_irq(data->irq, data);
	cancel_work_sync(&data->ts_event_work);
	input_unregister_device(data->input_dev);
	kfree(data->object_message);
	kfree(data->object_table);
	kfree(data->info);
	kfree(data);

	sysfs_remove_group(&client->dev.kobj, &qt602240_attr_group);
	i2c_set_clientdata(client, NULL);

	return 0;
}

#ifdef CONFIG_PM
static int qt602240_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct qt602240_data *data = i2c_get_clientdata(client);
	int i;

	/* touch disable */
	qt602240_write_object(data, QT602240_TOUCH_MULTI, QT602240_TOUCH_CTRL, 0);

	qt602240_write_object(data, QT602240_GEN_POWER, QT602240_POWER_IDLEACQINT, 0);
	qt602240_write_object(data, QT602240_GEN_POWER, QT602240_POWER_ACTVACQINT, 0);

	/* for multi-touch */
	for (i=0; i<MAX_USING_FINGER_NUM ; i++)
		fingerInfo[i].pressure = -1;

    if(HWREV >=0x0)
    {
        gpio_set_value(KEYLED_EN, 0);
    }
	return 0;
}

static int qt602240_resume(struct i2c_client *client)
{
	struct qt602240_data *data = i2c_get_clientdata(client);

	/* soft reset */
	qt602240_write_object(data, QT602240_GEN_COMMAND, QT602240_COMMAND_RESET, 1);

	if(HWREV >=0x0)
	{
        	init_led();
	}

#if 0	// temp. remove
	//moon: for checking Charger
	if(maxim_chg_status())
		set_mode_for_ta = 1;
	else
		set_mode_for_ta = 0;
#endif	
	return 0;
}
#else
#define qt602240_suspend	NULL
#define qt602240_resume		NULL
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
static void qt602240_early_suspend(struct early_suspend *h)
{
	qt602240_suspend(p_qt602240_data->client, PMSG_SUSPEND);
}

static void qt602240_late_resume(struct early_suspend *h)
{
	qt602240_resume(p_qt602240_data->client);
}
#endif	// End of CONFIG_HAS_EARLYSUSPEND

static const struct i2c_device_id qt602240_id[] = {
	{ "qt602240_ts", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, qt602240_id);

static struct i2c_driver qt602240_driver = {
	.driver = {
		.name = "qt602240_ts",
	},
	.probe		= qt602240_probe,
	.remove		= __devexit_p(qt602240_remove),
	.suspend	= qt602240_suspend,
	.resume		= qt602240_resume,
	.id_table	= qt602240_id,
};

static int __init qt602240_init(void)
{
	return i2c_add_driver(&qt602240_driver);
}

static void __exit qt602240_exit(void)
{
	i2c_del_driver(&qt602240_driver);
}

module_init(qt602240_init);
module_exit(qt602240_exit);

/* Module information */
MODULE_AUTHOR("Joonyoung Shim <jy0922.shim@samsung.com>");
MODULE_DESCRIPTION("AT42QT602240 Touchscreen driver");
MODULE_LICENSE("GPL");
