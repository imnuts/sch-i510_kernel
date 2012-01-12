/***********************************************************************
* Driver for SR130PC10 (1.3MP Camera) from SILICONFILE
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
***********************************************************************/

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-i2c-drv.h>
#include <media/sr130pc10_platform.h>

#ifdef CONFIG_VIDEO_SAMSUNG_V4L2
#include <linux/videodev2_samsung.h>
#endif // CONFIG_VIDEO_SAMSUNG_V4L2

#include <linux/rtc.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <mach/regs-gpio.h>
#include <mach/regs-clock.h>

#include "sr130pc10.h"

//#define FRONT_CONFIG_LOAD_FILE	//For tunning binary

/**************************************************************************
* DEFINES
***************************************************************************/

#define SR130PC10_DRIVER_NAME	"SR130PC10"

#define FORMAT_FLAGS_PACKED	            0x1
#define FORMAT_FLAGS_PLANAR	            0x2
#define FORMAT_FLAGS_ENCODED	        0x3
#define FORMAT_FLAGS_COMPRESSED			FORMAT_FLAGS_ENCODED

#define SENSOR_MAX_WIDTH        1280
#define SENSOR_MAX_HEIGHT       960

#define PREVIEW_BASE_WIDTH      640
#define PREVIEW_BASE_HEIGHT     480

#define MAX_BUFFER			    (2048)
#define DELAY_SEQ               0xFF

/* Default resolution & pixelformat. */
#define DEFAULT_PIX_FMT		    V4L2_PIX_FMT_YUYV	/* YUV422 */
#define DEFUALT_MCLK		    24000000
#define POLL_TIME_MS		    10

#define INIT_NUM_OF_REGS                (sizeof(front_init_regs) / sizeof(regs_short_t))
#define INIT_VT_NUM_OF_REGS             (sizeof(front_init_vt_regs) / sizeof(regs_short_t))
#define PREVIEW_CAMERA_NUM_OF_REGS      (sizeof(front_preview_camera_regs) / sizeof(regs_short_t))
#define SNAPSHOT_NORMAL_NUM_OF_REGS     (sizeof(front_snapshot_normal_regs) / sizeof(regs_short_t))

#define WB_AUTO_NUM_OF_REGS	            (sizeof(front_wb_auto_regs) / sizeof(regs_short_t))
#define WB_SUNNY_NUM_OF_REGS	        (sizeof(front_wb_sunny_regs) / sizeof(regs_short_t))
#define WB_CLOUDY_NUM_OF_REGS	        (sizeof(front_wb_cloudy_regs) / sizeof(regs_short_t))
#define WB_TUNSTEN_NUM_OF_REGS	        (sizeof(front_wb_tungsten_regs) / sizeof(regs_short_t))
#define WB_FLUORESCENT_NUM_OF_REGS	    (sizeof(front_wb_fluorescent_regs) / sizeof(regs_short_t))

#define EFFECT_NORMAL_NUM_OF_REGS	    (sizeof(front_effect_normal_regs) / sizeof(regs_short_t))
#define EFFECT_NEGATIVE_NUM_OF_REGS	    (sizeof(front_effect_negative_regs) / sizeof(regs_short_t))
#define EFFECT_SEPIA_NUM_OF_REGS	    (sizeof(front_effect_sepia_regs) / sizeof(regs_short_t))
#define EFFECT_MONO_NUM_OF_REGS         (sizeof(front_effect_mono_regs) / sizeof(regs_short_t))

#define EV_M4_NUM_OF_REGS	            (sizeof(front_ev_minus_4_regs) / sizeof(regs_short_t))
#define EV_M3_NUM_OF_REGS	            (sizeof(front_ev_minus_3_regs) / sizeof(regs_short_t))
#define EV_M2_NUM_OF_REGS	            (sizeof(front_ev_minus_2_regs) / sizeof(regs_short_t))
#define EV_M1_NUM_OF_REGS	            (sizeof(front_ev_minus_1_regs) / sizeof(regs_short_t))
#define EV_DEFAULT_NUM_OF_REGS	        (sizeof(front_ev_default_regs) / sizeof(regs_short_t))
#define EV_P1_NUM_OF_REGS	            (sizeof(front_ev_plus_1_regs) / sizeof(regs_short_t))
#define EV_P2_NUM_OF_REGS	            (sizeof(front_ev_plus_2_regs) / sizeof(regs_short_t))
#define EV_P3_NUM_OF_REGS	            (sizeof(front_ev_plus_3_regs) / sizeof(regs_short_t))
#define EV_P4_NUM_OF_REGS	            (sizeof(front_ev_plus_4_regs) / sizeof(regs_short_t))

#define EV_VT_M4_NUM_OF_REGS	        (sizeof(front_ev_vt_minus_4_regs) / sizeof(regs_short_t))
#define EV_VT_M3_NUM_OF_REGS	        (sizeof(front_ev_vt_minus_3_regs) / sizeof(regs_short_t))
#define EV_VT_M2_NUM_OF_REGS	        (sizeof(front_ev_vt_minus_2_regs) / sizeof(regs_short_t))
#define EV_VT_M1_NUM_OF_REGS	        (sizeof(front_ev_vt_minus_1_regs) / sizeof(regs_short_t))
#define EV_VT_DEFAULT_NUM_OF_REGS	    (sizeof(front_ev_vt_default_regs) / sizeof(regs_short_t))
#define EV_VT_P1_NUM_OF_REGS	        (sizeof(front_ev_vt_plus_1_regs) / sizeof(regs_short_t))
#define EV_VT_P2_NUM_OF_REGS	        (sizeof(front_ev_vt_plus_2_regs) / sizeof(regs_short_t))
#define EV_VT_P3_NUM_OF_REGS	        (sizeof(front_ev_vt_plus_3_regs) / sizeof(regs_short_t))
#define EV_VT_P4_NUM_OF_REGS	        (sizeof(front_ev_vt_plus_4_regs) / sizeof(regs_short_t))

#define FPS_AUTO_NUM_OF_REGS	        (sizeof(front_fps_auto_regs) / sizeof(regs_short_t))
#define FPS_7_NUM_OF_REGS	            (sizeof(front_fps_7_regs) / sizeof(regs_short_t))
#define FPS_10_NUM_OF_REGS	            (sizeof(front_fps_10_regs) / sizeof(regs_short_t))
#define FPS_15_NUM_OF_REGS	            (sizeof(front_fps_15_regs) / sizeof(regs_short_t))

#define FPS_VT_AUTO_NUM_OF_REGS	        (sizeof(front_fps_vt_auto_regs) / sizeof(regs_short_t))
#define FPS_VT_7_NUM_OF_REGS	        (sizeof(front_fps_vt_7_regs) / sizeof(regs_short_t))
#define FPS_VT_10_NUM_OF_REGS	        (sizeof(front_fps_vt_10_regs) / sizeof(regs_short_t))
#define FPS_VT_15_NUM_OF_REGS	        (sizeof(front_fps_vt_15_regs) / sizeof(regs_short_t))

#define PATTERN_ON_NUM_OF_REGS	        (sizeof(front_pattern_on_regs) / sizeof(regs_short_t))
#define PATTERN_OFF_NUM_OF_REGS	        (sizeof(front_pattern_off_regs) / sizeof(regs_short_t))

//#define SR130PC10_DEBUG 

#ifdef SR130PC10_DEBUG
#define CAM_ERROR_MSG(dev, format, arg...)  dev_err(dev, format, ## arg)
#define CAM_WARN_MSG(dev, format, arg...)   dev_warn(dev, format, ## arg)
#define CAM_INFO_MSG(dev, format, arg...)   dev_warn(dev, format, ## arg)
#define CAM_PRINTK(format, arg...)          printk(format, ## arg)
#else
#define CAM_ERROR_MSG(dev, format, arg...)  dev_err(dev, format, ## arg)
#define CAM_WARN_MSG(dev, format, arg...)   dev_warn(dev, format, ## arg)
#define CAM_INFO_MSG(dev, format, arg...)
#define CAM_PRINTK(format, arg...)
#endif // SR130PC10_DEBUG

/**************************************************************************
* ENUM STRUCTURES
***************************************************************************/

enum operation_mode 
{
	OP_MODE_VIDEO = 0,
	OP_MODE_IMAGE = 1,
};

enum sensor_frame_size 
{
	PREVIEW_SIZE_VGA = 0,	    /* VGA  :   640x480 */
	CAPTURE_SIZE_4VGA,		    /* 4VGA :  1280x960 */
    CAPTURE_SIZE_PREVIEW        /* 416x320 */
};

/**************************************************************************
* STRUCTURES
***************************************************************************/
struct sensor_enum_framesize 
{
	/* op_mode is 0 for preview, 1 for capture */
	enum operation_mode op_mode;
	enum sensor_frame_size size_index;
	unsigned int width;
	unsigned int height;	
};

/* Camera functional setting values configured by user concept */
struct sensor_userset 
{
	signed int exposure_bias;	/* V4L2_CID_EXPOSURE */
	unsigned int auto_wb;		/* V4L2_CID_AUTO_WHITE_BALANCE */
	unsigned int manual_wb;		/* V4L2_CID_WHITE_BALANCE_PRESET */
	unsigned int effect;		/* Color FX (AKA Color tone) */
	unsigned int contrast;		/* V4L2_CID_CONTRAST */
	unsigned int saturation;	/* V4L2_CID_SATURATION */
	unsigned int sharpness;		/* V4L2_CID_SHARPNESS */
};

struct sensor_jpeg_param 
{
	unsigned int enable;
	unsigned int quality;
	unsigned int main_size;  /* Main JPEG file size */
	unsigned int thumb_size; /* Thumbnail file size */
	unsigned int main_offset;
	unsigned int thumb_offset;
	unsigned int postview_offset;
} ; 

struct sensor_image_info {
	int width;
	int height;
} ; 

struct sensor_state 
{
	struct sr130pc10_platform_data *platform_data;
	struct v4l2_subdev sd;
	struct v4l2_pix_format pix;
	struct sensor_userset userset;
	struct sensor_image_info postview_info;
	struct v4l2_streamparm strm;
	int sensor_mode;
	int freq;	            /* MCLK in Hz */
	int fps;
    int vt_mode;                /*For VT camera*/
    int capture_mode;
    int check_dataline;
};

/**************************************************************************
* GLOBAL, STATIC VARIABLES
***************************************************************************/

/* protect s_ctrl calls */
static DEFINE_MUTEX(front_sensor_s_ctrl);

/* Save the focus mode value, it can be marco or auto */
static int front_gCurrentWB = WHITE_BALANCE_AUTO;
static int front_preview_ratio = 16;
static int front_gISOSpeedRating = 0;
static int front_gExposureTime = 0;

/**************************************************************************
* EXTERN VARIABLES
***************************************************************************/

/**************************************************************************
* FUNCTION DECLARE
***************************************************************************/
static int sr130pc10_i2c_read_byte(struct i2c_client *client,  
                                     unsigned short subaddr, 
                                     unsigned short *data);
                                     
static int sr130pc10_i2c_write_byte(struct i2c_client *client, 
                                     unsigned short subaddr, 
                                     unsigned short data);

static int sr130pc10_i2c_read_word(struct i2c_client *client,  
                                     unsigned short subaddr, 
                                     unsigned short *data);

static int sr130pc10_i2c_write_word(struct i2c_client *client, 
                                     unsigned short subaddr, 
                                     unsigned short data);

static int sr130pc10_i2c_set_data_burst(struct i2c_client *client, 
                                         regs_short_t reg_buffer[], 
          				                 int num_of_regs); 

static int sr130pc10_i2c_set_config_register(struct i2c_client *client, 
                                         regs_short_t reg_buffer[], 
          				                 int num_of_regs,
          				                 char *name);
          				                 
static int sr130pc10_check_dataline_onoff(struct v4l2_subdev *sd, int onoff);
static int sr130pc10_get_iso_speed_rate(struct v4l2_subdev *sd);
static int sr130pc10_get_shutterspeed(struct v4l2_subdev *sd);

static inline struct sensor_state *to_state(struct v4l2_subdev *sd)
{
	return container_of(sd, struct sensor_state, sd);
}

/**************************************************************************
* TUNING CONFIGURATION FUNCTIONS, DATAS
***************************************************************************/
#ifdef FRONT_CONFIG_LOAD_FILE

#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

#define MAX_REG_TABLE_LEN 3500
#define MAX_ONE_LINE_LEN 500

typedef struct
{
    char name[100];
    char *location_ptr;
} reg_hash_t;

static char *front_regs_buf_ptr = NULL; 
static char *front_curr_pos_ptr = NULL;
static char front_current_line[MAX_ONE_LINE_LEN];
static regs_short_t front_reg_table[MAX_REG_TABLE_LEN];
static int front_reg_num_of_element = 0;

// Warning!! : Register order is very important in aspect of performance of loading regs.
// Place regs by the order as described in register header file.
static reg_hash_t front_reg_hash_table[] =
{
    {"front_init_regs",                   NULL},
    {"front_init_vt_regs",              NULL},
    {"front_preview_camera_regs",         NULL},
    {"front_snapshot_normal_regs",        NULL},
    {"front_ev_minus_4_regs",             NULL},
    {"front_ev_minus_3_regs",             NULL},
    {"front_ev_minus_2_regs",             NULL},
    {"front_ev_minus_1_regs",             NULL},
    {"front_ev_default_regs",             NULL},
    {"front_ev_plus_1_regs",              NULL},
    {"front_ev_plus_2_regs",              NULL},
    {"front_ev_plus_3_regs",              NULL},
    {"front_ev_plus_4_regs",              NULL},  
    {"front_ev_vt_minus_4_regs",          NULL},
    {"front_ev_vt_minus_3_regs",          NULL},
    {"front_ev_vt_minus_2_regs",          NULL},
    {"front_ev_vt_minus_1_regs",          NULL},
    {"front_ev_vt_default_regs",          NULL},
    {"front_ev_vt_plus_1_regs",           NULL},
    {"front_ev_vt_plus_2_regs",           NULL},
    {"front_ev_vt_plus_3_regs",           NULL},
    {"front_ev_vt_plus_4_regs",           NULL},    
    {"front_wb_auto_regs",                NULL},
    {"front_wb_sunny_regs",               NULL},
    {"front_wb_cloudy_regs",              NULL},
    {"front_wb_tungsten_regs",            NULL},
    {"front_wb_fluorescent_regs",         NULL},
    {"front_effect_normal_regs",          NULL},
    {"front_effect_negative_regs",        NULL},
    {"front_effect_sepia_regs",           NULL},
    {"front_effect_mono_regs",            NULL},
    {"front_fps_auto_regs",               NULL},
    {"front_fps_7_regs",                  NULL},
    {"front_fps_10_regs",                 NULL},
    {"front_fps_15_regs",                 NULL},
    {"front_fps_vt_auto_regs",            NULL},
    {"front_fps_vt_7_regs",               NULL},
    {"front_fps_vt_10_regs",              NULL},
    {"front_fps_vt_15_regs",              NULL},
    {"front_pattern_on_regs",             NULL},
    {"front_pattern_off_regs",            NULL},
};

static bool sr130pc10_regs_get_line(char *line_buf)
{
    int i;
    char *r_n_ptr = NULL;

    memset(line_buf, 0, MAX_ONE_LINE_LEN);

    r_n_ptr = strstr(front_curr_pos_ptr, "\n"); // Wr

    //\n exists.
    if(r_n_ptr )
    {
        for(i = 0; i < MAX_ONE_LINE_LEN; i++)
        {
            if(front_curr_pos_ptr + i == r_n_ptr)
            {
                front_curr_pos_ptr = r_n_ptr + 1;
                break;
            }
            line_buf[i] = front_curr_pos_ptr[i];
        }
        line_buf[i] = '\0';

        return true;
    }
    //\n doesn't exist.
    else
    {
        if(strlen(front_curr_pos_ptr) > 0)
        {
            strcpy(line_buf, front_curr_pos_ptr);
            return true;
        }
        else
        {
            return false;
        }            
    }
}

static bool sr130pc10_regs_trim(char *line_buf)
{
    int left_index;
    int buff_len;
    int i;

    buff_len = strlen(line_buf);
    left_index  = -1;

    if(buff_len == 0)
    {
        return false;
    }        

    /* Find the first letter that is not a white space from left side */
    for(i = 0; i < buff_len; i++)
    {
        if((line_buf[i] != ' ') && (line_buf[i] != '\t') && (line_buf[i] != '\n') && (line_buf[i] != '\r'))
        {
            left_index = i;
            break;
        }
    }

    if(left_index == -1)
    {
        return false;
    }

    // Skip comments and empty line
    if((line_buf[left_index] == '\0') || ((line_buf[left_index] == '/') && (line_buf[left_index + 1] == '/')))
    {
        return false;
    }

    if(left_index != 0)
    {
        strcpy(line_buf, line_buf + left_index);
    }        

    return true;
}

static int sr130pc10_regs_parse_table(void)
{
#if 0 // Parsing a register format : {0x0000, 0x0000},
	char reg_buf[7], data_buf[7];
	int reg_index = 0;

	reg_buf[6] = '\0';
	data_buf[6] = '\0';

    while(sr130pc10_regs_get_line(front_current_line))
    {
        if(sr130pc10_regs_trim(front_current_line) == false)
        {
            continue;
        }

        // Check End line of a table.
        if((front_current_line[0] == '}') && (front_current_line[1] == ';'))
        {
            break;
        }

        // Parsing a register format : {0x0000, 0x0000},
        if((front_current_line[0] == '{') && (front_current_line[1] == '0') && (front_current_line[15] == '}'))
        {
            memcpy(reg_buf, (const void *)&front_current_line[1], 6);
            memcpy(data_buf, (const void *)&front_current_line[9], 6);

            front_reg_table[reg_index].subaddr = (unsigned short)simple_strtoul(reg_buf, NULL, 16); // 16 : Hexadecimal 
            front_reg_table[reg_index].value = (unsigned int)simple_strtoul(data_buf, NULL, 16);    // 16 : Hexadecimal 

            reg_index++;
        }
    }
    
#else // Parsing a register format : {0x00, 0x00},

	char reg_buf[5], data_buf[5];
	int reg_index = 0;

	reg_buf[4] = '\0';
	data_buf[4] = '\0';

    while(sr130pc10_regs_get_line(front_current_line))
    {
        if(sr130pc10_regs_trim(front_current_line) == false)
        {
            continue;
        }

        // Check End line of a table.
        if((front_current_line[0] == '}') && (front_current_line[1] == ';'))
        {
            break;
        }

        // Parsing a register format : {0x00, 0x00},
        if((front_current_line[0] == '{') && (front_current_line[1] == '0') && (front_current_line[11] == '}'))
        {
            memcpy(reg_buf, (const void *)&front_current_line[1], 4);
            memcpy(data_buf, (const void *)&front_current_line[7], 4);

            front_reg_table[reg_index].subaddr = (unsigned short)simple_strtoul(reg_buf, NULL, 16); // 16 : Hexadecimal
            front_reg_table[reg_index].value = (unsigned int)simple_strtoul(data_buf, NULL, 16);    // 16 : Hexadecimal

            reg_index++;
        }
    }
#endif

    return reg_index;
}

static int sr130pc10_regs_table_write(struct i2c_client *client, char *name)
{
    bool bFound_table = false;
    int i, err = 0;
    
    front_reg_num_of_element = 0;

    for(i = 0; i < sizeof(front_reg_hash_table)/sizeof(reg_hash_t); i++)
    {
        if(strcmp(name, front_reg_hash_table[i].name) == 0)
        {
            bFound_table = true;

            front_curr_pos_ptr = front_reg_hash_table[i].location_ptr;
            break;
        }
    }
    
    if(bFound_table)
    {
        front_reg_num_of_element = sr130pc10_regs_parse_table();
    }
    else
    {
        CAM_ERROR_MSG(&client->dev, "[%s: %d] %s front_reg_table doesn't exist\n", __FILE__, __LINE__, name);	
        return -EIO;
    }

    err = sr130pc10_i2c_set_data_burst(client, front_reg_table, front_reg_num_of_element);
	if(err < 0)
	{
		CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! sr130pc10_i2c_set_data_burst failed\n", __FILE__, __LINE__);	
		return -EIO;
	}
    
	return err;
}

int sr130pc10_regs_table_init(void)
{
	struct file *filp;
	char *dp;
	long l;
	loff_t pos;
	int ret, i, retry_cnt;
	mm_segment_t fs = get_fs();
    char *location_ptr = NULL;
    bool bFound_name;
    
	CAM_PRINTK("%s %d\n", __func__, __LINE__);

	set_fs(get_ds());

	
	filp = filp_open("/sdcard/sr130pc10.h", O_RDONLY, 0);

	if(IS_ERR(filp)) 
	{
		CAM_PRINTK(KERN_ERR "file open error\n");
		return -EIO;
	}
	
	l = filp->f_path.dentry->d_inode->i_size;	
	CAM_PRINTK("%s file size = %ld\n", __func__, l);

	//msleep(50);
    CAM_PRINTK("Start vmalloc\n");

    for(retry_cnt = 5; retry_cnt > 0; retry_cnt--)
    {
	    dp = kmalloc(l, GFP_KERNEL);  
	    if(dp != NULL)
	        break;

	    msleep(50);
    }
    
	if(dp == NULL) 
	{
		CAM_PRINTK(KERN_ERR "Out of Memory\n");
		filp_close(filp, current->files);
		return -ENOMEM;
	}
      CAM_PRINTK("End vmalloc\n");	     

	pos = 0;
       memset(dp, 0, l);
       CAM_PRINTK("Start vfs_read\n");
	ret = vfs_read(filp, (char __user *)dp, l, &pos);	
	if(ret != l) 
	{
		CAM_PRINTK(KERN_ERR "Failed to read file ret = %d\n", ret);
		vfree(dp);
		filp_close(filp, current->files);
		return -EINVAL;
	}
       CAM_PRINTK("End vfs_read\n");

	filp_close(filp, current->files);
		
	set_fs(fs);
	
	front_regs_buf_ptr = dp;
		
	*((front_regs_buf_ptr + l) - 1) = '\0';

    // Make hash table to enhance speed.
    front_curr_pos_ptr = front_regs_buf_ptr;
    location_ptr = front_curr_pos_ptr;
    
    for(i = 0; i < sizeof(front_reg_hash_table)/sizeof(reg_hash_t); i++)
    {
        front_reg_hash_table[i].location_ptr = NULL;
        bFound_name = false;
        
        while(sr130pc10_regs_get_line(front_current_line))	
        {
            if(strstr(front_current_line, front_reg_hash_table[i].name) != NULL)
            {
                bFound_name = true;
                front_reg_hash_table[i].location_ptr = location_ptr;
                break;
            }

            location_ptr = front_curr_pos_ptr;
        }

        if(bFound_name == false)
        {
            if(i == 0)
            {
                CAM_PRINTK(KERN_ERR "[%s : %d] ERROR! Couldn't find the reg name in hash table\n", __FILE__, __LINE__);	
                return -EIO;
            }
            else
            {
                front_curr_pos_ptr = front_reg_hash_table[i-1].location_ptr;
            }
            location_ptr = front_curr_pos_ptr;
            
            CAM_PRINTK(KERN_ERR "[%s : %d] ERROR! Couldn't find the reg name in hash table\n", __FILE__, __LINE__);	
        }
    }
    
	CAM_PRINTK("sr130pc10_reg_table_init\n");

	return 0;
}

void sr130pc10_regs_table_exit(void)
{
	CAM_PRINTK("%s start\n", __func__);

	if(front_regs_buf_ptr) 
	{
		vfree(front_regs_buf_ptr);
		front_regs_buf_ptr = NULL;
	}

	CAM_PRINTK("%s done\n", __func__);
}
#endif // FRONT_CONFIG_LOAD_FILE

/**************************************************************************
* FUNCTIONS
***************************************************************************/
/**************************************************************************
 * sr130pc10_i2c_read_byte: Read (I2C) multiple bytes to the camera sensor 
 * @client: pointer to i2c_client
 * @subaddr: data to be written
 * @*data: buffer where data is read
 *
 * Returns 0 on success, <0 on error
 ***************************************************************************/
static int sr130pc10_i2c_read_byte(struct i2c_client *client,  
                                     unsigned short subaddr, 
                                     unsigned short *data)
{
	unsigned char buf[2] = {0,};
	struct i2c_msg msg = {client->addr, 0, 1, buf};
	int err = 0;

	if(!client->adapter)
	{
		CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! can't search i2c client adapter\n", __FILE__, __LINE__);
		return -EIO;
	} 

	buf[0] = (unsigned char)subaddr;

	err = i2c_transfer(client->adapter, &msg, 1);
	if(err < 0)
	{
		CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! %d register read failed\n", __FILE__, __LINE__, subaddr);	
		return -EIO;
	}

	msg.flags = I2C_M_RD;
	msg.len = 1;

	err = i2c_transfer(client->adapter, &msg, 1);
	if(err < 0)
	{
		CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! %d register read failed\n", __FILE__, __LINE__, subaddr);	
		return -EIO;
	}

    *data = (unsigned short)buf[0];

	return 0;
}

/**************************************************************************
 * sr130pc10_i2c_write_byte: Write (I2C) multiple bytes to the camera sensor 
 * @client: pointer to i2c_client
 * @subaddr: register address
 * @data: data to be written
 *
 * Returns 0 on success, <0 on error
 ***************************************************************************/
static int sr130pc10_i2c_write_byte(struct i2c_client *client, 
                                     unsigned short subaddr, 
                                     unsigned short data)
{
	unsigned char buf[2] = {0,};
	struct i2c_msg msg = {client->addr, 0, 2, buf};	
	int err = 0;

	if(!client->adapter)
	{
		CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! can't search i2c client adapter\n", __FILE__, __LINE__);
		return -EIO;
	} 

	buf[0] = subaddr & 0xFF;
	buf[1] = data & 0xFF;	

	err = i2c_transfer(client->adapter, &msg, 1);

	return (err == 1)? 0 : -EIO;
}

/**************************************************************************
 * sr130pc10_i2c_read_word: Read (I2C) multiple bytes to the camera sensor 
 * @client: pointer to i2c_client
 * @subaddr: data to be written
 * @*data: buffer where data is read
 *
 * Returns 0 on success, <0 on error
 ***************************************************************************/
static int sr130pc10_i2c_read_word(struct i2c_client *client,  
                                     unsigned short subaddr, 
                                     unsigned short *data)
{
	unsigned char buf[4];
	struct i2c_msg msg = {client->addr, 0, 2, buf};
	int err = 0;

	if(!client->adapter)
	{
		CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! can't search i2c client adapter\n", __FILE__, __LINE__);
		return -EIO;
	} 

	buf[0] = subaddr>> 8;
	buf[1] = subaddr & 0xff;

	err = i2c_transfer(client->adapter, &msg, 1);
	if(err < 0)
	{
		CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! %d register read failed\n", __FILE__, __LINE__, subaddr);	
		return -EIO;
	}

	msg.flags = I2C_M_RD;
	msg.len = 2;

	err = i2c_transfer(client->adapter, &msg, 1);
	if(err < 0)
	{
		CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! %d register read failed\n", __FILE__, __LINE__, subaddr);	
		return -EIO;
	}

    *data = ((buf[0] << 8) | buf[1]);

	return 0;
}

/**************************************************************************
 * sr130pc10_i2c_write_word: Write (I2C) multiple bytes to the camera sensor 
 * @client: pointer to i2c_client
 * @subaddr: register address
 * @data: data to be written
 *
 * Returns 0 on success, <0 on error
 ***************************************************************************/
static int sr130pc10_i2c_write_word(struct i2c_client *client, 
                                     unsigned short subaddr, 
                                     unsigned short data)
{
	unsigned char buf[4];
	struct i2c_msg msg = {client->addr, 0, 4, buf};	
	int err = 0;

	if(!client->adapter)
	{
		CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! can't search i2c client adapter\n", __FILE__, __LINE__);
		return -EIO;
	} 

	buf[0] = subaddr >> 8;
	buf[1] = subaddr & 0xFF;	
	buf[2] = data >> 8;
	buf[3] = data & 0xFF;

	err = i2c_transfer(client->adapter, &msg, 1);

	return (err == 1)? 0 : -EIO;
}

/**************************************************************************
 * sr130pc10_i2c_set_data_burst: Write (I2C) multiple bytes to the camera sensor 
 * @client: pointer to i2c_client
 * @reg_buffer: buffer which includes all registers to be written.
 * @num_of_regs: number of registers to be written.
 * @name : This will be used for tuning.
 * Returns 0 on success, <0 on error
 ***************************************************************************/
static int sr130pc10_i2c_set_data_burst(struct i2c_client *client, 
                                         regs_short_t reg_buffer[], 
          				                 int num_of_regs)
{
    unsigned short subaddr, data_value;
    int i, err = 0;

    for(i = 0; i < num_of_regs; i++)
    {
        subaddr = reg_buffer[i].subaddr;
        data_value = reg_buffer[i].value;

        switch(subaddr)
        {
            case DELAY_SEQ:
            {
                //CAM_ERROR_MSG(&client->dev,"delay = %d\n",data_value*10);
                msleep(data_value * 10);
                break;
            }

            default:
            {
                err = sr130pc10_i2c_write_byte(client, subaddr, data_value);
            	if(err < 0)
            	{
            		CAM_ERROR_MSG(&client->dev, "[%s: %d] i2c write fail\n", __FILE__, __LINE__);	
            		return -EIO;
            	}
            	break;
            }            
        }
    }

    return 0;
}

/**************************************************************************
 * sr130pc10_i2c_set_config_register: Write (I2C) multiple bytes to the camera sensor 
 * @client: pointer to i2c_client
 * @reg_buffer: buffer which includes all registers to be written.
 * @num_of_regs: number of registers to be written.
 * @name : This will be used for tuning.
 * Returns 0 on success, <0 on error
 ***************************************************************************/
static int sr130pc10_i2c_set_config_register(struct i2c_client *client, 
                                         regs_short_t reg_buffer[], 
          				                 int num_of_regs, 
          				                 char *name)
{
    int err = 0;

#ifdef FRONT_CONFIG_LOAD_FILE 
	err = sr130pc10_regs_table_write(client, name);
#else
    err = sr130pc10_i2c_set_data_burst(client, reg_buffer, num_of_regs);
#endif // FRONT_CONFIG_LOAD_FILE

    return err;
}

/**************************************************************************
 * sr130pc10_set_frame_rate
 *
 * Returns 0 on success, <0 on error
 ***************************************************************************/
static int sr130pc10_set_frame_rate(struct v4l2_subdev *sd, int fps)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct sensor_state *state = to_state(sd);
	int err = 0;

    CAM_INFO_MSG(&client->dev, "%s : %d fps ~~~~~~~~~~~~~~\n", __func__, fps);	    
    {
    	switch(fps)
    	{
    		case FRAME_RATE_AUTO:
    		{
    			err = sr130pc10_i2c_set_config_register(client, 
    			                                  front_fps_auto_regs, 
    			                                  FPS_AUTO_NUM_OF_REGS, 
    			                                  "front_fps_auto_regs");
    			break;
            }
            
    		case FRAME_RATE_7:
    		{
    			err = sr130pc10_i2c_set_config_register(client, 
    			                                  front_fps_7_regs, 
    			                                  FPS_7_NUM_OF_REGS, 
    			                                  "front_fps_7_regs");
    			break;
            }
            
    		case FRAME_RATE_10:
    		{
    			err = sr130pc10_i2c_set_config_register(client, 
    			                                  front_fps_10_regs, 
    			                                  FPS_10_NUM_OF_REGS, 
    			                                  "front_fps_10_regs");
    			break;
            }
            
    		case FRAME_RATE_15:
    		{
    			err = sr130pc10_i2c_set_config_register(client, 
    			                                  front_fps_15_regs, 
    			                                  FPS_15_NUM_OF_REGS, 
    			                                  "front_fps_15_regs");
    			break;
            }
            
    		default:
    		{
    			CAM_WARN_MSG(&client->dev, "[%s : %d] WARNING! unsupported %d fps\n", 
    			            __FILE__, __LINE__, fps);
                err = -EIO;       
    			break;
            }			
    	}    
	}
    
	if(err < 0)
	{
		CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! frame rate set failed\n", 
		                __FILE__, __LINE__);
		return -EIO;
	}

        if(fps== FRAME_RATE_AUTO)
        {
            state->fps = 15;    // default 15
        }	
        else
        {
            state->fps = fps;
        }
	
	return 0;
}

/**************************************************************************
 * sr130pc10_set_preview_stop
 *
 * Returns 0 on success, <0 on error
 ***************************************************************************/
static int sr130pc10_set_preview_stop(struct v4l2_subdev *sd)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);

	CAM_INFO_MSG(&client->dev, "%s ~~~~~~~~~~~~~~\n", __func__);	
	return 0;
}

/**************************************************************************
 * sr130pc10_set_dzoom
 *
 * Returns 0 on success, <0 on error
 ***************************************************************************/
static int sr130pc10_set_dzoom(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);

    CAM_INFO_MSG(&client->dev, "[%s] Not support set zoom\n", __func__);
	return 0;
}

/**************************************************************************
 * sr130pc10_set_preview_size
 *
 * Returns 0 on success, <0 on error
 ***************************************************************************/
static int sr130pc10_set_preview_start(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct sensor_state *state = to_state(sd);	
	int err = 0;

    CAM_INFO_MSG(&client->dev, "%s ~~~~~~~~~~~~~~\n", __func__);

	if(!state->pix.width || !state->pix.height)
	{
	    CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! Could not start preview\n", __FILE__, __LINE__);
		return -EINVAL;
	}

	err = sr130pc10_i2c_set_config_register(client, 
	                                  front_preview_camera_regs, 
	                                  PREVIEW_CAMERA_NUM_OF_REGS, 
	                                  "front_preview_camera_regs");
	if(err < 0)
	{
		CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! Could not start preview\n", __FILE__, __LINE__);
		return -EIO;
	}

	if(state->check_dataline) // Output Test Pattern
	{
        err = sr130pc10_check_dataline_onoff(sd, 1);	
        
		if(err < 0)
		{
			CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! Pattern setting failed\n", __FILE__, __LINE__);	
			return -EIO;
		}		
	}
    
	return 0;
}

static int sr130pc10_set_jpeg_quality(struct v4l2_subdev *sd)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);

    CAM_INFO_MSG(&client->dev, "[%s] Not support set quality\n", __func__);
	return 0;
}

static int sr130pc10_set_capture_start(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int err = 0;
    unsigned short read_value = 0;

    CAM_INFO_MSG(&client->dev,"%s",__func__);
    
    // Initialize...
	front_gISOSpeedRating = 100;
	front_gExposureTime = 0;	

    /* Set Snapshot registers */ 
	err = sr130pc10_i2c_set_config_register(client, 
	                                  front_snapshot_normal_regs, 
	                                  SNAPSHOT_NORMAL_NUM_OF_REGS, 
	                                  "front_snapshot_normal_regs");
	if(err < 0)
	{
        CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! Could not take a picture\n", __FILE__, __LINE__);
		return -EIO;
	}
 
    // Get iso speed rating and exposure time for EXIF.
    front_gISOSpeedRating = sr130pc10_get_iso_speed_rate(sd);
    front_gExposureTime = sr130pc10_get_shutterspeed(sd);
	
    sr130pc10_i2c_write_byte(client, 0x03, 0x00);   // Page mode : 0x00
    sr130pc10_i2c_read_byte(client, 0x01, &read_value);

    if(read_value != 0xF8){
		CAM_ERROR_MSG(&client->dev, "sleep register : 0x%x\n",read_value);
		sr130pc10_i2c_write_byte(client, 0x03, 0x00);   // Page mode : 0x00
		sr130pc10_i2c_write_byte(client, 0x01, 0xf8);
		msleep(50);
    	}
	
	return 0;
}

static int sr130pc10_set_scene_mode(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);

    CAM_INFO_MSG(&client->dev, "[%s] Not support set scene mode\n", __func__);
	return 0;
}

static int sr130pc10_set_effect(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;

	CAM_INFO_MSG(&client->dev, "%s: setting value =%d\n", __func__, ctrl->value);

	switch(ctrl->value) 
	{
		case IMAGE_EFFECT_NONE:
		{
			err = sr130pc10_i2c_set_config_register(client, 
			                                  front_effect_normal_regs, 
			                                  EFFECT_NORMAL_NUM_OF_REGS, 
			                                  "front_effect_normal_regs");
			break;
		}
		
		case IMAGE_EFFECT_BNW:
		{
			err = sr130pc10_i2c_set_config_register(client, 
			                                  front_effect_mono_regs, 
			                                  EFFECT_MONO_NUM_OF_REGS, 
			                                  "front_effect_mono_regs");
			break;
		}
			
		case IMAGE_EFFECT_SEPIA:
		case IMAGE_EFFECT_ANTIQUE:
		{
			err = sr130pc10_i2c_set_config_register(client, 
			                                  front_effect_sepia_regs, 
			                                  EFFECT_SEPIA_NUM_OF_REGS, 
			                                  "front_effect_sepia_regs");
			break;
		}
			
		case IMAGE_EFFECT_NEGATIVE:
		{
			err = sr130pc10_i2c_set_config_register(client, 
			                                  front_effect_negative_regs, 
			                                  EFFECT_NEGATIVE_NUM_OF_REGS, 
			                                  "front_effect_negative_regs");
			break;
		}
			
		case IMAGE_EFFECT_AQUA:
		case IMAGE_EFFECT_SHARPEN:
		default:
		{
		    CAM_WARN_MSG(&client->dev, "[%s : %d] WARNING! unsupported effect(%d)\n", __FILE__, __LINE__, ctrl->value);
			break;
        }			
	}

	if(err < 0)
	{
	    CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! effect set failed\n", __FILE__, __LINE__);
		return -EIO;
	}
	
	return 0;
}

static int sr130pc10_set_white_balance(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;

	CAM_INFO_MSG(&client->dev, "%s: setting value =%d\n", __func__, ctrl->value);

	switch(ctrl->value)
	{
		case WHITE_BALANCE_AUTO:
		{
			err = sr130pc10_i2c_set_config_register(client, 
			                                  front_wb_auto_regs, 
			                                  WB_AUTO_NUM_OF_REGS, 
			                                  "front_wb_auto_regs");
			break;
		}

		case WHITE_BALANCE_SUNNY:
		{
			err = sr130pc10_i2c_set_config_register(client, 
			                                  front_wb_sunny_regs, 
			                                  WB_SUNNY_NUM_OF_REGS, 
			                                  "front_wb_sunny_regs");
			break;
		}

		case WHITE_BALANCE_CLOUDY:
		{
			err = sr130pc10_i2c_set_config_register(client, 
			                                  front_wb_cloudy_regs, 
			                                  WB_CLOUDY_NUM_OF_REGS, 
			                                  "front_wb_cloudy_regs");
			break;
		}

		case WHITE_BALANCE_TUNGSTEN:  // WHITE_BALANCE_INCANDESCENT:
		{
			err = sr130pc10_i2c_set_config_register(client, 
			                                  front_wb_tungsten_regs, 
			                                  WB_TUNSTEN_NUM_OF_REGS, 
			                                  "front_wb_tungsten_regs");
			break;
		}
		
		case WHITE_BALANCE_FLUORESCENT:
		{
			err = sr130pc10_i2c_set_config_register(client, 
			                                  front_wb_fluorescent_regs, 
			                                  WB_FLUORESCENT_NUM_OF_REGS, 
			                                  "front_wb_fluorescent_regs");
			break;
		}

		default:
		{
		    CAM_WARN_MSG(&client->dev, "[%s : %d] WARNING! unsupported wb(%d)\n", __FILE__, __LINE__, ctrl->value);
			break;
        }			
	}

	if(err < 0)
	{
	    CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! white balance set failed\n", __FILE__, __LINE__);
		return -EIO;
	}

    front_gCurrentWB = ctrl->value;
    
	return 0;
}

static int sr130pc10_set_metering(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

    CAM_INFO_MSG(&client->dev, "[%s] Not support set metering mode\n", __func__);
	return 0;
}

static int sr130pc10_set_iso(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);

    CAM_INFO_MSG(&client->dev, "[%s] Not support set ISO\n", __func__);
	return 0;
}

static int sr130pc10_set_ev(struct v4l2_subdev *sd, int value)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct sensor_state *state = to_state(sd);
	int err = 0;

    CAM_INFO_MSG(&client->dev, "%s: setting value =%d\n", __func__, value);
#if 0
    if(state->vt_mode == VT_MODE_ON)
    {
    	switch(ctrl->value)
    	{
    		case EV_MINUS_4:
    		{
    			err = sr130pc10_i2c_set_config_register(client, 
    			                                  front_ev_vt_minus_4_regs, 
    			                                  EV_VT_M4_NUM_OF_REGS, 
    			                                  "front_ev_vt_minus_4_regs");
    			break;
            }

    		case EV_MINUS_3:
    		{
    			err = sr130pc10_i2c_set_config_register(client, 
    			                                  front_ev_vt_minus_3_regs, 
    			                                  EV_VT_M3_NUM_OF_REGS, 
    			                                  "front_ev_vt_minus_3_regs");
    			break;
            }

    		case EV_MINUS_2:
    		{
    			err = sr130pc10_i2c_set_config_register(client, 
    			                                  front_ev_vt_minus_2_regs, 
    			                                  EV_VT_M2_NUM_OF_REGS, 
    			                                  "front_ev_vt_minus_2_regs");
    			break;
            }

    		case EV_MINUS_1:
    		{
    			err = sr130pc10_i2c_set_config_register(client, 
    			                                  front_ev_vt_minus_1_regs, 
    			                                  EV_VT_M1_NUM_OF_REGS, 
    			                                  "front_ev_vt_minus_1_regs");
    			break;
            }

    		case EV_DEFAULT:
    		{
    			err = sr130pc10_i2c_set_config_register(client, 
    			                                  front_ev_vt_default_regs, 
    			                                  EV_VT_DEFAULT_NUM_OF_REGS, 
    			                                  "front_ev_vt_default_regs");
    			break;
            }

    		case EV_PLUS_1:
    		{
    			err = sr130pc10_i2c_set_config_register(client, 
    			                                  front_ev_vt_plus_1_regs, 
    			                                  EV_VT_P1_NUM_OF_REGS, 
    			                                  "front_ev_vt_plus_1_regs");
    			break;
            }

    		case EV_PLUS_2:
    		{
    			err = sr130pc10_i2c_set_config_register(client, 
    			                                  front_ev_vt_plus_2_regs, 
    			                                  EV_VT_P2_NUM_OF_REGS, 
    			                                  "front_ev_vt_plus_2_regs");
    			break;
            }
    		
    		case EV_PLUS_3:
    		{
    			err = sr130pc10_i2c_set_config_register(client, 
    			                                  front_ev_vt_plus_3_regs, 
    			                                  EV_VT_P3_NUM_OF_REGS, 
    			                                  "front_ev_vt_plus_3_regs");
    			break;
            }

    		case EV_PLUS_4:
    		{
    			err = sr130pc10_i2c_set_config_register(client, 
    			                                  front_ev_vt_plus_4_regs, 
    			                                  EV_VT_P4_NUM_OF_REGS, 
    			                                  "front_ev_vt_plus_4_regs");
    			break;
            }

    		default:
    		{
    			CAM_WARN_MSG(&client->dev, "[%s : %d] WARNING! unsupported ev(%d)\n", __FILE__, __LINE__, ctrl->value);
    			break;
            }			
    	}
    }
    else
#endif        
    {
    	switch(value)
    	{
    		case EV_MINUS_4:
    		{
    			err = sr130pc10_i2c_set_config_register(client, 
    			                                  front_ev_minus_4_regs, 
    			                                  EV_M4_NUM_OF_REGS, 
    			                                  "front_ev_minus_4_regs");
    			break;
            }

    		case EV_MINUS_3:
    		{
    			err = sr130pc10_i2c_set_config_register(client, 
    			                                  front_ev_minus_3_regs, 
    			                                  EV_M3_NUM_OF_REGS, 
    			                                  "front_ev_minus_3_regs");
    			break;
            }

    		case EV_MINUS_2:
    		{
    			err = sr130pc10_i2c_set_config_register(client, 
    			                                  front_ev_minus_2_regs, 
    			                                  EV_M2_NUM_OF_REGS, 
    			                                  "front_ev_minus_2_regs");
    			break;
            }

    		case EV_MINUS_1:
    		{
    			err = sr130pc10_i2c_set_config_register(client, 
    			                                  front_ev_minus_1_regs, 
    			                                  EV_M1_NUM_OF_REGS, 
    			                                  "front_ev_minus_1_regs");
    			break;
            }

    		case EV_DEFAULT:
    		{
    			err = sr130pc10_i2c_set_config_register(client, 
    			                                  front_ev_default_regs, 
    			                                  EV_DEFAULT_NUM_OF_REGS, 
    			                                  "front_ev_default_regs");
    			break;
            }

    		case EV_PLUS_1:
    		{
    			err = sr130pc10_i2c_set_config_register(client, 
    			                                  front_ev_plus_1_regs, 
    			                                  EV_P1_NUM_OF_REGS, 
    			                                  "front_ev_plus_1_regs");
    			break;
            }

    		case EV_PLUS_2:
    		{
    			err = sr130pc10_i2c_set_config_register(client, 
    			                                  front_ev_plus_2_regs, 
    			                                  EV_P2_NUM_OF_REGS, 
    			                                  "front_ev_plus_2_regs");
    			break;
            }
    		
    		case EV_PLUS_3:
    		{
    			err = sr130pc10_i2c_set_config_register(client, 
    			                                  front_ev_plus_3_regs, 
    			                                  EV_P3_NUM_OF_REGS, 
    			                                  "front_ev_plus_3_regs");
    			break;
            }

    		case EV_PLUS_4:
    		{
    			err = sr130pc10_i2c_set_config_register(client, 
    			                                  front_ev_plus_4_regs, 
    			                                  EV_P4_NUM_OF_REGS, 
    			                                  "front_ev_plus_4_regs");
    			break;
            }

    		default:
    		{
    			CAM_WARN_MSG(&client->dev, "[%s : %d] WARNING! unsupported ev(%d)\n", __FILE__, __LINE__, value);
    			break;
            }			
    	}
	}
    
	if(err < 0)
	{
		CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! ev set failed\n", __FILE__, __LINE__);
		return -EIO;
	}

	return 0;
}

static int sr130pc10_set_saturation(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	CAM_INFO_MSG(&client->dev, "[%s] Not support set saturation\n", __func__);
	return 0;
}

static int sr130pc10_set_contrast(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

    CAM_INFO_MSG(&client->dev, "[%s] Not support set contrast\n", __func__);
	return 0;
}

static int sr130pc10_set_sharpness(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

    CAM_INFO_MSG(&client->dev, "[%s] Not support set sharpness\n", __func__);
	return 0;
}

static int sr130pc10_set_focus_mode(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

    CAM_INFO_MSG(&client->dev, "[%s] Not support set focus mode\n", __func__);
	return 0;
}

static int sr130pc10_get_iso_speed_rate(struct v4l2_subdev *sd)
{
#if 0    
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    unsigned short read_value = 0;
    int GainValue = 0;
    int isospeedrating = 100;

    sr130pc10_i2c_write_word(client, 0xFCFC, 0xD000);
    sr130pc10_i2c_write_word(client, 0x002C, 0x7000);
    sr130pc10_i2c_write_word(client, 0x002E, 0x2A18);
    sr130pc10_i2c_read_word(client, 0x0F12, &read_value);

    GainValue = ((read_value * 10) / 256);

    if(GainValue < 19)
    {
        isospeedrating = 50;
    }
    else if(GainValue < 23)
    {
        isospeedrating = 100;
    }
    else if(GainValue < 28)
    {
        isospeedrating = 200;
    }
    else
    {
        isospeedrating = 400;
    }

    return isospeedrating;
#else
    return 100;
#endif
}

static int sr130pc10_get_shutterspeed(struct v4l2_subdev *sd)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);	
    unsigned short read_value1 = 0, read_value2 = 0, read_value3 = 0;
    int ShutterSpeed = 0;

    sr130pc10_i2c_write_byte(client, 0x03, 0x20);   // Page mode : 0x20
    sr130pc10_i2c_read_byte(client, 0x80, &read_value1);
    sr130pc10_i2c_read_byte(client, 0x81, &read_value2);
    sr130pc10_i2c_read_byte(client, 0x82, &read_value3);

    ShutterSpeed = ((read_value1 << 19) + (read_value2 << 11) + (read_value3 << 3)) / 24;   // 24Mhz

    CAM_INFO_MSG(&client->dev,"ShutterSpeed=%d,read_value1=%d,read_value2=%d,read_value3=%d",
                ShutterSpeed,read_value1,read_value2,read_value3);
    
    return ShutterSpeed;
}

static void sr130pc10_init_parameters(struct v4l2_subdev *sd)
{
	struct sensor_state *state = to_state(sd);

    /* Default value */	
    state->fps = 15;
    state->capture_mode = V4L2_MODE_PREVIEW;
    
// Don't set this here.    
//    state->vt_mode = VT_MODE_OFF;

// Don't set this here.
//    state->sensor_mode = SENSOR_MODE_CAMERA;
    
	/* Set initial values for the sensor stream parameters */
	state->strm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	state->strm.parm.capture.timeperframe.numerator = 1;
	state->strm.parm.capture.timeperframe.denominator = state->fps;
	state->strm.parm.capture.capturemode = 0;

	state->postview_info.width = 320;
	state->postview_info.height = 240;

	front_gCurrentWB = WHITE_BALANCE_AUTO;
}

#if 0
/* Sample code */
static const char *sr130pc10_querymenu_wb_preset[] = {
	"WB Tungsten", "WB Fluorescent", "WB sunny", "WB cloudy", NULL
};
#endif

static struct v4l2_queryctrl sr130pc10_controls[] = 
{
#if 0
	/* Sample code */
	{
		.id = V4L2_CID_WHITE_BALANCE_PRESET,
		.type = V4L2_CTRL_TYPE_MENU,
		.name = "White balance preset",
		.minimum = 0,
		.maximum = ARRAY_SIZE(sr130pc10_querymenu_wb_preset) - 2,
		.step = 1,
		.default_value = 0,
	},
#endif
};

const char **sr130pc10_ctrl_get_menu(u32 id)
{
	switch (id) 
	{
#if 0
		/* Sample code */
		case V4L2_CID_WHITE_BALANCE_PRESET:
			return sr130pc10_querymenu_wb_preset;
#endif
		default:
			return v4l2_ctrl_get_menu(id);
	}
}

static inline struct v4l2_queryctrl const *sr130pc10_find_qctrl(int id)
{
	int i = 0;

	for (i = 0; i < ARRAY_SIZE(sr130pc10_controls); i++)
	{
		if (sr130pc10_controls[i].id == id)
		{
			return &sr130pc10_controls[i];
		}
	}

	return NULL;
}

static int sr130pc10_queryctrl(struct v4l2_subdev *sd, struct v4l2_queryctrl *qc)
{
	int i = 0;

	for (i = 0; i < ARRAY_SIZE(sr130pc10_controls); i++) 
	{
		if (sr130pc10_controls[i].id == qc->id) 
		{
			memcpy(qc, &sr130pc10_controls[i], sizeof(struct v4l2_queryctrl));
			return 0;
		}
	}

	return -EINVAL;
}

static int sr130pc10_querymenu(struct v4l2_subdev *sd, struct v4l2_querymenu *qm)
{
	struct v4l2_queryctrl qctrl;

	qctrl.id = qm->id;
	sr130pc10_queryctrl(sd, &qctrl);

	return v4l2_ctrl_query_menu(qm, &qctrl, sr130pc10_ctrl_get_menu(qm->id));
}

/*
 * Clock configuration
 * Configure expected MCLK from host and return EINVAL if not supported clock
 * frequency is expected
 * 	freq : in Hz
 * 	flag : not supported for now
 */
static int sr130pc10_s_crystal_freq(struct v4l2_subdev *sd, u32 freq, u32 flags)
{
	int err = -EINVAL;

	return err;
}

static int sr130pc10_g_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
	int err = 0;

	return err;
}

static int sr130pc10_s_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
	struct sensor_state *state = to_state(sd);

	state->pix.width = fmt->fmt.pix.width;
	state->pix.height = fmt->fmt.pix.height;
	state->pix.pixelformat = fmt->fmt.pix.pixelformat;
	
	return 0;
}

static int sr130pc10_enum_framesizes(struct v4l2_subdev *sd, struct v4l2_frmsizeenum *fsize)
{
	struct sensor_state *state = to_state(sd);

	/* The camera interface should read this value, this is the resolution
 	 * at which the sensor would provide framedata to the camera i/f
 	 *
 	 * In case of image capture, this returns the default camera resolution (WVGA)
 	 */
	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;

    if(state->capture_mode == V4L2_MODE_CAPTURE)
    {
        fsize->discrete.width = SENSOR_MAX_WIDTH;
        fsize->discrete.height = SENSOR_MAX_HEIGHT;
    }
    else
    {
        fsize->discrete.width = PREVIEW_BASE_WIDTH;
        fsize->discrete.height = PREVIEW_BASE_HEIGHT;
    }
    
	return 0;
}

static int sr130pc10_enum_frameintervals(struct v4l2_subdev *sd, struct v4l2_frmivalenum *fival)
{
	return 0;
}

static int sr130pc10_enum_fmt(struct v4l2_subdev *sd, struct v4l2_fmtdesc *fmtdesc)
{
	return 0;
}

static int sr130pc10_try_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
	return 0;
}

static int sr130pc10_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct sensor_state *state = to_state(sd);
	
	int err = 0;

	state->strm.parm.capture.timeperframe.numerator = 1;
	state->strm.parm.capture.timeperframe.denominator = state->fps;

	memcpy(param, &state->strm, sizeof(param));

	return err;
}

static int sr130pc10_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
    int err = 0;
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct sensor_state *state = to_state(sd);
    struct sec_cam_parm *new_parms =
        (struct sec_cam_parm *)&param->parm.raw_data;
    struct sec_cam_parm *parms =
        (struct sec_cam_parm *)&state->strm.parm.raw_data;

    dev_dbg(&client->dev, "%s: start\n", __func__);

    if (param->parm.capture.timeperframe.numerator !=
        parms->capture.timeperframe.numerator ||
        param->parm.capture.timeperframe.denominator !=
        parms->capture.timeperframe.denominator) {

        int fps = 0;
        int fps_max = 15;

        if (param->parm.capture.timeperframe.numerator &&
            param->parm.capture.timeperframe.denominator)
            fps =
                (int)(param->parm.capture.timeperframe.denominator /
                  param->parm.capture.timeperframe.numerator);
        else
            fps = 0;

        if (fps <= 0 || fps > fps_max) {
            dev_dbg(&client->dev,
                "%s: Framerate %d not supported,"
                " setting it to %d fps.\n",
                __func__, fps, fps_max);
            fps = fps_max;
        }

        /*
         * Don't set the fps value, just update it in the state
         * We will set the resolution and
         * fps in the start operation (preview/capture) call
         */
        parms->capture.timeperframe.numerator = 1;
        parms->capture.timeperframe.denominator = fps;
        
        state->fps = fps;
    }

    sr130pc10_set_ev(sd, new_parms->brightness);
	
    dev_dbg(&client->dev, " set frame rate %d\n",parms->capture.timeperframe.denominator);

    if(state->sensor_mode == 1){
        sr130pc10_set_frame_rate(sd,state->fps);
    }else{
        sr130pc10_set_frame_rate(sd,state->fps !=15?state->fps:0);
    }
    
    return 0;
}



static int sr130pc10_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct sensor_state *state = to_state(sd);
	struct sensor_userset userset = state->userset;	
	int err = 0;

	CAM_INFO_MSG(&client->dev, "%s : control id = 0x%x, %d\n", __func__, ctrl->id, ctrl->id & 0xFF);
	
	switch (ctrl->id) 
	{
		case V4L2_CID_EXPOSURE:
		{
			ctrl->value = userset.exposure_bias;
			break;
		}
		
		case V4L2_CID_AUTO_WHITE_BALANCE:
		{
			ctrl->value = userset.auto_wb;
			break;
		}
		
		case V4L2_CID_WHITE_BALANCE_PRESET:
		{
			ctrl->value = userset.manual_wb;
			break;
		}
		
		case V4L2_CID_COLORFX:
		{
			ctrl->value = userset.effect;
			break;
		}
		
		case V4L2_CID_CONTRAST:
		{
			ctrl->value = userset.contrast;
			break;
		}
		
		case V4L2_CID_SATURATION:
		{
			ctrl->value = userset.saturation;
			break;
		}
		
		case V4L2_CID_SHARPNESS:
		{
			ctrl->value = userset.sharpness;
			break;
        }
        
		case V4L2_CID_CAM_JPEG_QUALITY:
		{
			break;
        }
        
		case V4L2_CID_CAMERA_OBJ_TRACKING_STATUS:
		{
			break;
		}
		
		case V4L2_CID_CAMERA_SMART_AUTO_STATUS:
		{
			break;
        }
        
		case V4L2_CID_CAMERA_AUTO_FOCUS_RESULT:
		{
			break;
        }

#if 0        
		case V4L2_CID_CAMERA_FLASH_CHECK:
		{
			ctrl->value = 0;
			break;
		}
#endif
        case V4L2_CID_CAMERA_GET_ISO:
        {
            ctrl->value = front_gISOSpeedRating;
            break;
        }

        case V4L2_CID_CAMERA_GET_SHT_TIME:
        {
            ctrl->value = front_gExposureTime;
            break;	
        }

        default:
        {
            CAM_ERROR_MSG(&client->dev, "[%s: %d] ERROR! not support g_ctrl(%d)\n", __FILE__, __LINE__, ctrl->id);
            return -ENOIOCTLCMD;
        }			
	}
	
	return err;
}

static int sr130pc10_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct sensor_state *state = to_state(sd);
	int err = 0;

	//CAM_ERROR_MSG(&client->dev, "%s: V4l2 control ID =%d\n", __func__, ctrl->id - V4L2_CID_PRIVATE_BASE);

	if(state->check_dataline)
	{
        if((ctrl->id != V4L2_CID_CAM_PREVIEW_ONOFF) &&
           (ctrl->id != V4L2_CID_CAMERA_CHECK_DATALINE_STOP) &&
           (ctrl->id != V4L2_CID_CAMERA_CHECK_DATALINE))
        {
            return 0;
        }
	}
    
	mutex_lock(&front_sensor_s_ctrl);

	switch(ctrl->id) 
	{
	       case V4L2_CID_CAMERA_CAPTURE_MODE:
              {
                  state->capture_mode = ctrl->value;
                  break;
              }
           
		case V4L2_CID_CAMERA_VT_MODE:
		{
		    state->vt_mode = ctrl->value;
		    
			CAM_INFO_MSG(&client->dev, "vt mode = %d\n", ctrl->value);
			break;
		}
		
		case V4L2_CID_CAMERA_FLASH_MODE:
		{
			break;
		}
		
		case V4L2_CID_CAMERA_BRIGHTNESS:
		{
			err = sr130pc10_set_ev(sd, ctrl->value);
			break;
		}
		
		case V4L2_CID_CAMERA_WHITE_BALANCE:
		{
			err = sr130pc10_set_white_balance(sd, ctrl);
			break;
		}
		
		case V4L2_CID_CAMERA_EFFECT:
		{
			err = sr130pc10_set_effect(sd, ctrl);
			break;
		}
		
		case V4L2_CID_CAMERA_ISO:
		{
			err = sr130pc10_set_iso(sd, ctrl);
			break;
		}
		
		case V4L2_CID_CAMERA_METERING:
		{
			err = sr130pc10_set_metering(sd, ctrl);
			break;
		}
		
		case V4L2_CID_CAMERA_CONTRAST:
		{
			err = sr130pc10_set_contrast(sd, ctrl);
			break;
        }
        
		case V4L2_CID_CAMERA_SATURATION:
		{
			err = sr130pc10_set_saturation(sd, ctrl);
			break;
        }
		
		case V4L2_CID_CAMERA_SHARPNESS:
		{
			err = sr130pc10_set_sharpness(sd, ctrl);
			break;
        }
        
		/*Camcorder fix fps*/
		case V4L2_CID_CAMERA_SENSOR_MODE:
		{
		    state->sensor_mode = ctrl->value;
		    
			CAM_INFO_MSG(&client->dev, "sensor mode = %d\n", ctrl->value);
			break;
        }
        
		case V4L2_CID_CAMERA_FOCUS_MODE:
		{
			err = sr130pc10_set_focus_mode(sd, ctrl);
			break;
        }
        
		// need to be modified
		case V4L2_CID_CAM_JPEG_QUALITY:
		{
			if(ctrl->value < 0 || ctrl->value > 100)
			{
				err = -EINVAL;
			} 
			else 
			{
				err = sr130pc10_set_jpeg_quality(sd);
			}
			break;
        }
        
		case V4L2_CID_CAMERA_SCENE_MODE:
		{
			err = sr130pc10_set_scene_mode(sd, ctrl);
			break;
        }
        
		case V4L2_CID_CAMERA_ZOOM:
		{
			err = sr130pc10_set_dzoom(sd, ctrl);
			break;
        }
        
		case V4L2_CID_CAMERA_FRAME_RATE:
		{
			err = sr130pc10_set_frame_rate(sd, ctrl->value);
			err = 0;
			break;
		}
		
		case V4L2_CID_CAM_CAPTURE:
		{
			err = sr130pc10_set_capture_start(sd, ctrl);
			break;
		}
#if 0
		case V4L2_CID_CAM_CAPTURE_DONE:
		{
			break;
		}
#endif		
		/* Used to start / stop preview operation. 
	 	 * This call can be modified to START/STOP operation, which can be used in image capture also */
		case V4L2_CID_CAM_PREVIEW_ONOFF:
		{
			if(ctrl->value)
			{
				err = sr130pc10_set_preview_start(sd);
            }				
			else
			{
				err = sr130pc10_set_preview_stop(sd);
            }				
			break;
        }
        
		case V4L2_CID_CAMERA_CHECK_DATALINE:
		{
			state->check_dataline = ctrl->value;
			err = 0;
			break;	
        }
        
		case V4L2_CID_CAMERA_CHECK_DATALINE_STOP:
		{
			err = sr130pc10_check_dataline_onoff(sd, 0);
			break;	
        }

        case V4L2_CID_CAMERA_RESET:
        {
            dev_dbg(&client->dev, "%s: V4L2_CID_CAMERA_RESET \n", __func__);
            // err = sr130pc10_reset(sd);
            break;
        }
#if 0        
        case V4L2_CID_CAMCORER_MODE:
        {
            // state->camcorder_mode = ctrl->value;
            break;
        }
#endif
        case V4L2_CID_CAMERA_MON_MOVIE_SELECT:
        {
            break;
        }

	case V4L2_CID_CAMERA_SET_AUTO_FOCUS:
	{
		err = -1;
		break;
	}		
	
	default:
	{
		//err = -ENOTSUPP;
		err = 0;
		break;
	}
	}

	if(err < 0)
	{
	    CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! control %d failed\n", 
	                                __FILE__, __LINE__, ctrl->id - V4L2_CID_PRIVATE_BASE);
	}

	mutex_unlock(&front_sensor_s_ctrl);
	
	return err;
}

static int sr130pc10_check_dataline_onoff(struct v4l2_subdev *sd, int onoff)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct sensor_state *state = to_state(sd);
	int err = 0;

    if(onoff)
    {
        // data line on
		CAM_INFO_MSG(&client->dev, "pattern on setting~~~~~~~~~~~~~~\n");	
		
    	err = sr130pc10_i2c_set_config_register(client, 
    	                                  front_pattern_on_regs, 
    	                                  PATTERN_ON_NUM_OF_REGS, 
    	                                  "front_pattern_on_regs");        
    	if(err < 0)
    	{
    		CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! Pattern on failed\n", __FILE__, __LINE__);	
    		return -EIO;
    	}
    }
    else
    {
        // data line off
		CAM_INFO_MSG(&client->dev, "pattern off setting~~~~~~~~~~~~~~\n");	
		
    	err = sr130pc10_i2c_set_config_register(client, 
    	                                  front_pattern_off_regs, 
    	                                  PATTERN_OFF_NUM_OF_REGS, 
    	                                  "front_pattern_off_regs");
        
    	if(err < 0)
    	{
    		CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! Pattern off failed\n", __FILE__, __LINE__);	
    		return -EIO;
    	}

	    state->check_dataline = 0;
    }

	msleep(100);
	
	CAM_INFO_MSG(&client->dev, "pattern on setting done~~~~~~~~~~~~~~\n");	

	return err;
}

static int sr130pc10_init(struct v4l2_subdev *sd, u32 val)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct sensor_state *state = to_state(sd);
	int err = -EINVAL;

	CAM_ERROR_MSG(&client->dev, "%s ~~~~~~~~~~~~~~\n", __func__);
	
	sr130pc10_init_parameters(sd);

#ifdef FRONT_CONFIG_LOAD_FILE
	err = sr130pc10_regs_table_init();
	if(err < 0) 
	{
		CAM_ERROR_MSG(&client->dev, "%s: sr130pc10_regs_table_init failed\n", __func__);
		return -ENOIOCTLCMD;
	}
#endif // FRONT_CONFIG_LOAD_FILE

#if 0
    if(state->vt_mode == VT_MODE_ON)
    {
    	err = sr130pc10_i2c_set_config_register(client,
    	                                  front_init_vt_regs,
    	                                  INIT_VT_NUM_OF_REGS,
    	                                  "front_init_vt_regs");
    }
    else
#endif        
    {
        if(state->check_dataline)
        {
//            err = camsensor_config_set_register(sd, SECTION_DTP);
        }            
        else
        {
        	err = sr130pc10_i2c_set_config_register(client,
        	                                  front_init_regs,
        	                                  INIT_NUM_OF_REGS,
        	                                  "front_init_regs");
        }        	                                  
    }
    
	if(err < 0)
	{
		CAM_ERROR_MSG(&client->dev, "[%s: %d] ERROR! Sensor Init Failed\n", __FILE__, __LINE__);
		return -ENOIOCTLCMD;
	}		

	return 0;
}

/**************************************************************************
 * sr130pc10_s_config
 * With camera device, we need to re-initialize every single opening time therefor,
 * it is not necessary to be initialized on probe time. except for version checking
 * NOTE: version checking is optional
 ***************************************************************************/
static int sr130pc10_s_config(struct v4l2_subdev *sd, int irq, void *platform_data)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct sensor_state *state = to_state(sd);
	struct sr130pc10_platform_data *pdata;

	pdata = client->dev.platform_data;

	if(!pdata) 
	{
		CAM_ERROR_MSG(&client->dev, "%s: no platform data\n", __func__);
		return -ENODEV;
	}

	/*
	 * Assign default format and resolution
	 * Use configured default information in platform data
	 * or without them, use default information in driver
	 */
	if(!(pdata->default_width && pdata->default_height)) 
	{
		/* TODO: assign driver default resolution */
	} 
	else 
	{
		state->pix.width = pdata->default_width;
		state->pix.height = pdata->default_height;
	}

	if(!pdata->pixelformat)
	{
		state->pix.pixelformat = DEFAULT_PIX_FMT;
	}
	else
	{
		state->pix.pixelformat = pdata->pixelformat;
	}

	if(!pdata->freq)
	{
		state->freq = DEFUALT_MCLK;
	}
	else
	{
		state->freq = pdata->freq;
	}

	return 0;
}

/**************************************************************************
 * DRIVER REGISTRATION FACTORS
 ***************************************************************************/
static const struct v4l2_subdev_core_ops sr130pc10_core_ops = 
{
    .init       = sr130pc10_init,	    /* initializing API */
    .s_config   = sr130pc10_s_config,	/* Fetch platform data */
    .queryctrl  = sr130pc10_queryctrl,
    .querymenu  = sr130pc10_querymenu,
    .g_ctrl     = sr130pc10_g_ctrl,
    .s_ctrl     = sr130pc10_s_ctrl,
};

static const struct v4l2_subdev_video_ops sr130pc10_video_ops = 
{
    .s_crystal_freq     = sr130pc10_s_crystal_freq,
    .g_fmt              = sr130pc10_g_fmt,
    .s_fmt              = sr130pc10_s_fmt,
    .enum_framesizes    = sr130pc10_enum_framesizes,
    .enum_frameintervals = sr130pc10_enum_frameintervals,
    .enum_fmt           = sr130pc10_enum_fmt,
    .try_fmt            = sr130pc10_try_fmt,
    .g_parm             = sr130pc10_g_parm,
    .s_parm             = sr130pc10_s_parm,
};

static const struct v4l2_subdev_ops sr130pc10_ops = 
{
	.core = &sr130pc10_core_ops,
	.video = &sr130pc10_video_ops,
};

/**************************************************************************
 * sr130pc10_probe
 * Fetching platform data is being done with s_config sd call.
 * In probe routine, we just register sd device
 ***************************************************************************/
static int sr130pc10_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct sensor_state *state;
	struct v4l2_subdev *sd;

	CAM_INFO_MSG(&client->dev, "sr130pc10_probe......................................... \n");
	
	state = kzalloc(sizeof(struct sensor_state), GFP_KERNEL);
	if (state == NULL)
	{
		return -ENOMEM;
	}

	sd = &state->sd;
	strcpy(sd->name, SR130PC10_DRIVER_NAME);

	/* Registering sd */
	v4l2_i2c_subdev_init(sd, client, &sr130pc10_ops);

	CAM_ERROR_MSG(&client->dev, "3MP camera SR130PC10 loaded.\n");
	
	return 0;
}

/**************************************************************************
 * sr130pc10_remove
 ***************************************************************************/
static int sr130pc10_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);

	CAM_INFO_MSG(&client->dev, "sr130pc10_remove.......................................... \n");

#ifdef FRONT_CONFIG_LOAD_FILE
	sr130pc10_regs_table_exit();
#endif // FRONT_CONFIG_LOAD_FILE

	v4l2_device_unregister_subdev(sd);
	kfree(to_state(sd));

	CAM_ERROR_MSG(&client->dev, "Unloaded camera sensor SR130PC10.\n");

	return 0;
}

static const struct i2c_device_id sr130pc10_id[] = 
{
	{ SR130PC10_DRIVER_NAME, 0 },
	{ }
};

static struct v4l2_i2c_driver_data v4l2_i2c_data = 
{
	.name = SR130PC10_DRIVER_NAME,
	.probe = sr130pc10_probe,
	.remove = sr130pc10_remove,
	.id_table = sr130pc10_id,
};

MODULE_DEVICE_TABLE(i2c, sr130pc10_id);
MODULE_DESCRIPTION("SYSTEM LSI SR130PC10 1.3MP camera driver");
MODULE_AUTHOR("Minkeun Cho <mk76.cho@samsung.com>");
MODULE_LICENSE("GPL");
