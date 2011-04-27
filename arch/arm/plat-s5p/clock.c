/* linux/arch/arm/plat-s5p/clock.c
 *
 * Copyright 2009 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * S5P - Common clock support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/sysdev.h>
#include <linux/io.h>
#include <asm/div64.h>

#include <plat/clock.h>
#include <plat/clock-clksrc.h>
#include <plat/s5p-clock.h>

#include <mach/regs-power.h>

#ifdef CONFIG_PM_PWR_GATING
#include <mach/powergating.h>
extern int tvblk_turnon;
#endif

/* fin_apll, fin_mpll and fin_epll are all the same clock, which we call
 * clk_ext_xtal_mux.
*/
struct clk clk_ext_xtal_mux = {
	.name		= "ext_xtal",
	.id		= -1,
};

struct clk clk_xusbxti = {
	.name		= "xusbxti",
	.id		= -1,
	.rate		= 24000000,
};

struct clk clk_xrtcxti = {
	.name		= "xrtcxti",
	.id		= -1,
	.rate		= 32768,
};

static struct clk s5p_clk_27m = {
	.name		= "clk_27m",
	.id		= -1,
	.rate		= 27000000,
};

/* 48MHz USB Phy clock output */
struct clk clk_48m = {
	.name		= "clk_48m",
	.id		= -1,
	.rate		= 48000000,
};

/* APLL clock output
 * No need .ctrlbit, this is always on
*/
struct clk clk_fout_apll = {
	.name		= "fout_apll",
	.id		= -1,
};

/* MPLL clock output
 * No need .ctrlbit, this is always on
*/
struct clk clk_fout_mpll = {
	.name		= "fout_mpll",
	.id		= -1,
};

/* EPLL clock output */
struct clk clk_fout_epll = {
	.name		= "fout_epll",
	.id		= -1,
	.ctrlbit	= (1 << 31),
};

/* ARM clock */
struct clk clk_arm = {
	.name		= "armclk",
	.id		= -1,
	.rate		= 0,
	.ctrlbit	= 0,
};

/* Possible clock sources for APLL Mux */
static struct clk *clk_src_apll_list[] = {
	[0] = &clk_fin_apll,
	[1] = &clk_fout_apll,
};

struct clksrc_sources clk_src_apll = {
	.sources	= clk_src_apll_list,
	.nr_sources	= ARRAY_SIZE(clk_src_apll_list),
};

/* Possible clock sources for MPLL Mux */
static struct clk *clk_src_mpll_list[] = {
	[0] = &clk_fin_mpll,
	[1] = &clk_fout_mpll,
};

struct clksrc_sources clk_src_mpll = {
	.sources	= clk_src_mpll_list,
	.nr_sources	= ARRAY_SIZE(clk_src_mpll_list),
};

/* Possible clock sources for EPLL Mux */
static struct clk *clk_src_epll_list[] = {
	[0] = &clk_fin_epll,
	[1] = &clk_fout_epll,
};

struct clksrc_sources clk_src_epll = {
	.sources	= clk_src_epll_list,
	.nr_sources	= ARRAY_SIZE(clk_src_epll_list),
};

struct clk clk_vpll = {
	.name		= "vpll",
	.id		= -1,
};

extern int s5pc11x_blk_gate(struct clk *clk, int enable);

int s5p_gatectrl(void __iomem *reg, struct clk *clk, int enable)
{
	unsigned int ctrlbit = clk->ctrlbit;
	u32 con, mask;

	con = __raw_readl(reg);


	if (enable) {

		/*Disable the src mask before enabling the gating*/
		if(clk->srcMaskReg)
		{
			mask = __raw_readl(clk->srcMaskReg);
			mask |= (clk->srcMaskBit);
			__raw_writel(mask, clk->srcMaskReg);
			//printk("\nenable mask for %s\n",clk->name);
		}
#ifdef CONFIG_PM_PWR_GATING
		if(clk->powerDomain)
		{
			s5p_power_gating(clk->powerDomain,  DOMAIN_ACTIVE_MODE);
		}
#endif
		/*Enable clock level block, if required*/
		if(clk->gb)
			s5pc11x_blk_gate(clk, enable);

		con |= ctrlbit;
		__raw_writel(con, reg);

	} else {
		con &= ~ctrlbit;
		__raw_writel(con, reg);

		/*Disable clock level block, if required*/
		if(clk->gb)
			s5pc11x_blk_gate(clk, enable);

#ifdef CONFIG_PM_PWR_GATING
		if(clk->powerDomain)
		{
			s5p_power_gating(clk->powerDomain,  DOMAIN_LP_MODE);
		}
#endif
		/*Enable the src mask before enabling the gating*/
		if(clk->srcMaskReg)
		{
			mask = __raw_readl(clk->srcMaskReg);
			mask &= (~(clk->srcMaskBit));
			__raw_writel(mask, clk->srcMaskReg);
			//printk("\ndisable mask for %s\n",clk->name);
		}
	}
	return 0;
}

static struct clk *s5p_clks[] __initdata = {
	&clk_ext_xtal_mux,
	&clk_xrtcxti,
	&clk_48m,
	&s5p_clk_27m,
	&clk_fout_apll,
	&clk_fout_mpll,
	&clk_fout_epll,
	&clk_arm,
	&clk_vpll,
};

void __init s5p_register_clocks(unsigned long xtal_freq)
{
	int ret;

	clk_ext_xtal_mux.rate = xtal_freq;

	ret = s3c24xx_register_clocks(s5p_clks, ARRAY_SIZE(s5p_clks));
	if (ret > 0)
		printk(KERN_ERR "Failed to register s5p clocks\n");
}
