/*
 * Driver for isx005 (3MP Camera) from SONY
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
#include <media/isx005_platform.h>

#ifdef CONFIG_VIDEO_SAMSUNG_V4L2
#include <linux/videodev2_samsung.h>
#endif

#include <linux/rtc.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <mach/gpio-p1.h>
#include <mach/regs-clock.h>
#include <mach/max8998_function.h>
#include <plat/regs-fimc.h> 

#include "isx005.h"

#define ISX005_DRIVER_NAME	"ISX005"

#define FORMAT_FLAGS_COMPRESSED		0x3
#define SENSOR_JPEG_SNAPSHOT_MEMSIZE	0x33F000     //3403776 //2216 * 1536

//#define ISX005_DEBUG
//#define ISX005_INFO
//#define CONFIG_LOAD_FILE	//For tunning binary

#ifdef ISX005_DEBUG
#define isx005_msg	dev_err
#else
#define isx005_msg 	dev_dbg
#endif

#ifdef ISX005_INFO
#define isx005_info	dev_err
#else
#define isx005_info	dev_dbg
#endif

/* protect s_ctrl calls */
static DEFINE_MUTEX(sensor_s_ctrl);

/* stop_af_operation is used to cancel the operation while doing, or even before it has started */
static volatile int stop_af_operation;

/* Here we store the status of AF; 0 --> AF not started, 1 --> AF started , 2 --> AF operation finished */
static int af_operation_status;

/*
 * Whenever there is an AF cancell request the timer is started. The cancel operation
 * is valid for 100ms after that it is expired
 */
static struct timer_list af_cancel_timer;
static void af_cancel_handler(unsigned long data);

/* Save the focus mode value, it can be marco or auto */
static int af_mode;

#define CDBG(format, arg...) if (cdbg == 1) { printk("<ISX005> %s " format, __func__, ## arg); }

static int cdbg = 0;
static int gLowLight = 0;
static int gCurrentScene = SCENE_MODE_NONE;

/* Default resolution & pixelformat. plz ref isx005_platform.h */
#define DEFAULT_PIX_FMT		V4L2_PIX_FMT_UYVY	/* YUV422 */
#define DEFUALT_MCLK		24000000
#define POLL_TIME_MS		10

enum isx005_oprmode {
	ISX005_OPRMODE_VIDEO = 0,
	ISX005_OPRMODE_IMAGE = 1,
};

enum isx005_frame_size {
	ISX005_PREVIEW_QCIF = 0,	/* 176x144 */
	ISX005_PREVIEW_D1,		/* 720x480 */
	ISX005_PREVIEW_SVGA,		/* 800x600 */
	ISX005_PREVIEW_WSVGA,		/* 1024x600*/
	ISX005_CAPTURE_SVGA,		/* SVGA  - 800x600 */
	ISX005_CAPTURE_WSVGA,		/* SVGA  - 1024x600 */
	ISX005_CAPTURE_W1MP,		/* WUXGA  - 1600x960 */
	ISX005_CAPTURE_2MP,		/* UXGA  - 1600x1200 */
	ISX005_CAPTURE_W2MP,		/* WQXGA  - 2048x1232 */
	ISX005_CAPTURE_3MP,		/* QXGA  - 2048x1536 */
};

struct isx005_enum_framesize {
	/* mode is 0 for preview, 1 for capture */
	enum isx005_oprmode mode;
	unsigned int index;
	unsigned int width;
	unsigned int height;	
};

static struct isx005_enum_framesize isx005_framesize_list[] = {
	{ ISX005_OPRMODE_VIDEO, ISX005_PREVIEW_QCIF,	176,  144 },
	{ ISX005_OPRMODE_VIDEO, ISX005_PREVIEW_D1,	720,  480 },
	{ ISX005_OPRMODE_VIDEO, ISX005_PREVIEW_SVGA,	800,  600 },
	{ ISX005_OPRMODE_VIDEO, ISX005_PREVIEW_WSVGA,	1024,  600 },
	{ ISX005_OPRMODE_IMAGE, ISX005_CAPTURE_SVGA,	800,  600 },
	{ ISX005_OPRMODE_IMAGE, ISX005_CAPTURE_WSVGA,	1024,  600},	
	{ ISX005_OPRMODE_IMAGE, ISX005_CAPTURE_W1MP,	1600,  960 },
	{ ISX005_OPRMODE_IMAGE, ISX005_CAPTURE_2MP,	1600, 1200 },
	{ ISX005_OPRMODE_IMAGE, ISX005_CAPTURE_W2MP,	2048, 1232 },
	{ ISX005_OPRMODE_IMAGE, ISX005_CAPTURE_3MP,	2048, 1536 },
};

struct isx005_version {
	unsigned int major;
	unsigned int minor;
};

struct isx005_date_info {
	unsigned int year;
	unsigned int month;
	unsigned int date;
};

enum isx005_runmode {
	ISX005_RUNMODE_NOTREADY,
	ISX005_RUNMODE_IDLE, 
	ISX005_RUNMODE_RUNNING, 
};

struct isx005_firmware {
	unsigned int addr;
	unsigned int size;
};

/* Camera functional setting values configured by user concept */
struct isx005_userset {
	signed int exposure_bias;	/* V4L2_CID_EXPOSURE */
	unsigned int auto_wb;		/* V4L2_CID_AUTO_WHITE_BALANCE */
	unsigned int manual_wb;		/* V4L2_CID_WHITE_BALANCE_PRESET */
	unsigned int effect;		/* Color FX (AKA Color tone) */
	unsigned int contrast;		/* V4L2_CID_CONTRAST */
	unsigned int saturation;	/* V4L2_CID_SATURATION */
	unsigned int sharpness;		/* V4L2_CID_SHARPNESS */
};

struct isx005_jpeg_param {
	unsigned int enable;
	unsigned int quality;
	unsigned int main_size;  /* Main JPEG file size */
	unsigned int thumb_size; /* Thumbnail file size */
	unsigned int main_offset;
	unsigned int thumb_offset;
	unsigned int postview_offset;
} ; 

struct isx005_position {
	int x;
	int y;
} ; 

struct isx005_state {
	struct isx005_platform_data *pdata;
	struct v4l2_subdev sd;
	struct v4l2_pix_format pix;
	struct v4l2_fract timeperframe;
	struct isx005_userset userset;
	struct isx005_jpeg_param jpeg;
	struct isx005_version fw;
	struct isx005_version prm;
	struct isx005_date_info dateinfo;
	struct isx005_firmware fw_info;
	struct isx005_position position;
	struct v4l2_streamparm strm;
	enum isx005_runmode runmode;
	enum isx005_oprmode oprmode;
	int framesize_index;
	int sensor_version;
	int freq;	/* MCLK in Hz */
	int fps;
	int preview_size;
};

const static struct v4l2_fmtdesc capture_fmts[] = {
        {
                .index          = 0,
                .type           = V4L2_BUF_TYPE_VIDEO_CAPTURE,
                .flags          = FORMAT_FLAGS_COMPRESSED,
                .description    = "JPEG + Postview",
                .pixelformat    = V4L2_PIX_FMT_JPEG,
        },
};

extern int isx005_cam_stdby(bool en);

#ifdef CONFIG_LOAD_FILE
	static int isx005_regs_table_write(struct i2c_client *client, char *name);
#endif

static inline struct isx005_state *to_state(struct v4l2_subdev *sd)
{
	return container_of(sd, struct isx005_state, sd);
}

/**
 * isx005_i2c_read: Read 2 bytes from sensor 
 */
static inline int isx005_i2c_read(struct i2c_client *client, 
	unsigned short subaddr, unsigned short *data)
{
	unsigned char buf[2];
	int err = 0;
	struct i2c_msg msg = {client->addr, 0, 2, buf};

	buf[0] = subaddr>> 8;
	buf[1] = subaddr & 0xff;
	
	err = i2c_transfer(client->adapter, &msg, 1);
	if (unlikely(err < 0))
		isx005_msg(&client->dev, "%s: register read fail\n", __func__);	

	msg.flags = I2C_M_RD;

	err = i2c_transfer(client->adapter, &msg, 1);
	if (unlikely(err < 0))
		isx005_msg(&client->dev, "%s: register read fail\n", __func__);	

	/*
	 * [Arun c]Data comes in Little Endian in parallel mode; So there
	 * is no need for byte swapping here
	 */
	*data = *(unsigned short *)(&buf);
		
	return err;
}

/** 
 * isx005_i2c_read_multi: Read (I2C) multiple bytes to the camera sensor 
 * @client: pointer to i2c_client
 * @cmd: command register
 * @w_data: data to be written
 * @w_len: length of data to be written
 * @r_data: buffer where data is read
 * @r_len: number of bytes to read
 *
 * Returns 0 on success, <0 on error
 */
static inline int isx005_i2c_read_multi(struct i2c_client *client,  
	unsigned short subaddr, unsigned long *data)
{
	unsigned char buf[4];
	int err = 0;
	struct i2c_msg msg = {client->addr, 0, 2, buf};

	buf[0] = subaddr>> 8;
	buf[1] = subaddr & 0xff;

	err = i2c_transfer(client->adapter, &msg, 1);
	if (unlikely(err < 0))
		isx005_msg(&client->dev, "%s: register read fail\n", __func__);	

	msg.flags = I2C_M_RD;
	msg.len = 4;

	err = i2c_transfer(client->adapter, &msg, 1);
	if (unlikely(err < 0))
		isx005_msg(&client->dev, "%s: register read fail\n", __func__);	

	/*
	 * [Arun c]Data comes in Little Endian in parallel mode; So there
	 * is no need for byte swapping here
	 */
	*data = *(unsigned long *)(&buf);

	return err;
}

/** 
 * isx005_i2c_write_multi: Write (I2C) multiple bytes to the camera sensor 
 * @client: pointer to i2c_client
 * @cmd: command register
 * @w_data: data to be written
 * @w_len: length of data to be written
 *
 * Returns 0 on success, <0 on error
 */
static inline int isx005_i2c_write_multi(struct i2c_client *client, unsigned short addr, unsigned int w_data, unsigned int w_len)
{
	int retry_count = 5;
	unsigned char buf[w_len+2];
	struct i2c_msg msg = {client->addr, 0, w_len+2, buf};
	int ret;

	buf[0] = addr >> 8;
	buf[1] = addr & 0xff;	

	/* 
	 * [Arun c]Data should be written in Little Endian in parallel mode; So there
	 * is no need for byte swapping here
	 */
	if(w_len == 1)
		buf[2] = (unsigned char)w_data;
	else if(w_len == 2)
		*((unsigned short *)&buf[2]) = (unsigned short)w_data;
	else
		*((unsigned int *)&buf[2]) = w_data;

#ifdef ISX005_DEBUG
	{
		int j;
		printk("W: ");
		for(j = 0; j <= w_len+1; j++){
			printk("0x%02x ", buf[j]);
		}
		printk("\n");
	}
#endif

	while(retry_count--){
		ret  = i2c_transfer(client->adapter, &msg, 1);
		if (likely(ret == 1))
			break;
		msleep(POLL_TIME_MS);
		}

	return (ret == 1) ? 0 : -EIO;
}

static int isx005_write_regs(struct v4l2_subdev *sd, isx005_short_t regs[], 
				int size, char *name)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int i, err;

#ifdef CONFIG_LOAD_FILE
	isx005_regs_table_write(client, name);
#else
	for (i = 0; i < size; i++) {
		err = isx005_i2c_write_multi(client, regs[i].subaddr, regs[i].value, regs[i].len);
		if (unlikely(err < 0)) {
			v4l_info(client, "%s: register set failed\n",  __func__);
			return err;
		}
	}
#endif
	return 0;
}

#ifdef CONFIG_LOAD_FILE

#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

static char *isx005_regs_table = NULL;

static int isx005_regs_table_size;

int isx005_regs_table_init(void)
{
	printk("[BestIQ] + isx005_regs_table_init\n");
	struct file *filp;
	char *dp;
	long l;
	loff_t pos;
	int i;
	int ret;
	mm_segment_t fs = get_fs();

	printk("%s %d\n", __func__, __LINE__);

	set_fs(get_ds());
#if 0
	filp = filp_open("/data/camera/isx005.h", O_RDONLY, 0);
#else
	filp = filp_open("/mnt/internal_sd/external_sd/isx005.h", O_RDONLY, 0);
#endif

	if (IS_ERR(filp)) {
		printk("file open error\n");
		return PTR_ERR(filp);
	}
	
	l = filp->f_path.dentry->d_inode->i_size;	
	printk("l = %ld\n", l);
	dp = kmalloc(l, GFP_KERNEL);
//	dp = vmalloc(l);	
	if (dp == NULL) {
		printk("Out of Memory\n");
		filp_close(filp, current->files);
	}
	
	pos = 0;
	memset(dp, 0, l);
	ret = vfs_read(filp, (char __user *)dp, l, &pos);
	
	if (ret != l) {
		printk("Failed to read file ret = %d\n", ret);
//		kfree(dp);
		vfree(dp);
		filp_close(filp, current->files);
		return -EINVAL;
	}

	filp_close(filp, current->files);
		
	set_fs(fs);
	
	isx005_regs_table = dp;
		
	isx005_regs_table_size = l;
	
	*((isx005_regs_table + isx005_regs_table_size) - 1) = '\0';
	
	printk("isx005_regs_table 0x%08x, %ld\n", dp, l);
	printk("[BestIQ] - isx005_reg_table_init\n");

	return 0;
}

void isx005_regs_table_exit(void)
{
	printk("[BestIQ] + isx005_regs_table_exit\n");
	printk("%s %d\n", __func__, __LINE__);
	if (isx005_regs_table) {
		kfree(isx005_regs_table);
		isx005_regs_table = NULL;
	}
	printk("[BestIQ] - isx005_regs_table_exit\n");
}

static int isx005_regs_table_write(struct i2c_client *client, char *name)
{
	printk("[BestIQ] + isx005_regs_table_write\n");
	char *start, *end, *reg, *data;	
	unsigned short addr;
	unsigned int len, value;	
	char reg_buf[7], data_buf[7], len_buf[2];
	
	*(reg_buf + 6) = '\0';
	*(data_buf + 6) = '\0';
	*(len_buf + 1) = '\0';	

//	printk("[BestIQ] + isx005_regs_table_write ------- start\n");
	start = strstr(isx005_regs_table, name);
	end = strstr(start, "};");
	
	while (1) {	
		/* Find Address */	
		reg = strstr(start,"{0x");		
		if (reg)
			start = (reg + 19);  //{0x000b, 0x0004, 1},	
		if ((reg == NULL) || (reg > end))
			break;
		/* Write Value to Address */	
		if (reg != NULL) {
			memcpy(reg_buf, (reg + 1), 6);	
			memcpy(data_buf, (reg + 9), 6);	
			memcpy(len_buf, (reg + 17), 1);			
			addr = (unsigned short)simple_strtoul(reg_buf, NULL, 16); 
			value = (unsigned int)simple_strtoul(data_buf, NULL, 16); 
			len = (unsigned int)simple_strtoul(len_buf, NULL, 10); 			
//			printk("addr 0x%04x, value 0x%04x, len %d\n", addr, value, len);
			
			if (addr == 0xdddd)
			{
/*				if (value == 0x0010)
				mdelay(10);
				else if (value == 0x0020)
				mdelay(20);
				else if (value == 0x0030)
				mdelay(30);
				else if (value == 0x0040)
				mdelay(40);
				else if (value == 0x0050)
				mdelay(50);
				else if (value == 0x0100)
				mdelay(100);*/
				mdelay(value);
				printk("delay 0x%04x, value 0x%04x, , len 0x%01x\n", addr, value, len);
			}	
			else
				isx005_i2c_write_multi(client, addr, value, len);
		}
	}
	printk("[BestIQ] - isx005_regs_table_write\n");
	return 0;
}

static short isx005_regs_max_value(char *name)
{
	printk("[BestIQ] + isx005_regs_max_value\n");
	char *start, *reg, *data;	
	unsigned short value;
	char data_buf[7];
	
	*(data_buf + 6) = '\0';

	start = strstr(isx005_regs_table, name);
	
		/* Find Address */	
		reg = strstr(start," 0x");		
		if (reg == NULL)
			return 0;
		/* Write Value to Address */	
		if (reg != NULL) {
			memcpy(data_buf, (reg + 1), 6);	
			value = (unsigned short)simple_strtoul(data_buf, NULL, 16); 
		}
	printk("[BestIQ] - isx005_regs_max_value\n");
	return value;
}

#endif

static int isx005_set_preview_stop(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct isx005_state *state = to_state(sd);

	if(ISX005_RUNMODE_RUNNING == state->runmode){
		state->runmode = ISX005_RUNMODE_IDLE;
	}

	dev_err(&client->dev, "%s: change preview mode~~~~~~~~~~~~~~\n", __func__);		
//bestiq	isx005_i2c_write_multi(client, 0x0011, 0x0011, 1);
	return 0;
}

static int isx005_set_dzoom(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	int err;
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	isx005_msg(&client->dev, "[BestIQ] - isx005_set_dzoom~~~~~~ %d\n", ctrl->value);

	switch (ctrl->value) {
		case 0:
	case 20:
	case 30:
	default:
		err = isx005_write_regs(sd, isx005_Zoom_00, sizeof(isx005_Zoom_00) / sizeof(isx005_Zoom_00[0]),
				"isx005_Zoom_00");
		break;
		case 1:
		err = isx005_write_regs(sd, isx005_Zoom_01, sizeof(isx005_Zoom_01) / sizeof(isx005_Zoom_01[0]),
				"isx005_Zoom_01");
		break;
		case 2:
		err = isx005_write_regs(sd, isx005_Zoom_02, sizeof(isx005_Zoom_02) / sizeof(isx005_Zoom_02[0]),
				"isx005_Zoom_02");
		break;
		case 3:
		err = isx005_write_regs(sd, isx005_Zoom_03, sizeof(isx005_Zoom_03) / sizeof(isx005_Zoom_03[0]),
				"isx005_Zoom_03");
		break;
		case 4:
		err = isx005_write_regs(sd, isx005_Zoom_04, sizeof(isx005_Zoom_04) / sizeof(isx005_Zoom_04[0]),
				"isx005_Zoom_04");
		break;
		case 5:
		err = isx005_write_regs(sd, isx005_Zoom_05, sizeof(isx005_Zoom_05) / sizeof(isx005_Zoom_05[0]),
				"isx005_Zoom_05");
		break;
		case 6:
		err = isx005_write_regs(sd, isx005_Zoom_06, sizeof(isx005_Zoom_06) / sizeof(isx005_Zoom_06[0]),
				"isx005_Zoom_06");
		break;
		case 7:
		err = isx005_write_regs(sd, isx005_Zoom_07, sizeof(isx005_Zoom_07) / sizeof(isx005_Zoom_07[0]),
				"isx005_Zoom_07");
		break;
	}

		if(err < 0){
		dev_err(&client->dev, "%s: i2c_write failed\n", __func__);
		return -EIO;
	}
	return 0;
}

static int isx005_set_preview_size(struct v4l2_subdev *sd)
{
	int err = 0;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct isx005_state *state = to_state(sd);
	int index = state->framesize_index;

	dev_err(&client->dev, "[zzangdol] %s: index = %d\n", __func__, index);

	switch(index){
	case ISX005_PREVIEW_QCIF:
		err = isx005_i2c_write_multi(client, 0x0022, 0x00B0, 2);// HSIZE_MONI - 176
		err = isx005_i2c_write_multi(client, 0x0028, 0x0090, 2);// VSIZE_MONI - 144		
		break;
	case ISX005_PREVIEW_D1:
		err = isx005_i2c_write_multi(client, 0x0022, 0x02D0, 2);// HSIZE_MONI - 720
		err = isx005_i2c_write_multi(client, 0x0028, 0x01E0, 2);// VSIZE_MONI - 480	
		break;
	case ISX005_PREVIEW_SVGA:
		err = isx005_i2c_write_multi(client, 0x0022, 0x0320, 2);// HSIZE_MONI - 800
		err = isx005_i2c_write_multi(client, 0x0028, 0x0258, 2);// VSIZE_MONI - 600
		break;
	case ISX005_PREVIEW_WSVGA:
		err = isx005_i2c_write_multi(client, 0x0022, 0x0400, 2);// HSIZE_MONI - 1024
		err = isx005_i2c_write_multi(client, 0x0028, 0x0258, 2);// VSIZE_MONI - 600
		break;
	default:
		/* When running in image capture mode, the call comes here.
 		 * Set the default video resolution - CE147_PREVIEW_VGA
 		 */ 
		isx005_msg(&client->dev, "Setting preview resoution as VGA for image capture mode\n");
		break;
	}

	state->preview_size = index; 

	err = isx005_i2c_write_multi(client, 0x0012, 0x0001, 1); //Moni_Refresh
	err = isx005_i2c_write_multi(client, 0x00FC, 0x0002, 1); //Clear Interrupt Status (CM_Change)
	msleep(5);
	
	isx005_msg(&client->dev, "Using HD preview\n");

	return err;	
}

static int isx005_set_preview_start(struct v4l2_subdev *sd)
{
	int err, timeout_cnt;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct isx005_state *state = to_state(sd);
	unsigned short read_value;

	/* Reset the AF check variables for the next sequence */
	stop_af_operation = 0;
	af_operation_status = 0;

	if (!state->pix.width || !state->pix.height || !state->fps)
		return -EINVAL;


	err = isx005_write_regs(sd, isx005_cap_to_prev, sizeof(isx005_cap_to_prev) / sizeof(isx005_cap_to_prev[0]),
				"isx005_cap_to_prev");
				
	isx005_i2c_read(client, 0x0004, &read_value);
	if((read_value & 0x03) != 0) {
		isx005_i2c_write_multi(client, 0x0011, 0x0000, 1);	//MODE_SEL  0x00: Monitor mode

		/* Wait for Mode Transition (CM) */
		timeout_cnt = 0;
		do {
			timeout_cnt++;
			isx005_i2c_read(client, 0x00F8, &read_value);
			msleep(1);
			if (timeout_cnt > 1000) {
				dev_err(&client->dev, "%s: Entering moniter mode timed out \n", __func__);	
				break;
			}
		}while(!(read_value&0x02));

		timeout_cnt = 0;
		do {
			timeout_cnt++;
			isx005_i2c_write_multi(client, 0x00FC, 0x0002, 1);		
			msleep(1);			
			isx005_i2c_read(client, 0x00F8, &read_value);
			if (timeout_cnt > 1000) {
				dev_err(&client->dev, "%s: Entering moniter mode timed out \n", __func__);	
				break;
			}
		}while(read_value&0x02);	

	}
		
	err = isx005_set_preview_size(sd);
	if(err < 0){
		dev_err(&client->dev, "%s: failed: Could not set preview size\n", __func__);
	       return -EIO;
	}

	state->runmode = ISX005_RUNMODE_RUNNING;

	msleep(200);
	
	dev_err(&client->dev, "%s: init setting~~~~~~~~~~~~~~\n", __func__);		
	return 0;
}

static int isx005_set_capture_size(struct v4l2_subdev *sd)
{
	int err = 0;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct isx005_state *state = to_state(sd);

	int index = state->framesize_index;
	printk("isx005_set_capture_size ---------index : %d\n", index);	

	switch(index){
	case ISX005_CAPTURE_SVGA: /* 800x600 */
		err = isx005_i2c_write_multi(client, 0x0386, 0x0140, 2);// HSIZE_TN - 320
		err = isx005_i2c_write_multi(client, 0x0024, 0x0320, 2); //HSIZE_CAP 800
		err = isx005_i2c_write_multi(client, 0x002A, 0x0258, 2); //VSIZE_CAP 600
		break;
	case ISX005_CAPTURE_WSVGA: /* 1024x600 */
		err = isx005_i2c_write_multi(client, 0x0386, 0x0190, 2);// HSIZE_TN - 400
		err = isx005_i2c_write_multi(client, 0x0024, 0x0400, 2); //HSIZE_CAP 1024
		err = isx005_i2c_write_multi(client, 0x002A, 0x0258, 2); //VSIZE_CAP 600
		break;		
	case ISX005_CAPTURE_W1MP: /* 1600x960 */
		err = isx005_i2c_write_multi(client, 0x0386, 0x0190, 2);// HSIZE_TN - 400
		err = isx005_i2c_write_multi(client, 0x0024, 0x0640, 2); //HSIZE_CAP 1600
		err = isx005_i2c_write_multi(client, 0x002A, 0x03C0, 2); //VSIZE_CAP 960
		break;
	case ISX005_CAPTURE_2MP: /* 1600x1200 */
		err = isx005_i2c_write_multi(client, 0x0386, 0x0140, 2);// HSIZE_TN - 320
		err = isx005_i2c_write_multi(client, 0x0024, 0x0640, 2); //HSIZE_CAP 1600
		err = isx005_i2c_write_multi(client, 0x002A, 0x04B0, 2); //VSIZE_CAP 1200
		break;
	case ISX005_CAPTURE_W2MP: /* 2048x1232 */
		err = isx005_i2c_write_multi(client, 0x0386, 0x0190, 2);// HSIZE_TN - 400
		err = isx005_i2c_write_multi(client, 0x0024, 0x0800, 2); //HSIZE_CAP 2048
		err = isx005_i2c_write_multi(client, 0x002A, 0x04D0, 2); //VSIZE_CAP 1232
		break;
	case ISX005_CAPTURE_3MP: /* 2048x1536 */
		err = isx005_i2c_write_multi(client, 0x0386, 0x0140, 2);// HSIZE_TN - 320
		err = isx005_i2c_write_multi(client, 0x0024, 0x0800, 2); //HSIZE_CAP 2048
		err = isx005_i2c_write_multi(client, 0x002A, 0x0600, 2); //VSIZE_CAP 1536
		break;
	default:
		/* The framesize index was not set properly. 
 		 * Check s_fmt call - it must be for video mode. */
		return -EINVAL;
	}

	/* Set capture image size */
	if(err < 0){
		dev_err(&client->dev, "%s: failed: i2c_write for capture_resolution\n", __func__);
		return -EIO; 
	}

	printk("isx005_set_capture_size: %d\n", index);
	return 0;	
}

static int isx005_set_jpeg_quality(struct v4l2_subdev *sd)
{
	struct isx005_state *state = to_state(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	int err;

	if(state->jpeg.quality < 0)
		state->jpeg.quality = 0;
	if(state->jpeg.quality > 100)
		state->jpeg.quality = 100;

	switch(state->jpeg.quality)
	{
		case 100: //Super fine
			err = isx005_i2c_write_multi(client, 0x0207, 0x005A, 1);
//			err = isx005_i2c_write(client, 0x0207, 0x0264);			
		break;

		case 70: // Fine
			err = isx005_i2c_write_multi(client, 0x0207, 0x0050, 1);
		break;

		case 40: // Normal
			err = isx005_i2c_write_multi(client, 0x0207, 0x0046, 1);
		break;

		default:
			err = isx005_i2c_write_multi(client, 0x0207, 0x005A, 1);

		break;
	}

//	isx005_msg(&client->dev, "Quality = %d \n", state->jpeg.quality);
	printk("Quality = %d \n", state->jpeg.quality);	

	if(err < 0){
		dev_err(&client->dev, "%s: failed: i2c_write for jpeg_comp_level\n", __func__);
		return -EIO;
	}

	isx005_msg(&client->dev, "%s: done\n", __func__);

	return 0;
}

static int isx005_get_snapshot_data(struct v4l2_subdev *sd)
{
	int err;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct isx005_state *state = to_state(sd);

	unsigned long jpeg_framesize;

	if(state->jpeg.enable){
		/* Get main JPEG size */
		err = isx005_i2c_read_multi(client, 0x1624, &jpeg_framesize);
		printk("%s: JPEG main filesize = 0x%lx bytes\n", __func__, jpeg_framesize );				
		if(err < 0){
			dev_err(&client->dev, "%s: failed: i2c_read for jpeg_framesize\n", __func__);
			return -EIO;
		}			
		state->jpeg.main_size = jpeg_framesize;

		printk("%s: JPEG main filesize = %d bytes\n", __func__, state->jpeg.main_size );

		state->jpeg.main_offset = 0;
		state->jpeg.thumb_offset = 0x271000;
		state->jpeg.postview_offset = 0x280A00;		
	}

	isx005_msg(&client->dev, "%s: done\n", __func__);

	return 0;
}

static int isx005_get_LowLightCondition(struct v4l2_subdev *sd, int *Result)
{
	int err = 0;
	unsigned short read_value = 0;
	struct i2c_client *client = v4l2_get_subdevdata(sd);	

	printk("isx005_get_LowLightCondition : start \n");

	err = isx005_i2c_read(client, 0x027A, &read_value); // AGC_SCL_NOW
	if(err < 0){
		dev_err(&client->dev, "%s: failed: i2c_read for low_light Condition\n", __func__);
		return -EIO; 
	}

	printk("isx005_get_LowLightCondition : Read(0x%X) \n", read_value);
#ifdef CONFIG_LOAD_FILE
	unsigned short max_value = 0;
	max_value = isx005_regs_max_value("MAX_VALUE");
	printk("%s   max_value = %x \n", __func__, max_value);
	if(read_value >= max_value)//if(read_value >= 0x0B33)
	{
		*Result = 1; //gLowLight
	}
#else
	if(read_value >= 0x0A20)//if(read_value >= 0x0B33)
	{
		*Result = 1; //gLowLight
	}
#endif
	printk("isx005_get_LowLightCondition : end \n");

	return err;
}

static int isx005_LowLightCondition_Off(struct v4l2_subdev *sd)
{
	int err = 0;
	struct i2c_client *client = v4l2_get_subdevdata(sd);	

	printk("isx005_LowLightCondition_Off : start \n");


//write : Outdoor_off
	err = isx005_write_regs(sd, \
		isx005_Outdoor_Off, \
		sizeof(isx005_Outdoor_Off) / sizeof(isx005_Outdoor_Off[0]), \
		"isx005_Outdoor_Off");	
	if(err < 0){
		dev_err(&client->dev, "%s: failed: i2c_write for low_light Condition\n", __func__);
		return -EIO; 
	}

	if(gLowLight == 1)
	{
		gLowLight = 0;

		if(gCurrentScene == SCENE_MODE_NIGHTSHOT)
		{
			printk("SCENE_MODE_NIGHTSHOT --- isx005_Night_Mode_Off: start \n");		
			err = isx005_write_regs(sd, \
				isx005_Night_Mode_Off, \
				sizeof(isx005_Night_Mode_Off) / sizeof(isx005_Night_Mode_Off[0]), \
				"isx005_Night_Mode_Off");	
		}
		else
		{
			printk("Not Night mode --- isx005_Low_Cap_Off: start \n");			
			err = isx005_write_regs(sd, \
				isx005_Low_Cap_Off, \
				sizeof(isx005_Low_Cap_Off) / sizeof(isx005_Low_Cap_Off[0]), \
				"isx005_Low_Cap_Off");			
		}
	}
	printk("isx005_LowLightCondition_Off : end \n");

	return err;
}

static int isx005_set_capture_start(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	int err, timeout_cnt;
	unsigned short read_value;
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	isx005_i2c_write_multi(client, 0x0014, 0x0002, 1);/*CAPNUM is setted 2. default value is 2. */
	/*
	 *  1. capture number
	 */
	isx005_i2c_write_multi(client, 0x00FC, 0x0002, 1);	//interupt clear

	err = isx005_write_regs(sd, isx005_prev_to_cap, sizeof(isx005_prev_to_cap) / sizeof(isx005_prev_to_cap[0]),
				"isx005_prev_to_cap");
	/* Outdoor setting */
	isx005_i2c_read(client, 0x6C21, &read_value);
	dev_err(&client->dev, "%s: i2c_read ---  OUTDOOR_F == 0x%x \n", __func__, read_value);	
	if(read_value == 0x01)
	{
		isx005_i2c_write_multi(client, 0x0014, 0x0003, 1);/*CAPNUM is setted 3. default value is 2. */
		err = isx005_write_regs(sd, \
			isx005_Outdoor_On, \
			sizeof(isx005_Outdoor_On) / sizeof(isx005_Outdoor_On[0]), \
			"isx005_Outdoor_On");		
	}
	
	err = isx005_get_LowLightCondition(sd, &gLowLight);

	if(gLowLight)
	{
		isx005_i2c_write_multi(client, 0x0014, 0x0003, 1);/*CAPNUM is setted 3. default value is 2. */
		if(gCurrentScene == SCENE_MODE_NIGHTSHOT)
		{
			err = isx005_write_regs(sd, \
				isx005_Night_Mode_On, \
				sizeof(isx005_Night_Mode_On) / sizeof(isx005_Night_Mode_On[0]), \
				"isx005_Night_Mode_On");	
		}
		else
		{
			err = isx005_write_regs(sd, \
				isx005_Low_Cap_On, \
				sizeof(isx005_Low_Cap_On) / sizeof(isx005_Low_Cap_On[0]), \
				"isx005_Low_Cap_On");
		}
	}

	/*
	 *  2. Set image size
	 */
	err = isx005_set_capture_size(sd);
	if(err < 0){
		dev_err(&client->dev, "%s: failed: i2c_write for capture_resolution\n", __func__);
		return -EIO; 
	}

	isx005_i2c_write_multi(client, 0x0011, 0x0002, 1); //capture_command

	msleep(30);
	/* Wait for Mode Transition (CM) */
	timeout_cnt = 0;
	do {
		timeout_cnt++;
		isx005_i2c_read(client, 0x00F8, &read_value);
		msleep(1);
		if (timeout_cnt > 1000) {
			dev_err(&client->dev, "%s: Entering capture mode timed out\n", __func__);	
			break;
		}
	}while(!(read_value&0x02));

	timeout_cnt = 0;
	do {
		timeout_cnt++;
		isx005_i2c_write_multi(client, 0x00FC, 0x0002, 1);		
		msleep(1);			
		isx005_i2c_read(client, 0x00F8, &read_value);
		if (timeout_cnt > 1000) {
			dev_err(&client->dev, "%s: Entering capture mode timed out\n", __func__);	
			break;
		}
	}while(read_value&0x02);	

//capture frame out....
	dev_err(&client->dev, "%s: Capture frame out~~~~ \n", __func__);	
	msleep(50);

	timeout_cnt = 0;
	do {
		timeout_cnt++;
		isx005_i2c_read(client, 0x00F8, &read_value);
		msleep(1);		
		if (timeout_cnt > 1000) {
			dev_err(&client->dev, "%s: JPEG capture timed out\n", __func__);	
			break;
		}
	}while(!(read_value&0x08));	

	timeout_cnt = 0;
	do {
		timeout_cnt++;
		isx005_i2c_write_multi(client, 0x00FC, 0x0008, 1);	
		msleep(1);
		isx005_i2c_read(client, 0x00F8, &read_value);
		if (timeout_cnt > 1000) {
			dev_err(&client->dev, "%s: JPEG capture timed out\n", __func__);	
			break;
		}
	}while(read_value&0x08);	

	isx005_i2c_read(client, 0x0200, &read_value);
	dev_err(&client->dev, "%s: JPEG STS --- read_value == 0x%x \n", __func__, read_value);			


	/*
	 * 8. Get JPEG Main Data
	 */ 
	err = isx005_get_snapshot_data(sd);
	if(err < 0){
		dev_err(&client->dev, "%s: failed: get_snapshot_data\n", __func__);
		return err;
	}

	err = isx005_LowLightCondition_Off(sd);
	if(err < 0){
		dev_err(&client->dev, "%s: failed: isx005_LowLightCondition_Off\n", __func__);
		return err;
	}
	return 0;
}

static int isx005_change_scene_mode(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	int err;
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	isx005_msg(&client->dev, "%s: start   CurrentScene : %d, Scene : %d\n", __func__, gCurrentScene, ctrl->value);
	
	switch(ctrl->value)
	{
		case SCENE_MODE_NONE:
			err = isx005_write_regs(sd, \
				isx005_Scene_Default, \
				sizeof(isx005_Scene_Default) / sizeof(isx005_Scene_Default[0]), \
				"isx005_Scene_Default");			
		break;

		case SCENE_MODE_PORTRAIT:
			err = isx005_write_regs(sd, \
				isx005_Scene_Portrait, \
				sizeof(isx005_Scene_Portrait) / sizeof(isx005_Scene_Portrait[0]), \
				"isx005_Scene_Portrait");			
		break;

		case SCENE_MODE_NIGHTSHOT:
			err = isx005_write_regs(sd, \
				isx005_Scene_Nightshot, \
				sizeof(isx005_Scene_Nightshot) / sizeof(isx005_Scene_Nightshot[0]), \
				"isx005_Scene_Nightshot");			
		break;

		case SCENE_MODE_BACK_LIGHT:
			err = isx005_write_regs(sd, \
				isx005_Scene_Backlight, \
				sizeof(isx005_Scene_Backlight) / sizeof(isx005_Scene_Backlight[0]), \
				"isx005_Scene_Backlight");			
		break;

		case SCENE_MODE_LANDSCAPE:
			err = isx005_write_regs(sd, \
				isx005_Scene_Landscape, \
				sizeof(isx005_Scene_Landscape) / sizeof(isx005_Scene_Landscape[0]), \
				"isx005_Scene_Landscape");			
		break;

		case SCENE_MODE_SPORTS:
			err = isx005_write_regs(sd, \
				isx005_Scene_Sports, \
				sizeof(isx005_Scene_Sports) / sizeof(isx005_Scene_Sports[0]), \
				"isx005_Scene_Sports");			
		break;

		case SCENE_MODE_PARTY_INDOOR:
			err = isx005_write_regs(sd, \
				isx005_Scene_Party_Indoor, \
				sizeof(isx005_Scene_Party_Indoor) / sizeof(isx005_Scene_Party_Indoor[0]), \
				"isx005_Scene_Party_Indoor");			
		break;

		case SCENE_MODE_BEACH_SNOW:
			err = isx005_write_regs(sd, \
				isx005_Scene_Beach_Snow, \
				sizeof(isx005_Scene_Beach_Snow) / sizeof(isx005_Scene_Beach_Snow[0]), \
				"isx005_Scene_Beach_Snow");			
		break;

		case SCENE_MODE_SUNSET:
			err = isx005_write_regs(sd, \
				isx005_Scene_Sunset, \
				sizeof(isx005_Scene_Sunset) / sizeof(isx005_Scene_Sunset[0]), \
				"isx005_Scene_Sunset");			
		break;

		case SCENE_MODE_DUST_DAWN:
			err = isx005_write_regs(sd, \
				isx005_Scene_Duskdawn, \
				sizeof(isx005_Scene_Duskdawn) / sizeof(isx005_Scene_Duskdawn[0]), \
				"isx005_Scene_Duskdawn");			
		break;

		case SCENE_MODE_FALL_COLOR:
			err = isx005_write_regs(sd, \
				isx005_Scene_Fall_Color, \
				sizeof(isx005_Scene_Fall_Color) / sizeof(isx005_Scene_Fall_Color[0]), \
				"isx005_Scene_Fall_Color");			
		break;

		case SCENE_MODE_FIREWORKS:
			err = isx005_write_regs(sd, \
				isx005_Scene_Fireworks, \
				sizeof(isx005_Scene_Fireworks) / sizeof(isx005_Scene_Fireworks[0]), \
				"isx005_Scene_Fireworks");			
		break;		

		case SCENE_MODE_TEXT:
			err = isx005_write_regs(sd, \
				isx005_Scene_Text, \
				sizeof(isx005_Scene_Text) / sizeof(isx005_Scene_Text[0]), \
				"isx005_Scene_Text");			
		break;	

		case SCENE_MODE_CANDLE_LIGHT:
			err = isx005_write_regs(sd, \
				isx005_Scene_Candle_Light, \
				sizeof(isx005_Scene_Candle_Light) / sizeof(isx005_Scene_Candle_Light[0]), \
				"isx005_Scene_Candle_Light");			
		break;			
		default:
		dev_err(&client->dev, "%s: unsupported scene mode\n", __func__);
		break;
	}

		if(err < 0){
		dev_err(&client->dev, "%s: i2c_write failed\n", __func__);
		return -EIO;
	}
	
	gCurrentScene = ctrl->value;	
	isx005_msg(&client->dev, "%s: done   CurrentScene : %d, Scene : %d\n", __func__, gCurrentScene, ctrl->value);
	return 0;
}

static int isx005_set_effect(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	int err;
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	isx005_msg(&client->dev, "%s: setting value =%d\n", __func__, ctrl->value);

	switch(ctrl->value) {
		case IMAGE_EFFECT_NONE:
	case IMAGE_EFFECT_AQUA:
	case IMAGE_EFFECT_ANTIQUE:
	case IMAGE_EFFECT_SHARPEN:
	default:
		err = isx005_write_regs(sd, isx005_Effect_Normal, sizeof(isx005_Effect_Normal) / sizeof(isx005_Effect_Normal[0]),
			"isx005_Effect_Normal");
		break;
		case IMAGE_EFFECT_BNW:
		err = isx005_write_regs(sd, isx005_Effect_Black_White, sizeof(isx005_Effect_Black_White) / sizeof(isx005_Effect_Black_White[0]),
			"isx005_Effect_Black_White");
		break;
		case IMAGE_EFFECT_SEPIA:
		err = isx005_write_regs(sd, isx005_Effect_Sepia, sizeof(isx005_Effect_Sepia) / sizeof(isx005_Effect_Sepia[0]),
			"isx005_Effect_Sepia");
		break;
		case IMAGE_EFFECT_NEGATIVE:
		err = isx005_write_regs(sd, isx005_Effect_Negative, sizeof(isx005_Effect_Negative) / sizeof(isx005_Effect_Negative[0]),
			"isx005_Effect_Negative");
		break;
	}

		if(err < 0){
		dev_err(&client->dev, "%s: i2c_write failed\n", __func__);
		return -EIO;
	}
	return 0;
}

static int isx005_set_saturation(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	int err;
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	switch(ctrl->value)
	{
		case SATURATION_MINUS_2:
			err = isx005_write_regs(sd, \
				isx005_Saturation_Minus_2, \
				sizeof(isx005_Saturation_Minus_2) / sizeof(isx005_Saturation_Minus_2[0]), \
				"isx005_Saturation_Minus_2");	
		break;

		case SATURATION_MINUS_1:
			err = isx005_write_regs(sd, \
				isx005_Saturation_Minus_1, \
				sizeof(isx005_Saturation_Minus_1) / sizeof(isx005_Saturation_Minus_1[0]), \
				"isx005_Saturation_Minus_1");	
		break;

		case SATURATION_DEFAULT:
		default:
			err = isx005_write_regs(sd, \
				isx005_Saturation_Default, \
				sizeof(isx005_Saturation_Default) / sizeof(isx005_Saturation_Default[0]), \
				"isx005_Saturation_Default");	
		break;

		case SATURATION_PLUS_1:
			err = isx005_write_regs(sd, \
				isx005_Saturation_Plus_1, \
				sizeof(isx005_Saturation_Plus_1) / sizeof(isx005_Saturation_Plus_1[0]), \
				"isx005_Saturation_Plus_1");	
		break;

		case SATURATION_PLUS_2:
			err = isx005_write_regs(sd, \
				isx005_Saturation_Plus_2, \
				sizeof(isx005_Saturation_Plus_2) / sizeof(isx005_Saturation_Plus_2[0]), \
				"isx005_Saturation_Plus_2");	
		break;
	}

	if(err < 0){
		dev_err(&client->dev, "%s: failed: i2c_write for set_saturation\n", __func__);
		return -EIO;
	}
	
	isx005_msg(&client->dev, "%s: done, saturation: %d\n", __func__, ctrl->value);

	return 0;
}

static int isx005_set_contrast(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	int err;
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	switch(ctrl->value)
	{
		case CONTRAST_MINUS_2:
			err = isx005_write_regs(sd, \
				isx005_Contrast_Minus_2, \
				sizeof(isx005_Contrast_Minus_2) / sizeof(isx005_Contrast_Minus_2[0]), \
				"isx005_Contrast_Minus_2");	
		break;

		case CONTRAST_MINUS_1:
			err = isx005_write_regs(sd, \
				isx005_Contrast_Minus_1, \
				sizeof(isx005_Contrast_Minus_1) / sizeof(isx005_Contrast_Minus_1[0]), \
				"isx005_Contrast_Minus_1");	
		break;

		case CONTRAST_DEFAULT:
		default:
			err = isx005_write_regs(sd, \
				isx005_Contrast_Default, \
				sizeof(isx005_Contrast_Default) / sizeof(isx005_Contrast_Default[0]), \
				"isx005_Contrast_Default");	
		break;

		case CONTRAST_PLUS_1:
			err = isx005_write_regs(sd, \
				isx005_Contrast_Plus_1, \
				sizeof(isx005_Contrast_Plus_1) / sizeof(isx005_Contrast_Plus_1[0]), \
				"isx005_Contrast_Plus_1");	
		break;

		case CONTRAST_PLUS_2:
			err = isx005_write_regs(sd, \
				isx005_Contrast_Plus_2, \
				sizeof(isx005_Contrast_Plus_2) / sizeof(isx005_Contrast_Plus_2[0]), \
				"isx005_Contrast_Plus_2");	
		break;
	}

	if(err < 0){
		dev_err(&client->dev, "%s: failed: i2c_write for set_contrast\n", __func__);
		return -EIO;
	}
	
	isx005_msg(&client->dev, "%s: done, contrast: %d\n", __func__, ctrl->value);

	return 0;
}

static int isx005_set_sharpness(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	int err;
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	switch(ctrl->value)
	{
		case SHARPNESS_MINUS_2:
			err = isx005_write_regs(sd, \
				isx005_Sharpness_Minus_2, \
				sizeof(isx005_Sharpness_Minus_2) / sizeof(isx005_Sharpness_Minus_2[0]), \
				"isx005_Sharpness_Minus_2");	
		break;

		case SHARPNESS_MINUS_1:
			err = isx005_write_regs(sd, \
				isx005_Sharpness_Minus_1, \
				sizeof(isx005_Sharpness_Minus_1) / sizeof(isx005_Sharpness_Minus_1[0]), \
				"isx005_Sharpness_Minus_1");	
		break;

		case SHARPNESS_DEFAULT:
		default:
			err = isx005_write_regs(sd, \
				isx005_Sharpness_Default, \
				sizeof(isx005_Sharpness_Default) / sizeof(isx005_Sharpness_Default[0]), \
				"isx005_Sharpness_Default");	
		break;

		case SHARPNESS_PLUS_1:
			err = isx005_write_regs(sd, \
				isx005_Sharpness_Plus_1, \
				sizeof(isx005_Sharpness_Plus_1) / sizeof(isx005_Sharpness_Plus_1[0]), \
				"isx005_Sharpness_Plus_1");	
		break;

		case SHARPNESS_PLUS_2:
			err = isx005_write_regs(sd, \
				isx005_Sharpness_Plus_2, \
				sizeof(isx005_Sharpness_Plus_2) / sizeof(isx005_Sharpness_Plus_2[0]), \
				"isx005_Sharpness_Plus_2");	
		break;
	}

	if(err < 0){
		dev_err(&client->dev, "%s: failed: i2c_write for set_saturation\n", __func__);
		return -EIO;
	}
	
	isx005_msg(&client->dev, "%s: done, sharpness: %d\n", __func__, ctrl->value);

	return 0;
}

/*Camcorder fix fps*/
static int isx005_set_sensor_mode(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	int err;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	printk("[kidggang]:func(%s):line(%d):ctrl->value(%d)\n",__func__,__LINE__,ctrl->value);
	if (ctrl->value) {
		err = isx005_i2c_write_multi(client, 0x0104, 0x000D, 1);
		err = isx005_i2c_write_multi(client, 0x0012, 0x0001, 1); //Moni_Refresh
		
		if (err < 0) {
			dev_err(&client->dev, "%s: failed: i2c_write for set_sensor_mode\n", __func__);
			return -EIO;
		}
	}
	return 0;
}

static int isx005_set_focus_mode(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err;

	switch(ctrl->value) {
		case FOCUS_MODE_MACRO:
		isx005_msg(&client->dev, "%s: FOCUS_MODE_MACRO \n", __func__);	
		err = isx005_write_regs(sd, isx005_AF_Macro_mode, ISX005_AF_MACRO_MODE_REGS, "isx005_AF_Macro_mode");
		if (err < 0) {
			dev_err(&client->dev, "%s: i2c_write failed\n", __func__);
			return -EIO;
		}
		af_mode = FOCUS_MODE_MACRO;
		break;
		case FOCUS_MODE_FD:
		break;
		case FOCUS_MODE_AUTO:
		default:
		isx005_msg(&client->dev, "%s: FOCUS_MODE_AUTO \n", __func__);	
		err = isx005_write_regs(sd, isx005_AF_Normal_mode, ISX005_AF_NORMAL_MODE_REGS, "isx005_AF_Normal_mode");
		if (err < 0) {
			dev_err(&client->dev, "%s: i2c_write failed\n", __func__);
			return -EIO;
		}
		af_mode = FOCUS_MODE_AUTO;
		break;
	}

	return 0;
}

static int isx005_set_white_balance(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	int err;
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	switch(ctrl->value)
	{
		case WHITE_BALANCE_AUTO:
			err = isx005_write_regs(sd, \
				isx005_WB_Auto, \
				sizeof(isx005_WB_Auto) / sizeof(isx005_WB_Auto[0]), \
				"isx005_WB_Auto");
		break;

		case WHITE_BALANCE_SUNNY:
			err = isx005_write_regs(sd, \
				isx005_WB_Sunny, \
				sizeof(isx005_WB_Sunny) / sizeof(isx005_WB_Sunny[0]), \
				"isx005_WB_Sunny");
		break;

		case WHITE_BALANCE_CLOUDY:
			err = isx005_write_regs(sd, \
				isx005_WB_Cloudy, \
				sizeof(isx005_WB_Cloudy) / sizeof(isx005_WB_Cloudy[0]), \
				"isx005_WB_Cloudy");
		break;

		case WHITE_BALANCE_TUNGSTEN:
			err = isx005_write_regs(sd, \
				isx005_WB_Tungsten, \
				sizeof(isx005_WB_Tungsten) / sizeof(isx005_WB_Tungsten[0]), \
				"isx005_WB_Tungsten");
		break;

		case WHITE_BALANCE_FLUORESCENT:
			err = isx005_write_regs(sd, \
				isx005_WB_Fluorescent, \
				sizeof(isx005_WB_Fluorescent) / sizeof(isx005_WB_Fluorescent[0]), \
				"isx005_WB_Fluorescent");
		break;

		default:
			dev_err(&client->dev, "%s: failed: to set_white_balance, enum: %d\n", __func__, ctrl->value);
			return -EINVAL;
		break;
	}

	if(err < 0){
		dev_err(&client->dev, "%s: failed: i2c_write for white_balance\n", __func__);
		return -EIO;
	}
	
	isx005_msg(&client->dev, "%s: done\n", __func__);

	return 0;
}

static int isx005_set_ev(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	int err;
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	switch(ctrl->value)
	{
		case EV_MINUS_4:
			err = isx005_write_regs(sd, \
				isx005_EV_Minus_4, \
				sizeof(isx005_EV_Minus_4) / sizeof(isx005_EV_Minus_4[0]), \
				"isx005_EV_Minus_4");
		break;

		case EV_MINUS_3:
			err = isx005_write_regs(sd, \
				isx005_EV_Minus_3, \
				sizeof(isx005_EV_Minus_3) / sizeof(isx005_EV_Minus_3[0]), \
				"isx005_EV_Minus_3");
		break;

		case EV_MINUS_2:
			err = isx005_write_regs(sd, \
				isx005_EV_Minus_2, \
				sizeof(isx005_EV_Minus_2) / sizeof(isx005_EV_Minus_2[0]), \
				"isx005_EV_Minus_2");
		break;

		case EV_MINUS_1:
			err = isx005_write_regs(sd, \
				isx005_EV_Minus_1, \
				sizeof(isx005_EV_Minus_1) / sizeof(isx005_EV_Minus_1[0]), \
				"isx005_EV_Minus_1");
		break;

		case EV_DEFAULT:
			err = isx005_write_regs(sd, \
				isx005_EV_Default, \
				sizeof(isx005_EV_Default) / sizeof(isx005_EV_Default[0]), \
				"isx005_EV_Default");
		break;

		case EV_PLUS_1:
			err = isx005_write_regs(sd, \
				isx005_EV_Plus_1, \
				sizeof(isx005_EV_Plus_1) / sizeof(isx005_EV_Plus_1[0]), \
				"isx005_EV_Default");
		break;

		case EV_PLUS_2:
			err = isx005_write_regs(sd, \
				isx005_EV_Plus_2, \
				sizeof(isx005_EV_Plus_2) / sizeof(isx005_EV_Plus_2[0]), \
				"isx005_EV_Plus_2");
		break;
		
		case EV_PLUS_3:
			err = isx005_write_regs(sd, \
				isx005_EV_Plus_3, \
				sizeof(isx005_EV_Plus_3) / sizeof(isx005_EV_Plus_3[0]), \
				"isx005_EV_Plus_3");
		break;

		case EV_PLUS_4:
			err = isx005_write_regs(sd, \
				isx005_EV_Plus_4, \
				sizeof(isx005_EV_Plus_4) / sizeof(isx005_EV_Plus_4[0]), \
				"isx005_EV_Plus_4");
		break;			

		default:
			dev_err(&client->dev, "%s: failed: to set_ev, enum: %d\n", __func__, ctrl->value);
			return -EINVAL;
		break;
	}

	//printk("isx005_set_ev: set_ev:, data: 0x%02x\n", isx005_buf_set_ev[1]);
	
		if(err < 0){
		dev_err(&client->dev, "%s: failed: i2c_write for set_ev\n", __func__);
		return -EIO;
	}
	
	isx005_msg(&client->dev, "%s: done\n", __func__);

	return 0;
}

static int isx005_set_metering(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	int err;
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	switch(ctrl->value)
	{
		case METERING_MATRIX:
			err = isx005_write_regs(sd, \
				isx005_Metering_Matrix, \
				sizeof(isx005_Metering_Matrix) / sizeof(isx005_Metering_Matrix[0]), \
				"isx005_Metering_Matrix");
		break;

		case METERING_CENTER:
			err = isx005_write_regs(sd, \
				isx005_Metering_Center, \
				sizeof(isx005_Metering_Center) / sizeof(isx005_Metering_Center[0]), \
				"isx005_Metering_Center");
		break;

		case METERING_SPOT:
			err = isx005_write_regs(sd, \
				isx005_Metering_Spot, \
				sizeof(isx005_Metering_Spot) / sizeof(isx005_Metering_Spot[0]), \
				"isx005_Metering_Spot");
		break;

		default:
			dev_err(&client->dev, "%s: failed: to set_photometry, enum: %d\n", __func__, ctrl->value);
			return -EINVAL;
		break;
	}
	
	if(err < 0){
		dev_err(&client->dev, "%s: failed: i2c_write for set_photometry\n", __func__);
		return -EIO;
	}
	
	isx005_msg(&client->dev, "%s: done\n", __func__);

	return 0;
}

static int isx005_set_iso(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	int err = 0;
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	switch(ctrl->value)
	{
		case ISO_50:
		case ISO_800:
		case ISO_1600:
		case ISO_SPORTS:
		case ISO_NIGHT:
		case ISO_AUTO:
		default:
			err = isx005_write_regs(sd, \
				isx005_ISO_Auto, \
				sizeof(isx005_ISO_Auto) / sizeof(isx005_ISO_Auto[0]), \
				"isx005_ISO_Auto");
		break;

		case ISO_100:
			err = isx005_write_regs(sd, \
				isx005_ISO_100, \
				sizeof(isx005_ISO_100) / sizeof(isx005_ISO_100[0]), \
				"isx005_ISO_100");
		break;

		case ISO_200:
			err = isx005_write_regs(sd, \
				isx005_ISO_200, \
				sizeof(isx005_ISO_200) / sizeof(isx005_ISO_200[0]), \
				"isx005_ISO_200");
		break;

		case ISO_400:
			err = isx005_write_regs(sd, \
				isx005_ISO_400, \
				sizeof(isx005_ISO_400) / sizeof(isx005_ISO_400[0]), \
				"isx005_ISO_400");
		break;

	}
	
	if(err < 0){
		dev_err(&client->dev, "%s: i2c_write failed\n", __func__);
		return -EIO;
	}
	return 0;
}
	
static void af_cancel_handler(unsigned long data)
{
	stop_af_operation = 0;
}

static DEFINE_MUTEX(af_cancel_op);
/* GAUDI Project([arun.c@samsung.com]) 2010.05.19. [Implemented AF cancel] */
static int isx005_set_auto_focus(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	int err;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	unsigned short read_value;
	int timeout_cnt;

	/*
	 * ctrl -> value can be 0, 1, 2
	 *
	 * 1 --> start SINGLE AF operation
	 * 0 --> stop SINGLE AF operation or cancel it
	 * 2 --> Check the status of AF cancel operation
	 */
	if (ctrl->value == 1) {
		af_operation_status = 0;
		if (stop_af_operation) {
			del_timer(&af_cancel_timer);
			stop_af_operation = 0;
			return 0;
		}

		/* Enter moniter mode if it is some different mode */
		isx005_i2c_read(client, 0x0011, &read_value);
		if ((read_value & 0xFF) != 0x00) {
			isx005_i2c_write_multi(client, 0x0011, 0x0000, 1);
			isx005_i2c_write_multi(client, 0x0012, 0x0001, 1); //Moni_Refresh
			/* Wait for Mode Transition (CM) */
			timeout_cnt = 0;
			do {
				timeout_cnt++;
				isx005_i2c_read(client, 0x00F8, &read_value);
				msleep(1);
				if (timeout_cnt > 1000) {
					dev_err(&client->dev, "%s: Entering moniter mode timed out \n", __func__);	
					break;
				}
			} while (!(read_value & 0x02));

			timeout_cnt = 0;
			do {
				timeout_cnt++;
				isx005_i2c_write_multi(client, 0x00FC, 0x0002, 1);		
				msleep(1);			
				isx005_i2c_read(client, 0x00F8, &read_value);
				if (timeout_cnt > 1000) {
					dev_err(&client->dev, "%s: Entering moniter mode timed out \n", __func__);	
					break;
				}
			} while (read_value & 0x02);	

			/* delay for AE and AWB to settle down */
			msleep(300);
		}

		if (af_mode == FOCUS_MODE_MACRO) {
			err = isx005_write_regs(sd, isx005_AF_Return_Macro_pos, ISX005_AF_RETURN_MACRO_POS_REGS, "isx005_AF_Return_Macro_pos");
			if (err < 0) {
				isx005_msg(&client->dev, "%s: register write fail \n", __func__);	
				return -EIO;
			}
		} else {
			err = isx005_write_regs(sd, isx005_AF_Return_Inf_pos, ISX005_AF_RETURN_INF_POS_REGS, "isx005_AF_Return_Inf_pos");
			if (err < 0) {
				isx005_msg(&client->dev, "%s: register write fail \n", __func__);	
				return -EIO;
			}
		}

		/* Go to Half release mode */
		isx005_i2c_read(client, 0x0011, &read_value);
		if ((read_value & 0xFF) != 0x01) {
			err = isx005_write_regs(sd, isx005_half_release, sizeof(isx005_half_release) / sizeof(isx005_half_release[0]),
				       	"isx005_half_release");

		/* Wait for Mode Transition (CM) */
			timeout_cnt = 0;
		do {
				timeout_cnt++;
			isx005_i2c_read(client, 0x00F8, &read_value);
			msleep(1);
				if (timeout_cnt > 1000) {
					dev_err(&client->dev, "%s: Entering Half release mode timed out \n", __func__);	
					break;
				}
		} while (!(read_value & 0x02));

			timeout_cnt = 0;
		do {
				timeout_cnt++;
			isx005_i2c_write_multi(client, 0x00FC, 0x0002, 1);		
			msleep(1);			
			isx005_i2c_read(client, 0x00F8, &read_value);
				if (timeout_cnt > 1000) {
					dev_err(&client->dev, "%s: Entering Half release mode timed out \n", __func__);	
					break;
				}
		} while (read_value & 0x02);	
		} 

		err = isx005_write_regs(sd, isx005_Single_AF_Start, ISX005_SINGLE_AF_START_REGS, "isx005_Single_AF_Start");
		if (err < 0) {
			isx005_msg(&client->dev, "%s: register write fail \n", __func__);	
			return -EIO;
		}

		af_operation_status = 1;
	} else if (ctrl->value == 0) {
		mutex_lock(&af_cancel_op);
		stop_af_operation = 1;
		mod_timer(&af_cancel_timer, get_jiffies_64() + msecs_to_jiffies(150));
		if (af_operation_status == 2) {
			stop_af_operation = 0;
			af_operation_status = 0;

			/* Return to moniter mode */
			isx005_i2c_read(client, 0x0011, &read_value);
			if ((read_value & 0xFF) != 0x00) {
			isx005_i2c_write_multi(client, 0x0011, 0x0000, 1);
			isx005_i2c_write_multi(client, 0x0012, 0x0001, 1); //Moni_Refresh

			/* Wait for Mode Transition (CM) */
				timeout_cnt = 0;
			do {
					timeout_cnt++;
				isx005_i2c_read(client, 0x00F8, &read_value);
				msleep(1);
					if (timeout_cnt > 1000) {
						dev_err(&client->dev, "%s: Entering moniter mode timed out \n", __func__);	
						break;
					}
			} while (!(read_value & 0x02));

				timeout_cnt = 0;
			do {
					timeout_cnt++;
				isx005_i2c_write_multi(client, 0x00FC, 0x0002, 1);		
				msleep(1);			
				isx005_i2c_read(client, 0x00F8, &read_value);
					if (timeout_cnt > 1000) {
						dev_err(&client->dev, "%s: Entering moniter mode timed out \n", __func__);	
						break;
					}
			} while (read_value & 0x02);	
			}

			if (af_mode == FOCUS_MODE_MACRO) {
				err = isx005_write_regs(sd, isx005_AF_Return_Macro_pos, ISX005_AF_RETURN_MACRO_POS_REGS, "isx005_AF_Return_Macro_pos");
				if (err < 0) {
					isx005_msg(&client->dev, "%s: register write fail \n", __func__);	
					mutex_unlock(&af_cancel_op);
			return -EIO;
		}
			} else {
				err = isx005_write_regs(sd, isx005_AF_Return_Inf_pos, ISX005_AF_RETURN_INF_POS_REGS, "isx005_AF_Return_Inf_pos");
				if (err < 0) {
					isx005_msg(&client->dev, "%s: register write fail \n", __func__);	
					mutex_unlock(&af_cancel_op);
			return -EIO;
		}
	}
		}
		mutex_unlock(&af_cancel_op);
	} else {
		isx005_msg(&client->dev, "%s:get AF cancel status start \n", __func__);	
		while (stop_af_operation)
			msleep(5);
		isx005_msg(&client->dev, "%s:get AF cancel status end \n", __func__);	
	}

	return 0;
}

static int isx005_get_auto_focus_status(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	int err, count = 0;
	unsigned short read_value;	
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int timeout_cnt;

	isx005_msg(&client->dev, "%s: Check AF Result~~~~~~~ \n", __func__);		

	/* If af operation is not started do not perform status check */
	if (!af_operation_status) {
		ctrl->value = 0x02;
		return 0;
	}

	/* Check the AF result by polling at 0x6d76 for 0x08*/
	do {
		/* count is used to prevent endless looping here */
		count++;

		/* Check for AF cancel if af is cancelled stop the operation and return */
		if (stop_af_operation) {
			isx005_msg(&client->dev, "AF is cancelled while doing\n");
			del_timer(&af_cancel_timer);
			isx005_i2c_write_multi(client, 0x4885, 0x0001, 1);

			/* Return to Macro or infinite position */
			if (af_mode == FOCUS_MODE_MACRO) {
				err = isx005_write_regs(sd, isx005_AF_Return_Macro_pos, ISX005_AF_RETURN_MACRO_POS_REGS, "isx005_AF_Return_Macro_pos");
				if (err < 0) {
					isx005_msg(&client->dev, "%s: register read fail \n", __func__);	
					return -EIO;
				}
			} else {
				err = isx005_write_regs(sd, isx005_AF_Return_Inf_pos, ISX005_AF_RETURN_INF_POS_REGS, "isx005_AF_Return_Inf_pos");
				if (err < 0) {
					isx005_msg(&client->dev, "%s: register read fail \n", __func__);	
					return -EIO;
				}
			}

			/* Return to moniter mode */
			isx005_i2c_read(client, 0x0011, &read_value);
			if ((read_value & 0xFF) != 0x00) {
			isx005_i2c_write_multi(client, 0x0011, 0x0000, 1);
			isx005_i2c_write_multi(client, 0x0012, 0x0001, 1); //Moni_Refresh

			/* Wait for Mode Transition (CM) */
				timeout_cnt = 0;
			do {
					timeout_cnt++;
				isx005_i2c_read(client, 0x00F8, &read_value);
				msleep(1);
					if (timeout_cnt > 1000) {
						dev_err(&client->dev, "%s: Entering moniter mode timed out \n", __func__);	
						break;
					}
			} while (!(read_value & 0x02));

				timeout_cnt = 0;
			do {
					timeout_cnt++;
				isx005_i2c_write_multi(client, 0x00FC, 0x0002, 1);		
				msleep(1);			
				isx005_i2c_read(client, 0x00F8, &read_value);
					if (timeout_cnt > 1000) {
						dev_err(&client->dev, "%s: Entering moniter mode timed out \n", __func__);	
						break;
					}
			} while (read_value & 0x02);	
			}

			/* Clear AF_LOCK_STS */
			isx005_i2c_write_multi(client, 0x00FC, 0x0010, 1);

			/*
			 * Inform user that AF is cancelled
			 * 0 --> AF failure
			 * 1 --> AF success
			 * 2 --> AF cancel
			 */
			if (count < 100 && count > 15)
				msleep(100);			
			else if (count >= 100)
				msleep(300);			

			ctrl->value = 0x02;
			stop_af_operation = 0;
			isx005_msg(&client->dev, "AF cancel finished\n");
			return 0;
		}
		isx005_i2c_read(client, 0x6D76, &read_value);
		isx005_msg(&client->dev, "%s: i2c_read --- read_value == 0x%x \n", __func__, read_value);		
		msleep(1);
		if (count > 1000) {
			ctrl->value = 0x00;	/* 0x00 --> AF failed*/ 
			break;
		}
	}while(!(read_value&0x08));	

	mutex_lock(&af_cancel_op);
	/* Clear AF_LOCK_STS */
	isx005_i2c_write_multi(client, 0x00FC, 0x0010, 1);

	/* Read AF result */
	isx005_i2c_read(client, 0x6D77, &read_value);
	dev_err(&client->dev, "%s: i2c_read --- read_value == 0x%x \n", __func__, read_value);		

	if ((read_value & 0xFF) == 0x01) {
		isx005_msg(&client->dev, "%s: AF is success~~~~~~~ \n", __func__);
		ctrl->value = 0x01;	/* 0x01 --> AF sucess */ 
	} else if ((read_value & 0xFF) == 0x00) {
		isx005_msg(&client->dev, "%s: AF is Failure~~~~~~~ \n", __func__);
		ctrl->value = 0x00;	/* 0x00 --> AF failed*/ 
	}

	/* We finished turn off the single AF now */
	isx005_msg(&client->dev, "%s: single AF Off command Setting~~~~ \n", __func__);	

		err = isx005_write_regs(sd, isx005_Single_AF_Off, ISX005_SINGLE_AF_OFF_REGS, "isx005_Single_AF_Off");
	if (err < 0) {
		isx005_msg(&client->dev, "%s: register write fail \n", __func__);	
		mutex_unlock(&af_cancel_op);
		return -EIO;
	}

	if (stop_af_operation) {
		/* Return to moniter mode */
		isx005_i2c_read(client, 0x0011, &read_value);
		if ((read_value & 0xFF) != 0x00) {
		isx005_i2c_write_multi(client, 0x0011, 0x0000, 1);
		isx005_i2c_write_multi(client, 0x0012, 0x0001, 1); //Moni_Refresh

		/* Wait for Mode Transition (CM) */
			timeout_cnt = 0;
		do {
				timeout_cnt++;
			isx005_i2c_read(client, 0x00F8, &read_value);
			msleep(1);
				if (timeout_cnt > 1000) {
					dev_err(&client->dev, "%s: Entering moniter mode timed out \n", __func__);	
					break;
				}
		} while (!(read_value & 0x02));

			timeout_cnt = 0;
		do {
				timeout_cnt++;
			isx005_i2c_write_multi(client, 0x00FC, 0x0002, 1);		
			msleep(1);			
			isx005_i2c_read(client, 0x00F8, &read_value);
				if (timeout_cnt > 1000) {
					dev_err(&client->dev, "%s: Entering moniter mode timed out \n", __func__);	
					break;
				}
		} while (read_value & 0x02);	
		}

		/* Return to Macro or infinite position */
		if (af_mode == FOCUS_MODE_MACRO) {
			err = isx005_write_regs(sd, isx005_AF_Return_Macro_pos, ISX005_AF_RETURN_MACRO_POS_REGS, "isx005_AF_Return_Macro_pos");
			if (err < 0) {
				isx005_msg(&client->dev, "%s: register write fail \n", __func__);	
				mutex_unlock(&af_cancel_op);
			return -EIO;
			}
		} else {
			err = isx005_write_regs(sd, isx005_AF_Return_Inf_pos, ISX005_AF_RETURN_INF_POS_REGS, "isx005_AF_Return_Inf_pos");
			if (err < 0) {
				isx005_msg(&client->dev, "%s: register write fail \n", __func__);	
				mutex_unlock(&af_cancel_op);
				return -EIO;
			}	
		}	
	}

	stop_af_operation = 0;
	af_operation_status = 2;
	isx005_msg(&client->dev, "%s: single AF check finished~~~~ \n", __func__);	
	mutex_unlock(&af_cancel_op);
	return 0;
}

static int isx005_aeawb_unlock(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	unsigned short read_value;	
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int timeout_cnt;

	isx005_msg(&client->dev, "%s: unlocking AE&AWB\n", __func__);	

	mutex_lock(&af_cancel_op);
	if (af_operation_status == 2) {
		af_operation_status = 0;

		/* Enter moniter mode if it is some different mode */
		isx005_i2c_read(client, 0x0011, &read_value);
		if ((read_value & 0xFF) != 0x00) {
		isx005_i2c_write_multi(client, 0x0011, 0x0000, 1);
		isx005_i2c_write_multi(client, 0x0012, 0x0001, 1); //Moni_Refresh
		/* Wait for Mode Transition (CM) */
			timeout_cnt = 0;
		do {
				timeout_cnt++;
			isx005_i2c_read(client, 0x00F8, &read_value);
			msleep(1);
				if (timeout_cnt > 1000) {
					dev_err(&client->dev, "%s: Entering moniter mode timed out \n", __func__);	
					break;
				}
		} while (!(read_value & 0x02));

			timeout_cnt = 0;
		do {
				timeout_cnt++;
			isx005_i2c_write_multi(client, 0x00FC, 0x0002, 1);		
			msleep(1);			
			isx005_i2c_read(client, 0x00F8, &read_value);
				if (timeout_cnt > 1000) {
					dev_err(&client->dev, "%s: Entering moniter mode timed out \n", __func__);	
					break;
				}
		} while (read_value & 0x02);	
		}
	}
	mutex_unlock(&af_cancel_op);
	return 0;
}

static int isx005_get_iso(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	int err;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	unsigned short tmp;
	unsigned int iso_table[19] = {25, 32, 40, 50, 64,
							 80, 100, 125, 160, 200,
							 250, 320, 400, 500, 640,
							 800, 1000, 1250, 1600};

	err = isx005_i2c_read(client, 0x00F0, &tmp);
	ctrl->value = iso_table[tmp-1];

	return err;
}

static int isx005_get_shutterspeed(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	int err;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	unsigned short tmp;

	err = isx005_i2c_read(client, 0x00F2, &tmp);
	ctrl->value |= tmp;
	err = isx005_i2c_read(client, 0x00F4, &tmp);
	ctrl->value |= (tmp << 16);

	return err;
}

static void isx005_init_parameters(struct v4l2_subdev *sd)
{
	struct isx005_state *state = to_state(sd);

	/* Set initial values for the sensor stream parameters */
	state->strm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	state->strm.parm.capture.timeperframe.numerator = 1;
	state->strm.parm.capture.capturemode = 0;

//	state->framesize_index = ISX005_PREVIEW_SVGA;//bestiq
	state->fps = 30; /* Default value */
	
	state->jpeg.enable = 0;
	state->jpeg.quality = 100;
	state->jpeg.main_offset = 0;
	state->jpeg.main_size = 0;
	state->jpeg.thumb_offset = 0;
	state->jpeg.thumb_size = 0;
	state->jpeg.postview_offset = 0;
}

#if 0
/* Sample code */
static const char *isx005_querymenu_wb_preset[] = {
	"WB Tungsten", "WB Fluorescent", "WB sunny", "WB cloudy", NULL
};
#endif

static struct v4l2_queryctrl isx005_controls[] = {
#if 0
	/* Sample code */
	{
		.id = V4L2_CID_WHITE_BALANCE_PRESET,
		.type = V4L2_CTRL_TYPE_MENU,
		.name = "White balance preset",
		.minimum = 0,
		.maximum = ARRAY_SIZE(isx005_querymenu_wb_preset) - 2,
		.step = 1,
		.default_value = 0,
	},
#endif
};

const char **isx005_ctrl_get_menu(u32 id)
{
	switch (id) {
#if 0
	/* Sample code */
	case V4L2_CID_WHITE_BALANCE_PRESET:
		return isx005_querymenu_wb_preset;
#endif
	default:
		return v4l2_ctrl_get_menu(id);
	}
}

static inline struct v4l2_queryctrl const *isx005_find_qctrl(int id)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(isx005_controls); i++)
		if (isx005_controls[i].id == id)
			return &isx005_controls[i];

	return NULL;
}

static int isx005_queryctrl(struct v4l2_subdev *sd, struct v4l2_queryctrl *qc)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(isx005_controls); i++) {
		if (isx005_controls[i].id == qc->id) {
			memcpy(qc, &isx005_controls[i], sizeof(struct v4l2_queryctrl));
			return 0;
		}
	}

	return -EINVAL;
}

static int isx005_querymenu(struct v4l2_subdev *sd, struct v4l2_querymenu *qm)
{
	struct v4l2_queryctrl qctrl;

	qctrl.id = qm->id;
	isx005_queryctrl(sd, &qctrl);

	return v4l2_ctrl_query_menu(qm, &qctrl, isx005_ctrl_get_menu(qm->id));
}

/*
 * Clock configuration
 * Configure expected MCLK from host and return EINVAL if not supported clock
 * frequency is expected
 * 	freq : in Hz
 * 	flag : not supported for now
 */
static int isx005_s_crystal_freq(struct v4l2_subdev *sd, u32 freq, u32 flags)
{
	int err = -EINVAL;

	return err;
}

static int isx005_g_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
	int err = 0;

	return err;
}

static int isx005_get_framesize_index(struct v4l2_subdev *sd);
static int isx005_set_framesize_index(struct v4l2_subdev *sd, unsigned int index);
/* Information received: 
 * width, height
 * pixel_format -> to be handled in the upper layer 
 *
 * */
static int isx005_s_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
	int err = 0;
	struct isx005_state *state = to_state(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int framesize_index = -1;

	if(fmt->fmt.pix.pixelformat == V4L2_PIX_FMT_JPEG && fmt->fmt.pix.colorspace != V4L2_COLORSPACE_JPEG){
		dev_err(&client->dev, "%s: mismatch in pixelformat and colorspace\n", __func__);
		return -EINVAL;
	}

	if(fmt->fmt.pix.width == 800 && fmt->fmt.pix.height == 448) {
		state->pix.width = 1280;
		state->pix.height = 720;
	} else {
		state->pix.width = fmt->fmt.pix.width;
		state->pix.height = fmt->fmt.pix.height;
	}
	
	state->pix.pixelformat = fmt->fmt.pix.pixelformat;

	if(fmt->fmt.pix.colorspace == V4L2_COLORSPACE_JPEG)
		state->oprmode = ISX005_OPRMODE_IMAGE;
	else
		state->oprmode = ISX005_OPRMODE_VIDEO; 


	framesize_index = isx005_get_framesize_index(sd);

	isx005_msg(&client->dev, "%s:framesize_index = %d\n", __func__, framesize_index);
	
	err = isx005_set_framesize_index(sd, framesize_index);
	if(err < 0){
		dev_err(&client->dev, "%s: set_framesize_index failed\n", __func__);
		return -EINVAL;
	}

	if(state->pix.pixelformat == V4L2_PIX_FMT_JPEG){
		state->jpeg.enable = 1;
	} else {
		state->jpeg.enable = 0;
	}

	return 0;
}

static int isx005_enum_framesizes(struct v4l2_subdev *sd, \
					struct v4l2_frmsizeenum *fsize)
{
	struct isx005_state *state = to_state(sd);
	int num_entries = sizeof(isx005_framesize_list)/sizeof(struct isx005_enum_framesize);	
	struct isx005_enum_framesize *elem;	
	int index = 0;
	int i = 0;

	/* The camera interface should read this value, this is the resolution
 	 * at which the sensor would provide framedata to the camera i/f
 	 *
 	 * In case of image capture, this returns the default camera resolution (SVGA)
 	 */
	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;

	if(state->pix.pixelformat == V4L2_PIX_FMT_JPEG){
		index = ISX005_PREVIEW_SVGA;
	} else {
		index = state->framesize_index;
	}

	for(i = 0; i < num_entries; i++){
		elem = &isx005_framesize_list[i];
		if(elem->index == index){
			fsize->discrete.width = isx005_framesize_list[index].width;
			fsize->discrete.height = isx005_framesize_list[index].height;
			return 0;
		}
	}

	return -EINVAL;
}

static int isx005_enum_frameintervals(struct v4l2_subdev *sd, 
					struct v4l2_frmivalenum *fival)
{
	int err = 0;

	return err;
}

static int isx005_enum_fmt(struct v4l2_subdev *sd, struct v4l2_fmtdesc *fmtdesc)
{
	int num_entries;

	num_entries = sizeof(capture_fmts)/sizeof(struct v4l2_fmtdesc);

	if(fmtdesc->index >= num_entries)
		return -EINVAL;

        memset(fmtdesc, 0, sizeof(*fmtdesc));
        memcpy(fmtdesc, &capture_fmts[fmtdesc->index], sizeof(*fmtdesc));

	return 0;
}

static int isx005_try_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
	int num_entries;
	int i;

	num_entries = sizeof(capture_fmts)/sizeof(struct v4l2_fmtdesc);

	for(i = 0; i < num_entries; i++){
		if(capture_fmts[i].pixelformat == fmt->fmt.pix.pixelformat)
			return 0;
	} 

	return -EINVAL;
}

/** Gets current FPS value */
static int isx005_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct isx005_state *state = to_state(sd);
	int err = 0;

	state->strm.parm.capture.timeperframe.numerator = 1;
	state->strm.parm.capture.timeperframe.denominator = state->fps;

	memcpy(param, &state->strm, sizeof(param));

	return err;
}

/** Sets the FPS value */
static int isx005_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	int err = 0;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct isx005_state *state = to_state(sd);

	if(param->parm.capture.timeperframe.numerator != state->strm.parm.capture.timeperframe.numerator ||
			param->parm.capture.timeperframe.denominator != state->strm.parm.capture.timeperframe.denominator){
		
		int fps = 0;
		int fps_max = 30;

		if(param->parm.capture.timeperframe.numerator && param->parm.capture.timeperframe.denominator)
			fps = (int)(param->parm.capture.timeperframe.denominator/param->parm.capture.timeperframe.numerator);
		else 
			fps = 0;

		if(fps <= 0 || fps > fps_max){
			dev_err(&client->dev, "%s: Framerate %d not supported, setting it to %d fps.\n",__func__, fps, fps_max);
			fps = fps_max;
		}

		param->parm.capture.timeperframe.numerator = 1;
		param->parm.capture.timeperframe.denominator = fps;
	
		state->fps = fps;
	}

	/* Don't set the fps value, just update it in the state 
	 * We will set the resolution and fps in the start operation (preview/capture) call */
	
	return err;
}

/* This function is called from the g_ctrl api
 *
 * This function should be called only after the s_fmt call,
 * which sets the required width/height value.
 *
 * It checks a list of available frame sizes and returns the 
 * most appropriate index of the frame size.
 *
 * Note: The index is not the index of the entry in the list. It is
 * the value of the member 'index' of the particular entry. This is
 * done to add additional layer of error checking.
 *
 * The list is stored in an increasing order (as far as possible).
 * Hene the first entry (searching from the beginning) where both the 
 * width and height is more than the required value is returned.
 * In case of no match, we return the last entry (which is supposed
 * to be the largest resolution supported.)
 *
 * It returns the index (enum isx005_frame_size) of the framesize entry.
 */
static int isx005_get_framesize_index(struct v4l2_subdev *sd)
{
	int i = 0;
	struct isx005_state *state = to_state(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct isx005_enum_framesize *frmsize;

	isx005_msg(&client->dev, "%s: Requested Res: %dx%d\n", __func__, state->pix.width, state->pix.height);

	/* Check for video/image mode */
	for(i = 0; i < (sizeof(isx005_framesize_list)/sizeof(struct isx005_enum_framesize)); i++)
	{
		frmsize = &isx005_framesize_list[i];

		if(frmsize->mode != state->oprmode)
			continue;

		if(state->oprmode == ISX005_OPRMODE_IMAGE){
			/* In case of image capture mode, if the given image resolution is not supported,
 			 * return the next higher image resolution. */
			if(frmsize->width >= state->pix.width && frmsize->height >= state->pix.height)
				return frmsize->index;
		} else {
			/* In case of video mode, if the given video resolution is not matching, use
 			 * the default rate (currently ISX005_PREVIEW_WVGA).
 			 */		 
			if(frmsize->width == state->pix.width && frmsize->height == state->pix.height)
				return frmsize->index;
		}

	} 
	
	/* If it fails, return the default value. */
	return (state->oprmode == ISX005_OPRMODE_IMAGE) ? ISX005_CAPTURE_3MP : ISX005_PREVIEW_SVGA;
}


/* This function is called from the s_ctrl api
 * Given the index, it checks if it is a valid index.
 * On success, it returns 0.
 * On Failure, it returns -EINVAL
 */
static int isx005_set_framesize_index(struct v4l2_subdev *sd, unsigned int index)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct isx005_state *state = to_state(sd);
	int i = 0;

	for(i = 0; i < (sizeof(isx005_framesize_list)/sizeof(struct isx005_enum_framesize)); i++)
	{
		if(isx005_framesize_list[i].index == index && isx005_framesize_list[i].mode == state->oprmode){
			state->framesize_index = isx005_framesize_list[i].index;	
			state->pix.width = isx005_framesize_list[i].width;
			state->pix.height = isx005_framesize_list[i].height;
			isx005_info(&client->dev, "%s: Camera Res: %dx%d\n", __func__, state->pix.width, state->pix.height);
			return 0;
		} 
	} 
	
	return -EINVAL;
}

static int isx005_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct isx005_state *state = to_state(sd);
	struct isx005_userset userset = state->userset;
	int err = -ENOIOCTLCMD;

	CDBG("control id = 0x%x, %d\n", ctrl->id, ctrl->id & 0xFF);
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
		ctrl->value = userset.sharpness;
		err = 0;
		break;

	case V4L2_CID_CAM_JPEG_MAIN_SIZE:
		ctrl->value = state->jpeg.main_size;
		err = 0;
		break;
	
	case V4L2_CID_CAM_JPEG_MAIN_OFFSET:
		ctrl->value = state->jpeg.main_offset;
		err = 0;
		break;

	case V4L2_CID_CAM_JPEG_THUMB_SIZE:
		ctrl->value = state->jpeg.thumb_size;
		err = 0;
		break;
	case V4L2_CID_CAM_JPEG_THUMB_OFFSET:
		ctrl->value = state->jpeg.thumb_offset;
		err = 0;
		break;

	case V4L2_CID_CAM_JPEG_POSTVIEW_OFFSET:
		ctrl->value = state->jpeg.postview_offset;
		err = 0;
		break; 
	
	case V4L2_CID_CAM_JPEG_MEMSIZE:
		ctrl->value = SENSOR_JPEG_SNAPSHOT_MEMSIZE;
		err = 0;
		break;

	//need to be modified
	case V4L2_CID_CAM_JPEG_QUALITY:
		ctrl->value = state->jpeg.quality;
		err = 0;
		break;
	case V4L2_CID_CAMERA_OBJ_TRACKING_STATUS:
		err = 0;
		break;
	case V4L2_CID_CAMERA_SMART_AUTO_STATUS:
		err = 0;
		break;

	case V4L2_CID_CAMERA_AUTO_FOCUS_RESULT:
		err = isx005_get_auto_focus_status(sd, ctrl);
		break;

	case V4L2_CID_CAM_DATE_INFO_YEAR:
		ctrl->value = 2010;//state->dateinfo.year;//bestiq 
		err = 0;
		break; 
	case V4L2_CID_CAM_DATE_INFO_MONTH:
		ctrl->value = 2;//state->dateinfo.month;
		err = 0;
		break; 
	case V4L2_CID_CAM_DATE_INFO_DATE:
		ctrl->value = 25;//state->dateinfo.date;
		err = 0;
		break; 
	case V4L2_CID_CAM_SENSOR_VER:
		ctrl->value = state->sensor_version;
		err = 0;
		break; 
	case V4L2_CID_CAM_FW_MINOR_VER:
		ctrl->value = state->fw.minor;
		err = 0;
		break; 
	case V4L2_CID_CAM_FW_MAJOR_VER:
		ctrl->value = state->fw.major;
		err = 0;
		break; 
	case V4L2_CID_CAM_PRM_MINOR_VER:
		ctrl->value = state->prm.minor;
		err = 0;
		break; 
	case V4L2_CID_CAM_PRM_MAJOR_VER:
		ctrl->value = state->prm.major;
		err = 0;
		break; 
	default:
		dev_err(&client->dev, "%s: no such ctrl\n", __func__);
		break;
	}
	
	return err;
}

static int isx005_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct isx005_state *state = to_state(sd);
	int err = 0;

	isx005_info(&client->dev, "%s: V4l2 control ID =%d\n", __func__, ctrl->id - V4L2_CID_PRIVATE_BASE);

	mutex_lock(&sensor_s_ctrl);

	switch (ctrl->id) {
	case V4L2_CID_CAMERA_AEAWB_LOCK_UNLOCK:
		err = isx005_aeawb_unlock(sd, ctrl);
		break;
		
	case V4L2_CID_CAMERA_FLASH_MODE:
		err = 0;
		break;
	case V4L2_CID_CAMERA_BRIGHTNESS:
		err = isx005_set_ev(sd, ctrl);
		break;
	case V4L2_CID_CAMERA_WHITE_BALANCE:
		err = isx005_set_white_balance(sd, ctrl);
		break;
	case V4L2_CID_CAMERA_EFFECT:
		err = isx005_set_effect(sd, ctrl);
		break;
	case V4L2_CID_CAMERA_ISO:
		err = isx005_set_iso(sd, ctrl);
		break;
	case V4L2_CID_CAMERA_METERING:
		err = isx005_set_metering(sd, ctrl);
		break;
	case V4L2_CID_CAMERA_CONTRAST:
		err = isx005_set_contrast(sd, ctrl);
		break;
	case V4L2_CID_CAMERA_SATURATION:
		err = isx005_set_saturation(sd, ctrl);
		break;
	case V4L2_CID_CAMERA_SHARPNESS:
		err = isx005_set_sharpness(sd, ctrl);
		break;
/*Camcorder fix fps*/
	case V4L2_CID_CAMERA_SENSOR_MODE:
		err = isx005_set_sensor_mode(sd, ctrl);
		break;

	case V4L2_CID_CAMERA_WDR:
		err = 0;
		break;

	case V4L2_CID_CAMERA_ANTI_SHAKE:
		err = 0;
		break;

	case V4L2_CID_CAMERA_FACE_DETECTION:
		err = 0;
		break;

	case V4L2_CID_CAMERA_SMART_AUTO:
		err = 0;
		break;

	case V4L2_CID_CAMERA_FOCUS_MODE:
		err = isx005_set_focus_mode(sd, ctrl);
		break;
		
	case V4L2_CID_CAMERA_VINTAGE_MODE:
		err = 0;
		break;
		
	case V4L2_CID_CAMERA_BEAUTY_SHOT:
		err = 0;
		break;

	case V4L2_CID_CAMERA_FACEDETECT_LOCKUNLOCK:
		err = 0;
		break;		

	//need to be modified
	case V4L2_CID_CAM_JPEG_QUALITY:
		if(ctrl->value < 0 || ctrl->value > 100){
			err = -EINVAL;
		} else {
			state->jpeg.quality = ctrl->value;
			err = isx005_set_jpeg_quality(sd);
		}
		break;

	case V4L2_CID_CAMERA_SCENE_MODE:
		err = isx005_change_scene_mode(sd, ctrl);
		printk("isx005_change_scene_mode = %d \n", ctrl->value);	
//		err = 0;
		break;

	case V4L2_CID_CAMERA_GPS_LATITUDE:
		dev_err(&client->dev, "%s: V4L2_CID_CAMERA_GPS_LATITUDE: not implemented\n", __func__);
		break;

	case V4L2_CID_CAMERA_GPS_LONGITUDE:
		dev_err(&client->dev, "%s: V4L2_CID_CAMERA_GPS_LONGITUDE: not implemented\n", __func__);
		break;

	case V4L2_CID_CAMERA_GPS_TIMESTAMP:
		dev_err(&client->dev, "%s: V4L2_CID_CAMERA_GPS_TIMESTAMP: not implemented\n", __func__);
		break;

	case V4L2_CID_CAMERA_GPS_ALTITUDE:
		dev_err(&client->dev, "%s: V4L2_CID_CAMERA_GPS_ALTITUDE: not implemented\n", __func__);
		break;

	case V4L2_CID_CAMERA_ZOOM:
		err = isx005_set_dzoom(sd, ctrl);
//		err = 0;
		break;

	case V4L2_CID_CAMERA_TOUCH_AF_START_STOP:
		err = 0;
		break;
		
	case V4L2_CID_CAMERA_CAF_START_STOP:
		err = 0;
		break;	

	case V4L2_CID_CAMERA_OBJECT_POSITION_X:
		state->position.x = ctrl->value;
		err = 0;
		break;

	case V4L2_CID_CAMERA_OBJECT_POSITION_Y:
		state->position.y = ctrl->value;
		err = 0;
		break;

	case V4L2_CID_CAMERA_OBJ_TRACKING_START_STOP:
		err = 0;
		break;

	case V4L2_CID_CAMERA_SET_AUTO_FOCUS:
		err = isx005_set_auto_focus(sd, ctrl);
		break;		

	case V4L2_CID_CAMERA_FRAME_RATE:
		state->fps = ctrl->value;
		err = 0;		
		break;
		
	case V4L2_CID_CAMERA_ANTI_BANDING:
		err = 0;		
		break;

	case V4L2_CID_CAM_CAPTURE:
		err = isx005_set_capture_start(sd, ctrl);
		break;
	
	/* Used to start / stop preview operation. 
 	 * This call can be modified to START/STOP operation, which can be used in image capture also */
	case V4L2_CID_CAM_PREVIEW_ONOFF:
		if(ctrl->value)
			err = isx005_set_preview_start(sd);
		else
			err = isx005_set_preview_stop(sd);
		break;

	case V4L2_CID_CAM_UPDATE_FW:
		err = 0;
		break;

	case V4L2_CID_CAM_SET_FW_ADDR:
		err = 0;
		break;

	case V4L2_CID_CAM_SET_FW_SIZE:
		err = 0;
		break;

	case V4L2_CID_CAM_FW_VER:
		err = 0;
		break;
		
	case V4L2_CID_CAMERA_GET_ISO:
		err = isx005_get_iso(sd, ctrl); 	
		break;
	
	case V4L2_CID_CAMERA_GET_SHT_TIME:
		err = isx005_get_shutterspeed(sd, ctrl);		
		break;	

	default:
		dev_err(&client->dev, "%s: no such control\n", __func__);
		break;
	}

	if (err < 0)
		dev_err(&client->dev, "%s: vidioc_s_ctrl failed %d\n", __func__, err);

	mutex_unlock(&sensor_s_ctrl);
	return err;
}

static int isx005_calibration(struct v4l2_subdev *sd)
{	
	unsigned long OTP00, OTP10;
	unsigned long OTP0, OTP1, OTP2; //OTPX0, OTPX1, OTPX2
	unsigned char valid_OPT = 3;
	unsigned char ret0, ret1;	
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	
	printk("isx005_calibration : start \n");
	
	ret0 = isx005_i2c_read_multi(client, 0x0238, &OTP10);
	ret1 = isx005_i2c_read_multi(client, 0x022C, &OTP00);	

	if(ret0 & ret1)
	{
		dev_err(&client->dev, "%s [cam] OPT10=0x%lx, OPT00=0x%lx \n", __func__, OTP10,OTP00);	

		if(((OTP10 & 0x10) >> 4) == 1) 
		{// CASE1 : READ OPT1 DATA
		    OTP0 = OTP10;
		    
		    isx005_i2c_read_multi(client, 0x023C, &OTP1);
		    isx005_i2c_read_multi(client, 0x0240, &OTP2);
		    valid_OPT = 1;
		}
		else if(((OTP00 & 0x10) >> 4) == 1)
		{// CASE2 : READ OPT0 DATA 
		    OTP0 = OTP00;
		    isx005_i2c_read_multi(client, 0x0230, &OTP1);
		    isx005_i2c_read_multi(client, 0x0234, &OTP2);
		    valid_OPT = 0;
		}
		else  // if((((OTP10 & 0x10) >> 4) == 0) && (((OTP00 & 0x10) >> 4) == 0 ) )
		{// CASE3 : Default cal. 
		    // Module was not calibrated in module vendor.
		    valid_OPT = 2;
		    return FALSE;
		}
	}
	else
	{
	    dev_err(&client->dev,"%s [cam] CALIBRATION : READ FAIL \n", __func__); 
	    return FALSE;
	}

	dev_err(&client->dev,"%s [cam] valid_OPT = %d \n", __func__, valid_OPT); 
	dev_err(&client->dev,"%s [cam] OTP2=0x%lx, OTP1=0x%lx, OTP0=0x%lx \n", __func__, OTP2, OTP1, OTP0); 


	// Shading Cal. 적용
	if (valid_OPT == 1 || valid_OPT ==0)
	{ //CASE 1 || CASE 2:
	//Shading Index : OPTx0 [14-13]

	    unsigned char shading_index;
	    int err = 0;
	    
	    shading_index = (OTP0 & 0x6000) >> 13;
	    dev_err(&client->dev,"%s [cam] Shading Cal. : shading_index = %d  \n", __func__, shading_index); 

	    if (shading_index == 1) // 01
	    {
		err = isx005_write_regs(sd, \
			isx005_shading_2, \
			sizeof(isx005_shading_2) / sizeof(isx005_shading_2[0]), \
			"isx005_shading_2");
			if (err < 0)
			{
				isx005_msg(&client->dev, "%s: register write fail \n", __func__);	
				return -EIO;
			}		
	    }
	    else if (shading_index== 2) // 10
	    {
		err = isx005_write_regs(sd, \
			isx005_shading_3, \
			sizeof(isx005_shading_3) / sizeof(isx005_shading_3[0]), \
			"isx005_shading_3");
			if (err < 0)
			{
				isx005_msg(&client->dev, "%s: register write fail \n", __func__);	
				return -EIO;
			}		
	    }
	    else //  if (shading_index == 0), 00 or 11
	    {
		err = isx005_write_regs(sd, \
			isx005_shading_1, \
			sizeof(isx005_shading_1) / sizeof(isx005_shading_1[0]), \
			"isx005_shading_1");
			if (err < 0)
			{
				isx005_msg(&client->dev, "%s: register write fail \n", __func__);	
				return -EIO;
			}		
	    }
	}


	// AWB Cal. 적용

	{
	unsigned short NORMR, NORMB; //14bit
	unsigned short AWBPRER, AWBPREB; //10bit

	NORMR = ((OTP1 & 0x3F) << 8) | ((OTP0 & 0xFF000000) >> 24);
	dev_err(&client->dev,"%s [cam] NORMR = 0x%x \n", __func__, NORMR); 
	if(NORMR <= 0x3FFF)
	    isx005_i2c_write_multi(client, 0x4A04, NORMR, 2);


	NORMB =  ((OTP1 & 0xFFFC0) >> 6);
	dev_err(&client->dev,"%s [cam] NORMB = 0x%x \n", __func__, NORMB); 
	if(NORMB <= 0x3FFF)
	    isx005_i2c_write_multi(client, 0x4A06, NORMB, 2);


	AWBPRER = ((OTP1 & 0x3FF00000) >> 20);
	dev_err(&client->dev,"%s [cam] AWBPRER = 0x%x \n", __func__, AWBPRER); 
	if(AWBPRER <= 0x3FF)
	    isx005_i2c_write_multi(client, 0x4A08, AWBPRER, 2);


	AWBPREB = ((OTP2 & 0xFF) << 2) | ((OTP1 & 0xC0000000) >> 30);
	dev_err(&client->dev,"%s [cam] AWBPREB = 0x%x \n", __func__, AWBPREB); 
	if(AWBPREB <= 0x3FF)
	    isx005_i2c_write_multi(client, 0x4A0A, AWBPREB, 2);

	}

	dev_err(&client->dev,"%s [cam] CALIBRATION : END \n", __func__); 
	return TRUE;		
}

static int  isx005_set_default_calibration(struct v4l2_subdev *sd)
{
	int err;

	struct i2c_client *client = v4l2_get_subdevdata(sd);
	dev_err(&client->dev,"%s [cam] isx005_set_default_calibration  \n", __func__); 

	err = isx005_write_regs(sd, \
		isx005_default_calibration, \
		sizeof(isx005_default_calibration) / sizeof(isx005_default_calibration[0]), \
		"isx005_default_calibration");
		if (err < 0)
		{
			isx005_msg(&client->dev, "%s: register write fail \n", __func__);	
			return -EIO;
		}    
	return TRUE;
}

static int isx005_init(struct v4l2_subdev *sd, u32 val)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = -EINVAL, i, count=0;
	unsigned short read_value_1, read_value_2;

	dev_err(&client->dev, "%s: init setting~~~~~~~~~~~~~~\n", __func__);
	isx005_init_parameters(sd);

	msleep(10);	
#ifdef CONFIG_LOAD_FILE
	printk("[BestIQ] + isx005_init\n");
	err = isx005_regs_table_init();
	if (err) {
		isx005_msg(&client->dev, "%s: config file read fail\n", __func__);
		return -EIO;
	}
	msleep(100);
	
	isx005_write_regs(sd, isx005_init_reg, ISX005_INIT_REGS, "isx005_init_reg");
	msleep(20);	

	if (isx005_calibration(sd) == FALSE)
	{
	    isx005_set_default_calibration(sd);
	}

	dev_err(&client->dev, "%s: isx005_init_image_tuning_setting~~~~~~~~~~~~~~\n", __func__);		
	isx005_write_regs(sd, isx005_init_image_tuning_setting, ISX005_INIT_IMAGETUNING_SETTING_REGS, "isx005_init_image_tuning_setting");
	printk("[BestIQ] - isx005_init\n");

	isx005_cam_stdby(TRUE);//bestiq  standby pin
#else

	for(i = 0; i <ISX005_INIT_REGS; i++)
	{
		err = isx005_i2c_write_multi(client, isx005_init_reg[i].subaddr ,  isx005_init_reg[i].value,  isx005_init_reg[i].len);
		if (err < 0)
		{
			isx005_msg(&client->dev, "%s: register read fail \n", __func__);	
			return -EIO;
		}		
	}

	msleep(5);

	if (isx005_calibration(sd) == FALSE)
	{
	    isx005_set_default_calibration(sd);
	}
	
	dev_err(&client->dev, "%s: isx005_init_image_tuning_setting~~~~~~~~~~~~~~\n", __func__);		
	for(i = 0; i <ISX005_INIT_IMAGETUNING_SETTING_REGS; i++)
	{
		err = isx005_i2c_write_multi(client, isx005_init_image_tuning_setting[i].subaddr ,  isx005_init_image_tuning_setting[i].value ,  isx005_init_image_tuning_setting[i].len);
		if (err < 0)
		{
			isx005_msg(&client->dev, "%s: register read fail \n", __func__);	
			return -EIO;
		}		
	}	

	isx005_cam_stdby(TRUE);//bestiq  standby pin
#endif	
//Can use AF Command
	do
	{
		count++;
		isx005_i2c_read(client, 0x000A, &read_value_1);
		dev_err(&client->dev, "%s: i2c_read --- read_value_1 == 0x%x \n", __func__, read_value_1);

 		isx005_i2c_read(client, 0x6D76, &read_value_2);
		dev_err(&client->dev, "%s: i2c_read --- read_value_2 == 0x%x \n", __func__, read_value_2);
		msleep(10);
		/*
		 * Arun c
		 * When the esd error occures during init the while loop never returns
		 * so keep a count of the loops
		 */
		if (count > 100)
			break;
	}while((!(read_value_1&0x02))&&(!(read_value_2&0x03)));		

	
	return err;
}

/*
 * s_config subdev ops
 * With camera device, we need to re-initialize every single opening time therefor,
 * it is not necessary to be initialized on probe time. except for version checking
 * NOTE: version checking is optional
 */
static int isx005_s_config(struct v4l2_subdev *sd, int irq, void *platform_data)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct isx005_state *state = to_state(sd);
	struct isx005_platform_data *pdata;

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
		state->pix.pixelformat = DEFAULT_PIX_FMT;
	else
		state->pix.pixelformat = pdata->pixelformat;

	if (!pdata->freq)
		state->freq = DEFUALT_MCLK;	/* 24MHz default */
	else
		state->freq = pdata->freq;

	return 0;
}

static const struct v4l2_subdev_core_ops isx005_core_ops = {
	.init = isx005_init,	/* initializing API */
	.s_config = isx005_s_config,	/* Fetch platform data */
	.queryctrl = isx005_queryctrl,
	.querymenu = isx005_querymenu,
	.g_ctrl = isx005_g_ctrl,
	.s_ctrl = isx005_s_ctrl,
};

static const struct v4l2_subdev_video_ops isx005_video_ops = {
	.s_crystal_freq = isx005_s_crystal_freq,
	.g_fmt = isx005_g_fmt,
	.s_fmt = isx005_s_fmt,
	.enum_framesizes = isx005_enum_framesizes,
	.enum_frameintervals = isx005_enum_frameintervals,
	.enum_fmt = isx005_enum_fmt,
	.try_fmt = isx005_try_fmt,
	.g_parm = isx005_g_parm,
	.s_parm = isx005_s_parm,
};

static const struct v4l2_subdev_ops isx005_ops = {
	.core = &isx005_core_ops,
	.video = &isx005_video_ops,
};

/*
 * isx005_probe
 * Fetching platform data is being done with s_config subdev call.
 * In probe routine, we just register subdev device
 */
static int isx005_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct isx005_state *state;
	struct v4l2_subdev *sd;

	CDBG("probing.................................................... \n");
	
	state = kzalloc(sizeof(struct isx005_state), GFP_KERNEL);
	if (state == NULL)
		return -ENOMEM;

	state->runmode = ISX005_RUNMODE_NOTREADY;

	sd = &state->sd;
	strcpy(sd->name, ISX005_DRIVER_NAME);

	/* Registering subdev */
	v4l2_i2c_subdev_init(sd, client, &isx005_ops);

	init_timer(&af_cancel_timer);
	af_cancel_timer.expires  = (get_jiffies_64() + msecs_to_jiffies(100));
	af_cancel_timer.function = af_cancel_handler;

	isx005_info(&client->dev, "3MP camera ISX005 loaded.\n");

	return 0;
}

static int isx005_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	CDBG("isx005_remove.................................................... \n");

//	isx005_i2c_write_multi(client, 0x0008, 0x0001, 1);	//set standby in the register
//bestiq	msleep(200);

	v4l2_device_unregister_subdev(sd);
	kfree(to_state(sd));

	del_timer(&af_cancel_timer);
	isx005_info(&client->dev, "Unloaded camera sensor ISX005.\n");

	return 0;
}

static const struct i2c_device_id isx005_id[] = {
	{ ISX005_DRIVER_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, isx005_id);

static struct v4l2_i2c_driver_data v4l2_i2c_data = {
	.name = ISX005_DRIVER_NAME,
	.probe = isx005_probe,
	.remove = isx005_remove,
	.id_table = isx005_id,
};

MODULE_DESCRIPTION("NEC ISX005-SONY 3MP camera driver");
MODULE_AUTHOR("Tushar Behera <tushar.b@samsung.com>");
MODULE_LICENSE("GPL");
