#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <plat/pm.h>
#include <asm/irq.h>
#include <linux/delay.h>
#include <plat/gpio-cfg.h>
#include <mach/regs-gpio.h>
#include <mach/regs-clock.h>
#include <mach/gpio-p1.h>

//#include "sii9234.h"
#include "MHD_SiI9234.h"
#include "sii9234_tpi_regs.h"


#include <linux/syscalls.h> //denis
#include <linux/fcntl.h> //denis
#include <asm/uaccess.h> //denis
#include <mach/max8998_function.h>


/* Slave address */

#define SII9234_SLAVE_ADDR	0x72
#define SUBJECT "MHL_DRIVER"

#define SII_DEV_DBG(format,...)\
	printk ("[ "SUBJECT " (%s,%d) ] " format "\n", __func__, __LINE__, ## __VA_ARGS__);


struct i2c_driver SII9234_i2c_driver;
struct i2c_client *SII9234_i2c_client = NULL;

struct i2c_driver SII9234A_i2c_driver;
struct i2c_client *SII9234A_i2c_client = NULL;

struct i2c_driver SII9234B_i2c_driver;
struct i2c_client *SII9234B_i2c_client = NULL;

struct i2c_driver SII9234C_i2c_driver;
struct i2c_client *SII9234C_i2c_client = NULL;

static struct i2c_device_id SII9234_id[] = {
	{"SII9234", 0},
	{}
};

static struct i2c_device_id SII9234A_id[] = {
	{"SII9234A", 0},
	{}
};

static struct i2c_device_id SII9234B_id[] = {
	{"SII9234B", 0},
	{}
};

static struct i2c_device_id SII9234C_id[] = {
	{"SII9234C", 0},
	{}
};

int MHL_i2c_init = 0;


struct SII9234_state {
	struct i2c_client *client;
};

static struct timer_list MHL_reg_check;

static u8 SII9234_i2c_read(struct i2c_client *client, u8 reg)
{
	u8 ret;

	ret = i2c_smbus_read_byte_data(client, reg);
	if (ret < 0)
	{
		SII_DEV_DBG("i2c read fail");
		return -EIO;
	}
	return ret;

}


static int SII9234_i2c_write(struct i2c_client *client, u8 reg, u8 data)
{
	return i2c_smbus_write_byte_data(client, reg, data);
}

void MHL_HW_Reset(void)
{
	
	SII_DEV_DBG("");
	s3c_gpio_cfgpin(GPIO_HDMI_EN1, S3C_GPIO_OUTPUT);
	s3c_gpio_setpin(GPIO_HDMI_EN1, 1);
	mdelay(200);
	s3c_gpio_cfgpin(GPIO_MHL_RST, S3C_GPIO_OUTPUT);
	s3c_gpio_setpin(GPIO_MHL_RST, 1);
	mdelay(5);
	s3c_gpio_setpin(GPIO_MHL_RST, 0);
	mdelay(20);
	s3c_gpio_setpin(GPIO_MHL_RST, 1);  
	mdelay(10);
}

#if 0
u8 ReadByteTPI (u8 Offset) 
{
	u8 ret;

	ret = i2c_smbus_read_byte_data(SII9234_i2c_client, Offset);
	if (ret < 0)
	{
		SII_DEV_DBG("i2c read fail");
		return -EIO;
	}
	return ret & 0xff;
}

void WriteByteTPI (u8 Offset, u8 Data) 
{
	SII9234_i2c_write(SII9234_i2c_client, Offset, Data);
}

void ReadModifyWriteTPI(u8 Offset, u8 Mask, u8 Data) 
{
	u8 Temp;

	Temp = ReadByteTPI(Offset);		// Read the current value of the register.
	Temp &= ~Mask;					// Clear the bits that are set in Mask.
	Temp |= (Data & Mask);			// OR in new value. Apply Mask to Value for safety.
	WriteByteTPI(Offset, Temp);		// Write new value back to register.
}

u8 ReadIndexedRegister (u8 PageNum, u8 Offset) 
{
	WriteByteTPI(TPI_INDEXED_PAGE_REG, PageNum);		// Indexed page
	WriteByteTPI(TPI_INDEXED_OFFSET_REG, Offset);		// Indexed register
	return ReadByteTPI(TPI_INDEXED_VALUE_REG);			// Return read value
}

void ReadModifyWriteIndexedRegister (u8 PageNum, u8 Offset, u8 Mask, u8 Data) 
{
	u8 Temp;

	Temp = ReadIndexedRegister (PageNum, Offset);	// Read the current value of the register.
	Temp &= ~Mask;									// Clear the bits that are set in Mask.
	Temp |= (Data & Mask);							// OR in new value. Apply Mask to Value for safety.
	WriteByteTPI(TPI_INDEXED_VALUE_REG, Temp);		// Write new value back to register.
}

void WriteIndexedRegister (u8 PageNum, u8 Offset, u8 Data) 
{
	WriteByteTPI(TPI_INDEXED_PAGE_REG, PageNum);		// Indexed page
	WriteByteTPI(TPI_INDEXED_OFFSET_REG, Offset);		// Indexed register
	WriteByteTPI(TPI_INDEXED_VALUE_REG, Data);			// Write value
}




void sii9234_initial_registers_set(void)
{
	SII_DEV_DBG("");
/*	
	SII9234_i2c_write(SII9234_i2c_client, 0x05, 0x01);	// Set SW reset
	
	//Power up
	SII9234_i2c_write(SII9234A_i2c_client, 0x3D, 0x3F);	// Power up CVCC 1.2V core 
	SII9234_i2c_write(SII9234B_i2c_client, 0x11, 0x01);	// Enable TxPLL clock
	SII9234_i2c_write(SII9234B_i2c_client, 0x12, 0x15);	// Enable Tx clock path & Equalizer
	SII9234_i2c_write(SII9234_i2c_client, 0x08, 0x35);	// Power up TMDS TX core
	//Power up END
	
	//Analog PLL control
	SII9234_i2c_write(SII9234B_i2c_client, 0x17, 0x03); // PLL Calrefsel
	SII9234_i2c_write(SII9234B_i2c_client, 0x23, 0x6A); // Auto EQ
	SII9234_i2c_write(SII9234B_i2c_client, 0x24, 0xAA); // Auto EQ
	SII9234_i2c_write(SII9234B_i2c_client, 0x25, 0xCA); // Auto EQ
	SII9234_i2c_write(SII9234B_i2c_client, 0x26, 0xEA); // Auto EQ
	SII9234_i2c_write(SII9234B_i2c_client, 0x4C, 0xA0); // Manual zone control
	SII9234_i2c_write(SII9234B_i2c_client, 0x4D, 0x00); // Pll mode value 		  
	SII9234_i2c_write(SII9234_i2c_client, 0x80, 0x14); // Enable Rx PLL clock
	SII9234_i2c_write(SII9234B_i2c_client, 0x45, 0x44); // Rx PLL BW value from I2C
	SII9234_i2c_write(SII9234_i2c_client, 0xA1, 0xFC); // Tx PLL full BW ~ 400KHz
	SII9234_i2c_write(SII9234_i2c_client, 0xA3, 0xFA); // Tx amplitude CLK: 500mV, Data: 800mV
	//Analog PLL control END
	
	//CBUS & Discovery
	SII9234_i2c_write(SII9234_i2c_client, 0x90, 0x17); // Enable CBUS discovery
	SII9234_i2c_write(SII9234_i2c_client, 0x94, 0x66); // 1.8V CBUS VTH & RGND threshold
	SII9234_i2c_write(SII9234_i2c_client, 0xA5, 0x1C); // RGND hysterisis
	SII9234_i2c_write(SII9234_i2c_client, 0x95, 0x21); // RGND & single discovery attempt
	SII9234_i2c_write(SII9234_i2c_client, 0x96, 0x22); // use 1K and 2K setting
	SII9234_i2c_write(SII9234_i2c_client, 0x92, 0x86); // MHL CBUS discovery
	//SII9234_i2c_write (0xC8, 0x07, 0xF6); // Increase DDC translation layer timer
	//SII9234_i2c_write (0xC8, 0x40, 0x02); // CBUS drive strength to 10 (0x2 value)
	//CBUS & Discovery END
	
	SII9234_i2c_write(SII9234_i2c_client, 0x0D, 0x1C); // Enable HDMI Trans-code mode & DDC port
	SII9234_i2c_write(SII9234_i2c_client, 0x05, 0x04); // Bring out of reset & Enable Auto Soft reset on SCDT = 0
	
	SII9234_i2c_write(SII9234_i2c_client, 0x74, 0xFF); // Clear interrupts
	SII9234_i2c_write(SII9234_i2c_client, 0x78, 0x00); // Disable Interrupts
	
	SII9234_i2c_write(SII9234_i2c_client, 0xC7, 0x00); // HW TPI mode
	
	SII9234_i2c_write(SII9234_i2c_client, 0x1A, 0x00); // Enable TMDS output
	SII9234_i2c_write(SII9234_i2c_client, 0x1E, 0x00); // Put device in Active mode
	
	SII9234_i2c_write(SII9234_i2c_client, 0x3C, 0x03); // Interrupt Status Setup RSEN and HPD

	
*/	
	//u8 data;
	
    // Power Up
	SII9234_i2c_write(SII9234A_i2c_client, 0x3D, 0x3F);			// Power up CVCC 1.2V core
	SII9234_i2c_write(SII9234B_i2c_client, 0x11, 0x01);			// Enable TxPLL Clock
	SII9234_i2c_write(SII9234B_i2c_client, 0x12, 0x15);			// Enable Tx Clock Path & Equalizer
	SII9234_i2c_write(SII9234_i2c_client, 0x08, 0x35);			// Power Up TMDS Tx Core
	
	SII9234_i2c_write(SII9234B_i2c_client, 0x17, 0x03);
	SII9234_i2c_write(SII9234B_i2c_client, 0x1A, 0x20);
	SII9234_i2c_write(SII9234B_i2c_client, 0x22, 0x8A);
	SII9234_i2c_write(SII9234B_i2c_client, 0x23, 0x6A);
	SII9234_i2c_write(SII9234B_i2c_client, 0x24, 0xAA);
	SII9234_i2c_write(SII9234B_i2c_client, 0x29, 0xCA);
	SII9234_i2c_write(SII9234B_i2c_client, 0x26, 0xEA);
	SII9234_i2c_write(SII9234B_i2c_client, 0x4C, 0xA0);
	SII9234_i2c_write(SII9234B_i2c_client, 0x4D, 0x00);
	SII9234_i2c_write(SII9234_i2c_client, 0x80, 0x14);			// Enable Rx PLL Clock Value
	SII9234_i2c_write(SII9234B_i2c_client, 0x45, 0x44);
	SII9234_i2c_write(SII9234B_i2c_client, 0x31, 0x0A);
	//SII9234_i2c_write(SII9234_i2c_client, 0xA0, 0xD0);




	
		
	SII9234_i2c_write(SII9234_i2c_client, 0xA1, 0xFC);
	SII9234_i2c_write(SII9234_i2c_client, 0xA3, 0xFA);
	SII9234_i2c_write(SII9234_i2c_client, 0x2B, 0x01);
	SII9234_i2c_write(SII9234_i2c_client, 0x91, 0xE5);
	SII9234_i2c_write(SII9234_i2c_client, 0xA5, 0x00);
	SII9234_i2c_write(SII9234_i2c_client, 0x90, 0x27);			// Enable CBUS discovery
	SII9234_i2c_write(SII9234_i2c_client, 0x05, 0x04);
	SII9234_i2c_write(SII9234_i2c_client, 0x0D, 0x1C); 			// HDMI Transcode mode enable


/*

	SII9234_i2c_read(SII9234A_i2c_client, 0x3D, &data);		
	printk("SII9234_i2c_read  A 0x3D: 0x%02x\n", data);
	SII9234_i2c_read(SII9234B_i2c_client, 0x11, &data);		
	printk("SII9234_i2c_read  B 0x11: 0x%02x\n", data);
	SII9234_i2c_read(SII9234B_i2c_client, 0x12, &data);		
	printk("SII9234_i2c_read  B 0x12: 0x%02x\n", data);

	SII9234_i2c_read(SII9234_i2c_client, 0x08, &data);		
	printk("SII9234_i2c_read  0x08: 0x%02x\n", data);
	SII9234_i2c_read(SII9234_i2c_client, 0x80, &data);		
	printk("SII9234_i2c_read  0x80: 0x%02x\n", data);
	SII9234_i2c_read(SII9234_i2c_client, 0x90, &data);		
	printk("SII9234_i2c_read  0x90: 0x%02x\n", data);
	SII9234_i2c_read(SII9234_i2c_client, 0x0D, &data);		
	printk("SII9234_i2c_read  0x0D: 0x%02x\n", data);

*/
}

void startTPI(void)
{
	WriteByteTPI(TPI_ENABLE, 0x00);				// Write "0" to 72:C7 to start HW TPI mode
	mdelay(100); 
}


void mhl_output_enable(void)
{
	mdelay(100); 
	//ReadModifyWriteIndexedRegister(INDEXED_PAGE_0, 0x95, BIT_5, BIT_5);  
	//WriteIndexedRegister(INDEXED_PAGE_0, 0x74, 0x40);	  
	WriteIndexedRegister(INDEXED_PAGE_0, 0xA0, 0x10);  
	mdelay(100); 
	ReadModifyWriteTPI(TPI_SYSTEM_CONTROL_DATA_REG, TMDS_OUTPUT_CONTROL_MASK, TMDS_OUTPUT_CONTROL_ACTIVE);   
}

#endif

#include "MHD_SiI9234.c"

void sii_9234_monitor(unsigned long arg)
{
	SII_DEV_DBG("");
	//sii9234_polling();
	ReadIndexedRegister(INDEXED_PAGE_0, 0x81);
	//printk("SII9234_i2c_read  INDEXED_PAGE_0: 0x%02x\n", data);

	MHL_reg_check.expires = get_jiffies_64() + (HZ*3);
	add_timer(&MHL_reg_check);
}

static void check_HDMI_signal(unsigned long arg)
{
	SII_DEV_DBG("");
	//u8 data;

	//MHL_HW_Reset();
  	//sii9234_initial_registers_set();
	//startTPI();
  	//mhl_output_enable();
	sii9234_tpi_init();
	
	MHL_reg_check.function = sii_9234_monitor;
	MHL_reg_check.expires = get_jiffies_64() + (HZ*3);
	add_timer(&MHL_reg_check);
	
	//data=ReadIndexedRegister(INDEXED_PAGE_0, 0x81);
	//printk("SII9234_i2c_read  INDEXED_PAGE_0: 0x%02x\n", data);
}




//static DECLARE_DELAYED_WORK(init_sii9234, sii92324_init_sequance);

static int SII9234_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	SII_DEV_DBG("");
	//int retval;

	struct SII9234_state *state;

	state = kzalloc(sizeof(struct SII9234_state), GFP_KERNEL);
	if (state == NULL) {		
		printk("failed to allocate memory \n");
		return -ENOMEM;
	}
	
	state->client = client;
	i2c_set_clientdata(client, state);
	
	/* rest of the initialisation goes here. */
	
	printk("SII9234 attach success!!!\n");

	SII9234_i2c_client = client;

	MHL_i2c_init = 1;
	//schedule_delayed_work(&init_sii9234,5000);
	init_timer(&MHL_reg_check);
	MHL_reg_check.function = check_HDMI_signal;
	MHL_reg_check.expires = get_jiffies_64() + (HZ*10);
	add_timer(&MHL_reg_check);
	
	//MHL_HW_Reset();
  	//sii9234_initial_registers_set();
  	//startTPI();
  	//mhl_output_enable();
	
	return 0;

}



static int __devexit SII9234_remove(struct i2c_client *client)
{
	struct SII9234_state *state = i2c_get_clientdata(client);
	kfree(state);

	return 0;
}

static int SII9234A_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	SII_DEV_DBG("");

	struct SII9234_state *state;

	state = kzalloc(sizeof(struct SII9234_state), GFP_KERNEL);
	if (state == NULL) {		
		printk("failed to allocate memory \n");
		return -ENOMEM;
	}
	
	state->client = client;
	i2c_set_clientdata(client, state);
	
	/* rest of the initialisation goes here. */
	
	printk("SII9234A attach success!!!\n");

	SII9234A_i2c_client = client;

	return 0;

}



static int __devexit SII9234A_remove(struct i2c_client *client)
{
	struct SII9234_state *state = i2c_get_clientdata(client);
	kfree(state);
	return 0;
}

static int SII9234B_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	SII_DEV_DBG("");

	struct SII9234_state *state;

	state = kzalloc(sizeof(struct SII9234_state), GFP_KERNEL);
	if (state == NULL) {		
		printk("failed to allocate memory \n");
		return -ENOMEM;
	}
	
	state->client = client;
	i2c_set_clientdata(client, state);
	
	/* rest of the initialisation goes here. */
	
	printk("SII9234B attach success!!!\n");

	SII9234B_i2c_client = client;

	
	return 0;

}



static int __devexit SII9234B_remove(struct i2c_client *client)
{
	struct SII9234_state *state = i2c_get_clientdata(client);
	kfree(state);
	return 0;
}

static int SII9234C_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	SII_DEV_DBG("");

	struct SII9234_state *state;

	state = kzalloc(sizeof(struct SII9234_state), GFP_KERNEL);
	if (state == NULL) {		
		printk("failed to allocate memory \n");
		return -ENOMEM;
	}
	
	state->client = client;
	i2c_set_clientdata(client, state);
	
	/* rest of the initialisation goes here. */
	
	printk("SII9234C attach success!!!\n");

	SII9234C_i2c_client = client;

	
	return 0;

}



static int __devexit SII9234C_remove(struct i2c_client *client)
{
	struct SII9234_state *state = i2c_get_clientdata(client);
	kfree(state);
	return 0;
}


struct i2c_driver SII9234_i2c_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "SII9234",
	},
	.id_table	= SII9234_id,
	.probe	= SII9234_i2c_probe,
	.remove	= __devexit_p(SII9234_remove),
	.command = NULL,
};

struct i2c_driver SII9234A_i2c_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "SII9234A",
	},
	.id_table	= SII9234A_id,
	.probe	= SII9234A_i2c_probe,
	.remove	= __devexit_p(SII9234A_remove),
	.command = NULL,
};

struct i2c_driver SII9234B_i2c_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "SII9234B",
	},
	.id_table	= SII9234B_id,
	.probe	= SII9234B_i2c_probe,
	.remove	= __devexit_p(SII9234B_remove),
	.command = NULL,
};

struct i2c_driver SII9234C_i2c_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "SII9234C",
	},
	.id_table	= SII9234C_id,
	.probe	= SII9234C_i2c_probe,
	.remove	= __devexit_p(SII9234C_remove),
	.command = NULL,
};


