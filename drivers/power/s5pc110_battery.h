/*
 * linux/drivers/power/s3c6410_battery.h
 *
 * Battery measurement code for S3C6410 platform.
 *
 * Copyright (C) 2009 Samsung Electronics.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#define DRIVER_NAME	"sec-battery"

/*
 * AriesQ Rev00 board Temperature Table
 */
const int temper_table[][2] =  {
	/* ADC, Temperature (C) */
	{ 630,		-70	},
	{ 635,		-60	},
	{ 640,		-50	},
	{ 645,		-40	},
	{ 650,		-30	},
	{ 655,		-20	},
	{ 660,		-10	},
	{ 665,		0	},
	{ 670,		10	},
	{ 675,		20	},
	{ 680,		30	},
	{ 685,		40	},
	{ 690,		50	},
	{ 695,		60	},
	{ 700,		70	},
	{ 705,		80	},
	{ 710,		90	},
	{ 715,		100	},
	{ 720,		110	},
	{ 725,		120	},
	{ 730,		130	},
	{ 735,		140	},
	{ 740,		150	},
	{ 745,		160	},
	{ 750,		170	},
	{ 755,		180	},
	{ 760,		190	},
	{ 765,		200	},
	{ 770,		210	},
	{ 775,		220	},
	{ 780,		230	},
	{ 785,		240	},
	{ 790,		250	},
	{ 795,		260	},
	{ 800,		270	},
	{ 805,		280	},
	{ 810,		290	},
	{ 815,		300	},
	{ 820,		310	},
	{ 825,		320	},
	{ 830,		330	},
	{ 835,		340	},
	{ 840,		350	},
	{ 845,		360	},
	{ 850,		370	},
	{ 855,		380	},
	{ 860,		390	},
	{ 865,		400	},
	{ 870,		410	},
	{ 875,		420	},
	{ 880,		430	},
	{ 885,		440	},
	{ 890,		450	},
	{ 895,		460	},
	{ 900,		470	},
	{ 905,		480	},
	{ 910,		490	},
	{ 915,		500	},
	{ 920,		510	},
	{ 925,		520	},
	{ 930,		530	},
	{ 935,		540	},
	{ 940,		550	},
	{ 945,		560	},
	{ 950,		570	},
	{ 955,		580	},
	{ 960,		590	},
	{ 965,		600	},
	{ 970,		610	},
	{ 975,		620	},
	{ 980,		630	},
	{ 985,		640	},
	{ 990,		650	},
};


/*
-. 고온 차단(45도)	: 915
-. 고온 복귀(43도)	: 890
-. 저온 복귀( 0도)	: 646
-. 저온 차단(-5도)	: 626 
-. 이벤트 차단(65도): 970 
*/

#define TEMP_HIGH_BLOCK			915
#define TEMP_HIGH_RECOVER		890
#define TEMP_LOW_RECOVER		646
#define TEMP_LOW_BLOCK			626

#define TEMP_EVENT_HIGH_BLOCK			970


/*
 * AriesQ Rev00 board ADC channel
 */
typedef enum s3c_adc_channel {
	S3C_ADC_BATT_MONITOR = 1,
	S3C_ADC_CHG_CURRENT = 2,
	S3C_ADC_EAR = 3,
	S3C_ADC_WC = 4,
	S3C_ADC_TEMPERATURE = 6,
	ENDOFADC
} adc_channel_type;


/******************************************************************************
 * Battery driver features
 * ***************************************************************************/

#define __VZW_AUTH_CHECK__
#define __ADJUST_RECHARGE_ADC__
#define __TEST_MODE_INTERFACE__
#define __CHECK_CHG_CURRENT__
#define __ADJUST_ADC_VALUE__
#define __TEMP_BLOCK_EXCEPTION__

//#define __TEMPERATURE_TEST__
//#define __MANUAL_TEMP_TEST__
//#define __SOC_TEST__
//#define __FULL_CHARGE_TEST__

/*****************************************************************************/

#ifdef __FULL_CHARGE_TEST__
#define TOTAL_CHARGING_TIME	(1*60*1000)	/* 1 min */
#else
#define TOTAL_CHARGING_TIME	(6*60*60*1000)	/* 6 hours */
#endif
#define TOTAL_RECHARGING_TIME	(2*60*60*1000)	/* 2 hours */

#ifdef __ADJUST_RECHARGE_ADC__
#define BATT_RECHARGE_CODE	0	// hanapark (fix compile error)
#define BATT_RECHARGE_COUNT	20
#endif

#define FULL_CHARGE_COND_VOLTAGE	4000
#define RECHARGE_COND_VOLTAGE		4120
#define RECHARGE_COND_VOLTAGE_BACKUP		4000

#define LOW_BATT_COUNT	60
#define LOW_BATT_COND_VOLTAGE		3400
#define LOW_BATT_COND_LEVEL			0

#ifdef __CHECK_CHG_CURRENT__
#define CURRENT_OF_FULL_CHG  80
#define CHG_CURRENT_COUNT		20
#endif

