#ifndef __KR3DM_I2C_HEADER__
#define __KR3DM_I2C_HEADER__

/* kr3dm i2c slave address & etc */
#define KR3DM_I2C_ADDR    0x09
#define I2C_DF_NOTIFY 0x01
#define I2C_M_WR				0x00

char  i2c_acc_kr3dm_read(u8, u8 *, unsigned int);
int i2c_acc_kr3dm_read_word(u8 reg);
char i2c_acc_kr3dm_write(u8 reg, u8 *val);

int  i2c_acc_kr3dm_init(void);
void i2c_acc_kr3dm_exit(void);

#endif
