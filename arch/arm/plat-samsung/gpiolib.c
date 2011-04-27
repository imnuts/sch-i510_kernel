/* arch/arm/plat-samsung/gpiolib.c
 *
 * Copyright 2008 Openmoko, Inc.
 * Copyright 2008 Simtec Electronics
 *      Ben Dooks <ben@simtec.co.uk>
 *      http://armlinux.simtec.co.uk/
 *
 * Copyright (c) 2009 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * SAMSUNG - GPIOlib support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <mach/gpio.h>
#include <plat/gpio-core.h>
#include <plat/gpio-cfg.h>
#include <plat/gpio-cfg-helpers.h>

#ifndef DEBUG_GPIO
#define gpio_dbg(x...) do { } while (0)
#else
#define gpio_dbg(x...) printk(KERN_DEBUG x)
#endif

/* The samsung_gpiolib_4bit routines are to control the gpio banks where
 * the gpio configuration register (GPxCON) has 4 bits per GPIO, as the
 * following example:
 *
 * base + 0x00: Control register, 4 bits per gpio
 *		gpio n: 4 bits starting at (4*n)
 *		0000 = input, 0001 = output, others mean special-function
 * base + 0x04: Data register, 1 bit per gpio
 *		bit n: data bit n
 *
 * Note, since the data register is one bit per gpio and is at base + 0x4
 * we can use s3c_gpiolib_get and s3c_gpiolib_set to change the state of
 * the output.
*/

static int samsung_gpiolib_4bit_input(struct gpio_chip *chip,
				      unsigned int offset)
{
	struct s3c_gpio_chip *ourchip = to_s3c_gpio(chip);
	void __iomem *base = ourchip->base;
	unsigned long con;

	con = __raw_readl(base + GPIOCON_OFF);
	con &= ~(0xf << con_4bit_shift(offset));
	__raw_writel(con, base + GPIOCON_OFF);

	gpio_dbg("%s: %p: CON now %08lx\n", __func__, base, con);

	return 0;
}

static int samsung_gpiolib_4bit_output(struct gpio_chip *chip,
				       unsigned int offset, int value)
{
	struct s3c_gpio_chip *ourchip = to_s3c_gpio(chip);
	void __iomem *base = ourchip->base;
	unsigned long con;
	unsigned long dat;

	con = __raw_readl(base + GPIOCON_OFF);
	con &= ~(0xf << con_4bit_shift(offset));
	con |= 0x1 << con_4bit_shift(offset);

	dat = __raw_readl(base + GPIODAT_OFF);

	if (value)
		dat |= 1 << offset;
	else
		dat &= ~(1 << offset);

	__raw_writel(dat, base + GPIODAT_OFF);
	__raw_writel(con, base + GPIOCON_OFF);
	__raw_writel(dat, base + GPIODAT_OFF);

	gpio_dbg("%s: %p: CON %08lx, DAT %08lx\n", __func__, base, con, dat);

	return 0;
}

/* The next set of routines are for the case where the GPIO configuration
 * registers are 4 bits per GPIO but there is more than one register (the
 * bank has more than 8 GPIOs.
 *
 * This case is the similar to the 4 bit case, but the registers are as
 * follows:
 *
 * base + 0x00: Control register, 4 bits per gpio (lower 8 GPIOs)
 *		gpio n: 4 bits starting at (4*n)
 *		0000 = input, 0001 = output, others mean special-function
 * base + 0x04: Control register, 4 bits per gpio (up to 8 additions GPIOs)
 *		gpio n: 4 bits starting at (4*n)
 *		0000 = input, 0001 = output, others mean special-function
 * base + 0x08: Data register, 1 bit per gpio
 *		bit n: data bit n
 *
 * To allow us to use the s3c_gpiolib_get and s3c_gpiolib_set routines we
 * store the 'base + 0x4' address so that these routines see the data
 * register at ourchip->base + 0x04.
 */

static int samsung_gpiolib_4bit2_input(struct gpio_chip *chip,
				       unsigned int offset)
{
	struct s3c_gpio_chip *ourchip = to_s3c_gpio(chip);
	void __iomem *base = ourchip->base;
	void __iomem *regcon = base;
	unsigned long con;

	if (offset > 7)
		offset -= 8;
	else
		regcon -= 4;

	con = __raw_readl(regcon);
	con &= ~(0xf << con_4bit_shift(offset));
	__raw_writel(con, regcon);

	gpio_dbg("%s: %p: CON %08lx\n", __func__, base, con);

	return 0;
}

static int samsung_gpiolib_4bit2_output(struct gpio_chip *chip,
					unsigned int offset, int value)
{
	struct s3c_gpio_chip *ourchip = to_s3c_gpio(chip);
	void __iomem *base = ourchip->base;
	void __iomem *regcon = base;
	unsigned long con;
	unsigned long dat;
	unsigned con_offset = offset;

	if (con_offset > 7)
		con_offset -= 8;
	else
		regcon -= 4;

	con = __raw_readl(regcon);
	con &= ~(0xf << con_4bit_shift(con_offset));
	con |= 0x1 << con_4bit_shift(con_offset);

	dat = __raw_readl(base + GPIODAT_OFF);

	if (value)
		dat |= 1 << offset;
	else
		dat &= ~(1 << offset);

	__raw_writel(dat, base + GPIODAT_OFF);
	__raw_writel(con, regcon);
	__raw_writel(dat, base + GPIODAT_OFF);

	gpio_dbg("%s: %p: CON %08lx, DAT %08lx\n", __func__, base, con, dat);

	return 0;
}


int s3c_gpio_slp_cfgpin(unsigned int pin, unsigned int config)
{
        struct s3c_gpio_chip *chip = s3c_gpiolib_getchip(pin);
        void __iomem *reg;
        unsigned long flags;
        int offset;
        u32 con;
        int shift;

        if (!chip)
                return -EINVAL;

	if((pin <= S5PV210_GPH3(7)) && (pin >= S5PV210_GPH0(0))) {
                return -EINVAL;
        }

        if(config > S3C_GPIO_SLP_PREV)
        {
                 return -EINVAL;
        }

        reg = chip->base + 0x10;

        offset = pin - chip->chip.base;
        shift = offset * 2;

        local_irq_save(flags);

        con = __raw_readl(reg);
        con &= ~(3 << shift);
        con |= config << shift;
         __raw_writel(con, reg);

        local_irq_restore(flags);
        return 0;
}


s3c_gpio_pull_t s3c_gpio_get_slp_cfgpin(unsigned int pin)
{
        struct s3c_gpio_chip *chip = s3c_gpiolib_getchip(pin);
        void __iomem *reg;
        unsigned long flags;
        int offset;
        u32 con;
        int shift;

        if (!chip)
                return -EINVAL;

	if((pin <= S5PV210_GPH3(7)) && (pin >= S5PV210_GPH0(0))) {
                return -EINVAL;
        }

        reg = chip->base + 0x10;

        offset = pin - chip->chip.base;
        shift = offset * 2;

        local_irq_save(flags);

        con = __raw_readl(reg);
        con >>= shift;
        con &= 0x3;

        local_irq_restore(flags);

        return (__force s3c_gpio_pull_t)con;
}


int s3c_gpio_slp_setpull_updown(unsigned int pin, unsigned int config)
{
        struct s3c_gpio_chip *chip = s3c_gpiolib_getchip(pin);
        void __iomem *reg;
        unsigned long flags;
        int offset;
        u32 con;
        int shift;

        if (!chip)
                return -EINVAL;

	if((pin <= S5PV210_GPH3(7)) && (pin >= S5PV210_GPH0(0))) {
                return -EINVAL;
        }

        if(config > S3C_GPIO_PULL_UP)
        {
                return -EINVAL;
        }
        reg = chip->base + 0x14;

        offset = pin - chip->chip.base;
        shift = offset * 2;

        local_irq_save(flags);

        con = __raw_readl(reg);
        con &= ~(3 << shift);
        con |= config << shift;
        __raw_writel(con, reg);

        local_irq_restore(flags);
                           
	return 0;
}
EXPORT_SYMBOL(s3c_gpio_slp_setpull_updown);


int s3c_gpio_set_drvstrength(unsigned int pin, unsigned int config)
{
        struct s3c_gpio_chip *chip = s3c_gpiolib_getchip(pin);
        void __iomem *reg;
        unsigned long flags;
        int offset;
        u32 con;
        int shift;

        if (!chip)
                return -EINVAL;

        if(config  > S3C_GPIO_DRVSTR_4X)
        {
                return -EINVAL;
        }

        reg = chip->base + 0x0c;

        offset = pin - chip->chip.base;
        shift = offset * 2; 

        local_irq_save(flags);

        con = __raw_readl(reg);
        con &= ~(3 << shift);
        con |= config << shift;

        __raw_writel(con, reg);
#ifdef  S5PC11X_ALIVEGPIO_STORE
        con = __raw_readl(reg);
#endif

        local_irq_restore(flags);
                           
	return 0;
}

int s3c_gpio_set_slewrate(unsigned int pin, unsigned int config)
{
        struct s3c_gpio_chip *chip = s3c_gpiolib_getchip(pin);
        void __iomem *reg;
        unsigned long flags;
        int offset;
        u32 con;
        int shift;

        if (!chip)
                return -EINVAL;

        if(config > S3C_GPIO_SLEWRATE_SLOW)
        {
                return -EINVAL;
        }

        reg = chip->base + 0x0c;

        offset = pin - chip->chip.base;
        shift = offset; 

        local_irq_save(flags);

        con = __raw_readl(reg);
        con &= ~(1<< shift);
        con |= config << shift;

        __raw_writel(con, reg);
#ifdef  S5PC11X_ALIVEGPIO_STORE
        con = __raw_readl(reg);
#endif

        local_irq_restore(flags);
                           
	return 0;
}

















void __init samsung_gpiolib_add_4bit(struct s3c_gpio_chip *chip)
{
	chip->chip.direction_input = samsung_gpiolib_4bit_input;
	chip->chip.direction_output = samsung_gpiolib_4bit_output;
	chip->pm = __gpio_pm(&s3c_gpio_pm_4bit);
}

void __init samsung_gpiolib_add_4bit2(struct s3c_gpio_chip *chip)
{
	chip->chip.direction_input = samsung_gpiolib_4bit2_input;
	chip->chip.direction_output = samsung_gpiolib_4bit2_output;
	chip->pm = __gpio_pm(&s3c_gpio_pm_4bit);
}

void __init samsung_gpiolib_add_4bit_chips(struct s3c_gpio_chip *chip,
					   int nr_chips)
{
	for (; nr_chips > 0; nr_chips--, chip++) {
		samsung_gpiolib_add_4bit(chip);
		s3c_gpiolib_add(chip);
	}
}

void __init samsung_gpiolib_add_4bit2_chips(struct s3c_gpio_chip *chip,
					    int nr_chips)
{
	for (; nr_chips > 0; nr_chips--, chip++) {
		samsung_gpiolib_add_4bit2(chip);
		s3c_gpiolib_add(chip);
	}
}
