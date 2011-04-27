#ifndef __KR3DM_ACC_HEADER__
#define __KR3DM__ACC_HEADER__

#include "kr3dm_i2c.h"

/* dev info */
#define ACC_DEV_NAME "accelerometer"
#define ACC_DEV_MAJOR 241

/* kr3dm ioctl command label */
#define KR3DM_IOCTL_BASE 'a'
#define KR3DM_IOCTL_SET_DELAY       _IOW(KR3DM_IOCTL_BASE, 0, int)
#define KR3DM_IOCTL_GET_DELAY       _IOR(KR3DM_IOCTL_BASE, 1, int)
#define KR3DM_IOCTL_SET_ENABLE      _IOW(KR3DM_IOCTL_BASE, 2, int)
#define KR3DM_IOCTL_GET_ENABLE      _IOR(KR3DM_IOCTL_BASE, 3, int)
#define KR3DM_IOCTL_SET_G_RANGE     _IOW(KR3DM_IOCTL_BASE, 4, int)
#define KR3DM_IOCTL_READ_ACCEL_XYZ  _IOW(KR3DM_IOCTL_BASE, 8, int)

/* kr3dm registers */
#define WHO_AM_I    0x0F
/* ctrl 1: pm2 pm1 pm0 dr1 dr0 zenable yenable zenable */
#define CTRL_REG1       0x20    /* power control reg */
#define CTRL_REG2       0x21    /* power control reg */
#define CTRL_REG3       0x22    /* power control reg */
#define CTRL_REG4       0x23    /* interrupt control reg */
#define CTRL_REG5       0x24    /* interrupt control reg */
#define STATUS_REG		0x27
#define AXISDATA_REG    0x28
#define OUT_X			0x29
#define OUT_Y			0x2B
#define OUT_Z			0x2D
#define INT1_CFG		0x30
#define INT1_SOURCE		0x31
#define INT1_THS		0x32
#define INT1_DURATION	0x33
#define INT2_CFG		0x34
#define INT2_SOURCE		0x35
#define INT2_THS		0x36
#define INT2_DURATION	0x37

#define KR3DM_G_2G    0x00
#define KR3DM_G_4G    0x10
#define KR3DM_G_8G    0x30

#define PM_OFF            0x00
#define PM_NORMAL         0x20
#define ENABLE_ALL_AXES   0x07

#define ODRHALF           0x40  /* 0.5Hz output data rate */
#define ODR1              0x60  /* 1Hz output data rate */
#define ODR2              0x80  /* 2Hz output data rate */
#define ODR5              0xA0  /* 5Hz output data rate */
#define ODR10             0xC0  /* 10Hz output data rate */
#define ODR50             0x00  /* 50Hz output data rate */
#define ODR100            0x08  /* 100Hz output data rate */
#define ODR400            0x10  /* 400Hz output data rate */

/* CTRL_REG1 */
#define CTRL_REG1_PM2		(1 << 7)
#define CTRL_REG1_PM1		(1 << 6)
#define CTRL_REG1_PM0		(1 << 5)
#define CTRL_REG1_DR1		(1 << 4)
#define CTRL_REG1_DR0		(1 << 3)
#define CTRL_REG1_Zen		(1 << 2)
#define CTRL_REG1_Yen		(1 << 1)
#define CTRL_REG1_Xen		(1 << 0)

#define PM_down		0
#define PM_Normal	(0x0|0x0|CTRL_REG1_PM0)
#define PM_Low05	(0x0|CTRL_REG1_PM1|0x0)
#define PM_Low1		(0x0|CTRL_REG1_PM1|CTRL_REG1_PM0)
#define PM_Low2		(CTRL_REG1_PM2|0x0|0x0)
#define PM_Low5		(CTRL_REG1_PM2|0x0|CTRL_REG1_PM0)
#define PM_Low10	(CTRL_REG1_PM2|CTRL_REG1_PM1|0x0)

/* CTRL_REG2 */
#define CTRL_REG2_BOOT		(1 << 7)
#define CTRL_REG2_HPM1		(1 << 6)
#define CTRL_REG2_HPM0		(1 << 5)
#define CTRL_REG2_FDS		(1 << 4)
#define CTRL_REG2_HPen2		(1 << 3)
#define CTRL_REG2_HPen1		(1 << 2)
#define CTRL_REG2_HPCF1		(1 << 1)
#define CTRL_REG2_HPCF0		(1 << 0)

#define HPM_Normal	(CTRL_REG2_HPM1|0x0)
#define HPM_Filter	(0x00|CTRL_REG2_HPM0)

#define HPCF_ft8	(0x000|0x000)
#define HPCF_ft4	(0x000|CTRL_REG2_HPCF0)
#define HPCF_ft2	(CTRL_REG2_HPCF1|0x000)
#define HPCF_ft1	(CTRL_REG2_HPCF1|CTRL_REG2_HPCF0)

/* CTRL_REG3 */
#define CTRL_REG3_IHL		(1 << 7)
#define CTRL_REG3_PP_OD		(1 << 6)
#define CTRL_REG3_LIR2		(1 << 5)
#define CTRL_REG3_I2_CFG1	(1 << 4)
#define ICTRL_REG3_2_CFG0	(1 << 3)
#define CTRL_REG3_LIR1		(1 << 2)
#define CTRL_REG3_I1_CFG1	(1 << 1)
#define CTRL_REG3_I1_CFG0	(1 << 0)

/* Interrupt 1 (2) source */
#define I1_CFG_SC	(0x00 | 0x00)
/* Interrupt 1 source OR Interrupt 2 source */
#define I1_CFG_OR	(0x00 | CTRL_REG3_I1_CFG0)
/* Data Ready */
#define I1_CFG_DR	(CTRL_REG3_I1_CFG1|0x000)
/* Boot running */
#define I1_CFG_BR	(CTRL_REG3_I1_CFG1|CTRL_REG3_I1_CFG0)

 /* Interrupt 1 (2) source */
#define I2_CFG_SC	(0x00 | 0x00)
/* Interrupt 1 source OR Interrupt 2 source */
#define I2_CFG_OR	(0x00 | CTRL_REG3_I2_CFG0)
/* Data Ready */
#define I2_CFG_DR	(CTRL_REG3_I2_CFG1|0x000)
/* Boot running */
#define I2_CFG_BR	(CTRL_REG3_I2_CFG1|CTRL_REG3_I2_CFG0)

/* CTRL_REG4 */
#define CTRL_REG4_FS1		(1 << 5)
#define CTRL_REG4_FS0		(1 << 4)
#define CTRL_REG4_STsign	(1 << 3)
#define CTRL_REG4_ST		(1 << 1)
#define CTRL_REG4_SIM		(1 << 0)

#define FS2g		0x0
#define FS4g		(0x0|CTRL_REG4_FS0)
#define FS8g		(CTRL_REG4_FS1|CTRL_REG4_FS0)

/* CTRL_REG5 */
#define CTRL_REG5_TurnOn1		(1 << 1)
#define CTRL_REG5_TurnOn0		(1 << 0)

/* STATUS_REG */
#define ZYXOR		(1 << 7)
#define ZOR		(1 << 6)
#define YOR		(1 << 5)
#define XOR		(1 << 4)
#define ZYXDA		(1 << 3)
#define ZDA		(1 << 2)
#define YDA		(1 << 1)
#define XDA		(1 << 0)

/* INT1/2_CFG */
#define INT_CFG_AOI		(1 << 7)
#define INT_CFG_6D		(1 << 6)
#define INT_CFG_ZHIE		(1 << 5)
#define INT_CFG_ZLIE		(1 << 4)
#define INT_CFG_YHIE		(1 << 3)
#define INT_CFG_YLIE		(1 << 2)
#define INT_CFG_XHIE		(1 << 1)
#define INT_CFG_XLIE		(1 << 0)

/* INT1/2_SRC */
#define IA		(1 << 6)
#define ZH		(1 << 5)
#define ZL		(1 << 4)
#define YH		(1 << 3)
#define YL		(1 << 2)
#define XH		(1 << 1)
#define XL		(1 << 0)

/*************************************************************/
static inline void switch_on_bits(unsigned char *data, unsigned char bits_on)
{
	*data |= bits_on;
}

static inline void switch_off_bits(unsigned char *data, unsigned char bits_off)
{
	char aux = 0xFF;
	aux ^= bits_off;
	*data &= aux;
}

#define BIT_ON   1
#define BIT_OFF  0

static inline int check_bit(unsigned char data, unsigned char bit)
{
	return (data|bit) ? BIT_ON : BIT_OFF;
}
/**************************************************************/

typedef struct {
	unsigned char ctrl_reg1;
	unsigned char ctrl_reg2;
	unsigned char ctrl_reg3;
	unsigned char ctrl_reg4;
	unsigned char ctrl_reg5;
	unsigned char int1_cfg;
	unsigned char int1_ths;
	unsigned char int1_duration;
	unsigned char int1_source;
	unsigned char int2_cfg;
	unsigned char int2_ths;
	unsigned char int2_duration;
	unsigned char int2_source;
} dev_regs_t;

/********************************************************************/
static inline void CTRL_REGS_BITSET_R2g(dev_regs_t *regs)
{
	switch_off_bits(&(regs->ctrl_reg4), (CTRL_REG4_FS1 | CTRL_REG4_FS0));
}

static inline void CTRL_REGS_BITSET_R4g(dev_regs_t *regs)
{
	switch_off_bits(&(regs->ctrl_reg4), (CTRL_REG4_FS1 | CTRL_REG4_FS0));
	switch_on_bits(&(regs->ctrl_reg4), (FS4g));
}

static inline void CTRL_REGS_BITSET_R8g(dev_regs_t *regs)
{
	switch_off_bits(&(regs->ctrl_reg4), (CTRL_REG4_FS1 | CTRL_REG4_FS0));
	switch_on_bits(&(regs->ctrl_reg4), (FS8g));
}

static inline void CTRL_REGS_BITSET_BW50(dev_regs_t *regs)
{
	switch_off_bits(&(regs->ctrl_reg1), (ODR50));
}

static inline void CTRL_REGS_BITSET_BW100(dev_regs_t *regs)
{
	switch_off_bits(&(regs->ctrl_reg1), (ODR50));
	switch_on_bits(&(regs->ctrl_reg1), (ODR100));
}

static inline void CTRL_REGS_BITSET_BW400(dev_regs_t *regs)
{
	switch_off_bits(&(regs->ctrl_reg1), (ODR50));
	switch_on_bits(&(regs->ctrl_reg1), (ODR400));
}

static inline void CTRL_REGS_BITSET_mwup_rsp_latchd(dev_regs_t *regs)
{
	switch_on_bits(&(regs->ctrl_reg3), (CTRL_REG3_LIR1));
}

static inline void CTRL_REGS_BITSET_mwup_rsp_unlatchd(dev_regs_t *regs)
{
	switch_off_bits(&(regs->ctrl_reg3), (CTRL_REG3_LIR1));
}

/********************************************************************/

/********************************************************************/
static inline void CTRL_REGS_BITSET_mwup_enb(dev_regs_t *regs)
{
	switch_off_bits(&(regs->ctrl_reg1),
		(CTRL_REG1_PM2|CTRL_REG1_PM1|CTRL_REG1_PM0));
	switch_on_bits(&(regs->ctrl_reg1), (PM_Low10));
	switch_on_bits(&(regs->ctrl_reg5),
		(CTRL_REG5_TurnOn1|CTRL_REG5_TurnOn0));
}

static inline void CTRL_REGS_BITSET_mwup_disb(dev_regs_t *regs)
{
	switch_off_bits(&(regs->ctrl_reg1),
		(CTRL_REG1_PM2|CTRL_REG1_PM1|CTRL_REG1_PM0));
	switch_on_bits(&(regs->ctrl_reg1), (PM_Normal));
	switch_off_bits(&(regs->ctrl_reg5),
		(CTRL_REG5_TurnOn1|CTRL_REG5_TurnOn0));
}

static inline void CTRL_REGS_BITSET_ST_actvt(dev_regs_t *regs)
{
/*    switch_on_bits(&(regs->ctrl_reg), (CTRL_REGSB_ST)); */
}

static inline void CTRL_REGS_BITSET_ST_dactvt(dev_regs_t *regs)
{
/*    switch_off_bits(&(regs->ctrl_reg), (CTRL_REGSB_ST)); */
}

static inline void CTRL_REGS_BITSET_KR3DM_xyz_en(dev_regs_t *regs)
{
	switch_on_bits(&(regs->ctrl_reg1),
		(CTRL_REG1_Xen|CTRL_REG1_Yen|CTRL_REG1_Zen));
}

static inline void CTRL_REGS_BITSET_KR3DM_nor_op(dev_regs_t *regs)
{
	switch_off_bits(&(regs->ctrl_reg1),
		(CTRL_REG1_PM2|CTRL_REG1_PM1|CTRL_REG1_PM0));
	switch_on_bits(&(regs->ctrl_reg1), (PM_Normal));
}

static inline void CTRL_REGS_BITSET_KR3DM_standby(dev_regs_t *regs)
{
	switch_off_bits(&(regs->ctrl_reg1),
		(CTRL_REG1_PM2|CTRL_REG1_PM1|CTRL_REG1_PM0));
}
/********************************************************************/

#endif
