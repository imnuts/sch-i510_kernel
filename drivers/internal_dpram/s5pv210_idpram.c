/**
 *	S5PV210 Internal dpram specific settings
 */
#include <linux/module.h>
#include <mach/regs-gpio.h>
#include <mach/regs-mem.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/wakelock.h>
#include <linux/semaphore.h>
#include <linux/delay.h>

#include "dpram.h"

/*	S5PV210 Interanl Dpram Special Function Register 	*/
#define IDPRAM_MIFCON_INT2APEN      (1<<2)
#define IDPRAM_MIFCON_INT2MSMEN     (1<<3)
#define IDPRAM_MIFCON_DMATXREQEN_0  (1<<16)
#define IDPRAM_MIFCON_DMATXREQEN_1  (1<<17)
#define IDPRAM_MIFCON_DMARXREQEN_0  (1<<18)
#define IDPRAM_MIFCON_DMARXREQEN_1  (1<<19)
#define IDPRAM_MIFCON_FIXBIT        (1<<20)

#define IDPRAM_MIFPCON_ADM_MODE     (1<<6) // mux / demux mode

#define IDPRAM_DMA_ADR_MASK         0x3FFF
#define IDPRAM_DMA_TX_ADR_0         // shift 0
#define IDPRAM_DMA_TX_ADR_1         // shift 16
#define IDPRAM_DMA_RX_ADR_0         // shift 0
#define IDPRAM_DMA_RX_ADR_1         // shift 16

#define IRQ_DPRAM_AP_NIT_N IRQ_MSM

struct idpram_sfr_reg {
	unsigned int2ap;
	unsigned int2msm;
	unsigned mifcon;
	unsigned mifpcon;
	unsigned msmintclr;
	unsigned dma_tx_adr;
	unsigned dma_rx_adr;
};

/*	S5PV210 Internal Dpram GPIO table 	*/
struct idpram_gpio_data {
	unsigned num;
	unsigned cfg;
	unsigned pud;
	unsigned val;
};

enum idpram_pm_states {
	IDPRAM_PM_SUSPEND_PREPARE,
	IDPRAM_PM_DPRAM_POWER_DOWN,
	IDPRAM_PM_SUSPEND_START,
	IDPRAM_PM_RESUME_START,
	IDPRAM_PM_ACTIVE,
};

struct idpramctl {
	volatile void __iomem *idpram_base;
	volatile struct idpram_sfr_reg __iomem *idpram_sfr;
	atomic_t read_lock;
	atomic_t write_lock;
	struct wake_lock rd_wlock;
	struct wake_lock hold_wlock; /**/
	struct wake_lock wakeup_wlock; /**/
	struct completion complete_dpramdown;
	struct delayed_work resume_work;
	unsigned last_pm_mailbox; /* 0xCC or 0x CA*/
	unsigned resume_retry;
	unsigned pm_states;
};
struct idpramctl g_idpram;
//EXPORT_SYMBOL_GPL(g_idpram);

#define IDPRAM_ADDRESS_DEMUX 1
static struct idpram_gpio_data idpram_gpio_address[] = {
#ifdef IDPRAM_ADDRESS_DEMUX
	{
		.num = S5PV210_GPJ0(0),	/* MSM_ADDR 0 -12 */
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}, {
		.num = S5PV210_GPJ0(1),
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}, {
		.num = S5PV210_GPJ0(2),
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}, {
		.num = S5PV210_GPJ0(3),
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}, {
		.num = S5PV210_GPJ0(4),
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}, {
		.num = S5PV210_GPJ0(5),
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}, {
		.num = S5PV210_GPJ0(6),
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}, {
		.num = S5PV210_GPJ0(7),
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}, {
		.num = S5PV210_GPJ1(0),
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}, {
		.num = S5PV210_GPJ1(1),
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}, {
		.num = S5PV210_GPJ1(2),
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}, {
		.num = S5PV210_GPJ1(3),
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}, {
		.num = S5PV210_GPJ1(4),
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}, {
		.num = S5PV210_GPJ1(5),
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	},
#endif
};

static struct idpram_gpio_data idpram_gpio_data[] = {
	{
		.num = S5PV210_GPJ2(0), /* MSM_DATA 0 - 15 */
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}, {
		.num = S5PV210_GPJ2(1),
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}, {
		.num = S5PV210_GPJ2(2),
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}, {
		.num = S5PV210_GPJ2(3),
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}, {
		.num = S5PV210_GPJ2(4),
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}, {
		.num = S5PV210_GPJ2(5),
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}, {
		.num = S5PV210_GPJ2(6),
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}, {
		.num = S5PV210_GPJ2(7),
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}, {
		.num = S5PV210_GPJ3(0),
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}, {
		.num = S5PV210_GPJ3(1),
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}, {
		.num = S5PV210_GPJ3(2),
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}, {
		.num = S5PV210_GPJ3(3),
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}, {
		.num = S5PV210_GPJ3(4),
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}, {
		.num = S5PV210_GPJ3(5),
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}, {
		.num = S5PV210_GPJ3(6),
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}, {
		.num = S5PV210_GPJ3(7),
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	},
};

static struct idpram_gpio_data idpram_gpio_init_control[] = {
	{
		.num = S5PV210_GPJ4(0), /* MSM_CSn */
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}, {
		.num = S5PV210_GPJ4(1), /* MSM_WEn */
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}, {
		.num = S5PV210_GPJ4(2), /* MSM_Rn */
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}, {
		.num = S5PV210_GPJ4(3), /* MSM_IRQn */
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	},
#ifndef IDPRAM_ADDRESS_DEMUX
	{
		.num = S5PV210_GPJ4(4), /* MSM_ADVN */
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	},
#endif
};

static struct idpram_gpio_data idpram_gpio_deinit_control[] = {
	{
		.num = S5PV210_GPJ4(0), /* MSM_CSn */
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}, {
		.num = S5PV210_GPJ4(1), /* MSM_WEn */
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}, {
		.num = S5PV210_GPJ4(2), /* MSM_Rn */
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}, {
		.num = S5PV210_GPJ4(3), /* MSM_IRQn */
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	},
#ifndef IDPRAM_ADDRESS_DEMUX
	{
		.num = S5PV210_GPJ4(4), /* MSM_ADVN */
		.cfg = S3C_GPIO_SFN(0x2),
		.pud = S3C_GPIO_PULL_NONE,
	}
#endif
};

static void idpram_gpio_cfg(struct idpram_gpio_data *gpio)
{
	printk(KERN_DEBUG "idpram set gpio num=%d, cfg=%d, pud=%d, val=%d\n",
		gpio->num, gpio->cfg, gpio->pud, gpio->val);

	s3c_gpio_cfgpin(gpio->num, gpio->cfg);
	s3c_gpio_setpull(gpio->num, gpio->pud);
	if (gpio->val)
		gpio_set_value(gpio->num, gpio->val);
}

static void idpram_gpio_init(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(idpram_gpio_address); i++)
		idpram_gpio_cfg(&idpram_gpio_address[i]);

	for (i = 0; i < ARRAY_SIZE(idpram_gpio_data); i++)
		idpram_gpio_cfg(&idpram_gpio_data[i]);

	for (i = 0; i < ARRAY_SIZE(idpram_gpio_init_control); i++)
		idpram_gpio_cfg(&idpram_gpio_init_control[i]);
}

static void idpram_sfr_init(struct idpramctl *idpram)
{
	volatile struct idpram_sfr_reg __iomem *sfr = idpram->idpram_sfr;

	sfr->mifcon = (IDPRAM_MIFCON_FIXBIT | IDPRAM_MIFCON_INT2APEN |
		IDPRAM_MIFCON_INT2MSMEN);
#ifndef IDPRAM_ADDRESS_DEMUX
	sfr->mifpcon = (IDPRAM_MIFPCON_ADM_MODE);
#endif
}

//#define IDPRAM_INT_CLEAR()	idpram_sfr_int_clear(&g_sfr)
void _idpram_sfr_int_clear(struct idpramctl *idpram)
{
	volatile struct idpram_sfr_reg __iomem *sfr;
	sfr = idpram->idpram_sfr;
	sfr->msmintclr = 0xFF;
}

void idpram_int_clear(void)
{
	return _idpram_sfr_int_clear(&g_idpram);
}

/* HOLD Wake Lock*/
void idpram_wakelock_timeout(unsigned msec)
{
	printk(KERN_ERR "%s : %d\n", __func__, msec);
	wake_lock_timeout(&g_idpram.wakeup_wlock, msecs_to_jiffies(msec));
}

static int _idpram_resume_check(struct idpramctl *idpram);
static int _idpram_write_lock(struct idpramctl *idpram, int lock);
static void idpram_resume_retry(struct work_struct *work)
{
	struct idpramctl *idpram =
		container_of(work, struct idpramctl, resume_work.work);

	printk(KERN_INFO "%s\n", __func__);

	if (!_idpram_resume_check(idpram)) {
		printk(KERN_INFO "idpram resume ok\n");
		_idpram_write_lock(idpram, 0);
		wake_lock_timeout(&idpram->hold_wlock, msecs_to_jiffies(20));
		return;
	}
	if (idpram->resume_retry--) {
		schedule_delayed_work(&idpram->resume_work, msecs_to_jiffies(200));
		wake_lock_timeout(&idpram->hold_wlock, msecs_to_jiffies(260));
	}else {
		printk(KERN_INFO "idpram resume T-I-M-E-O-UT\n");
		idpram_timeout_handler();
		/* hold wakelock until uevnet sent to rild */
		wake_lock_timeout(&idpram->hold_wlock, HZ*7);
		_idpram_write_lock(idpram, 0);
	}
}

#define IDPRAM_SFR_PHYSICAL_ADDR 0xED008000
#define IDPRAM_SFR_SIZE 0x1C
static int _dpram_ex_init(struct idpramctl *idpram)
{
	volatile struct idpram_sfr_reg __iomem *sfr;
	struct clk *clk;

	wake_lock_init(&idpram->rd_wlock, WAKE_LOCK_SUSPEND, "dpram_pwrdn");
	wake_lock_init(&idpram->hold_wlock, WAKE_LOCK_SUSPEND, "dpram_hold");
	wake_lock_init(&idpram->wakeup_wlock, WAKE_LOCK_SUSPEND, "dpram_wakeup");
	atomic_set(&idpram->read_lock, 0);
	atomic_set(&idpram->write_lock, 0);
	INIT_DELAYED_WORK(&idpram->resume_work, idpram_resume_retry);

	/* enable internal dpram clock */
	clk = clk_get(NULL, "modem");
	if (!clk) {
		printk(KERN_ERR  "idpram failed to get clock %s\n", __func__);
		return -EFAULT;
	}
	clk_enable(clk);

	/* get sfr io-remap */
	sfr = (volatile struct idpram_sfr_reg __iomem *)
		ioremap_nocache(IDPRAM_SFR_PHYSICAL_ADDR, IDPRAM_SFR_SIZE);
	if (!sfr) {
	        printk(KERN_ERR "idpram_sfr_base io-remap fail\n");
		/*iounmap(idpram_base);*/
		return -EFAULT;
	}
	idpram->idpram_sfr = sfr;

	idpram_sfr_init(idpram);
	/**/dpram_clear();
	printk(KERN_ERR "dpram clear add\n");
	idpram_gpio_init();

	printk(KERN_ERR "idpram %s init done\n", __func__);
	//wake_lock(&idpram->hold_wlock);

	return 0;
}

static int _dpram_ex_deinit(struct idpramctl *idpram)
{
	wake_lock_destroy(&idpram->rd_wlock);
	wake_lock_destroy(&idpram->hold_wlock);
	wake_lock_destroy(&idpram->wakeup_wlock);
	return 0;
}

int dpram_ex_init(void)
{
	return _dpram_ex_init(&g_idpram);
}
int dpram_ex_deinit(void)
{
	return _dpram_ex_deinit(&g_idpram);
}

static int _idpram_read_lock(struct idpramctl *idpram, int lock)
{
	int lock_value;

	//printk(KERN_ERR "idpram read_lock(%d) - %s\n", lock,
	//	__builtin_return_address(0));
	printk(KERN_ERR "idpram read_lock(%d)\n", lock);
	switch (lock) {
	case 0:		/* unlock */
		if (atomic_read(&idpram->read_lock))
			lock_value = atomic_dec_return(&idpram->read_lock);
		wake_lock_timeout(&idpram->rd_wlock, HZ*6);
		if (lock_value)
			printk(KERN_ERR "ipdram unlock but lock value=%d\n",
				lock_value);
		break;
	case 1:		/* lock */
		if (!atomic_read(&idpram->read_lock)) {
			lock_value = atomic_inc_return(&idpram->read_lock);
			wake_unlock(&idpram->rd_wlock);
		}
		if (lock_value != 1)
			printk(KERN_ERR "ipdram lock but lock value=%d\n",
				lock_value);
		break;
	}
	return 0;
}

int idpram_read_lock(int lock)
{
	return _idpram_read_lock(&g_idpram, lock);
}

int _idpram_get_write_lock(struct idpramctl *idpram)
{
	return atomic_read(&idpram->write_lock);
}

static int _idpram_write_lock(struct idpramctl *idpram, int lock)
{
	int lock_value;

	//printk(KERN_ERR "idpram write_lock(%d) - %s\n", lock,
	//	__builtin_return_address(0));
	printk(KERN_ERR "idpram read_lock(%d)\n", lock);

	switch (lock) {
	case 0:		/* unlock */
		if (atomic_read(&idpram->write_lock))
			lock_value = atomic_dec_return(&idpram->write_lock);
		if (lock_value)
			printk(KERN_ERR
				"ipdram write unlock but lock value=%d\n",
				lock_value);
		break;
	case 1:		/* lock */
		if (!atomic_read(&idpram->write_lock))
			lock_value = atomic_inc_return(&idpram->write_lock);
		if (lock_value != 1)
			printk(KERN_ERR "ipdram write lock but lock value=%d\n",
				lock_value);
		break;
	}
	return 0;
}

int idpram_write_lock(int lock)
{
	return _idpram_write_lock(&g_idpram, lock);
}

int idpram_get_write_lock(void)
{
	return _idpram_get_write_lock(&g_idpram);
}

static int _idpram_pre_suspend(struct idpramctl *idpram)
{
	int timeout_ret = 0;
	int suspend_retry = 2;
	u16 intr_out = INT_COMMAND(INT_MASK_CMD_PDA_SLEEP);

	idpram->pm_states = IDPRAM_PM_SUSPEND_PREPARE;
	idpram->last_pm_mailbox = 0;
	_idpram_write_lock(idpram, 1);

	gpio_set_value(S5PV210_GPJ4(3), 1);

	if (!atomic_read(&idpram->read_lock)) {
		do {
			init_completion(&idpram->complete_dpramdown);
			/* force make falling edge */
			//s3c_gpio_cfgpin(S5PV210_GPJ4(3), S3C_GPIO_OUTPUT);
			//msleep(20);
			send_interrupt_to_phone(intr_out);
			//s3c_gpio_cfgpin(S5PV210_GPJ4(3), S3C_GPIO_SFN(0x2));

			printk(KERN_ERR "idpram sent PDA_SLEEP Mailbox(0x%x)\n",
				intr_out);
			timeout_ret =
			wait_for_completion_timeout(&idpram->complete_dpramdown,
				(HZ/5));
			printk(KERN_ERR "suspend_enter cnt = %d\n",
				suspend_retry);
		} while ( !timeout_ret && suspend_retry--);

		switch (idpram->last_pm_mailbox) {
		case INT_COMMAND(INT_MASK_CMD_DPRAM_DOWN):
			break;
		/* if nack or other interrup, hold wakelock for DPM resume*/
		case INT_COMMAND(INT_MASK_CMD_DPRAM_DOWN_NACK):
			printk(KERN_ERR "idpram dpram down get NACK\n");
		default:
			printk(KERN_ERR "CP dpram Down not ready! intr=0x%04X\n",
				read_phone_pda_maibox());
			wake_lock_timeout(&idpram->hold_wlock,
				msecs_to_jiffies(500));
			_idpram_write_lock(idpram, 0);
			/*idpram_timeout_handler(); */
			return 0;
		}
#if 0
		if (intr_in != INT_COMMAND(INT_MASK_CMD_DPRAM_DOWN)) {
			printk(KERN_ERR "T-I-M-E-O-U-T !!! intr=0x%04X\n",
				intr_in);
			wake_lock_timeout(&idpram->hold_wlock,
				msecs_to_jiffies(500));
			_idpram_write_lock(idpram, 0);
			/*idpram_timeout_handler(); */
			return 0;
		}
#endif
		/*
        	* Because, if dpram was powered down, cp dpram random intr was
		* ocurred. so, fixed by muxing cp dpram intr pin to GPIO output
		* high,..
        	*/
		gpio_set_value(S5PV210_GPJ4(3), 1);
		s3c_gpio_cfgpin(S5PV210_GPJ4(3), S3C_GPIO_OUTPUT);
		idpram->pm_states = IDPRAM_PM_DPRAM_POWER_DOWN;

		return 0;
	} else {
		printk(KERN_ERR "idpram hold read_lock\n");
		return -EBUSY;
	}
}

int idpram_pre_suspend(void)
{
	return _idpram_pre_suspend(&g_idpram);
}

int idpram_suspend(void)
{
	g_idpram.pm_states = IDPRAM_PM_SUSPEND_START;
}

static int _idpram_resume_init(struct idpramctl *idpram)
{
	const u16 magic_code = 0x00AA;
	const u16 acc_code = 0x0001;

	idpram->pm_states = IDPRAM_PM_RESUME_START;
	idpram->last_pm_mailbox = 0;

	dpram_clear();
	dpram_magickey_init();

	/* Initialize the dpram controller */
	idpram_sfr_init(idpram);
	/* idpram_gpio_init();  re-initialize internal dpram gpios */
	s3c_gpio_cfgpin(S5PV210_GPJ4(3), S3C_GPIO_SFN(0x2));
/*
	send_interrupt_to_phone(INT_COMMAND(INT_MASK_CMD_PDA_WAKEUP));
	printk(KERN_ERR "idpram sent PDA_WAKEUP Mailbox(0x%x)\n",
		INT_COMMAND(INT_MASK_CMD_PDA_WAKEUP));

	idpram->resume_retry = 5;
	schedule_delayed_work(&idpram->resume_work, msecs_to_jiffies(100));

	wake_lock_timeout(&idpram->hold_wlock, msecs_to_jiffies(600));
*/
	_idpram_write_lock(idpram, 0);
	return 0;
}

int idpram_resume_init(void)
{
	return _idpram_resume_init(&g_idpram);
}

static int _idpram_resume_check(struct idpramctl *idpram)
{
	/* check last pm mailbox */
	printk(KERN_INFO "idpram %s, last_pm_mailbox=%x\n", __func__,
		idpram->last_pm_mailbox);

	if(idpram->last_pm_mailbox == INT_COMMAND(INT_MASK_CMD_PDA_WAKEUP)) {
		idpram->last_pm_mailbox = 0;
		return 0;
	}

	/* PDA ACTIVE workaround  */
/*	gpio_set_value(GPIO_PDA_ACTIVE, GPIO_LEVEL_LOW);
	printk(KERN_INFO "idpram PDA_ACTIVE LOW\n");
	msleep(100);
	gpio_set_value(GPIO_PDA_ACTIVE, GPIO_LEVEL_HIGH);
	printk(KERN_INFO "idpram PDA_ACTIVE HIGH\n");
	msleep(50);
*/
	send_interrupt_to_phone(INT_COMMAND(INT_MASK_CMD_PDA_WAKEUP));
	printk(KERN_ERR "idpram sent PDA_WAKEUP Mailbox(0x%x)\n",
		INT_COMMAND(INT_MASK_CMD_PDA_WAKEUP));

	return -1;
}

static int _idpram_post_resume(struct idpramctl *idpram)
{
	printk(KERN_INFO "idpram %s \n", __func__);

	switch (idpram->pm_states) {
	/* schedule_work */
	case IDPRAM_PM_DPRAM_POWER_DOWN:
		gpio_set_value(GPIO_PDA_ACTIVE, GPIO_LEVEL_LOW);
		printk(KERN_INFO "idpram PDA_ACTIVE LOW\n");
		msleep(100);
		gpio_set_value(GPIO_PDA_ACTIVE, GPIO_LEVEL_HIGH);
		printk(KERN_INFO "idpram PDA_ACTIVE HIGH\n");
//		msleep(20);

		idpram_resume_init();
		break;

	case IDPRAM_PM_RESUME_START:
//		idpram->resume_retry = 3;
//		if (!work_pending(&idpram->resume_work.work))
//			schedule_delayed_work(&idpram->resume_work,
//				msecs_to_jiffies(0));
		break;

	/* do nothing */
	case IDPRAM_PM_SUSPEND_PREPARE:
		break;
	}
	return 0;
}
int idpram_post_resume(void)
{
	return _idpram_post_resume(&g_idpram);
}

static void _dpram_power_down(struct idpramctl *idpram)
{
//#ifdef PRINT_DPRAM_PWR_CTRL
//    LOGA("Received MBX_CMD_DPRAM_DOWN (lock count = %d)!!!\n",
//          dpram_get_lock_read());
//#endif
	idpram->last_pm_mailbox = INT_COMMAND(INT_MASK_CMD_DPRAM_DOWN);
	complete(&idpram->complete_dpramdown);
}

static void _dpram_power_down_nack(struct idpramctl *idpram)
{
	idpram->last_pm_mailbox = INT_COMMAND(INT_MASK_CMD_DPRAM_DOWN_NACK);
	complete(&idpram->complete_dpramdown);
}

static void _dpram_powerup_start(struct idpramctl *idpram)
{
	idpram->last_pm_mailbox = INT_COMMAND(INT_MASK_CMD_PDA_WAKEUP);
	/*idpram->last_pm_mailbox = read_phone_pda_maibox();*/
	idpram->pm_states = IDPRAM_PM_ACTIVE;
}

void dpram_power_down(void)
{
	return _dpram_power_down(&g_idpram);
}

void dpram_power_down_nack(void)
{
	return _dpram_power_down_nack(&g_idpram);
}

void dpram_powerup_start(void)
{
	return _dpram_powerup_start(&g_idpram);
}

