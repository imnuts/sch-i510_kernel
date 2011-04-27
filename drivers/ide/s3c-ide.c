/* linux/drivers/ide/s3c-ide.c
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *              http://www.samsung.com/
 *
 * Driver for Samsung SoC onboard IDEs.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/ide.h>
#include <linux/sysdev.h>
#include <linux/clk.h>
#include <linux/io.h>

#include <plat/regs-ide.h>
#include <plat/ide.h>

/*
 * defines ide controller data transfer commands
 */
enum {
	CMD_STOP,
	CMD_START,
	CMD_ABORT,
	CMD_CONTINUE
};

/*
 * defines the transfer class
 */
enum {
	XFER_MODE_PIO_CPU,
	XFER_MODE_PIO_DMA,
	XFER_MODE_MULTIWORD_DMA,
	XFER_MODE_UDMA
};

/*
 * defines the bus state
 */
enum {
	BS_IDLE,
	BS_BUSYW,
	BS_PREP,
	BS_BUSYR,
	BS_PAUSER,
	BS_PAUSEW,
	BS_PAUSER2
};

struct dma_queue_t {
	ulong addr; /* Used to block on state transitions */
	ulong len; /* Power Managers device structure */
};

/*
 * struct s3c_ide_device - instance of ide controller device
 */
struct s3c_ide_device {
	struct platform_device *pdev;
	struct clk *ide_clk;
	ide_hwif_t *hwif;
	int irq;
	ulong piotime[5];
	ulong udmatime[5];
	void __iomem *regbase;
	u32 index; /* current queue index */
	u32 queue_size;	/* total queue size requested */
	struct dma_queue_t table[PRD_ENTRIES];
	u32 dma_mode; /* in DMA session */
};

struct s3c_ide_device *s3c_ide_hwif;

static inline void ide_writel(struct s3c_ide_device *dev, u32 value, u32 reg)
{
	writel(value, dev->regbase + reg);
}

static inline u32 ide_readl(struct s3c_ide_device *dev, u32 reg)
{
	return readl(dev->regbase + reg);
}

/*
 * wait for a specified ide bus state
 */
static int wait_for_bus_state(struct s3c_ide_device *ide_dev, u8 state)
{
	u32 status, current_state;
	ulong timeout;

	timeout = jiffies + msecs_to_jiffies(1000);
	while (time_before(jiffies, timeout)) {
		status = ide_readl(ide_dev, S5P_BUS_FIFO_STATUS);
		current_state = (status >> 16) & 0x7;
		if (current_state == state) {
			if ((state == BS_PAUSER2) || (state == BS_IDLE)) {
				if (status & 0xFFFF)
					continue;
			}
		}
		return 0;
	}
	return -1;
}

/*
 * wait for a specified status of the ide disk status
 */
static int wait_for_disk_status(ide_drive_t *drive, u8 status)
{
	u8 csd;
	ulong timeout;
	struct s3c_ide_device *ide_dev = drive->hwif->hwif_data;

	timeout = jiffies + msecs_to_jiffies(1000);
	while (time_before(jiffies, timeout)) {
		csd = ide_readl(ide_dev, S5P_ATA_PIO_CSD);
		if ((csd == status) || (csd & ATA_ERR))
			return 0;
	}
	dev_err(&ide_dev->pdev->dev,
	"timeout occured while waiting for disk status");
	return -1;
}

static int wait_for_device_ready(ide_drive_t *drive, u8 status)
{
	u8 csd;
	ulong timeout;
	struct s3c_ide_device *ide_dev = drive->hwif->hwif_data;

	timeout = jiffies + msecs_to_jiffies(1000);
	while (time_before(jiffies, timeout)) {
		csd = ide_readl(ide_dev, S5P_ATA_PIO_CSD);
		if ((csd & status) == 0)
			return 0;
	}
	dev_err(&ide_dev->pdev->dev, "device is not ready\n");
	return -1;
}

/*
 * sets the data transfer mode
 */
static void set_xfer_mode(struct s3c_ide_device *ide_dev, u8 mode, int rw)
{
	u32 reg = ide_readl(ide_dev, S5P_ATA_CFG) & ~(0x39c);

	/* set mode */
	reg |= mode << 2;

	/* set DMA write mode */
	if (mode && rw)
		reg |= 0x10;

	/* set ATA DMA auto mode (enable multi block transfer) */
	if (mode == XFER_MODE_UDMA)
		reg |= 0x380;

	ide_writel(ide_dev, reg, S5P_ATA_CFG);
}

static void set_ata_enable(struct s3c_ide_device *ide_dev, u8 state)
{
	u32 temp = ide_readl(ide_dev, S5P_ATA_CTRL);
	temp = state ? temp | 1 : temp & ~1;
	ide_writel(ide_dev, temp , S5P_ATA_CTRL);
}

static void set_endian_mode(struct s3c_ide_device *ide_dev, u8 mode)
{
	u32 reg = ide_readl(ide_dev, S5P_ATA_CFG);
	reg = mode ? (reg & ~S5P_ATA_CFG_SWAP) : (reg | S5P_ATA_CFG_SWAP);
	ide_writel(ide_dev, reg, S5P_ATA_CFG);
}

static void set_xfer_command(struct s3c_ide_device *ide_dev, u8 cmd)
{
	ide_writel(ide_dev, cmd, S5P_ATA_CMD);
}

/*
 * This function selects the maximum possible transfer speed.
 */
static u8 ide_ratefilter(ide_drive_t *drive, u8 speed)
{
	if (drive->media != ide_disk)
		return min(speed, (u8) XFER_PIO_4);

	switch (speed) {
	case XFER_UDMA_6:
	case XFER_UDMA_5:
		/* S5PC1OO and S5PV210 can support upto UDMA4 */
		speed = XFER_UDMA_4;
		break;
	case XFER_UDMA_4:
	case XFER_UDMA_3:
	case XFER_UDMA_2:
	case XFER_UDMA_1:
	case XFER_UDMA_0:
		break;
	default:
		speed = min(speed, (u8) XFER_PIO_4);
		break;
	}
	return speed;
}

/*
 * This function selects the best possible transfer speed.
 */
static void s3c_ide_tune_chipset(ide_drive_t *drive, u8 xferspeed)
{
	ide_hwif_t *hwif = (ide_hwif_t *)drive->hwif;
	struct s3c_ide_device *ide_dev = hwif->hwif_data;
	u8 speed = ide_ratefilter(drive, xferspeed);
	u32 ata_cfg;

	/* IORDY is enabled for modes > PIO2 */
	if (XFER_PIO_0 <= speed && speed <= XFER_PIO_4) {
		ata_cfg = ide_readl(ide_dev, S5P_ATA_CFG);

		switch (speed) {
		case XFER_PIO_0:
		case XFER_PIO_1:
		case XFER_PIO_2:
			ata_cfg &= ~(S5P_ATA_CFG_IORDYEN);
			break;
		case XFER_PIO_3:
		case XFER_PIO_4:
			ata_cfg |= S5P_ATA_CFG_IORDYEN;
			break;
		}

		ide_writel(ide_dev, ata_cfg, S5P_ATA_CFG);
		ide_writel(ide_dev, ide_dev->piotime[speed - XFER_PIO_0],
			S5P_ATA_PIO_TIME);
	} else {
		ide_writel(ide_dev, ide_dev->piotime[0], S5P_ATA_PIO_TIME);
		ide_writel(ide_dev, ide_dev->udmatime[speed - XFER_UDMA_0],
				S5P_ATA_UDMA_TIME);
		set_endian_mode(ide_dev, 1);
	}
	ide_config_drive_speed(drive, speed);
}

static void s3c_ide_tune_drive(ide_drive_t *drive, u8 pio)
{
	pio = ide_get_best_pio_mode(drive, 255, pio);
	(void)s3c_ide_tune_chipset(drive, (XFER_PIO_0 + pio));
}

/* Building the Scatter Gather Table */
static int ide_build_dmatable(ide_drive_t *drive, struct ide_cmd *cmd)
{
	int i, count = 0, nents = cmd->sg_nents;
	ide_hwif_t *hwif = drive->hwif;
	struct request *rq = hwif->rq;
	struct s3c_ide_device *ide_dev = hwif->hwif_data;
	struct scatterlist *sg;
	u32 addr_reg, size_reg;

	if (rq_data_dir(rq) == WRITE) {
		addr_reg = S5P_ATA_SBUF_START;
		size_reg = S5P_ATA_SBUF_SIZE;
	} else {
		addr_reg = S5P_ATA_TBUF_START;
		size_reg = S5P_ATA_TBUF_SIZE;
	}

	/* save information for interrupt context */
	if (nents > 1)
		ide_dev->dma_mode = 1;
	if (!nents)
		return 0;

	/* fill the descriptors */
	sg = hwif->sg_table;
	for (i = 0, sg = hwif->sg_table; i < nents && sg_dma_len(sg);
				 i++, sg++) {
		ide_dev->table[i].addr = sg_dma_address(sg);
		ide_dev->table[i].len = sg_dma_len(sg);
		count += ide_dev->table[i].len;
	}
	ide_dev->table[i].addr = 0;
	ide_dev->table[i].len = 0;
	ide_dev->queue_size = i;

	ide_writel(ide_dev, ide_dev->table[0].len - 0x1, size_reg);
	ide_writel(ide_dev, ide_dev->table[0].addr, addr_reg);

	ide_dev->index = 1;

	ide_writel(ide_dev, count - 1, S5P_ATA_XFR_NUM);

	return 1;
}

/*
 * following are the ide_dma_ops functions implemented by the ide driver
 */
static int s3c_ide_dma_init(ide_hwif_t *hwif, const struct ide_port_info *d)
{
	return 0;
}

static void s3c_ide_dma_host_set(ide_drive_t *drive, int on)
{
}

static int s3c_ide_dma_setup(ide_drive_t *drive, struct ide_cmd *cmd)
{
	if (!ide_build_dmatable(drive, cmd))
		return 1;

	drive->waiting_for_dma = 1;
	return 0;
}

static void s3c_ide_dma_start(ide_drive_t *drive)
{
	struct request *rq = drive->hwif->rq;
	uint rw = (rq_data_dir(rq) == WRITE);
	struct s3c_ide_device *ide_dev = drive->hwif->hwif_data;

	wait_for_disk_status(drive, DRIVE_READY | ATA_DRQ);
	ide_writel(ide_dev, 0x3f, S5P_ATA_IRQ);
	ide_writel(ide_dev, 0x3f, S5P_ATA_IRQ_MSK);

	wait_for_device_ready(drive, ATA_BUSY);
	wait_for_bus_state(ide_dev, BS_IDLE);

	set_xfer_mode(ide_dev, XFER_MODE_UDMA, rw);
	set_xfer_command(ide_dev, CMD_START);
	return;
}

static int s3c_ide_dma_end(ide_drive_t *drive)
{
	struct s3c_ide_device *ide_dev = drive->hwif->hwif_data;

	if ((ide_readl(ide_dev, S5P_BUS_FIFO_STATUS) >> 16) == BS_PAUSEW)
		ide_writel(ide_dev, CMD_CONTINUE, S5P_ATA_CMD);

	if (wait_for_bus_state(ide_dev, BS_IDLE))
		return 1;

	ide_writel(ide_dev, 0x3f, S5P_ATA_IRQ_MSK);
	set_xfer_mode(ide_dev, XFER_MODE_PIO_CPU, 0);
	if (wait_for_disk_status(drive, DRIVE_READY))
		return 1;
	drive->waiting_for_dma = 0;
	return 0;
}

static int s3c_ide_dma_test_irq(ide_drive_t *drive)
{
	return 1;
}

static void s3c_ide_dma_lostirq(ide_drive_t *drive)
{
	struct s3c_ide_device *ide_dev = drive->hwif->hwif_data;
	printk(KERN_ERR "%08x, %08x\n", ide_readl(ide_dev, S5P_ATA_IRQ),
		ide_readl(ide_dev, S5P_ATA_IRQ_MSK));
	dev_err(&ide_dev->pdev->dev, "irq lost");
}

static irqreturn_t s3c_irq_handler(int irq, void *dev_id)
{
	ide_hwif_t *hwif = (ide_hwif_t *)dev_id;
	struct s3c_ide_device *ide_dev = hwif->hwif_data;
	u32 reg = ide_readl(ide_dev, S5P_ATA_IRQ);
	u32 len, addr, stat;
	ulong timeout;

	ide_writel(ide_dev, reg, S5P_ATA_IRQ);

	if (ide_dev->dma_mode) {
		len = ide_dev->table[ide_dev->index].len - 1;
		addr = ide_dev->table[ide_dev->index].addr;
		if (reg & 0x10) {
			wait_for_bus_state(ide_dev, BS_PAUSER2);
			ide_writel(ide_dev, len, S5P_ATA_SBUF_SIZE);
			ide_writel(ide_dev, addr, S5P_ATA_SBUF_START);
		} else if (reg & 0x08) {
			wait_for_bus_state(ide_dev, BS_PAUSEW);
			ide_writel(ide_dev, len, S5P_ATA_TBUF_SIZE);
			ide_writel(ide_dev, addr, S5P_ATA_TBUF_START);
		} else
			return 1;

		if (ide_dev->queue_size == ++ide_dev->index)
			ide_dev->dma_mode = 0;

		ide_writel(ide_dev, CMD_CONTINUE, S5P_ATA_CMD);
		return 1;
	}

	timeout = jiffies + msecs_to_jiffies(1000);
	while (time_before(jiffies, timeout)) {
		stat = ide_readl(ide_dev, S5P_BUS_FIFO_STATUS);
		if (stat == 0)
			break;
	}

	return ide_intr(irq, dev_id);
}

static void setup_timing_value(struct s3c_ide_device *ide_dev, u32 clk_rate)
{
	u32 t1, t2, teoc, i;
	u32 uTdvh1, uTdvs, uTrp, uTss, uTackenv;
	ulong cycle_time = (uint)(1000000000 / clk_rate);

	/* transfer timing for PIO mode */
	uint pio_t1[5] = { 70, 50, 30, 30, 25 };
	uint pio_t2[5] = { 290, 290, 290, 80, 70 };
	uint pio_teoc[5] = { 240, 43, 10, 70, 25 };

	/* transfer timing for UDMA mode */
	uint uUdmaTdvh[5] = { 50, 32, 29, 25, 24 };
	uint uUdmaTdvs[5] = { 70, 48, 31, 20, 7 };
	uint uUdmaTrp[5] = { 160, 125, 100, 100, 100 };
	uint uUdmaTss[5] = { 50, 50, 50, 50, 50 };
	uint uUdmaTackenvMin[5] = { 20, 20, 20, 20, 20 };

	for (i = 0; i < 5; i++) {
		t1 = (pio_t1[i] / cycle_time) & 0x0f;
		t2 = (pio_t2[i] / cycle_time) & 0xff;
		teoc = (pio_teoc[i] / cycle_time) & 0xff;
		ide_dev->piotime[i] = (teoc << 12) | (t2 << 4) | t1;
	}

	for (i = 0; i < 5; i++) {
		uTdvh1 = (uUdmaTdvh[i] / cycle_time) & 0x0f;
		uTdvs = (uUdmaTdvs[i] / cycle_time) & 0xff;
		uTrp = (uUdmaTrp[i] / cycle_time) & 0xff;
		uTss = (uUdmaTss[i] / cycle_time) & 0x0f;
		uTackenv = (uUdmaTackenvMin[i] / cycle_time) & 0x0f;
		ide_dev->udmatime[i] = (uTdvh1 << 24) | (uTdvs << 16) |
			(uTrp << 8) | (uTss << 4) | uTackenv;
	}
}

static void ide_setup_ports(struct ide_hw *hw, struct s3c_ide_device *ide_dev)
{
	int i;
	unsigned long *ata_regs = hw->io_ports_array;

	for (i = 0; i < IDE_NR_PORTS-1; i++)
		*ata_regs++ = (ulong)ide_dev->regbase +
				S5P_ATA_PIO_DTR + (i << 2);
}

static void init_ide_device(struct s3c_ide_device *ide_dev)
{
	set_endian_mode(ide_dev, 0);
	set_ata_enable(ide_dev, 1);
	mdelay(100);

	/* Remove IRQ Status */
	ide_writel(ide_dev, 0x3f, S5P_ATA_IRQ);
	ide_writel(ide_dev, 0x3f, S5P_ATA_IRQ_MSK);
}

static u8 s3c_cable_detect(ide_hwif_t *hwif)
{
	/* UDMA4=80-pin cable */
	return ATA_CBL_PATA80;
}

static const struct ide_port_ops s3c_ide_port_ops = {
	.set_pio_mode = s3c_ide_tune_drive,
	.set_dma_mode = s3c_ide_tune_chipset,
	.cable_detect = s3c_cable_detect,
};

static const struct ide_dma_ops s3c_ide_dma_ops = {
	.dma_host_set	= s3c_ide_dma_host_set,
	.dma_setup	= s3c_ide_dma_setup,
	.dma_start	= s3c_ide_dma_start,
	.dma_end	= s3c_ide_dma_end,
	.dma_test_irq	= s3c_ide_dma_test_irq,
	.dma_lost_irq	= s3c_ide_dma_lostirq,
};

static const struct ide_port_info s3c_ide_port_info = {
	.name		= "s3c-ide",
	.init_dma	= &s3c_ide_dma_init,
	.dma_ops	= &s3c_ide_dma_ops,
	.port_ops	= &s3c_ide_port_ops,
/*tp_ops left to the default*/
	.chipset	= ide_s3c,
	.host_flags	= IDE_HFLAG_MMIO | IDE_HFLAG_NO_IO_32BIT |
		IDE_HFLAG_UNMASK_IRQS,
	.pio_mask	= ATA_PIO4,
	.udma_mask	= ATA_UDMA4,
};

static int __devinit s3c_ide_probe(struct platform_device *pdev)
{
	struct resource	*res;
	struct s3c_ide_device *ide_dev;
	struct s3c_ide_platdata *pdata = pdev->dev.platform_data;
	struct ide_host *host;
	int ret = 0;
	struct ide_hw hw, *hws[] = { &hw };

	ide_dev = kzalloc(sizeof(struct s3c_ide_device), GFP_KERNEL);
	if (!ide_dev) {
		dev_err(&pdev->dev, "no memory for s3c device instance\n");
		return -ENOMEM;
	}
	ide_dev->pdev = pdev;

	ide_dev->irq = platform_get_irq(pdev, 0);

	if (ide_dev->irq < 0) {
		dev_err(&pdev->dev, "could not obtain irq number\n");
		ret = -ENODEV;
		goto release_device_mem;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "could not obtain base address\n");
		ret = -ENODEV;
		goto release_device_mem;
	}

	if (!request_mem_region(res->start, res->end - res->start + 1,
		pdev->name)) {
		dev_err(&pdev->dev, "could not obtain i/o address\n");
		ret = -EBUSY;
		goto release_device_mem;
	}

	ide_dev->regbase = ioremap(res->start, res->end - res->start + 1);
	if (ide_dev->regbase == 0) {
		dev_err(&pdev->dev, "could not remap i/o address\n");
		ret = -ENOMEM;
		goto release_mem;
	}

	ide_dev->ide_clk = clk_get(&pdev->dev, "cfcon");
	if (IS_ERR(ide_dev->ide_clk)) {
		dev_err(&pdev->dev, "failed to find clock source\n");
		ret = PTR_ERR(ide_dev->ide_clk);
		ide_dev->ide_clk = NULL;
		goto unmap;
	}

	if (clk_enable(ide_dev->ide_clk)) {
		dev_err(&pdev->dev, "failed to enable clock source.\n");
		goto clkerr;
	}

	setup_timing_value(ide_dev, clk_get_rate(ide_dev->ide_clk));
	if (pdata && (pdata->setup_gpio))
		pdata->setup_gpio();

	init_ide_device(ide_dev);

	memset(&hw, 0, sizeof(hw));
	ide_setup_ports(&hw, ide_dev);
	hw.irq = ide_dev->irq;
	hw.dev = &pdev->dev;

	host = ide_host_alloc(&s3c_ide_port_info, hws, 1);
	if (!host) {
		dev_err(&pdev->dev, "failed to allocate ide host\n");
		ret = -ENOMEM;
		goto stop_clk;
	}

	host->irq_handler = s3c_irq_handler;
	host->ports[0]->hwif_data = (void *)ide_dev;
	s3c_ide_hwif = (void *)ide_dev;
	ide_dev->hwif = host->ports[0];
	platform_set_drvdata(pdev, host);

	ret = ide_host_register(host, &s3c_ide_port_info, hws);
	if (ret) {
		dev_err(&pdev->dev, "failed to register ide host\n");
		ide_host_free(host);
		goto dealloc_ide_host;
	}
	return 0;

dealloc_ide_host:
	ide_host_free(host);
stop_clk:
	clk_disable(ide_dev->ide_clk);
clkerr:
	clk_put(ide_dev->ide_clk);
unmap:
	iounmap(ide_dev->regbase);
release_mem:
	release_mem_region(res->start, res->end - res->start + 1);
release_device_mem:
	kfree(ide_dev);
return ret;
}

static int __devexit s3c_ide_remove(struct platform_device *pdev)
{
	struct ide_host *host = platform_get_drvdata(pdev);
	struct resource *res;
	struct s3c_ide_device *ide_dev = host->ports[0]->hwif_data;

	ide_host_remove(host);

	iounmap(ide_dev->regbase);
	clk_disable(ide_dev->ide_clk);
	clk_put(ide_dev->ide_clk);
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	release_mem_region(res->start, res->end - res->start + 1);
	kfree(ide_dev);

	return 0;
}

#ifdef CONFIG_PM
static int s3c_ide_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct ide_host *host = platform_get_drvdata(pdev);
	struct s3c_ide_device *ide_dev = host->ports[0]->hwif_data;

	disable_irq(IRQ_CFC);
	clk_disable(ide_dev->ide_clk);
	ide_host_remove(host);

	return 0;
}

static int s3c_ide_resume(struct platform_device *pdev)
{
	struct s3c_ide_device *ide_dev = s3c_ide_hwif;
	struct ide_host *host;
	struct ide_hw hw, *hws[] = { &hw };

	clk_enable(ide_dev->ide_clk);
	enable_irq(IRQ_CFC);

	init_ide_device(ide_dev);
	memset(&hw, 0, sizeof(hw));
	ide_setup_ports(&hw, ide_dev);
	hw.irq = ide_dev->irq;
	hw.dev = &pdev->dev;

	/* Re-enumerate the host */
	host = ide_host_alloc(&s3c_ide_port_info, hws, 1);
	host->irq_handler = s3c_irq_handler;
	host->ports[0]->hwif_data = (void *)ide_dev;
	ide_dev->hwif = host->ports[0];
	platform_set_drvdata(pdev, host);
	ide_host_register(host, &s3c_ide_port_info, hws);

	return 0;
}

#else
#define s3c_ide_suspend NULL
#define s3c_ide_resume  NULL
#endif

static struct platform_driver s3c_ide_driver = {
	.probe		= s3c_ide_probe,
	.remove		= __devexit_p(s3c_ide_remove),
	.suspend	= s3c_ide_suspend,
	.resume		= s3c_ide_resume,
	.driver = {
		.name	= "s3c-ide",
		.owner	= THIS_MODULE,
	},
};

static int __init s3c_ide_init(void)
{
	return platform_driver_register(&s3c_ide_driver);
}

static void __exit s3c_ide_exit(void)
{
	platform_driver_unregister(&s3c_ide_driver);
}

module_init(s3c_ide_init);
module_exit(s3c_ide_exit);

MODULE_DESCRIPTION("Samsung SoC IDE");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:s3c-cfcon");
