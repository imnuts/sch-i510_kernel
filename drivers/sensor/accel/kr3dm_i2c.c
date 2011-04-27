#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/workqueue.h>
#include <mach/hardware.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/i2c.h>
#include "kr3dm_i2c.h"
#include <linux/slab.h>

#undef DEBUG

static struct i2c_client *g_client;

struct kr3dm_state {
	struct i2c_client	*client;
};

static int kr3dm_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	struct kr3dm_state *kr3dm;
	kr3dm = kzalloc(sizeof(struct kr3dm_state), GFP_KERNEL);
	if (kr3dm == NULL) {
		pr_err("%s: failed to allocate memory\n", __func__);
		return -ENOMEM;
	}

	kr3dm->client = client;
	i2c_set_clientdata(client, kr3dm);
	g_client = client;
	return 0;
}

static int kr3dm_remove(struct i2c_client *client)
{
	g_client = NULL;
	return 0;
}
static const struct i2c_device_id kr3dm_id[] = {
	{ "kr3dm", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, kr3dm_id);

static struct i2c_driver kr3dm_i2c_driver = {
	.probe = kr3dm_probe,
	.remove = __devexit_p(kr3dm_remove),
	.id_table = kr3dm_id,
	.driver = {
	.owner = THIS_MODULE,
	.name = "kr3dm",
    },
};


int i2c_acc_kr3dm_init(void)
{
	int ret;
	ret = i2c_add_driver(&kr3dm_i2c_driver);
	if (ret) {
		printk(KERN_INFO, "Driver registration failed, module not inserted.\n");
		return ret;
	}

	return 0;
}

void i2c_acc_kr3dm_exit(void)
{
	printk(KERN_INFO, "%d\n",  __func__);
	i2c_del_driver(&kr3dm_i2c_driver);
}


char i2c_acc_kr3dm_read(u8 reg, u8 *val, unsigned int len)
{
	int err;
	struct i2c_msg msg[1];
	unsigned char data[1];
	if ((g_client == NULL) || (!g_client->adapter))
		return -ENODEV;

	msg->addr	= g_client->addr;
	msg->flags	= I2C_M_WR;
	msg->len	= 1;
	msg->buf	= data;
	*data		= reg;

	err = i2c_transfer(g_client->adapter, msg, 1);

	if (err >= 0) {
		msg->flags = I2C_M_RD;
		msg->len   = len;
		msg->buf   = val;
		err = i2c_transfer(g_client->adapter, msg, 1);
	}
	if (err >= 0)
		return 0;
	printk(KERN_ERR, "%s %d i2c transfer error\n", __func__, __LINE__);
	return err;
}

int i2c_acc_kr3dm_read_word(u8 reg)
{
	int err;
	int retval;
	unsigned char val[2];
	unsigned char data[1];
	struct i2c_msg msg[1];

	printk(KERN_INFO, "[KR3DM I2C]%s\n", __func__);
        memset(val,0,2);
	if ((g_client == NULL) || (g_client->adapter == NULL))
		return -ENODEV;

	msg->addr	= g_client->addr;
	msg->flags	= I2C_M_WR;
	msg->len	= 1;
	msg->buf	= data;
	*data		= reg;

	err = i2c_transfer(g_client->adapter, msg, 1);

	if (err < 0) 
        {
             return err;  
        } 
        else
        {
		msg->flags = I2C_M_RD;
		msg->len   = 2;
		msg->buf   = val;
		err = i2c_transfer(g_client->adapter, msg, 1);
	}

	retval = 0x00;
	retval = retval | (val[0] << 8);
	retval = retval | val[1];

	printk(KERN_INFO, "%s %d i2c transfer error\n", __func__, __LINE__);
	return retval;

}

char i2c_acc_kr3dm_write(u8 reg, u8 *val)
{
	int err;
	struct i2c_msg msg[1];
	unsigned char data[2];

	if ((g_client == NULL) || (!g_client->adapter))
		return -ENODEV;

	data[0] = reg;
	data[1] = *val;

	msg->addr = g_client->addr;
	msg->flags = I2C_M_WR;
	msg->len = 2;
	msg->buf = data;

	err = i2c_transfer(g_client->adapter, msg, 1);
#ifdef DEBUG
	{
		int j;
		printk(KERN_INFO, "[KR3DM I2C] g_client->addr: 0x%02x",
			g_client->addr);
		printk(KERN_INFO, "[KR3DM I2C] W: ");
		for (j = 0; j <= 1; j++)
			printk("0x%02x ", data[j]);
		printk("\n");
	}
#endif

	if (err >= 0)
		return 0;
	printk(KERN_ERR, "%s %d i2c transfer error\n", __func__, __LINE__);
	return err;
}
