/* linux/arch/arm/plat-samsung/dev-ide.c
 *
 * Copyright (C) 2009 Samsung Electronics
 * 	http://samsungsemi.com
 *
 * S3C IDE device definition.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <mach/map.h>
#include <mach/regs-gpio.h>
#include <plat/gpio-cfg.h>
#include <mach/gpio.h>
#include <mach/gpio-bank.h>
#include <plat/ide.h>

static struct resource s3c_cfcon_resource[] = {
	[0] = {
		.start  = S5PV210_PA_CFCON,
		.end    = S5PV210_PA_CFCON + S5PV210_SZ_CFCON - 1,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = IRQ_CFC,
		.end    = IRQ_CFC,
		.flags  = IORESOURCE_IRQ,
	},
};

struct platform_device s3c_device_cfcon = {
	.name           = "s3c-ide",
	.id             = 0,
	.num_resources  = ARRAY_SIZE(s3c_cfcon_resource),
	.resource       = s3c_cfcon_resource,
};
EXPORT_SYMBOL(s3c_device_cfcon);

void s3c_ide_set_platdata(struct s3c_ide_platdata *pdata)
{
	struct s3c_ide_platdata *pd;

	pd = (struct s3c_ide_platdata *)kmemdup(pdata,
		sizeof(struct s3c_ide_platdata), GFP_KERNEL);
	if (!pd) {
		printk(KERN_ERR "%s: no memory for platform data\n", __func__);
		return;
	}
	s3c_device_cfcon.dev.platform_data = pd;
}

void s3c_ide_setup_gpio(void)
{
	u8 i;

	/* CF_Add[0 - 2], CF_IORDY, CF_INTRQ, CF_DMARQ, CF_DMARST, CF_DMACK */
	for (i = 0; i < 8; i++) {
		s3c_gpio_cfgpin(S5PV210_GPJ0(i), S3C_GPIO_SFN(4));
		s3c_gpio_setpull(S5PV210_GPJ0(i), S3C_GPIO_PULL_NONE);
	}
	writel(0xffff, S5PV210_GPJ0DRV);

	/*CF_Data[0 - 7] */
	for (i = 0; i < 8; i++) {
		s3c_gpio_cfgpin(S5PV210_GPJ2(i), S3C_GPIO_SFN(4));
		s3c_gpio_setpull(S5PV210_GPJ2(i), S3C_GPIO_PULL_NONE);
	}
	writel(0xffff, S5PV210_GPJ2DRV);

	/* CF_Data[8 - 15] */
	for (i = 0; i < 8; i++) {
		s3c_gpio_cfgpin(S5PV210_GPJ3(i), S3C_GPIO_SFN(4));
		s3c_gpio_setpull(S5PV210_GPJ3(i), S3C_GPIO_PULL_NONE);
	}
	writel(0xffff, S5PV210_GPJ3DRV);

	/* CF_CS0, CF_CS1, CF_IORD, CF_IOWR */
	for (i = 0; i < 4; i++) {
		s3c_gpio_cfgpin(S5PV210_GPJ4(i), S3C_GPIO_SFN(4));
		s3c_gpio_setpull(S5PV210_GPJ4(i), S3C_GPIO_PULL_NONE);
	}
	writel(0xff, S5PV210_GPJ4DRV);
}
