/* linux/arch/arm/plat-s3c/include/plat/clock.h
 *
 * Copyright (c) 2004-2005 Simtec Electronics
 *	http://www.simtec.co.uk/products/SWLINUX/
 *	Written by Ben Dooks, <ben@simtec.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/spinlock.h>

struct clk;

#if defined(CONFIG_MACH_S5PC110_P1)
#define USE_1DOT2GHZ 1
#else
#define USE_1DOT2GHZ 0
#endif

#if defined(CONFIG_CPU_S5PV210_EVT1)
struct clkGateBlock {
	unsigned long           gb_ctrlbit;
	unsigned long		gb_countIP;
	unsigned long		gb_CheckIPBits1;
	volatile unsigned long	*gb_CheckIPReg1;
	unsigned long           gb_CheckIPBits2;
	volatile unsigned long	*gb_CheckIPReg2;
};
#endif

/**
 * struct clk_ops - standard clock operations
 * @set_rate: set the clock rate, see clk_set_rate().
 * @get_rate: get the clock rate, see clk_get_rate().
 * @round_rate: round a given clock rate, see clk_round_rate().
 * @set_parent: set the clock's parent, see clk_set_parent().
 *
 * Group the common clock implementations together so that we
 * don't have to keep setting the same fiels again. We leave
 * enable in struct clk.
 *
 * Adding an extra layer of indirection into the process should
 * not be a problem as it is unlikely these operations are going
 * to need to be called quickly.
 */
struct clk_ops {
	int		    (*set_rate)(struct clk *c, unsigned long rate);
	unsigned long	    (*get_rate)(struct clk *c);
	unsigned long	    (*round_rate)(struct clk *c, unsigned long rate);
	int		    (*set_parent)(struct clk *c, struct clk *parent);
};

struct clk {
	struct list_head      list;
	struct module        *owner;
	struct clk           *parent;
	const char           *name;
	int		      id;
	int		      usage;
	unsigned long         rate;
	unsigned long         ctrlbit;
#if defined(CONFIG_CPU_S5PV210_EVT1)
	int		    (*enable)(struct clk *, int enable);
	int		    (*set_rate)(struct clk *c, unsigned long rate);
	unsigned long	    (*get_rate)(struct clk *c);
	unsigned long	    (*round_rate)(struct clk *c, unsigned long rate);
	int		    (*set_parent)(struct clk *c, struct clk *parent);
#endif
	struct clk_ops		*ops;

#if defined(CONFIG_CPU_S5PV210_EVT1)
	unsigned int powerDomain;
	unsigned long         srcMaskBit;
	void __iomem         *srcMaskReg;
	struct clkGateBlock	*gb;
#endif
};

/* other clocks which may be registered by board support */

extern struct clk s3c24xx_dclk0;
extern struct clk s3c24xx_dclk1;
extern struct clk s3c24xx_clkout0;
extern struct clk s3c24xx_clkout1;
extern struct clk s3c24xx_uclk;

extern struct clk clk_usb_bus;

/* core clock support */

extern struct clk clk_f;
extern struct clk clk_h;
extern struct clk clk_p;
extern struct clk clk_mpll;
extern struct clk clk_upll;
extern struct clk clk_epll;
extern struct clk clk_xtal;
extern struct clk clk_ext;

#ifdef CONFIG_CPU_S5PV210_EVT1
extern struct clk clk_vpll;
extern struct clk clk_h200;
extern struct clk clk_h166;
extern struct clk clk_h133;
extern struct clk clk_p100;
extern struct clk clk_p83;
extern struct clk clk_p66;
#endif

/* S3C64XX specific clocks */
extern struct clk clk_h2;
extern struct clk clk_27m;
extern struct clk clk_48m;

extern int clk_default_setrate(struct clk *clk, unsigned long rate);
extern struct clk_ops clk_ops_def_setrate;

/* exports for arch/arm/mach-s3c2410
 *
 * Please DO NOT use these outside of arch/arm/mach-s3c2410
*/

extern spinlock_t clocks_lock;

extern int s3c2410_clkcon_enable(struct clk *clk, int enable);

extern int s3c24xx_register_clock(struct clk *clk);
extern int s3c24xx_register_clocks(struct clk **clk, int nr_clks);

extern void s3c_register_clocks(struct clk *clk, int nr_clks);

extern int s3c24xx_register_baseclocks(unsigned long xtal);

extern void s5p_register_clocks(unsigned long xtal_freq);

extern void s3c24xx_setup_clocks(unsigned long fclk,
				 unsigned long hclk,
				 unsigned long pclk);

extern void s3c2410_setup_clocks(void);
extern void s3c2412_setup_clocks(void);
extern void s3c244x_setup_clocks(void);
extern void s3c2443_setup_clocks(void);

/* S3C64XX specific functions and clocks */

extern int s3c64xx_sclk_ctrl(struct clk *clk, int enable);

#if defined(CONFIG_CPU_S5PV210_EVT1)
extern void s5pc11x_register_clocks(void);
extern int s5pc11x_clk_ip0_ctrl(struct clk *clk, int enable);
extern int s5pc11x_clk_ip1_ctrl(struct clk *clk, int enable);
extern int s5pc11x_clk_ip2_ctrl(struct clk *clk, int enable);
extern int s5pc11x_clk_ip3_ctrl(struct clk *clk, int enable);
extern int s5pc11x_clk_ip4_ctrl(struct clk *clk, int enable);
extern int s5pc11x_clk_block_ctrl(struct clk *clk, int enable);
extern int s5pc11x_audss_clkctrl(struct clk *clk, int enable);
#endif

/* Init for pwm clock code */

extern void s3c_pwmclk_init(void);

