/* linux/arch/arm/mach-s5pv210/pm.c
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *              http://www.samsung.com/
 *
 * S5PV210 - PM support
 *
 * Based on arch/arm/mach-s3c2410/pm.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/init.h>
#include <linux/suspend.h>
#include <linux/errno.h>
#include <linux/time.h>
#include <linux/sysdev.h>
#include <linux/io.h>

#include <mach/hardware.h>

#include <asm/mach-types.h>

#include <mach/regs-gpio.h>
#include <plat/cpu.h>
#include <plat/pm.h>

#include <mach/regs-irq.h>
#include <mach/regs-clock.h>
#define DBG(fmt...) printk(KERN_DEBUG fmt)

void s5pv210_cpu_suspend(void)
{
	unsigned long tmp;
	//printk("\n %s called.........\n",__func__);

	/* issue the standby signal into the pm unit. Note, we
	 * issue a write-buffer drain just in case */

	tmp = 0;
/*
 * MCR p15,0,<Rd>,c7,c10,5 ; Data Memory Barrier Operation.
 * MCR p15,0,<Rd>,c7,c10,4 ; Data Synchronization Barrier operation.
 * MCR p15,0,<Rd>,c7,c0,4 ; Wait For Interrupt.
 */
#if defined(CONFIG_CPU_S5PV210_EVT0)
	tmp = __raw_readl(S5P_PWR_CFG);
	tmp &= S5P_CFG_WFI_CLEAN;
	__raw_writel(tmp, S5P_PWR_CFG);

	tmp = (1 << 2);
	__raw_writel(tmp, S5P_PWR_MODE);
#else

	asm("b 1f\n\t"
	    ".align 5\n\t"
	    "1:\n\t"
	    "mcr p15, 0, %0, c7, c10, 5\n\t"
	    "mcr p15, 0, %0, c7, c10, 4\n\t"
	    ".word 0xe320f003" : : "r" (tmp));

	/* we should never get past here */
#endif
	printk("\nsleep resumed to originator?..............\n");
	while(1);
	panic("sleep resumed to originator?");
}

static void s5pv210_pm_prepare(void)
{
	unsigned int tmp;

	/* ensure at least INFORM0 has the resume address */
	__raw_writel(virt_to_phys(s3c_cpu_resume), S5P_INFORM0);

	tmp = __raw_readl(S5P_SLEEP_CFG);
//	tmp &= ~(S5P_SLEEP_CFG_OSC_EN | S5P_SLEEP_CFG_USBOSC_EN);
	tmp &= ~(S5P_SLEEP_CFG_OSC_EN);		/* S5P_SLEEP_CFG_USBOSC_EN Control in WM8994.c */
	__raw_writel(tmp, S5P_SLEEP_CFG);

	/* WFI for SLEEP mode configuration by SYSCON */
	tmp = __raw_readl(S5P_PWR_CFG);
	tmp &= S5P_CFG_WFI_CLEAN;
	tmp |= S5P_CFG_WFI_SLEEP;
	__raw_writel(tmp, S5P_PWR_CFG);


#if 0	
	/* SYSCON interrupt handling disable */
	tmp = __raw_readl(S5P_OTHERS);
	tmp |= S5P_OTHER_SYSC_INTOFF;
	__raw_writel(tmp, S5P_OTHERS);
#endif

	__raw_writel(0xffffffff, S5P_VIC0REG(VIC_INT_ENABLE_CLEAR));
	__raw_writel(0xffffffff, S5P_VIC1REG(VIC_INT_ENABLE_CLEAR));
	__raw_writel(0xffffffff, S5P_VIC2REG(VIC_INT_ENABLE_CLEAR));
	__raw_writel(0xffffffff, S5P_VIC3REG(VIC_INT_ENABLE_CLEAR));
	__raw_writel(0xffffffff, S5P_VIC0REG(VIC_INT_SOFT_CLEAR));
	__raw_writel(0xffffffff, S5P_VIC1REG(VIC_INT_SOFT_CLEAR));
	__raw_writel(0xffffffff, S5P_VIC2REG(VIC_INT_SOFT_CLEAR));
	__raw_writel(0xffffffff, S5P_VIC3REG(VIC_INT_SOFT_CLEAR));

	/* SYSCON interrupt handling disable */
	tmp = __raw_readl(S5P_OTHERS);
	tmp |= S5P_OTHER_SYSC_INTOFF;
	__raw_writel(tmp, S5P_OTHERS);


	/* Enable RTC TICK,ALARM interrupt as a wakeup source */
//	tmp = __raw_readl(S5P_WAKEUP_MASK);
//	tmp &= ~(1 << 2);
//	__raw_writel(tmp, S5P_WAKEUP_MASK);

}

static int s5pv210_pm_add(struct sys_device *sysdev)
{
	pm_cpu_prep = s5pv210_pm_prepare;
	pm_cpu_sleep = s5pv210_cpu_suspend;

	return 0;
}

static struct sleep_save s5pv210_sleep[] = {

};

static int s5pv210_pm_resume(struct sys_device *dev)
{
	s3c_pm_do_restore(s5pv210_sleep, ARRAY_SIZE(s5pv210_sleep));
	return 0;
}

static struct sysdev_driver s5pv210_pm_driver = {
	.add		= s5pv210_pm_add,
	.resume		= s5pv210_pm_resume,
};

static __init int s5pv210_pm_drvinit(void)
{
	return sysdev_driver_register(&s5pv210_sysclass, &s5pv210_pm_driver);
}

arch_initcall(s5pv210_pm_drvinit);

