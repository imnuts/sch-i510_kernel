#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <asm/gpio.h> 
#include <plat/gpio-cfg.h> 
#include <mach/gpio.h>

#include "A1026_dev.h"
#include "A1026_regs.h"
#include "A1026_buffers.h"

/*extern functions*/
int A1026_i2c_drv_init(void);
EXPORT_SYMBOL(A1026_i2c_drv_init);
void A1026_i2c_drv_exit(void);
EXPORT_SYMBOL(A1026_i2c_drv_exit);

/*static functions*/
static int A1026_probe (struct i2c_client *);
static int A1026_remove(struct i2c_client *);
static int A1026_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);

static struct i2c_client *A1026_i2c_client = NULL;
static struct i2c_driver A1026_i2c_driver;

enum A1026STATE {A1026SLEEP, A1026WAKEUP};
enum A1026STATE A1026_state = A1026SLEEP;

int prev_num_closetalk_data = 248;
int prev_num_bypass_data = 32;
int prev_num_fartalk_data;
int prev_num_NS0_data = 248;

enum
{
	eTRUE,
	eFALSE,
}dev_struct_status_t;

/*dev_state*/
typedef struct 
{
	struct mutex lock;
	struct i2c_client const *client;
	dev_state_t state;
	u16 registers[NUM_OF_REGISTERS];
	unsigned short valid;
	unsigned short valid_client_state;
}A1026_device_t;

/*extern functions*/
/******************************************************************************/
/* All the exported functions which view or modify the device                                                 */
/* state/data, do i2c com will have to lock the mutex before                                                  */
/* doing so                                                                                                                        */
/******************************************************************************/
int A1026_dev_init(struct i2c_client *);
int A1026_dev_exit(void);

int A1026_dev_suspend(void);
int A1026_dev_resume(void);

int A1026_dev_powerup(void);
EXPORT_SYMBOL(A1026_dev_powerup);
int A1026_dev_powerdown(void);
EXPORT_SYMBOL(A1026_dev_powerdown);

void A1026SetFeature(unsigned int feature);
EXPORT_SYMBOL(A1026SetFeature);
void A1026Sleep(void);
EXPORT_SYMBOL(A1026Sleep);
void A1026Wakeup(void);
EXPORT_SYMBOL(A1026Wakeup);
void A1026SetBypass(int bypass);
EXPORT_SYMBOL(A1026SetBypass);

/*static functions*/
/******************************************************************************/
static int powerup(void);
static int powerdown(void);
int A1026_Firmware_i2c_write(struct i2c_client *client);

/******************************************************************************/
/*A1026 device structure*/
static A1026_device_t A1026_dev =
{
	.client = NULL,
	.valid = eFALSE,
	.valid_client_state = eFALSE,
};

int A1026_dev_init(struct i2c_client *client)
{
	debug("A1026_dev_init");
	 
	A1026_dev.client = client;
	A1026_dev.valid_client_state = eTRUE; 	 
	 
	return 0;
}

int A1026_dev_exit(void)
{
	debug("A1026_dev_exit");

	A1026_dev.client = NULL;	
	A1026_dev.valid_client_state = eFALSE;
	A1026_dev.valid = eFALSE;

	return 0;
}

int A1026_dev_powerup(void)
{
	int ret = 0;
	
	debug("A1026_dev_powerup");

	if((ret = powerup()) < 0)
	{
		debug("A1026_dev_powerup : failed");
	}

	msleep(50);

	A1026_state = A1026WAKEUP;
	A1026Sleep();

	return ret;
}

int A1026_dev_powerdown(void)
{
	int ret = 0;

	debug("A1026_dev_powerdown"); 

	if(A1026_dev.valid == eFALSE)
	{
		debug("A1026_dev_powerdown called when DS is invalid");
	       ret = -1;	  
	}    
	else if((ret = powerdown()) < 0)
	{
	    debug("A1026_dev_powerdown : failed");
	}

	return ret;
}

int A1026_dev_suspend(void)
{
	int ret = 0;

	debug("A1026_dev_suspend");

	if(A1026_dev.valid_client_state == eFALSE)
	{
		debug("A1026_dev_suspend called when DS (state, client) is invalid");
		ret = -1;
	}   

	return ret;
}

int A1026_dev_resume(void)
{
	int ret = 0; 

	if(A1026_dev.valid_client_state == eFALSE)
	{
		debug("A1026_dev_resume called when DS (state, client) is invalid");
		ret = -1;	  
	}

	return ret;
}

int configureIO(void)
{
	//configure GPIOs for A1026
	if(gpio_is_valid(GPIO_NOISE_RST))
	{
		if (gpio_request(GPIO_NOISE_RST , "GPIO_GPC11"))
			debug("Failed to request GPIO_NOISE_RST!");

		gpio_direction_output(GPIO_NOISE_RST , 1);
	}

	if(gpio_is_valid(GPIO_NOISE_BYPASS))
	{
		if (gpio_request(GPIO_NOISE_BYPASS, "GPIO_GPC12"))
			debug("Failed to request GPIO_NOISE_BYPASS!");
		gpio_direction_output(GPIO_NOISE_BYPASS, 1);
	}	


	if(gpio_is_valid(GPIO_NOISE_WAKEUP))
	{
		if (gpio_request(GPIO_NOISE_WAKEUP, "GPIO_GPC14"))
			debug("Failed to request GPIO_NOISE_WAKEUP!");
		gpio_direction_output(GPIO_NOISE_WAKEUP, 0);
	}

	return 0;
}

int A1026Loadfirmware(void)
{
	int ret;
	int count = 0;
	u8 buf[2], buf_1[4]={0};

	debug("A1026 slave addr = 0x%x", A1026_i2c_client->addr);

	buf[0] = 0xff;
	buf[1] = 0xff;

	ret = i2c_master_send(A1026_i2c_client, a1026_bootinit_command, 2);

	debug("i2c_master_send ret = [%d]", ret);
	msleep(1);

	ret = i2c_master_recv(A1026_i2c_client, buf, 1);
	debug("bootinit command after read reg = 0x%x", buf[0]);

	do
	{
		ret = A1026_Firmware_i2c_write(A1026_i2c_client);
		msleep(20);
		i2c_master_send(A1026_i2c_client, a1026_sync_polling_command, 4);
		msleep(1);
		i2c_master_recv(A1026_i2c_client, buf_1, 4);
		ret = strncmp(a1026_sync_polling_command,buf_1,4);

		debug("A1026 SYNC 0x%x 0x%x 0x%x 0x%x, count = %d",
				buf_1[0], buf_1[1], buf_1[2], buf_1[3], count);

		if(ret!=0) 
			debug("A1026 SYNC fail 0x%x 0x%x 0x%x 0x%x, count = %d",
				buf_1[0], buf_1[1], buf_1[2], buf_1[3], count);

		count++;

		if(count>3) 
			break;
	}while(ret!=0);		

	msleep(10);

	return 0;
}

/******************************************************************************/
static int powerup(void)
{
	int ret=0;

	ret = configureIO();

	gpio_set_value(GPIO_NOISE_WAKEUP, 1);
	gpio_set_value(GPIO_NOISE_RST, 0);
	msleep(50);

	gpio_set_value(GPIO_NOISE_RST, 1);
	msleep(50);

	debug("A1026 Download Start");

	ret = A1026Loadfirmware();

	debug("A1026 Download End");

	s3c_gpio_cfgpin(GPIO_NOISE_SCL, 0);
	s3c_gpio_setpull(GPIO_NOISE_SCL, S3C_GPIO_PULL_NONE);

	s3c_gpio_cfgpin(GPIO_NOISE_SDA, 0);
	s3c_gpio_setpull(GPIO_NOISE_SDA, S3C_GPIO_PULL_NONE);

	return ret;
}

static int powerdown(void)
{
	return 0;
}

void A1026SetFeature(unsigned int feature)
{
	int i;

	A1026Wakeup();

	switch(feature)
	{
		case CLOSETALK:
		{
			debug("A1026SetFeature CLOSETALK mode");

			for(i=0; i<42; i++)
			{
				i2c_master_send(A1026_i2c_client, &(buf_ct_tuning[0+(i*4)]), 4);
			}
		}
		break;

		case CT_VPOFF:
		{
			debug("A1026SetFeature CLOSETALK VP OFF mode");

			for(i=0; i<42; i++)
			{
				i2c_master_send(A1026_i2c_client, &(buf_ct_vpoff_tuning[0+(i*4)]), 4);
			}
		}
		break;

		case FARTALK:
		{
			debug("A1026SetFeature FARTALK mode");

			for(i=0; i<50; i++)
			{
				i2c_master_send(A1026_i2c_client, &(buf_ft_tuning[0+(i*4)]), 4);
			}
		}
		break;

		case BYPASSMODE:
		{
			debug("A1026SetFeature BYPASSMODE mode");

			for(i=0; i<6; i++)
			{
				i2c_master_send(A1026_i2c_client, &(buf_bypass_tuning[0+(i*4)]), 4);
			}
		}
		break;		

		case SIDETONE_BYPASSMODE:
		{
			debug("A1026SetFeature SIDETONE_BYPASSMODE mode");

			for(i=0; i<42; i++)
			{
				i2c_master_send(A1026_i2c_client, 
						&(buf_sidetone_bypass_tuning[0+(i*4)]), 4);
			}
		}
		break;

		case BT_AECON:
		{
			debug("A1026SetFeature BT_AECON mode");

			for(i=0; i<34; i++)
			{
				 i2c_master_send(A1026_i2c_client, &(buf_bt_aecon_tuning[0+(i*4)]), 4);
			}
		}
		break;

		case BT_AECOFF:
		{
			debug("A1026SetFeature BT_AECOFF mode");

			for(i=0; i<30; i++)
			{
				i2c_master_send(A1026_i2c_client, &(buf_bt_aecoff_tuning[0+(i*4)]), 4);
			}
		}
		break;

		case EAR_AECON:
		{
			debug("A1026SetFeature EAR_AECON mode");

			for(i=0; i<34; i++)
			{
				i2c_master_send(A1026_i2c_client, &(buf_ear_tuning[0+(i*4)]), 4);
			}
		}
		break;

		case NS0:
		{
			debug("A1026SetFeature NS0 mode");

			i2c_master_send(A1026_i2c_client, buf_NS0_tuning, prev_num_NS0_data);							
		}		
		break;

		default:
			debug("A1026SetFeature Invalid mode");
		break;
	}
}

void A1026Sleep(void)
{
	char sleep_cmd[4]={0x80,0x10,0x00,0x01};
	char buf[4]={0};
	int ret=0;
	int count=0;

	if(A1026_state == A1026SLEEP) 
		return;

	A1026_state = A1026SLEEP;

	debug("A1026Sleep");
	msleep(50);

	do 
	{
		ret = i2c_master_send(A1026_i2c_client, sleep_cmd, 4);
		msleep(20);

		ret = i2c_master_recv(A1026_i2c_client, buf, 4);
		debug("A1026Sleep 0x%x 0x%x 0x%x 0x%x", buf[0], buf[1], buf[2], buf[3]);

		ret=strncmp(sleep_cmd,buf,4);
		if(ret != 0) 
			debug("A1026Sleep failed");

		count++;

		if(count > 10) 
			break;
	}while(ret != 0);

	debug("A1026Sleep ret=%d",ret);

	msleep(140);
}

void A1026Wakeup(void)
{
	char wakeup_cmd[4]={0x80,0x00,0x00,0x00};
	char buf[4]={0};
	int ret=0;
	int count=0;

	if(A1026_state == A1026WAKEUP) 
		return;

	A1026_state = A1026WAKEUP;

	debug("A1026Wakeup");
	msleep(90);

	do
	{
		gpio_set_value(GPIO_NOISE_WAKEUP, 1);
		msleep(10);

		gpio_set_value(GPIO_NOISE_WAKEUP, 0);
		msleep(30);

		i2c_master_send(A1026_i2c_client, wakeup_cmd, 4);
		msleep(10);

		i2c_master_recv(A1026_i2c_client, buf, 4);

		debug("A1026Wakeup 0x%x 0x%x 0x%x 0x%x", buf[0], buf[1], buf[2], buf[3]);

		ret = strncmp(wakeup_cmd,buf,4);
		if(ret != 0) 
			debug("A1026Wakeup failed");

		count++;

		if(count > 10)
			break;
	}while(ret != 0);

	debug("A1026Wakeup ret=%d", ret);

	msleep(5);
	gpio_set_value(GPIO_NOISE_WAKEUP, 1);
	msleep(30);
}

void A1026SetBypass(int bypass)
{
	debug("A1026SetBypass(%d)", bypass);

	if(bypass)
		gpio_set_value(GPIO_NOISE_BYPASS, 1);
	else
		gpio_set_value(GPIO_NOISE_BYPASS, 0);
}

int A1026_Firmware_i2c_write(struct i2c_client *client)
{
	int ret, total_num;
	unsigned int i, index;

	total_num = TOTAL_NUM_OF_FW;	


	for(i = 0; i < (total_num / NUM_OF_BYTE); i++)
	{
		index = i * NUM_OF_BYTE;
		ret = i2c_master_send(A1026_i2c_client, 
				&a1026_firmware_buf[index], NUM_OF_BYTE);

		if(ret != NUM_OF_BYTE)
		{
			debug("A1026 firmware download error");
			return -1;
		}
	}	

	ret = i2c_master_send(A1026_i2c_client, 
			&a1026_firmware_buf[index + NUM_OF_BYTE], REMAINED_NUM);

	if(ret != REMAINED_NUM)
	{
		debug("A1026 firmware download error!");
		return -1;
	}

	return 0;
}

/******************************************************************************/
/*I2C Setting*/
struct a1026_state {
	struct i2c_client *client;
};

static struct i2c_device_id A1026_id[] = {
	{"A1026_driver", 0},
	{}
};

static struct i2c_driver A1026_i2c_driver =
{
	.driver = {
		.name = "A1026_driver",
	},
	
	.id_table = A1026_id,
	.probe = A1026_i2c_probe,
	.remove = __devexit_p(A1026_remove),
	.command = NULL,
};

static int A1026_probe (struct i2c_client *client)
{
	int ret = 0;

	debug("A1026_probe"); 

	if((ret = A1026_dev_init(client)) < 0)
	{
		debug("A1026_dev_init failed");
	}

	return ret;
}

static int A1026_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct a1026_state *state;
	struct device *dev = &client->dev;

	debug("A1026_i2c_probe");

	state = kzalloc(sizeof(struct a1026_state), GFP_KERNEL);

	if(!state) {
		dev_err(dev, "%s: failed to create a1026_state", __func__);
		return -ENOMEM;
	}

	state->client = client;
	A1026_i2c_client = client;

	i2c_set_clientdata(client, state);

	if(!A1026_i2c_client)
	{
		dev_err(dev, "%s: failed to create A1026_i2c_client", __func__);
		return -ENODEV;
	}

	A1026_probe(A1026_i2c_client);

	return 0;
}


static int A1026_remove(struct i2c_client *client)
{
	int ret = 0;

	debug("A1026_remove"); 

	if(strcmp(client->name, "A1026") != 0)
	{
		ret = -1;
		debug("A1026_remove: device not supported");
	}
	else if((ret = A1026_dev_exit()) < 0)
	{
		debug("A1026_dev_exit failed");
	}

	return ret;
}

int A1026_i2c_drv_init(void)
{
	int ret;

	debug("A1026_i2c_drv_init");

	ret = i2c_add_driver(&A1026_i2c_driver);
	if (ret != 0) {
		return ret;
	}

	return 0;
}

void A1026_i2c_drv_exit(void)
{
	debug("A1026_i2c_drv_exit"); 

	i2c_del_driver(&A1026_i2c_driver);
}

