#ifndef _A1026_I2C_DRV_H
#define _A1026_I2C_DRV_H

extern int A1026_i2c_drv_init(void);
extern int A1026_i2c_drv_exit(void);
extern int A1026_i2c_attach(struct i2c_adapter *adap);
extern int A1026_i2c_detach(struct i2c_client *client);
#endif
