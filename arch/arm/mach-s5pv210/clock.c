/* linux/arch/arm/mach-s5pv210/clock.c
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * S5PV210 - Clock support
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

#include <mach/map.h>

#include <plat/cpu-freq.h>
#include <mach/regs-clock.h>
#include <mach/regs-power.h>
#include <plat/clock.h>
#include <plat/cpu.h>
#include <plat/pll.h>
#include <plat/s5p-clock.h>
#include <plat/clock-clksrc.h>
#include <plat/s5pv210.h>
#include <mach/regs-audss.h>

#define DBG(fmt...) 
//#define DBG(fmt...) printk(fmt)
#define CLK_DIV_CHANGE_BY_STEP 0
#define MAX_DVFS_LEVEL  7
extern unsigned int s5pc11x_cpufreq_index;

#if 0
/*APLL_FOUT, MPLL_FOUT, ARMCLK, HCLK_DSYS*/
static const u32 s5p_sysout_clk_tab_1GHZ[][4] = {
	// APLL:1000,ARMCLK:1000,HCLK_MSYS:200,MPLL:667,HCLK_DSYS:166,HCLK_PSYS:133,PCLK_MSYS:100,PCLK_DSYS:83,PCLK_PSYS:66
	{1000* MHZ, 667 *MHZ, 1000 *MHZ, 166 *MHZ},
	// APLL:800,ARMCLK:800,HCLK_MSYS:200,MPLL:667,HCLK_DSYS:166,HCLK_PSYS:133,PCLK_MSYS:100,PCLK_DSYS:83,PCLK_PSYS:66
	{800* MHZ, 667 *MHZ, 800 *MHZ, 166 *MHZ},
	// APLL:800,ARMCLK:400,HCLK_MSYS:200,MPLL:667,HCLK_DSYS:166,HCLK_PSYS:133,PCLK_MSYS:100,PCLK_DSYS:83,PCLK_PSYS:66
	{800* MHZ, 667 *MHZ, 400 *MHZ, 166 *MHZ},
	// APLL:800,ARMCLK:200,HCLK_MSYS:200,MPLL:667,HCLK_DSYS:166,HCLK_PSYS:133,PCLK_MSYS:100,PCLK_DSYS:83,PCLK_PSYS:66
	{800* MHZ, 667 *MHZ, 200 *MHZ, 166 *MHZ},
	// APLL:800,ARMCLK:100,HCLK_MSYS:100,MPLL:667,HCLK_DSYS:83,HCLK_PSYS:66,PCLK_MSYS:100,PCLK_DSYS:83,PCLK_PSYS:66
	{800* MHZ, 667 *MHZ, 100 *MHZ, 133 *MHZ},
};


#define DIV_TAB_MAX_FIELD	12

/*div0 ratio table*/
/*apll, a2m, HCLK_MSYS, PCLK_MSYS, HCLK_DSYS, PCLK_DSYS, HCLK_PSYS, PCLK_PSYS, MFC_DIV, G3D_DIV, MSYS source(2D, 3D, MFC)(0->apll,1->mpll), DMC0 div*/
static const u32 s5p_sys_clk_div0_tab_1GHZ[][DIV_TAB_MAX_FIELD] = {
        {0, 4, 4, 1, 3, 1, 4, 1, 3, 3, 0, 3},
        {0, 3, 3, 1, 3, 1, 4, 1, 3, 3, 0, 3},
        {1, 3, 1, 1, 3, 1, 4, 1, 3, 3, 0, 3},
        {3, 3, 0, 1, 3, 1, 4, 1, 3, 3, 0, 3},
        {7, 3, 0, 0, 7, 0, 9, 0, 3, 3, 1, 4},
};

/*pms value table*/
/*APLL(m, p, s), MPLL(m, p, s)*/
static const u32 s5p_sys_clk_mps_tab_1GHZ[][6] = {
        {125, 3, 1, 667, 12, 1},
        {100, 3, 1, 667, 12, 1},
        {100, 3, 1, 667, 12, 1},
        {100, 3, 1, 667, 12, 1},
        {100, 3, 1, 667, 12, 1},
};


/*APLL_FOUT, MPLL_FOUT, ARMCLK, HCLK_DSYS*/
static const u32 s5p_sysout_clk_tab_1DOT2GHZ[][4] = {
	// APLL:1200,ARMCLK:1200,HCLK_MSYS:200,MPLL:667,HCLK_DSYS:166,HCLK_PSYS:133,PCLK_MSYS:100,PCLK_DSYS:83,PCLK_PSYS:66
	{1200* MHZ, 667 *MHZ, 1200 *MHZ, 166 *MHZ},
	// APLL:1000,ARMCLK:1000,HCLK_MSYS:200,MPLL:667,HCLK_DSYS:166,HCLK_PSYS:133,PCLK_MSYS:100,PCLK_DSYS:83,PCLK_PSYS:66
	{1000* MHZ, 667 *MHZ, 1000 *MHZ, 166 *MHZ},
	// APLL:800,ARMCLK:800,HCLK_MSYS:200,MPLL:667,HCLK_DSYS:166,HCLK_PSYS:133,PCLK_MSYS:100,PCLK_DSYS:83,PCLK_PSYS:66
	{800* MHZ, 667 *MHZ, 800 *MHZ, 166 *MHZ},
	// APLL:800,ARMCLK:400,HCLK_MSYS:200,MPLL:667,HCLK_DSYS:166,HCLK_PSYS:133,PCLK_MSYS:100,PCLK_DSYS:83,PCLK_PSYS:66
	{800* MHZ, 667 *MHZ, 400 *MHZ, 166 *MHZ},
	// APLL:800,ARMCLK:200,HCLK_MSYS:200,MPLL:667,HCLK_DSYS:166,HCLK_PSYS:133,PCLK_MSYS:100,PCLK_DSYS:83,PCLK_PSYS:66
	{800* MHZ, 667 *MHZ, 200 *MHZ, 166 *MHZ},
	// APLL:800,ARMCLK:100,HCLK_MSYS:100,MPLL:667,HCLK_DSYS:83,HCLK_PSYS:66,PCLK_MSYS:100,PCLK_DSYS:83,PCLK_PSYS:66
	{800* MHZ, 667 *MHZ, 100 *MHZ, 133 *MHZ},
};


/*div0 ratio table*/
/*apll, a2m, HCLK_MSYS, PCLK_MSYS, HCLK_DSYS, PCLK_DSYS, HCLK_PSYS, PCLK_PSYS, MFC_DIV, G3D_DIV,MSYS source, DMC0 div*/
static const u32 s5p_sys_clk_div0_tab_1DOT2GHZ[][DIV_TAB_MAX_FIELD] = {
	{0, 5, 5, 1, 3, 1, 4, 1, 3, 3, 0, 3},
        {0, 4, 4, 1, 3, 1, 4, 1, 3, 3, 0, 3},
        {0, 3, 3, 1, 3, 1, 4, 1, 3, 3, 0, 3},
        {1, 3, 1, 1, 3, 1, 4, 1, 3, 3, 0, 3},
        {3, 3, 0, 1, 3, 1, 4, 1, 3, 3, 0, 3},
        {7, 3, 0, 0, 7, 0, 9, 0, 3, 3, 1, 4},
};

#endif

struct S5PC110_clk_info {
	u32	armclk;
	u32 	apllout;
	u32 	mpllout;
	u32	apll_mps;
	u32	mpll_mps;
	u32	msys_div0;
	u32	psys_dsys_div0;
	u32	div2val;
	u32	dmc0_div6;
};


struct S5PC110_clk_info clk_info[] = {
#if USE_1DOT2GHZ
{
	// APLL:1200,ARMCLK:1200,HCLK_MSYS:200,MPLL:667,HCLK_DSYS:166,HCLK_PSYS:133,PCLK_MSYS:100,PCLK_DSYS:83,PCLK_PSYS:66
	.armclk		=	1200* MHZ,
	.apllout	=	1200* MHZ,
	.apll_mps	=	((150<<16)|(3<<8)|1),
	.msys_div0	=	(0|(5<<4)|(5<<8)|(1<<12)),
	.mpllout	=	667* MHZ,
	.mpll_mps	=	((667<<16)|(12<<8)|(1)),
	.psys_dsys_div0 =	((3<<16)|(1<<20)|(4<<24)|(1<<28)),
	.div2val	=	((3<<0)|(3<<4)|(3<<8)),
	.dmc0_div6 	=	(3<<28),
},
#endif// A extra entry for 1200MHZ level 
{
	// APLL:1000,ARMCLK:1000,HCLK_MSYS:200,MPLL:667,HCLK_DSYS:166,HCLK_PSYS:133,PCLK_MSYS:100,PCLK_DSYS:83,PCLK_PSYS:66
	.armclk		=	1000* MHZ,
	.apllout	=	1000* MHZ,
	.apll_mps	=	((125<<16)|(3<<8)|1),
	.msys_div0	=	(0|(4<<4)|(4<<8)|(1<<12)),
	.mpllout	=	667* MHZ,
	.mpll_mps	=	((667<<16)|(12<<8)|(1)),
	.psys_dsys_div0 =	((3<<16)|(1<<20)|(4<<24)|(1<<28)),
	.div2val	=	((3<<0)|(3<<4)|(3<<8)),
	.dmc0_div6 	=	(3<<28),
},
{
	// APLL:800,ARMCLK:800,HCLK_MSYS:200,MPLL:667,HCLK_DSYS:166,HCLK_PSYS:133,PCLK_MSYS:100,PCLK_DSYS:83,PCLK_PSYS:66
	.armclk		=	800* MHZ,
	.apllout	=	800* MHZ,
	.apll_mps	=	((100<<16)|(3<<8)|1),
	.msys_div0	=	(0|(3<<4)|(3<<8)|(1<<12)),
	.mpllout	=	667* MHZ,
	.mpll_mps	=	((667<<16)|(12<<8)|(1)),
	.psys_dsys_div0 =	((3<<16)|(1<<20)|(4<<24)|(1<<28)),
	.div2val	=	((3<<0)|(3<<4)|(3<<8)),
	.dmc0_div6 	=	(3<<28),
},
{
	// APLL:800,ARMCLK:400,HCLK_MSYS:200,MPLL:667,HCLK_DSYS:166,HCLK_PSYS:133,PCLK_MSYS:100,PCLK_DSYS:83,PCLK_PSYS:66
	.armclk		=	400* MHZ,
	.apllout	=	800* MHZ,
	.apll_mps	=	((100<<16)|(3<<8)|1),
	.msys_div0	=	(1|(3<<4)|(1<<8)|(1<<12)),
	.mpllout	=	667* MHZ,
	.mpll_mps	=	((667<<16)|(12<<8)|(1)),
	.psys_dsys_div0 =	((3<<16)|(1<<20)|(4<<24)|(1<<28)),
	.div2val	=	((3<<0)|(3<<4)|(3<<8)),
	.dmc0_div6 	=	(3<<28),
},
{
	// APLL:800,ARMCLK:200,HCLK_MSYS:200,MPLL:667,HCLK_DSYS:166,HCLK_PSYS:133,PCLK_MSYS:100,PCLK_DSYS:83,PCLK_PSYS:66
	.armclk		=	200* MHZ,
	.apllout	=	800* MHZ,
	.apll_mps	=	((100<<16)|(3<<8)|1),
	.msys_div0	=	(3|(3<<4)|(0<<8)|(1<<12)),
	.mpllout	=	667* MHZ,
	.mpll_mps	=	((667<<16)|(12<<8)|(1)),
	.psys_dsys_div0 =	((3<<16)|(1<<20)|(4<<24)|(1<<28)),
	.div2val	=	((3<<0)|(3<<4)|(3<<8)),
	.dmc0_div6 	=	(3<<28),
},
{
	// APLL:800,ARMCLK:100,HCLK_MSYS:100,MPLL:667,HCLK_DSYS:83,HCLK_PSYS:66,PCLK_MSYS:100,PCLK_DSYS:83,PCLK_PSYS:66
	.armclk		=	100* MHZ,
	.apllout	=	800* MHZ,
	.apll_mps	=	((100<<16)|(3<<8)|1),
	.msys_div0	=	(7|(3<<4)|(0<<8)|(0<<12)),
	.mpllout	=	667* MHZ,
	.mpll_mps	=	((667<<16)|(12<<8)|(1)),
	.psys_dsys_div0 =	((7<<16)|(0<<20)|(9<<24)|(0<<28)),
	.div2val	=	((3<<0)|(3<<4)|(3<<8)),
	.dmc0_div6 	=	(7<<28),
}
};

#if 0
/*pms value table*/
/*APLL(m, p, s), MPLL(m, p, s)*/
static const u32 s5p_sys_clk_mps_tab_1DOT2GHZ[][6] = {
        {150, 3, 1, 667, 12, 1},
        {125, 3, 1, 667, 12, 1},
        {100, 3, 1, 667, 12, 1},
        {100, 3, 1, 667, 12, 1},
        {100, 3, 1, 667, 12, 1},
        {100, 3, 1, 667, 12, 1},
};


static const u32 (*s5p_sysout_clk_tab_all[2])[4] = {
        s5p_sysout_clk_tab_1GHZ,
        s5p_sysout_clk_tab_1DOT2GHZ,
};

static const u32 (*s5p_sys_clk_div0_tab_all[2])[DIV_TAB_MAX_FIELD] = {
        s5p_sys_clk_div0_tab_1GHZ,
        s5p_sys_clk_div0_tab_1DOT2GHZ,
};

static const u32 (*s5p_sys_clk_mps_tab_all[2])[6] = {
        s5p_sys_clk_mps_tab_1GHZ,
        s5p_sys_clk_mps_tab_1DOT2GHZ,
};

#endif

#ifdef CONFIG_PM_PWR_GATING
extern int s5p_power_gating(unsigned int power_domain, unsigned int on_off);
extern int s5p_domain_off_check(unsigned int power_domain);
int tvblk_turnon;
#endif
struct clkGateBlock gb_img = { 
	.gb_ctrlbit	= S5P_CLKGATE_BLOCK_IMG,
	.gb_countIP	= 1,
	.gb_CheckIPBits1 = S5P_CLKGATE_IP0_FIMC0|S5P_CLKGATE_IP0_FIMC1|\
			S5P_CLKGATE_IP0_FIMC2|S5P_CLKGATE_IP0_JPEG| \
			S5P_CLKGATE_IP0_CSIS|S5P_CLKGATE_IP0_ROTATOR,
	.gb_CheckIPReg1 = S5P_CLKGATE_IP0,
	.gb_CheckIPBits2 = 0,
	.gb_CheckIPReg2 = NULL,
};

struct clkGateBlock gb_mfc = { 
	.gb_ctrlbit	= S5P_CLKGATE_BLOCK_MFC,
	.gb_countIP	= 1,
	.gb_CheckIPBits1 = S5P_CLKGATE_IP0_MFC, 
	.gb_CheckIPReg1 = S5P_CLKGATE_IP0,
	.gb_CheckIPBits2 = 0,
	.gb_CheckIPReg2 = NULL,
};

struct clkGateBlock gb_g3d = { 
	.gb_ctrlbit	= S5P_CLKGATE_BLOCK_G3D,
	.gb_countIP	= 1,
	.gb_CheckIPBits1 = S5P_CLKGATE_IP0_G3D, 
	.gb_CheckIPReg1 = S5P_CLKGATE_IP0,
	.gb_CheckIPBits2 = 0,
	.gb_CheckIPReg2 = NULL,
};

struct clkGateBlock gb_usb = { 
	.gb_ctrlbit	= S5P_CLKGATE_BLOCK_USB,
	.gb_countIP	= 1,
	.gb_CheckIPBits1 = S5P_CLKGATE_IP1_USBHOST|S5P_CLKGATE_IP1_USBOTG, 
	.gb_CheckIPReg1 = S5P_CLKGATE_IP1,
	.gb_CheckIPBits2 = 0,
	.gb_CheckIPReg2 = NULL,
};

struct clkGateBlock gb_tv = { 
	.gb_ctrlbit	= S5P_CLKGATE_BLOCK_TV,
	.gb_countIP	= 1,
	.gb_CheckIPBits1 = S5P_CLKGATE_IP1_VP|S5P_CLKGATE_IP1_MIXER| \
			S5P_CLKGATE_IP1_TVENC|S5P_CLKGATE_IP1_HDMI, 
	.gb_CheckIPReg1 = S5P_CLKGATE_IP1,
	.gb_CheckIPBits2 = 0,
	.gb_CheckIPReg2 = NULL,
};

struct clkGateBlock gb_lcd = { 
	.gb_ctrlbit	= S5P_CLKGATE_BLOCK_LCD,
	.gb_countIP	= 2,
	.gb_CheckIPBits1 = S5P_CLKGATE_IP1_FIMD|S5P_CLKGATE_IP1_DSIM,
	.gb_CheckIPReg1 = S5P_CLKGATE_IP1,
	.gb_CheckIPBits2 = S5P_CLKGATE_IP0_G2D,
	.gb_CheckIPReg2 = S5P_CLKGATE_IP0,
};

struct clkGateBlock gb_mmc = { 
	.gb_ctrlbit	= S5P_CLKGATE_BLOCK_HSMMC,
	.gb_countIP	= 1,
	.gb_CheckIPBits1 = S5P_CLKGATE_IP2_HSMMC0|S5P_CLKGATE_IP2_HSMMC1| \
			S5P_CLKGATE_IP2_HSMMC2|S5P_CLKGATE_IP2_HSMMC3| \
			S5P_CLKGATE_IP2_TSI, 
	.gb_CheckIPReg1 = S5P_CLKGATE_IP2,
	.gb_CheckIPBits2 = 0,
	.gb_CheckIPReg2 = NULL,
};

struct clkGateBlock gb_debug = { 
	.gb_ctrlbit	= S5P_CLKGATE_BLOCK_DEBUG,
	.gb_countIP	= 1,
	.gb_CheckIPBits1 = S5P_CLKGATE_IP2_SECJTAG|S5P_CLKGATE_IP2_HOSTIF| \
			S5P_CLKGATE_IP2_MODEM|S5P_CLKGATE_IP2_CORESIGHT, 
	.gb_CheckIPReg1 = S5P_CLKGATE_IP2,
	.gb_CheckIPBits2 = 0,
	.gb_CheckIPReg2 = NULL,
};

struct clkGateBlock gb_security = { 
	.gb_ctrlbit	= S5P_CLKGATE_BLOCK_SECURITY,
	.gb_countIP	= 1,
	.gb_CheckIPBits1 = S5P_CLKGATE_IP2_SECSS,
	.gb_CheckIPReg1 = S5P_CLKGATE_IP2,
	.gb_CheckIPBits2 = 0,
	.gb_CheckIPReg2 = NULL,
};

struct clkGateBlock gb_memory = { 
	.gb_ctrlbit	= S5P_CLKGATE_BLOCK_MEMORY,
	.gb_countIP	= 1,
	.gb_CheckIPBits1 = S5P_CLKGATE_IP1_NFCON|S5P_CLKGATE_IP1_SROMC| \
			S5P_CLKGATE_IP1_CFCON|S5P_CLKGATE_IP1_NANDXL,
	.gb_CheckIPReg1 = S5P_CLKGATE_IP1,
	.gb_CheckIPBits2 = 0,
	.gb_CheckIPReg2 = NULL,
};

int s5pc11x_blk_gate(struct clk *clk, int enable)
{
	unsigned int regval = 0;
	unsigned int isSetOff = 0;
	int countIPSet = clk->gb->gb_countIP;
	if(!clk->gb)	
		return -1;
	if(countIPSet)
	{
		regval = readl(clk->gb->gb_CheckIPReg1);
		regval &= (clk->gb->gb_CheckIPBits1);
		isSetOff |= regval;
	}
	countIPSet--;
	if(countIPSet) // check for next set
	{
		regval = readl(clk->gb->gb_CheckIPReg2);
		regval &= (clk->gb->gb_CheckIPBits2);
		isSetOff |= regval;
	}
	if(isSetOff == 0)
	{
		/*Mask/Unmask the clock gate block*/
		regval = readl(S5P_CLKGATE_BLOCK);
		regval &= (~(clk->gb->gb_ctrlbit));
		if(enable)
			regval |= clk->gb->gb_ctrlbit;
		else
			regval &= (~(clk->gb->gb_ctrlbit));
		writel(regval, S5P_CLKGATE_BLOCK);
		//printk("\n+++++ %s called for %s enable=%d S5P_CLKGATE_BLOCK=%x\n",__func__,clk->name,enable,regval);
	}

	return 0;
}

static int s5pv210_clk_ip0_ctrl(struct clk *clk, int enable)
{
	return s5p_gatectrl(S5P_CLKGATE_IP0, clk, enable);
}

static int s5pv210_clk_ip1_ctrl(struct clk *clk, int enable)
{
	return s5p_gatectrl(S5P_CLKGATE_IP1, clk, enable);
}

static int s5pv210_clk_ip2_ctrl(struct clk *clk, int enable)
{
	return s5p_gatectrl(S5P_CLKGATE_IP2, clk, enable);
}

static int s5pv210_clk_ip3_ctrl(struct clk *clk, int enable)
{
	return s5p_gatectrl(S5P_CLKGATE_IP3, clk, enable);
}

static int s5pv210_clk_ip4_ctrl(struct clk *clk, int enable)
{
	return s5p_gatectrl(S5P_CLKGATE_IP4, clk, enable);
}

#if defined(CONFIG_CPU_S5PV210_EVT1)
static int s5pv210_clk_ip5_ctrl(struct clk *clk, int enable)
{
	return s5p_gatectrl(S5P_CLKGATE_IP5, clk, enable);
}
#endif

static int s5pv210_clk_block_ctrl(struct clk *clk, int enable)
{
	return s5p_gatectrl(S5P_CLKGATE_BLOCK, clk, enable);
}

static int s5pv210_clk_mask0_ctrl(struct clk *clk, int enable)
{
	return s5p_gatectrl(S5P_CLK_SRC_MASK0, clk, enable);
}

static int s5pv210_clk_mask1_ctrl(struct clk *clk, int enable)
{
	return s5p_gatectrl(S5P_CLK_SRC_MASK1, clk, enable);
}

static int s5pv210_clk_audss_ctrl(struct clk *clk, int enable)
{
	return s5p_gatectrl(S5P_CLKGATE_AUDSS, clk, enable);
}


static unsigned long s5pc11x_armclk_get_rate(struct clk *clk)
{
  	unsigned long rate = clk_get_rate(clk->parent);

	rate /= (((__raw_readl(S5P_CLK_DIV0) & S5P_CLKDIV0_APLL_MASK) >> S5P_CLKDIV0_APLL_SHIFT) + 1);

	return rate;
}

extern unsigned int dvfs_change_direction;
extern unsigned int prevIndex;
extern unsigned int S5PC11X_FREQ_TAB;
extern unsigned int dvfs_change_direction;
static u32 s5p_cpu_clk_tab_size(void)
{
	return ARRAY_SIZE(clk_info);
}

static int s5pc11x_clk_set_withapllchange(unsigned int target_freq,
                                unsigned int index )
{
	u32 val, reg;
	unsigned int mask;	

	/*change the apll*/
	
	//////////////////////////////////////////////////
	/* APLL should be changed in this level
	 * APLL -> MPLL(for stable transition) -> APLL
	 * Some clock source's clock API  are not prepared. Do not use clock API
	 * in below code.
	 */

	if (dvfs_change_direction == 0) {
		__raw_writel(0x40e, S5P_VA_DMC1 + 0x30);
	}

	reg = __raw_readl(S5P_CLK_DIV2);
	DBG("before apll transition DIV2=%x\n",reg);
	reg &= ~(S5P_CLKDIV2_G3D_MASK | S5P_CLKDIV2_MFC_MASK);
	reg |= clk_info[index].div2val ;
	__raw_writel(reg, S5P_CLK_DIV2);	
	DBG("during apll transition DIV2=%x\n",reg);
	do {
		reg = __raw_readl(S5P_CLK_DIV_STAT0);
	} while (reg & ((1<<16)|(1<<17)));

	/* Change APLL to MPLL in MFC_MUX and G3D MUX*/	
	reg = __raw_readl(S5P_CLK_SRC2);
	DBG("before apll transition SRC2=%x\n",reg);
	reg &= ~(S5P_CLKSRC2_G3D_MASK | S5P_CLKSRC2_MFC_MASK);
	reg |= (1<<S5P_CLKSRC2_G3D_SHIFT) | (1<<S5P_CLKSRC2_MFC_SHIFT);
	__raw_writel(reg, S5P_CLK_SRC2);
	DBG("during apll transition SRC2=%x\n",reg);	
	do {
		reg = __raw_readl(S5P_CLK_MUX_STAT1);
	} while (reg & ((1<<7)|(1<<3)));

	if (dvfs_change_direction == 1) {
		__raw_writel(0x40e, S5P_VA_DMC1 + 0x30);
	}

	/* HCLKMSYS change: SCLKAPLL -> SCLKMPLL */
	reg = __raw_readl(S5P_CLK_SRC0);
	DBG("before apll transition SRC0=%x\n",reg);	
	reg &= ~(S5P_CLKSRC0_MUX200_MASK);
	reg |= (0x1 << S5P_CLKSRC0_MUX200_SHIFT);
	__raw_writel(reg, S5P_CLK_SRC0);
	DBG("durint apll transition SRC0=%x\n",reg);
	do {
		reg = __raw_readl(S5P_CLK_MUX_STAT0);
	} while (reg & (0x1<<18));	
		
	//////////////////////////////////////////
		
	/*set apll divider value*/	
	val = __raw_readl(S5P_CLK_DIV0);
	val = val & (~(0xffff));
	val = val | clk_info[index].msys_div0;
	__raw_writel(val, S5P_CLK_DIV0);

	mask = S5P_CLK_DIV_STAT0_DIV_APLL | S5P_CLK_DIV_STAT0_DIV_A2M | S5P_CLK_DIV_STAT0_DIV_HCLK_MSYS | S5P_CLK_DIV_STAT0_DIV_PCLK_MSYS;
	do {
		val = __raw_readl(S5P_CLK_DIV_STAT0) & mask;
	} while (val);
	DBG("\n DIV0 = %x\n",__raw_readl(S5P_CLK_DIV0));

	/*set apll_con M P S value*/
	val = (0 << 31) | clk_info[index].apll_mps;

	__raw_writel(val, S5P_APLL_CON);
	__raw_writel(val | (1 << 31), S5P_APLL_CON);
	while(!(__raw_readl(S5P_APLL_CON) & (1 << 29)));

	////////////////////////////////////
	
	/*  HCLKMSYS change: SCLKMPLL -> SCLKAPLL */
	reg = __raw_readl(S5P_CLK_SRC0);
	reg &= ~(S5P_CLKSRC0_MUX200_MASK);
	reg |= (0x0 << S5P_CLKSRC0_MUX200_SHIFT);
	__raw_writel(reg, S5P_CLK_SRC0);
	DBG("after apll transition SRC0=%x\n",reg);
	do {
		reg = __raw_readl(S5P_CLK_MUX_STAT0);
	} while (reg & (0x1<<18));

	if (dvfs_change_direction == 1) {
		__raw_writel(0x618, S5P_VA_DMC1 + 0x30);
	}

	/* Change MPLL to APLL in MFC_MUX and G3D MUX */
	reg = __raw_readl(S5P_CLK_SRC2);
	reg &= ~(S5P_CLKSRC2_G3D_MASK | S5P_CLKSRC2_MFC_MASK);
	reg |= (0<<S5P_CLKSRC2_G3D_SHIFT) | (0<<S5P_CLKSRC2_MFC_SHIFT);
	__raw_writel(reg, S5P_CLK_SRC2);
	DBG("after apll transition SRC2=%x\n",reg);
	do {
		reg = __raw_readl(S5P_CLK_MUX_STAT1);
	} while (reg & ((1<<7)|(1<<3)));

	reg = __raw_readl(S5P_CLK_DIV2);
	reg &= ~(S5P_CLKDIV2_G3D_MASK | S5P_CLKDIV2_MFC_MASK);
	reg |= (0x0<<S5P_CLKDIV2_G3D_SHIFT)|(0x0<<S5P_CLKDIV2_MFC_SHIFT);
	__raw_writel(reg, S5P_CLK_DIV2);
	DBG("after apll transition DIV2=%x\n",reg);
	do {
		reg = __raw_readl(S5P_CLK_DIV_STAT0);
	} while (reg & ((1<<16)|(1<<17)));

	if (dvfs_change_direction == 0) {
		__raw_writel(0x618, S5P_VA_DMC1 + 0x30);
	}

	////////////////////////////////////

	clk_fout_apll.rate = target_freq ;
	DBG("S5P_APLL_CON = %x, S5P_CLK_DIV0=%x\n",__raw_readl(S5P_APLL_CON),__raw_readl(S5P_CLK_DIV0));

	return 0;
}

int s5pc11x_armclk_set_rate(struct clk *clk, unsigned long rate)
{
	int cur_freq;
	unsigned int mask;
	u32 val;
	u32 size;
        int index;
        index = s5pc11x_cpufreq_index;

	size = s5p_cpu_clk_tab_size();

	if(index >= size)
	{
		printk("=DVFS ERR index(%d) > size(%d)\n", index, size);
		return 1;
	}

	/* validate target frequency */ 
	if(clk_info[index].armclk != rate)
	{
		DBG("=DVFS ERR target_freq (%d) != cpu_tab_freq (%d)\n", clk_info[index].armclk, rate);
		return 0;
	}

	cur_freq = (int)s5pc11x_armclk_get_rate(clk);

	/*check if change in DMC0 divider*/
	if(clk_info[prevIndex].dmc0_div6 != clk_info[index].dmc0_div6)
	{

		if(clk_info[index].dmc0_div6 == (3<<28)) { // for 200mhz/166mhz
			__raw_writel(0x618, S5P_VA_DMC1 + 0x30);
			__raw_writel(0x50e, S5P_VA_DMC0 + 0x30);
		} else {					// for 100mhz/83mhz
			__raw_writel(0x30c, S5P_VA_DMC1 + 0x30);
			__raw_writel(0x287, S5P_VA_DMC0 + 0x30);
		}

		val = __raw_readl(S5P_CLK_DIV6);
		val &= ~(S5P_CLKDIV6_ONEDRAM_MASK);
		val |= (clk_info[index].dmc0_div6);
		__raw_writel(val, S5P_CLK_DIV6);
		do {
			val = __raw_readl(S5P_CLK_DIV_STAT1);
		} while (val & ((1<<15)));
	}

	/* check if change in apll */
	if(clk_info[prevIndex].apllout != clk_info[index].apllout)
	{
		DBG("changing apll\n");
		s5pc11x_clk_set_withapllchange(rate, index);
		return 0;	
	}
	DBG("apll target frequency = %d, index=%d\n",clk_info[index].apllout,index);

	/*return if current frequency is same as target frequency*/
	if(cur_freq == clk_info[index].armclk)
		return 0;

	// change clock divider
	mask = (~(S5P_CLKDIV0_APLL_MASK)) & (~(S5P_CLKDIV0_A2M_MASK)) & 
			(~(S5P_CLKDIV0_HCLK200_MASK)) & (~(S5P_CLKDIV0_PCLK100_MASK));
	val = __raw_readl(S5P_CLK_DIV0) & mask;
	val |= clk_info[index].msys_div0;
	
	__raw_writel(val, S5P_CLK_DIV0);

	mask = S5P_CLK_DIV_STAT0_DIV_APLL | S5P_CLK_DIV_STAT0_DIV_A2M | S5P_CLK_DIV_STAT0_DIV_HCLK_MSYS | S5P_CLK_DIV_STAT0_DIV_PCLK_MSYS;
	do {
		val = __raw_readl(S5P_CLK_DIV_STAT0) & mask;
	} while (val);
	DBG("\n DIV0 = %x\n",__raw_readl(S5P_CLK_DIV0));

       return 0;
}


/*Change MPll rate*/
int s5pc11x_clk_set_mpll(unsigned int index )
{
	int cur_freq;
	u32 val, mask;
	u32 size;
	u32 mpll_target_freq = 0;
	u32 vsel = 0;
	struct clk *xtal_clk;
        u32 xtal;

	printk("mpll changed.\n");
	size = s5p_cpu_clk_tab_size();

	if(index >= size)
	{
		printk("=DVFS ERR index(%d) > size(%d)\n", index, size);
		return 1;
	}


	mpll_target_freq = clk_info[index].armclk;
	
	cur_freq = clk_fout_mpll.rate;

	DBG("Current mpll frequency = %d\n",cur_freq);
	DBG("target mpll frequency = %d\n",mpll_target_freq);


	/* current frquency is same as target frequency */
	if(cur_freq == mpll_target_freq)
	{
		return 0;
	}

	/* Set mpll_out=fin */ //workaround for evt0
	val = __raw_readl(S5P_CLK_SRC0);
	val = val & (~S5P_CLKSRC0_MPLL_MASK);
	__raw_writel(val, S5P_CLK_SRC0);
	
	/*stop mpll*/
	val  = __raw_readl(S5P_MPLL_CON);
        val = val & (~(0x1 << 31));
        __raw_writel(val, S5P_MPLL_CON);


	/*set mpll divider value*/	
	val = __raw_readl(S5P_CLK_DIV0);
	val = val & (~(0xffff<<16));
	val = val | clk_info[index].psys_dsys_div0;

	__raw_writel(val, S5P_CLK_DIV0);

	/*set mpll_con*/
	xtal_clk = clk_get(NULL, "xtal");
        BUG_ON(IS_ERR(xtal_clk));

        xtal = clk_get_rate(xtal_clk);
        clk_put(xtal_clk);
	DBG("xtal = %d\n",xtal);


	vsel = 0;

	val = (0 << 31) | (0 << 28) | (vsel << 27) | clk_info[index].mpll_mps; 

	__raw_writel(val, S5P_MPLL_CON);
	__raw_writel(val | (1 << 31), S5P_MPLL_CON);
	while(!(__raw_readl(S5P_MPLL_CON) & (1 << 29)));

	/* Set mpll_out=fout */ //workaround for evt0
	val = __raw_readl(S5P_CLK_SRC0);
	val = val | (0x1 << S5P_CLKSRC0_MPLL_SHIFT);
	__raw_writel(val, S5P_CLK_SRC0);


	clk_fout_mpll.rate = mpll_target_freq ;

	mask = S5P_CLK_DIV_STAT0_DIV_PCLK_PSYS | S5P_CLK_DIV_STAT0_DIV_HCLK_PSYS |  S5P_CLK_DIV_STAT0_DIV_PCLK_DSYS | S5P_CLK_DIV_STAT0_DIV_HCLK_DSYS;
	do {
		val = __raw_readl(S5P_CLK_DIV_STAT0) & mask;
	} while (val);
	
	DBG("S5P_MPLL_CON = %x, S5P_CLK_DIV0=%x, S5P_CLK_SRC0=%x, \n",__raw_readl(S5P_MPLL_CON),__raw_readl(S5P_CLK_DIV0),__raw_readl(S5P_CLK_SRC0));

	return 0;
}

int s5pc11x_clk_dsys_psys_change(int index) {
	unsigned int val, mask;
	u32 size;

	size = s5p_cpu_clk_tab_size();

	if(index >= size)
	{
		printk("index(%d) > size(%d)\n", index, size);
		return 1;
	}

	/* check if change in mpll */
        if(clk_info[prevIndex].mpllout != clk_info[index].mpllout)
        {
                DBG("changing mpll\n");
                s5pc11x_clk_set_mpll(index);
                return 0;
        }


	val = __raw_readl(S5P_CLK_DIV0);

	if ((val & 0xFFFF0000) == clk_info[index].psys_dsys_div0) {
		DBG("No change in psys, dsys domain\n");
		return 0;
	} else {
		mask = (~(S5P_CLKDIV0_HCLK166_MASK)) & (~(S5P_CLKDIV0_HCLK133_MASK)) & 
			(~(S5P_CLKDIV0_PCLK83_MASK)) & (~(S5P_CLKDIV0_PCLK66_MASK)) ;
		val = val & mask;
		val |= clk_info[index].psys_dsys_div0;
		__raw_writel(val, S5P_CLK_DIV0);


		DBG("DSYS/PSYS DIV0 = %x\n",__raw_readl(S5P_CLK_DIV0));
		//udelay(30);
		mask = S5P_CLK_DIV_STAT0_DIV_PCLK_PSYS | S5P_CLK_DIV_STAT0_DIV_HCLK_PSYS | S5P_CLK_DIV_STAT0_DIV_PCLK_DSYS | S5P_CLK_DIV_STAT0_DIV_HCLK_DSYS;
		do {
			val = __raw_readl(S5P_CLK_DIV_STAT0) & mask;
		} while (val);

		return 0;
	}	
}


/* MOUT APLL */
static struct clksrc_clk clk_mout_apll = {
	.clk	= {
		.name		= "mout_apll",
		.id		= -1,
	},
	.sources	= &clk_src_apll,
	.reg_src	= { .reg = S5P_CLK_SRC0, .shift = 0, .size = 1 },
};

/* MOUT MPLL */
static struct clksrc_clk clk_mout_mpll = {
	.clk = {
		.name		= "mout_mpll",
		.id		= -1,
	},
	.sources	= &clk_src_mpll,
	.reg_src	= { .reg = S5P_CLK_SRC0, .shift = 4, .size = 1 },
};

/* MOUT EPLL */
static struct clksrc_clk clk_mout_epll = {
	.clk	= {
		.name		= "mout_epll",
		.id		= -1,
	},
	.sources	= &clk_src_epll,
	.reg_src	= { .reg = S5P_CLK_SRC0, .shift = 8, .size = 1 },
};


/* SCLK HDMI27M */
static struct clk clk_sclk_hdmi27m = {
	.name		= "sclk_hdmi27m",
	.id		= -1,
};

/* VPLL Source Clock */
static struct clk *clkset_vpllsrc_list[] = {
	[0] = &clk_fin_epll,
	[1] = &clk_sclk_hdmi27m,
};

static struct clksrc_sources clkset_vpllsrc = {
	.sources	= clkset_vpllsrc_list,
	.nr_sources	= ARRAY_SIZE(clkset_vpllsrc_list),
};

static struct clksrc_clk clk_vpllsrc = {
	.clk		= {
		.name		= "vpll_src",
		.id		= -1,
		.enable		= s5pv210_clk_mask0_ctrl,
		.ctrlbit	= (1 << 7),
	},
	.sources	= &clkset_vpllsrc,
	.reg_src	= { .reg = S5P_CLK_SRC1, .shift = 28, .size = 1 },
};

/* MOUT VPLL */
static struct clk *clkset_mout_vpll_list[] = {
	[0] = &clk_vpllsrc.clk,
	[1] = &clk_fout_epll,
};

static struct clksrc_sources clkset_mout_vpll = {
	.sources	= clkset_mout_vpll_list,
	.nr_sources	= ARRAY_SIZE(clkset_vpllsrc_list),
};

static struct clksrc_clk clk_mout_vpll = {
	.clk		= {
		.name		= "mout_vpll",
		.id		= -1,
	},
	.sources	= &clkset_mout_vpll,
	.reg_src	= { .reg = S5P_CLK_SRC0, .shift = 12, .size = 1 },
};

/* SCLK A2M */
static struct clksrc_clk clk_sclk_a2m = {
	.clk = {
		.name		= "sclk_a2m",
		.id		= -1,
		.parent		= &clk_mout_apll.clk,
	},
	.reg_div = { .reg = S5P_CLK_DIV0, .shift = 4, .size = 3 },
};

/* ARMCLK */
static struct clk *clkset_armclk_list[] = {
	[0] = &clk_mout_apll.clk,
	[1] = &clk_mout_mpll.clk,
};

static struct clksrc_sources clkset_armclk = {
	.sources	= clkset_armclk_list,
	.nr_sources	= ARRAY_SIZE(clkset_armclk_list),
};

static struct clksrc_clk clk_armclk = {
	.clk	= {
		.name		= "armclk",
		.id		= -1,
		.get_rate = s5pc11x_armclk_get_rate,
		.set_rate = s5pc11x_armclk_set_rate,
	},
	.sources	= &clkset_armclk,
	.reg_src	= { .reg = S5P_CLK_SRC0, .shift = 16, .size = 1 },
	.reg_div	= { .reg = S5P_CLK_DIV0, .shift = 0, .size = 3 },
};

/* HCLK200 */
static struct clksrc_clk clk_hclk_200 = {
	.clk	= {
		.name		= "hclk_200",
		.id		= -1,
		.parent		= &clk_armclk.clk,
	},
	.reg_div	= { .reg = S5P_CLK_DIV0, .shift = 8, .size = 3 },
};

/* PCLK100 */
static struct clksrc_clk clk_pclk_100 = {
	.clk	= {
		.name		= "pclk_100",
		.id		= -1,
		.parent		= &clk_hclk_200.clk,
	},
	.reg_div	= { .reg = S5P_CLK_DIV0, .shift = 12, .size = 3 },
};

/* MOUT166 */
static struct clk *clkset_mout_166_list[] = {
	[0] = &clk_mout_mpll.clk,
	[1] = &clk_sclk_a2m.clk,
};

static struct clksrc_sources clkset_mout_166 = {
	.sources	= clkset_mout_166_list,
	.nr_sources	= ARRAY_SIZE(clkset_mout_166_list),
};

static struct clksrc_clk clk_mout_166 = {
	.clk	= {
		.name		= "mout_166",
		.id		= -1,
	},
	.sources	= &clkset_mout_166,
	.reg_src	= { .reg = S5P_CLK_SRC0, .shift = 20, .size = 1 },
};

/* HCLK166 */
static struct clksrc_clk clk_hclk_166 = {
	.clk	= {
		.name		= "hclk_166",
		.id		= -1,
		.parent		= &clk_mout_166.clk,
	},
	.reg_div	= { .reg = S5P_CLK_DIV0, .shift = 16, .size = 4 },
};


/* PCLK83 */
static struct clksrc_clk clk_pclk_83 = {
	.clk	= {
		.name		= "pclk_83",
		.id		= -1,
		.parent		= &clk_hclk_166.clk,
	},
	.reg_div = { .reg = S5P_CLK_DIV0, .shift = 20, .size = 3 },
};

/* HCLK133 */
static struct clksrc_clk clk_hclk_133 = {
	.clk	= {
		.name		= "hclk_133",
		.id		= -1,
	},
	.sources	= &clkset_mout_166,
	.reg_src	= { .reg = S5P_CLK_SRC0, .shift = 24, .size = 1 },
	.reg_div	= { .reg = S5P_CLK_DIV0, .shift = 24, .size = 4 },
};

/* PCLK66 */
static struct clksrc_clk clk_pclk_66 = {
	.clk	= {
		.name		= "pclk_66",
		.id		= -1,
		.parent		= &clk_hclk_133.clk,
	},
	.reg_div	= { .reg = S5P_CLK_DIV0, .shift = 28, .size = 3 },
};


/* SCLK ONENAND */
static struct clk *clkset_sclk_onenand_list[] = {
	[0] = &clk_hclk_133.clk,
	[1] = &clk_mout_166.clk,
};

static struct clksrc_sources clkset_sclk_onenand = {
	.sources	= clkset_sclk_onenand_list,
	.nr_sources	= ARRAY_SIZE(clkset_sclk_onenand_list),
};

/* SCLK USBPHY 0 */
static struct clk clk_sclk_usbphy0 = {
	.name		= "sclk_usbphy",
	.id		= 0,
};

/* SCLK USBPHY 1 */
static struct clk clk_sclk_usbphy1 = {
	.name		= "sclk_usbphy",
	.id		= 1,
};

/* SCLK HDMIPHY */
static struct clk clk_sclk_hdmiphy = {
	.name		= "sclk_hdmiphy",
	.id		= -1,
};

/* PCMCDCLK0 */
static struct clk clk_pcmcdclk0 = {
	.name		= "pcmcdclk",
	.id		= 0,
};

/* PCMCDCLK1 */
static struct clk clk_pcmcdclk1 = {
	.name		= "pcmcdclk",
	.id		= 1,
};
/* I2SCDCLK0 */
static struct clk clk_i2scdclk0 = {
	.name		= "i2scdclk",
	.id		= 0,
};

/* SCLK DAC */
static struct clk *clkset_sclk_dac_list[] = {
	[0] = &clk_mout_vpll.clk,
	[1] = &clk_sclk_hdmiphy,
};

static struct clksrc_sources clkset_sclk_dac = {
	.sources	= clkset_sclk_dac_list,
	.nr_sources	= ARRAY_SIZE(clkset_sclk_dac_list),
};

static struct clksrc_clk clk_sclk_dac = {
	.clk		= {
		.name		= "sclk_dac",
		.id		= -1,
		.ctrlbit	= (1 << 10),
		.enable		= s5pv210_clk_ip1_ctrl,
	},
	.sources	= &clkset_sclk_dac,
	.reg_src	= { .reg = S5P_CLK_SRC1, .shift = 8, .size = 1 },
};

/* SCLK PIXEL */
static struct clksrc_clk clk_sclk_pixel = {
	.clk		= {
		.name		= "sclk_pixel",
		.id		= -1,
		.parent		= &clk_mout_vpll.clk,
	},
	.reg_div	= { .reg = S5P_CLK_DIV1, .shift = 0, .size = 4},
};

/* SCLK HDMI */
static struct clk *clkset_sclk_hdmi_list[] = {
	[0] = &clk_sclk_pixel.clk,
	[1] = &clk_sclk_hdmiphy,
};

static struct clksrc_sources clkset_sclk_hdmi = {
	.sources	= clkset_sclk_hdmi_list,
	.nr_sources	= ARRAY_SIZE(clkset_sclk_hdmi_list),
};

static struct clksrc_clk clk_sclk_hdmi = {
	.clk		= {
		.name		= "sclk_hdmi",
		.id		= -1,
		.enable		= s5pv210_clk_ip1_ctrl,
		.ctrlbit	= (1 << 11),
	},
	.sources	= &clkset_sclk_hdmi,
	.reg_src	= { .reg = S5P_CLK_SRC1, .shift = 0, .size = 1 },
};

/* SCLK MIXER */
static struct clk *clkset_sclk_mixer_list[] = {
	[0] = &clk_sclk_dac.clk,
	[1] = &clk_sclk_hdmi.clk,
};

static struct clksrc_sources clkset_sclk_mixer = {
	.sources	= clkset_sclk_mixer_list,
	.nr_sources	= ARRAY_SIZE(clkset_sclk_mixer_list),
};

static int s5pc11x_clk_out_set_rate(struct clk *clk, unsigned long rate)
{
        u32 val = 0, div = 0, rate_div = 1;
        int err = -EINVAL;
        if(rate && clk->parent)
        {
                if(clk->parent == &clk_fout_apll)
                        rate_div = 4;
                if(clk->parent == &clk_fout_mpll)
                        rate_div = 2;

                div = clk_get_rate(clk->parent) / rate/ rate_div;
                val = __raw_readl(S5P_CLK_OUT);
                val &= (~(0xF << 20));
                val |= (div - 1) << 20;
                __raw_writel(val, S5P_CLK_OUT);
                err = 0;
        }
        return err;
}

static int s5pc11x_clk_out_set_parent(struct clk *clk, struct clk *parent)
{
        u32 val = 0;
        int err = 0;
        clk->parent = parent;
        val = __raw_readl(S5P_CLK_OUT);

        if(parent == &clk_fout_apll)// rate is APLL/4
	{
                val = val & (~(0x1F << 12));
                val |= (0x0 << 12);
        }
        else if(parent == &clk_fout_mpll)// rate is MPLL/2
	{
                val = val & (~(0x1F << 12));
                val |= (0x1 << 12);
        }
        else if(parent == &clk_fout_epll)
        {
                val = val & (~(0x1F << 12));
                val |= (0x2 << 12);
        }
        else if(parent == &clk_mout_vpll.clk)
        {
                val = val & (~(0x1F << 12));
                val |= (0x3 << 12);
        }
        else
        {
                err = -EINVAL;
        }

        __raw_writel(val, S5P_CLK_OUT);
        return err;
}

static struct clk_ops s5pc11x_clkout_ops = {
	.set_rate = s5pc11x_clk_out_set_rate,
	.set_parent = s5pc11x_clk_out_set_parent,
};

struct clk xclk_out = {
        .name           = "clk_out",
        .id             = -1,
        .ops		= &s5pc11x_clkout_ops,
};

/* Clock Source Group 1 */
static struct clk *clkset_group1_list[] = {
	[0] = &clk_ext_xtal_mux,
	[1] = &clk_xusbxti,
	[2] = &clk_sclk_hdmi27m,
	[3] = &clk_sclk_usbphy0,
	[4] = &clk_sclk_usbphy1,
	[5] = &clk_sclk_hdmiphy,
	[6] = &clk_mout_mpll.clk,
	[7] = &clk_mout_epll.clk,
	[8] = &clk_mout_vpll.clk,
};

static struct clksrc_sources clkset_group1 = {
	.sources	= clkset_group1_list,
	.nr_sources	= ARRAY_SIZE(clkset_group1_list),
};

/* Clock sources for Audio 1 SCLK */
static struct clk *clkset_mout_audio1_list[] = {
	[0] = &clk_ext_xtal_mux,
	[1] = &clk_pcmcdclk1,
	[2] = &clk_sclk_hdmi27m,
	[3] = &clk_sclk_usbphy0,
	[4] = &clk_sclk_usbphy1,
	[5] = &clk_sclk_hdmiphy,
	[6] = &clk_mout_mpll.clk,
	[7] = &clk_mout_epll.clk,
	[8] = &clk_mout_vpll.clk,
};

static struct clksrc_sources clkset_mout_audio1 = {
	.sources	= clkset_mout_audio1_list,
	.nr_sources	= ARRAY_SIZE(clkset_mout_audio1_list),
};

/* Clock sources for Audio 2 SCLK */
static struct clk *clkset_mout_audio2_list[] = {
	[0] = &clk_ext_xtal_mux,
	[1] = &clk_pcmcdclk0,
	[2] = &clk_sclk_hdmi27m,
	[3] = &clk_sclk_usbphy0,
	[4] = &clk_sclk_usbphy1,
	[5] = &clk_sclk_hdmiphy,
	[6] = &clk_mout_mpll.clk,
	[7] = &clk_mout_epll.clk,
	[8] = &clk_mout_vpll.clk,
};

static struct clksrc_sources clkset_mout_audio2 = {
	.sources	= clkset_mout_audio2_list,
	.nr_sources	= ARRAY_SIZE(clkset_mout_audio2_list),
};

/* Clock Source Group 2 */
static struct clk *clkset_group2_list[] = {
	[0] = &clk_sclk_a2m.clk,
	[1] = &clk_mout_mpll.clk,
	[2] = &clk_mout_epll.clk,
	[3] = &clk_mout_vpll.clk,
};

static struct clksrc_sources clkset_group2 = {
	.sources	= clkset_group2_list,
	.nr_sources	= ARRAY_SIZE(clkset_group2_list),
};

/* MOUT CAM0 */
static struct clksrc_clk clk_mout_cam0 = {
	.clk		= {
		.name		= "mout_cam0",
		.id		= -1,
		.enable		= s5pv210_clk_mask0_ctrl,
		.ctrlbit	= (1 << 3),
	},
	.sources	= &clkset_group1,
	.reg_src	= { .reg = S5P_CLK_SRC1, .shift = 12, .size = 4 },
};

/* MOUT CAM1 */
static struct clksrc_clk clk_mout_cam1 = {
	.clk		= {
		.name		= "mout_cam1",
		.id		= -1,
		.enable		= s5pv210_clk_mask0_ctrl,
		.ctrlbit	= (1 << 4),
	},
	.sources	= &clkset_group1,
	.reg_src	= { .reg = S5P_CLK_SRC1, .shift = 16, .size = 4 },
};

/* MOUT FIMD */
static struct clksrc_clk clk_mout_fimd = {
	.clk		= {
		.name		= "mout_fimd",
		.id		= -1,
		.enable		= s5pv210_clk_mask0_ctrl,
		.ctrlbit	= (1 << 5),
	},
	.sources	= &clkset_group1,
	.reg_src	= { .reg = S5P_CLK_SRC1, .shift = 20, .size = 4 },
	//.reg_div 	= { .reg = S5P_CLK_DIV1, .shift = 20, .size = 4 },
};

/* MOUT MMC0 */
static struct clksrc_clk clk_mout_mmc0 = {
	.clk		= {
		.name		= "mout_mmc",
		.id		= 0,
		.enable		= s5pv210_clk_mask0_ctrl,
		.ctrlbit	= (1 << 8),

	},
	.sources	= &clkset_group1,
	.reg_src	= { .reg = S5P_CLK_SRC4, .shift = 0, .size = 4 },
};

/* MOUT MMC1 */
static struct clksrc_clk clk_mout_mmc1 = {
	.clk		= {
		.name		= "mout_mmc",
		.id		= 1,
		.enable		= s5pv210_clk_mask0_ctrl,
		.ctrlbit	= (1 << 9),
	},
	.sources	= &clkset_group1,
	.reg_src	= { .reg = S5P_CLK_SRC4, .shift = 4, .size = 4 },
};

/* MOUT MMC2 */
static struct clksrc_clk clk_mout_mmc2 = {
	.clk		= {
		.name		= "mout_mmc",
		.id		= 2,
		.enable		= s5pv210_clk_mask0_ctrl,
		.ctrlbit	= (1 << 10),
	},
	.sources	= &clkset_group1,
	.reg_src	= { .reg = S5P_CLK_SRC4, .shift = 8, .size = 4 },
};

/* MOUT MMC3 */
static struct clksrc_clk clk_mout_mmc3 = {
	.clk		= {
		.name		= "mout_mmc",
		.id		= 3,
		.enable		= s5pv210_clk_mask0_ctrl,
		.ctrlbit	= (1 << 11),
	},
	.sources	= &clkset_group1,
	.reg_src	= { .reg = S5P_CLK_SRC4, .shift = 12, .size = 4 },
};

/* SCLK MMC0 */
static struct clksrc_clk clk_sclk_mmc0 = {
	.clk		= {
		.name		= "sclk_mmc",  //"mmc_bus"
		.id		= 0,
		.parent		= &clk_mout_mmc0.clk,
		.enable		= s5pv210_clk_ip2_ctrl,
		.ctrlbit	= (1 << 16),
		.srcMaskReg     = S5P_CLK_SRC_MASK0,
		.srcMaskBit	= S5P_CLKSRC_MASK0_MMC0,
		.gb		= &gb_mmc,
	},
	.reg_div = { .reg = S5P_CLK_DIV4, .shift = 0, .size = 4 },
};

/* SCLK MMC1 */
static struct clksrc_clk clk_sclk_mmc1 = {
	.clk		= {
		.name		= "sclk_mmc",  //"mmc_bus"
		.id		= 1,
		.parent		= &clk_mout_mmc1.clk,
		.enable		= s5pv210_clk_ip2_ctrl,
		.ctrlbit	= (1 << 17),
		.srcMaskReg     = S5P_CLK_SRC_MASK0,
		.srcMaskBit	= S5P_CLKSRC_MASK0_MMC1,
		.gb		= &gb_mmc,
	},
	.reg_div = { .reg = S5P_CLK_DIV4, .shift = 4, .size = 4 },
};

/* SCLK MMC2 */
static struct clksrc_clk clk_sclk_mmc2 = {
	.clk		= {
		.name		= "sclk_mmc",  //"mmc_bus"
		.id		= 2,
		.parent		= &clk_mout_mmc2.clk,
		.enable		= s5pv210_clk_ip2_ctrl,
		.ctrlbit	= (1 << 18),
		.srcMaskReg     = S5P_CLK_SRC_MASK0,
		.srcMaskBit	= S5P_CLKSRC_MASK0_MMC2,
		.gb		= &gb_mmc,
	},
	.reg_div = { .reg = S5P_CLK_DIV4, .shift = 8, .size = 4 },
};

/* SCLK MMC3 */
static struct clksrc_clk clk_sclk_mmc3 = {
	.clk		= {
		.name		= "sclk_mmc",  //"mmc_bus"
		.id		= 3,
		.parent		= &clk_mout_mmc3.clk,
		.enable		= s5pv210_clk_ip2_ctrl,
		.ctrlbit	= (1 << 19),
		.srcMaskReg     = S5P_CLK_SRC_MASK0,
		.srcMaskBit	= S5P_CLKSRC_MASK0_MMC3,
		.gb		= &gb_mmc,
	},
	.reg_div = { .reg = S5P_CLK_DIV4, .shift = 12, .size = 4 },
};

/* MOUT AUDIO 0 */
static struct clksrc_clk clk_mout_audio0 = {
	.clk		= {
		.name		= "mout_audio",
		.id		= 0,
		.enable		= s5pv210_clk_mask0_ctrl,
		.ctrlbit	= (1 << 24),
	},
	.sources	= &clkset_group1,
	.reg_src	= { .reg = S5P_CLK_SRC6, .shift = 0, .size = 4 },
};

/* SCLK AUDIO 0 */
static struct clksrc_clk clk_sclk_audio0 = {
	.clk		= {
		.name		= "sclk_audio",	//"sclk_audio0"
		.id		= 0,
		.parent		= &clk_mout_audio0.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= (1 << 4),
		.srcMaskReg     = S5P_CLK_SRC_MASK0,
		.srcMaskBit	= S5P_CLKSRC_MASK0_AUDIO0,
	},
	.reg_div	= { .reg = S5P_CLK_DIV6, .shift = 0, .size = 4 },
};

static struct clk *clkset_mout_audss_list[] = {
	NULL,
	&clk_fout_epll,
};

static struct clksrc_sources clkset_mout_audss = {
	.sources	= clkset_mout_audss_list,
	.nr_sources	= ARRAY_SIZE(clkset_mout_audss_list),
};

static struct clksrc_clk clk_mout_audss = {
	.clk		= {
		.name		= "mout_audss",
		.id		= -1,
	},
	.sources	= &clkset_mout_audss,
	.reg_src	= { .reg = S5P_CLKSRC_AUDSS, .shift = 0, .size = 1 },
};

static struct clk *clkset_mout_i2s_a_list[] = {
	&clk_mout_audss.clk,
	&clk_i2scdclk0,
	&clk_sclk_audio0.clk,
};

static struct clksrc_sources clkset_mout_i2s_a = {
	.sources	= clkset_mout_i2s_a_list,
	.nr_sources	= ARRAY_SIZE(clkset_mout_i2s_a_list),
};

static struct clksrc_clk clk_mout_i2s_a = {
	.clk		= {
		.name		= "audio-bus",
		.id		= 0,
		.enable		= s5pv210_clk_audss_ctrl,
		.ctrlbit	= (1 << 6),
	},
	.sources	= &clkset_mout_i2s_a,
	.reg_src	= { .reg = S5P_CLKSRC_AUDSS, .shift = 2, .size = 2 },
	.reg_div	= { .reg = S5P_CLKDIV_AUDSS, .shift = 4, .size = 4 },
};

/* MOUT AUDIO 1 */
static struct clksrc_clk clk_mout_audio1 = {
	.clk		= {
		.name		= "mout_audio",
		.id		= 1,
		.enable		= s5pv210_clk_mask0_ctrl,
		.ctrlbit	= (1 << 25),
	},
	.sources	= &clkset_mout_audio1,
	.reg_src	= { .reg = S5P_CLK_SRC6, .shift = 4, .size = 4 },
};

/* SCLK AUDIO 1 */
static struct clksrc_clk clk_sclk_audio1 = {
	.clk		= {
		.name		= "sclk_audio",	//"sclk_audio1"
		.id		= 1,
		.parent		= &clk_mout_audio1.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= (1 << 5),
		.srcMaskReg     = S5P_CLK_SRC_MASK0,
		.srcMaskBit	= S5P_CLKSRC_MASK0_AUDIO1,
	},
	.reg_div	= { .reg = S5P_CLK_DIV6, .shift = 4, .size = 4 },
};

/* MOUT AUDIO 2 */
static struct clksrc_clk clk_mout_audio2 = {
	.clk		= {
		.name		= "mout_audio",
		.id		= 2,
		.enable		= s5pv210_clk_mask0_ctrl,
		.ctrlbit	= (1 << 26),
	},
	.sources	= &clkset_mout_audio2,
	.reg_src	= { .reg = S5P_CLK_SRC6, .shift = 8, .size = 4 },
};

/* SCLK AUDIO 2 */
static struct clksrc_clk clk_sclk_audio2 = {
	.clk		= {
		.name		= "sclk_audio",	//"sclk_audio2"
		.id		= 2,
		.parent		= &clk_mout_audio2.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= (1 << 6),
		.srcMaskReg     = S5P_CLK_SRC_MASK0,
		.srcMaskBit	= S5P_CLKSRC_MASK0_AUDIO2,
	},
	.reg_div	= { .reg = S5P_CLK_DIV6, .shift = 8, .size = 4 },
};

static struct clksrc_clk clk_dout_audio_bus_clk_i2s = {
	.clk		= {
		.name		= "dout_audio_bus_clk_i2s",
		.id		= -1,
		.parent 	= &clk_mout_audss,
		.enable 	= s5pv210_clk_audss_ctrl,
		.ctrlbit	= (1 << 5),
	},
	.reg_div	= { .reg = S5P_CLKDIV_AUDSS, .shift = 0, .size = 4 },
};

/* MOUT FIMC LCLK 0 */
static struct clksrc_clk clk_mout_fimc_lclk0 = {
	.clk		= {
		.name		= "mout_fimc_lclk",
		.id		= 0,
		.enable		= s5pv210_clk_mask1_ctrl,
		.ctrlbit	= (1 << 2),
	},
	.sources	= &clkset_group1,
	.reg_src	= { .reg = S5P_CLK_SRC3, .shift = 12, .size = 4 },
};

/* MOUT FIMC LCLK 1 */
static struct clksrc_clk clk_mout_fimc_lclk1 = {
	.clk		= {
		.name		= "mout_fimc_lclk",
		.id		= 1,
		.enable		= s5pv210_clk_mask1_ctrl,
		.ctrlbit	= (1 << 3),
	},
	.sources	= &clkset_group1,
	.reg_src	= { .reg = S5P_CLK_SRC3, .shift = 16, .size = 4 },
};

/* MOUT FIMC LCLK 2 */
static struct clksrc_clk clk_mout_fimc_lclk2 = {
	.clk		= {
		.name		= "mout_fimc_lclk",
		.id		= 2,
		.enable		= s5pv210_clk_mask1_ctrl,
		.ctrlbit	= (1 << 4),
	},
	.sources	= &clkset_group1,
	.reg_src	= { .reg = S5P_CLK_SRC3, .shift = 20, .size = 4 },
};

/* MOUT CSIS */
static struct clksrc_clk clk_mout_csis = {
	.clk		= {
		.name		= "mout_csis",
		.id		= -1,
		.enable		= s5pv210_clk_mask0_ctrl,
		.ctrlbit	= (1 << 6),
	},
	.sources	= &clkset_group1,
	.reg_src	= { .reg = S5P_CLK_SRC1, .shift = 24, .size = 4 },
};

/* MOUT SPI 0 */
static struct clksrc_clk clk_mout_spi0 = {
	.clk		= {
		.name		= "mout_spi",
		.id		= 0,
		.enable		= s5pv210_clk_mask0_ctrl,
		.ctrlbit	= (1 << 16),
	},
	.sources	= &clkset_group1,
	.reg_src	= { .reg = S5P_CLK_SRC5, .shift = 0, .size = 4 },
};

/* MOUT SPI 1 */
static struct clksrc_clk clk_mout_spi1 = {
	.clk		= {
		.name		= "mout_spi",
		.id		= 1,
		.enable		= s5pv210_clk_mask0_ctrl,
		.ctrlbit	= (1 << 17),
	},
	.sources	= &clkset_group1,
	.reg_src	= { .reg = S5P_CLK_SRC5, .shift = 4, .size = 4 },
};

/* MOUT PWI */
static struct clksrc_clk clk_mout_pwi = {
	.clk		= {
		.name		= "mout_pwi",
		.id		= -1,
		.enable		= s5pv210_clk_mask0_ctrl,
		.ctrlbit	= (1 << 29),
	},
	.sources	= &clkset_group1,
	.reg_src	= { .reg = S5P_CLK_SRC6, .shift = 20, .size = 4 },
};

/* MOUT UART 0 */
static struct clksrc_clk clk_mout_uart0 = {
	.clk		= {
		.name		= "mout_uart",
		.id		= 0,
		.enable		= s5pv210_clk_mask0_ctrl,
		.ctrlbit	= (1 << 12),
	},
	.sources	= &clkset_group1,
	.reg_src	= { .reg = S5P_CLK_SRC4, .shift = 16, .size = 4 },
};

/* MOUT UART 1 */
static struct clksrc_clk clk_mout_uart1 = {
	.clk		= {
		.name		= "mout_uart",
		.id		= 1,
		.enable		= s5pv210_clk_mask0_ctrl,
		.ctrlbit	= (1 << 13),
	},
	.sources	= &clkset_group1,
	.reg_src	= { .reg = S5P_CLK_SRC4, .shift = 20, .size = 4 },
};

/* MOUT UART 2 */
static struct clksrc_clk clk_mout_uart2 = {
	.clk		= {
		.name		= "mout_uart",
		.id		= 2,
		.enable		= s5pv210_clk_mask0_ctrl,
		.ctrlbit	= (1 << 14),
	},
	.sources	= &clkset_group1,
	.reg_src	= { .reg = S5P_CLK_SRC4, .shift = 24, .size = 4 },
};

/* MOUT UART 3 */
static struct clksrc_clk clk_mout_uart3 = {
	.clk		= {
		.name		= "mout_uart",
		.id		= 3,
		.enable		= s5pv210_clk_mask0_ctrl,
		.ctrlbit	= (1 << 15),
	},
	.sources	= &clkset_group1,
	.reg_src	= { .reg = S5P_CLK_SRC4, .shift = 28, .size = 4 },
};

/* MOUT PWM */
static struct clksrc_clk clk_mout_pwm = {
	.clk		= {
		.name		= "mout_pwm",
		.id		= -1,
		.enable		= s5pv210_clk_mask0_ctrl,
		.ctrlbit	= (1 << 19),
	},
	.sources	= &clkset_group1,
	.reg_src	= { .reg = S5P_CLK_SRC5, .shift = 12, .size = 4 },
};

/* SCLK SPDIF */
static struct clk *clkset_sclk_spdif_list[] = {
	[0] = &clk_sclk_audio0.clk,
	[1] = &clk_sclk_audio1.clk,
	[2] = &clk_sclk_audio2.clk,
};

static struct clksrc_sources clkset_sclk_spdif = {
	.sources	= clkset_sclk_spdif_list,
	.nr_sources	= ARRAY_SIZE(clkset_sclk_spdif_list),
};

/* MOUT MDNIE */
static struct clksrc_clk clk_mout_mdnie = {
	.clk		= {
		.name		= "mout_mdnie",
		.id		= -1,
		.enable		= s5pv210_clk_mask1_ctrl,
		.ctrlbit	= (1 << 0),
	},
	.sources	= &clkset_group1,
	.reg_src	= { .reg = S5P_CLK_SRC3, .shift = 0, .size = 4 },
};

/* MOUT MDNIE PWM */
static struct clksrc_clk clk_mout_mdnie_pwm = {
	.clk		= {
		.name		= "mout_mdnie_pwm",
		.id		= -1,
		.enable		= s5pv210_clk_mask1_ctrl,
		.ctrlbit	= (1 << 1),
	},
	.sources	= &clkset_group1,
	.reg_src	= { .reg = S5P_CLK_SRC3, .shift = 4, .size = 4 },
};

/* COPY */
static struct clk *clkset_copy_list[] = {
	[6] = &clk_mout_apll.clk,
	[7] = &clk_mout_mpll.clk,
};

static struct clksrc_sources clkset_copy = {
	.sources	= clkset_copy_list,
	.nr_sources	= ARRAY_SIZE(clkset_copy_list),
};

static struct clksrc_clk clk_copy = {
	.clk		= {
		.name		= "copy",
		.id		= -1,
	},
	.sources	= &clkset_copy,
	.reg_src	= { .reg = S5P_CLK_SRC6, .shift = 16, .size = 1 },
	.reg_div	= { .reg = S5P_CLK_DIV6, .shift = 16, .size = 3 },
};

static struct clksrc_clk clksrcs[] = {
	{
		.clk	= {
			.name		= "sclk_onenand",
			.id		= -1,
		},
		.sources = &clkset_sclk_onenand,
		.reg_src = { .reg = S5P_CLK_SRC0, .shift = 28, .size = 1 },
		.reg_div = { .reg = S5P_CLK_DIV6, .shift = 12, .size = 3 },
	}, {
		.clk	= {
			.name		= "sclk_fimc",
			.id		= -1,
			.parent		= &clk_mout_166.clk,
		},
		.reg_div = { .reg = S5P_CLK_DIV1, .shift = 8, .size = 4 },
	}, {
		.clk	= {
			.name		= "sclk_mixer",
			.id		= -1,
			.enable 	= s5pv210_clk_ip1_ctrl,
			.ctrlbit	= (1 << 9),
		},
		.sources = &clkset_sclk_mixer,
		.reg_src = { .reg = S5P_CLK_SRC1, .shift = 4, .size = 1 },
	}, {
		.clk		= {
			.name		= "sclk_cam0",
			.id		= -1,
			.parent		= &clk_mout_cam0.clk,
		},
		.reg_div = { .reg = S5P_CLK_DIV1, .shift = 12, .size = 4 },
	}, {
		.clk		= {
			.name		= "sclk_cam1",
			.id		= -1,
			.parent		= &clk_mout_cam1.clk,
		},
		.reg_div = { .reg = S5P_CLK_DIV1, .shift = 16, .size = 4 },
	}, {
		.clk		= {
			.name		= "sclk_fimd", //"sclk_lcd"
			.id		= -1,
			.parent		= &clk_mout_fimd.clk,
			.enable		= s5pv210_clk_ip1_ctrl,
			.ctrlbit	= (1 << 0),
			.srcMaskReg     = S5P_CLK_SRC_MASK0,
			.srcMaskBit	= S5P_CLKSRC_MASK0_FIMD,
			.powerDomain = S5PC110_POWER_DOMAIN_LCD,
			.gb		= &gb_lcd,
		},
		.reg_div = { .reg = S5P_CLK_DIV1, .shift = 20, .size = 4 },
	}, {
		.clk		= {
			.name		= "sclk_fimc_lclk", //"sclk_fimc"
			.id		= 0,
			.parent		= &clk_mout_fimc_lclk0.clk,
			.enable		= s5pv210_clk_ip0_ctrl,
			.ctrlbit	= (1 << 24),
			.powerDomain	= S5PC110_POWER_DOMAIN_CAM,
			.srcMaskReg     = S5P_CLK_SRC_MASK1,
			.srcMaskBit	= S5P_CLKSRC_MASK1_FIMC0_LCLK,
			.gb		= &gb_img,
		},
		.reg_div = { .reg = S5P_CLK_DIV3, .shift = 12, .size = 4 },
	}, {
		.clk		= {
			.name		= "sclk_fimc_lclk", //"sclk_fimc"
			.id		= 1,
			.parent		= &clk_mout_fimc_lclk1.clk,
			.enable		= s5pv210_clk_ip0_ctrl,
			.ctrlbit	= (1 << 25),
			.powerDomain	= S5PC110_POWER_DOMAIN_CAM,
			.srcMaskReg     = S5P_CLK_SRC_MASK1,
			.srcMaskBit	= S5P_CLKSRC_MASK1_FIMC1_LCLK,
			.gb		= &gb_img,
		},
		.reg_div = { .reg = S5P_CLK_DIV3, .shift = 16, .size = 4 },
	}, {
		.clk		= {
			.name		= "sclk_fimc_lclk", //"sclk_fimc"
			.id		= 2,
			.parent		= &clk_mout_fimc_lclk2.clk,
			.enable		= s5pv210_clk_ip0_ctrl,
			.ctrlbit	= (1 << 26),
			.powerDomain	= S5PC110_POWER_DOMAIN_CAM,
			.srcMaskReg     = S5P_CLK_SRC_MASK1,
			.srcMaskBit	= S5P_CLKSRC_MASK1_FIMC2_LCLK,
			.gb		= &gb_img,
		},
		.reg_div = { .reg = S5P_CLK_DIV3, .shift = 20, .size = 4 },
	}, {
		.clk		= {
			.name		= "sclk_onedram",
			.id		= -1,
			.enable		= s5pv210_clk_ip0_ctrl,
			.ctrlbit	= (1 << 1),
		},
		.sources = &clkset_group2,
		.reg_src = { .reg = S5P_CLK_SRC6, .shift = 24, .size = 2 },
		.reg_div = { .reg = S5P_CLK_DIV6, .shift = 28, .size = 4 },
	}, {
		.clk		= {
			.name		= "sclk_mfc",
			.id		= -1,
			.enable		= s5pv210_clk_ip0_ctrl,
			.ctrlbit	= (1 << 16),
		},
		.sources = &clkset_group2,
		.reg_src = { .reg = S5P_CLK_SRC2, .shift = 4, .size = 2 },
		.reg_div = { .reg = S5P_CLK_DIV2, .shift = 4, .size = 4 },
	}, {
		.clk		= {
			.name		= "sclk_g2d",
			.id		= -1,
			.enable		= s5pv210_clk_ip0_ctrl,
			.ctrlbit	= (1 << 12),
		},
		.sources = &clkset_group2,
		.reg_src = { .reg = S5P_CLK_SRC2, .shift = 8, .size = 2 },
		.reg_div = { .reg = S5P_CLK_DIV2, .shift = 8, .size = 4 },
	}, {
		.clk		= {
			.name		= "sclk_g3d",
			.id		= -1,
			.enable		= s5pv210_clk_ip0_ctrl,
			.ctrlbit	= (1 << 8),
		},
		.sources = &clkset_group2,
		.reg_src = { .reg = S5P_CLK_SRC2, .shift = 0, .size = 2 },
		.reg_div = { .reg = S5P_CLK_DIV2, .shift = 0, .size = 4 },
	}, {
		.clk		= {
			.name		= "sclk_csis",
			.id		= -1,
			.parent		= &clk_mout_csis.clk,
			.enable		= s5pv210_clk_ip0_ctrl,
			.ctrlbit	= (1 << 31),
		},
		.reg_div = { .reg = S5P_CLK_DIV1, .shift = 28, .size = 4 },
	}, {
		.clk		= {
			.name		= "sclk_spi",  //"spi-bus"
			.id		= 0,
			.parent		= &clk_mout_spi0.clk,
			.enable		= s5pv210_clk_ip3_ctrl,
			.ctrlbit	= (1 << 12),
			.srcMaskReg     = S5P_CLK_SRC_MASK0,
			.srcMaskBit	= S5P_CLKSRC_MASK0_SPI0,
		},
		.reg_div = { .reg = S5P_CLK_DIV5, .shift = 0, .size = 4 },
	}, {
		.clk		= {
			.name		= "sclk_spi", //"spi-bus"
			.id		= 1,
			.parent		= &clk_mout_spi1.clk,
			.enable		= s5pv210_clk_ip3_ctrl,
			.ctrlbit	= (1 << 13),
			.srcMaskReg     = S5P_CLK_SRC_MASK0,
			.srcMaskBit	= S5P_CLKSRC_MASK0_SPI1,
		},
		.reg_div = { .reg = S5P_CLK_DIV5, .shift = 4, .size = 4 },
	}, {
		.clk		= {
			.name		= "sclk_spi", //"spi-bus"
			.id		= 2,
			.enable		= &s5pv210_clk_ip3_ctrl,
			.ctrlbit	= (1 << 14),		// reserved
			.srcMaskReg     = S5P_CLK_SRC_MASK0,
			.srcMaskBit	= S5P_CLKSRC_MASK0_SPI2, // reserved
		},
		.sources = &clkset_group1,
		.reg_src = { .reg = S5P_CLK_SRC5, .shift = 8, .size = 4 },
	}, {
		.clk		= {
			.name		= "sclk_pwi",
			.id		= -1,
			.parent		= &clk_mout_pwi.clk,
			.enable		= &s5pv210_clk_ip4_ctrl,
			.ctrlbit	= (1 << 2),
		},
		.sources = &clkset_group1,
		.reg_div = { .reg = S5P_CLK_DIV6, .shift = 24, .size = 4 },
	}, {
		.clk		= {
			.name		= "sclk_uart", //"uclk1"
			.id		= 0,
			.parent		= &clk_mout_uart0.clk,
			.enable		= s5pv210_clk_ip3_ctrl,
			.ctrlbit	= (1 << 17),
			.srcMaskReg     = S5P_CLK_SRC_MASK0,
			.srcMaskBit	= S5P_CLKSRC_MASK0_UART0,
		},
		.reg_div = { .reg = S5P_CLK_DIV4, .shift = 16, .size = 4 },
	}, {
		.clk		= {
			.name		= "sclk_uart", //"uclk1"
			.id		= 1,
			.parent		= &clk_mout_uart1.clk,
			.enable		= s5pv210_clk_ip3_ctrl,
			.ctrlbit	= (1 << 18),
			.srcMaskReg     = S5P_CLK_SRC_MASK0,
			.srcMaskBit	= S5P_CLKSRC_MASK0_UART1,
		},
		.reg_div = { .reg = S5P_CLK_DIV4, .shift = 20, .size = 4 },
	}, {
		.clk		= {
			.name		= "sclk_uart",  //"uclk1"
			.id		= 2,
			.parent		= &clk_mout_uart2.clk,
			.enable		= s5pv210_clk_ip3_ctrl,
			.ctrlbit	= (1 << 19),
			.srcMaskReg     = S5P_CLK_SRC_MASK0,
			.srcMaskBit	= S5P_CLKSRC_MASK0_UART2,
		},
		.reg_div = { .reg = S5P_CLK_DIV4, .shift = 24, .size = 4 },
	}, {
		.clk		= {
			.name		= "sclk_uart",  //"uclk1"
			.id		= 3,
			.parent		= &clk_mout_uart3.clk,
			.enable		= s5pv210_clk_ip3_ctrl,
			.ctrlbit	= (1 << 20),
			.srcMaskReg     = S5P_CLK_SRC_MASK0,
			.srcMaskBit	= S5P_CLKSRC_MASK0_UART3,
		},
		.reg_div = { .reg = S5P_CLK_DIV4, .shift = 28, .size = 4 },
	}, {
		.clk		= {
			.name		= "sclk_pwm",
			.id		= -1,
			.parent		= &clk_mout_pwm.clk,
			.enable		= s5pv210_clk_ip3_ctrl,
			.ctrlbit	= (1 << 23),
		},
		.reg_div = { .reg = S5P_CLK_DIV5, .shift = 12, .size = 4 },
	}, {
		.clk		= {
			.name		= "sclk_spdif",
			.id		= -1,
			.enable		= s5pv210_clk_mask0_ctrl,
			.ctrlbit	= (1 << 27),
		},
		.sources = &clkset_sclk_spdif,
		.reg_src = { .reg = S5P_CLK_SRC6, .shift = 12, .size = 2 },
	}, {
		.clk		= {
			.name		= "sclk_mdnie",	//"mdnie_sel"
			.id		= -1,
			.parent		= &clk_mout_mdnie.clk,
			.enable		= s5pv210_clk_ip1_ctrl,
			.ctrlbit	= (1 << 1), //MIE
			.srcMaskReg     = S5P_CLK_SRC_MASK1,
			.srcMaskBit	= S5P_CLKSRC_MASK1_MDNIE,
			.powerDomain = S5PC110_POWER_DOMAIN_LCD,
		},
		.reg_div = { .reg = S5P_CLK_DIV3, .shift = 0, .size = 4 },
	}, {
		.clk		= {
			.name		= "sclk_mdnie_pwm",	//"mdnie_pwmclk_sel"
			.id		= -1,
			.parent		= &clk_mout_mdnie_pwm.clk,
			.srcMaskReg     = S5P_CLK_SRC_MASK1,
			.srcMaskBit	= S5P_CLKSRC_MASK1_MDNIE_PWM,
		},
		.reg_div = { .reg = S5P_CLK_DIV3, .shift = 4, .size = 7 },
	}, {
		.clk		= {
			.name		= "sclk_hpm",
			.id		= -1,
			.parent		= &clk_copy.clk,
		},
		.reg_div = { .reg = S5P_CLK_DIV6, .shift = 20, .size = 3 },
	},
};


static struct clk init_clocks_disable[] = {
/*Disable: IP0*/
	{
		.name		= "csis",
		.id		= -1,
		.parent		= &clk_pclk_83.clk,
		.enable		= s5pv210_clk_ip0_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP0_CSIS,
		.powerDomain = S5PC110_POWER_DOMAIN_CAM,
		.srcMaskReg 	= S5P_CLK_SRC_MASK0,
		.srcMaskBit 	= S5P_CLKSRC_MASK0_CSIS,
		.gb		= &gb_img,
	}, {
		.name		= "ipc",
		.id		= -1,
		.parent		= &clk_hclk_166.clk,
		.enable		= s5pv210_clk_ip0_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP0_IPC,
		.powerDomain = S5PC110_POWER_DOMAIN_CAM,
	}, {
		.name		= "rotator",
		.id		= -1,
		.parent		= &clk_hclk_166.clk,
		.enable		= s5pv210_clk_ip0_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP0_ROTATOR,
		.powerDomain = S5PC110_POWER_DOMAIN_CAM,
		.gb		= &gb_img,
	}, {
		.name		= "jpeg",
		.id		= -1,
		.parent		= &clk_hclk_166.clk,
		.enable		= s5pv210_clk_ip0_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP0_JPEG, //S5P_CLKGATE_IP0_JPEG
		.powerDomain = S5PC110_POWER_DOMAIN_CAM,
		.gb		= &gb_img,
	}, {
		.name		= "fimc",
		.id		= 2,
		.parent		= &clk_hclk_166.clk,
		.enable		= s5pv210_clk_ip0_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP0_FIMC2,
		.powerDomain = S5PC110_POWER_DOMAIN_CAM,
		.srcMaskReg 	= S5P_CLK_SRC_MASK1,
		.srcMaskBit 	= S5P_CLKSRC_MASK1_FIMC2_LCLK,
		.gb		= &gb_img,
	}, {
		.name		= "fimc",
		.id		= 1,
		.parent		= &clk_hclk_166.clk,
		.enable		= s5pv210_clk_ip0_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP0_FIMC1,
		.powerDomain = S5PC110_POWER_DOMAIN_CAM,
		.srcMaskReg 	= S5P_CLK_SRC_MASK1,
		.srcMaskBit 	= S5P_CLKSRC_MASK1_FIMC1_LCLK,
		.gb		= &gb_img,
	}, {
		.name		= "fimc",
		.id		= 0,
		.parent		= &clk_hclk_166.clk,
		.enable		= s5pv210_clk_ip0_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP0_FIMC0,
		.powerDomain = S5PC110_POWER_DOMAIN_CAM,
		.srcMaskReg 	= S5P_CLK_SRC_MASK1,
		.srcMaskBit 	= S5P_CLKSRC_MASK1_FIMC0_LCLK,
		.gb		= &gb_img,
	}, {
		.name		= "mfc",
		.id		= -1,
		.parent		= &clk_hclk_200.clk,
		.enable		= s5pv210_clk_ip0_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP0_MFC,
		.powerDomain = S5PC110_POWER_DOMAIN_MFC,
		.gb		= &gb_mfc,
	}, {
		.name		= "g2d",
		.id		= -1,
		.parent		= &clk_hclk_166.clk,
		.enable		= s5pv210_clk_ip0_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP0_G2D,
		.powerDomain = S5PC110_POWER_DOMAIN_LCD,
		.gb		= &gb_lcd,
	}, {
#if 0
		.name		= "g3d",
		.id		= -1,
		.parent		= &clk_hclk_200.clk,
		.enable		= s5pv210_clk_ip0_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP0_G3D,
		.powerDomain = S5PC110_POWER_DOMAIN_G3D,
	}, {
		.name		= "imem",
		.id		= -1,
		.parent		= &clk_pclk_100.clk,
		.enable		= s5pv210_clk_ip0_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP0_IMEM,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
#endif
//		.name		= "pdma1",
//		.id		= -1,
//		.parent		= &clk_hclk_133.clk,
//		.enable		= s5pv210_clk_ip0_ctrl,
//		.ctrlbit	= S5P_CLKGATE_IP0_PDMA1,
//		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
//	}, {
//		.name		= "pdma0",
//		.id		= -1,
//		.parent		= &clk_hclk_133.clk,
//		.enable		= s5pv210_clk_ip0_ctrl,
//		.ctrlbit	= S5P_CLKGATE_IP0_PDMA0,
//		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
//	}, {
//		.name		= "mdma",
//		.id		= -1,
//		.parent		= &clk_hclk_166.clk,
//		.enable		= s5pv210_clk_ip0_ctrl,
//		.ctrlbit	= S5P_CLKGATE_IP0_MDMA,
//		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
//	}, {
//		.name		= "sromc",
//		.id		= -1,
//		.parent		= &clk_hclk_133.clk,
//		.enable		= s5pv210_clk_ip1_ctrl,
//		.ctrlbit	= S5P_CLKGATE_IP1_SROMC,
//		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
//		.gb		= &gb_memory,
//	}, {
/*Disable: IP1*/
		.name		= "nfcon",
		.id		= -1,
		.parent		= &clk_hclk_133.clk,
		.enable		= s5pv210_clk_ip1_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP1_NFCON,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
		.gb		= &gb_memory,
	}, {
		.name		= "cfcon",
		.id		= -1,
		.parent		= &clk_hclk_133.clk,
		.enable		= s5pv210_clk_ip1_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP1_CFCON,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
		.gb		= &gb_memory,
	}, {
#if 0
		.name		= "nandxl",
		.id		= -1,
		.parent		= &clk_hclk_133.clk,
		.enable		= s5pv210_clk_ip1_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP1_NANDXL,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
#endif
#if !defined(CONFIG_MACH_S5PC110_P1)
		.name		= "usbhost",
		.id		= -1,
		.parent		= &clk_hclk_133.clk,
		.enable		= s5pv210_clk_ip1_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP1_USBHOST,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
		.gb		= &gb_usb,
	}, {
		.name           = "usbotg",
		.id             = -1,
		.parent         = &clk_hclk_133.clk,
		.enable         = s5pv210_clk_ip1_ctrl,
		.ctrlbit        = S5P_CLKGATE_IP1_USBOTG,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
		.gb		= &gb_usb,
	}, {
#endif
		.name		= "hdmi",
		.id		= -1,
		.parent		= &clk_hclk_166.clk,
		.enable		= s5pv210_clk_ip1_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP1_HDMI,
		.powerDomain = S5PC110_POWER_DOMAIN_TV,
//		.srcMaskReg 	= S5P_CLK_SRC_MASK0,
//		.srcMaskBit 	= S5P_CLKSRC_MASK0_HDMI,
		.gb		= &gb_tv,
	}, {
		.name		= "tvenc",
		.id		= -1,
		.parent		= &clk_hclk_166.clk,
		.enable		= s5pv210_clk_ip1_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP1_TVENC,
		.powerDomain = S5PC110_POWER_DOMAIN_TV,
//		.srcMaskReg 	= S5P_CLK_SRC_MASK0,
//		.srcMaskBit 	= S5P_CLKSRC_MASK0_DAC,
		.gb		= &gb_tv,
	}, {
		.name		= "mixer",
		.id		= -1,
		.parent		= &clk_hclk_166.clk,
		.enable		= s5pv210_clk_ip1_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP1_MIXER,
		.powerDomain = S5PC110_POWER_DOMAIN_TV,
//		.srcMaskReg 	= S5P_CLK_SRC_MASK0,
//		.srcMaskBit 	= S5P_CLKSRC_MASK0_MIXER,
		.gb		= &gb_tv,
	}, {
		.name		= "vp",
		.id		= -1,
		.parent		= &clk_hclk_166.clk,
		.enable		= s5pv210_clk_ip1_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP1_VP,
		.powerDomain = S5PC110_POWER_DOMAIN_TV,
		.gb		= &gb_tv,
	}, {
		.name		= "dsim",
		.id		= -1,
		.parent		= &clk_hclk_166.clk,
		.enable		= s5pv210_clk_ip1_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP1_DSIM,
		.powerDomain = S5PC110_POWER_DOMAIN_LCD,
		.gb		= &gb_lcd,
	}, {
#ifndef CONFIG_FB_S3C_MDNIE
		.name		= "mdnie",
		.id		= -1,
		.parent		= &clk_hclk_166.clk,
		.enable		= s5pv210_clk_ip1_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP1_MIE,
		.powerDomain = S5PC110_POWER_DOMAIN_LCD,
	}, {
#endif
#if 0
		.name		= "lcd",
		.id		= -1,
		.parent		= &clk_hclk_166.clk,
		.enable		= s5pv210_clk_ip1_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP1_FIMD,
		.powerDomain = S5PC110_POWER_DOMAIN_LCD,
		.srcMaskReg 	= S5P_CLK_SRC_MASK0,
		.srcMaskBit 	= S5P_CLKSRC_MASK0_FIMD,
		.gb		= &gb_lcd,
	}, {
#endif
/*Disable: IP2*/
		.name		= "tzic3",
		.id		= -1,
		.parent		= &clk_hclk_200.clk,
		.enable		= s5pv210_clk_ip2_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP2_TZIC3,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "tzic2",
		.id		= -1,
		.parent		= &clk_hclk_200.clk,
		.enable		= s5pv210_clk_ip2_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP2_TZIC2,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "tzic1",
		.id		= -1,
		.parent		= &clk_hclk_200.clk,
		.enable		= s5pv210_clk_ip2_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP2_TZIC1,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "tzic0",
		.id		= -1,
		.parent		= &clk_hclk_200.clk,
		.enable		= s5pv210_clk_ip2_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP2_TZIC0,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "tsi",
		.id		= -1,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip2_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP2_TSI,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
		.gb		= &gb_mmc,
	}, {
		.name		= "hsmmc",
		.id		= 3,
		.parent		= &clk_hclk_133.clk,
		.enable		= s5pv210_clk_ip2_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP2_HSMMC3,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
		.srcMaskReg 	= S5P_CLK_SRC_MASK0,
		.srcMaskBit 	= S5P_CLKSRC_MASK0_MMC3,
		.gb		= &gb_mmc,
	}, {
		.name		= "hsmmc",
		.id		= 2,
		.parent		= &clk_hclk_133.clk,
		.enable		= s5pv210_clk_ip2_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP2_HSMMC2,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
		.srcMaskReg 	= S5P_CLK_SRC_MASK0,
		.srcMaskBit 	= S5P_CLKSRC_MASK0_MMC2,
		.gb		= &gb_mmc,
	}, {
		.name		= "hsmmc",
		.id		= 1,
		.parent		= &clk_hclk_133.clk,
		.enable		= s5pv210_clk_ip2_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP2_HSMMC1,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
		.srcMaskReg 	= S5P_CLK_SRC_MASK0,
		.srcMaskBit 	= S5P_CLKSRC_MASK0_MMC1,
		.gb		= &gb_mmc,
	}, {
		.name		= "hsmmc",
		.id		= 0,
		.parent		= &clk_hclk_133.clk,
		.enable		= s5pv210_clk_ip2_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP2_HSMMC0,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
		.srcMaskReg 	= S5P_CLK_SRC_MASK0,
		.srcMaskBit 	= S5P_CLKSRC_MASK0_MMC0,
		.gb		= &gb_mmc,
        }, {
		.name		= "secjtag",
		.id		= -1,
		.parent		= &clk_hclk_133.clk,
		.enable		= s5pv210_clk_ip2_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP2_SECJTAG,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
		.gb		= &gb_debug,
        }, {
		.name		= "hostif",
		.id		= -1,
		.parent		= &clk_hclk_133.clk,
		.enable		= s5pv210_clk_ip2_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP2_HOSTIF,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
		.gb		= &gb_debug,
        }, {
		.name		= "modem",
		.id		= -1,
		.parent		= &clk_hclk_133.clk,
		.enable		= s5pv210_clk_ip2_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP2_MODEM,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
		.gb		= &gb_debug,
        }, {
//		.name		= "coresight",
//		.id		= -1,
//		.parent		= &clk_hclk_200.clk,
//		.enable		= s5pv210_clk_ip2_ctrl,
//		.ctrlbit	= S5P_CLKGATE_IP2_CORESIGHT,
//		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
//        }, {
		.name		= "sdm",
		.id		= -1,
		.parent		= &clk_hclk_200.clk,
		.enable		= s5pv210_clk_ip2_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP2_SDM,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
        }, {

		.name		= "secss",
		.id		= -1,
		.parent		= &clk_hclk_133.clk,
		.enable		= s5pv210_clk_ip2_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP2_SECSS,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
		.gb		= &gb_security,
        }, {

/*Disable: IP3*/
		.name		= "pcm",
		.id		= 2,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_PCM2,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "pcm",
		.id		= 1,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_PCM1 | S5P_CLKGATE_IP3_I2S1 ,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "pcm",
		.id		= 0,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_PCM0,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "adc",
		.id		= -1,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_TSADC,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
#if 0
		.name		= "timers",
		.id		= -1,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_PWM,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
#endif
#if 0
		.name		= "watchdog",
		.id		= -1,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_WDT,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
#endif
		.name		= "keypad",
		.id		= -1,
		.parent		 = &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_KEYIF,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
#if 0
		.name		= "uart",
		.id		= 3,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_UART3,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "uart",
		.id		= 2,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_UART2,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "uart",
		.id		= 1,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_UART1,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "uart",
		.id		= 0,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_UART0,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "systimer",
		.id		= -1,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_SYSTIMER,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
#endif
#if 0
		.name		= "rtc",
		.id		= -1,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_RTC,
		.powerDomain = S5PC110_POWER_DOMAIN_RTC,
	}, {
#endif
		.name           = "spi",
		.id             = 2,
		.parent         = &clk_pclk_66.clk,
		.enable         = s5pv210_clk_ip3_ctrl,
		.ctrlbit        = S5P_CLKGATE_IP3_SPI2,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
		.srcMaskReg 	= S5P_CLK_SRC_MASK0,
		.srcMaskBit 	= S5P_CLKSRC_MASK0_SPI2,
        }, {
		.name           = "spi",
		.id             = 1,
		.parent         = &clk_pclk_66.clk,
		.enable         = s5pv210_clk_ip3_ctrl,
		.ctrlbit        = S5P_CLKGATE_IP3_SPI1,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
		.srcMaskReg 	= S5P_CLK_SRC_MASK0,
		.srcMaskBit 	= S5P_CLKSRC_MASK0_SPI1,
        }, {
		.name           = "spi",
		.id             = 0,
		.parent         = &clk_pclk_66.clk,
		.enable         = s5pv210_clk_ip3_ctrl,
		.ctrlbit        = S5P_CLKGATE_IP3_SPI0,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
		.srcMaskReg 	= S5P_CLK_SRC_MASK0,
		.srcMaskBit 	= S5P_CLKSRC_MASK0_SPI0,
        }, {
		.name		= "i2c-hdmiphy",
		.id		= -1,
		.parent		= &clk_pclk_83.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_I2C_HDMI_PHY,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "i2c",
		.id		= 2,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_I2C2,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "i2c",
		.id		= 1,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_I2C_HDMI_DDC,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "i2c",
		.id		= 0,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_I2C0,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
//	}, {
//		.name		= "i2s_v50",
//		.id		= 0,
//		.parent		= &clk_pclk_66.clk,
//		.enable		= s5pv210_clk_ip3_ctrl,
//		.ctrlbit	= S5P_CLKGATE_IP3_I2S0 | S5P_CLKGATE_IP3_PCM0, /* I2S0 is v5.0 */
////		.powerDomain = S5PC110_POWER_DOMAIN_AUDIO,
//		.srcMaskReg 	= S5P_CLK_SRC_MASK0,
//		.srcMaskBit 	= S5P_CLKSRC_MASK0_AUDIO0,
	}, {
		.name		= "i2s_v32",
		.id		= 0,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_I2S1 | S5P_CLKGATE_IP3_PCM1, /* I2S1 is v3.2 */
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
		.srcMaskReg 	= S5P_CLK_SRC_MASK0,
		.srcMaskBit 	= S5P_CLKSRC_MASK0_AUDIO1,
	}, {
		.name		= "i2s_v32",
		.id		= 1,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_I2S2, /* I2S2 is v3.2 */
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
		.srcMaskReg 	= S5P_CLK_SRC_MASK0,
		.srcMaskBit 	= S5P_CLKSRC_MASK0_AUDIO2,
	}, {
		.name		= "ac97",
		.id		= 1,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_AC97,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "spdif",
		.id		= -1,
		.parent		= NULL,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_SPDIF | S5P_CLKGATE_IP3_PCM0 | S5P_CLKGATE_IP3_PCM1 | S5P_CLKGATE_IP3_I2S0 | S5P_CLKGATE_IP3_I2S1 | S5P_CLKGATE_IP3_I2S2,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
		.srcMaskReg 	= S5P_CLK_SRC_MASK0,
		.srcMaskBit 	= S5P_CLKSRC_MASK0_SPDIF,
/*Disable: IP4*/
	}, {
		.name		= "tzpc",
		.id		= 0,
		.parent		= &clk_pclk_100.clk,
		.enable		= s5pv210_clk_ip4_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP4_TZPC0,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "tzpc",
		.id		= 1,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip4_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP4_TZPC1,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "tzpc",
		.id		= 2,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip4_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP4_TZPC2,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "tzpc",
		.id		= 3,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip4_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP4_TZPC3,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "seckey",
		.id		= -1,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip4_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP4_SECKEY,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "iem_apc",
		.id		= -1,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip4_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP4_IEM_APC,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "iem_iec",
		.id		= -1,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip4_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP4_IEM_IEC,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "chip_id",
		.id		= -1,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip4_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP4_CHIP_ID,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}
};

static struct clk init_clocks[] = {
/*Enable: IP0*/
	{	
#if 0
		.name		= "csis",
		.id		= -1,
		.parent		= &clk_pclk_83.clk,
		.enable		= s5pv210_clk_ip0_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP0_CSIS,
		.powerDomain = ,
	}, {
		.name		= "ipc",
		.id		= -1,
		.parent		= &clk_hclk_166.clk,
		.enable		= s5pv210_clk_ip0_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP0_IPC,
		.powerDomain = ,
	}, {
		.name		= "rotator",
		.id		= -1,
		.parent		= &clk_hclk_166.clk,
		.enable		= s5pv210_clk_ip0_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP0_ROTATOR,
		.powerDomain = ,
	}, {n
		.name		= "jpeg",
		.id		= -1,
		.parent		= &clk_hclk_166.clk,
		.enable		= s5pv210_clk_ip5_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP5_JPEG, //S5P_CLKGATE_IP0_JPEG,
		.powerDomain = ,
	}, {
		.name		= "fimc2",
		.id		= -1,
		.parent		= &clk_hclk_166.clk,
		.enable		= s5pv210_clk_ip0_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP0_FIMC2,
		.powerDomain = S5PC110_POWER_DOMAIN_CAM,
	}, {
		.name		= "fimc1",
		.id		= -1,
		.parent		= &clk_hclk_166.clk,
		.enable		= s5pv210_clk_ip0_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP0_FIMC1,
		.powerDomain = S5PC110_POWER_DOMAIN_CAM,
	}, {
		.name		= "fimc0",
		.id		= -1,
		.parent		= &clk_hclk_166.clk,
		.enable		= s5pv210_clk_ip0_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP0_FIMC0,
		.powerDomain = S5PC110_POWER_DOMAIN_CAM,
	}, {
		.name		= "g2d",
		.id			= -1,
		.parent		= &clk_hclk_166.clk,
		.enable		= s5pv210_clk_ip0_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP0_G2D,
		.powerDomain = S5PC110_POWER_DOMAIN_LCD,
	}, {
#endif
		.name		= "g3d",
		.id		= -1,
		.parent		= &clk_hclk_200.clk,
		.enable		= s5pv210_clk_ip0_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP0_G3D,
		.powerDomain = S5PC110_POWER_DOMAIN_G3D,
		.gb		= &gb_g3d,
	}, {
		.name		= "imem",
		.id		= -1,
		.parent		= &clk_pclk_100.clk,
		.enable		= s5pv210_clk_ip0_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP0_IMEM,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "pdma1",
		.id		= -1,
		.parent		= &clk_hclk_133.clk,
		.enable		= s5pv210_clk_ip0_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP0_PDMA1,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "pdma0",
		.id		= -1,
		.parent		= &clk_hclk_133.clk,
		.enable		= s5pv210_clk_ip0_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP0_PDMA0,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "mdma",
		.id		= -1,
		.parent		= &clk_hclk_166.clk,
		.enable		= s5pv210_clk_ip0_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP0_MDMA,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "dmc1",
		.id		= -1,
		.parent		= &clk_hclk_200.clk,
		.enable		= s5pv210_clk_ip0_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP0_DMC1,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "dmc0",
		.id		= -1,
		.parent		= &clk_hclk_200.clk,
		.enable		= s5pv210_clk_ip0_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP0_DMC0,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {

/*Enable: IP1*/
//		.name		= "nfcon",
//		.id		= -1,
//		.parent		= &clk_hclk_133.clk,
//		.enable		= s5pv210_clk_ip1_ctrl,
//		.ctrlbit	= S5P_CLKGATE_IP1_NFCON,
//		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
//	}, {
		.name		= "sromc",
		.id		= -1,
		.parent		= &clk_hclk_133.clk,
		.enable		= s5pv210_clk_ip1_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP1_SROMC,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
#if 0
		.name		= "cfcon",
		.id		= -1,
		.parent		= &clk_hclk_133.clk,
		.enable		= s5pv210_clk_ip1_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP1_CFCON,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
#endif
		.name		= "nandxl",
		.id		= -1,
		.parent		= &clk_hclk_133.clk,
		.enable		= s5pv210_clk_ip1_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP1_NANDXL,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
		.gb		= &gb_memory,
	}, {
//		.name		= "usbhost",
//		.id		= -1,
//		.parent		= &clk_hclk_133.clk,
//		.enable		= s5pv210_clk_ip1_ctrl,
//		.ctrlbit	= S5P_CLKGATE_IP1_USBHOST,
//		.powerDomain = 
//	}, {
//		.name		= "usbotg",
//		.id		= -1,
//		.parent		= &clk_hclk_133.clk,
//		.enable		= s5pv210_clk_ip1_ctrl,
//		.ctrlbit	= S5P_CLKGATE_IP1_USBOTG,
//		.powerDomain = 
//	}, {
//		.name		= "hdmi",
//		.id		= -1,
//		.parent		= &clk_hclk_166.clk,
//		.enable		= s5pv210_clk_ip1_ctrl,
//		.ctrlbit	= S5P_CLKGATE_IP1_HDMI,
//		.powerDomain = 
//	}, {
//		.name		= "tvenc",
//		.id		= -1,
//		.parent		= &clk_hclk_166.clk,
//		.enable		= s5pv210_clk_ip1_ctrl,
//		.ctrlbit	= S5P_CLKGATE_IP1_TVENC,
//		.powerDomain = 
//	}, {
//		.name		= "mixer",
//		.id		= -1,
//		.parent		= &clk_hclk_166.clk,
//		.enable		= s5pv210_clk_ip1_ctrl,
//		.ctrlbit	= S5P_CLKGATE_IP1_MIXER,
//		.powerDomain = 
//	}, {
//		.name		= "vp",
//		.id		= -1,
//		.parent		= &clk_hclk_166.clk,
//		.enable		= s5pv210_clk_ip1_ctrl,
//		.ctrlbit	= S5P_CLKGATE_IP1_VP,
//		.powerDomain = 
//	}, {
//		.name		= "dsim",
//		.id		= -1,
//		.parent		= &clk_hclk_166.clk,
//		.enable		= s5pv210_clk_ip1_ctrl,
//		.ctrlbit	= S5P_CLKGATE_IP1_DSIM,
//		.powerDomain = 
//	}, {
#ifdef CONFIG_FB_S3C_MDNIE
		.name		= "mdnie",
		.id		= -1,
		.parent		= &clk_hclk_166.clk,
		.enable		= s5pv210_clk_ip1_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP1_MIE,
		.powerDomain = S5PC110_POWER_DOMAIN_LCD,
	}, {
#endif
		.name		= "lcd",
		.id		= -1,
		.parent		= &clk_hclk_166.clk,
		.enable		= s5pv210_clk_ip1_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP1_FIMD,
		.powerDomain = S5PC110_POWER_DOMAIN_LCD,
		.srcMaskReg 	= S5P_CLK_SRC_MASK0,
		.srcMaskBit 	= S5P_CLKSRC_MASK0_FIMD,
	}, {
/*Enable: IP2*/
//		.name		= "tzic3",
//		.id		= -1,
//		.parent		= &clk_hclk_200.clk,
//		.enable		= s5pv210_clk_ip2_ctrl,
//		.ctrlbit	= S5P_CLKGATE_IP2_TZIC3,
//		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
//	}, {
//		.name		= "tzic2",
//		.id		= -1,
//		.parent		= &clk_hclk_200.clk,
//		.enable		= s5pv210_clk_ip2_ctrl,
//		.ctrlbit	= S5P_CLKGATE_IP2_TZIC2,
//		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
//	}, {
//		.name		= "tzic1",
//		.id		= -1,
//		.parent		= &clk_hclk_200.clk,
//		.enable		= s5pv210_clk_ip2_ctrl,
//		.ctrlbit	= S5P_CLKGATE_IP2_TZIC1,
//		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
//	}, {
//		.name		= "tzic0",
//		.id		= -1,
//		.parent		= &clk_hclk_200.clk,
//		.enable		= s5pv210_clk_ip1_ctrl,
//		.ctrlbit	= S5P_CLKGATE_IP2_TZIC0,
//		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
//	}, {
		.name		= "vic3",
		.id		= -1,
		.parent		= &clk_hclk_200.clk,
		.enable		= s5pv210_clk_ip2_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP2_VIC3,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "vic2",
		.id		= -1,
		.parent		= &clk_hclk_200.clk,
		.enable		= s5pv210_clk_ip2_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP2_VIC2,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "vic1",
		.id		= -1,
		.parent		= &clk_hclk_200.clk,
		.enable		= s5pv210_clk_ip2_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP2_VIC1,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "vic0",
		.id		= -1,
		.parent		= &clk_hclk_200.clk,
		.enable		= s5pv210_clk_ip2_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP2_VIC0,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
//		.name		= "tsi",
//		.id		= -1,
//		.parent		= &clk_pclk_66.clk,
//		.enable		= s5pv210_clk_ip2_ctrl,
//		.ctrlbit	= S5P_CLKGATE_IP2_TSI,
//		.powerDomain = 
//	}, {
//		.name		= "hsmmc",
//		.id		= 3,
//		.parent		= &clk_hclk_133.clk,
//		.enable		= s5pv210_clk_ip2_ctrl,
//		.ctrlbit	= S5P_CLKGATE_IP2_HSMMC3,
//		.powerDomain = 
//	}, {
//		.name		= "hsmmc",
//		.id		= 2,
//		.parent		= &clk_hclk_133.clk,
//		.enable		= s5pv210_clk_ip2_ctrl,
//		.ctrlbit	= S5P_CLKGATE_IP2_HSMMC2,
//		.powerDomain = 
//	}, {
//		.name		= "hsmmc",
//		.id		= 1,
//		.parent		= &clk_hclk_133.clk,
//		.enable		= s5pv210_clk_ip2_ctrl,
//		.ctrlbit	= S5P_CLKGATE_IP2_HSMMC1,
//		.powerDomain = 
//	}, {
//		.name		= "hsmmc",
//		.id		= 0,
//		.parent		= &clk_hclk_133.clk,
//		.enable		= s5pv210_clk_ip2_ctrl,
//		.ctrlbit	= S5P_CLKGATE_IP2_HSMMC0,
//		.powerDomain = 
//        }, {
//		.name		= "secjtag",
//		.id		= -1,
//		.parent		= &clk_hclk_133.clk,
//		.enable		= s5pv210_clk_ip2_ctrl,
//		.ctrlbit	= S5P_CLKGATE_IP2_SECJTAG,
//		.powerDomain = 
//        }, {
//		.name		= "hostif",
//		.id		= -1,
//		.parent		= &clk_hclk_133.clk,
//		.enable		= s5pv210_clk_ip2_ctrl,
//		.ctrlbit	= S5P_CLKGATE_IP2_HOSTIF,
//		.powerDomain = 
//        }, {
//		.name		= "modem",
//		.id		= -1,
//		.parent		= &clk_hclk_133.clk,
//		.enable		= s5pv210_clk_ip2_ctrl,
//		.ctrlbit	= S5P_CLKGATE_IP2_MODEM,
//		.powerDomain = 
//      }, {
		.name		= "coresight",
		.id		= -1,
		.parent		= &clk_hclk_200.clk,
		.enable		= s5pv210_clk_ip2_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP2_CORESIGHT,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
		.gb		= &gb_debug,
        }, {
//		.name		= "sdm",
//		.id		= -1,
//		.parent		= &clk_hclk_200.clk,
//		.enable		= s5pv210_clk_ip2_ctrl,
//		.ctrlbit	= S5P_CLKGATE_IP2_SDM,
//		.powerDomain = ,
//      }, {
//		.name		= "secss",
//		.id		= -1,
//		.parent		= &clk_hclk_133.clk,
//		.enable		= s5pv210_clk_ip2_ctrl,
//		.ctrlbit	= S5P_CLKGATE_IP2_SECSS,
//		.powerDomain = ,
//        }, {

/*Enable: IP3*/
#if 0
		.name		= "pcm",
		.id		= 2,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_PCM2,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "pcm",
		.id		= 1,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_PCM1,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "pcm",
		.id		= 0,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_PCM0,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
#endif
		.name		= "syscon",
		.id		= 0,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_SYSCON,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "gpio",
		.id		= 0,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_GPIO,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
#if 0
		.name		= "adc",
		.id		= -1,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_TSADC,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
#endif
		.name		= "timers",
		.id		= -1,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_PWM,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "watchdog",
		.id		= -1,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_WDT,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
#if 0
		.name		= "keypad",
		.id		= -1,
		.parent		 = &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_KEYIF,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
#endif
		.name		= "uart",
		.id		= 3,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_UART3,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
		.srcMaskReg 	= S5P_CLK_SRC_MASK0,
		.srcMaskBit 	= S5P_CLKSRC_MASK0_UART3,
	}, {
		.name		= "uart",
		.id		= 2,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_UART2,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
		.srcMaskReg 	= S5P_CLK_SRC_MASK0,
		.srcMaskBit 	= S5P_CLKSRC_MASK0_UART2,
	}, {
		.name		= "uart",
		.id		= 1,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_UART1,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
		.srcMaskReg 	= S5P_CLK_SRC_MASK0,
		.srcMaskBit 	= S5P_CLKSRC_MASK0_UART1,
	}, {
		.name		= "uart",
		.id		= 0,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_UART0,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
		.srcMaskReg 	= S5P_CLK_SRC_MASK0,
		.srcMaskBit 	= S5P_CLKSRC_MASK0_UART0,
#if 1
	}, {
		.name		= "systimer",
		.id		= -1,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_SYSTIMER,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
#endif
	}, {
		.name		= "rtc",
		.id		= -1,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_RTC,
		.powerDomain = S5PC110_POWER_DOMAIN_RTC,
#if 0
	}, {
		.name           = "spi",
		.id             = 2,
		.parent         = &clk_pclk_66.clk,
		.enable         = s5pv210_clk_ip3_ctrl,
		.ctrlbit        = S5P_CLKGATE_IP3_SPI2,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
        }, {
		.name           = "spi",
		.id             = 1,
		.parent         = &clk_pclk_66.clk,
		.enable         = s5pv210_clk_ip3_ctrl,
		.ctrlbit        = S5P_CLKGATE_IP3_SPI1,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
        }, {
		.name           = "spi",
		.id             = 0,
		.parent         = &clk_pclk_66.clk,
		.enable         = s5pv210_clk_ip3_ctrl,
		.ctrlbit        = S5P_CLKGATE_IP3_SPI0,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
        }, {
		.name		= "i2c",
		.id			= 3,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_I2C_HDMI_PHY,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "i2c",
		.id			= 2,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_I2C2,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "i2c",
		.id			= 1,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_I2C_HDMI_DDC,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "i2c",
		.id			= 0,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_I2C0,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
#endif		
	}, {
		.name		= "i2s_v50",
		.id		= 0,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_I2S0 | S5P_CLKGATE_IP3_PCM0, /* I2S0 is v5.0 */
		.powerDomain 	= S5PC110_POWER_DOMAIN_AUDIO,
		.srcMaskReg 	= S5P_CLK_SRC_MASK0,
		.srcMaskBit 	= S5P_CLKSRC_MASK0_AUDIO0,	
#if 0	
	}, {
		.name		= "i2s_v32",
		.id		= 0,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_I2S1 | S5P_CLKGATE_IP3_PCM1, /* I2S1 is v3.2 */
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "i2s_v32",
		.id		= 1,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_I2S2, /* I2S2 is v3.2 */
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "ac97",
		.id		= 1,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_AC97,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "spdif",
		.id		= -1,
		.parent		= NULL,
		.enable		= s5pv210_clk_ip3_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP3_SPDIF | S5P_CLKGATE_IP3_PCM0 | S5P_CLKGATE_IP3_PCM1 | S5P_CLKGATE_IP3_I2S0 | S5P_CLKGATE_IP3_I2S1 | S5P_CLKGATE_IP3_I2S2,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
#endif
/*Enable: IP4*/
#if 0
		.name		= "tzpc",
		.id		= 0,
		.parent		= &clk_pclk_100.clk,
		.enable		= s5pv210_clk_ip4_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP4_TZPC0,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "tzpc",
		.id		= 1,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip4_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP4_TZPC1,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "tzpc",
		.id		= 2,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip4_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP4_TZPC2,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "tzpc",
		.id		= 3,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip4_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP4_TZPC3,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
#endif
//		.name		= "seckey",
//		.id		= -1,
//		.parent		= &clk_pclk_66.clk,
//		.enable		= s5pv210_clk_ip4_ctrl,
//		.ctrlbit	= S5P_CLKGATE_IP4_SECKEY,
//		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
//	}, {
#if 0
		.name		= "iem_apc",
		.id		= -1,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip4_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP4_IEM_APC,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "iem_iec",
		.id		= -1,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip4_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP4_IEM_IEC,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
	}, {
		.name		= "chip_id",
		.id		= -1,
		.parent		= &clk_pclk_66.clk,
		.enable		= s5pv210_clk_ip4_ctrl,
		.ctrlbit	= S5P_CLKGATE_IP4_CHIP_ID,
		.powerDomain = S5PC110_POWER_DOMAIN_TOP,
#endif
	}
};


static struct clksrc_clk *sys_clksrc[] = {
	&clk_mout_apll,
	&clk_mout_epll,
	&clk_mout_mpll,
	&clk_armclk,
	&clk_xusbxti,
	&clk_mout_166,
	&clk_sclk_a2m,
	&clk_hclk_200,
	&clk_pclk_100,
	&clk_hclk_166,
	&clk_pclk_83,
	&clk_hclk_133,
	&clk_pclk_66,
	&clk_vpllsrc,
	&clk_mout_vpll,
	&clk_sclk_dac,
	&clk_sclk_pixel,
	&clk_sclk_hdmi,
	&clk_mout_cam0,
	&clk_mout_cam1,
	&clk_mout_fimd,
	&clk_mout_mmc0,
	&clk_mout_mmc1,
	&clk_mout_mmc2,
	&clk_mout_mmc3,
	&clk_sclk_mmc0,
	&clk_sclk_mmc1,
	&clk_sclk_mmc2,
	&clk_sclk_mmc3,
	&clk_mout_audio0,
	&clk_sclk_audio0,
	&clk_mout_audio1,
	&clk_sclk_audio1,
	&clk_mout_audio2,
	&clk_sclk_audio2,
	&clk_mout_fimc_lclk0,
	&clk_mout_fimc_lclk1,
	&clk_mout_fimc_lclk2,
	&clk_mout_csis,
	&clk_mout_spi0,
	&clk_mout_spi1,
	&clk_mout_pwi,
	&clk_mout_uart0,
	&clk_mout_uart1,
	&clk_mout_uart2,
	&clk_mout_uart3,
	&clk_mout_pwm,
	&clk_mout_mdnie,
	&clk_mout_mdnie_pwm,
	&clk_copy,
	&clk_mout_i2s_a,
	&clk_mout_audss,
	&clk_dout_audio_bus_clk_i2s,
};



static int s5pv210_epll_enable(struct clk *clk, int enable)
{
	unsigned int ctrlbit = clk->ctrlbit;
	unsigned int epll_con = __raw_readl(S5P_EPLL_CON) & ~ctrlbit;

	if (enable)
		__raw_writel(epll_con | ctrlbit, S5P_EPLL_CON);
	//else
	//	__raw_writel(epll_con, S5P_EPLL_CON);

	return 0;
}

static unsigned long s5pv210_epll_get_rate(struct clk *clk)
{
	return clk->rate;
}

static u32 epll_div[][6] = {
	{  48000000, 0, 48, 3, 3, 0 },
	{  96000000, 0, 48, 3, 2, 0 },
	{ 144000000, 1, 72, 3, 2, 0 },
	{ 192000000, 0, 48, 3, 1, 0 },
	{ 288000000, 1, 72, 3, 1, 0 },
	{  32750000, 1, 65, 3, 4, 35127 },
	{  32768000, 1, 65, 3, 4, 35127 },
	{  45158400, 0, 45, 3, 3, 10355 },
	{  45000000, 0, 45, 3, 3, 10355 },
	{  45158000, 0, 45, 3, 3, 10355 },
	{  49125000, 0, 49, 3, 3, 9961 },
	{  49152000, 0, 49, 3, 3, 9961 },
	{  67737600, 1, 67, 3, 3, 48366 },
	{  67738000, 1, 67, 3, 3, 48366 },
	{  73800000, 1, 73, 3, 3, 47710 },
	{  73728000, 1, 73, 3, 3, 47710 },
	{  36000000, 1, 32, 3, 4, 0 },
	{  60000000, 1, 60, 3, 3, 0 },
	{  72000000, 1, 72, 3, 3, 0 },
	{  80000000, 1, 80, 3, 3, 0 },
	{  84000000, 0, 42, 3, 2, 0 },
	{  50000000, 0, 50, 3, 3, 0 },
};

static int s5pv210_epll_set_rate(struct clk *clk, unsigned long rate)
{
	unsigned int epll_con, epll_con_k;
	unsigned int i;
#if 0 //epll clock getting changed durning suspend-resume without this function
	/* Return if nothing changed */
	if (clk->rate == rate)
		return 0;
#endif
	epll_con = __raw_readl(S5P_EPLL_CON);
	epll_con_k = __raw_readl(S5P_EPLL_CON_K);

	epll_con_k &= ~(PLL90XX_KDIV_MASK);
	epll_con &= ~(PLL90XX_MDIV_MASK << PLL90XX_MDIV_SHIFT |   \
			PLL90XX_PDIV_MASK << PLL90XX_PDIV_SHIFT | \
			PLL90XX_VDIV_MASK << PLL90XX_VDIV_SHIFT | \
			PLL90XX_SDIV_MASK << PLL90XX_SDIV_SHIFT);

	for (i = 0; i < ARRAY_SIZE(epll_div); i++) {
		if (epll_div[i][0] == rate) {
			epll_con_k |= epll_div[i][5] << 0;
			epll_con |= epll_div[i][1] << 27;
			epll_con |= epll_div[i][2] << 16;
			epll_con |= epll_div[i][3] << 8;
			epll_con |= epll_div[i][4] << 0;
			break;
		}
	}

	if (i == ARRAY_SIZE(epll_div)) {
		printk(KERN_ERR "%s: Invalid Clock EPLL Frequency\n",
				__func__);
		return -EINVAL;
	}

	__raw_writel(epll_con, S5P_EPLL_CON);
	__raw_writel(epll_con_k, S5P_EPLL_CON_K);

	clk->rate = rate;

	return 0;
}

static struct clk_ops s5pv210_epll_ops = {
	.get_rate = s5pv210_epll_get_rate,
	.set_rate = s5pv210_epll_set_rate,
};


#ifdef CONFIG_CPU_FREQ_LOG
void print_clocks(void) 
{
	struct clk *xtal_clk;
	unsigned long xtal;
	unsigned long armclk;
	unsigned long hclk200;
	unsigned long hclk166;
	unsigned long hclk133;
	unsigned long pclk100;
	unsigned long pclk83;
	unsigned long pclk66;
	unsigned long apll;
	unsigned long mpll;
	unsigned long epll;
	//unsigned int ptr;
	u32 clkdiv0, clkdiv1;

	clkdiv0 = __raw_readl(S5P_CLK_DIV0);
	clkdiv1 = __raw_readl(S5P_CLK_DIV1);

	xtal_clk = clk_get(NULL, "xtal");
	BUG_ON(IS_ERR(xtal_clk));

	xtal = clk_get_rate(xtal_clk);
	clk_put(xtal_clk);

	apll = s5p_get_pll45xx(xtal, __raw_readl(S5P_APLL_CON), pll_4508);
	mpll = s5p_get_pll45xx(xtal, __raw_readl(S5P_MPLL_CON), pll_4502);
	epll = s5p_get_pll90xx(xtal, __raw_readl(S5P_EPLL_CON),
			__raw_readl(S5P_EPLL_CON_K));


	armclk = apll / GET_DIV(clkdiv0, S5P_CLKDIV0_APLL);
	if(__raw_readl(S5P_CLK_SRC0)&(1<<S5P_CLKSRC0_MUX200_SHIFT)) {
		hclk200 = mpll / GET_DIV(clkdiv0, S5P_CLKDIV0_HCLK200);
	} else {
		hclk200 = armclk / GET_DIV(clkdiv0, S5P_CLKDIV0_HCLK200);
	}
	if(__raw_readl(S5P_CLK_SRC0)&(1<<S5P_CLKSRC0_MUX166_SHIFT)) {
		hclk166 = apll / GET_DIV(clkdiv0, S5P_CLKDIV0_A2M);
		hclk166 = hclk166 / GET_DIV(clkdiv0, S5P_CLKDIV0_HCLK166);
	} else {
		hclk166 = mpll / GET_DIV(clkdiv0, S5P_CLKDIV0_HCLK166);
	}
	if(__raw_readl(S5P_CLK_SRC0)&(1<<S5P_CLKSRC0_MUX133_SHIFT)) {
		hclk133 = apll / GET_DIV(clkdiv0, S5P_CLKDIV0_A2M);
		hclk133 = hclk133 / GET_DIV(clkdiv0, S5P_CLKDIV0_HCLK133);
	} else {
		hclk133 = mpll / GET_DIV(clkdiv0, S5P_CLKDIV0_HCLK133);
	}

	pclk100 = hclk200 / GET_DIV(clkdiv0, S5P_CLKDIV0_PCLK100);
	pclk83 = hclk166 / GET_DIV(clkdiv0, S5P_CLKDIV0_PCLK83);
	pclk66 = hclk133 / GET_DIV(clkdiv0, S5P_CLKDIV0_PCLK66);

	printk("S5PC110: ARMCLK=%ld, HCLKM=%ld, HCLKD=%ld, HCLKP=%ld, PCLKM=%ld, PCLKD=%ld, PCLKP=%ld\n",
	       armclk, hclk200, hclk166, hclk133, pclk100, pclk83, pclk66);

}
#endif

#define GET_DIV(clk, field) ((((clk) & field##_MASK) >> field##_SHIFT) + 1)

void __init_or_cpufreq s5pv210_setup_clocks(void)
{
	struct clk *xtal_clk;
	unsigned long xtal;
	unsigned long apll;
	unsigned long mpll;
	unsigned long epll;
	u32 clkdiv0, clkdiv1;
	struct clk *clk_mmc;
	struct clk *pSclk_mdnie;

	clk_fout_epll.enable = s5pv210_epll_enable;
	clk_fout_epll.ops = &s5pv210_epll_ops;

	printk(KERN_DEBUG "%s: registering clocks\n", __func__);

	clkdiv0 = __raw_readl(S5P_CLK_DIV0);
	clkdiv1 = __raw_readl(S5P_CLK_DIV1);

	printk(KERN_DEBUG "%s: clkdiv0 = %08x, clkdiv1 = %08x\n",
				__func__, clkdiv0, clkdiv1);

	xtal_clk = clk_get(NULL, "xtal");
	BUG_ON(IS_ERR(xtal_clk));

	xtal = clk_get_rate(xtal_clk);
	clk_put(xtal_clk);

	printk(KERN_DEBUG "%s: xtal is %ld\n", __func__, xtal);

	apll = s5p_get_pll45xx(xtal, __raw_readl(S5P_APLL_CON), pll_4508);
	mpll = s5p_get_pll45xx(xtal, __raw_readl(S5P_MPLL_CON), pll_4502);
	epll = s5p_get_pll90xx(xtal, __raw_readl(S5P_EPLL_CON),
			__raw_readl(S5P_EPLL_CON_K));

	printk(KERN_INFO "S5PV210: PLL settings, A=%ld, M=%ld, E=%ld\n",
			apll, mpll, epll);

	clk_fout_apll.rate = apll;
	clk_fout_mpll.rate = mpll;
	clk_fout_epll.rate = epll;

	clk_f.rate = clk_get_rate(&clk_armclk.clk);
	clk_h.rate = clk_get_rate(&clk_hclk_133.clk);
	clk_p.rate = clk_get_rate(&clk_pclk_66.clk);

	clk_set_parent(&clk_mout_mmc0.clk, &clk_mout_mpll.clk);
	clk_set_parent(&clk_mout_mmc1.clk, &clk_mout_mpll.clk);
	clk_set_parent(&clk_mout_mmc2.clk, &clk_mout_mpll.clk);
	clk_set_parent(&clk_mout_mmc3.clk, &clk_mout_mpll.clk);

	clk_set_parent(&clk_mout_spi0.clk, &clk_mout_epll.clk);
	clk_set_parent(&clk_mout_spi1.clk, &clk_mout_epll.clk);
	//clk_set_parent(&clk_spi2.clk, &clk_mout_epll.clk);  /spi2 is not exist

	clk_set_parent(&clk_mout_uart0.clk, &clk_mout_mpll.clk);
	clk_set_parent(&clk_mout_uart1.clk, &clk_mout_mpll.clk);
	clk_set_parent(&clk_mout_uart2.clk, &clk_mout_mpll.clk);
	clk_set_parent(&clk_mout_uart3.clk, &clk_mout_mpll.clk);
	clk_set_parent(&clk_mout_fimd.clk, &clk_mout_mpll.clk);
	clk_set_parent(&clk_mout_mdnie.clk, &clk_mout_mpll.clk);

	clk_set_rate(&clk_sclk_mmc0.clk, 52*MHZ);
	clk_set_rate(&clk_sclk_mmc1.clk, 50*MHZ);
	clk_set_rate(&clk_sclk_mmc2.clk, 50*MHZ);
	clk_set_rate(&clk_sclk_mmc3.clk, 50*MHZ);
	writel((readl(S5P_CLK_DIV4) & ~(0xffff0000)) | 0x44440000, S5P_CLK_DIV4);

	//clk_set_rate(&clk_mout_fimd.clk, 167*MHZ); //, &clk_lcd.clk

	pSclk_mdnie = clk_get(NULL, "sclk_mdnie");
	BUG_ON(IS_ERR(pSclk_mdnie));
	clk_set_rate(pSclk_mdnie, 167*MHZ);  //clk_mdnie_sel.clk
}

static struct clk *clks[] __initdata = {
	&clk_sclk_usbphy0,
	&clk_sclk_usbphy1,
	&clk_sclk_hdmiphy,
	&clk_pcmcdclk0,
	&clk_pcmcdclk1,
	&clk_i2scdclk0,
	&xclk_out,
};

void __init s5pv210_register_clocks(void)
{
	struct clk *clkp;
	int ret;
	int ptr;

	ret = s3c24xx_register_clocks(clks, ARRAY_SIZE(clks));
	if (ret > 0)
		printk(KERN_ERR "Failed to register %u clocks\n", ret);

	s3c_register_clksrc(clksrcs, ARRAY_SIZE(clksrcs));
	s3c_register_clocks(init_clocks, ARRAY_SIZE(init_clocks));

	for (ptr = 0; ptr < ARRAY_SIZE(sys_clksrc); ptr++)
		s3c_register_clksrc(sys_clksrc[ptr], 1);

	clkp = init_clocks_disable;
	for (ptr = 0; ptr < ARRAY_SIZE(init_clocks_disable); ptr++, clkp++) {
		ret = s3c24xx_register_clock(clkp);
		if (ret < 0) {
			printk(KERN_ERR "Failed to register clock %s (%d)\n",
			       clkp->name, ret);
		}
		(clkp->enable)(clkp, 0);
	}

	s3c_pwmclk_init();
}
