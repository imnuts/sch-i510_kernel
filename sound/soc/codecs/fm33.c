/*
* File: sound/soc/codecs/fm33.c
*
* Driver for FM33 Noise Cancellation Chip.
*/

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/serio.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/clk.h>
#include <linux/mutex.h>
#include <linux/i2c.h>

#include <asm/mach-types.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <mach/map.h>
#include <asm/mach/irq.h>
#include "wm8994.h"


/************************Macros**********************************/
#define FM33_I2C_ADDR		0xC0
#define DATA_MEM_START		0x22C0
#define DATA_MEM_END		0x23FF
#define DATA_PORT_LBYTE  	          0x25
#define DATA_PORT_HBYTE  	         0x26

/***commands***/
#define RD_MEM_CMD   	0x37
#define WR_MEM_CMD  	0x3B
#define RD_REG_CMD   	0x60
#define WR_REG_CMD  	0x6A

#define SYNC_BYTE_1         	0xFC
#define SYNC_BYTE_2         	0xF3

extern void fm33_gpio_init(void);


/************************Macros and Functions**********************************/

struct fm33_data        {
        struct i2c_client       *client;
        struct device           *dev;
	char 		mode;	//To keep track to the mode.
} *fm33_data;

#ifdef SETTING_NC
	struct i2c_client *gl_fm33_client;
	extern int gl_path_num;
#endif //SETTING_NC
int fm33_probe(struct i2c_client *, const struct i2c_device_id *);
int fm33_remove(struct i2c_client *);
static int fm33_suspend(struct i2c_client *, pm_message_t mesg);
static int fm33_resume(struct i2c_client *);
void fm33_powerdown();
void fm33_set_normalmode(struct i2c_client *);
#ifdef	SETTING_NC
void fm33_set_normalmode_reciever(struct i2c_client *client);
void fm33_set_normalmode_speaker(struct i2c_client *client);
#endif//SETTING_NC
void fm33_set_commode(struct i2c_client *);
static ssize_t fm33_get_mode(struct device *, struct device_attribute *, const char *, size_t);
static ssize_t fm33_set_mode(struct device *, struct device_attribute *, const char *, size_t);

static const struct i2c_device_id fm33_ids[] = {
	{ "fm33", 0 },
	{ },
};

MODULE_DEVICE_TABLE(i2c, fm33_ids);

static struct i2c_driver fm33_i2c_driver = {
	.probe 	=  fm33_probe,
	.remove	= fm33_remove,
	.suspend = fm33_suspend,
	.resume = fm33_resume,
	.driver = {
		.name = "fm33",
	},
	.id_table	= fm33_ids,
};

/* Class Pointer for noise cancellation chip */
struct class *fm33_class;

/* Adding keypad properties*/
static struct device_attribute fm33_attributes[] = {
	__ATTR(fm33_mode, 0666, fm33_get_mode, fm33_set_mode),
};

extern unsigned int HWREV;

/**************************************************************
Function: To get the current working mode info.
**************************************************************/

static ssize_t  fm33_get_mode(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	sprintf( buf, "%d\n", fm33_data->mode);
	
	printk(KERN_INFO "chk %s, buf %d", __FUNCTION__, *buf);
	return sprintf( buf, "%d\n", fm33_data->mode);
}

/***************************************************************
* Function : To set the mode as per Userspace input.
***************************************************************/
static ssize_t  fm33_set_mode(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned char data;

	sscanf(buf, "%d\n", &data);
       	printk(KERN_INFO "chk %s: Value passed by user space: %d, count %d\n", __FUNCTION__, data, count);

	/*Read the input from the user space*/
	switch(data)	{
		case(0):
		fm33_powerdown();
		break;

		case(1):
		fm33_set_normalmode(fm33_data->client);
		break;

		case(2):
		fm33_set_commode(fm33_data->client);
		break;

		default:
		printk(KERN_INFO "FUNCTION %s: FM33 Mode not supported\n",__FUNCTION__);
	}	
	return count;
}

/***************************************************************
* Function : To read the data from FM33 Noise Cancellation IC.
@*fm33_client: i2c client
@mem_addr: Address to read from
@*val: pointer to the buffer where data will be stored
***************************************************************/

int read_fm33_mem(struct i2c_client *fm33_client,u16 mem_addr, u16 *val) {
        int      ret;
        struct   i2c_msg msg[1], r_msg[2];

        unsigned char data[6], read_data[2];

        printk(KERN_INFO "chk %s add 0x%x val 0x%x\n", __FUNCTION__, mem_addr, *val);
	
        if( fm33_client == NULL )       {
                printk(KERN_INFO "chk %s: ERROR I2C Client not set\n", __FUNCTION__);
                return -ENODEV;
        }

        data[0] = SYNC_BYTE_1;
        data[1] = SYNC_BYTE_2;
        data[2] = RD_MEM_CMD;
        data[3] = (mem_addr >> 8) & 0xFF; 	// Send higher address byte first
        data[4] = mem_addr & 0xFF;  		// send lower address byte later

        printk(KERN_INFO "*chk %s, Add1 0x%x Add0 0x%x \n", __FUNCTION__, data[4], data[3]);

        msg->addr       = fm33_client->addr;
        msg->flags      = 0;
        msg->len        = 5;
        msg->buf        = data;

        ret = i2c_transfer(fm33_client->adapter, msg, 1);
        if( ret <= 0 ){
                printk("\n %s, :%s I2C Transfrer Failed \n",__func__,__FILE__);
                return -1;
        }

        /***** read lower byte ***********/
              	data[0] = SYNC_BYTE_1;
        	data[1] = SYNC_BYTE_2;
                data[2] = RD_REG_CMD;
                data[3] = DATA_PORT_LBYTE ;  // lower byte

                r_msg[0].addr       = fm33_client->addr;
                r_msg[0].flags      = 0;
                r_msg[0].len        = 4;
                r_msg[0].buf        = data;

                r_msg[1].addr       = fm33_client->addr;
                r_msg[1].flags = I2C_M_RD;
                r_msg[1].len   = 1;
                r_msg[1].buf   = &read_data[0];
                ret = i2c_transfer(fm33_client->adapter, r_msg, 2);
                if ( ret <= 0 ){
                        printk("\n %s, :%s I2C Transfrer Failed \n",__func__,__FILE__);
                        return -1;
                }
                printk("\n chk err = %d, read0 0x%x\n",ret, read_data[0]);

         /***** read higher byte ***********/
               	data[0] = SYNC_BYTE_1;
               	data[1] = SYNC_BYTE_2;
               	data[2] = RD_REG_CMD;
               	data[3] = DATA_PORT_HBYTE ;  // lower byte

               	r_msg[0].addr       = fm33_client->addr;
               	r_msg[0].flags      = 0;
               	r_msg[0].len        = 4;
               	r_msg[0].buf        = data;

               	r_msg[1].addr       = fm33_client->addr;
               	r_msg[1].flags = I2C_M_RD;
               	r_msg[1].len   = 1;
               	r_msg[1].buf   = &read_data[1];
				
		ret = i2c_transfer(fm33_client->adapter, r_msg, 2);
                if( ret <= 0){
                        printk("\n %s, :%s I2C Transfrer Failed \n",__func__,__FILE__);
                        return -1;
                }

                *val =  ((read_data[1] << 8) | read_data[0]);
                printk(KERN_INFO "chk %s: read_data 0x%x 0x%x val 0x%x\n", __FUNCTION__, read_data[0], read_data[1], *val);

        /********** end of reading **************/

        return ret;
}

/***************************************************************
* Function : To write to FM33 Noise Cancellation IC.
* Function : To read the data from FM33 Noise Cancellation IC.
@*fm33_client: i2c client
@mem_addr: Address to write to
@val: data to be written
***************************************************************/
int write_fm33_mem( struct i2c_client *fm33_client,u16 mem_addr, u16 val ) {
        int ret;
        struct i2c_msg msg[1];
        unsigned char data[8];

        printk(KERN_INFO "chk %s, val 0x%x\n", __FUNCTION__, val);

        if( fm33_client == NULL) {
                printk(KERN_INFO "chk %s: ERROR I2C Client not set\n", __FUNCTION__);
                return -ENODEV;
        }

        data[0] = SYNC_BYTE_1;
        data[1] = SYNC_BYTE_2;
        data[2] = WR_MEM_CMD;
        data[3] = (mem_addr >> 8) & 0xFF; 	// send higher address byte first
        data[4] = mem_addr & 0xFF;  		// lower address byte
        data[5] = (val >> 8) & 0xFF; 		// send higher data byte
        data[6] = val & 0xFF;  			// lower data byte

        printk(KERN_INFO "*chk %s, Add1 0x%x Add0 0x%x Data1 0x%x Data0 0x%x\n", __FUNCTION__, data[4], data[3], data[6], data[5]);

        msg->addr = fm33_client->addr;
        msg->flags = 0;
        msg->len = 7;
        msg->buf = data;

        ret = i2c_transfer(fm33_client->adapter, msg, 1);
        if( ret <= 0){
                printk("\n ERROR %s :%d  i2c transfer error <<<<<<< \n", __func__, __LINE__);
                return -1;
        }

        return ret;
}

/*************************************************************
* Function: To reset the FM33 Noise Cancellation IC. Vendor specific Reset sequence.
BP#_(H) 10ms -> PWD_(H) 20ms -> Reset (L) 10ms -> Reset (H) 20ms 

**************************************************************/
void fm33_reset()
{
	printk(KERN_INFO "Function: %s\n", __FUNCTION__);
	gpio_set_value(GPIO_FM33_BP, 1);           	//FM33_BP
	msleep(10);
	gpio_set_value(GPIO_FM33_PWDN, 1);		//set PWDN PIN High
	msleep(20);

	gpio_set_value(GPIO_FM33_RST, 0);
	msleep(10);
	gpio_set_value(GPIO_FM33_RST, 1);
	msleep(20);
}

/***************************************************************
* Function: To power fown the Noise Cancellation IC.
***************************************************************/
void fm33_powerdown()
{
	printk(KERN_INFO "Function: %s\n", __FUNCTION__);
	s3c_gpio_setpull(GPIO_FM33_RST, S3C_GPIO_PULL_DOWN);
	msleep(20);			//Specified as 1ms. Giving more delay, reconfirm the value.
	fm33_data->mode = 0;
}

/***************************************************************
* Function: For setting FM33 Noise Cancellation IC in Communication mode.
***************************************************************/
void fm33_set_commode(struct i2c_client *client)
{
        printk(KERN_INFO "chk %s, \n", __FUNCTION__);
	fm33_reset();

        write_fm33_mem(client, 0x22c8, 0x0017);         //For MCLK setting 24MHz ext clk
        write_fm33_mem(client, 0x22d2, 0x8000);         //For I2S mode setting

        //FIX HERE: Need to pass param setting for comunication mode.

        write_fm33_mem(client, 0x22fb, 0x0000);         //To Start the chip (To Run the DSP)

        msleep(200);   
	fm33_data->mode = 2;
}

/***************************************************************
* Function: For setting FM33 Noise Cancellation IC in Normal mode.
***************************************************************/
void fm33_set_normalmode(struct i2c_client *client)
{
        printk(KERN_INFO "chk %s, \n", __FUNCTION__);
	fm33_reset();
#if 0
        write_fm33_mem(client, 0x22c8, 0x0017);		//For MCLK setting 24MHz ext clk
        write_fm33_mem(client, 0x22d2, 0x8000);		//For I2S mode setting

	//FIX HERE : check any param setting needed for normal mode.
        
	write_fm33_mem(client, 0x22fb, 0x0000);		//To Start the chip (To Run the DSP)
#else

	#if  0 
        write_fm33_mem(client, 0x22c8, 0x0017);		//For MCLK setting 24MHz ext clk
        write_fm33_mem(client, 0x22d2, 0x8000);		//For MCLK setting 24MHz ext clk
        write_fm33_mem(client, 0x22ec, 0x007c);		//For MCLK setting 24MHz ext clk
        write_fm33_mem(client, 0x22f9, 0x007f);		//For MCLK setting 24MHz ext clk
        write_fm33_mem(client, 0x22fa, 0x0003);		//For MCLK setting 24MHz ext clk
        write_fm33_mem(client, 0x2305, 0x00ee);		//For MCLK setting 24MHz ext clk
        write_fm33_mem(client, 0x2303, 0x6da1);		//For MCLK setting 24MHz ext clk
        write_fm33_mem(client, 0x2309, 0x0400);		//For MCLK setting 24MHz ext clk
        write_fm33_mem(client, 0x236e, 0x1000);		//For MCLK setting 24MHz ext clk
        write_fm33_mem(client, 0x22ee, 0x0001);		//For MCLK setting 24MHz ext clk
        write_fm33_mem(client, 0x2328, 0x7fff);		//For MCLK setting 24MHz ext clk
        write_fm33_mem(client, 0x230c, 0x0100);		//For MCLK setting 24MHz ext clk
        write_fm33_mem(client, 0x22f8, 0x8000);		//For MCLK setting 24MHz ext clk
        write_fm33_mem(client, 0x2301, 0x0012);		//For MCLK setting 24MHz ext clk
        write_fm33_mem(client, 0x230d, 0x0100);		//For MCLK setting 24MHz ext clk
        write_fm33_mem(client, 0x22f5, 0x0003);		//For MCLK setting 24MHz ext clk
        write_fm33_mem(client, 0x2300, 0x0000);		//For MCLK setting 24MHz ext clk
		write_fm33_mem(client, 0x22fb, 0x0000);		//To Start the chip (To Run the DSP)
	#endif
	
	#if  1 //2010.10.15
		write_fm33_mem(client, 0x22F8 , 0x8002); 	//For MCLK setting 24MHz ext clk
		write_fm33_mem(client, 0x22D2 , 0x8000); 	//For MCLK setting 24MHz ext clk
		write_fm33_mem(client, 0x22F9 , 0x007F); 	//For MCLK setting 24MHz ext clk
		write_fm33_mem(client, 0x22EE , 0x0001); 	//For MCLK setting 24MHz ext clk
		write_fm33_mem(client, 0x22FA , 0x0003); 	//For MCLK setting 24MHz ext clk
		write_fm33_mem(client, 0x22F5 , 0x0003); 	//For MCLK setting 24MHz ext clk
		write_fm33_mem(client, 0x22C8 , 0x0017); 	//For MCLK setting 24MHz ext clk
		write_fm33_mem(client, 0x230D , 0x0060); 	//For MCLK setting 24MHz ext clk
		write_fm33_mem(client, 0x230C , 0x0180); 	//For MCLK setting 24MHz ext clk
		write_fm33_mem(client, 0x2305 , 0x206E); 	//For MCLK setting 24MHz ext clk
		write_fm33_mem(client, 0x2304 , 0x01FF); 	//For MCLK setting 24MHz ext clk
		write_fm33_mem(client, 0x2303 , 0x2DA1); 	//For MCLK setting 24MHz ext clk
		write_fm33_mem(client, 0x23E2 , 0x4000); 	//For MCLK setting 24MHz ext clk
		write_fm33_mem(client, 0x23E3 , 0x4000); 	//For MCLK setting 24MHz ext clk
		write_fm33_mem(client, 0x23E4 , 0x4000); 	//For MCLK setting 24MHz ext clk
		write_fm33_mem(client, 0x2308 , 0x0400); 	//For MCLK setting 24MHz ext clk
		write_fm33_mem(client, 0x2307 , 0x0000); 	//For MCLK setting 24MHz ext clk
		write_fm33_mem(client, 0x22FB , 0x0000); 	//To Start the chip (To Run the DSP)
	#endif

#endif

        msleep(200);
	 fm33_data->mode = 1;
}

#ifdef	SETTING_NC
void fm33_set_normalmode_reciever(struct i2c_client *client)
{
	printk(KERN_INFO "chk %s, \n", __FUNCTION__);
	fm33_reset();

	write_fm33_mem(client, 0x22F8 , 0x8002);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x22D2 , 0x8000);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x22F9 , 0x007F);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x22EE , 0x0001);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x22FA , 0x0003);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x22F5 , 0x0003);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x22C8 , 0x0017);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x230D , 0x0060);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x230C , 0x0180);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x2305 , 0x206E);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x2304 , 0x01FF);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x2303 , 0x2DA1);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x23E2 , 0x4000);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x23E3 , 0x4000);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x23E4 , 0x4000);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x2308 , 0x0580);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x2307 , 0x0000);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x22FB , 0x0000);	//To Start the chip (To Run the DSP)

	msleep(200);
	 fm33_data->mode = 1;
}


void fm33_set_normalmode_speaker(struct i2c_client *client)
{
	printk(KERN_INFO "chk %s, \n", __FUNCTION__);
	fm33_reset();

	write_fm33_mem(client, 0x230C , 0x0180);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x2301 , 0x0002);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x2307 , 0x0000);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x22F8 , 0x8001);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x230D , 0x0100);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x22C8 , 0x0017);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x2310 , 0x1201);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x22F9 , 0x007F);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x22EE , 0x0000);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x22F5 , 0x0003);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x22D2 , 0x8000);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x2328 , 0x7FFF);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x2304 , 0x13DC);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x2309 , 0x0400);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x23E7 , 0x23E7);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x23E8 , 0x4F9B);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x23E9 , 0x3512);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x23EA , 0x6895);	//To Start the chip (To Run the DSP)
	write_fm33_mem(client, 0x2326 , 0x0044);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x2325 , 0x3000);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x2327 , 0x1000);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x23D5 , 0x1000);	//For MCLK setting 24MHz ext clk
	write_fm33_mem(client, 0x22FB , 0x0000);	//For MCLK setting 24MHz ext clk

	msleep(200);
	 fm33_data->mode = 1;
}
#endif//SETTING_NC


/***************************************************************
* Function: For setting FM33 Noise Cancellation IC in Bypass mode.
***************************************************************/
void fm33_set_bpmode(struct i2c_client *client)
{
        printk(KERN_INFO "chk %s, \n", __FUNCTION__);
	fm33_reset();

	write_fm33_mem(client, 0x22c8, 0x0017);		//For MCLK setting 24MHz ext clk 
	write_fm33_mem(client, 0x22d2, 0x8000);		//For I2S mode setting
	write_fm33_mem(client, 0x22f5, 0x0003);		//For setting bypass mode
	write_fm33_mem(client, 0x22fb, 0x0000);		//To Start the chip (To Run the DSP)
	
	msleep(200);
	gpio_set_value(GPIO_FM33_BP, 0);		//Pulling BP line Low for BYPASS mode
}

static int fm33_suspend(struct i2c_client *client ,pm_message_t mesg)
{

	//gpio_set_value(GPIO_FM33_PWDN, 0);		//set PWDN PIN High

	return 0;

}
static int fm33_resume(struct i2c_client *client)
{

#ifdef SETTING_NC
	switch(gl_path_num)
	{
		case CALL_RCV:
			//fm33_set_normalmode_reciever(client);
			fm33_set_bpmode(client);
			break;
			
		case CALL_SPK:
			//fm33_set_normalmode_speaker(client);
			fm33_set_bpmode(client);
			break;

		case CALL_HP:
			fm33_set_normalmode(client);
			break;
			
		default:
			fm33_set_bpmode(client);
			//fm33_set_normalmode(client);
			break;
	}
#else//SETTING_NC
	fm33_set_bpmode(client);
	//fm33_set_normalmode(client);
#endif//SETTING_NC
	return 0;

}
/***************************************************************
* Function: Initial function for setup for the Module.
***************************************************************/
int fm33_probe(struct i2c_client *client, const struct i2c_device_id *i2c_id)
{
	u16 val = 0; //fixed 69337 (UNINIT) prevent defect : u16 val;
	int ret = 0;
	static int fm33_reset_count = 0;
	struct device *dev;

	printk(KERN_INFO "chk %s, \n", __FUNCTION__);

	fm33_data = kzalloc(sizeof(struct fm33_data), GFP_KERNEL);
	if (!fm33_data )	{
		printk(KERN_INFO "chk %s, ERROR - cannot allcoate memory\n", __FUNCTION__);
		return -ENOMEM;
	}

	fm33_data->client = client;
	fm33_data->dev	= &client->dev;

#ifdef	SETTING_NC
	gl_fm33_client = fm33_data->client;
#endif//SETTING_NC

	do{
		//fm33_set_normalmode(fm33_data->client);
		fm33_set_bpmode(fm33_data->client);
		
		fm33_reset_count++;
                ret = read_fm33_mem(fm33_data->client, 0x22fb, &val);
		if(ret < 0)	{
			printk(KERN_INFO "%s: Read from FM33 failed\n",__FUNCTION__);
			return ret;
		}
                printk(KERN_INFO "chk %s,FM33 SETUP IN PROGRESS val [0x%x] ret [%d] fm33_reset_count %d\n", __FUNCTION__,val, ret, fm33_reset_count);
	} while(val != 0x5a5a);

	printk(KERN_INFO "chk %s, FM33 Started Successfully\n", __FUNCTION__);
	
	fm33_class = class_create( THIS_MODULE, "fm33_class");
	if(IS_ERR(fm33_class))	{
		printk(KERN_INFO "Function: %s, Unable to create a class for FM33 Module\n", __FUNCTION__);
		return -1;
	}
	dev = device_create( fm33_class, NULL, 0, NULL, "fm33_dev");
	if(IS_ERR(dev))	{
		printk(KERN_INFO "Function: %s, Unable to create device for FM33 Module\n", __FUNCTION__);
		return -1;
	}
	if (device_create_file(dev, &fm33_attributes[0]))	{	
		printk(KERN_WARNING "CHK: %s : error unable to attribute for FM33\n", __FUNCTION__);
		return -1;
	}

	return 0;
	
}

/**************************************************************
Function: Calling during remove.
**************************************************************/

int fm33_remove(struct i2c_client *client)
{
	printk(KERN_INFO "chk %s\n", __FUNCTION__);
	kfree(fm33_data);
	return 0;
}

/***************************************************************
* Function: Initial start up time I2C setup.
***************************************************************/
int __init fm33_init()
{

	if(HWREV < 0x08)
	{
		printk(KERN_INFO "chk %s\n", __FUNCTION__);
		fm33_gpio_init();

		if( i2c_add_driver(&fm33_i2c_driver))	{
			printk("i2c_add_driver failed \n");
			return -ENODEV;
		}
	}

	return 0;
}

/***************************************************************
* Function : Power of time driver unload.
***************************************************************/
void __exit fm33_exit(void)
{
	fm33_powerdown();	//chk: Pull down the POWER line
	i2c_del_driver(&fm33_i2c_driver);
};

module_init(fm33_init);
module_exit(fm33_exit);

MODULE_DESCRIPTION("FM33 Noise Cancellation Module driver");
MODULE_AUTHOR("xyz@samsung.com");
MODULE_LICENSE("GPL");
