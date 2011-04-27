/* linux/drivers/input/keyboard/s3c-keypad.h 
 *
 * Driver header for Samsung SoC keypad.
 *
 * Kim Kyoungil, Copyright (c) 2006-2009 Samsung Electronics
 *      http://www.samsungsemi.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#ifndef _S3C_KEYPAD_H_
#define _S3C_KEYPAD_H_

static void __iomem *key_base;


#if defined (CONFIG_MACH_S5PC110_P1P2)

#define KEYPAD_COLUMNS	14	
#define KEYPAD_ROWS	8
#define MAX_KEYPAD_NR	112
#err_1


#elif defined (CONFIG_MACH_S5PC110_ARIES)

 #define KEYPAD_COLUMNS       2
 #define KEYPAD_ROWS    4
 #define MAX_KEYPAD_NR  8
//#err_2 xxxxx

#elif defined (CONFIG_MACH_SMDKC110 ) || defined (CONFIG_MACH_S5PC110_P1)

#define KEYPAD_COLUMNS	8
#define KEYPAD_ROWS	8
#define MAX_KEYPAD_NR	64	/* 8*8 */
#err_3

#endif




#ifdef CONFIG_ANDROID
int keypad_keycode[] = {

#if defined (CONFIG_MACH_S5PC110_P1P2)
		0,  15,  399,   2,  16,  30,   0,   0,     //7
		0,   0,    0,   0,   0,   0, 100,  56,	  	//15			
		0,   0,   58,   4,  18,  32,  46,  57,	        //23			
		0,   0,    1,   3,  17,  31,  45,  44,	        //31			
		0,   0,    0,   0,   0,   0,   0,   0,	        //39			
		34,  20,   6,   5,  19,  33,  47,  48,	        //47			
		35,  21,   7,   8,  22,  36,  50,  49,          //55				
		56,   0,   0,  10,  24,  38,  52, 108,         //63			
		0,    0,   0,   0,  54,  42,   0,   0,	        //71			
		105,  0,   0,   9,  23,  37,  51,   0,	        //79			
		0,   97,  29,   0,   0,   0,   0,   0,	        //87			
		0,    0,   0, 150,  0,    0,   0,   0,	        //95			
		12,  106, 11,  25,  26,  39,  53, 103,      //103				
		14,  106,  3,  14,  27,  43,  40,  28,      //111			

#err_4

#elif defined (CONFIG_MACH_S5PC110_ARIES)
        57,34,50,49,0,58,57,3
//#err_5 xxxxx

#elif defined (CONFIG_MACH_SMDKC110)
		1,2,3,4,5,6,7,8,
		9,10,11,12,13,14,15,16,
		17,18,19,20,21,22,23,24,
		25,26,27,28,29,30,31,32,
		33,34,35,36,37,38,39,40,
		41,42,43,44,45,46,47,48,
		49,50,51,52,53,54,55,56,
		57,58,59,60,61,62,63,64

#endif
};

#else
int keypad_keycode[] = {
		1, 2, KEY_1, KEY_Q, KEY_A, 6, 7, KEY_LEFT,
		9, 10, KEY_2, KEY_W, KEY_S, KEY_Z, KEY_RIGHT, 16,
		17, 18, KEY_3, KEY_E, KEY_D, KEY_X, 23, KEY_UP,
		25, 26, KEY_4, KEY_R, KEY_F, KEY_C, 31, 32,
		33, KEY_O, KEY_5, KEY_T, KEY_G, KEY_V, KEY_DOWN, KEY_BACKSPACE,
		KEY_P, KEY_0, KEY_6, KEY_Y, KEY_H, KEY_SPACE, 47, 48,
		KEY_M, KEY_L, KEY_7, KEY_U, KEY_J, KEY_N, 55, KEY_ENTER,
		KEY_LEFTSHIFT, KEY_9, KEY_8, KEY_I, KEY_K, KEY_B, 63, KEY_COMMA
	};
#err_6

#endif

#if CONFIG_ANDROID
#ifdef CONFIG_CPU_S3C6410
#define KEYPAD_ROW_GPIOCON      S3C64XX_GPKCON
#define KEYPAD_ROW_GPIOPUD      S3C64XX_GPKPUD
#define KEYPAD_COL_GPIOCON      S3C64XX_GPLCON
#define KEYPAD_COL_GPIOPUD      S3C64XX_GPLPUD
#elif defined( CONFIG_CPU_S5PC100 )
#define KEYPAD_ROW_GPIOCON      S5PC1XX_GPH3CON
#define KEYPAD_ROW_GPIOPUD      S5PC1XX_GPH3PUD
#define KEYPAD_COL_GPIOCON      S5PC1XX_GPH2CON
#define KEYPAD_COL_GPIOPUD      S5PC1XX_GPH2PUD
#elif /*defined( CONFIG_CPU_S5PC110 ) || */defined (CONFIG_CPU_S5PV210 )
//#err_7 xxxxx

#define KEYPAD_ROW_GPIOCON      S5PV210_GPH3CON
#define KEYPAD_ROW_GPIOPUD      S5PV210_GPH3PUD
#define KEYPAD_COL_GPIOCON      S5PV210_GPH2CON
#define KEYPAD_COL_GPIOPUD      S5PV210_GPH2PUD
#endif
#endif /* CONFIG_ANDROID */

#if defined(CONFIG_CPU_S3C6410)
#define KEYPAD_DELAY		(50)
#elif defined(CONFIG_CPU_S5PC100) || defined(CONFIG_CPU_S5PC110) || defined(CONFIG_CPU_S5PV210)
#define KEYPAD_DELAY		(300)  //600
#endif

#define	KEYIFCOL_CLEAR		(readl(key_base+S3C_KEYIFCOL) & ~0xffff)
#define	KEYIFCON_CLEAR		(readl(key_base+S3C_KEYIFCON) & ~0x1f)
#define KEYIFFC_DIV		(readl(key_base+S3C_KEYIFFC) | 0x1)

#define KEYCODE_UNKNOWN 10 

enum
{
	KEYGPIO_VOLUME_UP = 0,
	KEYGPIO_POWER,
	KEYGPIO_MAX,
};

typedef struct _keygpio_info {
	int irq;
	int gpio;
	int gpio_data;
	int gpio_data_mask;

	int keycode;
	int prev_key_status;
} KEYGPIO_INFO;

typedef struct _keygpio_int_info {
	int					index;
	KEYGPIO_INFO *		info;
	struct input_dev *	dev;
} KEYGPIO_INT_INFO;

struct s3c_keypad {
	struct input_dev *dev;
	int nr_rows;	
	int no_cols;
	int total_keys; 
	int keycodes[MAX_KEYPAD_NR];

	struct timer_list	keypad_timer;
	unsigned int		keypad_timer_on;	

	/* for gpio keys */
	KEYGPIO_INFO		keygpio_info_tbl[KEYGPIO_MAX];
	KEYGPIO_INT_INFO	keygpio_int_info_tbl[KEYGPIO_MAX];
};

extern void s3c_setup_keypad_cfg_gpio(int rows, int columns);

#endif				/* _S3C_KEYIF_H_ */
