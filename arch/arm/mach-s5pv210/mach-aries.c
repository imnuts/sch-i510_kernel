/* linux/arch/arm/mach-s5pv210/mach-aries.c
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/gpio.h>
#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/i2c/qt602240_ts.h>
#include <linux/regulator/max8998.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/usb/ch9.h>
#include <linux/pwm_backlight.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_gpio.h>
#include <linux/reboot.h>
#include <linux/sec_jack.h>

#include <linux/i2c/l3g4200d.h>

#ifdef CONFIG_KERNEL_DEBUG_SEC
#include <linux/kernel_sec_common.h>
#endif

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/setup.h>
#include <asm/mach-types.h>

#include <mach/map.h>
#include <mach/regs-clock.h>
#include <mach/regs-mem.h>
#include <mach/regs-gpio.h>
#include <mach/gpio-bank.h>
#include <mach/adcts.h>
#include <mach/ts.h>
#include <mach/adc.h>
#include <media/ce147_platform.h>
#include <media/s5ka3dfx_platform.h>
#include <media/s5k5aafa_platform.h>

#include <plat/regs-serial.h>
#include <plat/s5pv210.h>
#include <plat/devs.h>
#include <plat/cpu.h>
#include <plat/fb.h>
#include <plat/gpio-cfg.h>
#include <plat/iic.h>

#include <plat/fimc.h>
#include <plat/regs-fimc.h>
#include <plat/csis.h>
#include <plat/mfc.h>
#include <plat/sdhci.h>
#include <plat/ide.h>
#include <plat/regs-otg.h>
#include <plat/clock.h>
#include <plat/gpio-core.h>

#include <mach/gpio.h>
#include <mach/gpio-aries-setting.h>

#ifdef CONFIG_ANDROID_PMEM
#include <linux/android_pmem.h>
#include <plat/media.h>
#endif

#if defined(CONFIG_PM)
#include <plat/pm.h>
#endif
#include <mach/max8998_function.h>
#include <mach/param.h>

#if defined (CONFIG_SAMSUNG_PHONE_SVNET) || defined (CONFIG_SAMSUNG_PHONE_SVNET_MODULE)
#include <linux/modemctl.h>
#include <linux/onedram.h>
#include <linux/irq.h>
#endif 

#include <linux/skbuff.h>


struct class *sec_class;
EXPORT_SYMBOL(sec_class);

struct device *switch_dev;
EXPORT_SYMBOL(switch_dev);

struct device *gps_dev = NULL;
EXPORT_SYMBOL(gps_dev);

void (*sec_set_param_value)(int idx, void *value);
EXPORT_SYMBOL(sec_set_param_value);
        
void (*sec_get_param_value)(int idx, void *value);
EXPORT_SYMBOL(sec_get_param_value);

unsigned int HWREV=0;
EXPORT_SYMBOL(HWREV);

static void jupiter_switch_init(void)
{
	sec_class = class_create(THIS_MODULE, "sec");
	
	if (IS_ERR(sec_class))
		pr_err("Failed to create class(sec)!\n");

	switch_dev = device_create(sec_class, NULL, 0, NULL, "switch");
	
	if (IS_ERR(switch_dev))
		pr_err("Failed to create device(switch)!\n");

};

/* Following are default values for UCON, ULCON and UFCON UART registers */
#define S5PV210_UCON_DEFAULT	(S3C2410_UCON_TXILEVEL |	\
				 S3C2410_UCON_RXILEVEL |	\
				 S3C2410_UCON_TXIRQMODE |	\
				 S3C2410_UCON_RXIRQMODE |	\
				 S3C2410_UCON_RXFIFO_TOI |	\
				 S3C2443_UCON_RXERR_IRQEN)

#define S5PV210_ULCON_DEFAULT	S3C2410_LCON_CS8

#define S5PV210_UFCON_DEFAULT	(S3C2410_UFCON_FIFOMODE |	\
				 S5PV210_UFCON_TXTRIG4 |	\
				 S5PV210_UFCON_RXTRIG4)

extern void s5pv210_reserve_bootmem(void);
extern void s3c_sdhci_set_platdata(void);

static struct s3c2410_uartcfg smdkv210_uartcfgs[] __initdata = {
	[0] = {
		.hwport		= 0,
		.flags		= 0,
		.ucon		= S5PV210_UCON_DEFAULT,
		.ulcon		= S5PV210_ULCON_DEFAULT,
		.ufcon		= S5PV210_UFCON_DEFAULT,
	},
	[1] = {
		.hwport		= 1,
		.flags		= 0,
		.ucon		= S5PV210_UCON_DEFAULT,
		.ulcon		= S5PV210_ULCON_DEFAULT,
		.ufcon		= S5PV210_UFCON_DEFAULT,
	},
	[2] = {
		.hwport		= 2,
		.flags		= 0,
		.ucon		= S5PV210_UCON_DEFAULT,
		.ulcon		= S5PV210_ULCON_DEFAULT,
		.ufcon		= S5PV210_UFCON_DEFAULT,
	},
	[3] = {
		.hwport		= 3,
		.flags		= 0,
		.ucon		= S5PV210_UCON_DEFAULT,
		.ulcon		= S5PV210_ULCON_DEFAULT,
		.ufcon		= S5PV210_UFCON_DEFAULT,
	},
};


#if defined(CONFIG_TOUCHSCREEN_QT602240)
static struct platform_device s3c_device_qtts = {
        .name = "qt602240-ts",
        .id = -1,
};
#endif


/* PMIC */
static struct regulator_consumer_supply dcdc1_consumers[] = {
        {
                .supply         = "vddarm",
        },
};

static struct regulator_init_data max8998_dcdc1_data = {
        .constraints    = {
                .name           = "VCC_ARM",
                .min_uV         =  750000,
                .max_uV         = 1500000,
                .always_on      = 1,
                .valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
        },
        .num_consumer_supplies  = ARRAY_SIZE(dcdc1_consumers),
        .consumer_supplies      = dcdc1_consumers,
};

static struct regulator_consumer_supply dcdc2_consumers[] = {
        {
                .supply         = "vddint",
        },
};

static struct regulator_init_data max8998_dcdc2_data = {
        .constraints    = {
                .name           = "VCC_INTERNAL",
                .min_uV         =  750000,
                .max_uV         = 1500000,
                .always_on      = 1,
//              .apply_uV       = 1,
                .valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
        },
        .num_consumer_supplies  = ARRAY_SIZE(dcdc2_consumers),
        .consumer_supplies      = dcdc2_consumers,
};
static struct regulator_init_data max8998_ldo4_data = {
        .constraints    = {
                .name           = "VCC_ADC",
                .min_uV         = 3300000,
                .max_uV         = 3300000,
                .always_on      = 1,
                .apply_uV       = 1,
                .valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
        },
};




static struct regulator_init_data max8998_ldo7_data = {
        .constraints    = {
                .name           = "VCC_LCD",
                .min_uV         = 1600000,
                .max_uV         = 3600000,
                .always_on      = 1,
                //.apply_uV     = 1,
                .valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
        },
};

static struct regulator_init_data max8998_ldo17_data = {
        .constraints    = {
                .name           = "PM_LVDS_VDD",
                .min_uV         = 1600000,
                .max_uV         = 3600000,
                .always_on      = 1,
                //.apply_uV     = 1,
                .valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
        },
};
static struct max8998_subdev_data universal_regulators[] = {
        { MAX8998_DCDC1, &max8998_dcdc1_data },
        { MAX8998_DCDC2, &max8998_dcdc2_data },
/*	{ MAX8998_DCDC4, &max8998_dcdc4_data },	*/
/*	{ MAX8998_LDO4, &max8998_ldo4_data },	*/
/*	{ MAX8998_LDO11, &max8998_ldo11_data },	*/
/*	{ MAX8998_LDO12, &max8998_ldo12_data },	*/
/*	{ MAX8998_LDO13, &max8998_ldo13_data },	*/
/*	{ MAX8998_LDO14, &max8998_ldo14_data },	*/
/*	{ MAX8998_LDO15, &max8998_ldo15_data },	*/
        { MAX8998_LDO7, &max8998_ldo7_data },
        { MAX8998_LDO17, &max8998_ldo17_data },
};

static struct max8998_platform_data max8998_platform_data = {
        .num_regulators = ARRAY_SIZE(universal_regulators),
        .regulators     = universal_regulators,
};

#if 0
/* I2C2 */
static struct i2c_board_info i2c_devs2[] __initdata = {
        {
                /* The address is 0xCC used since SRAD = 0 */
                I2C_BOARD_INFO("max8998", (0xCC >> 1)),
                .platform_data = &max8998_platform_data,
        },
};
#endif

/*
#if defined (CONFIG_SAMSUNG_PHONE_TTY) || defined (CONFIG_SAMSUNG_PHONE_TTY_MODULE)
struct platform_device sec_device_dpram = {
        .name   = "dpram-device",
        .id             = -1,
};
#endif
*/

struct platform_device sec_device_dpram = {
        .name   = "dpram-device",
        .id             = -1,
};

struct platform_device s3c_device_8998consumer = {
        .name             = "max8998-consumer",
        .id               = 0,
        .dev = { .platform_data = &max8998_platform_data },
};


static void tl2796_cfg_gpio(struct platform_device *pdev)
{
	int i;

	/* DISPLAY_HSYNC */
	s3c_gpio_cfgpin(GPIO_DISPLAY_HSYNC, GPIO_DISPLAY_HSYNC_AF);
	s3c_gpio_setpull(GPIO_DISPLAY_HSYNC, S3C_GPIO_PULL_NONE);

	/* DISPLAY_VSYNC */
	s3c_gpio_cfgpin(GPIO_DISPLAY_VSYNC, GPIO_DISPLAY_VSYNC_AF);
	s3c_gpio_setpull(GPIO_DISPLAY_VSYNC, S3C_GPIO_PULL_NONE);

	/* DISPLAY_DE */
	s3c_gpio_cfgpin(GPIO_DISPLAY_DE, GPIO_DISPLAY_DE_AF);
	s3c_gpio_setpull(GPIO_DISPLAY_DE, S3C_GPIO_PULL_NONE);

	/* DISPLAY_PCLK */
	s3c_gpio_cfgpin(GPIO_DISPLAY_PCLK, GPIO_DISPLAY_PCLK_AF);
	s3c_gpio_setpull(GPIO_DISPLAY_PCLK, S3C_GPIO_PULL_NONE);

	/*
		WARNING:
			This code works on situation that LCD data pin is
			set serially by hardware
	 */
	for (i = 0; i < 24; i++)	{
		s3c_gpio_cfgpin(GPIO_LCD_D0 + i, GPIO_LCD_D0_AF);
		s3c_gpio_setpull(GPIO_LCD_D0 + i, S3C_GPIO_PULL_NONE);
	}
	
	/* mDNIe SEL: why we shall write 0x2 ? */
#ifdef CONFIG_FB_S3C_MDNIE
	writel(0x1, S5P_MDNIE_SEL);
#else
	writel(0x2, S5P_MDNIE_SEL);
#endif
#if 0
	/* drive strength to max */
	writel(0xffffffff, S5PC_VA_GPIO + 0x12c);
	writel(0xffffffff, S5PC_VA_GPIO + 0x14c);
	writel(0xffffffff, S5PC_VA_GPIO + 0x16c);
	writel(0x000000ff, S5PC_VA_GPIO + 0x18c);
#endif

	/* DISPLAY_CS */
	gpio_set_value(GPIO_DISPLAY_CS, 1);
	s3c_gpio_cfgpin(GPIO_DISPLAY_CS, GPIO_DISPLAY_CS_AF);
	s3c_gpio_setpull(GPIO_DISPLAY_CS, S3C_GPIO_PULL_NONE);
	/* DISPLAY_CLK */
	gpio_set_value(GPIO_DISPLAY_CLK, 1);
	s3c_gpio_cfgpin(GPIO_DISPLAY_CLK, GPIO_DISPLAY_CLK_AF);
	s3c_gpio_setpull(GPIO_DISPLAY_CLK, S3C_GPIO_PULL_NONE);
	/* DISPLAY_SI */
	gpio_set_value(GPIO_DISPLAY_SI, 1);
	s3c_gpio_cfgpin(GPIO_DISPLAY_SI, GPIO_DISPLAY_SI_AF);
	s3c_gpio_setpull(GPIO_DISPLAY_SI, S3C_GPIO_PULL_NONE);

     	s3c_gpio_cfgpin(GPIO_MLCD_RST, S3C_GPIO_OUTPUT);
      	gpio_set_value(GPIO_MLCD_RST, GPIO_LEVEL_HIGH);
	s3c_gpio_setpull(GPIO_MLCD_RST, S3C_GPIO_PULL_NONE);

	/*KGVS : configuring GPJ2(4) as FM interrupt */
	//s3c_gpio_cfgpin(S5PV210_GPJ2(4), S5PV210_GPJ2_4_GPIO_INT20_4);

}

void lcd_cfg_gpio_early_suspend(void)
{
	int i;
	printk("[%s]\n", __func__);

	/* DISPLAY_HSYNC */
	s3c_gpio_cfgpin(GPIO_DISPLAY_HSYNC, S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(GPIO_DISPLAY_HSYNC, S3C_GPIO_PULL_NONE);
	s3c_gpio_setpin(GPIO_DISPLAY_HSYNC, 0);

	/* DISPLAY_VSYNC */
	s3c_gpio_cfgpin(GPIO_DISPLAY_VSYNC, S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(GPIO_DISPLAY_VSYNC, S3C_GPIO_PULL_NONE);
	s3c_gpio_setpin(GPIO_DISPLAY_VSYNC, 0);

	/* DISPLAY_DE */
	s3c_gpio_cfgpin(GPIO_DISPLAY_DE, S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(GPIO_DISPLAY_DE, S3C_GPIO_PULL_NONE);
	s3c_gpio_setpin(GPIO_DISPLAY_DE, 0);

	/* DISPLAY_PCLK */
	s3c_gpio_cfgpin(GPIO_DISPLAY_PCLK, S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(GPIO_DISPLAY_PCLK, S3C_GPIO_PULL_NONE);
	s3c_gpio_setpin(GPIO_DISPLAY_PCLK, 0);

	/*
		WARNING:
			This code works on situation that LCD data pin is
			set serially by hardware
	 */
	for (i = 0; i < 24; i++)	{
		s3c_gpio_cfgpin(GPIO_LCD_D0 + i, S3C_GPIO_OUTPUT);
		s3c_gpio_setpull(GPIO_LCD_D0 + i, S3C_GPIO_PULL_NONE);
		s3c_gpio_setpin(GPIO_LCD_D0 + i, 0);
	}
	// drive strength to min
	writel(0x00000000, S5P_VA_GPIO + 0x12c);		// GPF0DRV
	writel(0x00000000, S5P_VA_GPIO + 0x14c);		// GPF1DRV
	writel(0x00000000, S5P_VA_GPIO + 0x16c);		// GPF2DRV
	writel(0x00000000, S5P_VA_GPIO + 0x18c);		// GPF3DRV

	/* DISPLAY_CS */
	s3c_gpio_cfgpin(GPIO_DISPLAY_CS, S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(GPIO_DISPLAY_CS, S3C_GPIO_PULL_NONE);
	gpio_set_value(GPIO_DISPLAY_CS, 0);
	/* DISPLAY_CLK */
	s3c_gpio_cfgpin(GPIO_DISPLAY_CLK, S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(GPIO_DISPLAY_CLK, S3C_GPIO_PULL_NONE);
	gpio_set_value(GPIO_DISPLAY_CLK, 0);
	/* DISPLAY_SI */
	s3c_gpio_cfgpin(GPIO_DISPLAY_SI, S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(GPIO_DISPLAY_SI, S3C_GPIO_PULL_NONE);
	gpio_set_value(GPIO_DISPLAY_SI, 0);

	s3c_gpio_cfgpin(GPIO_OLED_DET, S3C_GPIO_INPUT);
	s3c_gpio_setpull(GPIO_OLED_DET, S3C_GPIO_PULL_DOWN);
}
EXPORT_SYMBOL(lcd_cfg_gpio_early_suspend);

void lcd_cfg_gpio_late_resume(void)
{
	printk("[%s]\n", __func__);
}
EXPORT_SYMBOL(lcd_cfg_gpio_late_resume);

static int tl2796_reset_lcd(struct platform_device *pdev)
{
	int err;

	//  Ver1 & Ver2 universal board kyoungheon
	err = gpio_request(GPIO_MLCD_RST, "MLCD_RST");
	if (err) {
		printk(KERN_ERR "failed to request GPIO_MLCD_RST for "
				"lcd reset control\n");
		return err;
	}

	gpio_direction_output(GPIO_MLCD_RST, 1);
	msleep(10);

	gpio_set_value(GPIO_MLCD_RST, 0);
	msleep(10);

	gpio_set_value(GPIO_MLCD_RST, 1);
	msleep(10);

	gpio_free(GPIO_MLCD_RST);

	return 0;
}

static int tl2796_backlight_on(struct platform_device *pdev)
{

}

static struct s3c_platform_fb tl2796_data __initdata = {
        .hw_ver = 0x62,
        .clk_name = "sclk_fimd",
        .nr_wins = 5,
        .default_win = CONFIG_FB_S3C_DEFAULT_WINDOW,
        .swap = FB_SWAP_HWORD | FB_SWAP_WORD,

        .cfg_gpio = tl2796_cfg_gpio,
        .backlight_on = tl2796_backlight_on,
        .reset_lcd = tl2796_reset_lcd,
};

#define LCD_BUS_NUM     3
#define DISPLAY_CS      S5PV210_MP01(1)
#define SUB_DISPLAY_CS  S5PV210_MP01(2)
#define DISPLAY_CLK     S5PV210_GPC0(1)
#define DISPLAY_SI      S5PV210_GPC0(3)

//#define DISPLAY_CS      GPIO_DISPLAY_CS//S5PV210_MP01(1)
//#define SUB_DISPLAY_CS  GPIO_SUB_DISPLAY_CS //S5PV210_MP01(2)
//#define DISPLAY_CLK     GPIO_DISPLAY_CLK //S5PV210_MP04(1)
//#define DISPLAY_SI      GPIO_DISPLAY_DAT //S5PV210_MP04(3)


static struct spi_board_info spi_board_info[] __initdata = {
        {
                .modalias       = "tl2796",
                .platform_data  = NULL,
                .max_speed_hz   = 1200000,
                .bus_num        = LCD_BUS_NUM,
                .chip_select    = 0,
                .mode           = SPI_MODE_3,
                .controller_data = (void *)DISPLAY_CS,
        },
};

static struct spi_gpio_platform_data tl2796_spi_gpio_data = {
        .sck    = DISPLAY_CLK,
        .mosi   = DISPLAY_SI,
        .miso   = 0,
        .num_chipselect = 2,
};

static struct platform_device s3c_device_spi_gpio = {
        .name   = "spi_gpio",
        .id     = LCD_BUS_NUM,
        .dev    = {
                .parent         = &s3c_device_fb.dev,
                .platform_data  = &tl2796_spi_gpio_data,
        },
};




static  struct  i2c_gpio_platform_data  i2c4_platdata = {
        .sda_pin                = GPIO_AP_SDA_18V,
        .scl_pin                = GPIO_AP_SCL_18V,
        .udelay                 = 2,    /* 250KHz */
        .sda_is_open_drain      = 0,
        .scl_is_open_drain      = 0,
        .scl_is_output_only     = 0,
//      .scl_is_output_only     = 1,
      };

static struct platform_device s3c_device_i2c4 = {
        .name                           = "i2c-gpio",
        .id                                     = 4,
        .dev.platform_data      = &i2c4_platdata,
};

static  struct  i2c_gpio_platform_data  i2c5_platdata = {
        .sda_pin                = GPIO_AP_SDA_28V,
        .scl_pin                = GPIO_AP_SCL_28V,
        .udelay                 = 2,    /* 250KHz */
//      .udelay                 = 4,
        .sda_is_open_drain      = 0,
        .scl_is_open_drain      = 0,
        .scl_is_output_only     = 0,
//      .scl_is_output_only     = 1,
};

static struct platform_device s3c_device_i2c5 = {
        .name                           = "i2c-gpio",
        .id                                     = 5,
        .dev.platform_data      = &i2c5_platdata,
};


static  struct  i2c_gpio_platform_data  i2c6_platdata = {
        .sda_pin                = GPIO_AP_PMIC_SDA,
        .scl_pin                = GPIO_AP_PMIC_SCL,
        .udelay                 = 2,    /* 250KHz */
        .sda_is_open_drain      = 0,
        .scl_is_open_drain      = 0,
        .scl_is_output_only     = 0,
};

static struct platform_device s3c_device_i2c6 = {
        .name                           = "i2c-gpio",
        .id                                     = 6,
        .dev.platform_data      = &i2c6_platdata,
};

static  struct  i2c_gpio_platform_data  i2c7_platdata = {
        .sda_pin                = GPIO_USB_SDA_28V,
        .scl_pin                = GPIO_USB_SCL_28V,
        .udelay                 = 2,    /* 250KHz */
        .sda_is_open_drain      = 0,
        .scl_is_open_drain      = 0,
        .scl_is_output_only     = 0,
};

static struct platform_device s3c_device_i2c7 = {
        .name                           = "i2c-gpio",
        .id                                     = 7,
        .dev.platform_data      = &i2c7_platdata,
};

static  struct  i2c_gpio_platform_data  i2c9_platdata = {
        .sda_pin                = GPIO_FUEL_SDA_28,
        .scl_pin                = GPIO_FUEL_SCL_28,
        .udelay                 = 2,    /* 250KHz */
        .sda_is_open_drain      = 0,
        .scl_is_open_drain      = 0,
        .scl_is_output_only     = 0,
};

static struct platform_device s3c_device_i2c9 = {
        .name                           = "i2c-gpio",
        .id                                     = 9,
        .dev.platform_data      = &i2c9_platdata,
};
#ifdef CONFIG_KEYPAD_MELFAS_TOUCH
static  struct  i2c_gpio_platform_data  i2c10_platdata = {
        .sda_pin                = _3_TOUCH_SDA_28V,
        .scl_pin                = _3_TOUCH_SCL_28V,
        .udelay                 = 0,    /* 250KHz */
        .sda_is_open_drain      = 0,
        .scl_is_open_drain      = 0,
        .scl_is_output_only     = 0,
};
static struct platform_device s3c_device_i2c10 = {
        .name                           = "i2c-gpio",
        .id                                     = 10,
        .dev.platform_data      = &i2c10_platdata,
};
#endif
static  struct  i2c_gpio_platform_data  i2c11_platdata = {
        .sda_pin                = GPIO_ALS_SDA_28V,
        .scl_pin                = GPIO_ALS_SCL_28V,
        .udelay                 = 2,    /* 250KHz */
        .sda_is_open_drain      = 0,
        .scl_is_open_drain      = 0,
        .scl_is_output_only     = 0,
};

static struct platform_device s3c_device_i2c11 = {
        .name                           = "i2c-gpio",
        .id                                     = 11,
        .dev.platform_data      = &i2c11_platdata,
};

static  struct  i2c_gpio_platform_data  i2c12_platdata = {
        .sda_pin                = GPIO_MSENSE_SDA_28V,
        .scl_pin                = GPIO_MSENSE_SCL_28V,
        .udelay                 = 0,    /* 250KHz */
        .sda_is_open_drain      = 0,
        .scl_is_open_drain      = 0,
        .scl_is_output_only     = 0,
};

static struct platform_device s3c_device_i2c12 = {
        .name                           = "i2c-gpio",
        .id                                     = 12,
        .dev.platform_data      = &i2c12_platdata,
};

static  struct  i2c_gpio_platform_data  i2c13_platdata = {
        .sda_pin                = GPIO_CAM_SDA_2_8V,
        .scl_pin                = GPIO_CAM_SCL_2_8V,
        .udelay                 = 0,    /* 250KHz */
        .sda_is_open_drain      = 0,
        .scl_is_open_drain      = 0,
        .scl_is_output_only     = 0,
};

static struct platform_device s3c_device_i2c13 = {
        .name                           = "i2c-gpio",
        .id                                     = 13,
        .dev.platform_data      = &i2c13_platdata,
};

static	struct	i2c_gpio_platform_data	i2c14_platdata = {
	.sda_pin		= GPIO_FM33_SDA,
	.scl_pin		= GPIO_FM33_SCL,
	.udelay 		= 2, /* 177. 4Khz */
};

static struct platform_device s3c_device_i2c14 = {
	.name				= "i2c-gpio",
	.id					= 14,
	.dev.platform_data		= &i2c14_platdata,
};

static struct sec_jack_zone jack_zones[] = {
	[0] = {
		.adc_high	= 700,
		.delay_ms	= 30,
		.check_count	= 10,
		.jack_type	= SEC_HEADSET_3POLE,
	},
	[1] = {
		.adc_high	= 2000,
		.delay_ms	= 20,
		.check_count	= 10,
		.jack_type	= SEC_HEADSET_4POLE,
	},
	[2] = {
		.adc_high	= 3000,
		.delay_ms	= 30,
		.check_count	= 10,
		.jack_type	= SEC_HEADSET_4POLE,
	},
	[3] = {
		.adc_high	= 4000,
		.delay_ms	= 100,
		.check_count	= 50,
		.jack_type	= SEC_HEADSET_3POLE,
	},
};

static int get_stealth_det_jack_state(void)
{
	return gpio_get_value(GPIO_DET_35) ^ 1;
}

static int get_stealth_send_key_state(void)
{
	return gpio_get_value(GPIO_EAR_SEND_END) ^ 1;

}

static void set_stealth_micbias_state(int state)
{
	gpio_set_value(GPIO_EAR_MICBIAS_EN, state);
}

static int sec_jack_get_adc_value(void)
{

	return s3c_adc_get_adc_data(3);
}

void sec_jack_gpio_init(void)
{
	s3c_gpio_cfgpin(GPIO_EAR_SEND_END, S3C_GPIO_SFN(GPIO_EAR_SEND_END_AF));
	s3c_gpio_setpull(GPIO_EAR_SEND_END, S3C_GPIO_PULL_NONE);


	s3c_gpio_cfgpin(GPIO_DET_35, S3C_GPIO_SFN(GPIO_DET_35_AF));
	s3c_gpio_setpull(GPIO_DET_35, S3C_GPIO_PULL_NONE);

	if (gpio_is_valid(GPIO_EAR_MICBIAS_EN)) {
		if (gpio_request(GPIO_EAR_MICBIAS_EN, "EAR_MICBIAS_EN"))
			pr_err("[JACK] Failed to GPIO_EAR_MICBIAS_EN!\n");
		gpio_direction_output(GPIO_EAR_MICBIAS_EN, 0);
	}
	s3c_gpio_slp_cfgpin(GPIO_EAR_MICBIAS_EN, S3C_GPIO_SLP_PREV);

}

static struct sec_jack_platform_data sec_jack_data = {
	.get_det_jack_state	= get_stealth_det_jack_state,
	.get_send_key_state	= get_stealth_send_key_state,
	.set_micbias_state	= set_stealth_micbias_state,
	.get_adc_value	= sec_jack_get_adc_value,
	.zones		= &jack_zones,
	.num_zones	= ARRAY_SIZE(jack_zones),
	.det_int	= IRQ_EINT6,
	.send_int	= IRQ_EINT(30),
};

static struct platform_device sec_device_jack = {
	.name           = "sec_jack",
	.id             = -1,
	.dev            = {
		.platform_data  = &sec_jack_data,
	},
};  



#ifdef CONFIG_S5PV210_ADCTS
static struct s3c_adcts_plat_info s3c_adcts_cfgs __initdata = {
	.channel = {
		{ /* 0 */
			.delay = 0xFF,
			.presc = 49,
			.resol = S3C_ADCCON_RESSEL_12BIT,
		},{ /* 1 */
			.delay = 0xFF,
			.presc = 49,
			.resol = S3C_ADCCON_RESSEL_12BIT,
		},{ /* 2 */
			.delay = 0xFF,
			.presc = 49,
			.resol = S3C_ADCCON_RESSEL_12BIT,
		},{ /* 3 */
			.delay = 0xFF,
			.presc = 49,
			.resol = S3C_ADCCON_RESSEL_12BIT,
		},{ /* 4 */
			.delay = 0xFF,
			.presc = 49,
			.resol = S3C_ADCCON_RESSEL_12BIT,
		},{ /* 5 */
			.delay = 0xFF,
			.presc = 49,
			.resol = S3C_ADCCON_RESSEL_12BIT,
		},{ /* 6 */
			.delay = 0xFF,
			.presc = 49,
			.resol = S3C_ADCCON_RESSEL_12BIT,
		},{ /* 7 */
			.delay = 0xFF,
			.presc = 49,
			.resol = S3C_ADCCON_RESSEL_12BIT,
		},
	},
};
#endif

#ifdef CONFIG_TOUCHSCREEN_S3C
static struct s3c_ts_mach_info s3c_ts_platform __initdata = {
	.adcts = {
		.delay = 0xFF,
	.presc                  = 49,
		.resol = S3C_ADCCON_RESSEL_12BIT,
	},
	.sampling_time = 18,
	.sampling_interval_ms = 20,
	.x_coor_min	= 180,
	.x_coor_max = 4000,
	.x_coor_fuzz = 32,
	.y_coor_min = 300,
	.y_coor_max = 3900,
	.y_coor_fuzz = 32,
	.use_tscal = false,
	.tscal = {0, 0, 0, 0, 0, 0, 0},
};
#endif

#if 0
#ifndef CONFIG_S5PV210_ADCTS 
static struct s3c_adc_mach_info s3c_adc_platform __initdata = {
	/* s5pc110 support 12-bit resolution */
	.delay  = 10000,
	.presc  = 49,
	.resolution = 12,
};
#endif
#endif
#ifdef CONFIG_S5P_ADC 
static struct s3c_adc_mach_info s3c_adc_platform __initdata = {
	/* s5pc110 support 12-bit resolution */
	.delay  = 10000,
	.presc  = 65,
	.resolution = 12,
};
#endif

#ifdef CONFIG_VIDEO_FIMC
/*
 * Guide for Camera Configuration for Aries
*/

#ifdef CONFIG_VIDEO_CE147
/*
 * Guide for Camera Configuration for Jupiter board
 * ITU CAM CH A: CE147
*/
static void ce147_ldo_en(bool onoff)
{
	int err;

	//For Emul Rev0.1
	// Because B4, B5 do not use this GPIO, this GPIO is enabled in all HW version
	/* CAM_IO_EN - GPB(7) */
	err = gpio_request(GPIO_GPB7, "GPB7");

	if(err) {
		printk(KERN_ERR "failed to request GPB7 for camera control\n");

		return err;
	}

	if(onoff == TRUE) { //power on 
		// Turn CAM_ISP_1.2V on
		Set_MAX8998_PM_OUTPUT_Voltage(BUCK4, VCC_1p200);

		Set_MAX8998_PM_REG(EN4, 1);

		mdelay(1);

		// Turn CAM_AF_2.8V on
		Set_MAX8998_PM_OUTPUT_Voltage(LDO11, VCC_2p800);

		Set_MAX8998_PM_REG(ELDO11, 1);

		// Turn CAM_SENSOR_1.2V on
		Set_MAX8998_PM_OUTPUT_Voltage(LDO12, VCC_1p200);

		Set_MAX8998_PM_REG(ELDO12, 1);

		// Turn CAM_SENSOR_A2.8V on
		Set_MAX8998_PM_OUTPUT_Voltage(LDO13, VCC_2p800);

		Set_MAX8998_PM_REG(ELDO13, 1);

		// Turn CAM_ISP_1.8V on
		Set_MAX8998_PM_OUTPUT_Voltage(LDO14, VCC_1p800);

		Set_MAX8998_PM_REG(ELDO14, 1);

		// Turn CAM_ISP_2.8V on
		Set_MAX8998_PM_OUTPUT_Voltage(LDO15, VCC_2p800);

		Set_MAX8998_PM_REG(ELDO15, 1);

		// Turn CAM_SENSOR_1.8V on
		Set_MAX8998_PM_OUTPUT_Voltage(LDO16, VCC_1p800);

		Set_MAX8998_PM_REG(ELDO16, 1);

		// Turn CAM_ISP_SYS_2.8V on
		gpio_direction_output(GPIO_GPB7, 0);

		gpio_set_value(GPIO_GPB7, 1);
	}
	
	else { // power off
		// Turn CAM_ISP_SYS_2.8V off
		gpio_direction_output(GPIO_GPB7, 1);
				
		gpio_set_value(GPIO_GPB7, 0);

		// Turn CAM_AF_2.8V off
		Set_MAX8998_PM_REG(ELDO11, 0);
		
		// Turn CAM_SENSOR_1.2V off
		Set_MAX8998_PM_REG(ELDO12, 0);
		
		// Turn CAM_SENSOR_A2.8V off
		Set_MAX8998_PM_REG(ELDO13, 0);
		
		// Turn CAM_ISP_1.8V off
		Set_MAX8998_PM_REG(ELDO14, 0);
		
		// Turn CAM_ISP_2.8V off
		Set_MAX8998_PM_REG(ELDO15, 0);
		
		// Turn CAM_SENSOR_1.8V off
		Set_MAX8998_PM_REG(ELDO16, 0);
		
		mdelay(1);
		
		// Turn CAM_ISP_1.2V off
		Set_MAX8998_PM_REG(EN4, 0);
	}

	gpio_free(GPIO_GPB7);
}


static int ce147_cam_en(bool onoff)
{
	int err;
	/* CAM_MEGA_EN - GPJ0(6) */
	err = gpio_request(S5PV210_GPJ0(6), "GPJ0");
        if (err) {
                printk(KERN_ERR "failed to request GPJ0 for camera control\n");
                return err;
        }

	gpio_direction_output(S5PV210_GPJ0(6), 0);
	msleep(1);
	gpio_direction_output(S5PV210_GPJ0(6), 1);
	msleep(1);

	if(onoff){
	gpio_set_value(S5PV210_GPJ0(6), 1);
	} else {
		gpio_set_value(S5PV210_GPJ0(6), 0); 
	}
	msleep(1);

	gpio_free(S5PV210_GPJ0(6));

	return 0;
}

static int ce147_cam_nrst(bool onoff)
{
	int err;

	/* CAM_MEGA_nRST - GPJ1(5)*/
	err = gpio_request(S5PV210_GPJ1(5), "GPJ1");
        if (err) {
                printk(KERN_ERR "failed to request GPJ1 for camera control\n");
                return err;
        }

	gpio_direction_output(S5PV210_GPJ1(5), 0);
	msleep(1);
	gpio_direction_output(S5PV210_GPJ1(5), 1);
	msleep(1);

	gpio_set_value(S5PV210_GPJ1(5), 0);
	msleep(1);

	if(onoff){
	gpio_set_value(S5PV210_GPJ1(5), 1);
		msleep(1);
	}
	gpio_free(S5PV210_GPJ1(5));

	return 0;
}


#if defined(CONFIG_ARIES_NTT)
int bCamera_start = 0;
EXPORT_SYMBOL(bCamera_start);
#endif



static int ce147_power_on(void)
{	
	int err;

	printk(KERN_DEBUG "ce147_power_on\n");
#if defined(CONFIG_ARIES_NTT)
		bCamera_start=TRUE;
#endif		


	/* CAM_MEGA_EN - GPJ0(6) */
	err = gpio_request(GPIO_CAM_MEGA_EN, "GPJ0");

	if(err) {
		printk(KERN_ERR "failed to request GPJ0 for camera control\n");

		return err;
	}

	/* CAM_MEGA_nRST - GPJ1(5) */
	err = gpio_request(GPIO_CAM_MEGA_nRST, "GPJ1");

	if(err) {
		printk(KERN_ERR "failed to request GPJ1 for camera control\n");

		return err;
	}
		
	/* CAM_VGA_nSTBY - GPB(0)  */
	err = gpio_request(GPIO_CAM_VGA_nSTBY, "GPB0");

	if (err) {
		printk(KERN_ERR "failed to request GPB0 for camera control\n");

		return err;
	}

	/* CAM_VGA_nRST - GPB(2) */
	err = gpio_request(GPIO_CAM_VGA_nRST, "GPB2");

	if (err) {
		printk(KERN_ERR "failed to request GPB2 for camera control\n");

		return err;
	}
	
	ce147_ldo_en(TRUE);

	mdelay(1);

	// CAM_VGA_nSTBY  HIGH		
	gpio_direction_output(GPIO_CAM_VGA_nSTBY, 0);

	gpio_set_value(GPIO_CAM_VGA_nSTBY, 1);

	mdelay(1);

	// Mclk enable
	s3c_gpio_cfgpin(GPIO_CAM_MCLK, S5PV210_GPE1_3_CAM_A_CLKOUT);

	mdelay(1);

	// CAM_VGA_nRST  HIGH		
	gpio_direction_output(GPIO_CAM_VGA_nRST, 0);

	gpio_set_value(GPIO_CAM_VGA_nRST, 1);	

	mdelay(1);

	// CAM_VGA_nSTBY  LOW	
	gpio_direction_output(GPIO_CAM_VGA_nSTBY, 1);

	gpio_set_value(GPIO_CAM_VGA_nSTBY, 0);

	mdelay(1);

	// CAM_MEGA_EN HIGH
	gpio_direction_output(GPIO_CAM_MEGA_EN, 0);

	gpio_set_value(GPIO_CAM_MEGA_EN, 1);

	mdelay(1);

	// CAM_MEGA_nRST HIGH
	gpio_direction_output(GPIO_CAM_MEGA_nRST, 0);

	gpio_set_value(GPIO_CAM_MEGA_nRST, 1);

	gpio_free(GPIO_CAM_MEGA_EN);

	gpio_free(GPIO_CAM_MEGA_nRST);

	gpio_free(GPIO_CAM_VGA_nSTBY);

	gpio_free(GPIO_CAM_VGA_nRST);	

	mdelay(5);

	return 0;
}


static int ce147_power_off(void)
{
	int err;
	
	printk(KERN_DEBUG "ce147_power_off\n");
#if defined(CONFIG_ARIES_NTT)
			bCamera_start=FALSE;
#endif

	/* CAM_MEGA_EN - GPJ0(6) */
	err = gpio_request(GPIO_CAM_MEGA_EN, "GPJ0");

	if(err) {
		printk(KERN_ERR "failed to request GPJ0 for camera control\n");
	
		return err;
	}

	/* CAM_MEGA_nRST - GPJ1(5) */
	err = gpio_request(GPIO_CAM_MEGA_nRST, "GPJ1");
	
	if(err) {
		printk(KERN_ERR "failed to request GPJ1 for camera control\n");
	
		return err;
	}

	/* CAM_VGA_nRST - GPB(2) */
	err = gpio_request(GPIO_CAM_VGA_nRST, "GPB2");

	if (err) {
		printk(KERN_ERR "failed to request GPB2 for camera control\n");

		return err;
	}

	// CAM_VGA_nRST  LOW		
	gpio_direction_output(GPIO_CAM_VGA_nRST, 1);
	
	gpio_set_value(GPIO_CAM_VGA_nRST, 0);

	mdelay(1);

	// CAM_MEGA_nRST - GPJ1(5) LOW
	gpio_direction_output(GPIO_CAM_MEGA_nRST, 1);
	
	gpio_set_value(GPIO_CAM_MEGA_nRST, 0);
	
	mdelay(1);

	// Mclk disable
	s3c_gpio_cfgpin(GPIO_CAM_MCLK, 0);
	
	mdelay(1);

	// CAM_MEGA_EN - GPJ0(6) LOW
	gpio_direction_output(GPIO_CAM_MEGA_EN, 1);
	
	gpio_set_value(GPIO_CAM_MEGA_EN, 0);

	mdelay(1);

	ce147_ldo_en(FALSE);

	mdelay(1);
	
	gpio_free(GPIO_CAM_MEGA_EN);
	
	gpio_free(GPIO_CAM_MEGA_nRST);

	gpio_free(GPIO_CAM_VGA_nRST);

	return 0;
}


static int ce147_power_en(int onoff)
{
	int bd_level;
#if 0
	if(onoff){
		ce147_ldo_en(true);
		s3c_gpio_cfgpin(S5PV210_GPE1(3), S5PV210_GPE1_3_CAM_A_CLKOUT);
		ce147_cam_en(true);
		ce147_cam_nrst(true);
	} else {
		ce147_cam_en(false);
		ce147_cam_nrst(false);
		s3c_gpio_cfgpin(S5PV210_GPE1(3), 0);
		ce147_ldo_en(false);
	}

	return 0;
#endif

	if(onoff == 1) {
		ce147_power_on();
	}

	else {
		ce147_power_off();
		s3c_i2c0_force_stop();
	}

	return 0;
}

static int smdkc110_cam1_power(int onoff)
{
	int err;
	/* Implement on/off operations */

	/* CAM_VGA_nSTBY - GPB(0) */
	err = gpio_request(S5PV210_GPB(0), "GPB");

	if (err) {
		printk(KERN_ERR "failed to request GPB for camera control\n");

		return err;
	}

	gpio_direction_output(S5PV210_GPB(0), 0);
	
	mdelay(1);

	gpio_direction_output(S5PV210_GPB(0), 1);

	mdelay(1);

	gpio_set_value(S5PV210_GPB(0), 1);

	mdelay(1);

	gpio_free(S5PV210_GPB(0));
	
	mdelay(1);

	/* CAM_VGA_nRST - GPB(2) */
	err = gpio_request(S5PV210_GPB(2), "GPB");

	if (err) {
		printk(KERN_ERR "failed to request GPB for camera control\n");

		return err;
	}

	gpio_direction_output(S5PV210_GPB(2), 0);

	mdelay(1);

	gpio_direction_output(S5PV210_GPB(2), 1);

	mdelay(1);

	gpio_set_value(S5PV210_GPB(2), 1);

	mdelay(1);

	gpio_free(S5PV210_GPB(2));

	return 0;
}

/*
 * Guide for Camera Configuration for Jupiter board
 * ITU CAM CH A: CE147
*/

/* External camera module setting */
static struct ce147_platform_data ce147_plat = {
	.default_width = 640,
	.default_height = 480,
	.pixelformat = V4L2_PIX_FMT_UYVY,
	.freq = 24000000,
	.is_mipi = 0,
};

static struct i2c_board_info  ce147_i2c_info = {
	I2C_BOARD_INFO("CE147", 0x78>>1),
	.platform_data = &ce147_plat,
};

static struct s3c_platform_camera ce147 = {
	.id		= CAMERA_PAR_A,
	.type		= CAM_TYPE_ITU,
	.fmt		= ITU_601_YCBCR422_8BIT,
	.order422	= CAM_ORDER422_8BIT_CBYCRY,
	.i2c_busnum	= 0,
	.info		= &ce147_i2c_info,
	.pixelformat	= V4L2_PIX_FMT_UYVY,
	.srclk_name	= "xusbxti",
	.clk_name	= "sclk_cam0",
	.clk_rate	= 24000000,
	.line_length	= 1920,
	.width		= 640,
	.height		= 480,
	.window		= {
		.left	= 0,
		.top	= 0,
		.width	= 640,
		.height	= 480,
	},

	/* Polarity */
	.inv_pclk	= 0,
	.inv_vsync 	= 1,
	.inv_href	= 0,
	.inv_hsync	= 0,

	.initialized 	= 0,
	.cam_power	= ce147_power_en,
};
#endif

#ifdef CONFIG_VIDEO_M5MO_LS

#define IRQ_M5MO_LS_CAM_MEGA_INT (IRQ_EINT_GROUP18_BASE+6) /* J0_6 */

static void m5moLS_ldo_en(bool onoff)
{
        int err;

        printk(KERN_ERR "m5moLS_ldo_en\n");        

	 printk("m5moLS_ldo_en\n"); 
		

        //For Emul Rev0.1
        // Because B4, B5 do not use this GPIO, this GPIO is enabled in all HW version
        /* CAM_IO_EN - GPB(7) */
        if(onoff == TRUE) { //power on 

            /* 1. CAM_VDIG 1.2V(SENSOR DIGITAL POWER)
                         2. CAM_I_CORE 1.2V
                         3. CAM_VANA_2.8V(SENSOR ANALOG POWER)
                         4. CAM_VIF 1.8V(SENSOR I/O POWER)
                         5. CAM_I_HOST 1.8V(I/O POWER SUPPLY FOR YUV I/F)
                     */
            /* STEALTH REV0.1
                        LDO11 = CAM_AF_2.8V
                        LDO12 = CAM_SENSOR_CORE_1.2V
                        LDO13 = CAM_SENSOR_A2.8V
                        LDO14 = CAM_DVDD_1.5V
                        LDO15 = CAM_HOST_1.8V
                        LDO16 = CAM_SENSOR_IO_1.8V
                        LX4(BUCK4)  = CAM_ISP_CORE_1.2V
                        ON SEQUENCE : LDO12->LX4->LDO13->LDO14->LDO15->LDO16->LDO11
            */
            
            Set_MAX8998_PM_OUTPUT_Voltage(LDO12, VCC_1p300);
            Set_MAX8998_PM_REG(ELDO12, 1);

            Set_MAX8998_PM_OUTPUT_Voltage(BUCK4, VCC_1p300);
            Set_MAX8998_PM_REG(EN4, 1);

            if(HWREV>=8)
            {
                if(gpio_request(GPIO_CAM_IO_EN, "GPB") != 0)
                {
                        printk(KERN_ERR "failed to request GPB7 for camera control = %d\n",HWREV);
                        return err;
                }
                gpio_direction_output(GPIO_CAM_IO_EN, 0);
                gpio_set_value(GPIO_CAM_IO_EN, 1);                
            }
            else
            {
                Set_MAX8998_PM_OUTPUT_Voltage(LDO13, VCC_2p800);
                Set_MAX8998_PM_REG(ELDO13, 1);
            }
            
            Set_MAX8998_PM_OUTPUT_Voltage(LDO14, VCC_1p500);
            Set_MAX8998_PM_REG(ELDO14, 1);

            Set_MAX8998_PM_OUTPUT_Voltage(LDO15, VCC_1p800);
            Set_MAX8998_PM_REG(ELDO15, 1);

            Set_MAX8998_PM_OUTPUT_Voltage(LDO16, VCC_1p800);
            Set_MAX8998_PM_REG(ELDO16, 1);

            Set_MAX8998_PM_OUTPUT_Voltage(LDO11, VCC_2p800);
            Set_MAX8998_PM_REG(ELDO11, 1);
        }
        else { // power off
            /*
                    1. CAM_I_HOST 1.8V
                    2. CAM_VIF 1.8V
                    3. CAM_VANA_2.8V
                    4. CAM_I_CORE_1.2V
                    5. CAM_VDIG 1.2V
                    */
            //OFF SEQUENCE : LDO11->LDO15->LDO16->LDO13->LDO14->LX4->LDO12

            Set_MAX8998_PM_REG(ELDO11, 0);

            Set_MAX8998_PM_REG(ELDO15, 0);

            Set_MAX8998_PM_REG(ELDO16, 0);

            if(HWREV>=8)
            {
                if(gpio_request(GPIO_CAM_IO_EN, "GPB") != 0)
                {
                        printk(KERN_ERR "failed to request GPB7 for camera control = %d\n",HWREV);
                        return err;
                }            
                gpio_direction_output(GPIO_CAM_IO_EN, 1);    
                gpio_set_value(GPIO_CAM_IO_EN, 0);
            }
            else
            {
                Set_MAX8998_PM_REG(ELDO13, 0);
            }
            
            Set_MAX8998_PM_REG(ELDO14, 0);                

            Set_MAX8998_PM_REG(EN4, 0);

            Set_MAX8998_PM_REG(ELDO12, 0);
        }

        if(HWREV >= 8)
            gpio_free(GPIO_CAM_IO_EN);
        
};

static int m5moLS_enable_interrupt_pin(void)
{
        int err = -1;

        err =gpio_request(GPIO_CAM_MEGA_EN,"GPJ1");
        if(err)
            goto err_request_int;
        gpio_direction_input(GPIO_CAM_MEGA_EN);

        s3c_gpio_setpull(GPIO_CAM_MEGA_EN, S3C_GPIO_PULL_UP); 
        set_irq_type(IRQ_M5MO_LS_CAM_MEGA_INT, IRQ_TYPE_EDGE_RISING);
        
        return err;

        err_request_int:
            gpio_free(GPIO_CAM_MEGA_EN);
            return err;
}

static void m5moLS_disable_interrupt_pin(void)
{
		s3c_gpio_setpull(GPIO_CAM_MEGA_EN, S3C_GPIO_PULL_DOWN);
		set_irq_type(IRQ_M5MO_LS_CAM_MEGA_INT, IRQ_TYPE_NONE);
        gpio_free(GPIO_CAM_MEGA_EN);
}

static int m5moLS_cam_nrst(bool onoff)
{
        int err;

        /* CAM_MEGA_nRST - GPJ1(5)*/
        err = gpio_request(GPIO_CAM_MEGA_nRST, "GPJ1");
        if (err) {
                printk(KERN_ERR "failed to request GPJ1 for camera control\n");
                return err;
        }

        gpio_direction_output(GPIO_CAM_MEGA_nRST, 0);
        msleep(1);
        gpio_direction_output(GPIO_CAM_MEGA_nRST, 1);
        msleep(1);

        gpio_set_value(GPIO_CAM_MEGA_nRST, 0);
        msleep(1);

        if(onoff){
            gpio_set_value(GPIO_CAM_MEGA_nRST, 1);
            msleep(1);
        }
        gpio_free(GPIO_CAM_MEGA_nRST);

        return 0;
};

/* 1. CAM_VDIG 1.2V(SENSOR DIGITAL POWER)
             2. CAM_I_CORE 1.2V
             3. CAM_VANA_2.8V(SENSOR ANALOG POWER)
             4. CAM_VIF 1.8V(SENSOR I/O POWER)
             5. CAM_I_HOST 1.8V(I/O POWER SUPPLY FOR YUV I/F)
         */
/* STEALTH REV0.1
            LDO11 = CAM_AF_2.8V
            LDO12 = CAM_SENSOR_CORE_1.2V
            LDO13 = CAM_SENSOR_A2.8V
            LDO14 = CAM_DVDD_1.5V
            LDO15 = CAM_HOST_1.8V
            LDO16 = CAM_SENSOR_IO_1.8V
            LX4(BUCK4)  = CAM_ISP_CORE_1.2V
            ON SEQUENCE : LDO12->LX4->LDO13->LDO14->LDO15->LDO16->LDO11
*/
static int m5moLS_power_on(void)
{	
    int err, err2, err3, err4;

    //printk("m5moLS_power_on\n");

    /* CAM_MEGA_nRST - GPJ1(5) */
    err = gpio_request(GPIO_CAM_MEGA_nRST, "GPJ1");
    if(err) {
        printk(KERN_ERR "failed to request GPJ1 for camera control\n");
        return err;
    }

    /* CAM_VT_EN - GPJ(0)  */
    err = gpio_request(GPIO_CAM_VT_EN_28V, "GPJ4");
    if (err) {
        printk(KERN_ERR "failed to request GPB0 for camera control\n");
        return err;
    }

    /* GPIO_CAM_VT_RST_28V - GPJ4(1) */
    err = gpio_request(GPIO_CAM_VT_RST_28V, "GPJ4");
    if(err) {
        printk(KERN_ERR "failed to request GPJ1 for camera control\n");
        return err;
    }
    
    // Power On Sequence

    /* CAM_ISP_CORE_1.2V */
    Set_MAX8998_PM_OUTPUT_Voltage(BUCK4, VCC_1p300);
    Set_MAX8998_PM_REG(EN4, 1);

    udelay(20);
    
    /* CAM_SENSOR_CORE_1.2V */
    Set_MAX8998_PM_OUTPUT_Voltage(LDO12, VCC_1p300);
    Set_MAX8998_PM_REG(ELDO12, 1);

    udelay(150);
    
    /* CAM_1.3M_nSTBY  HIGH	*/
    gpio_direction_output(GPIO_CAM_VT_EN_28V, 1);
    gpio_set_value(GPIO_CAM_VT_EN_28V, 1);

    udelay(20);

    /* CAM_DVDD_1.5V */
    Set_MAX8998_PM_OUTPUT_Voltage(LDO14, VCC_1p500);
    Set_MAX8998_PM_REG(ELDO14, 1);    

    udelay(20);
    
    /* CAM_SENSOR_A2.8V */
    if(HWREV>=8)
    {
        if(gpio_request(GPIO_CAM_IO_EN, "GPB") != 0)
        {
                printk(KERN_ERR "failed to request GPB7 for camera control = %d\n",HWREV);
                return err;
        }
        gpio_direction_output(GPIO_CAM_IO_EN, 0);
        gpio_set_value(GPIO_CAM_IO_EN, 1);                
    }
    else
    {
        Set_MAX8998_PM_OUTPUT_Voltage(LDO13, VCC_2p800);
        Set_MAX8998_PM_REG(ELDO13, 1);
    }

    udelay(20);
    
    /* CAM_HOST_1.8V */
    Set_MAX8998_PM_OUTPUT_Voltage(LDO15, VCC_1p800);
    Set_MAX8998_PM_REG(ELDO15, 1);

   udelay(20);
   
    /* CAM_SENSOR_IO_1.8V */
    Set_MAX8998_PM_OUTPUT_Voltage(LDO16, VCC_1p800);
    Set_MAX8998_PM_REG(ELDO16, 1);

    udelay(100);

    // Mclk enable
    s3c_gpio_cfgpin(GPIO_CAM_MCLK, S5PV210_GPE1_3_CAM_A_CLKOUT);

    mdelay(1);

    /* CAM_AF_2.8V */
    Set_MAX8998_PM_OUTPUT_Voltage(LDO11, VCC_2p800);
    Set_MAX8998_PM_REG(ELDO11, 1);

   udelay(20);
   
    // CAM_VGA_nRST  HIGH		
    gpio_direction_output(GPIO_CAM_VT_RST_28V, 0);
    gpio_set_value(GPIO_CAM_VT_RST_28V, 1);

    mdelay(85);

    // CAM_1.3M_nSTBY  LOW	
    gpio_direction_output(GPIO_CAM_VT_EN_28V, 1);
    gpio_set_value(GPIO_CAM_VT_EN_28V, 0);

   udelay(20);
   
    // CAM_MEGA_nRST HIGH
    gpio_direction_output(GPIO_CAM_MEGA_nRST, 0);
    gpio_set_value(GPIO_CAM_MEGA_nRST, 1);    
    

    gpio_free(GPIO_CAM_MEGA_nRST);

    gpio_free(GPIO_CAM_VT_EN_28V);

    gpio_free(GPIO_CAM_VT_RST_28V);	

    if(HWREV >= 8)
        gpio_free(GPIO_CAM_IO_EN);

    mdelay(5);

    return 0;
};

static int m5moLS_power_off(void)
{
    int err;

    //printk(KERN_ERR "m5moLS_power_off\n");

    /* CAM_MEGA_nRST - GPJ1(5) */
    err = gpio_request(GPIO_CAM_MEGA_nRST, "GPJ1");
    if(err) {
        printk(KERN_ERR "failed to request GPJ1 for camera control\n");
        return err;
    }

    /* GPIO_CAM_VT_RST_28V - GPJ4(1) */
    err = gpio_request(GPIO_CAM_VT_RST_28V, "GPJ4");
    if(err) {
        printk(KERN_ERR "failed to request GPJ1 for camera control\n");
        return err;
    }

    // Power Off Sequence

    m5moLS_disable_interrupt_pin();
    mdelay(5);

    // CAM_MEGA_nRST - GPJ1(5) LOW
    gpio_direction_output(GPIO_CAM_MEGA_nRST, 1);
    gpio_set_value(GPIO_CAM_MEGA_nRST, 0);

    // CAM_VGA_nRST  LOW
    gpio_direction_output(GPIO_CAM_VT_RST_28V, 0);
    gpio_set_value(GPIO_CAM_VT_RST_28V, 0);

    /* CAM_AF_2.8V */
    Set_MAX8998_PM_OUTPUT_Voltage(LDO11, VCC_2p800);
    Set_MAX8998_PM_REG(ELDO11, 0);

    mdelay(1);
    
    // Mclk disable
    s3c_gpio_cfgpin(GPIO_CAM_MCLK, 0);

    /* CAM_SENSOR_IO_1.8V */
    Set_MAX8998_PM_OUTPUT_Voltage(LDO16, VCC_1p800);
    Set_MAX8998_PM_REG(ELDO16, 0);    

     /* CAM_HOST_1.8V */
     Set_MAX8998_PM_OUTPUT_Voltage(LDO15, VCC_1p800);
     Set_MAX8998_PM_REG(ELDO15, 0);

     /* CAM_SENSOR_A2.8V */
    if(HWREV>=8)
    {
        if(gpio_request(GPIO_CAM_IO_EN, "GPB") != 0)
        {
                printk(KERN_ERR "failed to request GPB7 for camera control = %d\n",HWREV);
                return err;
        }            
        gpio_direction_output(GPIO_CAM_IO_EN, 1);    
        gpio_set_value(GPIO_CAM_IO_EN, 0);
    }
    else
    {
        Set_MAX8998_PM_REG(ELDO13, 0);
    }    

    /* CAM_DVDD_1.5V */
    Set_MAX8998_PM_OUTPUT_Voltage(LDO14, VCC_1p500);
    Set_MAX8998_PM_REG(ELDO14, 0);

    /* CAM_SENSOR_CORE_1.2V */
    Set_MAX8998_PM_OUTPUT_Voltage(LDO12, VCC_1p300);
    Set_MAX8998_PM_REG(ELDO12, 0);

    /* CAM_ISP_CORE_1.2V */
    Set_MAX8998_PM_OUTPUT_Voltage(BUCK4, VCC_1p300);
    Set_MAX8998_PM_REG(EN4, 0);    
    

    gpio_free(GPIO_CAM_MEGA_nRST);
    
    gpio_free(GPIO_CAM_VT_RST_28V);

    if(HWREV >= 8)
        gpio_free(GPIO_CAM_IO_EN);

    return 0;
};

static int m5moLS_power_en(int onoff)
{
        printk("m5moLS_power_en\n");
        if(onoff == 1) {
                m5moLS_power_on();
        }else {
                m5moLS_power_off();
        }

        return 0;
};
/* External camera module setting */
static struct ce147_platform_data m5moLS_plat = {
        .default_width = 640,
        .default_height = 480,
        .pixelformat = V4L2_PIX_FMT_UYVY,
        .freq = 24000000,
        .is_mipi = 0,
};

static struct i2c_board_info  m5moLS_i2c_info = {
        I2C_BOARD_INFO("M5MO_LS", 0x3E>>1),
        .platform_data = &s3c_device_i2c13,
        //.irq		= IRQ_M5MO_LS_CAM_MEGA_INT,
};

static struct s3c_platform_camera m5moLS = {
        .id		= CAMERA_PAR_A,
        .type		= CAM_TYPE_ITU,
        .fmt		= ITU_601_YCBCR422_8BIT,
        .order422	= CAM_ORDER422_8BIT_CBYCRY,
        .i2c_busnum	= 13,
        .info		= &m5moLS_i2c_info,
        .pixelformat	= V4L2_PIX_FMT_UYVY,
        .srclk_name	= "xusbxti",
        .clk_name	= "sclk_cam0",
        .clk_rate	= 24000000,
        .line_length	= 1920,
        .width		= 640,
        .height		= 480,
        .window		= {
                .left	= 0,
                .top	= 0,
                .width	= 640,
                .height	= 480,
        },

        /* Polarity */
        .inv_pclk	= 0,
        .inv_vsync 	= 1,
        .inv_href	= 0,
        .inv_hsync	= 0,
        .initialized 	= 0,
        .cam_power	= m5moLS_power_en,
        .cam_power_flag = 0,
};
#endif

/* External camera module setting */
#ifdef CONFIG_VIDEO_S5KA3DFX
static void s5ka3dfx_ldo_en(bool onoff)
{
//////////////[5B] To fix build error
#if 0
	if(onoff){
		pmic_ldo_enable(LDO_CAM_IO);
		pmic_ldo_enable(LDO_CAM_A);
		pmic_ldo_enable(LDO_CAM_CIF);
	} else {
		pmic_ldo_disable(LDO_CAM_IO);
		pmic_ldo_disable(LDO_CAM_A);
		pmic_ldo_disable(LDO_CAM_CIF);
	}
#endif
}

static int s5ka3dfx_cam_stdby(bool onoff)
{
	int err;
	/* CAM_VGA_nSTBY - GPB(0) */
	err = gpio_request(S5PV210_GPB(0), "GPB");
	if (err) {
		printk(KERN_ERR "failed to request GPJ0 for camera control\n");
		return err;
	}

	gpio_direction_output(S5PV210_GPB(0), 0); 
	msleep(1);
	gpio_direction_output(S5PV210_GPB(0), 1); 
	msleep(1);

	if(onoff){
		gpio_set_value(S5PV210_GPB(0), 1); 
	} else {
		gpio_set_value(S5PV210_GPB(0), 0); 
	}
	msleep(1);

	gpio_free(S5PV210_GPB(0));

	return 0;
}

static int s5ka3dfx_cam_nrst(bool onoff)
{
	int err;

	/* CAM_VGA_nRST - GPB(2)*/
	err = gpio_request(S5PV210_GPB(2), "GPB");
	if (err) {
		printk(KERN_ERR "failed to request GPJ1 for camera control\n");
		return err;
	}

	gpio_direction_output(S5PV210_GPB(2), 0);
	msleep(1);
	gpio_direction_output(S5PV210_GPB(2), 1);
	msleep(1);

	gpio_set_value(S5PV210_GPB(2), 0);
	msleep(1);

	if(onoff){
		gpio_set_value(S5PV210_GPB(2), 1);
		msleep(1);
	}
	gpio_free(S5PV210_GPB(2));

	return 0;
}



static int s5ka3dfx_power_on()
{
	int err;

	printk(KERN_DEBUG "s5ka3dfx_power_on\n");

	/* CAM_VGA_nSTBY - GPB(0)  */
	err = gpio_request(GPIO_CAM_VGA_nSTBY, "GPB0");

	if (err) {
		printk(KERN_ERR "failed to request GPB0 for camera control\n");

		return err;
	}

	/* CAM_VGA_nRST - GPB(2) */
	err = gpio_request(GPIO_CAM_VGA_nRST, "GPB2");

	if (err) {
		printk(KERN_ERR "failed to request GPB2 for camera control\n");

		return err;
	}

	/* CAM_IO_EN - GPB(7) */
	err = gpio_request(GPIO_GPB7, "GPB7");

	if(err) {
		printk(KERN_ERR "failed to request GPB7 for camera control\n");

		return err;
	}

	// Turn CAM_ISP_SYS_2.8V on
	gpio_direction_output(GPIO_GPB7, 0);
	gpio_set_value(GPIO_GPB7, 1);

	mdelay(1);

	// Turn CAM_SENSOR_A2.8V on
	Set_MAX8998_PM_OUTPUT_Voltage(LDO13, VCC_2p800);
	Set_MAX8998_PM_REG(ELDO13, 1);

	mdelay(1);

	// Turn CAM_ISP_HOST_2.8V on
	Set_MAX8998_PM_OUTPUT_Voltage(LDO15, VCC_2p800);
	Set_MAX8998_PM_REG(ELDO15, 1);

	mdelay(1);

	// Turn CAM_ISP_RAM_1.8V on
	Set_MAX8998_PM_OUTPUT_Voltage(LDO14, VCC_1p800);
	Set_MAX8998_PM_REG(ELDO14, 1);

	mdelay(1);
	
	gpio_free(GPIO_GPB7);	

	// CAM_VGA_nSTBY  HIGH		
	gpio_direction_output(GPIO_CAM_VGA_nSTBY, 0);
	gpio_set_value(GPIO_CAM_VGA_nSTBY, 1);

	mdelay(1);

	// Mclk enable
	s3c_gpio_cfgpin(GPIO_CAM_MCLK, S5PV210_GPE1_3_CAM_A_CLKOUT);

	mdelay(1);

	// CAM_VGA_nRST  HIGH		
	gpio_direction_output(GPIO_CAM_VGA_nRST, 0);
	gpio_set_value(GPIO_CAM_VGA_nRST, 1);		

	mdelay(4);

	gpio_free(GPIO_CAM_VGA_nSTBY);
	gpio_free(GPIO_CAM_VGA_nRST);	

	return 0;
}



static int s5ka3dfx_power_off()
{
	int err;

	printk(KERN_DEBUG "s5ka3dfx_power_off\n");

	/* CAM_VGA_nSTBY - GPB(0)  */
	err = gpio_request(GPIO_CAM_VGA_nSTBY, "GPB0");

	if (err) {
		printk(KERN_ERR "failed to request GPB for camera control\n");

		return err;
	}

	/* CAM_VGA_nRST - GPB(2) */
	err = gpio_request(GPIO_CAM_VGA_nRST, "GPB2");

	if (err) {
		printk(KERN_ERR "failed to request GPB for camera control\n");

		return err;
	}


	// CAM_VGA_nRST  LOW		
	gpio_direction_output(GPIO_CAM_VGA_nRST, 1);
	gpio_set_value(GPIO_CAM_VGA_nRST, 0);

	mdelay(1);

	// Mclk disable
	s3c_gpio_cfgpin(GPIO_CAM_MCLK, 0);

	mdelay(1);

	// CAM_VGA_nSTBY  LOW		
	gpio_direction_output(GPIO_CAM_VGA_nSTBY, 1);
	gpio_set_value(GPIO_CAM_VGA_nSTBY, 0);

	mdelay(1);

	/* CAM_IO_EN - GPB(7) */
	err = gpio_request(GPIO_GPB7, "GPB7");

	if(err) {
		printk(KERN_ERR "failed to request GPB for camera control\n");

		return err;
	}

	// Turn CAM_ISP_HOST_2.8V off
	Set_MAX8998_PM_REG(ELDO15, 0);

	mdelay(1);

	// Turn CAM_SENSOR_A2.8V off
	Set_MAX8998_PM_REG(ELDO13, 0);

	// Turn CAM_ISP_RAM_1.8V off
	Set_MAX8998_PM_REG(ELDO14, 0);

	// Turn CAM_ISP_SYS_2.8V off
	gpio_direction_output(GPIO_GPB7, 1);
	gpio_set_value(GPIO_GPB7, 0);
	
	gpio_free(GPIO_GPB7);
	gpio_free(GPIO_CAM_VGA_nSTBY);
	gpio_free(GPIO_CAM_VGA_nRST);	

	return 0;
}



static int s5ka3dfx_power_en(int onoff)
{
#if 0
	if(onoff){
		s5ka3dfx_ldo_en(true);
		s3c_gpio_cfgpin(S5PV210_GPE1(3), S5PV210_GPE1_3_CAM_A_CLKOUT);
		s5ka3dfx_cam_stdby(true);
		s5ka3dfx_cam_nrst(true);
		mdelay(100);
	} else {
		s5ka3dfx_cam_stdby(false);
		s5ka3dfx_cam_nrst(false);
		s3c_gpio_cfgpin(S5PV210_GPE1(3), 0);
		s5ka3dfx_ldo_en(false);
	}

	return 0;
#endif

	if(onoff){
		s5ka3dfx_power_on();
	} else {
		s5ka3dfx_power_off();
		s3c_i2c0_force_stop();
	}

	return 0;
}



static struct s5ka3dfx_platform_data s5ka3dfx_plat = {
	.default_width = 640,
	.default_height = 480,
	.pixelformat = V4L2_PIX_FMT_UYVY,
	.freq = 24000000,
	.is_mipi = 0,
};

static struct i2c_board_info  s5ka3dfx_i2c_info = {
	I2C_BOARD_INFO("S5KA3DFX", 0xc4>>1),
	.platform_data = &s5ka3dfx_plat,
};

static struct s3c_platform_camera s5ka3dfx = {
	.id		= CAMERA_PAR_A,
	.type		= CAM_TYPE_ITU,
	.fmt		= ITU_601_YCBCR422_8BIT,
	.order422	= CAM_ORDER422_8BIT_CBYCRY,
	.i2c_busnum	= 0,
	.info		= &s5ka3dfx_i2c_info,
	.pixelformat	= V4L2_PIX_FMT_UYVY,
	.srclk_name	= "xusbxti",
	.clk_name	= "sclk_cam0",
	.clk_rate	= 24000000,
	.line_length	= 480,
	.width		= 640,
	.height		= 480,
	.window		= {
		.left	= 0,
		.top	= 0,
		.width	= 640,
		.height	= 480,
	},

	/* Polarity */
	.inv_pclk	= 0,
	.inv_vsync 	= 1,
	.inv_href	= 0,
	.inv_hsync	= 0,

	.initialized 	= 0,
	.cam_power	= s5ka3dfx_power_en,
};
#endif

#ifdef CONFIG_VIDEO_S5K5AAFA
static void s5k5aafa_ldo_en(bool onoff)
{
    int err;

    printk(KERN_ERR "s5k5aafa_ldo_en\n");        

    //For Emul Rev0.1
    // Because B4, B5 do not use this GPIO, this GPIO is enabled in all HW version
    /* CAM_IO_EN - GPB(7) */
#if 0        
    err = gpio_request(GPIO_CAM_IO_EN, "CAM_IO_EN");

    if(err) {
            printk(KERN_ERR "failed to request GPB7 for camera control\n");

            return err;
    }
#endif
    if(onoff == TRUE) { //power on 

        /* 1. CAM_VDIG 1.2V(SENSOR DIGITAL POWER)
                     2. CAM_I_CORE 1.2V
                     3. CAM_VANA_2.8V(SENSOR ANALOG POWER)
                     4. CAM_VIF 1.8V(SENSOR I/O POWER)
                     5. CAM_I_HOST 1.8V(I/O POWER SUPPLY FOR YUV I/F)
                 */
        /* STEALTH REV0.1
                    LDO11 = CAM_AF_2.8V
                    LDO12 = CAM_SENSOR_CORE_1.2V
                    LDO13 = CAM_SENSOR_A2.8V
                    LDO14 = CAM_DVDD_1.5V
                    LDO15 = CAM_HOST_1.8V
                    LDO16 = CAM_SENSOR_IO_1.8V
                    LX4(BUCK4)  = CAM_ISP_CORE_1.2V
                    ON SEQUENCE : LDO13->LDO15->LDO14 ->STBYN->I2c->MCLK->RSTN
                */

        if(HWREV >= 8)
        {
            if(gpio_request(GPIO_CAM_IO_EN, "GPB") != 0)
            {
                    printk(KERN_ERR "failed to request GPB7 for camera control = %d\n",HWREV);
                    return err;
            }
            gpio_direction_output(GPIO_CAM_IO_EN, 0);
            gpio_set_value(GPIO_CAM_IO_EN, 1);           
        }
        else
        {
            Set_MAX8998_PM_OUTPUT_Voltage(LDO13, VCC_2p800);
            Set_MAX8998_PM_REG(ELDO13, 1);
        }
        
        Set_MAX8998_PM_OUTPUT_Voltage(LDO15, VCC_1p800);
        Set_MAX8998_PM_REG(ELDO15, 1);

        Set_MAX8998_PM_OUTPUT_Voltage(LDO14, VCC_1p500);
        Set_MAX8998_PM_REG(ELDO14, 1);
    }
    else { // power off
        /*
                1. CAM_I_HOST 1.8V
                2. CAM_VIF 1.8V
                3. CAM_VANA_2.8V
                4. CAM_I_CORE_1.2V
                5. CAM_VDIG 1.2V
                */
        //OFF SEQUENCE : STBYN->RSTN->MCLK->LDO14->LDO13->LDO15

        Set_MAX8998_PM_REG(ELDO14, 0);

        if(HWREV >= 8)
        {

            if(gpio_request(GPIO_CAM_IO_EN, "GPB") != 0)
            {
                    printk(KERN_ERR "failed to request GPB7 for camera control = %d\n",HWREV);
                    return err;
            }        
            gpio_direction_output(GPIO_CAM_IO_EN, 1);    
            gpio_set_value(GPIO_CAM_IO_EN, 0);        
        }
        else
        {
            Set_MAX8998_PM_REG(ELDO13, 0);
        }
        
        Set_MAX8998_PM_REG(ELDO15, 0);
    }

    if(HWREV >= 8)
        gpio_free(GPIO_CAM_IO_EN);

}


static int s5k5aafa_cam_stdby(bool onoff)
{
#if 0
	int err;
	/* CAM_VGA_nSTBY - GPB(0) */
	err = gpio_request(GPIO_CAM_VGA_nSTBY, "GPB");
	if (err) {
		printk(KERN_ERR "failed to request GPJ0 for camera control\n");
		return err;
	}

	gpio_direction_output(GPIO_CAM_VGA_nSTBY, 0); 
	msleep(1);
	gpio_direction_output(GPIO_CAM_VGA_nSTBY, 1); 
	msleep(1);

	if(onoff){
		gpio_set_value(GPIO_CAM_VGA_nSTBY, 1); 
	} else {
		gpio_set_value(GPIO_CAM_VGA_nSTBY, 0); 
	}
	msleep(1);

	gpio_free(GPIO_CAM_VGA_nSTBY);
#endif
	return 0;
}

static int s5k5aafa_cam_nrst(bool onoff)
{
#if 0
	int err;

	/* CAM_VGA_nRST - GPB(2)*/
	err = gpio_request(GPIO_CAM_VGA_nRST, "GPB");
	if (err) {
		printk(KERN_ERR "failed to request GPJ1 for camera control\n");
		return err;
	}

	gpio_direction_output(GPIO_CAM_VGA_nRST, 0);
	msleep(1);
	gpio_direction_output(GPIO_CAM_VGA_nRST, 1);
	msleep(1);

	gpio_set_value(GPIO_CAM_VGA_nRST, 0);
	msleep(1);

	if(onoff){
		gpio_set_value(GPIO_CAM_VGA_nRST, 1);
		msleep(1);
	}
	gpio_free(GPIO_CAM_VGA_nRST);
#endif
	return 0;
}

/* 1. CAM_VDIG 1.2V(SENSOR DIGITAL POWER)
             2. CAM_I_CORE 1.2V
             3. CAM_VANA_2.8V(SENSOR ANALOG POWER)
             4. CAM_VIF 1.8V(SENSOR I/O POWER)
             5. CAM_I_HOST 1.8V(I/O POWER SUPPLY FOR YUV I/F)
         */
/* STEALTH REV0.1
            LDO11 = CAM_AF_2.8V
            LDO12 = CAM_SENSOR_CORE_1.2V
            LDO13 = CAM_SENSOR_A2.8V
            LDO14 = CAM_DVDD_1.5V
            LDO15 = CAM_HOST_1.8V
            LDO16 = CAM_SENSOR_IO_1.8V
            LX4(BUCK4)  = CAM_ISP_CORE_1.2V
            ON SEQUENCE : LDO12->LX4->LDO13->LDO14->LDO15->LDO16->LDO11
*/
static int s5k5aafa_power_on()
{   
    int err, err2, err3, err4;
        
    /* CAM_MEGA_nRST - GPJ1(5) */
    err = gpio_request(GPIO_CAM_MEGA_nRST, "GPJ1");
    if(err) {
        printk(KERN_ERR "failed to request GPJ1 for camera control\n");
        return err;
    }

    /* CAM_VT_EN - GPJ(0)  */
    err = gpio_request(GPIO_CAM_VT_EN_28V, "GPJ4");
    if (err) {
        printk(KERN_ERR "failed to request GPB0 for camera control\n");
        return err;
    }

	  /* GPIO_CAM_VT_RST_28V - GPJ(0)  */
    err = gpio_request(GPIO_CAM_VT_RST_28V, "GPJ4");
    if (err) {
        printk(KERN_ERR "failed to request GPB0 for camera control\n");
        return err;
    }

    // CAM_MEGA_nRST LOW
    gpio_direction_output(GPIO_CAM_MEGA_nRST, 0);
    gpio_set_value(GPIO_CAM_MEGA_nRST, 0);

    // Power on Seqeunce
    
    /* CAM_ISP_CORE_1.2V */
    Set_MAX8998_PM_OUTPUT_Voltage(BUCK4, VCC_1p300);
    Set_MAX8998_PM_REG(EN4, 1);

    mdelay(1);
    
    /* CAM_1.3M_nSTBY  HIGH	*/
    gpio_direction_output(GPIO_CAM_VT_EN_28V, 1);
    gpio_set_value(GPIO_CAM_VT_EN_28V, 1);

    /* CAM_DVDD_1.5V */
    Set_MAX8998_PM_OUTPUT_Voltage(LDO14, VCC_1p500);
    Set_MAX8998_PM_REG(ELDO14, 1);    

    /* CAM_SENSOR_A2.8V */
    if(HWREV>=8)
    {
        if(gpio_request(GPIO_CAM_IO_EN, "GPB") != 0)
        {
                printk(KERN_ERR "failed to request GPB7 for camera control = %d\n",HWREV);
                return err;
        }
        gpio_direction_output(GPIO_CAM_IO_EN, 0);
        gpio_set_value(GPIO_CAM_IO_EN, 1);                
    }
    else
    {
        Set_MAX8998_PM_OUTPUT_Voltage(LDO13, VCC_2p800);
        Set_MAX8998_PM_REG(ELDO13, 1);
    }

    /* CAM_HOST_1.8V */
    Set_MAX8998_PM_OUTPUT_Voltage(LDO15, VCC_1p800);
    Set_MAX8998_PM_REG(ELDO15, 1);

    udelay(200);
    
    // Mclk enable
    s3c_gpio_cfgpin(GPIO_CAM_MCLK, S5PV210_GPE1_3_CAM_A_CLKOUT);

    mdelay(10);
    
    // CAM_VGA_nRST  HIGH		
    gpio_direction_output(GPIO_CAM_VT_RST_28V, 0);
    gpio_set_value(GPIO_CAM_VT_RST_28V, 1);

    mdelay(85);

    gpio_free(GPIO_CAM_MEGA_nRST);

    gpio_free(GPIO_CAM_VT_EN_28V);

    gpio_free(GPIO_CAM_VT_RST_28V); 

    if(HWREV >= 8)
        gpio_free(GPIO_CAM_IO_EN);

    return 0;
}




static int s5k5aafa_power_off()
{
	int err;

	printk(KERN_DEBUG "s5k5aafa_power_off\n");

    //OFF SEQUENCE : STBYN->RSTN->MCLK->LDO14->LDO13->LDO15    
    /* CAM_VT_EN - GPJ(0)  */
    err = gpio_request(GPIO_CAM_VT_EN_28V, "GPJ4");
    if (err) {
        printk(KERN_ERR "failed to request GPB0 for camera control\n");
        return err;
    }

    /* GPIO_CAM_VT_RST_28V - GPJ(0)  */
    err = gpio_request(GPIO_CAM_VT_RST_28V, "GPJ4");
    if (err) {
        printk(KERN_ERR "failed to request GPB0 for camera control\n");
        return err;
    }

     /* CAM_1.3M_nSTBY  LOW	*/     
    gpio_direction_output(GPIO_CAM_VT_EN_28V, 0);
    gpio_set_value(GPIO_CAM_VT_EN_28V, 0);

    mdelay(1);

    // CAM_VT_RST  HIGH       
    gpio_direction_output(GPIO_CAM_VT_RST_28V, 0);

    gpio_set_value(GPIO_CAM_VT_RST_28V, 0); 

    mdelay(1);   

    // Mclk enable
    s3c_gpio_cfgpin(GPIO_CAM_MCLK, 0);

    mdelay(1);
    
     /* CAM_HOST_1.8V */
     Set_MAX8998_PM_OUTPUT_Voltage(LDO15, VCC_1p800);
     Set_MAX8998_PM_REG(ELDO15, 0);

     /* CAM_SENSOR_A2.8V */
    if(HWREV>=8)
    {
        if(gpio_request(GPIO_CAM_IO_EN, "GPB") != 0)
        {
                printk(KERN_ERR "failed to request GPB7 for camera control = %d\n",HWREV);
                return err;
        }            
        gpio_direction_output(GPIO_CAM_IO_EN, 1);    
        gpio_set_value(GPIO_CAM_IO_EN, 0);
    }
    else
    {
        Set_MAX8998_PM_REG(ELDO13, 0);
    }    

    /* CAM_DVDD_1.5V */
    Set_MAX8998_PM_OUTPUT_Voltage(LDO14, VCC_1p500);
    Set_MAX8998_PM_REG(ELDO14, 0);

    /* CAM_ISP_CORE_1.2V */
    Set_MAX8998_PM_OUTPUT_Voltage(BUCK4, VCC_1p300);
    Set_MAX8998_PM_REG(EN4, 0);    


    gpio_free(GPIO_CAM_VT_EN_28V);
    
    gpio_free(GPIO_CAM_VT_RST_28V); 
    
    if(HWREV >= 8)
        gpio_free(GPIO_CAM_IO_EN);

    return 0;
}

static int s5k5aafa_power_en(int onoff)
{
    printk(KERN_ERR "s5k5aafa_power_en\n");

	if(onoff){
		s5k5aafa_power_on();
	} else {
		s5k5aafa_power_off();
	}

	return 0;
}

static struct s5k5aafa_platform_data s5k5aafa_plat = {
	.default_width = 640,
	.default_height = 480,
	.pixelformat = V4L2_PIX_FMT_UYVY,
	.freq = 24000000,
	.is_mipi = 0,
};

static struct i2c_board_info  s5k5aafa_i2c_info = {
	I2C_BOARD_INFO("S5K5AAFA", 0x5A>>1),
	//.platform_data = &s5k5aafa_plat,
	.platform_data = &s3c_device_i2c13,
};

static struct s3c_platform_camera s5k5aafa = {
	.id		= CAMERA_PAR_A,
	.type		= CAM_TYPE_ITU,
	.fmt		= ITU_601_YCBCR422_8BIT,
	.order422	= CAM_ORDER422_8BIT_CBYCRY,
	.i2c_busnum	= 13,
	.info		= &s5k5aafa_i2c_info,
	.pixelformat	= V4L2_PIX_FMT_UYVY,
	.srclk_name	= "xusbxti",
	.clk_name	= "sclk_cam0",
	.clk_rate	= 24000000,
	.line_length	= 480,
	//.line_length	= 1920,
	.width		= 640,
	.height		= 480,
	.window		= {
		.left	= 0,
		.top	= 0,
		.width	= 640,
		.height	= 480,
	},

	/* Polarity */
	.inv_pclk	= 0,
	.inv_vsync 	= 1,
	.inv_href	= 0,
	.inv_hsync	= 0,

	.initialized 	= 0,
	.cam_power	= s5k5aafa_power_en,
	.cam_power_flag = 0,
};
#endif


/* Interface setting */
static struct s3c_platform_fimc fimc_plat = {
	.srclk_name	= "mout_mpll",
	.clk_name	= "sclk_fimc_lclk",
	.clk_rate	= 166750000,
	.default_cam	= CAMERA_PAR_A,
	.camera		= {
#ifdef CONFIG_VIDEO_CE147
		&ce147,
#endif
#ifdef CONFIG_VIDEO_M5MO_LS
        &m5moLS,
#endif
#ifdef CONFIG_VIDEO_S5KA3DFX
		&s5ka3dfx,
#endif
#ifdef CONFIG_VIDEO_S5K5AAFA
        &s5k5aafa,
#endif
	},
	.hw_ver		= 0x43,
};

#endif

#if defined(CONFIG_HAVE_PWM)
static struct platform_pwm_backlight_data smdk_backlight_data = {
	.pwm_id  = 3,
	.max_brightness = 255,
	.dft_brightness = 255,
	.pwm_period_ns  = 78770,
};

static struct platform_device smdk_backlight_device = {
	.name      = "pwm-backlight",
	.id        = -1,
	.dev        = {
		.parent = &s3c_device_timer[3].dev,
		.platform_data = &smdk_backlight_data,
	},
};
static void __init smdk_backlight_register(void)
{
	int ret = platform_device_register(&smdk_backlight_device);
	if (ret)
		printk(KERN_ERR "smdk: failed to register backlight device: %d\n", ret);
}
#endif

#if defined(CONFIG_BLK_DEV_IDE_S3C)
static struct s3c_ide_platdata smdkv210_ide_pdata __initdata = {
	.setup_gpio     = s3c_ide_setup_gpio,
};
#endif

/* I2C0 */
static struct i2c_board_info i2c_devs0[] __initdata = {
};

static struct i2c_board_info i2c_devs4[] __initdata = {
#ifdef CONFIG_SND_SOC_WM8580
	{
		I2C_BOARD_INFO("wm8580", 0x1b),
	},
#endif
#ifdef CONFIG_SND_SOC_WM8994 
	{
		I2C_BOARD_INFO("wm8994", (0x34>>1)),
	},
#endif
};


/* I2C1 */
static struct i2c_board_info i2c_devs1[] __initdata = {
	{
	I2C_BOARD_INFO("s5p_ddc", (0x74>>1)),
	},
};

/* i2c board & device info. */
static struct qt602240_platform_data qt602240_p1_platform_data = {
        .x_line = 19,
        .y_line = 11,
        .x_size = 1024,
        .y_size = 1024,
        .blen = 0x41,
        .threshold = 0x30,
        .orient = QT602240_VERTICAL_FLIP,
};

/* I2C2 */
static struct i2c_board_info i2c_devs2[] __initdata = {
    {
        I2C_BOARD_INFO("qt602240_ts", 0x4a),
        .platform_data  = &qt602240_p1_platform_data,
    },
};



/* I2C2 */
static struct i2c_board_info i2c_devs10[] __initdata = {
    {
        I2C_BOARD_INFO("melfas_touchkey", 0x20),
       // .platform_data  = &qt602240_p1_platform_data,
    },
};

#ifdef CONFIG_SENSORS_L3G4200D_GYRO
static struct l3g4200d_platform_data l3g4200d_p1p2_platform_data = {
};
#endif

static struct i2c_board_info i2c_devs5[] __initdata = {
	{
		I2C_BOARD_INFO("kr3dm", 0x09),
	},	
#ifdef CONFIG_SENSORS_L3G4200D_GYRO
	{
		I2C_BOARD_INFO("l3g4200d", 0x69), 
		.platform_data = &l3g4200d_p1p2_platform_data,
	}
#endif		
};

static struct i2c_board_info i2c_devs6[] __initdata = {
#ifdef CONFIG_REGULATOR_MAX8998
	{
		/* The address is 0xCC used since SRAD = 0 */
		I2C_BOARD_INFO("max8998", (0xCC >> 1)),
		.platform_data = &max8998_platform_data,
	},
	{
		I2C_BOARD_INFO("rtc_max8998", (0x0D >> 1)),
	},
#endif
};

static struct i2c_board_info i2c_devs7[] __initdata = {
	{
		I2C_BOARD_INFO("fsa9480", (0x4A >> 1)),
	},
};

static struct i2c_board_info i2c_devs9[] __initdata = {
	{
		I2C_BOARD_INFO("fuelgauge", (0x6D >> 1)),
	},
};

static struct i2c_board_info i2c_devs11[] __initdata = {
	{
		I2C_BOARD_INFO("gp2a", (0x88 >> 1)),
	},
};

static struct i2c_board_info i2c_devs12[] __initdata = {
	{
		I2C_BOARD_INFO("yamaha", 0x2e),
	},
};

/*
static struct i2c_board_info i2c_devs14[] __initdata = {
	{
		I2C_BOARD_INFO("fm33", (0xC0 >> 1)),
	},
};
*/

static struct i2c_board_info i2c_devs14[] __initdata = {
	{
		I2C_BOARD_INFO("A1026_driver", (0x3E)),
	},
	{
		I2C_BOARD_INFO("fm33", (0xC0 >> 1)),
	},
};

#ifdef CONFIG_DM9000
static void __init smdkv210_dm9000_set(void)
{
	unsigned int tmp;

	tmp = ((0<<28)|(0<<24)|(5<<16)|(0<<12)|(0<<8)|(0<<4)|(0<<0));
	__raw_writel(tmp, (S5P_SROM_BW+0x18));

	tmp = __raw_readl(S5P_SROM_BW);
	tmp &= ~(0xf << 20);

#ifdef CONFIG_DM9000_16BIT
	tmp |= (0x1 << 20);
#else
	tmp |= (0x2 << 20);
#endif
	__raw_writel(tmp, S5P_SROM_BW);

	tmp = __raw_readl(S5PV210_MP01CON);
	tmp &= ~(0xf << 20);
	tmp |= (2 << 20);

	__raw_writel(tmp, S5PV210_MP01CON);
}
#endif

#ifdef CONFIG_ANDROID_PMEM
static struct android_pmem_platform_data pmem_pdata = {
	.name = "pmem",
	.no_allocator = 1,
	.cached = 1,
	.start = 0, // will be set during proving pmem driver.
	.size = 0 // will be set during proving pmem driver.
};

static struct android_pmem_platform_data pmem_gpu1_pdata = {
   .name = "pmem_gpu1",
   .no_allocator = 1,
   .cached = 1,
   .buffered = 1,
   .start = 0,
   .size = 0,
};

static struct android_pmem_platform_data pmem_adsp_pdata = {
   .name = "pmem_adsp",
   .no_allocator = 1,
   .cached = 1,
   .buffered = 1,
   .start = 0,
   .size = 0,
};

static struct platform_device pmem_device = {
   .name = "android_pmem",
   .id = 0,
   .dev = { .platform_data = &pmem_pdata },
};

static struct platform_device pmem_gpu1_device = {
	.name = "android_pmem",
	.id = 1,
	.dev = { .platform_data = &pmem_gpu1_pdata },
};

static struct platform_device pmem_adsp_device = {
	.name = "android_pmem",
	.id = 2,
	.dev = { .platform_data = &pmem_adsp_pdata },
};

static void __init android_pmem_set_platdata(void)
{
	pmem_pdata.start = (u32)s3c_get_media_memory_bank(S3C_MDEV_PMEM, 0);
	pmem_pdata.size = (u32)s3c_get_media_memsize_bank(S3C_MDEV_PMEM, 0);

	pmem_gpu1_pdata.start = (u32)s3c_get_media_memory_bank(S3C_MDEV_PMEM_GPU1, 0);
	pmem_gpu1_pdata.size = (u32)s3c_get_media_memsize_bank(S3C_MDEV_PMEM_GPU1, 0);

	pmem_adsp_pdata.start = (u32)s3c_get_media_memory_bank(S3C_MDEV_PMEM_ADSP, 0);
	pmem_adsp_pdata.size = (u32)s3c_get_media_memsize_bank(S3C_MDEV_PMEM_ADSP, 0);
}
#endif
struct platform_device sec_device_battery = {
	.name	= "sec-battery",
	.id		= -1,
};

static struct platform_device opt_gp2a = {
	.name = "gp2a-opt",
	.id = -1,
};

/********************/
/* bluetooth - start >> */

static struct platform_device	sec_device_rfkill = {
	.name = "bt_rfkill",
	.id	  = -1,
};

static struct platform_device	sec_device_btsleep = {
	.name = "bt_sleep",
	.id	  = -1,
};

/* << bluetooth -end */
/******************/

#if defined (CONFIG_SAMSUNG_PHONE_SVNET) || defined (CONFIG_SAMSUNG_PHONE_SVNET_MODULE)
/* onedram */
static void onedram_cfg_gpio(void)
{
	//unsigned gpio_onedram_int_ap = S5PC11X_GPH1(3);
	s3c_gpio_cfgpin(GPIO_nINT_ONEDRAM_AP, S3C_GPIO_SFN(GPIO_nINT_ONEDRAM_AP_AF));
	s3c_gpio_setpull(GPIO_nINT_ONEDRAM_AP, S3C_GPIO_PULL_UP);
	set_irq_type(GPIO_nINT_ONEDRAM_AP, IRQ_TYPE_LEVEL_LOW);
}

static struct onedram_platform_data onedram_data = {
		.cfg_gpio = onedram_cfg_gpio,
		};

static struct resource onedram_res[] = {
	[0] = {
		.start = (S5PV210_PA_SDRAM + 0x05000000),
		.end = (S5PV210_PA_SDRAM + 0x05000000 + SZ_16M - 1),
		.flags = IORESOURCE_MEM,
		},
	[1] = {
		.start = IRQ_EINT11,
		.end = IRQ_EINT11,
		.flags = IORESOURCE_IRQ,
		},
	};

static struct platform_device onedram = {
		.name = "onedram",
		.id = -1,
		.num_resources = ARRAY_SIZE(onedram_res),
		.resource = onedram_res,
		.dev = {
			.platform_data = &onedram_data,
			},
		};

/* Modem control */
static void modemctl_cfg_gpio(void);
#if defined(CONFIG_ARIES_EUR)
static struct modemctl_platform_data mdmctl_data = {
#if 1// defined(CONFIG_SAMSUNG_SVNET)
	.name = "lte",
	.gpio_phone_active = S5PV210_GPH3(5),
	.gpio_pda_active = S5PV210_GPJ2(5),
	.gpio_phone_on = S5PV210_GPH1(6),
	.gpio_cp_reset = S5PV210_GPH1(2),
	.cfg_gpio = modemctl_cfg_gpio,
#else
	.name = "xmm",
	.gpio_phone_on = GPIO_PHONE_ON,
	.gpio_phone_active = GPIO_PHONE_ACTIVE,
	.gpio_pda_active = GPIO_PDA_ACTIVE,
	.gpio_cp_reset = GPIO_CP_RST,
	//.gpio_sim_ndetect = GPIO_SIM_nDETECT,		/* Galaxy S does not include SIM detect pin */
	.cfg_gpio = modemctl_cfg_gpio,
#endif
};

static struct resource mdmctl_res[] = {
#if 1 //defined(CONFIG_SAMSUNG_SVNET)
	[0] = {
		.start = IRQ_EINT(29),
		.end = IRQ_EINT(29),
		.flags = IORESOURCE_IRQ,
	},
#else
	[0] = {
		.start = IRQ_EINT15,
		.end = IRQ_EINT15,
		.flags = IORESOURCE_IRQ,
	},
	[1] = {
		.start = IRQ_EINT(27),
		.end = IRQ_EINT(27),
		.flags = IORESOURCE_IRQ,
	},
#endif
};

#elif defined(CONFIG_ARIES_NTT)
static struct modemctl_platform_data mdmctl_data = {
	.name = "msm",
	.gpio_phone_on = GPIO_PHONE_ON,
	.gpio_phone_active = GPIO_PHONE_ACTIVE,
	.gpio_pda_active = GPIO_PDA_ACTIVE,
	.gpio_cp_reset = GPIO_CP_RST,
	.gpio_usim_boot = GPIO_USIM_BOOT,
	.gpio_flm_sel = GPIO_FLM_SEL,
	//.gpio_sim_ndetect = GPIO_SIM_nDETECT,		/* Galaxy S does not include SIM detect pin */
	.cfg_gpio = modemctl_cfg_gpio,
};

static struct resource mdmctl_res[] = {
	[0] = {
		.start = IRQ_EINT(19),
		.end = IRQ_EINT(19),
		.flags = IORESOURCE_IRQ,
		},
	[1] = {
		.start = IRQ_EINT(27),
		.end = IRQ_EINT(27),
		.flags = IORESOURCE_IRQ,
		},
	};

#else
# error Need configuration (EUR/NTT) ???
#endif

static struct platform_device modemctl = {
		.name = "modemctl",
		.id = -1,
		.num_resources = ARRAY_SIZE(mdmctl_res),
		.resource = mdmctl_res,
		.dev = {
			.platform_data = &mdmctl_data,
			},
		};

static void modemctl_cfg_gpio(void)
{
	int err = 0;
	
	unsigned gpio_phone_on = mdmctl_data.gpio_phone_on;
	unsigned gpio_phone_active = mdmctl_data.gpio_phone_active;
	unsigned gpio_cp_rst = mdmctl_data.gpio_cp_reset;
	unsigned gpio_pda_active = mdmctl_data.gpio_pda_active;
	unsigned gpio_sim_ndetect = mdmctl_data.gpio_sim_ndetect;
#if defined(CONFIG_ARIES_NTT)
	unsigned gpio_flm_sel = mdmctl_data.gpio_flm_sel;
	unsigned gpio_usim_boot = mdmctl_data.gpio_usim_boot;
#endif

	err = gpio_request(gpio_cp_rst, "CP_RST");
	if (err) {
		printk("fail to request gpio %s\n","CP_RST");
	} else {
		gpio_direction_output(gpio_cp_rst, GPIO_LEVEL_LOW);
		s3c_gpio_setpull(gpio_cp_rst, S3C_GPIO_PULL_NONE);
	}
	err = gpio_request(gpio_phone_on, "PHONE_ON");
	if (err) {
		printk("fail to request gpio %s\n","PHONE_ON");
	} else {
		gpio_direction_output(gpio_phone_on, GPIO_LEVEL_HIGH);
		s3c_gpio_setpull(gpio_phone_on, S3C_GPIO_PULL_NONE);
	}
	err = gpio_request(gpio_pda_active, "PDA_ACTIVE");
	if (err) {
		printk("fail to request gpio %s\n","PDA_ACTIVE");
	} else {
		gpio_direction_output(gpio_pda_active, GPIO_LEVEL_HIGH);
		s3c_gpio_setpull(gpio_pda_active, S3C_GPIO_PULL_NONE);
	}
#if defined(CONFIG_ARIES_NTT)
	err = gpio_request(gpio_flm_sel, "FLM_SEL");
	if (err) {
		printk("fail to request gpio %s\n","FLM_SEL");
	} else {
		gpio_direction_output(gpio_flm_sel, GPIO_LEVEL_LOW);
		s3c_gpio_setpull(gpio_flm_sel, S3C_GPIO_PULL_NONE);
	}

	err = gpio_request(gpio_usim_boot, "USIM_BOOT");
	if (err) {
		printk("fail to request gpio %s\n","USIM_BOOT");
	} else {
		gpio_direction_output(gpio_usim_boot, GPIO_LEVEL_LOW);
		s3c_gpio_setpull(gpio_usim_boot, S3C_GPIO_PULL_NONE);
	}
#endif
	s3c_gpio_cfgpin(gpio_phone_active, S3C_GPIO_SFN(0xF));
	s3c_gpio_setpull(gpio_phone_active, S3C_GPIO_PULL_DOWN);
	set_irq_type(gpio_phone_active, IRQ_TYPE_EDGE_BOTH);

	s3c_gpio_cfgpin(gpio_sim_ndetect, S3C_GPIO_SFN(0xF));
	s3c_gpio_setpull(gpio_sim_ndetect, S3C_GPIO_PULL_NONE);
	set_irq_type(gpio_sim_ndetect, IRQ_TYPE_EDGE_BOTH);
	}

#endif  // CONFIG_SAMSUNG_PHONE_SVNET_MODULE


void s3c_config_gpio_table(int array_size, unsigned int (*gpio_table)[6])
{
	u32 i, gpio;
	for (i = 0; i < array_size; i++) {
		gpio = gpio_table[i][0];
		if (gpio <= S5PV210_MP07(7)) {
			s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(gpio_table[i][1]));
			s3c_gpio_setpull(gpio, gpio_table[i][3]);

			if (gpio_table[i][2] != S3C_GPIO_SETPIN_NONE)
				gpio_set_value(gpio, gpio_table[i][2]);

			s3c_gpio_set_drvstrength(gpio, gpio_table[i][4]);
		}
	}
}

static void lte_power_off(void)
{
#if 1 
	/* LTE Power Off */
	printk(KERN_EMERG "%s: LTE / MAX8996 Power Off\n", __func__);
	s3c_gpio_cfgpin(GPIO_220_PMIC_PWRON, S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(GPIO_220_PMIC_PWRON, S3C_GPIO_PULL_NONE);
	s3c_gpio_setpin(GPIO_220_PMIC_PWRON, 0);

	s3c_gpio_cfgpin(GPIO_LTE_PS_HOLD_OFF, S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(GPIO_LTE_PS_HOLD_OFF, S3C_GPIO_PULL_NONE);
	s3c_gpio_setpin(GPIO_LTE_PS_HOLD_OFF, 1);

    /* VIA Power Off*/
	s3c_gpio_cfgpin(GPIO_VIA_PS_HOLD_OFF, S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(GPIO_VIA_PS_HOLD_OFF, S3C_GPIO_PULL_NONE);
	s3c_gpio_setpin(GPIO_VIA_PS_HOLD_OFF, 1);
#else
	/* LTE Power Off */
	printk(KERN_EMERG "%s: LTE / MAX8996 Power Off\n", __func__);
	s3c_gpio_cfgpin(GPIO_220_PMIC_PWRON, GPIO_OUTPUT);
	s3c_gpio_setpull(GPIO_220_PMIC_PWRON, S3C_GPIO_PULL_NONE);
	gpio_set_value(GPIO_220_PMIC_PWRON, 0);

	s3c_gpio_cfgpin(GPIO_LTE_PS_HOLD_OFF, GPIO_OUTPUT);
	s3c_gpio_setpull(GPIO_LTE_PS_HOLD_OFF, S3C_GPIO_PULL_NONE);
	gpio_set_value(GPIO_LTE_PS_HOLD_OFF, 1);

#endif


	msleep(2000);
}

#define S5PV210_PS_HOLD_CONTROL_REG (S3C_VA_SYS+0xE81C)

static void smdkc110_power_off(void)
{
	int err;
#if 0
	printk("smdkc110_power_off\n");

	/* temporary power off code */
	/*PS_HOLD high  PS_HOLD_CONTROL, R/W, 0xE010_E81C */
	writel(readl(S5PV210_PS_HOLD_CONTROL_REG) & 0xFFFFFEFF,
	       S5PV210_PS_HOLD_CONTROL_REG);

#else
	int mode = REBOOT_MODE_NONE;
	char reset_mode = 'r';
	int phone_wait_cnt = 0;
	int check_power_button;

	// Change this API call just before power-off to take the dump.
	// kernel_sec_clear_upload_magic_number();    

	err = gpio_request(GPIO_N_POWER, "GPIO_N_POWER"); // will never be freed
	WARN(err, "failed to request GPIO_N_POWER");

	err = gpio_request(GPIO_PHONE_ACTIVE, "GPIO_PHONE_ACTIVE");
	WARN(err, "failed to request GPIO_PHONE_ACTIVE"); // will never be freed

	gpio_direction_input(GPIO_N_POWER);
	gpio_direction_input(GPIO_PHONE_ACTIVE);

	gpio_set_value(GPIO_PHONE_ON, 0);	//prevent phone reset when AP off

	// confirm phone off
	while (1) {
		if (gpio_get_value(GPIO_PHONE_ACTIVE)) {
			if (phone_wait_cnt > 10) {
				printk(KERN_EMERG
				       "%s: Try to Turn Phone Off by CP_RST\n",
				       __func__);
				gpio_set_value(GPIO_CP_RST, 0);
			}
			if (phone_wait_cnt > 12) {
				printk(KERN_EMERG "%s: PHONE OFF Failed\n",
				       __func__);
				break;
			}
			phone_wait_cnt++;
			msleep(1000);
		} else {
			printk(KERN_EMERG "%s: PHONE OFF Success\n", __func__);
			break;
		}
	}

	lte_power_off();

#if 0				// check JIG connection
	// never watchdog reset at JIG. It cause the unwanted reset at final IMEI progress
	// infinite delay is better than reset because jig is not the user case.
	if (get_usb_cable_state() &
	    (JIG_UART_ON | JIG_UART_OFF | JIG_USB_OFF | JIG_USB_ON)) {
		/* Watchdog Reset */
		printk(KERN_EMERG "%s: JIG is connected, rebooting...\n",
		       __func__);
		arch_reset(reset_mode);
		printk(KERN_EMERG "%s: waiting for reset!\n", __func__);
		while (1) ;
	}
#endif

	while (1) {
		// Reboot Charging
		if (Get_MAX8998_PM_REG(VDCINOK_status) == 1) {	// SCH-I510 (P110117-0044): Fix kernel panic during power on/off auto-test.
			mode = REBOOT_MODE_CHARGING;
			if (sec_set_param_value)
				sec_set_param_value(__REBOOT_MODE, &mode);
			/* Watchdog Reset */
			printk(KERN_EMERG
			       "%s: TA or USB connected, rebooting...\n",
			       __func__);
			kernel_sec_clear_upload_magic_number();
			__raw_writel(0x0 , S5P_INFORM6);
			kernel_sec_hw_reset(TRUE);
			printk(KERN_EMERG "%s: waiting for reset!\n", __func__);
			while (1) ;
		}
		kernel_sec_clear_upload_magic_number();
		check_power_button = (HWREV == 8) ? 1 : gpio_get_value(GPIO_N_POWER);
		// wait for power button release
		if (check_power_button) {
			printk(KERN_EMERG "%s: set PS_HOLD low.\n", __func__);

			/*PS_HOLD high  PS_HOLD_CONTROL, R/W, 0xE010_E81C */
			writel(readl(S5PV210_PS_HOLD_CONTROL_REG) & 0xFFFFFEFF,
			       S5PV210_PS_HOLD_CONTROL_REG);

			printk(KERN_EMERG "%s: should not reach here!\n",
			       __func__);
		}
		// if power button is not released, wait for a moment. then check TA again.
		printk(KERN_EMERG "%s: PowerButton is not released.\n",
		       __func__);
		mdelay(1000);
	}
#endif

	while (1) ;
}

void s3c_config_sleep_gpio_table(int array_size, unsigned int (*gpio_table)[3])
{
	u32 i, gpio;

	for (i = 0; i < array_size; i++)
	{
		gpio = gpio_table[i][0];
		s3c_gpio_slp_cfgpin(gpio, gpio_table[i][1]);
		s3c_gpio_slp_setpull_updown(gpio, gpio_table[i][2]);
	}

	if (gpio_get_value(GPIO_PS_ON))
	{
		s3c_gpio_slp_setpull_updown(GPIO_ALS_SDA_28V, S3C_GPIO_PULL_NONE);
		s3c_gpio_slp_setpull_updown(GPIO_ALS_SCL_28V, S3C_GPIO_PULL_NONE);
	}
	else
	{
		s3c_gpio_setpull(GPIO_PS_VOUT, S3C_GPIO_PULL_DOWN);
	}

	printk(KERN_DEBUG "SLPGPIO : BT(%d) WLAN(%d) BT+WIFI(%d)\n",
		gpio_get_value(GPIO_BT_nRST),gpio_get_value(GPIO_WLAN_nRST),gpio_get_value(GPIO_WLAN_BT_EN));
	printk(KERN_DEBUG "SLPGPIO : CODEC_LDO_EN(%d) MICBIAS_EN(%d) \n",
		gpio_get_value(GPIO_CODEC_LDO_EN),gpio_get_value(GPIO_MICBIAS_EN));
#ifdef CONFIG_FM_SI4709
	printk(KERN_DEBUG "SLPGPIO : PS_ON(%d) UART_SEL(%d)\n",
		gpio_get_value(GPIO_PS_ON), gpio_get_value(GPIO_UART_SEL));
#endif
}

// just for ref.. 	
//
void s3c_config_sleep_gpio(void)
{
    u32 i, gpio;

    for (i = 0; i < ARRAY_SIZE(jupiter_sleep_alive_gpio_table); i++)
    {
        gpio = jupiter_sleep_alive_gpio_table[i][0];

        s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(jupiter_sleep_alive_gpio_table[i][1]));
        if (jupiter_sleep_alive_gpio_table[i][2] != S3C_GPIO_SETPIN_NONE)
        {
            gpio_set_value(gpio, jupiter_sleep_alive_gpio_table[i][2]);
        }
        s3c_gpio_setpull(gpio, jupiter_sleep_alive_gpio_table[i][3]);
    }

    s3c_config_sleep_gpio_table(ARRAY_SIZE(jupiter_sleep_gpio_table), jupiter_sleep_gpio_table);
}

EXPORT_SYMBOL(s3c_config_sleep_gpio);

void s3c_config_gpio_alive_table(int array_size, int (*gpio_table)[4])
{
	u32 i, gpio;

	for (i = 0; i < array_size; i++) {
		gpio = gpio_table[i][0];
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(gpio_table[i][1]));
		s3c_gpio_setpull(gpio, gpio_table[i][3]);
		if (gpio_table[i][2] != GPIO_LEVEL_NONE)
			gpio_set_value(gpio, gpio_table[i][2]);
	}
}

static struct platform_device watchdog_device = {
	.name = "watchdog",
	.id = -1,
};

static struct platform_device *smdkc110_devices[] __initdata = {
	&watchdog_device,
#ifdef CONFIG_RTC_DRV_S3C
	&s5p_device_rtc,
#endif
	&s3c_device_keypad,

#ifdef CONFIG_REGULATOR_MAX8998
	&s3c_device_8998consumer,
#endif

#ifdef CONFIG_MTD_ONENAND
	&s3c_device_onenand,
#endif
#ifdef CONFIG_FB_S3C
	&s3c_device_fb,
#endif
#ifdef CONFIG_TOUCHSCREEN_S3C
	&s3c_device_ts,
#endif
#ifdef CONFIG_S5PV210_ADCTS
	&s3c_device_adcts,
#endif
#ifdef CONFIG_DM9000
	&s5p_device_dm9000,
#endif
#ifdef CONFIG_S3C2410_WATCHDOG
	&s3c_device_wdt,
#endif
#if defined(CONFIG_BLK_DEV_IDE_S3C)
	&s3c_device_cfcon,
#endif

#ifdef CONFIG_FB_S3C_TL2796
        &s3c_device_spi_gpio,
#endif

#ifdef CONFIG_HAVE_PWM
	&s3c_device_timer[0],
	&s3c_device_timer[1],
	&s3c_device_timer[2],
	&s3c_device_timer[3],
#endif

#ifdef	CONFIG_S5P_ADC
	&s3c_device_adc,
#endif
#ifdef CONFIG_SND_S3C24XX_SOC
	&s3c64xx_device_iis0,
#endif
#ifdef CONFIG_VIDEO_FIMC
	&s3c_device_fimc0,
	&s3c_device_fimc1,
	&s3c_device_fimc2,
	&s3c_device_csis,
	&s3c_device_ipc,
#endif

#ifdef CONFIG_VIDEO_MFC50
	&s3c_device_mfc,
#endif

#ifdef CONFIG_VIDEO_JPEG_V2
	&s3c_device_jpeg,
#endif

#ifdef CONFIG_VIDEO_ROTATOR
	&s5p_device_rotator,
#endif


	&sec_device_jack,
	&s3c_device_i2c0,
	&s3c_device_i2c1,
	&s3c_device_i2c2,
	&s3c_device_i2c4,
	&s3c_device_i2c5,  /* accel sensor */
	&s3c_device_i2c6,  /* PMIC */
	&s3c_device_i2c7,  /* For FSA micro USB switch */
	&s3c_device_i2c9,  /* fuel gague */
#ifdef CONFIG_KEYPAD_MELFAS_TOUCH
	&s3c_device_i2c10, /* For touchkey */
#endif
	&s3c_device_i2c11, /* optical sensor */
	&s3c_device_i2c12, /* magnetic sensor */
	&s3c_device_i2c13, /*camera*/
	&s3c_device_i2c14, /* FM33 */

#ifdef CONFIG_USB
	&s3c_device_usb_ehci,
	&s3c_device_usb_ohci,
#endif
#ifdef CONFIG_USB_GADGET
	&s3c_device_usbgadget,
#endif
#ifdef CONFIG_USB_ANDROID
	&s3c_device_android_usb,
	&s3c_device_usb_mass_storage,
#endif

    &sec_device_dpram,

#ifdef CONFIG_S3C_DEV_HSMMC
	&s3c_device_hsmmc0,
#endif
#ifdef CONFIG_S3C_DEV_HSMMC1
	&s3c_device_hsmmc1,
#endif
#ifdef CONFIG_S3C_DEV_HSMMC2
	&s3c_device_hsmmc2,
#endif
#ifdef CONFIG_S3C_DEV_HSMMC3
	&s3c_device_hsmmc3,
#endif

#ifdef CONFIG_VIDEO_TV20
	&s5p_device_tvout,
	&s5p_device_cec,
	&s5p_device_hpd,
#endif

#ifdef CONFIG_ANDROID_PMEM
	&pmem_device,
	&pmem_gpu1_device,
	&pmem_adsp_device,
#endif
	&sec_device_battery,
#ifdef CONFIG_VIDEO_G2D
	&s5p_device_g2d,
#endif
	&sec_device_rfkill,
	&sec_device_btsleep,
#if defined(CONFIG_TOUCHSCREEN_QT602240)
	&s3c_device_qtts,
#endif
	&opt_gp2a 
};

static int read_hwversion(void)
{
        int err;
        int hwver = -1;
        int hwver_0 = -1;
        int hwver_1 = -1;
        int hwver_2 = -1;

        err = gpio_request(S5PV210_GPJ0(2), "HWREV_MODE0");

        if (err) {
                printk(KERN_ERR "failed to request GPJ0(2) for "
                        "HWREV_MODE0\n");
                return err;
        }
        err = gpio_request(S5PV210_GPJ0(3), "HWREV_MODE1");

        if (err) {
                printk(KERN_ERR "failed to request GPJ0(3) for "
                        "HWREV_MODE1\n");
                return err;
        }
        err = gpio_request(S5PV210_GPJ0(4), "HWREV_MODE2");

        if (err) {
                printk(KERN_ERR "failed to request GPJ0(4) for "
                        "HWREV_MODE2\n");
                return err;
        }

        gpio_direction_input(S5PV210_GPJ0(2));
        gpio_direction_input(S5PV210_GPJ0(3));
        gpio_direction_input(S5PV210_GPJ0(4));

        hwver_0 = gpio_get_value(S5PV210_GPJ0(2));
        hwver_1 = gpio_get_value(S5PV210_GPJ0(3));
        hwver_2 = gpio_get_value(S5PV210_GPJ0(4));

        gpio_free(S5PV210_GPJ0(2));
        gpio_free(S5PV210_GPJ0(3));
        gpio_free(S5PV210_GPJ0(4));
	
	if((hwver_0 == 0)&&(hwver_1 == 1)&&(hwver_2 == 0)){
                hwver = 2;
                printk("+++++++++[I9000 Rev0.1 board]++++++++ hwver_0: %d, hwver_1: %d, hwver_2: %d\n", hwver_0, hwver_1, hwver_2);
        }
        else if((hwver_0 == 1)&&(hwver_1 == 0)&&(hwver_2 == 1)){
                hwver = 2;
                printk("+++++++++[B5 board]++++++++ hwver_0: %d, hwver_1: %d, hwver_2: %d\n", hwver_0, hwver_1, hwver_2);
        }
        else if((hwver_0 == 0)&&(hwver_1 == 1)&&(hwver_2 == 1)){
                hwver = 2;
                printk("+++++++++[ARIES B5 board]++++++++ hwver_0: %d, hwver_1: %d, hwver_2: %d\n", hwver_0, hwver_1, hwver_2);
        }
        else{
                hwver = 0;
                //printk("+++++++++[B2, B3 board]++++++++ hwver_0: %d, hwver_1: %d, hwver_2: %d\n", hwver_0, hwver_1, hwver_2);
        }

        return hwver;
}


static void __init smdkc110_fixup(struct machine_desc *desc,
                                       struct tag *tags, char **cmdline,
                                       struct meminfo *mi)
{

	mi->bank[0].start = 0x30000000;
	mi->bank[0].size = 80 * SZ_1M;
	mi->bank[0].node = 0;

	mi->bank[1].start = 0x40000000;
        //mi->bank[1].size = 256 * SZ_1M;
        mi->bank[1].size = 256 * SZ_1M; /* this value wil be changed to 256MB */
        mi->bank[1].node = 1;

	mi->nr_banks = 2;

        mi->bank[2].start = 0x50000000;
        mi->bank[2].size = 128 * SZ_1M;
        mi->bank[2].node = 2;
        mi->nr_banks = 3;

}

static void __init smdkc110_map_io(void)
{
	s5p_init_io(NULL, 0, S5P_VA_CHIPID);
	s3c24xx_init_clocks(24000000);
	s5pv210_gpiolib_init();
	s3c24xx_init_uarts(smdkv210_uartcfgs, ARRAY_SIZE(smdkv210_uartcfgs));
	s5pv210_reserve_bootmem();

#ifdef CONFIG_MTD_ONENAND
	s3c_device_onenand.name = "s5pc110-onenand";
#endif
}

#ifdef CONFIG_S3C_SAMSUNG_PMEM
static void __init s3c_pmem_set_platdata(void)
{
	pmem_pdata.start = s3c_get_media_memory_bank(S3C_MDEV_PMEM, 1);
	pmem_pdata.size = s3c_get_media_memsize_bank(S3C_MDEV_PMEM, 1);
}
#endif

#ifdef CONFIG_FB_S3C_LTE480WV
static struct s3c_platform_fb lte480wv_fb_data __initdata = {
	.hw_ver	= 0x62,
	.nr_wins = 5,
	.default_win = CONFIG_FB_S3C_DEFAULT_WINDOW,
	.swap = FB_SWAP_WORD | FB_SWAP_HWORD,
};
#endif
/* this function are used to detect s5pc110 chip version temporally */

int s5pc110_version ;

void _hw_version_check(void)
{
	void __iomem * phy_address ;
	int temp; 

	phy_address = ioremap (0x40,1);

	temp = __raw_readl(phy_address);


	if (temp == 0xE59F010C)
	{
		s5pc110_version = 0;
	}
	else
	{
		s5pc110_version=1 ;
	}
	printk("S5PC110 Hardware version : EVT%d \n",s5pc110_version);
	
	iounmap(phy_address);
}

/* Temporally used
 * return value 0 -> EVT 0
 * value 1 -> evt 1
 */

int hw_version_check(void)
{
	return s5pc110_version ;
}
EXPORT_SYMBOL(hw_version_check);

/* touch screen device init */
static void __init qt_touch_init(void)
{
    int gpio, irq;

        /* qt602240 TSP */
    qt602240_p1_platform_data.blen = 0x1;
    qt602240_p1_platform_data.threshold = 0x13;
    qt602240_p1_platform_data.orient = QT602240_VERTICAL_FLIP;

    gpio = S5PV210_GPG3(6);                     /* XMMC3DATA_3 */
    gpio_request(gpio, "TOUCH_EN");
    s3c_gpio_cfgpin(gpio, S3C_GPIO_OUTPUT);
    gpio_direction_output(gpio, 1);
    gpio_free(gpio);

    gpio = S5PV210_GPJ0(5);                             /* XMSMADDR_5 */
    gpio_request(gpio, "TOUCH_INT");
    s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(0xf));
    s3c_gpio_setpull(gpio, S3C_GPIO_PULL_UP);
    irq = gpio_to_irq(gpio);
    gpio_free(gpio);
    
    i2c_devs2[0].irq = irq;
}

void fm33_gpio_init(void)
{

	if (gpio_is_valid(GPIO_FM33_RST)) {
		if (gpio_request(GPIO_FM33_RST, "FM33_RST"))
			pr_err("[JACK] Failed to GPIO_FM33_RST!\n");
		gpio_direction_output(GPIO_FM33_RST, 0);
	}
	s3c_gpio_slp_cfgpin(GPIO_FM33_RST, S3C_GPIO_SLP_PREV);
	s3c_gpio_setpull(GPIO_FM33_RST, S3C_GPIO_PULL_NONE);

	if (gpio_is_valid(GPIO_FM33_PWDN)) {
		if (gpio_request(GPIO_FM33_PWDN, "FM33_PWDN"))
			pr_err("[JACK] Failed to GPIO_FM33_PWDN!\n");
		gpio_direction_output(GPIO_FM33_PWDN, 1);
	}
	s3c_gpio_slp_cfgpin(GPIO_FM33_PWDN, S3C_GPIO_SLP_PREV);
	s3c_gpio_setpull(GPIO_FM33_PWDN, S3C_GPIO_PULL_NONE);

	if (gpio_is_valid(GPIO_FM33_BP)) {
		if (gpio_request(GPIO_FM33_BP, "FM33_BP"))
			pr_err("[JACK] Failed to GPIO_FM33_BP!\n");
		gpio_direction_output(GPIO_FM33_BP, 1);
	}
	s3c_gpio_slp_cfgpin(GPIO_FM33_BP, S3C_GPIO_SLP_PREV);
        s3c_gpio_setpull(GPIO_FM33_BP, S3C_GPIO_PULL_NONE);
}

void dpram_access_init(void)
{    unsigned int regVal;
/* SINGALS
1) C110_DPRAM_nCS --> XM0CSN_3  ( ie Xm0CSn[3] MP0_1[3])
2) C110_OE_N -->XM0OEN    
3) C110_LB -> XM0BEN_0    
4) C110_UB --> XM0BEN_1    
5) C110_DPRAM_INT_N --> XEINT_8    
6) C110_WE_N --> XM0WEN    
7) DATA LINES --> XM0DATA_0 to XM0DATA_15    
8) Address Lines -->XM0ADDR_0 to XM0ADDR_12 */

    regVal = __raw_readl (S5PV210_VA_SYSCON + 0x0464);    
    regVal |= BIT26; //CLK_SROMC [26]     
    writel(regVal, S5PV210_VA_SYSCON + 0x0464);

    regVal = __raw_readl ( S5P_SROM_BW);	
    regVal |= (BIT15 | BIT12);	
    writel(regVal, S5P_SROM_BW);

    regVal = __raw_readl (S5P_SROM_BC3);	
    regVal = 0x00040000;	
    writel(regVal, S5P_SROM_BC3);    //ADDR LINES //0xE0200340  and 0xE0200360	

    regVal = 0x22222222;	
    writel(regVal, S5PV210_GPA0_BASE + 0x0340);

    regVal = __raw_readl (S5PV210_GPA0_BASE + 0x0360);
    regVal |=  0x00022222;
    writel(regVal, S5PV210_GPA0_BASE + 0x0360);    //DATA LINES MP06 and MP07 //0xE0200380 and 0xE02003A0

    regVal = 0x22222222;
    writel(regVal, S5PV210_GPA0_BASE + 0x0380);

    regVal = 0x22222222;
    writel(regVal, S5PV210_GPA0_BASE + 0x03A0);        // XM0CSN_3: MP0_1CON at  0xE02002E0

    regVal = readl(S5PV210_GPA0_BASE + 0x02E0);
    regVal |= (BIT29 | BIT25 | BIT13);
    regVal &= ~BIT24;
    regVal &= ~BIT12;
    writel(regVal, S5PV210_GPA0_BASE + 0x02E0);
    printk( "dpram_access_init completed \n");
}

extern void set_pmic_gpio(void);
static void jupiter_init_gpio(void)
{
    s3c_config_gpio_table(ARRAY_SIZE(jupiter_gpio_table), jupiter_gpio_table);
    //s3c_config_sleep_gpio_table(ARRAY_SIZE(jupiter_sleep_gpio_table), jupiter_sleep_gpio_table);

	/*Adding pmic gpio(GPH3, GPH4, GPH5) initialisation*/
	set_pmic_gpio();
}

static int arise_notifier_call(struct notifier_block *this, unsigned long code,
			       void *_cmd)
{
	int mode = REBOOT_MODE_NONE;

	if ((code == SYS_RESTART) && _cmd) {
		if (!strcmp((char *)_cmd, "arm11_fota"))
			mode = REBOOT_MODE_ARM11_FOTA;
		else if (!strcmp((char *)_cmd, "arm9_fota"))
			mode = REBOOT_MODE_ARM9_FOTA;
		else if (!strcmp((char *)_cmd, "recovery")) {
			mode = REBOOT_MODE_RECOVERY;
#ifdef CONFIG_KERNEL_DEBUG_SEC 
		    //etinum.factory.reboot disable uart msg in bootloader for
		    // factory reset 2nd ack
		    kernel_sec_set_upload_cause(BLK_UART_MSG_FOR_FACTRST_2ND_ACK);
#endif			
        }
		else if (!strcmp((char *)_cmd, "download"))
			mode = REBOOT_MODE_DOWNLOAD;
#ifdef CONFIG_KERNEL_DEBUG_SEC 
	    //etinum.factory.reboot disable uart msg in bootloader for
	    // factory reset 2nd ack
	    else if (!strcmp((char *)_cmd, "factory_reboot")) { 
		    mode = REBOOT_MODE_NONE;
		    kernel_sec_set_upload_cause(BLK_UART_MSG_FOR_FACTRST_2ND_ACK);
        }
#endif	
	}

	if (code != SYS_POWER_OFF) {
		if (sec_set_param_value) {
			sec_set_param_value(__REBOOT_MODE, &mode);
		}
	}

	return NOTIFY_DONE;
}

static struct notifier_block arise_reboot_notifier = {
	.notifier_call = arise_notifier_call,
};

#define DHD_SKB_HDRSIZE 		336
#define DHD_SKB_1PAGE_BUFSIZE	((PAGE_SIZE*1)-DHD_SKB_HDRSIZE)
#define DHD_SKB_2PAGE_BUFSIZE	((PAGE_SIZE*2)-DHD_SKB_HDRSIZE)
#define DHD_SKB_4PAGE_BUFSIZE	((PAGE_SIZE*4)-DHD_SKB_HDRSIZE)

#define WLAN_SKB_BUF_NUM	17

struct sk_buff *wlan_static_skb[WLAN_SKB_BUF_NUM];
EXPORT_SYMBOL(wlan_static_skb);


int __init aries_init_wifi_mem(void)
{
	int i;
	int j;

	printk("aries_init_wifi_mem\n");
	for (i = 0; i < 8; i++) {
		wlan_static_skb[i] = dev_alloc_skb(DHD_SKB_1PAGE_BUFSIZE);
		if (!wlan_static_skb[i])
			goto err_skb_alloc;
	}
	
	for (; i < 16; i++) {
		wlan_static_skb[i] = dev_alloc_skb(DHD_SKB_2PAGE_BUFSIZE);
		if (!wlan_static_skb[i])
			goto err_skb_alloc;
	}
	
	wlan_static_skb[i] = dev_alloc_skb(DHD_SKB_4PAGE_BUFSIZE);
	if (!wlan_static_skb[i])
		goto err_skb_alloc;

	printk("aries_init_wifi_mem success\n");
	return 0;

 err_skb_alloc:
	pr_err("Failed to skb_alloc for WLAN\n");
	for (j = 0 ; j <WLAN_SKB_BUF_NUM ; j++)
		dev_kfree_skb(wlan_static_skb[j]);

	return -ENOMEM;
}


static void __init smdkc110_machine_init(void)
{
	/* Find out S5PC110 chip version */
	_hw_version_check();

	pm_power_off = smdkc110_power_off ; 


	s3c_gpio_cfgpin(GPIO_HWREV_MODE0, S3C_GPIO_INPUT);
	s3c_gpio_setpull( GPIO_HWREV_MODE0, S3C_GPIO_PULL_NONE); 
	s3c_gpio_cfgpin(GPIO_HWREV_MODE1, S3C_GPIO_INPUT);
	s3c_gpio_setpull( GPIO_HWREV_MODE1, S3C_GPIO_PULL_NONE);  
	s3c_gpio_cfgpin(GPIO_HWREV_MODE2, S3C_GPIO_INPUT);
	s3c_gpio_setpull( GPIO_HWREV_MODE2, S3C_GPIO_PULL_NONE); 
	HWREV = gpio_get_value(GPIO_HWREV_MODE0);
	HWREV = HWREV | (gpio_get_value(GPIO_HWREV_MODE1) <<1);
	HWREV = HWREV | (gpio_get_value(GPIO_HWREV_MODE2) <<2);
	s3c_gpio_cfgpin(GPIO_HWREV_MODE3, S3C_GPIO_INPUT);
	s3c_gpio_setpull( GPIO_HWREV_MODE3, S3C_GPIO_PULL_NONE); 
#if !defined(CONFIG_ARIES_NTT)
	HWREV = HWREV | (gpio_get_value(GPIO_HWREV_MODE3) <<3);
	printk("HWREV is 0x%x\n", HWREV);
#else
	HWREV = 0x0E;
	printk("HWREV is 0x%x\n", HWREV);
#endif
	/*initialise the gpio's*/
	jupiter_init_gpio();

	/* OneNAND */
#ifdef CONFIG_MTD_ONENAND
	//s3c_device_onenand.dev.platform_data = &s5p_onenand_data;
#endif


	qt_touch_init();
	//fm33_gpio_init();

#ifdef CONFIG_DM9000
	smdkv210_dm9000_set();
#endif

#ifdef CONFIG_ANDROID_PMEM
	android_pmem_set_platdata();
#endif
	{
		int tint = GPIO_TOUCH_INT;
		s3c_gpio_cfgpin(tint, S3C_GPIO_INPUT);
		s3c_gpio_setpull(tint, S3C_GPIO_PULL_UP);
	}

	/* new code added to keep the RST/PWR lines low until controlled by DPRAM driver */             
	printk("setting GPIO_PHONE_RST_N as LOW \n");
	gpio_set_value(GPIO_PHONE_RST_N, GPIO_LEVEL_LOW);

	printk("setting GPIO_PHONE_ON as LOW \n");
	gpio_set_value(GPIO_PHONE_ON, GPIO_LEVEL_LOW);

	/* i2c */
	s3c_i2c0_set_platdata(NULL);
	s3c_i2c1_set_platdata(NULL);
	s3c_i2c2_set_platdata(NULL);
	i2c_register_board_info(0, i2c_devs0, ARRAY_SIZE(i2c_devs0));
	i2c_register_board_info(1, i2c_devs1, ARRAY_SIZE(i2c_devs1));
	i2c_register_board_info(2, i2c_devs2, ARRAY_SIZE(i2c_devs2));
	i2c_register_board_info(4, i2c_devs4, ARRAY_SIZE(i2c_devs4));
	i2c_register_board_info(5, i2c_devs5, ARRAY_SIZE(i2c_devs5));
	i2c_register_board_info(6, i2c_devs6, ARRAY_SIZE(i2c_devs6));
	i2c_register_board_info(7, i2c_devs7, ARRAY_SIZE(i2c_devs7)); /* for fsa9480 */
	i2c_register_board_info(9, i2c_devs9, ARRAY_SIZE(i2c_devs9));
	i2c_register_board_info(10, i2c_devs10, ARRAY_SIZE(i2c_devs10)); /* for touchkey */
	i2c_register_board_info(11, i2c_devs11, ARRAY_SIZE(i2c_devs11)); /* optical sensor */
	i2c_register_board_info(12, i2c_devs12, ARRAY_SIZE(i2c_devs12)); /* magnetic sensor */
	i2c_register_board_info(14, i2c_devs14, ARRAY_SIZE(i2c_devs14)); /* for FM33 */

#ifdef CONFIG_FB_S3C_LTE480WV
	s3cfb_set_platdata(&lte480wv_fb_data);
#endif

#if defined(CONFIG_BLK_DEV_IDE_S3C)
	s3c_ide_set_platdata(&smdkv210_ide_pdata);
#endif

#if defined(CONFIG_TOUCHSCREEN_S3C)
	s3c_ts_set_platdata(&s3c_ts_platform);
#endif

#ifdef CONFIG_FB_S3C_TL2796
        spi_register_board_info(spi_board_info, ARRAY_SIZE(spi_board_info));
        s3cfb_set_platdata(&tl2796_data);
#endif

#if defined(CONFIG_S5PV210_ADCTS)
	s3c_adcts_set_platdata(&s3c_adcts_cfgs);
#endif

#if defined(CONFIG_S5P_ADC)
	s3c_adc_set_platdata(&s3c_adc_platform);
#endif

#if defined(CONFIG_PM)
	s3c_pm_init();
//	s5pc11x_pm_init();
#endif
#ifdef CONFIG_VIDEO_FIMC
	/* fimc */
	s3c_fimc0_set_platdata(&fimc_plat);
	s3c_fimc1_set_platdata(&fimc_plat);
	s3c_fimc2_set_platdata(&fimc_plat);
	s3c_csis_set_platdata(NULL);
#endif

#ifdef CONFIG_VIDEO_MFC50
	/* mfc */
	s3c_mfc_set_platdata(NULL);
#endif

#ifdef CONFIG_S3C_DEV_HSMMC
	s5pv210_default_sdhci0();
#endif
#ifdef CONFIG_S3C_DEV_HSMMC1
	s5pv210_default_sdhci1();
#endif
#ifdef CONFIG_S3C_DEV_HSMMC2
	s5pv210_default_sdhci2();
#endif
#ifdef CONFIG_S3C_DEV_HSMMC3
	s5pv210_default_sdhci3();
#endif
#ifdef CONFIG_S5PV210_SETUP_SDHCI
	s3c_sdhci_set_platdata();
#endif
	platform_add_devices(smdkc110_devices, ARRAY_SIZE(smdkc110_devices));
#ifdef CONFIG_SAMSUNG_PHONE_SVNET_MODULE
	platform_device_register(&modemctl);
	platform_device_register(&onedram);
#endif
#if defined(CONFIG_HAVE_PWM)
	smdk_backlight_register();
#endif

	s3c_gpio_cfgpin( GPIO_AP_SCL_28V, 1 );
	s3c_gpio_setpull( GPIO_AP_SCL_28V, S3C_GPIO_PULL_UP); 
	s3c_gpio_cfgpin( GPIO_AP_SDA_28V, 1 );
	s3c_gpio_setpull( GPIO_AP_SDA_28V, S3C_GPIO_PULL_UP); 
	
	register_reboot_notifier(&arise_reboot_notifier);

	jupiter_switch_init();

	BUG_ON(!sec_class);
	gps_dev = device_create(sec_class, NULL, 0, NULL, "gps");
	if (IS_ERR(gps_dev))
		pr_err("Failed to create device(gps)!\n");

	if (gpio_is_valid(GPIO_MSENSE_nRST)) {
               if (gpio_request(GPIO_MSENSE_nRST, "GPB"))
                        printk(KERN_ERR "Failed to request GPIO_MSENSE_nRST! \n");
		gpio_direction_output(GPIO_MSENSE_nRST, 1);
	}
	gpio_free(GPIO_MSENSE_nRST);

	aries_init_wifi_mem();
	
    dpram_access_init();
    	sec_jack_gpio_init();
	printk("Set DEFAULT INFORM6 !!!!!!\n");
	__raw_writel(0xee00 , S5P_INFORM6);
	printk("Read Defaulit INFORM 6 : 0x%x\n", __raw_readl(S5P_INFORM6));
}

#if defined(CONFIG_USB_SUPPORT) || defined(CONFIG_VIDEO_TV20)
static int ldo38_onoff_state = 0;
#define LDO38_USB_BIT    (0x1)
#define LDO38_TVOUT_BIT  (0x2)
static DEFINE_MUTEX(ldo38_mutex);

static int ldo38_control(int module, bool onoff)
{
    mutex_lock(&ldo38_mutex);
    if (onoff)
    {
        if (!ldo38_onoff_state)
        {
            Set_MAX8998_PM_REG(ELDO3, 1);
            msleep(1);
            Set_MAX8998_PM_REG(ELDO8, 1);
            msleep(1);
        }
        printk(KERN_INFO "%s : turn ON LDO3 and LDO8 (cur_stat=%d, req=%d)\n",__func__,ldo38_onoff_state,module);
        ldo38_onoff_state |= module;
    }
    else
    {
        printk(KERN_INFO "%s : turn OFF LDO3 and LDO8 (cur_stat=%d, req=%d)\n",__func__,ldo38_onoff_state,module);
        ldo38_onoff_state &= ~module;
        if (!ldo38_onoff_state)
        {
            Set_MAX8998_PM_REG(ELDO8, 0);
            msleep(1);
            Set_MAX8998_PM_REG(ELDO3, 0);
        }
    }
    mutex_unlock(&ldo38_mutex);	
}

int ldo38_control_by_usb(bool onoff)
{
    ldo38_control(LDO38_USB_BIT, onoff);
}

int ldo38_control_by_tvout(bool onoff)
{
    ldo38_control(LDO38_TVOUT_BIT, onoff);
}
#endif

#ifdef CONFIG_USB_SUPPORT
/* Initializes OTG Phy. */
void otg_phy_init(void)
{
	__raw_writel(__raw_readl(S5P_USB_PHY_CONTROL)
		|(0x1<<0), S5P_USB_PHY_CONTROL); /*USB PHY0 Enable */
	__raw_writel((__raw_readl(S3C_USBOTG_PHYPWR)
		&~(0x3<<3)&~(0x1<<0))|(0x1<<5), S3C_USBOTG_PHYPWR);
	__raw_writel((__raw_readl(S3C_USBOTG_PHYCLK)
		&~(0x5<<2))|(0x3<<0), S3C_USBOTG_PHYCLK);
	__raw_writel((__raw_readl(S3C_USBOTG_RSTCON)
		&~(0x3<<1))|(0x1<<0), S3C_USBOTG_RSTCON);
	udelay(10);
	__raw_writel(__raw_readl(S3C_USBOTG_RSTCON)
		&~(0x7<<0), S3C_USBOTG_RSTCON);
	udelay(10);
	// Enables HS Transmitter pre-emphasis [20]
	__raw_writel(__raw_readl(S3C_USBOTG_PHYTUNE) |(0x1<<20), S3C_USBOTG_PHYTUNE);
	udelay(10);
	// HD DC Voltage Level Adjustment [3:0] (1000 : +10%)
	__raw_writel((__raw_readl(S3C_USBOTG_PHYTUNE) & ~(0xf)) | (0x1<<3), S3C_USBOTG_PHYTUNE);
	udelay(10);
}
EXPORT_SYMBOL(otg_phy_init);

/* USB Control request data struct must be located here for DMA transfer */
struct usb_ctrlrequest usb_ctrl __attribute__((aligned(64)));
EXPORT_SYMBOL(usb_ctrl);

/* OTG PHY Power Off */
void otg_phy_off(void)
{
	__raw_writel(__raw_readl(S3C_USBOTG_PHYPWR)
		|(0x3<<3), S3C_USBOTG_PHYPWR);
	__raw_writel(__raw_readl(S5P_USB_PHY_CONTROL)
		&~(1<<0), S5P_USB_PHY_CONTROL);
}
EXPORT_SYMBOL(otg_phy_off);

void usb_host_phy_init(void)
{
	struct clk *otg_clk;

	otg_clk = clk_get(NULL, "usbotg");
	clk_enable(otg_clk);

	if (readl(S5P_USB_PHY_CONTROL) & (0x1<<1))
		return;

	__raw_writel(__raw_readl(S5P_USB_PHY_CONTROL)
		|(0x1<<1), S5P_USB_PHY_CONTROL);
	__raw_writel((__raw_readl(S3C_USBOTG_PHYPWR)
		&~(0x1<<7)&~(0x1<<6))|(0x1<<8)|(0x1<<5), S3C_USBOTG_PHYPWR);
	__raw_writel((__raw_readl(S3C_USBOTG_PHYCLK)
		&~(0x1<<7))|(0x3<<0), S3C_USBOTG_PHYCLK);
	__raw_writel((__raw_readl(S3C_USBOTG_RSTCON))
		|(0x1<<4)|(0x1<<3), S3C_USBOTG_RSTCON);
	__raw_writel(__raw_readl(S3C_USBOTG_RSTCON)
		&~(0x1<<4)&~(0x1<<3), S3C_USBOTG_RSTCON);
}
EXPORT_SYMBOL(usb_host_phy_init);

void usb_host_phy_off(void)
{
	__raw_writel(__raw_readl(S3C_USBOTG_PHYPWR)
		|(0x1<<7)|(0x1<<6), S3C_USBOTG_PHYPWR);
	__raw_writel(__raw_readl(S5P_USB_PHY_CONTROL)
		&~(1<<1), S5P_USB_PHY_CONTROL);
}
EXPORT_SYMBOL(usb_host_phy_off);
#endif

#if defined(CONFIG_KEYPAD_S3C) || defined(CONFIG_KEYPAD_S3C_MODULE)
#if defined(CONFIG_KEYPAD_S3C_MSM)
void s3c_setup_keypad_cfg_gpio(void)
{
	unsigned int gpio;
	unsigned int end;

	/* gpio setting for KP_COL0 */
	s3c_gpio_cfgpin(S5PV210_GPJ1(5), S3C_GPIO_SFN(3));
	s3c_gpio_setpull(S5PV210_GPJ1(5), S3C_GPIO_PULL_NONE);

	/* gpio setting for KP_COL1 ~ KP_COL7 and KP_ROW0 */
	end = S5PV210_GPJ2(8);
	for (gpio = S5PV210_GPJ2(0); gpio < end; gpio++) {
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
	}

	/* gpio setting for KP_ROW1 ~ KP_ROW8 */
	end = S5PV210_GPJ3(8);
	for (gpio = S5PV210_GPJ3(0); gpio < end; gpio++) {
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
	}

	/* gpio setting for KP_ROW9 ~ KP_ROW13 */
	end = S5PV210_GPJ4(5);
	for (gpio = S5PV210_GPJ4(0); gpio < end; gpio++) {
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
	}
}
#else
void s3c_setup_keypad_cfg_gpio(int rows, int columns)
{
	unsigned int gpio;
	unsigned int end;

	end = S5PV210_GPH3(rows);

	/* Set all the necessary GPH2 pins to special-function 0 */
	for (gpio = S5PV210_GPH3(0); gpio < end; gpio++) {
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_UP);
	}

	end = S5PV210_GPH2(columns);

	/* Set all the necessary GPK pins to special-function 0 */
	for (gpio = S5PV210_GPH2(0); gpio < end; gpio++) {
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
	}
}
#endif /* if defined(CONFIG_KEYPAD_S3C_MSM)*/
EXPORT_SYMBOL(s3c_setup_keypad_cfg_gpio);
#endif

void
keypad_led_control(bool onOff)
{
	printk(KERN_DEBUG "keypad_led_control (HWREV : 0x%x)\n", HWREV);

    if( HWREV < 9 )
        return;

    if( onOff ) {
		Set_MAX8998_PM_OUTPUT_Voltage(LDO6, VCC_3p300);

		Set_MAX8998_PM_REG(ELDO6, 1);
    }
    else {
		Set_MAX8998_PM_REG(ELDO6, 0);
    }
}
EXPORT_SYMBOL(keypad_led_control);

MACHINE_START(SMDKC110, "SMDKC110")
	/* Maintainer: Kukjin Kim <kgene.kim@samsung.com> */
	.phys_io	= S3C_PA_UART & 0xfff00000,
	.io_pg_offst	= (((u32)S3C_VA_UART) >> 18) & 0xfffc,
	.boot_params	= S5P_PA_SDRAM + 0x100,
	.fixup		= smdkc110_fixup,
	.init_irq	= s5pv210_init_irq,
	.map_io		= smdkc110_map_io,
	.init_machine	= smdkc110_machine_init,
	.timer		= &s5p_systimer,
MACHINE_END

void s3c_setup_uart_cfg_gpio(unsigned char port)
{
	switch(port)
	{
	case 0:
		s3c_gpio_cfgpin(GPIO_BT_RXD, S3C_GPIO_SFN(GPIO_BT_RXD_AF));
		s3c_gpio_setpull(GPIO_BT_RXD, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(GPIO_BT_TXD, S3C_GPIO_SFN(GPIO_BT_TXD_AF));
		s3c_gpio_setpull(GPIO_BT_TXD, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(GPIO_BT_CTS, S3C_GPIO_SFN(GPIO_BT_CTS_AF));
		s3c_gpio_setpull(GPIO_BT_CTS, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(GPIO_BT_RTS, S3C_GPIO_SFN(GPIO_BT_RTS_AF));
		s3c_gpio_setpull(GPIO_BT_RTS, S3C_GPIO_PULL_NONE);
/*
		s3c_gpio_slp_cfgpin(GPIO_BT_RXD, S3C_GPIO_SLP_PREV);
		s3c_gpio_slp_setpull_updown(GPIO_BT_RXD, S3C_GPIO_PULL_NONE);
		s3c_gpio_slp_cfgpin(GPIO_BT_TXD, S3C_GPIO_SLP_PREV);
		s3c_gpio_slp_setpull_updown(GPIO_BT_TXD, S3C_GPIO_PULL_NONE);
		s3c_gpio_slp_cfgpin(GPIO_BT_CTS, S3C_GPIO_SLP_PREV);
		s3c_gpio_slp_setpull_updown(GPIO_BT_CTS, S3C_GPIO_PULL_NONE);
		s3c_gpio_slp_cfgpin(GPIO_BT_RTS, S3C_GPIO_SLP_PREV);
		s3c_gpio_slp_setpull_updown(GPIO_BT_RTS, S3C_GPIO_PULL_NONE);
*/
		break;
/* Stealth V - don't use AP GPS
	case 1:
		s3c_gpio_cfgpin(GPIO_GPS_RXD, S3C_GPIO_SFN(GPIO_GPS_RXD_AF));
		s3c_gpio_setpull(GPIO_GPS_RXD, S3C_GPIO_PULL_UP);
		s3c_gpio_cfgpin(GPIO_GPS_TXD, S3C_GPIO_SFN(GPIO_GPS_TXD_AF));
		s3c_gpio_setpull(GPIO_GPS_TXD, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(GPIO_GPS_CTS, S3C_GPIO_SFN(GPIO_GPS_CTS_AF));
		s3c_gpio_setpull(GPIO_GPS_CTS, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(GPIO_GPS_RTS, S3C_GPIO_SFN(GPIO_GPS_RTS_AF));
		s3c_gpio_setpull(GPIO_GPS_RTS, S3C_GPIO_PULL_NONE);
		break;
*/
	case 2:
		s3c_gpio_cfgpin(GPIO_AP_RXD, S3C_GPIO_SFN(GPIO_AP_RXD_AF));
		s3c_gpio_setpull(GPIO_AP_RXD, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(GPIO_AP_TXD, S3C_GPIO_SFN(GPIO_AP_TXD_AF));
		s3c_gpio_setpull(GPIO_AP_TXD, S3C_GPIO_PULL_NONE);
		break;
/* Stealth V - LTE modem use FLM with SPI interface
	case 3:
		s3c_gpio_cfgpin(GPIO_FLM_RXD, S3C_GPIO_SFN(GPIO_FLM_RXD_AF));
		s3c_gpio_setpull(GPIO_FLM_RXD, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(GPIO_FLM_TXD, S3C_GPIO_SFN(GPIO_FLM_TXD_AF));
		s3c_gpio_setpull(GPIO_FLM_TXD, S3C_GPIO_PULL_NONE);
		break;
*/
	default:
		break;
	}
}

EXPORT_SYMBOL(s3c_setup_uart_cfg_gpio);


/* << add uart gpio config - end */
/****************************/

static unsigned long wlan_reglock_flags = 0;
static spinlock_t wlan_reglock = SPIN_LOCK_UNLOCKED;

static unsigned int wlan_gpio_table[][4] = {	
	{GPIO_WLAN_nRST, GPIO_WLAN_nRST_AF, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE},
	{GPIO_WLAN_HOST_WAKE, GPIO_WLAN_HOST_WAKE_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN},
	{GPIO_WLAN_WAKE, GPIO_WLAN_WAKE_AF, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE},
};

static unsigned int wlan_sdio_on_table[][4] = {
	{GPIO_WLAN_SDIO_CLK, GPIO_WLAN_SDIO_CLK_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE},
	{GPIO_WLAN_SDIO_CMD, GPIO_WLAN_SDIO_CMD_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE},
	{GPIO_WLAN_SDIO_D0, GPIO_WLAN_SDIO_D0_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE},
	{GPIO_WLAN_SDIO_D1, GPIO_WLAN_SDIO_D1_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE},
	{GPIO_WLAN_SDIO_D2, GPIO_WLAN_SDIO_D2_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE},
	{GPIO_WLAN_SDIO_D3, GPIO_WLAN_SDIO_D3_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE},
};

static unsigned int wlan_sdio_off_table[][4] = {
	{GPIO_WLAN_SDIO_CLK, 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE},
	{GPIO_WLAN_SDIO_CMD, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE},
	{GPIO_WLAN_SDIO_D0, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE},
	{GPIO_WLAN_SDIO_D1, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE},
	{GPIO_WLAN_SDIO_D2, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE},
	{GPIO_WLAN_SDIO_D3, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE},
};

void wlan_setup_power(int on, int flag)
{
	printk(/*KERN_INFO*/ "%s %s", __func__, on ? "on" : "down");
	if (flag != 1) {	
		printk(/*KERN_DEBUG*/ "(on=%d, flag=%d)\n", on, flag);
//For Starting/Stopping Tethering service
#if 1
		if (on)
			gpio_set_value(GPIO_WLAN_nRST, GPIO_LEVEL_HIGH);
		else
			gpio_set_value(GPIO_WLAN_nRST, GPIO_LEVEL_LOW);		
#endif 
		return;
	}	
	printk(/*KERN_INFO*/ " --enter\n");
		
	if (on) {		
		s3c_config_gpio_alive_table(ARRAY_SIZE(wlan_gpio_table), wlan_gpio_table);
		s3c_config_gpio_alive_table(ARRAY_SIZE(wlan_sdio_on_table), wlan_sdio_on_table);
		
		/* PROTECT this check under spinlock.. No other thread should be touching
		 * GPIO_BT_REG_ON at this time.. If BT is operational, don't touch it. */
		spin_lock_irqsave(&wlan_reglock, wlan_reglock_flags);	
		/* need delay between v_bat & reg_on for 2 cycle @ 38.4MHz */
		udelay(5);
		
		gpio_set_value(GPIO_WLAN_BT_EN, GPIO_LEVEL_HIGH);
		s3c_gpio_slp_cfgpin(GPIO_WLAN_BT_EN, S3C_GPIO_SLP_OUT1);
		
		gpio_set_value(GPIO_WLAN_nRST, GPIO_LEVEL_HIGH);
		s3c_gpio_slp_cfgpin(GPIO_WLAN_nRST, S3C_GPIO_SLP_OUT1);
		
		printk(KERN_DEBUG "WLAN: GPIO_WLAN_BT_EN = %d, GPIO_WLAN_nRST = %d\n", 
			   gpio_get_value(GPIO_WLAN_BT_EN), gpio_get_value(GPIO_WLAN_nRST));
		
		spin_unlock_irqrestore(&wlan_reglock, wlan_reglock_flags);
	}
	else {
		/* PROTECT this check under spinlock.. No other thread should be touching
		 * GPIO_BT_REG_ON at this time.. If BT is operational, don't touch it. */
		spin_lock_irqsave(&wlan_reglock, wlan_reglock_flags);	
		/* need delay between v_bat & reg_on for 2 cycle @ 38.4MHz */
		udelay(5);
		
		if (gpio_get_value(GPIO_BT_nRST) == 0) {
			gpio_set_value(GPIO_WLAN_BT_EN, GPIO_LEVEL_LOW);	
			s3c_gpio_slp_cfgpin(GPIO_WLAN_BT_EN, S3C_GPIO_SLP_OUT0);
		}
		
		gpio_set_value(GPIO_WLAN_nRST, GPIO_LEVEL_LOW);
		s3c_gpio_slp_cfgpin(GPIO_WLAN_nRST, S3C_GPIO_SLP_OUT0);
		
		printk(KERN_DEBUG "WLAN: GPIO_WLAN_BT_EN = %d, GPIO_WLAN_nRST = %d\n", 
			   gpio_get_value(GPIO_WLAN_BT_EN), gpio_get_value(GPIO_WLAN_nRST));
		
		spin_unlock_irqrestore(&wlan_reglock, wlan_reglock_flags);
		
		s3c_config_gpio_alive_table(ARRAY_SIZE(wlan_sdio_off_table), wlan_sdio_off_table);		
	}
	
	/* mmc_rescan*/
	sdhci_s3c_force_presence_change(&s3c_device_hsmmc1);	
}
EXPORT_SYMBOL(wlan_setup_power);
