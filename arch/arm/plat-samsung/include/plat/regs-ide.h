/* arch/arm/plat-s3c/include/plat/regs-ide.h
 *
 * Copyright (C) 2009 Samsung Electronics
 * 	http://samsungsemi.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * S5PC1XX IDE register definitions
*/

#ifndef __ASM_ARM_REGS_IDE
#define __ASM_ARM_REGS_IDE __FILE__

#define S5P_CFATA_REG(x) (x)

#define S5P_ATA_CTRL            S5P_CFATA_REG(0x0)
#define S5P_ATA_STATUS          S5P_CFATA_REG(0x4)
#define S5P_ATA_CMD             S5P_CFATA_REG(0x8)
#define S5P_ATA_SWRST           S5P_CFATA_REG(0xc)
#define S5P_ATA_IRQ             S5P_CFATA_REG(0x10)
#define S5P_ATA_IRQ_MSK         S5P_CFATA_REG(0x14)
#define S5P_ATA_CFG             S5P_CFATA_REG(0x18)

#define S5P_ATA_MDMA_TIME       S5P_CFATA_REG(0x28)
#define S5P_ATA_PIO_TIME        S5P_CFATA_REG(0x2c)
#define S5P_ATA_UDMA_TIME       S5P_CFATA_REG(0x30)
#define S5P_ATA_XFR_NUM         S5P_CFATA_REG(0x34)
#define S5P_ATA_XFR_CNT         S5P_CFATA_REG(0x38)
#define S5P_ATA_TBUF_START      S5P_CFATA_REG(0x3c)
#define S5P_ATA_TBUF_SIZE       S5P_CFATA_REG(0x40)
#define S5P_ATA_SBUF_START      S5P_CFATA_REG(0x44)
#define S5P_ATA_SBUF_SIZE       S5P_CFATA_REG(0x48)
#define S5P_ATA_CADR_TBUF       S5P_CFATA_REG(0x4c)
#define S5P_ATA_CADR_SBUF       S5P_CFATA_REG(0x50)
#define S5P_ATA_PIO_DTR         S5P_CFATA_REG(0x54)
#define S5P_ATA_PIO_FED         S5P_CFATA_REG(0x58)
#define S5P_ATA_PIO_SCR         S5P_CFATA_REG(0x5c)
#define S5P_ATA_PIO_LLR         S5P_CFATA_REG(0x60)
#define S5P_ATA_PIO_LMR         S5P_CFATA_REG(0x64)
#define S5P_ATA_PIO_LHR         S5P_CFATA_REG(0x68)
#define S5P_ATA_PIO_DVR         S5P_CFATA_REG(0x6c)
#define S5P_ATA_PIO_CSD         S5P_CFATA_REG(0x70)
#define S5P_ATA_PIO_DAD         S5P_CFATA_REG(0x74)
#define S5P_ATA_PIO_READY       S5P_CFATA_REG(0x78)
#define S5P_ATA_PIO_RDATA       S5P_CFATA_REG(0x7c)
#define S5P_BUS_FIFO_STATUS     S5P_CFATA_REG(0x80)
#define S5P_ATA_FIFO_STATUS     S5P_CFATA_REG(0x84)

#define S5P_ATA_CFG_SWAP        0x40
#define S5P_ATA_CFG_IORDYEN     0x02

#endif /* __ASM_ARM_REGS_IDE */
