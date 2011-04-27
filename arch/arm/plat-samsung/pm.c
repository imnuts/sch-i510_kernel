/* linux/arch/arm/plat-s3c/pm.c
 *
 * Copyright 2008 Openmoko, Inc.
 * Copyright 2004-2008 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *	http://armlinux.simtec.co.uk/
 *
 * S3C common power management (suspend to ram) support.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/init.h>
#include <linux/suspend.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/serial_core.h>
#include <linux/io.h>
#include <linux/gpio.h>

#include <asm/cacheflush.h>
#include <mach/hardware.h>
#include <mach/map.h>

#include <plat/regs-serial.h>
#include <mach/regs-power.h>
#include <mach/regs-clock.h>
#include <mach/regs-irq.h>
#include <asm/irq.h>

#include <plat/pm.h>
#include <mach/pm-core.h>
#include <mach/regs-gpio.h>
#include <mach/gpio-bank.h>
#include <linux/sec_jack.h>

#include <plat/gpio-cfg.h>

#if defined(CONFIG_MACH_S5PC110_P1)
int g_pm_wakeup_stat=PM_WAKEUP_NONE;
#endif // CONFIG_MACH_S5PC110_P1


#define USE_DMA_ALLOC

#ifdef USE_DMA_ALLOC
#include <linux/dma-mapping.h>

static unsigned long *regs_save;
static dma_addr_t phy_regs_save;
#endif /* USE_DMA_ALLOC */

/* for external use */

unsigned long s3c_pm_flags;
extern void s3c_config_sleep_gpio(void);

//#define DBG(fmt...) printk(fmt)
#define DBG(fmt...)

/* Debug code:
 *
 * This code supports debug output to the low level UARTs for use on
 * resume before the console layer is available.
*/

#ifdef CONFIG_SAMSUNG_PM_DEBUG
extern void printascii(const char *);

void s3c_pm_dbg(const char *fmt, ...)
{
	va_list va;
	char buff[256];

	va_start(va, fmt);
	vsprintf(buff, fmt, va);
	va_end(va);

	printascii(buff);
}

static inline void s3c_pm_debug_init(void)
{
	/* restart uart clocks so we can use them to output */
	s3c_pm_debug_init_uart();
}

#else
#define s3c_pm_debug_init() do { } while(0)

#endif /* CONFIG_SAMSUNG_PM_DEBUG */

/* Save the UART configurations if we are configured for debug. */

unsigned char pm_uart_udivslot;

#define ENABLE_UART_SAVE_RESTORE
#ifdef ENABLE_UART_SAVE_RESTORE//CONFIG_SAMSUNG_PM_DEBUG

struct pm_uart_save uart_save[CONFIG_SERIAL_SAMSUNG_UARTS];

static void s3c_pm_save_uart(unsigned int uart, struct pm_uart_save *save)
{
	void __iomem *regs = S3C_VA_UARTx(uart);

	save->ulcon = __raw_readl(regs + S3C2410_ULCON);
	save->ucon = __raw_readl(regs + S3C2410_UCON);
	save->ufcon = __raw_readl(regs + S3C2410_UFCON);
	save->umcon = __raw_readl(regs + S3C2410_UMCON);
	save->ubrdiv = __raw_readl(regs + S3C2410_UBRDIV);

	if (pm_uart_udivslot)
		save->udivslot = __raw_readl(regs + S3C2443_DIVSLOT);

	//S3C_PMDBG("UART[%d]: ULCON=%04x, UCON=%04x, UFCON=%04x, UBRDIV=%04x\n",
		  //uart, save->ulcon, save->ucon, save->ufcon, save->ubrdiv);
}

static void s3c_pm_save_uarts(void)
{
	struct pm_uart_save *save = uart_save;
	unsigned int uart;

	for (uart = 0; uart < CONFIG_SERIAL_SAMSUNG_UARTS; uart++, save++)
		s3c_pm_save_uart(uart, save);
}

static void s3c_pm_restore_uart(unsigned int uart, struct pm_uart_save *save)
{
	void __iomem *regs = S3C_VA_UARTx(uart);

	s3c_pm_arch_update_uart(regs, save);

	__raw_writel(save->ulcon, regs + S3C2410_ULCON);
	__raw_writel(save->ucon,  regs + S3C2410_UCON);
	__raw_writel(save->ufcon, regs + S3C2410_UFCON);
	__raw_writel(save->umcon, regs + S3C2410_UMCON);
	__raw_writel(save->ubrdiv, regs + S3C2410_UBRDIV);

	if (pm_uart_udivslot)
		__raw_writel(save->udivslot, regs + S3C2443_DIVSLOT);
}

static void s3c_pm_restore_uarts(void)
{
	struct pm_uart_save *save = uart_save;
	unsigned int uart;

	for (uart = 0; uart < CONFIG_SERIAL_SAMSUNG_UARTS; uart++, save++)
		s3c_pm_restore_uart(uart, save);
}
#else
static void s3c_pm_save_uarts(void) { }
static void s3c_pm_restore_uarts(void) { }
#endif

/* The IRQ ext-int code goes here, it is too small to currently bother
 * with its own file. */

unsigned long s3c_irqwake_intmask	= 0xffffffffL;
unsigned long s3c_irqwake_eintmask	= 0xffffffffL;

int s3c_irqext_wake(unsigned int irqno, unsigned int state)
{
	unsigned long bit = 1L << IRQ_EINT_BIT(irqno);

	if (!(s3c_irqwake_eintallow & bit))
		return -ENOENT;

	printk(KERN_INFO "wake %s for irq %d\n",
	       state ? "enabled" : "disabled", irqno);

	if (!state)
		s3c_irqwake_eintmask |= bit;
	else
		s3c_irqwake_eintmask &= ~bit;

	return 0;
}
/////////////////////////////////

#if 1

#define eint_offset(irq)                (irq)
#define eint_irq_to_bit(irq)            (1 << (eint_offset(irq) & 0x7))
#define eint_conf_reg(irq)              ((eint_offset(irq)) >> 3)
#define eint_filt_reg(irq)              ((eint_offset(irq)) >> 2)
#define eint_mask_reg(irq)              ((eint_offset(irq)) >> 3)
#define eint_pend_reg(irq)              ((eint_offset(irq)) >> 3)

// intr_mode 0x2=>falling edge, 0x3=>rising dege, 0x4=>Both edge
static void s3c_pm_set_eint(unsigned int irq, unsigned int intr_mode)
{
	int offs = (irq);
	int shift;
	u32 ctrl, mask, tmp;
	//u32 newvalue = 0x2; // Falling edge

	shift = (offs & 0x7) * 4;
	if((0 <= offs) && (offs < 8)){
		tmp = readl(S5PV210_GPH0CON);
		tmp |= (0xf << shift);
		writel(tmp , S5PV210_GPH0CON);
		/*pull up disable*/
	}
	else if((8 <= offs) && (offs < 16)){
		tmp = readl(S5PV210_GPH1CON);
		tmp |= (0xf << shift);
		writel(tmp , S5PV210_GPH1CON);
	}
	else if((16 <= offs) && (offs < 24)){
		tmp = readl(S5PV210_GPH2CON);
		tmp |= (0xf << shift);
		writel(tmp , S5PV210_GPH2CON);
	}
	else if((24 <= offs) && (offs < 32)){
		tmp = readl(S5PV210_GPH3CON);
		tmp |= (0xf << shift);
		writel(tmp , S5PV210_GPH3CON);
	}
	else{
		printk(KERN_ERR "No such irq number %d", offs);
		return;
	}

	/*special handling for keypad eint*/
	if( (24 <= irq) && (irq <= 27))
	{// disable the pull up
		tmp = readl(S5PV210_GPH3PUD);
		tmp &= ~(0x3 << ((offs & 0x7) * 2));	
		writel(tmp, S5PV210_GPH3PUD);
		DBG("S5PV210_GPH3PUD = %x\n",readl(S5PV210_GPH3PUD));
	}
	

	/*Set irq type*/
	mask = 0x7 << shift;
	ctrl = readl(S5PV210_EINTCON(eint_conf_reg(irq)));
	ctrl &= ~mask;
	//ctrl |= newvalue << shift;
	ctrl |= intr_mode << shift;

	writel(ctrl, S5PV210_EINTCON(eint_conf_reg(irq)));
	/*clear mask*/
	mask = readl(S5PV210_EINTMASK(eint_mask_reg(irq)));
	mask &= ~(eint_irq_to_bit(irq));
	writel(mask, S5PV210_EINTMASK(eint_mask_reg(irq)));

	/*clear pending*/
	mask = readl(S5PV210_EINTPEND(eint_pend_reg(irq)));
	mask &= (eint_irq_to_bit(irq));
	writel(mask, S5PV210_EINTPEND(eint_pend_reg(irq)));
	
	/*Enable wake up mask*/
	tmp = readl(S5P_EINT_WAKEUP_MASK);
	tmp &= ~(1 << (irq));
	writel(tmp , S5P_EINT_WAKEUP_MASK);

	DBG("S5PV210_EINTCON = %x\n",readl(S5PV210_EINTCON(eint_conf_reg(irq))));
	DBG("S5PV210_EINTMASK = %x\n",readl(S5PV210_EINTMASK(eint_mask_reg(irq))));
	DBG("S5PV210_EINTPEND = %x\n",readl(S5PV210_EINTPEND(eint_pend_reg(irq))));
	
	return;
}

static void s3c_pm_clear_eint(unsigned int irq)
{
	u32 mask;
	/*clear pending*/
	mask = readl(S5PV210_EINTPEND(eint_pend_reg(irq)));
	mask &= (eint_irq_to_bit(irq));
	writel(mask, S5PV210_EINTPEND(eint_pend_reg(irq)));
}
#endif








//////////////////////////////////



/* helper functions to save and restore register state */

/**
 * s3c_pm_do_save() - save a set of registers for restoration on resume.
 * @ptr: Pointer to an array of registers.
 * @count: Size of the ptr array.
 *
 * Run through the list of registers given, saving their contents in the
 * array for later restoration when we wakeup.
 */
void s3c_pm_do_save(struct sleep_save *ptr, int count)
{
	for (; count > 0; count--, ptr++) {
		ptr->val = __raw_readl(ptr->reg);
	//	S3C_PMDBG("saved %p value %08lx\n", ptr->reg, ptr->val);
	}
}

/**
 * s3c_pm_do_restore() - restore register values from the save list.
 * @ptr: Pointer to an array of registers.
 * @count: Size of the ptr array.
 *
 * Restore the register values saved from s3c_pm_do_save().
 *
 * Note, we do not use S3C_PMDBG() in here, as the system may not have
 * restore the UARTs state yet
*/

void s3c_pm_do_restore(struct sleep_save *ptr, int count)
{
	for (; count > 0; count--, ptr++) {
		//printk(KERN_DEBUG "restore %p (restore %08lx, was %08x)\n",
		  //     ptr->reg, ptr->val, __raw_readl(ptr->reg));

		__raw_writel(ptr->val, ptr->reg);
	}
}

/**
 * s3c_pm_do_restore_core() - early restore register values from save list.
 *
 * This is similar to s3c_pm_do_restore() except we try and minimise the
 * side effects of the function in case registers that hardware might need
 * to work has been restored.
 *
 * WARNING: Do not put any debug in here that may effect memory or use
 * peripherals, as things may be changing!
*/

void s3c_pm_do_restore_core(struct sleep_save *ptr, int count)
{
	for (; count > 0; count--, ptr++)
		__raw_writel(ptr->val, ptr->reg);
}

/* s3c2410_pm_show_resume_irqs
 *
 * print any IRQs asserted at resume time (ie, we woke from)
*/
static void s3c_pm_show_resume_irqs(int start, unsigned long which,
				    unsigned long mask)
{
	int i;

	which &= ~mask;

	for (i = 0; i <= 31; i++) {
		if (which & (1L<<i)) {
			S3C_PMDBG("IRQ %d asserted at resume\n", start+i);
		}
	}
}


#if defined(CONFIG_MACH_S5PC110_P1)
/* s3c_pm_set_wakeup_stat
 *
 * MIDAS 2010/05/27
 * Set Wakeup stat from sleep
*/
void s3c_pm_set_wakeup_stat()
{
	if(!gpio_get_value(GPIO_nPOWER))
	{
		g_pm_wakeup_stat = PM_POWER_KEY;
	}
}

/* s3c_pm_set_wakeup_stat
 *
 * MIDAS 2010/05/27
 * Get Wakeup stat
*/
int s3c_pm_get_wakeup_stat()
{
	return g_pm_wakeup_stat;
}

/* s3c_pm_set_wakeup_stat
 *
 * MIDAS 2010/05/27
 * Clear Wakeup stat
*/
void s3c_pm_clear_wakeup_stat()
{
	g_pm_wakeup_stat = PM_WAKEUP_NONE;
}

EXPORT_SYMBOL(s3c_pm_get_wakeup_stat);
EXPORT_SYMBOL(s3c_pm_clear_wakeup_stat);
#endif // CONFIG_MACH_S5PC110_P1


void (*pm_cpu_prep)(void);
void (*pm_cpu_sleep)(void);

#define any_allowed(mask, allow) (((mask) & (allow)) != (allow))

extern short gp2a_get_proximity_enable(void); 

/* s3c_pm_enter
 *
 * central control for sleep/resume process
*/
extern unsigned int keywakeup_is_enabled;
static int s3c_pm_enter(suspend_state_t state)
{
#ifndef USE_DMA_ALLOC
	static unsigned long regs_save[16];
#endif /* !USE_DMA_ALLOC */
	unsigned int tmp,audiodomain_On;

	/* ensure the debug is initialised (if enabled) */
 #if 1
   s3c_gpio_cfgpin(GPIO_HDMI_EN1, S3C_GPIO_OUTPUT);  
   s3c_gpio_setpin(GPIO_HDMI_EN1, 1);            
   s3c_gpio_cfgpin(GPIO_HDMI_EN2, S3C_GPIO_OUTPUT);
   s3c_gpio_setpin(GPIO_HDMI_EN2, 1);
#endif
	s3c_pm_debug_init();

	S3C_PMDBG("%s(%d)\n", __func__, state);

	if (pm_cpu_prep == NULL || pm_cpu_sleep == NULL) {
		printk(KERN_ERR "%s: error: no cpu sleep function\n", __func__);
		return -EINVAL;
	}

	/* check if we have anything to wake-up with... bad things seem
	 * to happen if you suspend with no wakeup (system will often
	 * require a full power-cycle)
	*/
		s3c_irqwake_intmask = 0xFFFD; // rtc_alarm

	if (!any_allowed(s3c_irqwake_intmask, s3c_irqwake_intallow) &&
	    !any_allowed(s3c_irqwake_eintmask, s3c_irqwake_eintallow)) {
		printk(KERN_ERR "%s: No wake-up sources!\n", __func__);
		printk(KERN_ERR "%s: Aborting sleep\n", __func__);
		return -EINVAL;
	}

	/* store the physical address of the register recovery block */

#ifndef USE_DMA_ALLOC
	s3c_sleep_save_phys = virt_to_phys(regs_save);
#else
	__raw_writel(phy_regs_save, S5P_INFORM2);
#endif /* !USE_DMA_ALLOC */

	/* set flag for sleep mode idle2 flag is also reserved */
	__raw_writel(SLEEP_MODE, S5P_INFORM1);

	S3C_PMDBG("s3c_sleep_save_phys=0x%08lx\n", s3c_sleep_save_phys);

	/* save all necessary core registers not covered by the drivers */

	s3c_pm_save_gpios();
	s3c_pm_save_uarts();
	s3c_pm_save_core();

	s3c_config_sleep_gpio();

	/* set the irq configuration for wake */

	s3c_pm_configure_extint();

	S3C_PMDBG("sleep: irq wakeup masks: %08lx,%08lx\n",
			s3c_irqwake_intmask, s3c_irqwake_eintmask);

	/*Set EINT as wake up source*/
#if defined(CONFIG_OPTICAL_GP2A)
	if(gp2a_get_proximity_enable())
	{
		s3c_pm_set_eint(2, 0x4); // Proximity
	}
#endif
	s3c_pm_set_eint( 6, 0x4); // det_3.5
	s3c_pm_set_eint( 7, 0x2); // pmic
	s3c_pm_set_eint( 8, 0x2); // dpram
	s3c_pm_set_eint(11, 0x2); // onedram
	s3c_pm_set_eint(20, 0x3); // wifi
	s3c_pm_set_eint(13, 0x3); // HDMI-HPD
	s3c_pm_set_eint(21, 0x4); // bt
	s3c_pm_set_eint(22, 0x2); // power key
	s3c_pm_set_eint(23, 0x2); // microusb
	s3c_pm_set_eint(28, 0x4); // T_FLASH_DETECT
	s3c_pm_set_eint(29, 0x4); // ok key
	s3c_pm_set_eint(30, 0x4); // sendend

    if( keywakeup_is_enabled ) {
        s3c_pm_set_eint(24, 0x4); // volume up
        s3c_pm_set_eint(25, 0x4); // volume down (KBR1)
  
	    printk("keywakeup is enabled\n");
    }

	//s3c_pm_arch_prepare_irqs();
	

	/* call cpu specific preparation */

	pm_cpu_prep();

	/* flush cache back to ram */

	flush_cache_all();

	s3c_pm_check_store();

	__raw_writel(s3c_irqwake_intmask, S5P_WAKEUP_MASK); //0xFFDD:key, RTC_ALARM	
	

	/*clear for next wakeup*/
	tmp = __raw_readl(S5P_WAKEUP_STAT);
	__raw_writel(tmp, S5P_WAKEUP_STAT);

	//s3c_config_sleep_gpio();

	// Enable PS_HOLD pin to avoid reset failure */
	__raw_writel((0x5 << 12 | 0x1<<9 | 0x1<<8 | 0x1<<0),S5P_PSHOLD_CONTROL);


	/* send the cpu to sleep... */

	s3c_pm_arch_stop_clocks();

	/* s3c_cpu_save will also act as our return point from when
	 * we resume as it saves its own register state and restores it
	 * during the resume.  */

	s3c_cpu_save(regs_save);

	/* restore the cpu state using the kernel's cpu init code. */

	cpu_init();
	

	/* restore the system state */

	s3c_pm_restore_core();

	/*Reset the uart registers*/
	__raw_writel(0x0, S3C24XX_VA_UART3+S3C2410_UCON);
	__raw_writel(0xf, S3C24XX_VA_UART3+S5P_UINTM);
	__raw_writel(0xf, S3C24XX_VA_UART3+S5P_UINTSP);
	__raw_writel(0xf, S3C24XX_VA_UART3+S5P_UINTP);
	__raw_writel(0x0, S3C24XX_VA_UART2+S3C2410_UCON);
	__raw_writel(0xf, S3C24XX_VA_UART2+S5P_UINTM);
	__raw_writel(0xf, S3C24XX_VA_UART2+S5P_UINTSP);
	__raw_writel(0xf, S3C24XX_VA_UART2+S5P_UINTP);
	__raw_writel(0x0, S3C24XX_VA_UART1+S3C2410_UCON);
	__raw_writel(0xf, S3C24XX_VA_UART1+S5P_UINTM);
	__raw_writel(0xf, S3C24XX_VA_UART1+S5P_UINTSP);
	__raw_writel(0xf, S3C24XX_VA_UART1+S5P_UINTP);
	__raw_writel(0x0, S3C24XX_VA_UART0+S3C2410_UCON);
	__raw_writel(0xf, S3C24XX_VA_UART0+S5P_UINTM);
	__raw_writel(0xf, S3C24XX_VA_UART0+S5P_UINTSP);
	__raw_writel(0xf, S3C24XX_VA_UART0+S5P_UINTP);

	s3c_pm_restore_uarts();
	s3c_pm_restore_gpios();

	tmp = readl(S5P_NORMAL_CFG);
	if(!(tmp & S5PC110_POWER_DOMAIN_AUDIO)) {
		tmp = tmp | S5PC110_POWER_DOMAIN_AUDIO;
		writel(tmp , S5P_NORMAL_CFG);
		audiodomain_On = 1;
	} else {
		audiodomain_On = 0;
	}

	/* enable gpio, uart, mmc */
	tmp = __raw_readl(S5P_OTHERS);
	tmp |= (1<<31) | (1<<30) | (1<<28) | (1<<29);
	__raw_writel(tmp, S5P_OTHERS);

	tmp = readl(S5P_NORMAL_CFG);
	if (audiodomain_On) {
		tmp = tmp & ~S5PC110_POWER_DOMAIN_AUDIO;
		writel(tmp , S5P_NORMAL_CFG);
	}

	/*clear for next wakeup*/
	tmp = __raw_readl(S5P_WAKEUP_STAT);
	//printk("\nS5P_WAKEUP_STAT=%x\n",tmp);
	__raw_writel(tmp, S5P_WAKEUP_STAT);

	printk("wakeup source is 0x%x  \n", tmp);
	printk(" EXT_INT_0_PEND       %x \n", __raw_readl(S5PV210_EINTPEND(0)));
	printk(" EXT_INT_1_PEND       %x \n", __raw_readl(S5PV210_EINTPEND(1)));
	printk(" EXT_INT_2_PEND       %x \n", __raw_readl(S5PV210_EINTPEND(2)));
	printk(" EXT_INT_3_PEND       %x \n", __raw_readl(S5PV210_EINTPEND(3)));

	s3c_pm_clear_eint(21);
//	s3c_pm_clear_eint(22); // to be cleared later

	/* check what irq (if any) restored the system */
	s3c_pm_debug_init();

	s3c_pm_arch_show_resume_irqs();



#if defined(CONFIG_MACH_S5PC110_P1)
	// Set wakeup stat
	s3c_pm_set_wakeup_stat();
#endif // CONFIG_MACH_S5PC110_P1

	//printk("Int pending register before =%d\n",readl(S5PV210_EINTPEND(eint_pend_reg(22))));

	//printk("Int pending register after =%d\n",readl(S5PV210_EINTPEND(eint_pend_reg(22))));

	//S3C_PMDBG("%s: post sleep, preparing to return\n", __func__);
	//printk("%s: post sleep, preparing to return\n", __func__);

	/* LEDs should now be 1110 */
	//s3c_pm_debug_smdkled(1 << 1, 0);


	s3c_pm_check_restore();

	//mdelay(500);

	/* ok, let's return from sleep */
	printk(KERN_ERR "\n%s:%d\n", __func__, __LINE__);

	S3C_PMDBG("S3C PM Resume (post-restore)\n");
	return 0;
}

/* callback from assembly code */
void s3c_pm_cb_flushcache(void)
{
	flush_cache_all();
}

static int s3c_pm_prepare(void)
{
	/* prepare check area if configured */

	s3c_pm_check_prepare();
	return 0;
}

static void s3c_pm_finish(void)
{
	s3c_pm_check_cleanup();
}

static struct platform_suspend_ops s3c_pm_ops = {
	.enter		= s3c_pm_enter,
	.prepare	= s3c_pm_prepare,
	.finish		= s3c_pm_finish,
	.valid		= suspend_valid_only_mem,
};

/* s3c_pm_init
 *
 * Attach the power management functions. This should be called
 * from the board specific initialisation if the board supports
 * it.
*/

int __init s3c_pm_init(void)
{
	printk("S3C Power Management, Copyright 2004 Simtec Electronics\n");

#ifdef USE_DMA_ALLOC
	regs_save = dma_alloc_coherent(NULL, 4096, &phy_regs_save, GFP_KERNEL);
	if (regs_save == NULL) {
		printk(KERN_ERR "DMA alloc error\n");
		return -1;
	}
#endif /* USE_DMA_ALLOC */

	suspend_set_ops(&s3c_pm_ops);
	return 0;
}
