/* linux/arch/arm/plat-s5p/setup-i2c1.c
 *
 * Copyright (c) 2009 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * I2C0 GPIO configuration.
 *
 * Based on plat-s3c64xx/setup-i2c1.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/io.h>

struct platform_device; /* don't need the contents */

#include <mach/gpio.h>
#include <mach/regs-gpio.h>
#include <plat/iic.h>
#include <plat/gpio-cfg.h>

void s3c_i2c1_cfg_gpio(struct platform_device *dev)
{
	unsigned int gpio;

	s3c_gpio_cfgpin(S5PV210_GPD1(2), S3C_GPIO_SFN(2));
	s3c_gpio_cfgpin(S5PV210_GPD1(3), S3C_GPIO_SFN(2));

	s3c_gpio_setpull(S5PV210_GPD1(2), S3C_GPIO_PULL_NONE);
	s3c_gpio_setpull(S5PV210_GPD1(3), S3C_GPIO_PULL_NONE);

	gpio = readl(S5PV210_GPD1DRV);
	writel(gpio | 0x00f0, S5PV210_GPD1DRV);
}
