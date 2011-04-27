/* linux/arch/arm/plat-s3c/dev-audio.c
 *
 * Copyright (c) 2009 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * Author: Jaswinder Singh <jassi.brar@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/io.h>

#include <mach/map.h>
#include <mach/dma.h>

#include <plat/devs.h>
#include <plat/audio.h>
#include <plat/gpio-cfg.h>
#include <mach/regs-clock.h>

static int s3c64xx_cfg_i2s(struct platform_device *pdev)
{
	/* configure GPIO for i2s port */
	switch (pdev->id) {
	case 0:
		s3c_gpio_cfgpin(S5PV210_GPI(0), (0x2<<0));
		s3c_gpio_cfgpin(S5PV210_GPI(1), (0x2<<4));
		s3c_gpio_cfgpin(S5PV210_GPI(2), (0x2<<8));
		s3c_gpio_cfgpin(S5PV210_GPI(3), (0x2<<12));
		s3c_gpio_cfgpin(S5PV210_GPI(4), (0x2<<16));
		s3c_gpio_cfgpin(S5PV210_GPI(5), (0x2<<20));
		s3c_gpio_cfgpin(S5PV210_GPI(6), (0x2<<24));
		break;
/*
	case 1:
		s3c_gpio_cfgpin(S5PV210_GPC0(0), (0x2<<0));
		s3c_gpio_cfgpin(S5PV210_GPC0(1), (0x2<<4));
		s3c_gpio_cfgpin(S5PV210_GPC0(2), (0x2<<8));
		s3c_gpio_cfgpin(S5PV210_GPC0(3), (0x2<<12));
		s3c_gpio_cfgpin(S5PV210_GPC0(4), (0x2<<16));
		break;
*/
	case 2:
		s3c_gpio_cfgpin(S5PV210_GPC1(0), (0x4<<0));
		s3c_gpio_cfgpin(S5PV210_GPC1(1), (0x4<<4));
		s3c_gpio_cfgpin(S5PV210_GPC1(2), (0x4<<8));
		s3c_gpio_cfgpin(S5PV210_GPC1(3), (0x4<<12));
		s3c_gpio_cfgpin(S5PV210_GPC1(4), (0x4<<16));
		break;

	default:
		printk("Invalid Device %d!\n", pdev->id);
		return -EINVAL;
	}

	return 0;
}

static struct s3c_audio_pdata s3c64xx_i2s_pdata = {
	.cfg_gpio = s3c64xx_cfg_i2s,
};

static struct resource s3c64xx_iis0_resource[] = {
	[0] = {
		.start = S5P_PA_IIS0, /* V50 */
		.end   = S5P_PA_IIS0 + 0x100 - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = DMACH_I2S0_OUT,
		.end   = DMACH_I2S0_OUT,
		.flags = IORESOURCE_DMA,
	},
	[2] = {
		.start = DMACH_I2S0_IN,
		.end   = DMACH_I2S0_IN,
		.flags = IORESOURCE_DMA,
	},
};

struct platform_device s3c64xx_device_iis0 = {
	.name		  = "s3c64xx-iis",
	.id		  = 0,
	.num_resources	  = ARRAY_SIZE(s3c64xx_iis0_resource),
	.resource	  = s3c64xx_iis0_resource,
	.dev = {
		.platform_data = &s3c64xx_i2s_pdata,
	},
};
EXPORT_SYMBOL(s3c64xx_device_iis0);

static struct resource s3c64xx_iis1_resource[] = {
	[0] = {
		.start = S5P_PA_IIS1,
		.end   = S5P_PA_IIS1 + 0x100 - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = DMACH_I2S1_OUT,
		.end   = DMACH_I2S1_OUT,
		.flags = IORESOURCE_DMA,
	},
	[2] = {
		.start = DMACH_I2S1_IN,
		.end   = DMACH_I2S1_IN,
		.flags = IORESOURCE_DMA,
	},
};

struct platform_device s3c64xx_device_iis1 = {
	.name		  = "s3c64xx-iis",
	.id		  = 1,
	.num_resources	  = ARRAY_SIZE(s3c64xx_iis1_resource),
	.resource	  = s3c64xx_iis1_resource,
	.dev = {
		.platform_data = &s3c64xx_i2s_pdata,
	},
};
EXPORT_SYMBOL(s3c64xx_device_iis1);

static struct resource s3c64xx_iis2_resource[] = {
	[0] = {
		.start = S5P_PA_IIS2,
		.end   = S5P_PA_IIS2 + 0x100 - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = DMACH_I2S2_OUT,
		.end   = DMACH_I2S2_OUT,
		.flags = IORESOURCE_DMA,
	},
	[2] = {
		.start = DMACH_I2S2_IN,
		.end   = DMACH_I2S2_IN,
		.flags = IORESOURCE_DMA,
	},
};

struct platform_device s3c64xx_device_iis2 = {
	.name		  = "s3c64xx-iis",
	.id		  = 2,
	.num_resources	  = ARRAY_SIZE(s3c64xx_iis2_resource),
	.resource	  = s3c64xx_iis2_resource,
	.dev = {
		.platform_data = &s3c64xx_i2s_pdata,
	},
};
EXPORT_SYMBOL(s3c64xx_device_iis2);
