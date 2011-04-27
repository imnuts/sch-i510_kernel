/* linux/arch/arm/mach-s5pv210/button-smdkv210.c
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * S5PV210 - Button Driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/serial_core.h>
#include <linux/io.h>
#include <linux/platform_device.h>

#include <plat/map-base.h>
#include <plat/gpio-cfg.h>

#include <mach/regs-gpio.h>
#include <mach/regs-irq.h>
#include <linux/gpio.h>

static irqreturn_t
s3c_button_interrupt(int irq, void *dev_id)
{
	if (irq == IRQ_EINT4)
		printk(KERN_INFO "XEINT 4 Button Interrupt occure\n");
	else if (irq == IRQ_EINT(31))
		printk(KERN_INFO "XEINT 31 Button Interrupt occure\n");
	else
		printk(KERN_INFO "%d Button Interrupt occure\n", irq);

	return IRQ_HANDLED;
}

static struct irqaction s3c_button_irq = {
	.name		= "s3c button Tick",
	.flags		= IRQF_SHARED ,
	.handler	= s3c_button_interrupt,
};

static unsigned int s3c_button_gpio_init(void)
{
	u32 err;

	err = gpio_request(S5PV210_GPH0(4), "GPH0");
	if (err) {
		printk(KERN_INFO "gpio request error : %d\n", err);
	} else {
		s3c_gpio_cfgpin(S5PV210_GPH0(4), (0xf << 16));
		s3c_gpio_setpull(S5PV210_GPH0(4), S3C_GPIO_PULL_NONE);
	}

	err = gpio_request(S5PV210_GPH3(7), "GPH3");
	if (err) {
		printk(KERN_INFO "gpio request error : %d\n", err);
	} else {
		s3c_gpio_cfgpin(S5PV210_GPH3(7), (0xf << 28));
		s3c_gpio_setpull(S5PV210_GPH3(7), S3C_GPIO_PULL_NONE);
	}

	return err;
}

static int __init s3c_button_init(void)
{
	printk(KERN_INFO "SMDKC110 Button init function \n");

	if (s3c_button_gpio_init()) {
		printk(KERN_ERR "%s failed\n", __func__);
		return 0;
	}

	set_irq_type(IRQ_EINT4, IRQF_TRIGGER_FALLING);
	set_irq_wake(IRQ_EINT4, 1);
	setup_irq(IRQ_EINT4, &s3c_button_irq);

	set_irq_type(IRQ_EINT(31), IRQ_TYPE_EDGE_FALLING);
	set_irq_wake(IRQ_EINT(31), 1);
	setup_irq(IRQ_EINT(31), &s3c_button_irq);

	return 0;
}
late_initcall(s3c_button_init);
