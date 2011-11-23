/****************************************************************************
**
** COPYRIGHT(C) : Samsung Electronics Co.Ltd, 2006-2010 ALL RIGHTS RESERVED
**
**                LTE SPI Driver
** Author: Viswanath Puttagunta <vputtagunta@sta.samsung.com>
** Date Created: 04-20-2010
** Ported from Aries OneDRAM driver
****************************************************************************/
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/spi/spi.h>
#include <linux/mm.h>
#include <mach/map.h>
#include <mach/gpio.h>
#include <mach/regs-gpio.h>
#include <plat/gpio-cfg.h>
#include <asm/io.h>
#include <mach/hardware.h>




/*This is a temporary piece of crappy code that I was forced to write as I did not figure
 out how to properly use the SPI driver in the system. Please resist the temptation of ever using
 this code in a good Samsung phone. I will remove this code as soon as I figure out the correct
 way of using the SPI driver*/

typedef unsigned int UINT32;
typedef int			INT32;
typedef unsigned short UINT16;
typedef unsigned char UINT8;

#define DRV_SPI0_BASE       (0xE1300000)
#define SPI_1_SPEED            16000000            //16Mhz.
#define S5PC11X_VA_SYSCON S3C_VA_SYS

// SPI SFR Control Structure
typedef struct {
    UINT32 ch_cfg;          // 0x00 spi configuration register
    UINT32 clk_cfg;         // 0x04 clock configuration register
    UINT32 mode_cfg;        // 0x08 spi fifo control register
    UINT32 slave_sel;       // 0x0C slave selection signal
    UINT32 int_enable;      // 0x10 spi interrupt enable register
    UINT32 spi_status;      // 0x14 spi status register
    UINT32 spi_tx_data;     // 0x18 spi tx data register
    UINT32 spi_rx_data;     // 0x1C spi rx data register
    UINT32 packet_count;    // 0x20 count how many data master gets.
    UINT32 pending_clr;     // 0x24 pending clear register
    UINT32 swap_config;     // 0x28 swap config register
    UINT32 feedback_clk;    // 0x2C feedback clock config register
}S5PC100_SPI_SFR;

static void __iomem *spi0_regbase = 0;
static void __iomem *rd_addr = 0;

//sspi->regs = ioremap(sspi->iores->start, sspi->iores->end - sspi->iores->start + 1); // lifted from spi_s3c.c

int DRV_SPI_Init(void)
{
	UINT8 prescaler = 0;
	UINT32 src_clock = 0;
	UINT32 rd_val;
	

	//This lines tells how desperate I am right now .. ha ha..
	spi0_regbase = ioremap_nocache(DRV_SPI0_BASE, 4096);	 

	if (NULL == spi0_regbase)
	{
		printk(KERN_ERR "DRV_SPI_Init ioremap falied \n");
	}
	else
	{
		printk(KERN_ERR "DRV_SPI_Init: Virt addr = %d \n", (UINT32)spi0_regbase);
	}

	rd_addr =  (S5PC11X_VA_SYSCON + 0x314);		// Clk Div for SPI0 
	writel ((readl(rd_addr) | 4), rd_addr);
	printk(KERN_ERR "SPI0 CLK DIV = 0x%08x \n", readl(rd_addr));

	rd_addr =  (S5PC11X_VA_SYSCON + 0x46C);		// Clk Gate for SPI0
	writel ((readl(rd_addr) | (1 << 12)), rd_addr);
	printk(KERN_ERR "SPI0 CLK GATE = 0x%08x  \n", readl(rd_addr));

	

	//Set SPI0 GPIOs
	s3c_gpio_cfgpin(GPIO_LTE_SPI_CS, S3C_GPIO_SFN(GPIO_LTE_SPI_CS_AF));
	s3c_gpio_setpull(GPIO_LTE_SPI_CS, S3C_GPIO_PULL_UP);

	s3c_gpio_cfgpin(GPIO_LTE_SPI_CLK, S3C_GPIO_SFN(GPIO_LTE_SPI_CLK_AF));
	s3c_gpio_setpull(GPIO_LTE_SPI_CLK, S3C_GPIO_PULL_UP);

	s3c_gpio_cfgpin(GPIO_LTE_SPI_DI, S3C_GPIO_SFN(GPIO_LTE_SPI_DI_AF));
	s3c_gpio_setpull(GPIO_LTE_SPI_DI, S3C_GPIO_PULL_UP);


	rd_addr =  (spi0_regbase + 0);		// ch_cfg
	rd_val = readl(rd_addr) |	(0 << 6) |		// High_Speed_En = 0
							(0 << 5) |		// SW_RST
							(0 << 4) |		// MASTER
							(0 << 3) |		// CPOL = 0
							(0 << 2) |		// CPHA = 0
							(0 << 1) |		// SPI RX Channel = 0ff
							(0 << 0); 		// SPI TX Channel = off
	writel(rd_val, rd_addr);		// ch_cfg

	
	writel(0x009, (spi0_regbase + 4));		//clk_cfg; Using prescalar of 9
	writel(0x109, (spi0_regbase + 4));		//Now enable clocks

	rd_val = readl(spi0_regbase + 4);

	rd_addr =  (spi0_regbase + 8);		// mode_cfg
	rd_val = readl(rd_addr) | (0<<29)|				// channel transfer size.
                                                (0x3ff<<19)|			// trailling count.
                                                (0<<17)|				// bus transfer size.
                                                (0x20<<11)|			// Rx trigger level
                                                (0x20<<5)|			// Tx trigger level
                                                (0<<2)|				// DMA type	//Not using DMA
                                                (0<<1)|				// DMA type
                                                (0<<0);				// DMA type
       writel(rd_val, rd_addr);

	rd_addr =  (spi0_regbase + 0x0C);		// slave_sel
	rd_val = readl(rd_addr) | (1<<1) |				// Auto mode
							(1 << 0);				// Slave Inactive. NSSOUT = 1
	writel(rd_val, rd_addr);

	printk(KERN_ERR "SPICFG: \ nch_cfg = 0x%08x \n; clk_cfg = 0x%08x \n 	\
					mode_cfg = 0x%08x \n, slave_sel = 0x%08x \n",		\
					readl(spi0_regbase),								\
					readl(spi0_regbase + 4) ,							\
					readl(spi0_regbase + 8),							\
					readl(spi0_regbase + 0x0C) );

	return(0);
}

int spi_load_lte_boot(UINT8 *addr, unsigned int len)
{
	UINT32 rd_val, i;
	UINT8 *tmp_addr;

	DRV_SPI_Init();

	printk(KERN_ERR "DRV_SPI_Init Complete \n");

	//Enable Tx Channel  (ch_cfg)
	rd_addr = spi0_regbase;
	rd_val = readl(rd_addr) | (1 << 0);
	writel (rd_val, rd_addr);

	rd_addr = spi0_regbase + 0x0C;	// slave_sel
	rd_val = readl(rd_addr) & (~(1 << 0));		// Clear NSSOUT
	writel(rd_val, rd_addr);

	rd_addr = spi0_regbase + 0x18;	// spi_tx_data
	tmp_addr = addr;
	for ( i=0; i<len; i++)
	{
		rd_val = (UINT32) (*tmp_addr);
		writel((UINT32)rd_val, rd_addr);
		tmp_addr++;
		udelay(100);		//probably should wait for status and clear it. We'll see
	}
	
	printk(KERN_ERR "Img Load Complete \n");
	return(0);
}
EXPORT_SYMBOL(spi_load_lte_boot);



/* init & cleanup. */
static int __init ltespi_init(void)
{
	printk (KERN_ERR "ltespi_init: Vish\n");
	return 0;		//DRV_SPI_Init();
}

static void __exit ltespi_exit(void)
{
	//do nothing
}
 

module_init(ltespi_init);
module_exit(ltespi_exit);

//MODULE_AUTHOR("SAMSUNG ELECTRONICS CO., LTD");
//MODULE_DESCRIPTION("LTE SPI Device Driver.");
//MODULE_LICENSE("GPL");
