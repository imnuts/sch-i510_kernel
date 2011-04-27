/* linux/arch/arm/mach-s5pv210/gpio.c
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * S5PV210 - GPIOlib support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <mach/map.h>
#include <mach/gpio.h>
#include <mach/regs-gpio.h>
#include <plat/gpio-core.h>
#include <plat/gpio-cfg.h>
#include <plat/gpio-cfg-helpers.h>

static struct s3c_gpio_cfg gpio_cfg = {
	.cfg_eint	= 0xf,
	.set_config	= s3c_gpio_setcfg_s3c64xx_4bit,
	.set_pull	= s3c_gpio_setpull_updown,
	.get_pull	= s3c_gpio_getpull_updown,
	.set_pin	= s3c_gpio_setpin_updown,
};

static struct s3c_gpio_cfg gpio_cfg_noint = {
	.set_config	= s3c_gpio_setcfg_s3c64xx_4bit,
	.set_pull	= s3c_gpio_setpull_updown,
	.get_pull	= s3c_gpio_getpull_updown,
	.set_pin	= s3c_gpio_setpin_updown,
};

static struct s3c_gpio_chip s5pv210_gpio_4bit[] = {
	{
		.base	= S5PV210_GPA0_BASE,
		.config	= &gpio_cfg,
		.chip	= {
			.base	= S5PV210_GPA0(0),
			.ngpio	= S5PV210_GPIO_A0_NR,
			.label	= "GPA0",
		},
	}, {
		.base	= S5PV210_GPA1_BASE,
		.config	= &gpio_cfg,
		.chip	= {
			.base	= S5PV210_GPA1(0),
			.ngpio	= S5PV210_GPIO_A1_NR,
			.label	= "GPA1",
		},
	}, {
		.base	= S5PV210_GPB_BASE,
		.config	= &gpio_cfg,
		.chip	= {
			.base	= S5PV210_GPB(0),
			.ngpio	= S5PV210_GPIO_B_NR,
			.label	= "GPB",
		},
	}, {
		.base	= S5PV210_GPC0_BASE,
		.config	= &gpio_cfg,
		.chip	= {
			.base	= S5PV210_GPC0(0),
			.ngpio	= S5PV210_GPIO_C0_NR,
			.label	= "GPC0",
		},
	}, {
		.base	= S5PV210_GPC1_BASE,
		.config	= &gpio_cfg,
		.chip	= {
			.base	= S5PV210_GPC1(0),
			.ngpio	= S5PV210_GPIO_C1_NR,
			.label	= "GPC1",
		},
	}, {
		.base	= S5PV210_GPD0_BASE,
		.config	= &gpio_cfg,
		.chip	= {
			.base	= S5PV210_GPD0(0),
			.ngpio	= S5PV210_GPIO_D0_NR,
			.label	= "GPD0",
		},
	}, {
		.base	= S5PV210_GPD1_BASE,
		.config	= &gpio_cfg,
		.chip	= {
			.base	= S5PV210_GPD1(0),
			.ngpio	= S5PV210_GPIO_D1_NR,
			.label	= "GPD1",
		},
	}, {
		.base	= S5PV210_GPE0_BASE,
		.config	= &gpio_cfg,
		.chip	= {
			.base	= S5PV210_GPE0(0),
			.ngpio	= S5PV210_GPIO_E0_NR,
			.label	= "GPE0",
		},
	}, {
		.base	= S5PV210_GPE1_BASE,
		.config	= &gpio_cfg,
		.chip	= {
			.base	= S5PV210_GPE1(0),
			.ngpio	= S5PV210_GPIO_E1_NR,
			.label	= "GPE1",
		},
	}, {
		.base	= S5PV210_GPF0_BASE,
		.config	= &gpio_cfg,
		.chip	= {
			.base	= S5PV210_GPF0(0),
			.ngpio	= S5PV210_GPIO_F0_NR,
			.label	= "GPF0",
		},
	}, {
		.base	= S5PV210_GPF1_BASE,
		.config	= &gpio_cfg,
		.chip	= {
			.base	= S5PV210_GPF1(0),
			.ngpio	= S5PV210_GPIO_F1_NR,
			.label	= "GPF1",
		},
	}, {
		.base	= S5PV210_GPF2_BASE,
		.config	= &gpio_cfg,
		.chip	= {
			.base	= S5PV210_GPF2(0),
			.ngpio	= S5PV210_GPIO_F2_NR,
			.label	= "GPF2",
		},
	}, {
		.base	= S5PV210_GPF3_BASE,
		.config	= &gpio_cfg,
		.chip	= {
			.base	= S5PV210_GPF3(0),
			.ngpio	= S5PV210_GPIO_F3_NR,
			.label	= "GPF3",
		},
	}, {
		.base	= S5PV210_GPG0_BASE,
		.config	= &gpio_cfg,
		.chip	= {
			.base	= S5PV210_GPG0(0),
			.ngpio	= S5PV210_GPIO_G0_NR,
			.label	= "GPG0",
		},
	}, {
		.base	= S5PV210_GPG1_BASE,
		.config	= &gpio_cfg,
		.chip	= {
			.base	= S5PV210_GPG1(0),
			.ngpio	= S5PV210_GPIO_G1_NR,
			.label	= "GPG1",
		},
	}, {
		.base	= S5PV210_GPG2_BASE,
		.config	= &gpio_cfg,
		.chip	= {
			.base	= S5PV210_GPG2(0),
			.ngpio	= S5PV210_GPIO_G2_NR,
			.label	= "GPG2",
		},
	}, {
		.base	= S5PV210_GPG3_BASE,
		.config	= &gpio_cfg,
		.chip	= {
			.base	= S5PV210_GPG3(0),
			.ngpio	= S5PV210_GPIO_G3_NR,
			.label	= "GPG3",
		},
	}, {
		.base	= S5PV210_GPH0_BASE,
		.config	= &gpio_cfg_noint,
		.chip	= {
			.base	= S5PV210_GPH0(0),
			.ngpio	= S5PV210_GPIO_H0_NR,
			.label	= "GPH0",
		},
	}, {
		.base	= S5PV210_GPH1_BASE,
		.config	= &gpio_cfg_noint,
		.chip	= {
			.base	= S5PV210_GPH1(0),
			.ngpio	= S5PV210_GPIO_H1_NR,
			.label	= "GPH1",
		},
	}, {
		.base	= S5PV210_GPH2_BASE,
		.config	= &gpio_cfg_noint,
		.chip	= {
			.base	= S5PV210_GPH2(0),
			.ngpio	= S5PV210_GPIO_H2_NR,
			.label	= "GPH2",
		},
	}, {
		.base	= S5PV210_GPH3_BASE,
		.config	= &gpio_cfg_noint,
		.chip	= {
			.base	= S5PV210_GPH3(0),
			.ngpio	= S5PV210_GPIO_H3_NR,
			.label	= "GPH3",
		},
	}, {
		.base	= S5PV210_GPI_BASE,
		.config	= &gpio_cfg,
		.chip	= {
			.base	= S5PV210_GPI(0),
			.ngpio	= S5PV210_GPIO_I_NR,
			.label	= "GPI",
		},
	}, {
		.base	= S5PV210_GPJ0_BASE,
		.config	= &gpio_cfg,
		.chip	= {
			.base	= S5PV210_GPJ0(0),
			.ngpio	= S5PV210_GPIO_J0_NR,
			.label	= "GPJ0",
		},
	}, {
		.base	= S5PV210_GPJ1_BASE,
		.config	= &gpio_cfg,
		.chip	= {
			.base	= S5PV210_GPJ1(0),
			.ngpio	= S5PV210_GPIO_J1_NR,
			.label	= "GPJ1",
		},
	}, {
		.base	= S5PV210_GPJ2_BASE,
		.config	= &gpio_cfg,
		.chip	= {
			.base	= S5PV210_GPJ2(0),
			.ngpio	= S5PV210_GPIO_J2_NR,
			.label	= "GPJ2",
		},
	}, {
		.base	= S5PV210_GPJ3_BASE,
		.config	= &gpio_cfg,
		.chip	= {
			.base	= S5PV210_GPJ3(0),
			.ngpio	= S5PV210_GPIO_J3_NR,
			.label	= "GPJ3",
		},
	}, {
		.base	= S5PV210_GPJ4_BASE,
		.config	= &gpio_cfg,
		.chip	= {
			.base	= S5PV210_GPJ4(0),
			.ngpio	= S5PV210_GPIO_J4_NR,
			.label	= "GPJ4",
		},
	}, {
		.base	= S5PV210_MP01_BASE,
		.config	= &gpio_cfg_noint,
		.chip	= {
			.base	= S5PV210_MP01(0),
			.ngpio	= S5PV210_GPIO_MP01_NR,
			.label	= "MP01",
		},
	}, {
		.base	= S5PV210_MP02_BASE,
		.config	= &gpio_cfg_noint,
		.chip	= {
			.base	= S5PV210_MP02(0),
			.ngpio	= S5PV210_GPIO_MP02_NR,
			.label	= "MP02",
		},
	}, {
		.base	= S5PV210_MP03_BASE,
		.config	= &gpio_cfg_noint,
		.chip	= {
			.base	= S5PV210_MP03(0),
			.ngpio	= S5PV210_GPIO_MP03_NR,
			.label	= "MP03",
		},
	}, {
		.base	= S5PV210_MP04_BASE,
		.config	= &gpio_cfg_noint,
		.chip	= {
			.base	= S5PV210_MP04(0),
			.ngpio	= S5PV210_GPIO_MP04_NR,
			.label	= "MP04",
		},
	}, {
		.base	= S5PV210_MP05_BASE,
		.config	= &gpio_cfg_noint,
		.chip	= {
			.base	= S5PV210_MP05(0),
			.ngpio	= S5PV210_GPIO_MP05_NR,
			.label	= "MP05",
		},
	}, {
		.base	= S5PV210_MP06_BASE,
		.config	= &gpio_cfg_noint,
		.chip	= {
			.base	= S5PV210_MP06(0),
			.ngpio	= S5PV210_GPIO_MP06_NR,
			.label	= "MP06",
		},
	}, {
		.base	= S5PV210_MP07_BASE,
		.config	= &gpio_cfg_noint,
		.chip	= {
			.base	= S5PV210_MP07(0),
			.ngpio	= S5PV210_GPIO_MP07_NR,
			.label	= "MP07",
		},
	}, {
		.base	= S5PV210_MP10_BASE,
		.config	= &gpio_cfg_noint,
		.chip	= {
			.base	= S5PV210_MP10(0),
			.ngpio	= S5PV210_GPIO_MP10_NR,
			.label	= "MP10",
		},
	}, {
		.base	= S5PV210_MP11_BASE,
		.config	= &gpio_cfg_noint,
		.chip	= {
			.base	= S5PV210_MP11(0),
			.ngpio	= S5PV210_GPIO_MP11_NR,
			.label	= "MP11",
		},
	}, {
		.base	= S5PV210_MP12_BASE,
		.config	= &gpio_cfg_noint,
		.chip	= {
			.base	= S5PV210_MP12(0),
			.ngpio	= S5PV210_GPIO_MP12_NR,
			.label	= "MP12",
		},
	}, {
		.base	= S5PV210_MP13_BASE,
		.config	= &gpio_cfg_noint,
		.chip	= {
			.base	= S5PV210_MP13(0),
			.ngpio	= S5PV210_GPIO_MP13_NR,
			.label	= "MP13",
		},
	}, {
		.base	= S5PV210_MP14_BASE,
		.config	= &gpio_cfg_noint,
		.chip	= {
			.base	= S5PV210_MP14(0),
			.ngpio	= S5PV210_GPIO_MP14_NR,
			.label	= "MP14",
		},
	}, {
		.base	= S5PV210_MP15_BASE,
		.config	= &gpio_cfg_noint,
		.chip	= {
			.base	= S5PV210_MP15(0),
			.ngpio	= S5PV210_GPIO_MP15_NR,
			.label	= "MP15",
		},
	}, {
		.base	= S5PV210_MP16_BASE,
		.config	= &gpio_cfg_noint,
		.chip	= {
			.base	= S5PV210_MP16(0),
			.ngpio	= S5PV210_GPIO_MP16_NR,
			.label	= "MP16",
		},
	}, {
		.base	= S5PV210_MP17_BASE,
		.config	= &gpio_cfg_noint,
		.chip	= {
			.base	= S5PV210_MP17(0),
			.ngpio	= S5PV210_GPIO_MP17_NR,
			.label	= "MP17",
		},
	}, {
		.base	= S5PV210_MP18_BASE,
		.config	= &gpio_cfg_noint,
		.chip	= {
			.base	= S5PV210_MP18(0),
			.ngpio	= S5PV210_GPIO_MP18_NR,
			.label	= "MP18",
		},
	}, {
		.base	= S5PV210_MP20_BASE,
		.config	= &gpio_cfg_noint,
		.chip	= {
			.base	= S5PV210_MP20(0),
			.ngpio	= S5PV210_GPIO_MP20_NR,
			.label	= "MP20",
		},
	}, {
		.base	= S5PV210_MP21_BASE,
		.config	= &gpio_cfg_noint,
		.chip	= {
			.base	= S5PV210_MP21(0),
			.ngpio	= S5PV210_GPIO_MP21_NR,
			.label	= "MP21",
		},
	}, {
		.base	= S5PV210_MP22_BASE,
		.config	= &gpio_cfg_noint,
		.chip	= {
			.base	= S5PV210_MP22(0),
			.ngpio	= S5PV210_GPIO_MP22_NR,
			.label	= "MP22",
		},
	}, {
		.base	= S5PV210_MP23_BASE,
		.config	= &gpio_cfg_noint,
		.chip	= {
			.base	= S5PV210_MP23(0),
			.ngpio	= S5PV210_GPIO_MP23_NR,
			.label	= "MP23",
		},
	}, {
		.base	= S5PV210_MP24_BASE,
		.config	= &gpio_cfg_noint,
		.chip	= {
			.base	= S5PV210_MP24(0),
			.ngpio	= S5PV210_GPIO_MP24_NR,
			.label	= "MP24",
		},
	}, {
		.base	= S5PV210_MP25_BASE,
		.config	= &gpio_cfg_noint,
		.chip	= {
			.base	= S5PV210_MP25(0),
			.ngpio	= S5PV210_GPIO_MP25_NR,
			.label	= "MP25",
		},
	}, {
		.base	= S5PV210_MP26_BASE,
		.config	= &gpio_cfg_noint,
		.chip	= {
			.base	= S5PV210_MP26(0),
			.ngpio	= S5PV210_GPIO_MP26_NR,
			.label	= "MP26",
		},
	}, {
		.base	= S5PV210_MP27_BASE,
		.config	= &gpio_cfg_noint,
		.chip	= {
			.base	= S5PV210_MP27(0),
			.ngpio	= S5PV210_GPIO_MP27_NR,
			.label	= "MP27",
		},
	}, {
		.base	= S5PV210_MP28_BASE,
		.config	= &gpio_cfg_noint,
		.chip	= {
			.base	= S5PV210_MP28(0),
			.ngpio	= S5PV210_GPIO_MP28_NR,
			.label	= "MP28",
		},
	}, {
		.base	= S5PV210_ETC0_BASE,
		.config	= &gpio_cfg_noint,
		.chip	= {
			.base	= S5PV210_ETC0(0),
			.ngpio	= S5PV210_GPIO_ETC0_NR,
			.label	= "ETC0",
		},
	}, {
		.base	= S5PV210_ETC1_BASE,
		.config	= &gpio_cfg_noint,
		.chip	= {
			.base	= S5PV210_ETC1(0),
			.ngpio	= S5PV210_GPIO_ETC1_NR,
			.label	= "ETC1",
		},
	}, {
		.base	= S5PV210_ETC2_BASE,
		.config	= &gpio_cfg_noint,
		.chip	= {
			.base	= S5PV210_ETC2(0),
			.ngpio	= S5PV210_GPIO_ETC2_NR,
			.label	= "ETC2",
		},
	}, {
		.base	= S5PV210_ETC4_BASE,
		.config	= &gpio_cfg_noint,
		.chip	= {
			.base	= S5PV210_ETC4(0),
			.ngpio	= S5PV210_GPIO_ETC4_NR,
			.label	= "ETC4",
		},
	},
};

__init int s5pv210_gpiolib_init(void)
{
	samsung_gpiolib_add_4bit_chips(s5pv210_gpio_4bit,
				       ARRAY_SIZE(s5pv210_gpio_4bit));

	return 0;
}
