/*
 * linux/drivers/power/s3c6410_battery.c
 *
 * Battery measurement code for S3C6410 platform.
 *
 * based on palmtx_battery.c
 *
 * Copyright (C) 2009 Samsung Electronics.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/power_supply.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/irq.h>
#include <linux/wakelock.h>
#include <asm/mach-types.h>
#include <mach/hardware.h>
#include <mach/battery.h>
#include <plat/gpio-cfg.h>
#include <linux/earlysuspend.h>
#include <linux/io.h>
#include <mach/regs-clock.h>
#include <mach/regs-power.h>
#include <mach/map.h>
#include <mach/regs-gpio.h>
#include <mach/gpio.h>

#include "s5pc110_battery.h"


static struct wake_lock vbus_wake_lock;
static struct wake_lock low_batt_wake_lock;	// lobat pwroff

#ifdef CONFIG_WIRELESS_CHARGING
extern unsigned int HWREV;
#endif

extern int fg_init(void);
extern void fg_exit(void);
extern void fuel_gauge_rcomp(u8 msb, u8 lsb);
extern unsigned int fg_read_soc(void);
extern unsigned int fg_read_vcell(void);
extern unsigned int fg_reset_soc(void);
extern unsigned int fg_read_raw_vcell(void);
extern unsigned int fg_read_raw_soc(void);

/* Prototypes */
extern int s3c_adc_get_adc_data(int channel);
extern void MAX8998_IRQ_init(void);
extern void maxim_charging_control(unsigned int dev_type  , unsigned int cmd);
extern unsigned char maxim_chg_status(void);
extern unsigned char maxim_lpm_chg_status(void);
extern void lpm_mode_check(void);

extern void set_low_bat_interrupt(int on);	// lobat pwroff
extern void set_low_bat_interrupt_mask_enable(int on);

extern void charging_stop_without_magic_number(void);
extern void charging_start_without_magic_number(void);
extern u8 FSA9480_Get_JIG_Status(void);

#ifdef __VZW_AUTH_CHECK__
static int verizon_batt_auth_full_check(void);
static int verizon_batt_auth_check(void);
static int verizon_batt_auth_multi_check(void);

extern int Reset_TA(void);
extern int CRC_protection(void);
extern int rom_code_protection(void);
extern void BattAuth_Finish(void);

int batt_auth_check;
static int batt_auth_full_check = 0; 
static int auth_battery_enabled = 1;	// ignore battery auth check if hidden menu is enabled (0: ignore / 1: check)
#endif

#define LPM_MODE
static unsigned int s3c_bat_check_v_f(void);
static ssize_t s3c_bat_show_property(struct device *dev,
                                      struct device_attribute *attr,
                                      char *buf);
static ssize_t s3c_bat_store(struct device *dev, 
			     struct device_attribute *attr,
			     const char *buf, size_t count);
static void s3c_set_chg_en(int enable);
static void polling_timer_func(unsigned long unused);
static void s3c_bat_status_update(struct power_supply *bat_ps);

#ifdef __TEST_MODE_INTERFACE__
static struct power_supply *s3c_power_supplies_test = NULL;
#endif /* __TEST_MODE_INTERFACE__ */

static void use_browser_timer_func(unsigned long unused);
static void use_4g_timer_func(unsigned long unused);
static void use_data_call_timer_func(unsigned long unused);
static int s3c_bat_use_browser(int onoff);
static int s3c_bat_use_data_call(int onoff);
int s3c_bat_use_4g(int onoff);


#define USE_2G_CALL		(0x1 << 0)
#define USE_TALK_WCDMA		(0x1 << 0)
#define USE_VIDEO_PLAY		(0x1 << 1)
#define USE_MP3_PLAY		(0x1 << 2)
#define USE_BROWSER		(0x1 << 3)
#define USE_4G			(0x1 << 4)
#define USE_HOTSPOT		(0x1 << 5)
#define USE_CAMERA		(0x1 << 6)
#define USE_DATA_CALL		(0x1 << 7)

#define TRUE	1
#define FALSE	0

#define ADC_DATA_ARR_SIZE	6
#define ADC_TOTAL_COUNT		10
#define POLLING_INTERVAL	2000
#ifdef __TEST_MODE_INTERFACE__
#define POLLING_INTERVAL_TEST	1000
#endif /* __TEST_MODE_INTERFACE__ */

#define RCOMP_MSB_VALUE 0xD0
#define RCOMP_LSB_VALUE 0x00

static struct work_struct bat_work;
static struct work_struct cable_work;
static struct device *dev;
static struct timer_list polling_timer;

static struct timer_list use_4g_timer;
static struct timer_list use_browser_timer;
static struct timer_list use_data_call_timer;

static int s3c_battery_initial;
static int force_update;

static unsigned int start_time_msec;
static unsigned int total_time_msec;

static char *status_text[] = {
	[POWER_SUPPLY_STATUS_UNKNOWN] =		"Unknown",
	[POWER_SUPPLY_STATUS_CHARGING] =	"Charging",
	[POWER_SUPPLY_STATUS_DISCHARGING] =	"Discharging",
	[POWER_SUPPLY_STATUS_NOT_CHARGING] =	"Not Charging",
	[POWER_SUPPLY_STATUS_FULL] =		"Full",
};

typedef enum {
	CHARGER_BATTERY = 0,
	CHARGER_USB,
	CHARGER_AC,
	CHARGER_DISCHARGE
} charger_type_t;

struct battery_info {
	u32 batt_id;		/* Battery ID from ADC */
	s32 batt_vol;		/* Battery voltage from ADC */
	s32 batt_temp;		/* Battery Temperature (C) from ADC */
	s32 batt_temp_adc;	/* Battery Temperature ADC value */
	s32 batt_temp_adc_cal;	/* Battery Temperature ADC value (calibrated) */
	s32 batt_current;	/* Battery current from ADC */
	u32 level;		/* formula */
	u32 charging_source;	/* 0: no cable, 1:usb, 2:AC */
	u32 charging_enabled;	/* 0: Disable, 1: Enable */
	u32 batt_health;	/* Battery Health (Authority) */
	u32 batt_is_full;       /* 0 : Not full 1: Full */
	u32 batt_is_recharging; /* 0 : Not recharging 1: Recharging */
	u32 decimal_point_level;	// lobat pwroff
#ifdef CONFIG_WIRELESS_CHARGING
	u32 wc_status;	// wireless charging
#endif
#ifdef __TEST_MODE_INTERFACE__
	u32 batt_test_mode;	/* test mode */
	s32 batt_vol_aver;	/* batt vol average */
	s32 batt_temp_aver;	/* batt temp average */
	s32 batt_temp_adc_aver;	/* batt temp adc average */
#endif /* __TEST_MODE_INTERFACE__ */
};

/* lock to protect the battery info */
static DEFINE_MUTEX(work_lock);

struct s3c_battery_info {
	int present;
	int polling;
	unsigned int polling_interval;
	unsigned int device_state;

	struct battery_info bat_info;
#ifdef LPM_MODE
	unsigned int charging_mode_booting;
#endif
};
static struct s3c_battery_info s3c_bat_info;

#if 0
struct adc_sample_info {
	unsigned int cnt;
	int total_adc;
	int average_adc;
	int adc_arr[ADC_TOTAL_COUNT];
	int index;
};
static struct adc_sample_info adc_sample[ENDOFADC];
#endif

struct battery_driver 
{
	struct early_suspend	early_suspend;
};
struct battery_driver *battery = NULL;


extern charging_device_type curent_device_type;

#ifdef LPM_MODE
void charging_mode_set(unsigned int val)
{
	s3c_bat_info.charging_mode_booting=val;
}
unsigned int charging_mode_get(void)
{
	return s3c_bat_info.charging_mode_booting;
}
#endif

static int get_usb_power_state(void)
{
	if(curent_device_type==PM_CHARGER_USB_INSERT)
		return 1;
	else
		return 0;
}	

static inline int s3c_adc_get_adc_data_ex(int channel) {
	if (channel == S3C_ADC_TEMPERATURE)
		return ((4096 - s3c_adc_get_adc_data(channel)) >> 2);	// hanapark_Garnett (10-bit ADC; 2010.05.24)
	else
		return (s3c_adc_get_adc_data(channel) >> 2);
}

#if 0
static unsigned long calculate_average_adc(adc_channel_type channel, int adc)
{
	unsigned int cnt = 0;
	int total_adc = 0;
	int average_adc = 0;
	int index = 0;

	cnt = adc_sample[channel].cnt;
	total_adc = adc_sample[channel].total_adc;

	if (adc < 0 || adc == 0) {
		dev_err(dev, "%s: invalid adc : %d\n", __func__, adc);
		adc = adc_sample[channel].average_adc;
	}

	if( cnt < ADC_TOTAL_COUNT ) {
		adc_sample[channel].adc_arr[cnt] = adc;
		adc_sample[channel].index = cnt;
		adc_sample[channel].cnt = ++cnt;

		total_adc += adc;
		average_adc = total_adc / cnt;
	} else {
#if 0
		if (channel == S3C_ADC_VOLTAGE &&
				!s3c_bat_info.bat_info.charging_enabled && 
				adc > adc_sample[channel].average_adc) {
			dev_dbg(dev, "%s: adc over avg : %d\n", __func__, adc);
			return adc_sample[channel].average_adc;
		}
#endif
		index = adc_sample[channel].index;
		if (++index >= ADC_TOTAL_COUNT)
			index = 0;

		total_adc = (total_adc - adc_sample[channel].adc_arr[index]) + adc;
		average_adc = total_adc / ADC_TOTAL_COUNT;

		adc_sample[channel].adc_arr[index] = adc;
		adc_sample[channel].index = index;
	}

	adc_sample[channel].total_adc = total_adc;
	adc_sample[channel].average_adc = average_adc;

	dev_dbg(dev, "%s: ch:%d adc=%d, avg_adc=%d\n",
			__func__, channel, adc, average_adc);
	return average_adc;
}
#endif

static int s3c_bat_get_adc_data(adc_channel_type adc_ch)
{
	int adc_arr[ADC_DATA_ARR_SIZE];
	int adc_max = 0;
	int adc_min = 0;
	int adc_total = 0;
	int i;

	for (i = 0; i < ADC_DATA_ARR_SIZE; i++) {
		adc_arr[i] = s3c_adc_get_adc_data_ex(adc_ch);
		if ( i != 0) {
			if (adc_arr[i] > adc_max) 
				adc_max = adc_arr[i];
			else if (adc_arr[i] < adc_min)
				adc_min = adc_arr[i];
		} else {
			adc_max = adc_arr[0];
			adc_min = adc_arr[0];
		}
		adc_total += adc_arr[i];
	}

//	dev_dbg(dev, "%s: adc_max = %d, adc_min = %d\n",
//			__func__, adc_max, adc_min);

	return (adc_total - adc_max - adc_min) / (ADC_DATA_ARR_SIZE - 2);
}

#ifdef __TEMPERATURE_TEST__
int batt_temp_test_mode = 0;	// 0: auto test, 1: manual block, 2: manual normal (recover)
unsigned int start_time_test = 0;
unsigned int test_interval = 2;	// sec

static int _get_temp_adc_auto(void)
{
	unsigned int total_time;
	int adc;

	if (batt_temp_test_mode == 0) // auto test
	{
		if (!start_time_test)
			start_time_test = jiffies_to_msecs(jiffies);

		if (start_time_test)
		{
			total_time = jiffies_to_msecs(jiffies) - start_time_test;

			if ((0*60*1000) < total_time && total_time < (test_interval*60*1000))
			{
#if 0
				if (charging_mode_get())
					adc = TEMP_HIGH_RECOVER_LPM - 2;
				else
#endif
					adc = TEMP_HIGH_RECOVER - 2;
			}
			else if ((test_interval*60*1000) < total_time && total_time < (test_interval*2*60*1000))
			{
#if 0
				if (charging_mode_get())
					adc = TEMP_HIGH_BLOCK_LPM + 2;
				else
#endif
					adc = TEMP_HIGH_BLOCK + 2;
			}
			else if (total_time > (test_interval*2*60*1000))
			{
				start_time_test = jiffies_to_msecs(jiffies);	// Restart !
			}
		}
	} 
	else if (batt_temp_test_mode == 1)	// high block
	{
#if 0
		if (charging_mode_get())
			adc = TEMP_HIGH_BLOCK_LPM + 2;
		else
#endif
			adc = TEMP_HIGH_BLOCK + 2;
	}

	return adc;
}
#endif	/* __TEMPERATURE_TEST__ */

#ifdef __MANUAL_TEMP_TEST__
#define MANUAL_TEMP_ADC_DEFAULT	(TEMP_HIGH_RECOVER - 100)
static int _manual_temp_adc = MANUAL_TEMP_ADC_DEFAULT;
#endif

#ifdef __SOC_TEST__
#define SOC_TEST_DEFAULT_VALUE	10
static int soc_test = SOC_TEST_DEFAULT_VALUE;
#endif

static unsigned long s3c_read_temp(void)
{
	int adc = 0;

//	dev_dbg(dev, "%s\n", __func__);
	adc = s3c_bat_get_adc_data(S3C_ADC_TEMPERATURE);

#ifdef __TEMPERATURE_TEST__
	adc = _get_temp_adc_auto();
#endif

//	dev_dbg(dev, "%s: adc = %d\n", __func__, adc);

	s3c_bat_info.bat_info.batt_temp_adc = adc;
	return adc;
}

static int is_over_abs_time(void)
{
	unsigned int total_time;

	if (!start_time_msec)
		return 0;

	if (s3c_bat_info.bat_info.batt_is_recharging)
		total_time = TOTAL_RECHARGING_TIME;
	else
		total_time = TOTAL_CHARGING_TIME;

	if(jiffies_to_msecs(jiffies) >= start_time_msec)
		total_time_msec = jiffies_to_msecs(jiffies) - start_time_msec;
	else
		total_time_msec = jiffies_to_msecs(0xFFFFFFFF) - start_time_msec + jiffies_to_msecs(jiffies);

	if (total_time_msec > total_time) {
//		printk("%s abs time over (abs time: %u, total time: %u, start time: %u)\n",__func__, total_time, total_time_msec, start_time_msec);
		return 1;
	} else
		return 0;
}

#ifdef __CHECK_CHG_CURRENT__
static void check_chg_current(void)
{
	static int cnt = 0;
	unsigned long chg_current = 0; 

	chg_current = s3c_bat_get_adc_data(S3C_ADC_CHG_CURRENT);
	s3c_bat_info.bat_info.batt_current = chg_current;

	if ( (s3c_bat_info.bat_info.batt_vol >= FULL_CHARGE_COND_VOLTAGE) &&		// DH11 (full charge condition added)
		(s3c_bat_info.bat_info.batt_current <= CURRENT_OF_FULL_CHG) ) {
#ifdef __TEST_MODE_INTERFACE__
		if (s3c_bat_info.bat_info.batt_test_mode == 1)	// test mode (interval 1 sec)
			cnt++;
		else	// non-test mode (interval 2 sec)
#endif
			cnt += 2;

		if (cnt >= CHG_CURRENT_COUNT)
		{
//			dev_info(dev, "%s: battery full (bat_vol = %d) \n", __func__, s3c_bat_info.bat_info.batt_vol);
			s3c_set_chg_en(0);
			s3c_bat_info.bat_info.batt_is_full = 1;
			force_update = 1;
			cnt = 0;
		}
	} else {
		cnt = 0;
	}
}
#endif /* __CHECK_CHG_CURRENT__ */

#ifdef __ADJUST_RECHARGE_ADC__
static void check_recharging_bat(int bat_vol)
{
	static int cnt = 0;
	static int cnt_backup = 0;	// hanapark_DH17

//	if (s3c_bat_info.bat_info.batt_is_full)
//		dev_info(dev, "%s: check recharge : bat_vol = %d \n", __func__, bat_vol);

	if (s3c_bat_info.bat_info.batt_is_full && 
		!s3c_bat_info.bat_info.charging_enabled &&
		/*batt_recharging != -1 &&*/ bat_vol < RECHARGE_COND_VOLTAGE/*batt_recharging*/) {	// hanapark (recharge voltage)
#ifdef __TEST_MODE_INTERFACE__
		if (s3c_bat_info.bat_info.batt_test_mode == 1)	// test mode (interval 1 sec)
			cnt++;
		else	// non-test mode (interval 2 sec)
#endif
			cnt += 2;

		if (cnt >= BATT_RECHARGE_COUNT) {	// hanapark
//			dev_info(dev, "%s: recharging (bat_vol = %d) \n", __func__, bat_vol);
			s3c_bat_info.bat_info.batt_is_recharging = 1;
			s3c_set_chg_en(1);
			cnt = 0;
		}
	} else {
		cnt = 0;
	}

	if (s3c_bat_info.bat_info.batt_is_full && 
		!s3c_bat_info.bat_info.charging_enabled &&
		bat_vol < RECHARGE_COND_VOLTAGE_BACKUP) {	// hanapark_DH17 (recharge condition added)
#ifdef __TEST_MODE_INTERFACE__
		if (s3c_bat_info.bat_info.batt_test_mode == 1)	// test mode (interval 1 sec)
			cnt_backup++;
		else	// non-test mode (interval 2 sec)
#endif
			cnt_backup += 2;

		if (cnt_backup >= BATT_RECHARGE_COUNT) { // hanapark
//			dev_info(dev, "%s: recharging backup (bat_vol = %d) \n", __func__, bat_vol);
			s3c_bat_info.bat_info.batt_is_recharging = 1;
			s3c_set_chg_en(1);
			cnt_backup = 0;
		}
	} else {
		cnt_backup = 0;
	}
}
#endif /* __ADJUST_RECHARGE_ADC__ */

static int s3c_get_bat_level(void)
{
	int fg_soc = -1;
	int fg_vcell = -1;

#ifdef __SOC_TEST__
	fg_soc = soc_test;
#else
	if ((fg_soc = fg_read_soc()) < 0) {
		dev_err(dev, "%s: Can't read soc!!!\n", __func__);
		fg_soc = s3c_bat_info.bat_info.level;
	}
#endif
	if ((fg_vcell = fg_read_vcell()) < 0) {
		dev_err(dev, "%s: Can't read vcell!!!\n", __func__);
		fg_vcell = s3c_bat_info.bat_info.batt_vol;
	}
	else
		s3c_bat_info.bat_info.batt_vol = fg_vcell;

	if (is_over_abs_time()) {
		fg_soc = 100;
		s3c_bat_info.bat_info.batt_is_full = 1;
		dev_info(dev, "%s: charging time is over\n", __func__);
		s3c_set_chg_en(0);
		goto __end__;
	}

#ifdef __CHECK_CHG_CURRENT__
	if (s3c_bat_info.bat_info.charging_enabled) {
		check_chg_current();
	} 
#endif /* __CHECK_CHG_CURRENT__ */

	if (s3c_bat_info.bat_info.batt_is_full)
		fg_soc = 100;

#ifdef __ADJUST_RECHARGE_ADC__
	check_recharging_bat(fg_vcell);
#endif /* __ADJUST_RECHARGE_ADC__ */

__end__:
	dev_dbg(dev, "%s: fg_soc = %d, is_full = %d\n",
			__func__, fg_soc, 
			s3c_bat_info.bat_info.batt_is_full);
	return fg_soc;
}

static u32 s3c_get_bat_health(void)
{
	return s3c_bat_info.bat_info.batt_health;
}

static void s3c_set_bat_health(u32 batt_health)
{
	s3c_bat_info.bat_info.batt_health = batt_health;
}

#ifdef CONFIG_WIRELESS_CHARGING
int s3c_get_wireless_status(void)
{
	return s3c_bat_info.bat_info.wc_status;
}
EXPORT_SYMBOL(s3c_get_wireless_status);
#endif

static void s3c_set_time_for_charging(int mode) {
	if (mode) {
		/* record start time for abs timer */
		start_time_msec = jiffies_to_msecs(jiffies);
		//dev_info(dev, "%s: start_time(%u)\n", __func__,
		//		start_time_msec);
	} else {
		/* initialize start time for abs timer */
		start_time_msec = 0;
		total_time_msec = 0;
		//dev_info(dev, "%s: reset abs timer\n", __func__);
	}
}

static void s3c_set_chg_en(int enable)
{
	int chg_en_val = maxim_chg_status();

	if (enable) {
		if (chg_en_val) {
			if(curent_device_type==PM_CHARGER_TA)
				maxim_charging_control(PM_CHARGER_TA, TRUE);
			else if(curent_device_type==PM_CHARGER_USB_INSERT)
				maxim_charging_control(PM_CHARGER_USB_INSERT, TRUE);
			else
				maxim_charging_control(PM_CHARGER_DEFAULT, FALSE);
			s3c_set_time_for_charging(1);
		}
	} else {
			maxim_charging_control(PM_CHARGER_DEFAULT, FALSE);
			s3c_set_time_for_charging(0);
			s3c_bat_info.bat_info.batt_is_recharging = 0;
			s3c_bat_info.bat_info.batt_current = 0;
	}

	s3c_bat_info.bat_info.charging_enabled = enable;
}

static void s3c_temp_control(int mode) {
	switch (mode) {
	case POWER_SUPPLY_HEALTH_GOOD:
		dev_info(dev, "%s: GOOD\n", __func__);
		s3c_set_bat_health(mode);
		break;
	case POWER_SUPPLY_HEALTH_OVERHEAT:
		dev_info(dev, "%s: OVERHEAT\n", __func__);
		s3c_set_bat_health(mode);
		s3c_bat_info.bat_info.batt_is_full = 0;	// hanapark_Atlas
		break;
	case POWER_SUPPLY_HEALTH_FREEZE:
		dev_info(dev, "%s: FREEZE\n", __func__);
		s3c_set_bat_health(mode);
		s3c_bat_info.bat_info.batt_is_full = 0;	// hanapark_Atlas
		break;
	default:
		break;
	}
	schedule_work(&cable_work);
}

enum {
	HEALTH_GOOD,
	HEALTH_OVERHEAT,
	HEALTH_OVERHEAT_BY_EVENT,
};

#if 1	// P110120-0381 Fix
#define TEMP_AVERAGE_COUNT		20
static int temp_history[TEMP_AVERAGE_COUNT] = {0};
static int temp_count = 0;
static int temp_index = 0;

static int _average_temperature(int temp_adc)
{
	int i, sum, max, min, ret;

#ifdef __MANUAL_TEMP_TEST__
	return _manual_temp_adc;
#endif

	if (temp_adc == 0)
		return 0;

	if (temp_count == 0)	// no data
	{
		for (i=0; i<TEMP_AVERAGE_COUNT; i++)	temp_history[i] = temp_adc;
	}

	if (temp_index >= temp_count)	temp_count++;

	max = min = temp_history[0];
	sum = 0;

	for (i=0; i<TEMP_AVERAGE_COUNT; i++)
	{
		if (i == temp_index)
		{
			temp_history[i] = temp_adc;
		}

		if (max < temp_history[i])	max = temp_history[i];
		if (min > temp_history[i])	min = temp_history[i];

		sum += temp_history[i];
	}

	ret = ((sum-max-min) / (TEMP_AVERAGE_COUNT-2));

	temp_index++;
	if (temp_index == TEMP_AVERAGE_COUNT)
	{
		temp_history[0] = ret;
		temp_index = 1;
	}

	//printk(KERN_EMERG "[BATT] %s: adc=%d, sum=%d, max=%d, min=%d, ret=%d\n", __func__, temp_adc, sum, max, min, ret);
	return ret;
}
#endif

#ifdef __VZW_AUTH_CHECK__
static int s3c_get_bat_temp(int force_good)	// P110208-0686 (Stealth-V: parameter modified)
#else
static int s3c_get_bat_temp(void)
#endif
{
	int temp = 0;	// in celcius degree
	int array_size = 0;
	int i = 0;
	int temp_adc = s3c_read_temp();
	int health = s3c_get_bat_health();
	int health_new = health;
	int temp_high_block;
	int temp_high_recover;
	int temp_low_block;
	int temp_low_recover;

#if 1	// P110120-0381 Fix
	temp_adc = _average_temperature(temp_adc);
#endif

#ifdef __TEST_MODE_INTERFACE__
	s3c_bat_info.bat_info.batt_temp_adc_aver = temp_adc;
#endif /* __TEST_MODE_INTERFACE__ */

#ifdef __VZW_AUTH_CHECK__
	if (force_good == 1)
		health = health_new = POWER_SUPPLY_HEALTH_GOOD;	// P110208-0686 (Stealth-V)
#endif

	if ( (s3c_bat_info.bat_info.charging_source == CHARGER_BATTERY) &&
		(s3c_bat_info.device_state != 0x0) &&
		(temp_adc > TEMP_HIGH_RECOVER && temp_adc <= TEMP_HIGH_BLOCK) )
	{
		health_new = POWER_SUPPLY_HEALTH_GOOD;	// P110120-1102 Fix
	}

	if (s3c_bat_info.device_state == 0x0)
	{
		temp_high_block = TEMP_HIGH_BLOCK;
		temp_high_recover = TEMP_HIGH_RECOVER;
	}
	else
	{
		temp_high_block = TEMP_EVENT_HIGH_BLOCK;
		temp_high_recover = TEMP_HIGH_RECOVER;
	}

	temp_low_block = TEMP_LOW_BLOCK;
	temp_low_recover = TEMP_LOW_RECOVER;

	if (temp_adc >= temp_high_block)
	{
		if (health != POWER_SUPPLY_HEALTH_OVERHEAT &&
				health != POWER_SUPPLY_HEALTH_UNSPEC_FAILURE) {
			health_new = POWER_SUPPLY_HEALTH_OVERHEAT;
			pr_info("%s: temp code = %d, overheat (block) \n", __func__, temp_adc);
		}
	}
	else if (temp_adc <= temp_high_recover && temp_adc >= temp_low_recover) {
		if (health == POWER_SUPPLY_HEALTH_OVERHEAT ||
				health == POWER_SUPPLY_HEALTH_FREEZE) {
			health_new = POWER_SUPPLY_HEALTH_GOOD;
			pr_info("%s: temp code = %d, good (recover) \n", __func__, temp_adc);
		}
	}
	else if (temp_adc <= temp_low_block)
	{
		if (health != POWER_SUPPLY_HEALTH_FREEZE &&
				health != POWER_SUPPLY_HEALTH_UNSPEC_FAILURE) {
			health_new = POWER_SUPPLY_HEALTH_FREEZE;
			pr_info("%s: temp code = %d, freeze (block) \n", __func__, temp_adc);
		}
	}

	if (health != health_new)
	{
//		printk("battery health changed! (%d -> %d)\n", health, health_new);
		s3c_temp_control(health_new);
	}

#ifdef __VZW_AUTH_CHECK__
	// P110208-0686 (Stealth-V)
	if (force_good == 1 && health_new == POWER_SUPPLY_HEALTH_GOOD) {
		pr_info("%s: force_good (temp_adc = %d)\n", __func__, temp_adc);
		s3c_temp_control(POWER_SUPPLY_HEALTH_GOOD);
	}
#endif

	/* Convert temperature ADC to celcius degree */
	array_size = ARRAY_SIZE(temper_table);
	for (i = 0; i < (array_size - 1); i++) {
		if (i == 0) {
			if (temp_adc <= temper_table[0][0]) {
				temp = temper_table[0][1];
				break;
			} else if (temp_adc >= temper_table[array_size-1][0]) {
				temp = temper_table[array_size-1][1];
				break;
			}
		}

		if (temper_table[i][0] < temp_adc &&
				temper_table[i+1][0] >= temp_adc) {
			temp = temper_table[i+1][1];
		}
	}

//	dev_dbg(dev, "%s: temp = %d, adc = %d\n",
//			__func__, temp, temp_adc);

#ifdef __TEST_MODE_INTERFACE__
	s3c_bat_info.bat_info.batt_temp_aver = temp;	// hanapark ??
#endif /* __TEST_MODE_INTERFACE__ */

	return temp;	// temperature value in celcius degree
}

static int s3c_bat_get_charging_status(void)
{
	charger_type_t charger = CHARGER_BATTERY; 
	int ret = 0;

	charger = s3c_bat_info.bat_info.charging_source;

	switch (charger) {
		case CHARGER_BATTERY:
			ret = POWER_SUPPLY_STATUS_NOT_CHARGING;
			break;
		case CHARGER_USB:
		case CHARGER_AC:
			if (s3c_bat_info.bat_info.batt_is_full)
				ret = POWER_SUPPLY_STATUS_FULL;
			else
				ret = POWER_SUPPLY_STATUS_CHARGING;
			break;
		case CHARGER_DISCHARGE:
			ret = POWER_SUPPLY_STATUS_DISCHARGING;
			break;
		default:
			ret = POWER_SUPPLY_STATUS_UNKNOWN;
	}

	dev_dbg(dev, "%s: %s\n", __func__, status_text[ret]);
	return ret;
}

static int s3c_bat_get_property(struct power_supply *bat_ps, 
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	dev_dbg(bat_ps->dev, "%s: psp = %d\n", __func__, psp);

	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = s3c_bat_get_charging_status();
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = s3c_get_bat_health();
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = s3c_bat_info.present;
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
#ifdef __SOC_TEST__
		val->intval = soc_test;
#else
		val->intval = s3c_bat_info.bat_info.level;
#endif
		dev_dbg(dev, "%s: level = %d\n", __func__, 
				val->intval);
		break;
	case POWER_SUPPLY_PROP_TEMP:
		val->intval = s3c_bat_info.bat_info.batt_temp;
		dev_dbg(bat_ps->dev, "%s: temp = %d\n", __func__, 
				val->intval);
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int s3c_power_get_property(struct power_supply *bat_ps, 
		enum power_supply_property psp, 
		union power_supply_propval *val)
{
	switch (psp)
	{
		case POWER_SUPPLY_PROP_ONLINE:
			if(bat_ps->type == POWER_SUPPLY_TYPE_MAINS)
				val->intval = (curent_device_type == PM_CHARGER_TA ? 1 : 0);
			else if(bat_ps->type == POWER_SUPPLY_TYPE_USB)
				val->intval = (curent_device_type == PM_CHARGER_USB_INSERT ? 1 : 0);
			else
				val->intval = 0;
			break;
		default:
			return -EINVAL;
	}
	
	return 0;
}

#define SEC_BATTERY_ATTR(_name)								\
{											\
        .attr = { .name = #_name, .mode = S_IRUGO | S_IWUGO, .owner = THIS_MODULE },	\
        .show = s3c_bat_show_property,							\
        .store = s3c_bat_store,								\
}

static struct device_attribute s3c_battery_attrs[] = {
        SEC_BATTERY_ATTR(batt_vol),
        SEC_BATTERY_ATTR(batt_temp),
        SEC_BATTERY_ATTR(batt_temp_adc),
        SEC_BATTERY_ATTR(batt_temp_adc_cal),
#ifdef __TEST_MODE_INTERFACE__
	/* test mode */
	SEC_BATTERY_ATTR(batt_test_mode),
	/* average */
	SEC_BATTERY_ATTR(batt_vol_aver),
	SEC_BATTERY_ATTR(batt_temp_aver),
	SEC_BATTERY_ATTR(batt_temp_adc_aver),
#endif /* __TEST_MODE_INTERFACE__ */
#ifdef __CHECK_CHG_CURRENT__
	SEC_BATTERY_ATTR(batt_chg_current),
	SEC_BATTERY_ATTR(batt_chg_current_aver),
#endif /* __CHECK_CHG_CURRENT__ */
	SEC_BATTERY_ATTR(charging_source),
	SEC_BATTERY_ATTR(fg_soc),
	SEC_BATTERY_ATTR(reset_soc),
	SEC_BATTERY_ATTR(full_notify),
	SEC_BATTERY_ATTR(fg_point_level),	// lobat pwroff
#ifdef LPM_MODE
	SEC_BATTERY_ATTR(charging_mode_booting),
	SEC_BATTERY_ATTR(batt_health_check),
#endif
#ifdef __TEMPERATURE_TEST__
	SEC_BATTERY_ATTR(batt_temp_test),
#endif
#ifdef __MANUAL_TEMP_TEST__
	SEC_BATTERY_ATTR(manual_temp_adc),
#endif
#ifdef __SOC_TEST__
	SEC_BATTERY_ATTR(soc_test),
#endif
#ifdef __TEMP_BLOCK_EXCEPTION__
	SEC_BATTERY_ATTR(chargingblock_clear),	// hanapark_Atlas (from Max)
	SEC_BATTERY_ATTR(talk_wcdma),	// 3G voice call
	SEC_BATTERY_ATTR(data_call),	// Data call
	SEC_BATTERY_ATTR(lte),	// 4G LTE (I510V)
#endif /* __TEMP_BLOCK_EXCEPTION__ */
#ifdef __VZW_AUTH_CHECK__
	SEC_BATTERY_ATTR(auth_battery),
	SEC_BATTERY_ATTR(auth_battery_enable),
#endif
	SEC_BATTERY_ATTR(wc_status),
	SEC_BATTERY_ATTR(wc_adc),
};

enum {
        BATT_VOL = 0,
        BATT_TEMP,
        BATT_TEMP_ADC,
        BATT_TEMP_ADC_CAL,
#ifdef __TEST_MODE_INTERFACE__
	BATT_TEST_MODE,
	BATT_VOL_AVER,
	BATT_TEMP_AVER,
	BATT_TEMP_ADC_AVER,
#endif /* __TEST_MODE_INTERFACE__ */
#ifdef __CHECK_CHG_CURRENT__
	BATT_CHG_CURRENT,	
	BATT_CHG_CURRENT_AVER,
#endif /* __CHECK_CHG_CURRENT__ */
	BATT_CHARGING_SOURCE,
	BATT_FG_SOC,
	BATT_RESET_SOC,
	BATT_FULL_NOTIFY,
	BATT_DECIMAL_POINT_LEVEL,	// lobat pwroff
#ifdef LPM_MODE
	CHARGING_MODE_BOOTING,
	BATT_HEALTH_CHECK,
#endif
#ifdef __TEMPERATURE_TEST__
	BATT_TEMP_TEST,
#endif
#ifdef __MANUAL_TEMP_TEST__
	BATT_MANUAL_TEMP_ADC,
#endif
#ifdef __SOC_TEST__
	SOC_TEST,
#endif
#ifdef __TEMP_BLOCK_EXCEPTION__
	BATT_CHARGINGBLOCK_CLEAR,	// hanapark_Atlas (from Max)
	TALK_WCDMA,	// 3G voice call
	DATA_CALL,	// Data call
	LTE,	// 4G LTE (I510V)
#endif /* __TEMP_BLOCK_EXCEPTION__ */
#ifdef __VZW_AUTH_CHECK__
	AUTH_BATTERY,
	AUTH_BATTERY_ENABLE,
#endif
	WC_STATUS,
	WC_ADC,
};

static int s3c_bat_create_attrs(struct device * dev)
{
	int i, rc;

	for (i = 0; i < ARRAY_SIZE(s3c_battery_attrs); i++) {
		rc = device_create_file(dev, &s3c_battery_attrs[i]);
		if (rc)
			goto s3c_attrs_failed;
	}

	goto succeed;

s3c_attrs_failed:
	while (i--)
		device_remove_file(dev, &s3c_battery_attrs[i]);
succeed:
	return rc;
}

static ssize_t s3c_bat_show_property(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	int i = 0;
	const ptrdiff_t off = attr - s3c_battery_attrs;

	switch (off) {
		case BATT_VOL:	// voltage (mV)
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				s3c_bat_info.bat_info.batt_vol + 30);	// 30mA voltage drop
			break;
		case BATT_TEMP:	// temperature value in celcius degree (`C)
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				s3c_bat_info.bat_info.batt_temp);
			break;
		case BATT_TEMP_ADC:	// temperature ADC
			//s3c_bat_info.bat_info.batt_temp_adc = s3c_bat_get_adc_data(S3C_ADC_TEMPERATURE);
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				s3c_bat_info.bat_info.batt_temp_adc);
			break;
		case BATT_TEMP_ADC_CAL:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				s3c_bat_info.bat_info.batt_temp_adc_cal);
			break;
#ifdef __TEST_MODE_INTERFACE__
		case BATT_TEST_MODE:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				s3c_bat_info.bat_info.batt_test_mode);
			break;
		case BATT_VOL_AVER:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				s3c_bat_info.bat_info.batt_vol_aver);
			break;
		case BATT_TEMP_AVER:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				s3c_bat_info.bat_info.batt_temp_aver);
			break;
		case BATT_TEMP_ADC_AVER:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				s3c_bat_info.bat_info.batt_temp_adc_aver);
			break;
#endif /* __TEST_MODE_INTERFACE__ */
#ifdef __CHECK_CHG_CURRENT__
		case BATT_CHG_CURRENT:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				s3c_bat_get_adc_data(S3C_ADC_CHG_CURRENT));
			break;
		case BATT_CHG_CURRENT_AVER:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				s3c_bat_info.bat_info.batt_current);
			break;
#endif /* __CHECK_CHG_CURRENT__ */
		case BATT_CHARGING_SOURCE:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				s3c_bat_info.bat_info.charging_source);
			break;
		case BATT_FG_SOC:
#ifdef __SOC_TEST__
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				soc_test);
#else
			if (s3c_bat_info.bat_info.decimal_point_level == 0)	// lobat pwroff
			{
				i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
					s3c_bat_info.bat_info.level);
			}
			else
			{
				int soc = fg_read_soc();
				if (soc < 0)
					soc = s3c_bat_info.bat_info.level;
				i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", soc);
			}
#endif /*__SOC_TEST__*/
			break;
		case BATT_FULL_NOTIFY:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				s3c_bat_info.bat_info.batt_is_full);
			break;	
		case BATT_DECIMAL_POINT_LEVEL:	// lobat pwroff
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				s3c_bat_info.bat_info.decimal_point_level);
			break;	
#ifdef LPM_MODE
		case CHARGING_MODE_BOOTING:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				charging_mode_get());
			break;		
		case BATT_HEALTH_CHECK:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				s3c_get_bat_health());
			break;		
#endif
#ifdef __TEMPERATURE_TEST__
		case BATT_TEMP_TEST:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				batt_temp_test_mode);
			break;
#endif
#ifdef __MANUAL_TEMP_TEST__
		case BATT_MANUAL_TEMP_ADC:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				_manual_temp_adc);
			break;
#endif
#ifdef __SOC_TEST__
		case SOC_TEST:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				soc_test);
			break;
#endif
#ifdef __TEMP_BLOCK_EXCEPTION__
		case BATT_CHARGINGBLOCK_CLEAR:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				s3c_bat_info.device_state);	// s3c_bat_info.bat_info.batt_chargingblock_clear
				break;
		case TALK_WCDMA:	// 3G voice call
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				(s3c_bat_info.device_state & USE_TALK_WCDMA));
			break;
		case DATA_CALL: // Data call
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				(s3c_bat_info.device_state & USE_DATA_CALL));
			break;
		case LTE:	// 4G LTE (I510V)
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				(s3c_bat_info.device_state & USE_4G));
			break;
#endif /* __TEMP_BLOCK_EXCEPTION__ */
#ifdef __VZW_AUTH_CHECK__
		case AUTH_BATTERY:
			if (s3c_get_bat_health() == POWER_SUPPLY_HEALTH_UNSPEC_FAILURE)
				i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", 0);
			else
				i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", 1);
			break;
		case AUTH_BATTERY_ENABLE:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				auth_battery_enabled);
			break;
#endif
		case WC_STATUS:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				s3c_bat_info.bat_info.wc_status);
			break;
		case WC_ADC:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				s3c_bat_get_adc_data(S3C_ADC_WC));
			break;
		default:
			i = -EINVAL;
			break;
	}       

	return i;
}

static ssize_t s3c_bat_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	int x = 0;
	int ret = 0;
	const ptrdiff_t off = attr - s3c_battery_attrs;

	switch (off) {
		case BATT_TEMP_ADC_CAL:
			if (sscanf(buf, "%d\n", &x) == 1) {
				s3c_bat_info.bat_info.batt_temp_adc_cal = x;
				ret = count;
			}
			break;
#ifdef __TEST_MODE_INTERFACE__
		case BATT_TEST_MODE:
			if (sscanf(buf, "%d\n", &x) == 1) {
				s3c_bat_info.bat_info.batt_test_mode = x;
				ret = count;
			}
			if (s3c_bat_info.bat_info.batt_test_mode)
				s3c_bat_info.polling_interval = POLLING_INTERVAL_TEST;
			else
				s3c_bat_info.polling_interval = POLLING_INTERVAL;
			if (s3c_bat_info.polling) {
				del_timer_sync(&polling_timer);
			}
			mod_timer(&polling_timer, jiffies + msecs_to_jiffies(s3c_bat_info.polling_interval));
			s3c_bat_status_update(&s3c_power_supplies_test[CHARGER_BATTERY]);
			break;
#endif /* __TEST_MODE_INTERFACE__ */
		case BATT_RESET_SOC:
			if (sscanf(buf, "%d\n", &x) == 1) {
				if (x == 1)
					fg_reset_soc();
				ret = count;
			}
			break;
#ifdef LPM_MODE
		case CHARGING_MODE_BOOTING:
			if (sscanf(buf, "%d\n", &x) == 1) {
				charging_mode_set(x);
				ret = count;
			}
			break;		
#endif
#ifdef __TEMPERATURE_TEST__
		case BATT_TEMP_TEST:
			if (sscanf(buf, "%d\n", &x) == 1) {
				if (x > 2)
					x = 2;
				else if (x < 0)
					x = 0;
				batt_temp_test_mode = x;
				if (x != 0)	// not auto test
					start_time_test = 0;	// initialize start time
				ret = count;
			}
			break;
#endif
#ifdef __MANUAL_TEMP_TEST__
		case BATT_MANUAL_TEMP_ADC:
			if (sscanf(buf, "%d\n", &x) == 1) {
				if (x == 0)	// back to default
					_manual_temp_adc = MANUAL_TEMP_ADC_DEFAULT;
				else
					_manual_temp_adc = x;
				ret = count;
			}
			break;
#endif
#ifdef __SOC_TEST__
		case SOC_TEST:
			if (sscanf(buf, "%d\n", &x) == 1) {
				soc_test = x;
				force_update = 1;
				ret = count;
			}
			break;
#endif
#ifdef __VZW_AUTH_CHECK__
		case AUTH_BATTERY_ENABLE:
			if (sscanf(buf, "%d\n", &x) == 1) {
				if (x == 1) {
					auth_battery_enabled = 1;
					batt_auth_full_check = 0;
				} else
					auth_battery_enabled = 0;
				pr_info("%s: AUTH_BATTERY_ENABLE = %d (write %d)\n", __func__, auth_battery_enabled, x);
				ret = count;
			}
			break;
#endif
#ifdef __TEMP_BLOCK_EXCEPTION__
		case BATT_CHARGINGBLOCK_CLEAR:
			if (sscanf(buf, "%d\n", &x) == 1) {
				if ((s3c_bat_info.device_state & USE_BROWSER) && !(x & USE_BROWSER))
				{
					s3c_bat_use_browser(0);	// browser off
				}
				else if (!(s3c_bat_info.device_state & USE_BROWSER) && (x & USE_BROWSER))
				{
					s3c_bat_use_browser(1);	// browser on
				}
				else
				{
					s3c_bat_info.device_state = x;	//s3c_bat_info.bat_info.batt_chargingblock_clear= x;
				}
				ret = count;
			}
			break;
		case TALK_WCDMA:
			if (sscanf(buf, "%d\n", &x) == 1) {
				if (x == 1)
					s3c_bat_info.device_state = s3c_bat_info.device_state | USE_TALK_WCDMA;
				else
					s3c_bat_info.device_state = s3c_bat_info.device_state & (~USE_TALK_WCDMA);
				ret = count;
			}
			break;
		case DATA_CALL:
			if (sscanf(buf, "%d\n", &x) == 1) {
				s3c_bat_use_data_call(x);
				ret = count;
			}
			break;
		case LTE:	// 4G LTE (I510V)
			if (sscanf(buf, "%d\n", &x) == 1) {
				s3c_bat_use_4g(x);
				ret = count;
			}
			break;
#endif /* __TEMP_BLOCK_EXCEPTION__ */
		default:
			ret = -EINVAL;
			break;
	}       

	return ret;
}

static enum power_supply_property s3c_battery_properties[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_CAPACITY,
};

static enum power_supply_property s3c_power_properties[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

static char *supply_list[] = {
	"battery",
};

static struct power_supply s3c_power_supplies[] = {
	{
		.name = "battery",
		.type = POWER_SUPPLY_TYPE_BATTERY,
		.properties = s3c_battery_properties,
		.num_properties = ARRAY_SIZE(s3c_battery_properties),
		.get_property = s3c_bat_get_property,
	},
	{
		.name = "usb",
		.type = POWER_SUPPLY_TYPE_USB,
		.supplied_to = supply_list,
		.num_supplicants = ARRAY_SIZE(supply_list),
		.properties = s3c_power_properties,
		.num_properties = ARRAY_SIZE(s3c_power_properties),
		.get_property = s3c_power_get_property,
	},
	{
		.name = "ac",
		.type = POWER_SUPPLY_TYPE_MAINS,
		.supplied_to = supply_list,
		.num_supplicants = ARRAY_SIZE(supply_list),
		.properties = s3c_power_properties,
		.num_properties = ARRAY_SIZE(s3c_power_properties),
		.get_property = s3c_power_get_property,
	},
};

static int s3c_cable_status_update(int status)
{
	int ret = 0;
	dev_dbg(dev, "%s\n", __func__);

	if (!s3c_battery_initial)
		return -EPERM;

	switch (status)
	{
		case CHARGER_BATTERY:
			printk("%s: cable NOT PRESENT\n", __func__);
			s3c_bat_info.bat_info.charging_source = CHARGER_BATTERY;
			break;
		case CHARGER_USB:
			printk("%s: cable USB\n", __func__);
			s3c_bat_info.bat_info.charging_source = CHARGER_USB;
			break;
		case CHARGER_AC:
			printk("%s: cable AC\n", __func__);
			s3c_bat_info.bat_info.charging_source = CHARGER_AC;
			break;
		case CHARGER_DISCHARGE:
			printk("%s: Discharge\n", __func__);
			s3c_bat_info.bat_info.charging_source = CHARGER_DISCHARGE;
			break;
		default:
			printk("%s: Not supported status\n", __func__);
			ret = -EINVAL;
			break;
	}

	if (curent_device_type != PM_CHARGER_NULL)
	{
		/* do not enter kernel sleep mode during charging */
		wake_lock(&vbus_wake_lock);
	}
	else
	{
		/* give userspace some time to see the uevent and update
		* LED state or whatnot...
		*/
		if (!maxim_chg_status())
			wake_lock_timeout(&vbus_wake_lock, HZ * 5);
	}

	/* if the power source changes, all power supplies may change state */
	power_supply_changed(&s3c_power_supplies[CHARGER_BATTERY]);
	printk("%s: call power_supply_changed\n", __func__);

	return ret;
}

#ifdef CONFIG_WIRELESS_CHARGING
static void s3c_cable_check_status(void);
extern void fsa9480_chip_init(void);

static void s3c_bat_check_wireless_status(struct power_supply *bat_ps)
{
	int wc_status = s3c_bat_info.bat_info.wc_status;
	int is_wired = gpio_get_value(GPIO_WC_DETECT);

	if (is_wired && wc_status)	// insert wired charger during wireless charging... change charging source priority
	{
		pr_info("%s: Wired charger is inserted. Stop wireless charging...\n", __func__);
		s3c_set_chg_en(0);
		fsa9480_chip_init();	// force detach wireless charger (consider as USB)
	}
}
#endif

static void s3c_bat_status_update(struct power_supply *bat_ps)
{
	int old_level, old_temp, old_is_full;
	dev_dbg(dev, "%s\n", __func__);

	if (!s3c_battery_initial)
		return;

	mutex_lock(&work_lock);

	old_temp = s3c_bat_info.bat_info.batt_temp;	// in degree
	old_level = s3c_bat_info.bat_info.level; 
	old_is_full = s3c_bat_info.bat_info.batt_is_full;

#ifdef __VZW_AUTH_CHECK__
	s3c_bat_info.bat_info.batt_temp = s3c_get_bat_temp(0);	// P110208-0686 (Stealth-V)
#else
	s3c_bat_info.bat_info.batt_temp = s3c_get_bat_temp();
#endif
	s3c_bat_info.bat_info.level = s3c_get_bat_level();

#if 0
	if (!s3c_bat_info.bat_info.charging_enabled &&
			!s3c_bat_info.bat_info.batt_is_full) {
		if (s3c_bat_info.bat_info.level > old_level)
			s3c_bat_info.bat_info.level = old_level;
	}
#endif

	if ((s3c_bat_check_v_f() == 0) && (s3c_bat_info.bat_info.charging_source != CHARGER_BATTERY))
	{
		charging_stop_without_magic_number();	// Forced power off when battery is removed...
	}

	if (old_level != s3c_bat_info.bat_info.level 
			|| old_temp != s3c_bat_info.bat_info.batt_temp
			|| old_is_full != s3c_bat_info.bat_info.batt_is_full
			|| force_update) {
		force_update = 0;
		power_supply_changed(bat_ps);
		dev_dbg(dev, "%s: call power_supply_changed\n", __func__);
	}

	mutex_unlock(&work_lock);
}

static unsigned int s3c_bat_check_v_f(void)
{
#ifdef __VZW_AUTH_CHECK__
	int retval = 0;
	static int jig_status = 0;

	if ((FSA9480_Get_JIG_Status() == 1) || (auth_battery_enabled == 0))
	{
		if (jig_status == 0) {
			jig_status = 1;
			s3c_set_bat_health(POWER_SUPPLY_HEALTH_GOOD);
		}
		return 1;
	}
	else if (jig_status == 1)
	{
		jig_status = 0;
		batt_auth_full_check = 0; // retry
	}

	if (batt_auth_full_check == 0)
	{
		retval = verizon_batt_auth_full_check();
		batt_auth_full_check = 1;

		if (!retval)
		{
			s3c_set_bat_health(POWER_SUPPLY_HEALTH_UNSPEC_FAILURE);
			BattAuth_Finish();
			pr_info("%s: Vzw battery auth failed\n", __func__);

			force_update = 1;
			return 0;
		}
		else if (s3c_get_bat_health() == POWER_SUPPLY_HEALTH_UNSPEC_FAILURE) {
			// P110208-0686 (Stealth-V)
			s3c_get_bat_temp(1);
			force_update = 1;
			pr_info("%s: Batt auth retry succeed. (health=%d)\n", __func__, s3c_get_bat_health());
		}
	}
	else
	{
		retval = verizon_batt_auth_check();
		BattAuth_Finish();

		if (!retval)
			return 0;
	}

	BattAuth_Finish();
#endif	/* __VZW_AUTH_CHECK__ */

	return 1;
}

static void s3c_cable_check_status(void)
{
	charger_type_t status = 0;
	dev_dbg(dev, "%s\n", __func__);

	mutex_lock(&work_lock);

	if (maxim_chg_status())
	{
		if (s3c_get_bat_health() != POWER_SUPPLY_HEALTH_GOOD)
		{
			printk("%s: Unhealth battery state! (%d) \n", __func__, s3c_get_bat_health());
			status = CHARGER_DISCHARGE;
			s3c_set_chg_en(0);
			goto __end__;
		}

#ifdef CONFIG_WIRELESS_CHARGING
		if (gpio_get_value(GPIO_WC_DETECT) == 0)
		{
			s3c_bat_info.bat_info.wc_status = 1;
			pr_info("%s: wireless charger detected!\n", __func__);
		}
		else
			s3c_bat_info.bat_info.wc_status = 0;
#endif

		if (get_usb_power_state())
			status = CHARGER_USB;
		else
			status = CHARGER_AC;	// include wireless charging

		s3c_bat_info.bat_info.decimal_point_level = 1;	// lobat pwroff

		set_low_bat_interrupt(0);	// lobat pwroff
		set_low_bat_interrupt_mask_enable(0);	// lobat pwroff
		
		s3c_set_chg_en(1);
		printk("%s: status : %s\n", __func__, ((status == CHARGER_USB) ? "USB" : "AC"));
		
	}
	else
	{
		status = CHARGER_BATTERY;
		s3c_set_chg_en(0);
#ifdef CONFIG_WIRELESS_CHARGING
		s3c_bat_info.bat_info.wc_status = 0;
#endif
		printk("%s: No charger!\n", __func__);
	}

__end__:
	s3c_cable_status_update(status);
	mutex_unlock(&work_lock);
}

int s3c_bat_is_in_call(void)
{
	if (s3c_bat_info.device_state & 0x01)
		return 1;	// in call
	else
		return 0;
}
EXPORT_SYMBOL(s3c_bat_is_in_call);

void low_battery_power_off(void)	// lobat pwroff (sleep interrupt)
{
	if (FSA9480_Get_JIG_Status())	// skip low battery check when JIG cable is inserted
	{
		s3c_bat_info.bat_info.decimal_point_level = 1;
		return ;
	}

	if (s3c_bat_info.bat_info.decimal_point_level == 0)
		return ;	// already requested...

	s3c_bat_info.bat_info.level = 0;
	s3c_bat_info.bat_info.decimal_point_level = 0;

	force_update = 1;
	schedule_work(&bat_work);
	mod_timer(&polling_timer, jiffies + msecs_to_jiffies(s3c_bat_info.polling_interval));
}

static void low_battery_power_off_polling(void)	// lobat pwroff
{
	if (FSA9480_Get_JIG_Status())	// skip low battery check when JIG cable is inserted
	{
		s3c_bat_info.bat_info.decimal_point_level = 1;
		return ;
	}

	if (s3c_bat_info.bat_info.decimal_point_level == 0)
		return ;	// already requested...

	s3c_bat_info.bat_info.level = 0;
	s3c_bat_info.bat_info.decimal_point_level = 0;

	wake_lock_timeout(&low_batt_wake_lock, HZ * LOW_BATT_COUNT);

	force_update = 1;
	schedule_work(&bat_work);
	mod_timer(&polling_timer, jiffies + msecs_to_jiffies(s3c_bat_info.polling_interval));
}

static void s3c_check_low_batt(void)	// lobat pwroff
{
	static int cnt = 0;
	int low_batt_vol = LOW_BATT_COND_VOLTAGE;	// 3400mV

	if (FSA9480_Get_JIG_Status())	// skip low battery check when JIG cable is inserted
	{
		s3c_bat_info.bat_info.decimal_point_level = 1;
		cnt = 0;
		return ;
	}

	if (s3c_bat_info.bat_info.decimal_point_level == 0)
		return ;	// already requested...

	if (s3c_bat_info.device_state & 0x1)	// compensation for voice call
	{
		low_batt_vol = 3350;
	}

	if (s3c_bat_info.bat_info.batt_vol <= low_batt_vol)
	{
#ifdef __TEST_MODE_INTERFACE__
		if (s3c_bat_info.bat_info.batt_test_mode == 1)	// test mode (interval 1 sec)
			cnt++;
		else	// non-test mode (interval 2 sec)
#endif
			cnt += 2;

		if (cnt >= 20)
			low_battery_power_off_polling();
	}
	else {
		if (s3c_bat_info.bat_info.decimal_point_level == 0)
			force_update = 1;
		s3c_bat_info.bat_info.decimal_point_level = 1;
		cnt = 0;
	}
}

static void s3c_bat_work(struct work_struct *work)
{
#ifdef CONFIG_WIRELESS_CHARGING
	if (HWREV == 9) {
		if (s3c_bat_info.bat_info.charging_source != CHARGER_BATTERY)
			s3c_bat_check_wireless_status(&s3c_power_supplies[CHARGER_BATTERY]);
	}
#endif

	s3c_bat_status_update(&s3c_power_supplies[CHARGER_BATTERY]);

#if 1
	if (!charging_mode_get())	// except LPM mode
		s3c_check_low_batt(); // lobat pwroff
#endif
}

static void s3c_cable_work(struct work_struct *work)
{
	dev_dbg(dev, "%s\n", __func__);
	s3c_cable_check_status();
}

#ifdef __VZW_AUTH_CHECK__
static int verizon_batt_auth_full_check(void)
{
	int retval = 0;

	if (Reset_TA())
	{
		return 0;
	}

	if (rom_code_protection() == 0)
	{
		retval = 0;
	}
	else if (CRC_protection() == 0)
	{
		retval = 0;
	}
	else
	{
		retval = 1;
	}

	return retval;
}

static int verizon_batt_auth_multi_check(void)
{
	int i;
	int retval = 0;

	for (i = 0; i < 3 ; i++)
	{
		if (!Reset_TA())
			retval = 1;

		if(retval)
			break;
	}

	return retval;
}

static int verizon_batt_auth_check(void)
{
	int result = 0;

	if (!Reset_TA())
	{
		return 1;
	}
	else
	{
		result = verizon_batt_auth_multi_check();
	}

	return result;
}
#endif	/* __VZW_AUTH_CHECK__ */

#ifdef CONFIG_PM
static int s3c_bat_suspend(struct platform_device *pdev, 
		pm_message_t state)
{
	dev_dbg(dev, "%s\n", __func__);

	set_low_bat_interrupt(1);	// lobat pwroff

	if (s3c_bat_info.polling)
		del_timer_sync(&polling_timer);

	flush_scheduled_work();
	return 0;
}

static int s3c_bat_resume(struct platform_device *pdev)
{
	dev_dbg(dev, "%s\n", __func__);
	
	set_low_bat_interrupt(0);	// lobat pwroff
	schedule_work(&bat_work);

	if (s3c_bat_info.polling)
		mod_timer(&polling_timer,
			  jiffies + msecs_to_jiffies(s3c_bat_info.polling_interval));

	return 0;
}
#else
#define s3c_bat_suspend NULL
#define s3c_bat_resume NULL
#endif /* CONFIG_PM */

static void polling_timer_func(unsigned long unused)
{
	schedule_work(&bat_work);
	mod_timer(&polling_timer,
		jiffies + msecs_to_jiffies(s3c_bat_info.polling_interval));
}

void s3c_cable_changed(void)
{
	dev_dbg(dev, "%s\n", __func__);

	if (!s3c_battery_initial)
		return ;

	s3c_bat_info.bat_info.batt_is_full = 0;

#ifdef __VZW_AUTH_CHECK__
		pr_info("%s: retry vzw batt auth\n", __func__);
		batt_auth_full_check = 0; // P110208-0686 (Stealth-V: Retry vzw battery auth.)
		s3c_bat_check_v_f();
#endif

	schedule_work(&cable_work);
	/*
	 * Wait a bit before reading ac/usb line status and setting charger,
	 * because ac/usb status readings may lag from irq.
	 */
	mod_timer(&polling_timer,
		  jiffies + msecs_to_jiffies(s3c_bat_info.polling_interval));
}

void s3c_cable_charging(void)
{
	// do nothing (called by max8998_function.c)
}

static void battery_early_suspend(struct early_suspend *h)
{
	u32 con;
	dev_dbg(dev, "%s\n", __func__);

	/*hsmmc clock disable*/
	con = readl(S5P_CLKGATE_IP2);
	con &= ~(S5P_CLKGATE_IP2_HSMMC3|S5P_CLKGATE_IP2_HSMMC2|S5P_CLKGATE_IP2_HSMMC1 \
		|S5P_CLKGATE_IP2_HSMMC0);
	writel(con, S5P_CLKGATE_IP2);

	/*g3d clock disable*/
	con = readl(S5P_CLKGATE_IP0);
	con &= ~S5P_CLKGATE_IP0_G3D;
	writel(con, S5P_CLKGATE_IP0);

	/*power gating*/
	con = readl(S5P_NORMAL_CFG);
	con &= ~(S5PC110_POWER_DOMAIN_G3D|S5PC110_POWER_DOMAIN_MFC|S5PC110_POWER_DOMAIN_TV \
		|S5PC110_POWER_DOMAIN_CAM|S5PC110_POWER_DOMAIN_AUDIO);
	writel(con , S5P_NORMAL_CFG);
}

static void battery_late_resume(struct early_suspend *h)
{
	// do nothing!
}

/* Quick start condition check. */
static struct fuelgauge_linear_data {
	u32 min_vcell;
	u32 slope;
	u32 y_interception;
} qstrt_table[3][12] = {
	{	// w/o charger
		{ 450000000,		0,			0 },
		{ 405998040,	2650000,	144362000 },
		{ 387529140,	860000,		321121000 },
		{ 376441370,	600000,		341513000 },
		{ 369692680,	310000,		358015000 },
		{ 365367780,	230000,		361056000 },
		{ 356785120,	600000,		354243000 },
		{ 353123580,	1840000,	349019000 },
		{ 346112900,	4930000,	342157000 },
		{ 339809470,	7900000,	339809000 },
		{ 100000000,		0,			0 },
		{ 100000000,		0,			0 }
	},

	{	// TA (800mA)
		{ 450000000,		0,			0 },
		{ 419160000,	138478,		405313307 },
		{ 417850000,	632795,		360554037 },
		{ 408640000,	425641,		376298802 },
		{ 397270000,	215941,		386631884 },
		{ 391870000,	442927,		381126923 },
		{ 387660000,	501151,		380257313 },
		{ 382060000,	1355256,	377211610 },
		{ 379620000,	5679819,	369095764 },
		{ 370950000,	14768435,	365852416 },
		{ 100000000,		0,			0 },
		{ 100000000,		0,			0 }
	},

	{	// USB (475mA)
		{ 450000000,		0,			0 },
		{ 419290000,	796795,		339611926 },
		{ 402940000,	536608,		360291283 },
		{ 385730000,	169177,		377711783 },
		{ 381500000,	601621,		368032096 },
		{ 375700000,	529866,		368940706 },
		{ 370290000,	4829078,	357702643 },
		{ 361590000,	8720399,	354534233 },
		{ 100000000,		0,			0 },
		{ 100000000,		0,			0 },
		{ 100000000,		0,			0 },
		{ 100000000,		0,			0 }
	},
};

#define FG_SOC_TOLERANCE	20	// 15

#define STATUS_WO_CHARGER	0
#define	STATUS_TA_CHARGER	1
#define STATUS_USB_CHARGER	2

#define	WO_CHARGER_ARR_SIZE	11
#define	TA_CHARGER_ARR_SIZE	11
#define	USB_CHARGER_ARR_SIZE	9

static int check_quick_start(void)
{
	unsigned int vcell_raw = 0;
	int soc_raw = 0, soc_cal = 0;
	int i, curr_idx = 0;
	int status = 0;
	int array_size = 0;

	if (maxim_lpm_chg_status() == 0)	// check external input power
	{
		status = STATUS_WO_CHARGER;
		array_size = WO_CHARGER_ARR_SIZE;
		pr_debug("[BATT] %s: No charger !\n", __func__);
	}
	else
	{
		if (get_usb_power_state() || gpio_get_value(GPIO_WC_DETECT) == 0) {
			status = STATUS_USB_CHARGER;
			array_size = USB_CHARGER_ARR_SIZE;
		}
		else {
			status = STATUS_TA_CHARGER;
			array_size = TA_CHARGER_ARR_SIZE;
		}
		pr_debug("[BATT] %s: charger detected !\n", __func__);
	}

	/* get vcell. */
	vcell_raw = fg_read_raw_vcell();

	/* get soc. */
	soc_raw = fg_read_raw_soc(); 

	/* find range */
	for (i = 0; i < array_size-1; i++) {
		if (vcell_raw < qstrt_table[status][i].min_vcell &&
				vcell_raw >= qstrt_table[status][i+1].min_vcell) {
			curr_idx = i+1;
			break;
		}
	}

	pr_debug("[BATT] %s: curr_idx = %d (vol=%d)\n", __func__, curr_idx, qstrt_table[status][curr_idx].min_vcell);

	/* calculate assumed soc and compare */
	if ( (status == STATUS_WO_CHARGER && curr_idx > 0 && curr_idx < (WO_CHARGER_ARR_SIZE)) ||
		(status == STATUS_TA_CHARGER && curr_idx > 0 && curr_idx < (TA_CHARGER_ARR_SIZE)) ||
		(status == STATUS_USB_CHARGER && curr_idx > 0 && curr_idx < (USB_CHARGER_ARR_SIZE)) ) {
		int limit_min, limit_max;
		soc_cal = (int) ((vcell_raw - qstrt_table[status][curr_idx].y_interception) / qstrt_table[status][curr_idx].slope);

		pr_debug("[BATT] %s: soc_cal = %d\n", __func__, soc_cal);

		limit_min = soc_cal - FG_SOC_TOLERANCE;
		limit_max = soc_cal + FG_SOC_TOLERANCE;
		if (limit_min < 0)
			limit_min = 0;

		if (soc_raw > limit_max || soc_raw < limit_min) {
			charging_stop_without_magic_number();
			fg_reset_soc();
			charging_start_without_magic_number();
			pr_debug("\n[BATT] %s: QUICK START (reset_soc)!!! \n\n", __func__);
		}
	}

	return 0;
}

static int __devinit s3c_bat_probe(struct platform_device *pdev)
{
	int i = 0;
	int ret = 0;

	dev = &pdev->dev;
	dev_dbg(dev, "%s\n", __func__);

	s3c_bat_info.present = 1;
	s3c_bat_info.polling = 1;
	s3c_bat_info.polling_interval = POLLING_INTERVAL;
	s3c_bat_info.device_state = 0;

#ifdef __TEST_MODE_INTERFACE__
	s3c_bat_info.bat_info.batt_vol_aver = 0;
	s3c_bat_info.bat_info.batt_temp_aver = 0;
	s3c_bat_info.bat_info.batt_temp_adc_aver = 0;

	s3c_bat_info.bat_info.batt_test_mode = 0;
 	s3c_power_supplies_test = s3c_power_supplies;
#endif /* __TEST_MODE_INTERFACE__ */

	s3c_bat_info.bat_info.batt_id = 0;
	s3c_bat_info.bat_info.batt_vol = 0;
	s3c_bat_info.bat_info.batt_temp = 0;
	s3c_bat_info.bat_info.batt_temp_adc = 0;
	s3c_bat_info.bat_info.batt_temp_adc_cal = 0;
	s3c_bat_info.bat_info.batt_current = 0;
	s3c_bat_info.bat_info.level = 100;
	s3c_bat_info.bat_info.charging_source = CHARGER_BATTERY;
	s3c_bat_info.bat_info.charging_enabled = 0;
	s3c_bat_info.bat_info.batt_health = POWER_SUPPLY_HEALTH_GOOD;

	s3c_bat_info.bat_info.decimal_point_level = 1;	// lobat pwroff

//	memset(adc_sample, 0x00, sizeof adc_sample);

	INIT_WORK(&bat_work, s3c_bat_work);
	INIT_WORK(&cable_work, s3c_cable_work);

	/* init power supplier framework */
	for (i = 0; i < ARRAY_SIZE(s3c_power_supplies); i++) {
		ret = power_supply_register(&pdev->dev, 
				&s3c_power_supplies[i]);
		if (ret) {
			dev_err(dev, "Failed to register"
					"power supply %d,%d\n", i, ret);
			goto __end__;
		}
	}

	/* create sec detail attributes */
	s3c_bat_create_attrs(s3c_power_supplies[CHARGER_BATTERY].dev);

	if (s3c_bat_info.polling) {
		setup_timer(&polling_timer, polling_timer_func, 0);
		mod_timer(&polling_timer,
			  jiffies + msecs_to_jiffies(s3c_bat_info.polling_interval));
	}

	setup_timer(&use_4g_timer, use_4g_timer_func, 0);	// hanapark_Victory_DF29
	setup_timer(&use_data_call_timer, use_data_call_timer_func, 0);	// hanapark_Victory_DF29
	setup_timer(&use_browser_timer, use_browser_timer_func, 0);	// hanapark_Victory_DF29

	s3c_battery_initial = 1;
	force_update = 0;

	check_quick_start();

	fuel_gauge_rcomp(RCOMP_MSB_VALUE, RCOMP_LSB_VALUE);

#ifdef CONFIG_WIRELESS_CHARGING
	s3c_gpio_cfgpin(GPIO_WC_DETECT, S3C_GPIO_INPUT);
	s3c_gpio_setpull(GPIO_WC_DETECT, S3C_GPIO_PULL_NONE);
#endif

	s3c_bat_status_update(
			&s3c_power_supplies[CHARGER_BATTERY]);

	s3c_bat_check_v_f();
	s3c_cable_check_status();

	/* Request IRQ */ 
	MAX8998_IRQ_init();

	if(charging_mode_get())
	{
		lpm_mode_check();	// Forced power off if charger is disconnected before boot is complete.
		gpio_set_value(GPIO_PHONE_ON, 0); //prevent phone reset when AP off

		battery = kzalloc(sizeof(struct battery_driver), GFP_KERNEL);
		battery->early_suspend.level = EARLY_SUSPEND_LEVEL_DISABLE_FB + 1;
		battery->early_suspend.suspend = battery_early_suspend;
		battery->early_suspend.resume = battery_late_resume;
		register_early_suspend(&battery->early_suspend);
	}

__end__:
	return ret;
}

static int __devexit s3c_bat_remove(struct platform_device *pdev)
{
	int i;
	dev_dbg(dev, "%s\n", __func__);

	if (s3c_bat_info.polling)
		del_timer_sync(&polling_timer);

	for (i = 0; i < ARRAY_SIZE(s3c_power_supplies); i++) {
		power_supply_unregister(&s3c_power_supplies[i]);
	}
 
	return 0;
}

static struct platform_driver s3c_bat_driver = {
	.driver.name	= DRIVER_NAME,
	.driver.owner	= THIS_MODULE,
	.probe		= s3c_bat_probe,
	.remove		= __devexit_p(s3c_bat_remove),
	.suspend	= s3c_bat_suspend,
	.resume		= s3c_bat_resume,
};

static int __init s3c_bat_init(void)
{
	dev_dbg(dev, "%s\n", __func__);

	wake_lock_init(&vbus_wake_lock, WAKE_LOCK_SUSPEND, "vbus_present");
	wake_lock_init(&low_batt_wake_lock, WAKE_LOCK_SUSPEND, "low_batt_detected");	// lobat pwroff

	fg_init();

	return platform_driver_register(&s3c_bat_driver);
}

static void __exit s3c_bat_exit(void)
{
	dev_dbg(dev, "%s\n", __func__);

	fg_exit();
	platform_driver_unregister(&s3c_bat_driver);
}

#define USE_MODULE_TIMEOUT	(10*60*1000)

static void use_4g_timer_func(unsigned long unused)
{
	s3c_bat_info.device_state = s3c_bat_info.device_state & (~USE_4G);
	printk("%s: OFF (0x%x) \n", __func__, s3c_bat_info.device_state);
}

int s3c_bat_use_4g(int onoff)	
{
	if (onoff)
	{	
		del_timer_sync(&use_4g_timer);
		s3c_bat_info.device_state = s3c_bat_info.device_state | USE_4G;
		printk("%s: ON (0x%x) \n", __func__, s3c_bat_info.device_state);
	}
	else
	{
		mod_timer(&use_4g_timer, jiffies + msecs_to_jiffies(USE_MODULE_TIMEOUT));
	}

	return s3c_bat_info.device_state;
}
EXPORT_SYMBOL(s3c_bat_use_4g);

static void use_browser_timer_func(unsigned long unused)
{
	s3c_bat_info.device_state = s3c_bat_info.device_state & (~USE_BROWSER);
	printk("%s: OFF (0x%x) \n", __func__, s3c_bat_info.device_state);
}

static int s3c_bat_use_browser(int onoff)	
{
	if (onoff)
	{	
		del_timer_sync(&use_browser_timer);
		s3c_bat_info.device_state = s3c_bat_info.device_state | USE_BROWSER;
		printk("%s: ON (0x%x) \n", __func__, s3c_bat_info.device_state);
	}
	else
	{
		mod_timer(&use_browser_timer, jiffies + msecs_to_jiffies(USE_MODULE_TIMEOUT));
	}

	return s3c_bat_info.device_state;
}

static int data_call_off_request = 0;	// DG09

static void use_data_call_timer_func(unsigned long unused)
{
	s3c_bat_info.device_state = s3c_bat_info.device_state & (~USE_DATA_CALL);
	printk("%s: OFF (0x%x) \n", __func__, s3c_bat_info.device_state);
}

static int s3c_bat_use_data_call(int onoff)	
{
	if (onoff)
	{
		data_call_off_request = 0;
		del_timer_sync(&use_data_call_timer);
		s3c_bat_info.device_state = s3c_bat_info.device_state | USE_DATA_CALL;
		printk("%s: ON (0x%x) \n", __func__, s3c_bat_info.device_state);
	}
	else
	{
		if (data_call_off_request == 0)
		{
			data_call_off_request = 1;
			mod_timer(&use_data_call_timer, jiffies + msecs_to_jiffies(USE_MODULE_TIMEOUT));
			printk("%s: OFF waiting (0x%x) \n", __func__, s3c_bat_info.device_state);
		}
	}

	return s3c_bat_info.device_state;
}


late_initcall(s3c_bat_init);
module_exit(s3c_bat_exit);

MODULE_AUTHOR("Minsung Kim <ms925.kim@samsung.com>");
MODULE_DESCRIPTION("S3C6410 battery driver");
MODULE_LICENSE("GPL");
