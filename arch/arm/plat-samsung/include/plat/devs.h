/* arch/arm/plat-samsung/include/plat/devs.h
 *
 * Copyright (c) 2004 Simtec Electronics
 * Ben Dooks <ben@simtec.co.uk>
 *
 * Header file for s3c2410 standard platform devices
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#include <linux/platform_device.h>

struct s3c24xx_uart_resources {
	struct resource		*resources;
	unsigned long		 nr_resources;
};

extern struct s3c24xx_uart_resources s3c2410_uart_resources[];
extern struct s3c24xx_uart_resources s3c64xx_uart_resources[];
extern struct s3c24xx_uart_resources s5p_uart_resources[];

extern struct platform_device *s3c24xx_uart_devs[];
extern struct platform_device *s3c24xx_uart_src[];

extern struct platform_device s3c24xx_uart_device0;
extern struct platform_device s3c24xx_uart_device1;
extern struct platform_device s3c24xx_uart_device2;
extern struct platform_device s3c24xx_uart_device3;

extern struct platform_device s5pv210_device_fiqdbg_uart0;
extern struct platform_device s5pv210_device_fiqdbg_uart1;
extern struct platform_device s5pv210_device_fiqdbg_uart2;
extern struct platform_device s5pv210_device_fiqdbg_uart3;

extern struct platform_device s3c_device_timer[];

extern struct platform_device s3c64xx_device_iis0;
extern struct platform_device s3c64xx_device_iis1;
extern struct platform_device s3c64xx_device_iisv4;

extern struct platform_device s3c64xx_device_spi0;
extern struct platform_device s3c64xx_device_spi1;

extern struct platform_device s3c64xx_device_pcm0;
extern struct platform_device s3c64xx_device_pcm1;

extern struct platform_device s3c64xx_device_ac97;

extern struct platform_device s3c_device_ts;

extern struct platform_device s3c_device_fb;

extern struct platform_device s3c_device_fimc0;
extern struct platform_device s3c_device_fimc1;
extern struct platform_device s3c_device_fimc2;
extern struct platform_device s3c_device_csis;
extern struct platform_device s3c_device_ipc;
extern struct platform_device s3c_device_mfc;
extern struct platform_device s3c_device_jpeg;
extern struct platform_device s3c_device_g3d;

extern struct platform_device s3c_device_ohci;
extern struct platform_device s3c_device_lcd;
extern struct platform_device s3c_device_wdt;
extern struct platform_device s3c_device_i2c0;
extern struct platform_device s3c_device_i2c1;
extern struct platform_device s3c_device_i2c2;
extern struct platform_device s3c_device_rtc;
extern struct platform_device s3c_device_adc;
extern struct platform_device s3c_device_sdi;
extern struct platform_device s3c_device_iis;
extern struct platform_device s3c_device_hwmon;
extern struct platform_device s3c_device_hsmmc0;
extern struct platform_device s3c_device_hsmmc1;
extern struct platform_device s3c_device_hsmmc2;
extern struct platform_device s3c_device_hsmmc3;

extern struct platform_device s3c_device_spi0;
extern struct platform_device s3c_device_spi1;

extern struct platform_device s5pc100_device_spi0;
extern struct platform_device s5pc100_device_spi1;
extern struct platform_device s5pc100_device_spi2;
extern struct platform_device s5pv210_device_spi0;
extern struct platform_device s5pv210_device_spi1;
extern struct platform_device s5p6440_device_spi0;
extern struct platform_device s5p6440_device_spi1;

extern struct platform_device s3c_device_hwmon;
extern struct platform_device s3c_device_keypad;

extern struct platform_device s3c_device_nand;
extern struct platform_device s3c_device_onenand;
extern struct platform_device s3c64xx_device_onenand1;
extern struct platform_device s5pc110_device_onenand;

extern struct platform_device s3c_device_usbgadget;
extern struct platform_device s3c_device_android_usb;
extern struct platform_device s3c_device_usb_mass_storage;

#ifdef CONFIG_USB_ANDROID_SAMSUNG_COMPOSITE
/* soonyong.cho : Define samsung product id and config string.
 *                Sources such as 'android.c' and 'devs.c' refered below define
 */
#  define SAMSUNG_VENDOR_ID		0x04e8

#  ifdef CONFIG_USB_ANDROID_SAMSUNG_ESCAPE
	/* USE DEVGURU HOST DRIVER */
	/* 0x6860 : MTP(0) + MS Composite (UMS) */
	/* 0x685E : UMS(0) + MS Composite (ADB) */
#    define SAMSUNG_KIES_PRODUCT_ID	0x685d	/* acm(0,1) + mtp */
#    define SAMSUNG_DEBUG_PRODUCT_ID	0x685d	/* acm(0,1) + ums + adb */
#    define SAMSUNG_UMS_PRODUCT_ID	0x685B  /* UMS Only */
#    define SAMSUNG_MTP_PRODUCT_ID	0x685C  /* MTP Only */
#    ifdef CONFIG_USB_ANDROID_SAMSUNG_RNDIS_WITH_MS_COMPOSITE
#      define SAMSUNG_RNDIS_PRODUCT_ID	0x6861  /* RNDIS(0,1) + UMS (2) + MS Composite */
#    else
#      define SAMSUNG_RNDIS_PRODUCT_ID	0x6863  /* RNDIS only */
#    endif
#  else /* USE MCCI HOST DRIVER */
#    define SAMSUNG_KIES_PRODUCT_ID	0x6877	/* Shrewbury ACM+MTP */
#ifdef CONFIG_USB_ANDROID_DIAG
	#define SAMSUNG_DEBUG_PRODUCT_ID    0x689E  /* Shrewbury ACM+DM+UMS+ADB */
#else
	#define SAMSUNG_DEBUG_PRODUCT_ID	0x681C	/* Shrewbury ACM+UMS+ADB */
#endif
#    define SAMSUNG_UMS_PRODUCT_ID	0x681D
#    define SAMSUNG_MTP_PRODUCT_ID	0x5A0F
#if defined(CONFIG_USB_ANDROID_DIAG) && defined(CONFIG_USB_ANDROID_NMEA)
        #define SAMSUNG_RNDIS_PRODUCT_ID	0x68C4  /* Shrewbury RNDIS+DM+DM */
#elif defined(CONFIG_USB_ANDROID_DIAG)
	#define SAMSUNG_RNDIS_PRODUCT_ID	0x68C8  /* Shrewbury RNDIS+DM */
#else
	#define SAMSUNG_RNDIS_PRODUCT_ID	0x6881 /* Shrewbury RNDIS only */
#endif
#  endif
#  define       ANDROID_DEBUG_CONFIG_STRING	 "ACM + UMS + ADB (Debugging mode)"
#  define       ANDROID_KIES_CONFIG_STRING	 "ACM + MTP (SAMSUNG KIES mode)"
#  define       ANDROID_UMS_CONFIG_STRING	 "UMS Only (Not debugging mode)"
#  define       ANDROID_MTP_CONFIG_STRING	 "MTP Only (Not debugging mode)"
#ifdef CONFIG_USB_ANDROID_ACCESSORY
#  define       ANDORID_ACCESSORY_CONFIG_STRING "Google Accessory mode"
#  define       ANDORID_ACCESSORY_ADB_CONFIG_STRING "Google Accessory debugging mode"
#endif
#  ifdef CONFIG_USB_ANDROID_SAMSUNG_RNDIS_WITH_MS_COMPOSITE
#    define       ANDROID_RNDIS_CONFIG_STRING	 "RNDIS + UMS (Not debugging mode)"
#  else
#    define       ANDROID_RNDIS_CONFIG_STRING	 "RNDIS Only (Not debugging mode)"
#  endif
	/* Refered from S1, P1 */
#  define USBSTATUS_UMS				0x0
#  define USBSTATUS_SAMSUNG_KIES 		0x1
#  define USBSTATUS_MTPONLY			0x2
#  define USBSTATUS_ASKON			0x4
#  define USBSTATUS_VTP				0x8
#  define USBSTATUS_ADB				0x10
#  define USBSTATUS_DM				0x20
#  define USBSTATUS_ACM				0x40
#  define USBSTATUS_SAMSUNG_KIES_REAL		0x80
#ifdef CONFIG_USB_ANDROID_ACCESSORY
#  define USBSTATUS_ACCESSORY    0x100
#  define USBSTATUS_ACCESSORY_ADB    0x200
#endif

/* soonyong.cho : This is for setting unique serial number */
void __init s3c_usb_set_serial(void);
#endif /* CONFIG_USB_ANDROID_SAMSUNG_COMPOSITE */

extern struct platform_device s3c_device_rndis;
extern struct platform_device s3c_device_usb_hsotg;

extern struct platform_device s5p_device_rotator;
extern struct platform_device s5p_device_tvout;
extern struct platform_device s5p_device_cec;
extern struct platform_device s5p_device_hpd;
extern struct platform_device s5p_device_g3d;

extern struct platform_device s5pv210_device_ac97;
extern struct platform_device s5pv210_device_pcm0;
extern struct platform_device s5pv210_device_pcm1;
extern struct platform_device s5pv210_device_pcm2;
extern struct platform_device s5pv210_device_iis0;
extern struct platform_device s5pv210_device_iis1;
extern struct platform_device s5pv210_device_iis2;

extern struct platform_device s5p6442_device_pcm0;
extern struct platform_device s5p6442_device_pcm1;
extern struct platform_device s5p6442_device_iis0;
extern struct platform_device s5p6442_device_iis1;
extern struct platform_device s5p6442_device_spi;

extern struct platform_device s5p6440_device_pcm;
extern struct platform_device s5p6440_device_iis;

extern struct platform_device s5pc100_device_ac97;
extern struct platform_device s5pc100_device_pcm0;
extern struct platform_device s5pc100_device_pcm1;
extern struct platform_device s5pc100_device_iis0;
extern struct platform_device s5pc100_device_iis1;
extern struct platform_device s5pc100_device_iis2;
extern struct platform_device s5p_device_rtc;

extern struct platform_device s3c_device_adc;
/* s3c2440 specific devices */

extern struct platform_device s5pv210_device_pdma0;
extern struct platform_device s5pv210_device_pdma1;
extern struct platform_device s5pv210_device_mdma;

#ifdef CONFIG_CPU_S3C2440

extern struct platform_device s3c_device_camif;
extern struct platform_device s3c_device_ac97;

#endif

#ifdef CONFIG_SND_S5P_RP
extern struct platform_device s5p_device_rp;
#endif
void __init s3c_usb_set_serial(void);

extern struct platform_device s5p_device_ace;
