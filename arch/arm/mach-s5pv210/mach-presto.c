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

#ifdef CONFIG_KERNEL_DEBUG_SEC
#include <linux/kernel_sec_common.h>
#endif

#ifdef CONFIG_PRESTO_DEBUG_LED
#include <linux/sec_led.h>
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
//#include <mach/sec_jack.h>
#include <mach/param.h>

#if defined (CONFIG_SAMSUNG_PHONE_SVNET) || defined (CONFIG_SAMSUNG_PHONE_SVNET_MODULE)
#include <linux/modemctl.h>
#include <linux/onedram.h>
#include <linux/irq.h>
#endif 

struct class *sec_class;
EXPORT_SYMBOL(sec_class);

struct device *switch_dev;
EXPORT_SYMBOL(switch_dev);

void (*sec_set_param_value)(int idx, void *value);
EXPORT_SYMBOL(sec_set_param_value);
        
void (*sec_get_param_value)(int idx, void *value);
EXPORT_SYMBOL(sec_get_param_value);

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
                .name           = "VCC_DAC",
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
//      { MAX8998_DCDC4, &max8998_dcdc4_data },
        { MAX8998_LDO4, &max8998_ldo4_data },
//      { MAX8998_LDO11, &max8998_ldo11_data },
//      { MAX8998_LDO12, &max8998_ldo12_data },
//      { MAX8998_LDO13, &max8998_ldo13_data },
//      { MAX8998_LDO14, &max8998_ldo14_data },
//      { MAX8998_LDO15, &max8998_ldo15_data },
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

struct platform_device sec_device_dpram = {
        .name   = "dpram-device",
        .id             = -1,
};

struct platform_device s3c_device_8998consumer = {
        .name             = "max8998-consumer",
        .id               = 0,
        .dev = { .platform_data = &max8998_platform_data },
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

#ifdef CONFIG_S5P_ADC 
static struct s3c_adc_mach_info s3c_adc_platform __initdata = {
	/* s5pc110 support 12-bit resolution */
	.delay  = 10000,
	.presc  = 65,
	.resolution = 12,
};
#endif

/* I2C0 */
static struct i2c_board_info i2c_devs0[] __initdata = {
};

static struct i2c_board_info i2c_devs4[] __initdata = {
};

/* I2C1 */
static struct i2c_board_info i2c_devs1[] __initdata = {
};

static struct i2c_board_info i2c_devs5[] __initdata = {
	{
		I2C_BOARD_INFO("kr3dm", 0x09),
	},
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
	.name	= "jupiter-battery",
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
#if defined(CONFIG_ARIES_EUR)|| defined(CONFIG_PRESTO)
static struct modemctl_platform_data mdmctl_data = {
#if 1// defined(CONFIG_SAMSUNG_SVNET)
	.name = "lte",
	.gpio_phone_active = S5PV210_GPH3(5),
	.gpio_pda_active = S5PV210_GPC0(2),
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
//# error Need configuration (EUR/NTT) ???
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
		/* Off part */
		if((gpio <= S5PV210_GPG3(6)) ||
		   ((gpio <= S5PV210_GPJ4(7)) && (gpio >= S5PV210_GPI(0)))) {

	        s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(gpio_table[i][1]));
        	s3c_gpio_setpull(gpio, gpio_table[i][3]);

        	if (gpio_table[i][2] != S3C_GPIO_SETPIN_NONE)
            		gpio_set_value(gpio, gpio_table[i][2]);

        	s3c_gpio_set_drvstrength(gpio, gpio_table[i][4]);
        	//s3c_gpio_set_slewrate(gpio, gpio_table[i][5]);

        	//s3c_gpio_slp_cfgpin(gpio, gpio_table[i][6]);
        	//s3c_gpio_slp_setpull_updown(gpio, gpio_table[i][7]);
		}
#if 1
		/* Alive part */
		else if((gpio <= S5PV210_GPH3(7)) && (gpio >= S5PV210_GPH0(0))) {
	        s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(gpio_table[i][1]));
        	s3c_gpio_setpull(gpio, gpio_table[i][3]);

        	if (gpio_table[i][2] != S3C_GPIO_SETPIN_NONE)
            		gpio_set_value(gpio, gpio_table[i][2]);

        	s3c_gpio_set_drvstrength(gpio, gpio_table[i][4]);
        	//s3c_gpio_set_slewrate(gpio, gpio_table[i][5]);
		}
#endif
	}
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
//	char reset_mode = 'r';
	int phone_wait_cnt = 0;

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
		if (maxim_chg_status()) {
			mode = REBOOT_MODE_CHARGING;
			if (sec_set_param_value)
				sec_set_param_value(__REBOOT_MODE, &mode);
			/* Watchdog Reset */
			printk(KERN_EMERG
			       "%s: TA or USB connected, rebooting...\n",
			       __func__);
			kernel_sec_clear_upload_magic_number();
			kernel_sec_hw_reset(TRUE);
			printk(KERN_EMERG "%s: waiting for reset!\n", __func__);
			while (1) ;
		}
		kernel_sec_clear_upload_magic_number();
		// wait for power button release
		if (gpio_get_value(GPIO_N_POWER)) {
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
#if 0 // presto
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
	printk(KERN_DEBUG "SLPGPIO : PS_ON(%d) FM_RST(%d) UART_SEL(%d)\n",
		gpio_get_value(GPIO_PS_ON),gpio_get_value(GPIO_FM_RST),gpio_get_value(GPIO_UART_SEL));
#endif
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

static struct platform_device *smdkc110_devices[] __initdata = {
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

#ifdef CONFIG_HAVE_PWM
	&s3c_device_timer[0],
	&s3c_device_timer[1],
	&s3c_device_timer[2],
	&s3c_device_timer[3],
#endif

#ifdef	CONFIG_S5P_ADC
	&s3c_device_adc,
#endif
	&s3c_device_i2c0,
	&s3c_device_i2c1,
	&s3c_device_i2c2,
	&s3c_device_i2c6,  /* PMIC */
	&s3c_device_i2c7,  /* For FSA micro USB switch */
        &s3c_device_i2c9,  /* fuel gague */

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


#ifdef CONFIG_ANDROID_PMEM
	&pmem_device,
	&pmem_gpu1_device,
	&pmem_adsp_device,
#endif
	&sec_device_battery,
	&sec_device_rfkill,
	&sec_device_btsleep,
	&opt_gp2a 
};

unsigned int HWREV=0;
EXPORT_SYMBOL(HWREV);
/*
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

*/
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
		else if (!strcmp((char *)_cmd, "recovery"))
			mode = REBOOT_MODE_RECOVERY;
		else if (!strcmp((char *)_cmd, "download"))
			mode = REBOOT_MODE_DOWNLOAD;
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

static void __init smdkc110_machine_init(void)
{
	int i,j;
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


//	qt_touch_init();

#ifdef CONFIG_DM9000
	smdkv210_dm9000_set();
#endif

#ifdef CONFIG_ANDROID_PMEM
	android_pmem_set_platdata();
#endif
	/* i2c */
	s3c_i2c0_set_platdata(NULL);
	s3c_i2c1_set_platdata(NULL);
	s3c_i2c2_set_platdata(NULL);
	i2c_register_board_info(0, i2c_devs0, ARRAY_SIZE(i2c_devs0));
	i2c_register_board_info(1, i2c_devs1, ARRAY_SIZE(i2c_devs1));
//	i2c_register_board_info(2, i2c_devs2, ARRAY_SIZE(i2c_devs2));
	i2c_register_board_info(4, i2c_devs4, ARRAY_SIZE(i2c_devs4));
	i2c_register_board_info(5, i2c_devs5, ARRAY_SIZE(i2c_devs5));
	i2c_register_board_info(6, i2c_devs6, ARRAY_SIZE(i2c_devs6));
	i2c_register_board_info(7, i2c_devs7, ARRAY_SIZE(i2c_devs7)); /* for fsa9480 */
//	i2c_register_board_info(8, i2c_devs8, ARRAY_SIZE(i2c_devs8)); /* for Si4709 */	
	i2c_register_board_info(9, i2c_devs9, ARRAY_SIZE(i2c_devs9));
//	i2c_register_board_info(10, i2c_devs10, ARRAY_SIZE(i2c_devs10)); /* for touchkey */
	i2c_register_board_info(11, i2c_devs11, ARRAY_SIZE(i2c_devs11)); /* optical sensor */
//	i2c_register_board_info(12, i2c_devs12, ARRAY_SIZE(i2c_devs12)); /* magnetic sensor */


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
#if 0// defined(CONFIG_HAVE_PWM)
	smdk_backlight_register();
#endif

	register_reboot_notifier(&arise_reboot_notifier);

	jupiter_switch_init();

	BUG_ON(!sec_class);
    	dpram_access_init();

#ifdef CONFIG_PRESTO_DEBUG_LED
	sec_set_led_state(PWR_LED, LED_BLUE);
#endif
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
	return 0;
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
/*
	case 0:
		s3c_gpio_cfgpin(GPIO_BT_RXD, S3C_GPIO_SFN(GPIO_BT_RXD_AF));
		s3c_gpio_setpull(GPIO_BT_RXD, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(GPIO_BT_TXD, S3C_GPIO_SFN(GPIO_BT_TXD_AF));
		s3c_gpio_setpull(GPIO_BT_TXD, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(GPIO_BT_CTS, S3C_GPIO_SFN(GPIO_BT_CTS_AF));
		s3c_gpio_setpull(GPIO_BT_CTS, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(GPIO_BT_RTS, S3C_GPIO_SFN(GPIO_BT_RTS_AF));
		s3c_gpio_setpull(GPIO_BT_RTS, S3C_GPIO_PULL_NONE);

		s3c_gpio_slp_cfgpin(GPIO_BT_RXD, S3C_GPIO_SLP_PREV);
		s3c_gpio_slp_setpull_updown(GPIO_BT_RXD, S3C_GPIO_PULL_NONE);
		s3c_gpio_slp_cfgpin(GPIO_BT_TXD, S3C_GPIO_SLP_PREV);
		s3c_gpio_slp_setpull_updown(GPIO_BT_TXD, S3C_GPIO_PULL_NONE);
		s3c_gpio_slp_cfgpin(GPIO_BT_CTS, S3C_GPIO_SLP_PREV);
		s3c_gpio_slp_setpull_updown(GPIO_BT_CTS, S3C_GPIO_PULL_NONE);
		s3c_gpio_slp_cfgpin(GPIO_BT_RTS, S3C_GPIO_SLP_PREV);
		s3c_gpio_slp_setpull_updown(GPIO_BT_RTS, S3C_GPIO_PULL_NONE);

		break;
*/
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
/*	-- presto disable the BT	
		if (gpio_get_value(GPIO_BT_nRST) == 0) {
			gpio_set_value(GPIO_WLAN_BT_EN, GPIO_LEVEL_LOW);	
			s3c_gpio_slp_cfgpin(GPIO_WLAN_BT_EN, S3C_GPIO_SLP_OUT0);
		}
*/		
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

