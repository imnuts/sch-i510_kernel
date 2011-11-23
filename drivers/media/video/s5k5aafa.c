/*
 * Driver for S5K5AAFA (UXGA camera) from Samsung Electronics
 * 
 * 1/4" 2.0Mp CMOS Image Sensor SoC with an Embedded Image Processor
 *
 * Copyright (C) 2009, Jinsung Yang <jsgood.yang@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-i2c-drv.h>
#include <media/s5k5aafa_platform.h>
#include <linux/regulator/consumer.h>
#ifdef FEATURE_CAMERA_TUNING_MODE
#include <linux/fs.h>
#include <linux/mm.h>
#endif


#ifdef CONFIG_VIDEO_SAMSUNG_V4L2
#include <linux/videodev2_samsung.h>
#endif

#include "s5k5aafa.h"

#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <plat/regs-fimc.h>
#include <mach/regs-gpio.h>
#include <mach/regs-clock.h>

//#define VT_CAM_DEBUG

//#define FEATURE_CAMERA_TUNING_MODE
#define MAX_REG_TABLE_LEN 1024

#define FALSE   0
#define TRUE    1

#define FRONTCAMERA_PREVIEW_WIDTH   640
#define FRONTCAMERA_PREVIEW_HEIGHT  480

#define FRONTCAMERA_CAPTURE_WIDTH   1280
#define FRONTCAMERA_CAPTURE_HEIGHT  960

#define DEFINE_SECTION(name) { #name, name, sizeof(name)/sizeof(u16) } 

#ifdef VT_CAM_DEBUG
#define k5aa_msg     dev_err
#else
#define k5aa_msg     dev_dbg
#endif

#define DEFAULT_RES		WVGA	/* Index of resoultion */
#define DEFAUT_FPS_INDEX	S5K5AAFA_15FPS
#define DEFAULT_FMT		V4L2_PIX_FMT_UYVY	/* YUV422 */

#define S5K5AAFA_DRIVER_NAME	"S5K5AAFA"

#ifdef FEATURE_CAMERA_TUNING_MODE
typedef enum
{
    CAMSENSOR_TUNING_MODE_OFF,
    CAMSENSOR_TUNING_MODE_ON,
    CAMSENSOR_TUNING_MODE_MAX,
} camsensor_tuning_mode_type;

static camsensor_tuning_mode_type camsensor_tuning_mode = CAMSENSOR_TUNING_MODE_ON;

static u16 reg_table[MAX_REG_TABLE_LEN];
static u16 reg_table_len = 0;
static char*  curr_buf_ptr = 0;
#endif

static u16 threshold_table[CAM_THRESHOLD_MAX];
static u16 capture_delay_table[CAPTURE_DELAY_MAX];

extern unsigned int HWREV;

/*
 * Specification
 * Parallel : ITU-R. 656/601 YUV422, RGB565, RGB888 (Up to VGA), RAW10 
 * Serial : MIPI CSI2 (single lane) YUV422, RGB565, RGB888 (Up to VGA), RAW10
 * Resolution : 1280 (H) x 1024 (V)
 * Image control : Brightness, Contrast, Saturation, Sharpness, Glamour
 * Effect : Mono, Negative, Sepia, Aqua, Sketch
 * FPS : 15fps @full resolution, 30fps @VGA, 24fps @720p
 * Max. pixel clock frequency : 48MHz(upto)
 * Internal PLL (6MHz to 27MHz input frequency)
 */

static int s5k5aafa_init(struct v4l2_subdev *sd, u32 val);		//for fixing build error	//s1_camera [ Defense process by ESD input ]

/* Camera functional setting values configured by user concept */
struct s5k5aafa_userset {
	signed int exposure_bias;	/* V4L2_CID_EXPOSURE */
	unsigned int ae_lock;
	unsigned int awb_lock;
	unsigned int auto_wb;	/* V4L2_CID_AUTO_WHITE_BALANCE */
	unsigned int manual_wb;	/* V4L2_CID_WHITE_BALANCE_PRESET */
	unsigned int wb_temp;	/* V4L2_CID_WHITE_BALANCE_TEMPERATURE */
	unsigned int effect;	/* Color FX (AKA Color tone) */
	unsigned int contrast;	/* V4L2_CID_CONTRAST */
	unsigned int saturation;	/* V4L2_CID_SATURATION */
	unsigned int sharpness;		/* V4L2_CID_SHARPNESS */
	unsigned int glamour;
};

struct s5k5aafa_state {
        struct s5k5aafa_platform_data *pdata;
        struct v4l2_subdev sd;
        struct v4l2_pix_format pix;
        struct v4l2_fract timeperframe;
        struct s5k5aafa_userset userset;
        int framesize_index;
        int freq;	/* MCLK in KHz */
        int is_mipi;
        int isize;
        int ver;
        int fps;
        int vt_mode; /*For VT camera*/
        int capture_mode;
        int check_dataline;
        int check_previewdata;
        int camcorder_mode;
};

enum {
	S5K5AAFA_PREVIEW_VGA,
} S5K5AAFA_FRAME_SIZE;

struct s5k5aafa_enum_framesize {
	unsigned int index;
	unsigned int width;
	unsigned int height;	
};

struct s5k5aafa_enum_framesize s5k5aafa_framesize_list[] = {
	{S5K5AAFA_PREVIEW_VGA, 640, 480}
};

static inline struct s5k5aafa_state *to_state(struct v4l2_subdev *sd)
{
	return container_of(sd, struct s5k5aafa_state, sd);
}

//s1_camera [ Defense process by ESD input ] _[
#if 1
static struct regulator *s5k5aafa_vga_dvdd;
static struct regulator *s5k5aafa_vga_vddio;
static struct regulator *s5k5aafa_cam_isp_host;
static struct regulator *s5k5aafa_cam_isp_core;

static int s5k5aafa_regulator_init(struct v4l2_subdev *sd)
{
        struct i2c_client *client = v4l2_get_subdevdata(sd);

/*BUCK 4*/
	if (IS_ERR_OR_NULL(s5k5aafa_cam_isp_core)) {
		s5k5aafa_cam_isp_core = regulator_get(NULL, "cam_isp_core");
		if (IS_ERR_OR_NULL(s5k5aafa_cam_isp_core)) {
			pr_err("failed to get cam_isp_core regulator");
			return -EINVAL;
		}
	}
/*ldo 13*/
	if (IS_ERR_OR_NULL(s5k5aafa_vga_vddio)) {
		s5k5aafa_vga_vddio = regulator_get(NULL, "vga_vddio");
		if (IS_ERR_OR_NULL(s5k5aafa_vga_vddio)) {
			pr_err("failed to get vga_vddio regulator");
			return -EINVAL;
		}
	}
/*ldo 14*/
	if (IS_ERR_OR_NULL(s5k5aafa_vga_dvdd)) {
		s5k5aafa_vga_dvdd = regulator_get(NULL, "vga_dvdd");
		if (IS_ERR_OR_NULL(s5k5aafa_vga_dvdd)) {
			pr_err("failed to get vga_dvdd regulator");
			return -EINVAL;
		}
	}
/*ldo 15*/
	if (IS_ERR_OR_NULL(s5k5aafa_cam_isp_host)) {
		s5k5aafa_cam_isp_host = regulator_get(NULL, "cam_isp_host");
		if (IS_ERR_OR_NULL(s5k5aafa_cam_isp_host)) {
			pr_err("failed to get cam_isp_host regulator");
			return -EINVAL;
		}
	}

	k5aa_msg(&client->dev,"s5k5aafa_cam_isp_core = %p\n", s5k5aafa_cam_isp_core);
	k5aa_msg(&client->dev,"s5k5aafa_vga_vddio = %p\n", s5k5aafa_vga_vddio);
	k5aa_msg(&client->dev,"s5k5aafa_vga_dvdd = %p\n", s5k5aafa_vga_dvdd);
	k5aa_msg(&client->dev,"s5k5aafa_cam_isp_host = %p\n", s5k5aafa_cam_isp_host);
	return 0;
}

static void s5k5aafa_regulator_off(struct v4l2_subdev *sd)
{
        struct i2c_client *client = v4l2_get_subdevdata(sd);
        int err = 0;
        
        /* ldo 13 */
        err = regulator_disable(s5k5aafa_vga_vddio);
        if (err) {
                k5aa_msg(&client->dev,"Failed to disable regulator cam_vga_vddio\n");
        }
        /* ldo 14 */
        err = regulator_disable(s5k5aafa_vga_dvdd);
        if (err) {
                k5aa_msg(&client->dev,"Failed to disable regulator cam_vga_dvdd\n");
        }
        /* ldo 15 */
        err = regulator_disable(s5k5aafa_cam_isp_host);
        if (err) {
                k5aa_msg(&client->dev,"Failed to disable regulator cam_isp_core\n");
        }
        /* BUCK 4 */
        err = regulator_disable(s5k5aafa_cam_isp_core);
        if (err) {
                k5aa_msg(&client->dev,"Failed to disable regulator cam_isp_core\n");
        }
}

static int s5k5aafa_power_on(struct v4l2_subdev *sd)
{   
        int err = 0;

        if (s5k5aafa_regulator_init(sd)) {
                pr_err("Failed to initialize camera regulators\n");
                return -EINVAL;
        }

        /* CAM_MEGA_nRST - GPJ1(5) */
        err = gpio_request(GPIO_CAM_MEGA_nRST, "GPJ1");
        if(err) {
                printk(KERN_ERR "failed to request GPJ1 for camera control\n");
                return err;
        }

        /* CAM_VT_EN - GPJ(0)  */
        err = gpio_request(GPIO_CAM_VT_EN_28V, "GPJ4");
        if (err) {
                printk(KERN_ERR "failed to request GPB0 for camera control\n");
                return err;
        }

        /* GPIO_CAM_VT_RST_28V - GPJ(0)  */
        err = gpio_request(GPIO_CAM_VT_RST_28V, "GPJ4");
        if (err) {
                printk(KERN_ERR "failed to request GPB0 for camera control\n");
                return err;
        }

        // CAM_MEGA_nRST LOW
        gpio_direction_output(GPIO_CAM_MEGA_nRST, 0);
        gpio_set_value(GPIO_CAM_MEGA_nRST, 0);

        // Power on Seqeunce

        /* Turn CAM_ISP_CORE_1.2V(VDD_REG) on BUCK 4*/
        err = regulator_enable(s5k5aafa_cam_isp_core);
        if (err) {
                pr_err("Failed to enable regulator cam_isp_core\n");
                goto off;
        }
        mdelay(1);

        /* CAM_1.3M_nSTBY  HIGH */
        gpio_direction_output(GPIO_CAM_VT_EN_28V, 1);
        gpio_set_value(GPIO_CAM_VT_EN_28V, 1);

        /* CAM_DVDD_1.5V on ldo 14*/
        err = regulator_enable(s5k5aafa_vga_dvdd);
        if (err) {
                pr_err("Failed to enable regulator cam_isp_core\n");
                goto off;
        }

        /* CAM_SENSOR_A2.8V */
        if(HWREV>=8){
                if(gpio_request(GPIO_CAM_IO_EN, "GPB") != 0){
                        printk(KERN_ERR "failed to request GPB7 for camera control = %d\n",HWREV);
                        return err;
                }
                        gpio_direction_output(GPIO_CAM_IO_EN, 0);
                        gpio_set_value(GPIO_CAM_IO_EN, 1);                
        }else{
                /* CAM_SENSOR_A2.8V on ldo 13*/
                err = regulator_enable(s5k5aafa_vga_vddio);
                if (err) {
                        pr_err("Failed to enable regulator cam_isp_core\n");
                        goto off;
                }
        }

        /* CAM_HOST_1.8V on ldo 15*/
        err = regulator_enable(s5k5aafa_cam_isp_host);
        if (err) {
                pr_err("Failed to enable regulator cam_isp_core\n");
                goto off;
        }
        udelay(200);

        // Mclk enable
        s3c_gpio_cfgpin(GPIO_CAM_MCLK, S5PV210_GPE1_3_CAM_A_CLKOUT);

        mdelay(10);

        // CAM_VGA_nRST  HIGH       
        gpio_direction_output(GPIO_CAM_VT_RST_28V, 0);
        gpio_set_value(GPIO_CAM_VT_RST_28V, 1);

        mdelay(80);

        gpio_free(GPIO_CAM_MEGA_nRST);
        gpio_free(GPIO_CAM_VT_EN_28V);
        gpio_free(GPIO_CAM_VT_RST_28V); 

        if(HWREV >= 8)
                gpio_free(GPIO_CAM_IO_EN);

        mdelay(5);
        
        return 0;
off:
        s5k5aafa_regulator_off(sd);

        gpio_free(GPIO_CAM_MEGA_nRST);
        gpio_free(GPIO_CAM_VT_EN_28V);
        gpio_free(GPIO_CAM_VT_RST_28V); 

        if(HWREV >= 8)
                gpio_free(GPIO_CAM_IO_EN);

        mdelay(5);
        
        return 0;
}


static int s5k5aafa_power_off(struct v4l2_subdev *sd)
{
        int err = 0;
        
        printk(KERN_DEBUG "s5k5aafa_power_off\n");

        //OFF SEQUENCE : STBYN->RSTN->MCLK->LDO14->LDO13->LDO15    
        /* CAM_VT_EN - GPJ(0)  */
        err = gpio_request(GPIO_CAM_VT_EN_28V, "GPJ4");
        if (err) {
                printk(KERN_ERR "failed to request GPB0 for camera control\n");
                return err;
        }

        /* GPIO_CAM_VT_RST_28V - GPJ(0)  */
        err = gpio_request(GPIO_CAM_VT_RST_28V, "GPJ4");
        if (err) {
                printk(KERN_ERR "failed to request GPB0 for camera control\n");
                return err;
        }

        /* CAM_1.3M_nSTBY  LOW  */     
        gpio_direction_output(GPIO_CAM_VT_EN_28V, 0);
        gpio_set_value(GPIO_CAM_VT_EN_28V, 0);
        mdelay(1);

        // CAM_VT_RST  HIGH       
        gpio_direction_output(GPIO_CAM_VT_RST_28V, 0);
        gpio_set_value(GPIO_CAM_VT_RST_28V, 0); 
        mdelay(1);   

        // Mclk enable
        s3c_gpio_cfgpin(GPIO_CAM_MCLK, 0);
        mdelay(1);
    
        /* CAM_HOST_1.8V - ldo 15*/
        if(!IS_ERR_OR_NULL(s5k5aafa_cam_isp_host)){  
            err = regulator_disable(s5k5aafa_cam_isp_host);
            if (err) {
                    pr_err("Failed to disable regulator cam_isp_core\n");
            }
        }
        /* CAM_SENSOR_A2.8V */
        if(HWREV>=8){
                if(gpio_request(GPIO_CAM_IO_EN, "GPB") != 0){
                        printk(KERN_ERR "failed to request GPB7 for camera control = %d\n",HWREV);
                        return err;
                }            
                gpio_direction_output(GPIO_CAM_IO_EN, 1);    
                gpio_set_value(GPIO_CAM_IO_EN, 0);
        }else{
                /* ldo 13 */
                if(!IS_ERR_OR_NULL(s5k5aafa_vga_vddio)){  
                    err = regulator_disable(s5k5aafa_vga_vddio);
                    if (err) {
                            pr_err("Failed to disable regulator cam_vga_vddio\n");
                    }
                }
        }    

        /* CAM_DVDD_1.5V -ldo 14*/
        if(!IS_ERR_OR_NULL(s5k5aafa_vga_dvdd)){  
            err = regulator_disable(s5k5aafa_vga_dvdd);
            if (err) {
                    pr_err("Failed to disable regulator cam_vga_dvdd\n");
            }
        }
        
        /* CAM_ISP_CORE_1.2V - buck4*/
        if(!IS_ERR_OR_NULL(s5k5aafa_cam_isp_core)){          
            err = regulator_disable(s5k5aafa_cam_isp_core);
            if (err) {
                    pr_err("Failed to disable regulator cam_isp_core\n");
            }
        }
        
        gpio_free(GPIO_CAM_VT_EN_28V);
        gpio_free(GPIO_CAM_VT_RST_28V); 

        if(HWREV >= 8)
                gpio_free(GPIO_CAM_IO_EN);

        return 0;
}


static int s5k5aafa_power_en(struct v4l2_subdev *sd, int onoff)
{
       struct i2c_client *client = v4l2_get_subdevdata(sd);

        k5aa_msg(&client->dev,"CameraDriver-s5k5aafa_power_en\n");

        if(onoff){
                s5k5aafa_power_on(sd);
        } else {
                s5k5aafa_power_off(sd);
        }
        return 0;
}


static int s5k5aafa_reset(struct v4l2_subdev *sd)
{
	s5k5aafa_power_en(sd, 0);
	mdelay(5);
	s5k5aafa_power_en(sd, 1);
	mdelay(5);
	s5k5aafa_init(sd, 0);
	return 0;
}
// _]
#endif

int s5k5aafa_i2c_read_byte(struct v4l2_subdev *sd, u8 addr, u8 *data)
{
    int err = -1;
    int retry_count = 1;
    u8 buf[2];
    u8 addr_buf[2];
    
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct i2c_msg msg ={ client->addr, 0, 1, addr_buf};

    addr_buf[0] = addr;
    addr_buf[1] = 0;
    
    err = i2c_transfer(client->adapter, &msg, 1);
    if (unlikely(err < 0))
    {
         k5aa_msg(&client->dev, "%s: register read fail\n", __func__); 
         return -1;
    }

    msg.flags = I2C_M_RD;
    //msg.buf = addr_buf;
    
    err = i2c_transfer(client->adapter, &msg, 1);
    if (unlikely(err < 0))
    {
         k5aa_msg(&client->dev, "%s: register read fail\n", __func__); 
         return -1;
    }

    k5aa_msg(&client->dev, " R : %02x, %02x\n",addr_buf[0], addr_buf[1]);
    *data = addr_buf[0];
    return 0;
}

int s5k5aafa_i2c_write_byte(struct v4l2_subdev *sd, u8 addr, u8 data)
{
    int ret = -1;
    int retry_count = 1;
    u8 buf[2];
    
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct i2c_msg msg ={ client->addr, 0, 1, buf};

    buf[0] = addr;
    buf[1] = data;
    
    while(retry_count--){
    	ret  = i2c_transfer(client->adapter, &msg, 1);
    	if(ret == 1)
    		break;
    	msleep(10);
    }

    k5aa_msg(&client->dev, " W : Addr %02x, Data %02x\n",buf[0],buf[1]);
    
    return 0;

}

int s5k5aafa_i2c_write(struct v4l2_subdev *sd, unsigned char i2c_data[],
				unsigned char length)
{
	int ret = -1;
	int retry_count = 1;
	
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	unsigned char buf[length], i;
	struct i2c_msg msg = {client->addr, 0, length, buf};

	for (i = 0; i < length; i++) {
		buf[i] = i2c_data[i];
	}
	
#ifdef VGA_CAM_DEBUG
	printk("i2c cmd Length : %d\n", length);
	for (i = 0; i < length; i++) {
		printk("buf[%d] = %x  ", i, buf[i]);
		if(i == length)
			printk("\n");
	}
#endif

	while(retry_count--){
		ret  = i2c_transfer(client->adapter, &msg, 1);
		if(ret == 1)
			break;
		msleep(10);
	}

	return (ret == 1) ? 0 : -EIO;
}

static int s5k5aafa_write_regs(struct v4l2_subdev *sd, unsigned char regs[], 
				int size)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int i, err;

	for (i = 0; i < size; i++) {
		err = s5k5aafa_i2c_write(sd, &regs[i], sizeof(regs[i]));
		if (err < 0)
		{
			v4l_info(client, "%s: register set failed\n", \
			__func__);
			
			break;
		}
	}
	if(err < 0)
		return -EIO;	

	return 0;	/* FIXME */
}

static struct v4l2_queryctrl s5k5aafa_controls[] = {
	{
		.id = V4L2_CID_AUTO_WHITE_BALANCE,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.name = "Auto white balance",
		.minimum = 0,
		.maximum = 1,
		.step = 1,
		.default_value = 0,
	},
	{
		.id = V4L2_CID_EXPOSURE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "Exposure bias",
		.minimum = EV_MINUS_4,
		.maximum = EV_PLUS_4,
		.step = 1,
		.default_value = EV_DEFAULT,	/* 0 EV */
	},

};

static reg_section_table_type reg_section[SECTION_MAX] = // reg_config_section_type과 순서 맞출것
{
    DEFINE_SECTION(MCLK_TABLE),

    /* INITIALIZE */
    DEFINE_SECTION(INIT_DATA),
    DEFINE_SECTION(INIT_VT_DATA),
    /* DTP */
    DEFINE_SECTION(DTP_DATA),
    /* OP MODE */
    DEFINE_SECTION(OP_PREVIEW_DATA),
    DEFINE_SECTION(OP_NORMAL_LIGHT_PREVIEW_DATA),
    DEFINE_SECTION(OP_HIGH_LIGHT_PREVIEW_DATA),
    
    DEFINE_SECTION(OP_CAPTURE_DATA),  
    DEFINE_SECTION(OP_MIDLIGHT_CAPTURE_DATA),
    DEFINE_SECTION(OP_MID_LOW_LIGHT_CAPTURE_DATA),
    DEFINE_SECTION(OP_LOW_LIGHT_CAPTURE_DATA), 
    
    DEFINE_SECTION(OP_CAMCORDER_DATA),
    /*BRIGHTNESS_EV */
    DEFINE_SECTION(EV_MINUS_4_DATA),  
    DEFINE_SECTION(EV_MINUS_3_DATA),  
    DEFINE_SECTION(EV_MINUS_2_DATA),  
    DEFINE_SECTION(EV_MINUS_1_DATA),  
    DEFINE_SECTION(EV_0_DATA),  
    DEFINE_SECTION(EV_PLUS_1_DATA),  
    DEFINE_SECTION(EV_PLUS_2_DATA),  
    DEFINE_SECTION(EV_PLUS_3_DATA),  
    DEFINE_SECTION(EV_PLUS_4_DATA),  
    /* WHITE BALANCE */
    DEFINE_SECTION(WB_AUTO_DATA),  
    DEFINE_SECTION(WB_TUNGSTEN_DATA),  
    DEFINE_SECTION(WB_FLUORESCENT_DATA),  
    DEFINE_SECTION(WB_SUNNY_DATA),  
    DEFINE_SECTION(WB_CLOUDY_DATA),  
    /* COLOR EFFECT */
    DEFINE_SECTION(EFFECT_NORMAL_DATA),  
    DEFINE_SECTION(EFFECT_B_AND_W_DATA),  
    DEFINE_SECTION(EFFECT_NEGATIVE_DATA),  
    DEFINE_SECTION(EFFECT_SEPIA_DATA),  
    DEFINE_SECTION(EFFECT_GREEN_DATA),  
    DEFINE_SECTION(EFFECT_AQUA_DATA),  

    /* SHADE */
    DEFINE_SECTION(SHADE_DNP_DATA),
    DEFINE_SECTION(SHADE_ETC_DATA),

    DEFINE_SECTION(SET_THRESHOLD_DATA),

    /* CAPTURE DELAY TIME */
    DEFINE_SECTION(CAPTURE_DELAY_TIME_DATA),
};

#ifdef FEATURE_CAMERA_TUNING_MODE
void camsensor_toggle_tuning_mode(void)
{
    camsensor_tuning_mode++;

    if(camsensor_tuning_mode == CAMSENSOR_TUNING_MODE_MAX)
    {
        camsensor_tuning_mode = CAMSENSOR_TUNING_MODE_OFF;
    }
}

int camsensor_get_tuning_mode(void)
{
    return camsensor_tuning_mode;
}

static u16 camsensor_config_get_hex_val(char hex)
{
    if ( hex >= 'a' && hex <= 'f' )
        return (hex-'a'+10);
    else if ( hex >= 'A' && hex <= 'F' )
        return (hex - 'A' + 10 );
    else if ( hex >= '0' && hex <= '9' )
        return (hex - '0');
    else
        return 0;
}

static u16 camsensor_config_atoi(char* str)
{
    u16 i, j=0;
    u16 val_len;
    u16 ret_val=0;

    if ( str == NULL )
        return 0;
    val_len = strlen(str);

    //decimal
    if (    val_len < 4 
    ||  ( strstr(str, "0x")==NULL && strstr(str, "0X")==NULL )
    )
    {
        while((*str != '\0')&&(*str >= '0' && *str <= '9'))
        {
            ret_val = ret_val*10+((int)*str - (int) '0');
            str++;
        }
        //return atoi(str);
        return ret_val;
    }

    //hex ex:0xa0c
    //val_len = strlen(str);

    for ( i = val_len-1 ; i >= 2 ; i-- )
    {
        ret_val = ret_val + (camsensor_config_get_hex_val(str[i])<<(j*4));
        j++;
    }

    return ret_val;
}

static void camsensor_config_init_table(void)
{
    memset(reg_table, 0, MAX_REG_TABLE_LEN);
    reg_table_len = 0;
}

static boolean camsensor_config_get_line(struct v4l2_subdev *sd, char* buffer, char* line)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct m5moLS_state *state = to_state(sd); 
    int      i;
    char*    _r_n_ptr;

    memset(line, 0, 1024);

    _r_n_ptr = strstr(curr_buf_ptr, "\n");

    //\n? ?? ??
    if ( _r_n_ptr )
    {
        for ( i = 0 ; ; i++ )
        {
            if ( curr_buf_ptr + i == _r_n_ptr )
            {
                curr_buf_ptr = _r_n_ptr + 1;
                break;
            }
            line[i] = curr_buf_ptr[i];
        }
        line[i] = '\0';

        return TRUE;
    }
    //\n? ?? ??
    else
    {
        if ( strlen(curr_buf_ptr) > 0 )
        {
            strcpy(line, curr_buf_ptr);
            return TRUE;
        }
        else
            return FALSE;
    }

    return FALSE;
}

static boolean camsensor_config_trim(char* buff)
{
    int   left_index;
    int   right_index;
    int   buff_len;
    int   i;

    buff_len  = strlen(buff);
    left_index  = -1;
    right_index = -1;

    if ( buff_len == 0 )
        return FALSE;

    /* left index(???? ???? white space? ?? ??)? ???. */
    for ( i = 0 ; i < buff_len ; i++ )
    {
        if ( buff[i] != ' ' && buff[i] != '\t' && buff[i] != '\n' && buff[i] != '\r')
        {
            left_index = i;
            break;
        }
    }

    /* right index(??? ???? white space? ?? ??)? ???. */
    for ( i = buff_len-1 ; i >= 0 ; i-- )
    {
        if ( buff[i] != ' ' && buff[i] != '\t' && buff[i] != '\n' && buff[i] != '\r')
        {
            right_index = i;
            buff[i+1] = '\0';
            break;
        }
    }

    if ( left_index == -1 && right_index == -1 )
        strcpy(buff, "");
    else if ( left_index <= right_index )
        strcpy(buff, buff+left_index);
    else
        return FALSE;

    return TRUE;
}

static u32 camsensor_config_parse_table(struct v4l2_subdev *sd,char* buffer)
{
    int    i;
    char   line[1024];      
    char   reg_val_str[11];
    int    reg_val_str_idx;
    u16 reg_val;
    u16 reg_index = 0;

    while (camsensor_config_get_line(sd, buffer, line))
    {
        if(camsensor_config_trim(line) == FALSE)
        {
            continue;
        }

        if ( line[0] == '}' )
        {
            return reg_index;
        }

        if ( strlen(line) == 0 || (line[0] == '/' && line[1] == '/' ) || (line[0] == '/' && line[1] == '*' ) || line[0] == '{' )
        {
            continue;
        }

        reg_val_str_idx = 0;

        for (i = 0 ; ; i++)
        {
            if ( line[i] == ' ' || line[i] == '\t' )
                continue;

            if ( line[i] == '/' && line[i + 1] == '/' )
                break;

            if ( line[i] == ',' || line[i] == '\n' || line[i] == '\r' )
                break;

            reg_val_str[reg_val_str_idx++] = line[i];
        }

        reg_val_str[reg_val_str_idx] = '\0';

        reg_val = camsensor_config_atoi(reg_val_str);

        reg_table[reg_index++] = reg_val;
    }

    return reg_index;
}

static boolean camsensor_config_parse_section(char* line, reg_config_section_type section)
{
    if (strstr(line, reg_section[section].name) != NULL)
    {
        return TRUE;
    }

    return FALSE;
}

static boolean camsensor_config_make_table (struct v4l2_subdev *sd, reg_config_section_type section)
{
  char                line[1024];
  char*               file_buf = NULL;
  char*               end_ptr;
  unsigned int        file_size;
  mm_segment_t old_fs;
  int ret=0,j=0;
  u32 i =0;
  struct file *fp = NULL;
  struct i2c_client *client = v4l2_get_subdevdata(sd);
  struct m5moLS_state *state = to_state(sd);  

  k5aa_msg(&client->dev,"make_register_table - section no. : %d", section, 0, 0);

  if (0 == strcmp ("NULL", reg_section[section].name))
  {
    k5aa_msg(&client->dev,"make_register_table - not supported", 0, 0, 0);
    return -1;
  }
  
  camsensor_config_init_table();

  if(curr_buf_ptr != NULL)
  {
        kfree(curr_buf_ptr);
        curr_buf_ptr = NULL;
  }

  fp = filp_open("sdcard/sd/s5k5aafa.h", O_RDONLY, 0);  
  if (IS_ERR(fp)) 
  {
      k5aa_msg(&client->dev, "%s : file open error\n", __func__);
      return PTR_ERR(fp);
  }

  if(fp && (fp != 0xfffffffe))
  {
        old_fs = get_fs();
        set_fs(get_ds());
        
        file_size = fp->f_path.dentry->d_inode->i_size;
        fp->f_op->llseek(fp, 0, SEEK_SET);

        file_buf = (char*)kmalloc(file_size+1, GFP_KERNEL);
        if(file_buf == NULL)
        {
            filp_close(fp, current->files);
            set_fs(old_fs);
            k5aa_msg(&client->dev,"malloc is failed\n");
            return -1;
        }        
        ret = vfs_read(fp,(char __user *)file_buf, file_size, &fp->f_pos);
        if(ret != file_size)
        {
            if(file_buf != NULL)
            {
                kfree(file_buf);
                file_buf = NULL;
            }

            filp_close(fp, current->files);
            set_fs(old_fs);
            
            k5aa_msg(&client->dev," vfs_read is failed ::: ret = %d!!\n",ret);
            return -1;
        }     

  }
   curr_buf_ptr = file_buf;

    while (camsensor_config_get_line(sd, file_buf, line))
    {
        if (camsensor_config_parse_section(line, section))
        {
            reg_table_len = camsensor_config_parse_table(sd, file_buf);
            break;
        }
    }

   filp_close(fp, current->files);  
   set_fs(old_fs);
   return 0;
}
#endif

static void sensor_page_mode(struct v4l2_subdev *sd, u8 page)
{
    s5k5aafa_i2c_write_byte(sd,0xfc, page);
}

int camsensor_config_write(struct v4l2_subdev *sd,u16 sdata)
{
  u8 reg_addr,reg_data;
  int ret_val;
  unsigned char i2c_data[1][2]= {
    {(u8)((sdata & 0xff00)>>8),(u8)((sdata & 0x00ff)>>0)}
  };
  //k5aa_msg(&client->dev, " W: %04x\n",sdata);
  ret_val = s5k5aafa_i2c_write(sd, &i2c_data[0], sizeof(i2c_data[0]));

  return ret_val;
}

int camsensor_config_set_register(struct v4l2_subdev *sd,reg_config_section_type section)
    {
    u16 idx;
    int err = -EINVAL;
    struct i2c_client *client = v4l2_get_subdevdata(sd);
#ifdef FEATURE_CAMERA_TUNING_MODE
    if (camsensor_get_tuning_mode())
    {
        if (camsensor_config_make_table(sd,section) == 0)
        {
            for(idx = 0;idx < reg_table_len;idx++)
            {
                if(camsensor_config_write(sd,reg_table[idx]) < 0)
                {
                    return -1;
                } 
            }
            return 0;
        }
    }
    else
#endif
    if (reg_section[section].table && reg_section[section].table_len)
    {
        for(idx = 0;idx < reg_section[section].table_len;idx++)
        {
            err = camsensor_config_write(sd, reg_section[section].table[idx]);
            if(err < 0){
                k5aa_msg(&client->dev,"%s: failed\n",__func__);
                return err;
            }
        }
    }

    return err;
    }

u16 camsensor_config_get_data(struct v4l2_subdev *sd,reg_config_section_type section, u32 index)
{
#ifdef FEATURE_CAMERA_TUNING_MODE
    if (camsensor_get_tuning_mode())
    {
        if (camsensor_config_make_table(sd,section)==0)
        {
            return reg_table[index];
        }
    }
    else
#endif
    if (reg_section[section].table && reg_section[section].table_len)
    {
        return reg_section[section].table[index];
    }
    return 0xffff;
}

const char **s5k5aafa_ctrl_get_menu(u32 id)
{
	printk(KERN_DEBUG "s5k5aafa_ctrl_get_menu is called... id : %d \n", id);

	switch (id) {
#if 0	// temporary delete
	case V4L2_CID_WHITE_BALANCE_PRESET:
		return s5k5aafa_querymenu_wb_preset;

	case V4L2_CID_COLORFX:
		return s5k5aafa_querymenu_effect_mode;

	case V4L2_CID_EXPOSURE:
		return s5k5aafa_querymenu_ev_bias_mode;
#endif
	default:
		return v4l2_ctrl_get_menu(id);
	}
}

static inline struct v4l2_queryctrl const *s5k5aafa_find_qctrl(int id)
{
	int i;

	printk(KERN_DEBUG "s5k5aafa_find_qctrl is called...  id : %d \n", id);

	for (i = 0; i < ARRAY_SIZE(s5k5aafa_controls); i++)
		if (s5k5aafa_controls[i].id == id)
			return &s5k5aafa_controls[i];

	return NULL;
}

static int s5k5aafa_queryctrl(struct v4l2_subdev *sd, struct v4l2_queryctrl *qc)
{
	int i;

	printk(KERN_DEBUG "s5k5aafa_queryctrl is called... \n");

	for (i = 0; i < ARRAY_SIZE(s5k5aafa_controls); i++) {
		if (s5k5aafa_controls[i].id == qc->id) {
			memcpy(qc, &s5k5aafa_controls[i], \
				sizeof(struct v4l2_queryctrl));
			return 0;
		}
	}

	return -EINVAL;
}

static int s5k5aafa_querymenu(struct v4l2_subdev *sd, struct v4l2_querymenu *qm)
{
	struct v4l2_queryctrl qctrl;

	printk(KERN_DEBUG "s5k5aafa_querymenu is called... \n");

	qctrl.id = qm->id;
	s5k5aafa_queryctrl(sd, &qctrl);

	return v4l2_ctrl_query_menu(qm, &qctrl, s5k5aafa_ctrl_get_menu(qm->id));
}

/*
 * Clock configuration
 * Configure expected MCLK from host and return EINVAL if not supported clock
 * frequency is expected
 * 	freq : in Hz
 * 	flag : not supported for now
 */
static int s5k5aafa_s_crystal_freq(struct v4l2_subdev *sd, u32 freq, u32 flags)
{
	int err = -EINVAL;

	printk(KERN_DEBUG "s5k5aafa_s_crystal_freq is called... \n");

	return err;
}

static int s5k5aafa_g_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
	int err = 0;

	printk(KERN_DEBUG "s5k5aafa_g_fmt is called... \n");

	return err;
}

static int s5k5aafa_s_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
	int err = 0;

	struct  s5k5aafa_state *state = to_state(sd);
	state->pix.width = fmt->fmt.pix.width;	
	state->pix.height = fmt->fmt.pix.height;	
	state->pix.width = fmt->fmt.pix.width;	
	state->pix.height = fmt->fmt.pix.height;	
	state->pix.pixelformat = fmt->fmt.pix.pixelformat;	
	printk("%s : width - %d , height - %d\n", __func__, state->pix.width, state->pix.height);
	printk(KERN_DEBUG "s5k5aafa_s_fmt is called... \n");
	return 0;

	//return err;
}
static int s5k5aafa_enum_framesizes(struct v4l2_subdev *sd, \
					struct v4l2_frmsizeenum *fsize)
{
	struct  s5k5aafa_state *state = to_state(sd);

	printk(KERN_DEBUG "s5k5aafa_enum_framesizes is called... \n");

	/* The camera interface should read this value, this is the resolution
 	 * at which the sensor would provide framedata to the camera i/f
 	 *
 	 * In case of image capture, this returns the default camera resolution (WVGA)
 	 */
        fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;

        if(state->capture_mode == 1){
            fsize->discrete.width = FRONTCAMERA_CAPTURE_WIDTH;
            fsize->discrete.height =FRONTCAMERA_CAPTURE_HEIGHT;
        }else{
            fsize->discrete.width = FRONTCAMERA_PREVIEW_WIDTH;
            fsize->discrete.height = FRONTCAMERA_PREVIEW_HEIGHT;
        }        
        return 0;
}


static int s5k5aafa_enum_frameintervals(struct v4l2_subdev *sd, 
					struct v4l2_frmivalenum *fival)
{
	int err = 0;

	printk(KERN_DEBUG "s5k5aafa_enum_frameintervals is called... \n");
	
	return err;
}

static int s5k5aafa_enum_fmt(struct v4l2_subdev *sd, struct v4l2_fmtdesc *fmtdesc)
{
	int err = 0;

	printk(KERN_DEBUG "s5k5aafa_enum_fmt is called... \n");

	return err;
}

static int s5k5aafa_try_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
	int err = 0;

	printk(KERN_DEBUG "s5k5aafa_enum_fmt is called... \n");

	return err;
}

static int s5k5aafa_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;

	dev_dbg(&client->dev, "%s\n", __func__);

	return err;
}

static int s5k5aafa_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
        struct i2c_client *client = v4l2_get_subdevdata(sd);
        struct s5k5aafa_state *state = to_state(sd);
        int err = 0;
        int capture_mode = param->parm.capture.capturemode;

        state->capture_mode = capture_mode;
        
        dev_dbg(&client->dev, "%s: numerator %d, denominator: %d\n", \
                __func__, param->parm.capture.timeperframe.numerator, \
                param->parm.capture.timeperframe.denominator);

        return err;
}

static int s5k5aafa_get_framesize_index(struct v4l2_subdev *sd)
{
	int i = 0;
	struct s5k5aafa_state *state = to_state(sd);
	struct s5k5aafa_enum_framesize *frmsize;
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	/* Check for video/image mode */
	for(i = 0; i < (sizeof(s5k5aafa_framesize_list)/sizeof(struct s5k5aafa_enum_framesize)); i++)
	{
		frmsize = &s5k5aafa_framesize_list[i];
		if(frmsize->width >= state->pix.width && frmsize->height >= state->pix.height){
			return frmsize->index;
		} 
	}
	
	v4l_info(client, "%s: s5k5aafa_framesize_list[%d].index = %d\n", __func__, i - 1, s5k5aafa_framesize_list[i].index);
	
	/* FIXME: If it fails, return the last index. */
	return s5k5aafa_framesize_list[i-1].index;
}

/* This function is called from the s_ctrl api
 * Given the index, it checks if it is a valid index.
 * On success, it returns 0.
 * On Failure, it returns -EINVAL
 */
static int s5k5aafa_set_framesize_index(struct v4l2_subdev *sd, unsigned int index)
{
	int i = 0;
	struct s5k5aafa_state *state = to_state(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	v4l_info(client, "%s: index = %d\n", __func__, index);

	/* Check for video/image mode */
	for(i = 0; i < (sizeof(s5k5aafa_framesize_list)/sizeof(struct s5k5aafa_enum_framesize)); i++)
	{
		if(s5k5aafa_framesize_list[i].index == index){
			state->framesize_index = index; 
			state->pix.width = s5k5aafa_framesize_list[i].width;
			state->pix.height = s5k5aafa_framesize_list[i].height;
			return 0;
		} 
	} 
	
	return -EINVAL;
}

/* set sensor register values for adjusting brightness */
static int s5k5aafa_set_brightness(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k5aafa_state *state = to_state(sd);

	int err = -EINVAL;
	int ev_value = 0;

	dev_dbg(&client->dev, "%s: value : %d state->vt_mode %d \n", __func__, ctrl->value, state->vt_mode);

	ev_value = ctrl->value;

	printk(KERN_DEBUG "state->vt_mode : %d \n", state->vt_mode);
	if(state->vt_mode == 1)
	{
		switch(ev_value)
		{	
			case EV_MINUS_4:
				err = camsensor_config_set_register(sd,SECTION_EV_MINUS_4);
			break;

			case EV_MINUS_3:
				err = camsensor_config_set_register(sd,SECTION_EV_MINUS_3);
			break;

			
			case EV_MINUS_2:
				err = camsensor_config_set_register(sd,SECTION_EV_MINUS_2);
			break;
			
			case EV_MINUS_1:
				err = camsensor_config_set_register(sd,SECTION_EV_MINUS_1);
			break;

			case EV_DEFAULT:
				err = camsensor_config_set_register(sd,SECTION_EV_0);
			break;

			case EV_PLUS_1:
				err = camsensor_config_set_register(sd,SECTION_EV_PLUS_1);
			break;

			case EV_PLUS_2:
				err = camsensor_config_set_register(sd,SECTION_EV_PLUS_2);
			break;

			case EV_PLUS_3:
				err = camsensor_config_set_register(sd,SECTION_EV_PLUS_3);
			break;

			case EV_PLUS_4:
				err = camsensor_config_set_register(sd,SECTION_EV_PLUS_4);
			break;	
			
			default:
				err = camsensor_config_set_register(sd,SECTION_EV_0);				
			break;
		}
	}
	else
	{
		switch(ev_value)
		{	
			case EV_MINUS_4:
				err = camsensor_config_set_register(sd,SECTION_EV_MINUS_4);
			break;

			case EV_MINUS_3:
				err = camsensor_config_set_register(sd,SECTION_EV_MINUS_3);
			break;

			
			case EV_MINUS_2:
				err = camsensor_config_set_register(sd,SECTION_EV_MINUS_2);
			break;
			
			case EV_MINUS_1:
				err = camsensor_config_set_register(sd,SECTION_EV_MINUS_1);
			break;

			case EV_DEFAULT:
				err = camsensor_config_set_register(sd,SECTION_EV_0);
			break;

			case EV_PLUS_1:
				err = camsensor_config_set_register(sd,SECTION_EV_PLUS_1);
			break;

			case EV_PLUS_2:
				err = camsensor_config_set_register(sd,SECTION_EV_PLUS_2);
			break;

			case EV_PLUS_3:
				err = camsensor_config_set_register(sd,SECTION_EV_PLUS_3);
			break;

			case EV_PLUS_4:
				err = camsensor_config_set_register(sd,SECTION_EV_PLUS_4);
			break;	
			
			default:
				err = camsensor_config_set_register(sd,SECTION_EV_0);				
			break;
		}
	}
	if (err < 0)
	{
		v4l_info(client, "%s: register set failed\n", __func__);
		return -EIO;
	}
	return err;
}

/* set sensor register values for adjusting whitebalance, both auto and manual */
static int s5k5aafa_set_wb(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = -EINVAL;

	dev_dbg(&client->dev, "%s:  value : %d \n", __func__, ctrl->value);

	switch(ctrl->value)
	{
	case WHITE_BALANCE_AUTO:
		err = camsensor_config_set_register(sd,SECTION_WB_AUTO);			
		break;

	case WHITE_BALANCE_SUNNY:
		err = camsensor_config_set_register(sd,SECTION_WB_SUNNY);		
		break;

	case WHITE_BALANCE_CLOUDY:
		err = camsensor_config_set_register(sd,SECTION_WB_CLOUDY);		
		break;

	case WHITE_BALANCE_TUNGSTEN:
		err = camsensor_config_set_register(sd,SECTION_WB_TUNGSTEN);;		
		break;

	case WHITE_BALANCE_FLUORESCENT:
		err = camsensor_config_set_register(sd,SECTION_WB_FLUORESCENT);		
		break;

	default:
		dev_dbg(&client->dev, "%s: Not Support value \n", __func__);
		err = 0;
		break;

	}
	return err;
}

/* set sensor register values for adjusting color effect */
static int s5k5aafa_set_effect(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = -EINVAL;

	dev_dbg(&client->dev, "%s: value : %d \n", __func__, ctrl->value);

	switch(ctrl->value)
	{
	case IMAGE_EFFECT_NONE:
		err = camsensor_config_set_register(sd,SECTION_EFFECT_NORMAL);		
		break;

	case IMAGE_EFFECT_BNW:		//Gray
		err = camsensor_config_set_register(sd,SECTION_EFFECT_B_ANB_W);
		break;

	case IMAGE_EFFECT_SEPIA:
		err = camsensor_config_set_register(sd,SECTION_EFFECT_SEPIA);
		break;

	case IMAGE_EFFECT_AQUA:
		err = camsensor_config_set_register(sd,SECTION_EFFECT_AQUA);
		break;

	case IMAGE_EFFECT_NEGATIVE:
		err = camsensor_config_set_register(sd,SECTION_EFFECT_NEGATIVE);
		break;

	default:
		dev_dbg(&client->dev, "%s: Not Support value \n", __func__);
		err = 0;
		break;

	}
	
	return err;
}

static int s5k5aafa_set_shade(struct v4l2_subdev *sd)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int err = -1;
    u8 sh_val_high, sh_val_low;
    u16 shade_value;

    sensor_page_mode(sd,0x00);
    
    if(s5k5aafa_i2c_read_byte(sd,0xE5,&sh_val_high) < 0)
    {
        k5aa_msg(&client->dev,"Failed i2c_read_word!!\n");
    }
    if(s5k5aafa_i2c_read_byte(sd,0xE6,&sh_val_low) < 0)
    {
        k5aa_msg(&client->dev,"Failed i2c_read_word!!\n");
    }
    shade_value = sh_val_high<<8 |sh_val_low;

    k5aa_msg(&client->dev,"shade_value = %04x\n", shade_value);
        
    if(shade_value >= threshold_table[MIN_SHADE_COND] && 
       shade_value <= threshold_table[MAX_SHADE_COND])
    {
        err = camsensor_config_set_register(sd, SECTION_SHADE_DNP);
    }
    else
    {
        err = camsensor_config_set_register(sd, SECTION_SHADE_ETC);
    }

    if(err < 0)
        k5aa_msg(&client->dev,"Failed SECTION_SHADE\n");
    
    return err;
}

static int s5k5aafa_set_preview_start(struct v4l2_subdev *sd)
{
    int err = -EINVAL;
    u8 AgcValue;    
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct s5k5aafa_state *state = to_state(sd);

    k5aa_msg(&client->dev,"%s\n",__func__);

    /* check shade register value */
    if(s5k5aafa_set_shade(sd) < 0)
        return -1;

    if(state->camcorder_mode)
    {
        /* Set Camcorder_Init */
        k5aa_msg(&client->dev,"Set Camcorder_mode\n");
        err = camsensor_config_set_register(sd, SECTION_OP_CAMCORDER);
    }
    else
    {
        k5aa_msg(&client->dev,"Set Preview_mode\n");
        /* Set Preview_Init */
        if(camsensor_config_set_register(sd, SECTION_OP_PREVIEW) < 0)
            return -1;
        
        sensor_page_mode(sd,0x00);
        if(s5k5aafa_i2c_read_byte(sd,0xf8,&AgcValue) < 0)
        {
            k5aa_msg(&client->dev,"Failed i2c_read_word!!\n");
        }

        if(AgcValue <= threshold_table[NORMAL_LIGHT_COND])
        {
            k5aa_msg(&client->dev,"High light preview!!!! \n");    
            err = camsensor_config_set_register(sd, SECTION_OP_HIGH_LIGHT_PREVIEW);
        }
        else
        {
            k5aa_msg(&client->dev,"Normal light preview!!!! \n");
            err = camsensor_config_set_register(sd, SECTION_OP_NORMAL_LIGHT_PREVIEW);
        }
    }
    msleep(50);
    
    if(err < 0)
        k5aa_msg(&client->dev,"%s : Failed\n",__func__);
    else
        k5aa_msg(&client->dev,"%s:done\n",__func__);

    return err;
}

//Arun-SISO
static int s5k5aafx_set_capture_start(struct v4l2_subdev *sd)
{
    int err = -EINVAL;
    u8 AgcValue;
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    k5aa_msg(&client->dev,"%s\n",__func__);

    sensor_page_mode(sd,0x00);
    if(s5k5aafa_i2c_read_byte(sd,0xf8,&AgcValue) < 0)
    {
        k5aa_msg(&client->dev,"Failed i2c_read_word!!\n");
    }
    
    err = s5k5aafa_set_shade(sd);

    if(AgcValue >= threshold_table[LOW_LIGHT_COND])
    {
        k5aa_msg(&client->dev,"LOW_LIGHT_COND\n");
        err = camsensor_config_set_register(sd, SECTION_OP_LOW_LIGHT_CAPTURE);
        msleep(capture_delay_table[LOW_LIGHT_DELAY]);
    }
    else if(AgcValue >= threshold_table[MID_LOW_LIGHT_COND])
    {
        k5aa_msg(&client->dev,"MID_LOW_LIGHT_COND\n");    
        err = camsensor_config_set_register(sd, SECTION_OP_MID_LOW_LIGHT_CAPTURE);    
        msleep(capture_delay_table[MID_LOW_LIGHT_DELAY]);
    }
    else if(AgcValue <= threshold_table[NORMAL_LIGHT_COND])
    {
        k5aa_msg(&client->dev,"NORMAL_LIGHT_COND\n");    
        err = camsensor_config_set_register(sd, SECTION_OP_CAPTURE);    
        msleep(capture_delay_table[NORMAL_LIGHT_DELAY]);
    }
    else
    {
        k5aa_msg(&client->dev,"ETC_COND\n");    
        err = camsensor_config_set_register(sd, SECTION_OP_MIDLIGHT_CAPTURE);
        msleep(capture_delay_table[MID_LIGHT_DELAY]);
    }

    if(err < 0)
        k5aa_msg(&client->dev,"%s : Failed\n",__func__);
    else
        k5aa_msg(&client->dev,"%s:done\n",__func__);
    
    return err;
}

/* set sensor register values for frame rate(fps) setting */
static int s5k5aafa_set_frame_rate(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    // sunggeun : need to do later
    return 0;
}

/* set sensor register values for adjusting blur effect */
static int s5k5aafa_set_blur(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    // sunggeun : need to do later
    return 0;
}

static int s5k5aafa_set_camcorder_mode(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);

    k5aa_msg(&client->dev,"%s\n",__func__);

    if(ctrl->value)
    {
        if(camsensor_config_set_register(sd, SECTION_OP_CAMCORDER) < 0)
            return -1;            
    }
    else
    {
        if(camsensor_config_set_register(sd, SECTION_OP_PREVIEW) < 0)
            return -1;    
    }
    return 0;
}

static int s5k5aafa_check_dataline_stop(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k5aafa_state *state = to_state(sd);
	int err = -EINVAL;

	k5aa_msg(&client->dev, "%s\n", __func__);

	state->check_dataline = 0;
       err = s5k5aafa_reset(sd);
	if (err < 0)
	{
		v4l_info(client, "%s: register set failed\n", __func__);
		return -EIO;
	}
	return err;
}

/* if you need, add below some functions below */

static int s5k5aafa_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k5aafa_state *state = to_state(sd);
	struct s5k5aafa_userset userset = state->userset;
	int err = -EINVAL;

	dev_dbg(&client->dev, "%s: id : 0x%08lx \n", __func__, ctrl->id);

	switch (ctrl->id) {
	case V4L2_CID_EXPOSURE:
		ctrl->value = userset.exposure_bias;
		err = 0;
		break;

	case V4L2_CID_AUTO_WHITE_BALANCE:
		ctrl->value = userset.auto_wb;
		err = 0;
		break;

	case V4L2_CID_WHITE_BALANCE_PRESET:
		ctrl->value = userset.manual_wb;
		err = 0;
		break;

	case V4L2_CID_COLORFX:
		ctrl->value = userset.effect;
		err = 0;
		break;

	case V4L2_CID_CONTRAST:
		ctrl->value = userset.contrast;
		err = 0;
		break;

	case V4L2_CID_SATURATION:
		ctrl->value = userset.saturation;
		err = 0;
		break;

	case V4L2_CID_SHARPNESS:
		ctrl->value = userset.saturation;
		err = 0;
		break;

#if 0
	case V4L2_CID_CAM_FRAMESIZE_INDEX:
		ctrl->value = s5k5aafa_get_framesize_index(sd);
		err = 0;
		break;
#endif

       case V4L2_CID_CAMERA_GET_ISO:
            err = 0;
            
	default:
		dev_dbg(&client->dev, "%s: no such ctrl\n", __func__);
		break;
	}
	
	return err;
}

static int s5k5aafa_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
#ifdef S5K5AAFA_COMPLETE
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k5aafa_state *state = to_state(sd);

	int err = -EINVAL;

	k5aa_msg(&client->dev,"s5k5aafa_s_ctrl() : ctrl->id 0x%08lx, ctrl->value %d \n",ctrl->id, ctrl->value);

	switch (ctrl->id) {
	case V4L2_CID_CAMERA_BRIGHTNESS:	//V4L2_CID_EXPOSURE:
		dev_dbg(&client->dev, "%s: V4L2_CID_CAMERA_BRIGHTNESS\n", __func__);
		err = s5k5aafa_set_brightness(sd, ctrl);
		err = 0;
		break;        
#if 0
	case V4L2_CID_EXPOSURE:
		dev_dbg(&client->dev, "%s: V4L2_CID_EXPOSURE\n", __func__);
		err = s5k5aafa_set_brightness(sd, ctrl);
		break;

	case V4L2_CID_AUTO_WHITE_BALANCE:
		dev_dbg(&client->dev, "%s: V4L2_CID_AUTO_WHITE_BALANCE\n", \
			__func__);
		err = s5k5aafa_write_regs(sd, \
			(unsigned char *) s5k5aafa_regs_awb_enable[ctrl->value], \
			sizeof(s5k5aafa_regs_awb_enable[ctrl->value]));
		break;

	case V4L2_CID_WHITE_BALANCE_PRESET:
		dev_dbg(&client->dev, "%s: V4L2_CID_WHITE_BALANCE_PRESET\n", \
			__func__);
		err = s5k5aafa_write_regs(sd, \
			(unsigned char *) s5k5aafa_regs_wb_preset[ctrl->value], \
			sizeof(s5k5aafa_regs_wb_preset[ctrl->value]));
		break;

	case V4L2_CID_COLORFX:
		dev_dbg(&client->dev, "%s: V4L2_CID_COLORFX\n", __func__);
		err = s5k5aafa_write_regs(sd, \
			(unsigned char *) s5k5aafa_regs_color_effect[ctrl->value], \
			sizeof(s5k5aafa_regs_color_effect[ctrl->value]));
		break;

	case V4L2_CID_CONTRAST:
		dev_dbg(&client->dev, "%s: V4L2_CID_CONTRAST\n", __func__);
		err = s5k5aafa_write_regs(sd, \
			(unsigned char *) s5k5aafa_regs_contrast_bias[ctrl->value], \
			sizeof(s5k5aafa_regs_contrast_bias[ctrl->value]));
		break;

	case V4L2_CID_SATURATION:
		dev_dbg(&client->dev, "%s: V4L2_CID_SATURATION\n", __func__);
		err = s5k5aafa_write_regs(sd, \
			(unsigned char *) s5k5aafa_regs_saturation_bias[ctrl->value], \
			sizeof(s5k5aafa_regs_saturation_bias[ctrl->value]));
		break;

	case V4L2_CID_SHARPNESS:
		dev_dbg(&client->dev, "%s: V4L2_CID_SHARPNESS\n", __func__);
		err = s5k5aafa_write_regs(sd, \
			(unsigned char *) s5k5aafa_regs_sharpness_bias[ctrl->value], \
			sizeof(s5k5aafa_regs_sharpness_bias[ctrl->value]));
		break;	

	/* The camif supports only a few frame resolutions. 
 	 * Through this call, camif can set the camera resolution with given index.
 	 * Typically, camif gets the index through g_ctrl call with this ID.
 	 */

 	case V4L2_CID_CAM_FRAMESIZE_INDEX:
		err = s5k5aafa_set_framesize_index(sd, ctrl->value);
        break;
#endif

	case V4L2_CID_CAM_CAPTURE:
		k5aa_msg(&client->dev,"%s: V4L2_CID_CAM_CAPTURE ###############################\n", __func__);
		err = s5k5aafx_set_capture_start(sd);
		break;	
#if 0 
	case V4L2_CID_CAMERA_WHITE_BALANCE: //V4L2_CID_AUTO_WHITE_BALANCE:
		dev_dbg(&client->dev, "%s: V4L2_CID_AUTO_WHITE_BALANCE\n", __func__);
		err = s5k5aafa_set_wb(sd, ctrl);
		err = 0;		
		break;

	case V4L2_CID_CAMERA_EFFECT:	//V4L2_CID_COLORFX:
		dev_dbg(&client->dev, "%s: V4L2_CID_CAMERA_EFFECT\n", __func__);
		err = s5k5aafa_set_effect(sd, ctrl);
		err = 0;        
		break;

	case V4L2_CID_CAMERA_FRAME_RATE:
		dev_dbg(&client->dev, "%s: V4L2_CID_CAMERA_FRAME_RATE\n", __func__);
		//err = s5k5aafa_set_frame_rate(sd, ctrl);	
		err = 0;        
		break;
		
	case V4L2_CID_CAMERA_VGA_BLUR:
		dev_dbg(&client->dev, "%s: V4L2_CID_CAMERA_FRAME_RATE\n", __func__);
		//err = s5k5aafa_set_blur(sd, ctrl);	
		err = 0;        
		break;

	//s1_camera [ Defense process by ESD input ] _[
	case V4L2_CID_CAMERA_RESET:
		dev_dbg(&client->dev, "%s: V4L2_CID_CAMERA_RESET \n", __func__);
		err = s5k5aafa_reset(sd);
		err = 0;		
		break;
	// _]
#endif
        case V4L2_CID_CAMERA_MON_MOVIE_SELECT:
            state->camcorder_mode = ctrl->value;
            err = 0;
            break;

      case V4L2_CID_CAMERA_VT_MODE:
          state->vt_mode = ctrl->value;
          dev_dbg(&client->dev, "%s: V4L2_CID_CAMERA_VT_MODE : state->vt_mode %d \n", __func__, state->vt_mode);
          err = 0;
          break;
      
      case V4L2_CID_CAM_PREVIEW_ONOFF:
          err = s5k5aafa_set_preview_start(sd);
          //err = 0;
          break;

      case V4L2_CID_CAMERA_CHECK_DATALINE:
		state->check_dataline = ctrl->value;
		err = 0;
		break;	

	case V4L2_CID_CAMERA_CHECK_DATALINE_STOP:
		err = s5k5aafa_check_dataline_stop(sd);
		err = 0;        
		break;

	default:
		k5aa_msg(&client->dev, "%s: no support control in camera sensor, S5K5AAFA\n", __func__);
		err = 0;
		break;
	}

	if (err < 0)
		goto out;
	else
		return 0;

out:
	k5aa_msg(&client->dev, "%s: vidioc_s_ctrl failed\n", __func__);
	return err;
#else
	return 0;
#endif
}

static int s5k5aafa_init(struct v4l2_subdev *sd, u32 val)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct s5k5aafa_state *state = to_state(sd);
    int err = -EINVAL, i;

    //v4l_info(client, "%s: camera initialization start : state->vt_mode %d \n", __func__, state->vt_mode);
    printk(KERN_DEBUG "camera initialization start, state->vt_mode : %d \n", state->vt_mode); 
    printk(KERN_DEBUG "state->check_dataline : %d \n", state->check_dataline); 

    for(i=0;i<CAM_THRESHOLD_MAX;i++)
    {
        threshold_table[i] = camsensor_config_get_data(sd, SECTION_SET_THRESHOLD_TABLE,i);
        k5aa_msg(&client->dev,"threshold_table[%d] = %d\n",i,threshold_table[i]);
    }

    for(i=0; i<CAPTURE_DELAY_MAX;i++)
    {
        capture_delay_table[i] = camsensor_config_get_data(sd, SECTION_CAPTURE_DELAY_TABLE,i);
        k5aa_msg(&client->dev,"capture_delay_table[%d] = %d\n",i,capture_delay_table[i]);
    }

    k5aa_msg(&client->dev, "%s, dtp value is %d\n",__func__, state->check_dataline);
    
    if(state->vt_mode == 0)
    {
        if(state->check_dataline)
            err = camsensor_config_set_register(sd,SECTION_DTP);
        else
            err = camsensor_config_set_register(sd,SECTION_INIT);
    }
    else
    {
        err = camsensor_config_set_register(sd,SECTION_VT_INIT);
    }
    
    if(err == 0)
    {
        k5aa_msg(&client->dev,"%s : success to write section_init\n",__func__);
        err = 0;
    }
    else
    {   
        k5aa_msg(&client->dev,"set_register is failed\n");
        err = -EIO;
    }

    if (err < 0) {
        //This is preview fail 
        state->check_previewdata = 100;
        v4l_err(client, "%s: camera initialization failed. err(%d)\n", \
        __func__, state->check_previewdata);
        return -EIO;	/* FIXME */	
    }

    //This is preview success
    state->check_previewdata = 0;
    return err;
}

/*
 * s_config subdev ops
 * With camera device, we need to re-initialize every single opening time therefor,
 * it is not necessary to be initialized on probe time. except for version checking
 * NOTE: version checking is optional
 */
static int s5k5aafa_s_config(struct v4l2_subdev *sd, int irq, void *platform_data)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k5aafa_state *state = to_state(sd);
	struct s5k5aafa_platform_data *pdata;

	dev_dbg(&client->dev, "fetching platform data\n");

	pdata = client->dev.platform_data;

	if (!pdata) {
		dev_err(&client->dev, "%s: no platform data\n", __func__);
		return -ENODEV;
	}

	/*
	 * Assign default format and resolution
	 * Use configured default information in platform data
	 * or without them, use default information in driver
	 */
	if (!(pdata->default_width && pdata->default_height)) {
		/* TODO: assign driver default resolution */
	} else {
		state->pix.width = pdata->default_width;
		state->pix.height = pdata->default_height;
	}

	if (!pdata->pixelformat)
		state->pix.pixelformat = DEFAULT_FMT;
	else
		state->pix.pixelformat = pdata->pixelformat;

	if (!pdata->freq)
		state->freq = 24000000;	/* 24MHz default */
	else
		state->freq = pdata->freq;

	if (!pdata->is_mipi) {
		state->is_mipi = 0;
		dev_dbg(&client->dev, "parallel mode\n");
	} else
		state->is_mipi = pdata->is_mipi;

	return 0;
}

static const struct v4l2_subdev_core_ops s5k5aafa_core_ops = {
	.init = s5k5aafa_init,	/* initializing API */
	.s_config = s5k5aafa_s_config,	/* Fetch platform data */
	.queryctrl = s5k5aafa_queryctrl,
	.querymenu = s5k5aafa_querymenu,
	.g_ctrl = s5k5aafa_g_ctrl,
	.s_ctrl = s5k5aafa_s_ctrl,
};

static const struct v4l2_subdev_video_ops s5k5aafa_video_ops = {
	.s_crystal_freq = s5k5aafa_s_crystal_freq,
	.g_fmt = s5k5aafa_g_fmt,
	.s_fmt = s5k5aafa_s_fmt,
	.enum_framesizes = s5k5aafa_enum_framesizes,
	.enum_frameintervals = s5k5aafa_enum_frameintervals,
	.enum_fmt = s5k5aafa_enum_fmt,
	.try_fmt = s5k5aafa_try_fmt,
	.g_parm = s5k5aafa_g_parm,
	.s_parm = s5k5aafa_s_parm,
};

static const struct v4l2_subdev_ops s5k5aafa_ops = {
	.core = &s5k5aafa_core_ops,
	.video = &s5k5aafa_video_ops,
};

/*
 * s5k5aafa_probe
 * Fetching platform data is being done with s_config subdev call.
 * In probe routine, we just register subdev device
 */
static int s5k5aafa_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct s5k5aafa_state *state;
	struct v4l2_subdev *sd;

	state = kzalloc(sizeof(struct s5k5aafa_state), GFP_KERNEL);
	if (state == NULL)
		return -ENOMEM;

	sd = &state->sd;
	strcpy(sd->name, S5K5AAFA_DRIVER_NAME);

	/* Registering subdev */
	v4l2_i2c_subdev_init(sd, client, &s5k5aafa_ops);

	dev_dbg(&client->dev, "s5k5aafa has been probed\n");
	return 0;
}


static int s5k5aafa_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);

#ifdef FEATURE_CAMERA_TUNING_MODE
       if(curr_buf_ptr != NULL)
       {
            kfree(curr_buf_ptr);
            curr_buf_ptr = NULL;
       }
#endif       
	v4l2_device_unregister_subdev(sd);
	kfree(to_state(sd));
	return 0;
}

static const struct i2c_device_id s5k5aafa_id[] = {
	{ S5K5AAFA_DRIVER_NAME, 0 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, s5k5aafa_id);

static struct v4l2_i2c_driver_data v4l2_i2c_data = {
	.name = S5K5AAFA_DRIVER_NAME,
	.probe = s5k5aafa_probe,
	.remove = s5k5aafa_remove,
	.id_table = s5k5aafa_id,
};

MODULE_DESCRIPTION("Samsung Electronics S5K5AAFA UXGA camera driver");
MODULE_AUTHOR("Jinsung Yang <jsgood.yang@samsung.com>");
MODULE_LICENSE("GPL");
