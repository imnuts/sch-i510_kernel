/* linux/arch/arm/plat-s5p/irq-eint.c
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * S5P - IRQ EINT support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/sysdev.h>

#include <asm/hardware/vic.h>

#include <plat/regs-irqtype.h>

#include <mach/map.h>
#include <plat/cpu.h>
#include <plat/pm.h>

#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <mach/regs-gpio.h>

static inline void s5p_irq_eint_mask(unsigned int irq)
{
	u32 mask;

	mask = __raw_readl(S5P_EINTMASK(eint_mask_reg(irq)));
	mask |= eint_irq_to_bit(irq);
	__raw_writel(mask, S5P_EINTMASK(eint_mask_reg(irq)));
}

static void s5p_irq_eint_unmask(unsigned int irq)
{
	u32 mask;

	mask = __raw_readl(S5P_EINTMASK(eint_mask_reg(irq)));
	mask &= ~(eint_irq_to_bit(irq));
	__raw_writel(mask, S5P_EINTMASK(eint_mask_reg(irq)));
}

static inline void s5p_irq_eint_ack(unsigned int irq)
{
	__raw_writel(eint_irq_to_bit(irq), S5P_EINTPEND(eint_pend_reg(irq)));
}

static void s5p_irq_eint_maskack(unsigned int irq)
{
	/* compiler should in-line these */
	s5p_irq_eint_mask(irq);
	s5p_irq_eint_ack(irq);
}

static int s5p_irq_eint_set_type(unsigned int irq, unsigned int type)
{
	int offs = eint_offset(irq);
	int shift;
	u32 ctrl, mask;
	u32 newvalue = 0;

	switch (type) {
	case IRQ_TYPE_NONE:
		printk(KERN_WARNING "No edge setting!\n");
		break;

	case IRQ_TYPE_EDGE_RISING:
		newvalue = S5P_EXTINT_RISEEDGE;
		break;

	case IRQ_TYPE_EDGE_FALLING:
		newvalue = S5P_EXTINT_FALLEDGE;
		break;

	case IRQ_TYPE_EDGE_BOTH:
		newvalue = S5P_EXTINT_BOTHEDGE;
		break;

	case IRQ_TYPE_LEVEL_LOW:
		newvalue = S5P_EXTINT_LOWLEV;
		break;

	case IRQ_TYPE_LEVEL_HIGH:
		newvalue = S5P_EXTINT_HILEV;
		break;

	default:
		printk(KERN_ERR "No such irq type %d", type);
		return -1;
	}

	shift = (offs & 0x7) * 4;
	mask = 0x7 << shift;

	ctrl = __raw_readl(S5P_EINTCON(eint_conf_reg(irq)));
	ctrl &= ~mask;
	ctrl |= newvalue << shift;
	__raw_writel(ctrl, S5P_EINTCON(eint_conf_reg(irq)));

	if ((0 <= offs) && (offs < 8))
		s3c_gpio_cfgpin(S5P_EXT_INT0(offs&0x7), 0xf<<((offs&0x7)*4));

	else if ((8 <= offs) && (offs < 16))
		s3c_gpio_cfgpin(S5P_EXT_INT1(offs&0x7), 0xf<<((offs&0x7)*4));

	else if ((16 <= offs) && (offs < 24))
		s3c_gpio_cfgpin(S5P_EXT_INT2(offs&0x7), 0xf<<((offs&0x7)*4));

	else if ((24 <= offs) && (offs < 32))
		s3c_gpio_cfgpin(S5P_EXT_INT3(offs&0x7), 0xf<<((offs&0x7)*4));

	else
		printk(KERN_ERR "No such irq number %d", offs);

	return 0;
}

static struct irq_chip s5p_irq_eint = {
	.name		= "s5p-eint",
	.mask		= s5p_irq_eint_mask,
	.unmask		= s5p_irq_eint_unmask,
	.mask_ack	= s5p_irq_eint_maskack,
	.ack		= s5p_irq_eint_ack,
	.set_type	= s5p_irq_eint_set_type,
#ifdef CONFIG_PM
	.set_wake	= s3c_irqext_wake,
#endif
};

/* s5p_irq_demux_eint
 *
 * This function demuxes the IRQ from the group0 external interrupts,
 * from IRQ_EINT(16) to IRQ_EINT(31). It is designed to be inlined into
 * the specific handlers s5p_irq_demux_eintX_Y.
 */
static inline void s5p_irq_demux_eint(unsigned int start, unsigned int end)
{
	u32 status = __raw_readl(S5P_EINTPEND((start >> 3)));
	u32 mask = __raw_readl(S5P_EINTMASK((start >> 3)));
	unsigned int irq;

	status &= ~mask;
	status &= (1 << (end - start + 1)) - 1;

	for (irq = IRQ_EINT(start); irq <= IRQ_EINT(end); irq++) {
		if (status & 1)
			generic_handle_irq(irq);

		status >>= 1;
	}
}

static void s5p_irq_demux_eint16_31(unsigned int irq, struct irq_desc *desc)
{
	s5p_irq_demux_eint(16, 23);
	s5p_irq_demux_eint(24, 31);
}

/* EINT0 ~ EINT15 */

static void s5p_irq_vic_eint_mask(unsigned int irq)
{
	void __iomem *base = get_irq_chip_data(irq);

	s5p_irq_eint_mask(irq);

	irq &= 31;
	writel(1 << irq, base + VIC_INT_ENABLE_CLEAR);
}

static void s5p_irq_vic_eint_unmask(unsigned int irq)
{
	void __iomem *base = get_irq_chip_data(irq);

	s5p_irq_eint_unmask(irq);

	irq &= 31;
	writel(1 << irq, base + VIC_INT_ENABLE);
}

static inline void s5p_irq_vic_eint_ack(unsigned int irq)
{
	__raw_writel(eint_irq_to_bit(irq), S5P_EINTPEND(eint_pend_reg(irq)));
}

static void s5p_irq_vic_eint_maskack(unsigned int irq)
{
	/* compiler should in-line these */
	s5p_irq_vic_eint_mask(irq);
	s5p_irq_vic_eint_ack(irq);
}

static struct irq_chip s5p_irq_vic_eint = {
	.name		= "s5p_vic_eint",
	.mask		= s5p_irq_vic_eint_mask,
	.unmask		= s5p_irq_vic_eint_unmask,
	.mask_ack	= s5p_irq_vic_eint_maskack,
	.ack		= s5p_irq_vic_eint_ack,
	.set_type	= s5p_irq_eint_set_type,
#ifdef CONFIG_PM
	.set_wake	= s3c_irqext_wake,
#endif

};

int __init s5p_init_irq_eint(void)
{
	int irq;

	for (irq = IRQ_EINT0; irq <= IRQ_EINT15; irq++)
		set_irq_chip(irq, &s5p_irq_vic_eint);

	for (irq = IRQ_EINT(16); irq <= IRQ_EINT(31); irq++) {
		set_irq_chip(irq, &s5p_irq_eint);
		set_irq_handler(irq, handle_level_irq);
		set_irq_flags(irq, IRQF_VALID);
	}

	set_irq_chained_handler(IRQ_EINT16_31, s5p_irq_demux_eint16_31);
	return 0;
}

arch_initcall(s5p_init_irq_eint);
