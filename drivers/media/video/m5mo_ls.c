/*
 * Driver for Fujitsu M5MO_LS ISP
 *
 * Sensor can be attached up to resolution of 4096*1664
 * Embedded JPEG encoder and object recognition as well
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/firmware.h>	/* For firware update */
#include <linux/syscalls.h>
#include <linux/file.h>

#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-i2c-drv.h>
#include <media/m5mo_ls_platform.h>

#ifdef CONFIG_VIDEO_SAMSUNG_V4L2
#include <linux/videodev2_samsung.h>
#endif

#include <linux/rtc.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <mach/regs-gpio.h>
#include <mach/regs-clock.h>
#include <mach/max8998_function.h>

#include "m5mo_ls.h"

#define M5MO_LS_DRIVER_NAME	"M5MO_LS"

#define FORMAT_FLAGS_COMPRESSED		0x3
#define SENSOR_JPEG_SNAPSHOT_MEMSIZE	0x79EC00  // 7990273 = 3264 x 2448 //0x360000
#define M5MO_LS_THUMB_OFFSET    0x3A0000
#define M5MO_LS_THUMB_MAX_SIZE  0xf000

//#define M5MO_LS_DEBUG
#define M5MO_LS_INFO
#define M5MO_LS_CAM_POWER

#ifdef M5MO_LS_DEBUG
#define m5moLS_msg	dev_err
#else
#define m5moLS_msg 	dev_dbg
#endif

#ifdef M5MO_LS_INFO
#define m5moLS_info	dev_err
#else
#define m5moLS_info	dev_dbg
#endif

#define DBG(x...) 
#define ERR(x...) 

#define M5MO_LS_8BIT         1
#define M5MO_LS_16BIT        2
#define M5MO_LS_32BIT        4

#define ISO_TABLE_MAX 30

/* Wait Queue For Fujitsu ISP Interrupt */
static DECLARE_WAIT_QUEUE_HEAD(cam_wait);
static int fw_update_status = 100;
static int fw_dump_status = 100;
static u8* cam_fw_name = "RS_M5LS.bin";
static u32 cam_fw_version = 0x000A;
static u32 cam_parm_version = 0x5055;

/* MBG SetFlag */
static int cam_interrupted = 0;
static int stop_sleep = 0;
static int m5moLS_debug_flag = 0;
static int camfw_update = 2;

/* Default resolution & pixelformat. plz ref m5moLS_platform.h */
#define DEFAULT_PIX_FMT		V4L2_PIX_FMT_UYVY	/* YUV422 */
#define DEFUALT_MCLK		24000000
#define POLL_TIME_MS		10

#define M5MO_LS_FW_F2_PATH	"/system/firmware/M5MO_LSF02.bin"

#ifdef CONFIG_VIDEO_M5MO_LS
#define IRQ_M5MO_LS_CAM_MEGA_INT (IRQ_EINT_GROUP18_BASE+6) /* J0_6 */
#endif

unsigned char MAIN_SW_FW[4] = {0x0, 0x0, 0x0, 0x0};		/* {FW Maj, FW Min, PRM Maj, PRM Min} */
int MAIN_SW_DATE_INFO[3] = {0x0, 0x0, 0x0};		/* {Year, Month, Date} */

static unsigned int m5moLS_zoom_table[M5MO_LS_ZOOM_LEVEL_MAX] 
      = {  1,  
           2, 3, 5, 7, 9,11,13,15,17,20,
          21,22,23,24,25,26,27,28,29,30,
          31,32,33,34,35,36,37,38,39,40};

static unsigned int iso_reference_table[ISO_TABLE_MAX] = {
    11,     14,     17,     22,     28,     35,     44,     56,     71,     89,
    112,   141,   178,   224,   282,   356,   449,   565,   712,  890,
    1122, 1414, 1782, 2245, 2828, 3564, 4490, 5657, 7127, 8909};

static unsigned int iso_conversion_table[ISO_TABLE_MAX] = {
    10, 12, 16, 20, 25, 32, 40, 50, 64, 80,
    100, 125, 160, 200, 250, 320, 400, 500, 640, 800,
    1000, 1250, 1600, 2000, 2500, 3200, 4000, 5000, 6400, 8000};

static int DZoom_State = 0;
static int Flash_Mode = 0; //SecFeature.Camera aswoogi

static u32 main_jpeg_size = 0;
static u32 thumb_jpeg_size = 0;

extern unsigned int HWREV;

static struct device_attribute *dev_attr_m5moLS_debug = NULL;
enum m5moLS_oprmode {
	M5MO_LS_OPRMODE_VIDEO = 0,
	M5MO_LS_OPRMODE_IMAGE = 1,
};

/* Declare Funtion */
static int m5moLS_set_awb_lock(struct v4l2_subdev *sd, int lock);
static int m5moLS_set_ae_lock(struct v4l2_subdev *sd, int lock);
static int m5moLS_set_iso(struct v4l2_subdev * sd, struct v4l2_control * ctrl);
static int m5moLS_set_metering(struct v4l2_subdev * sd, struct v4l2_control * ctrl);
static int m5moLS_set_ev(struct v4l2_subdev * sd, struct v4l2_control * ctrl);
static int m5moLS_set_slow_ae(struct v4l2_subdev * sd, struct v4l2_control * ctrl);
static int m5moLS_set_gamma(struct v4l2_subdev * sd, struct v4l2_control * ctrl);
static int m5moLS_set_effect(struct v4l2_subdev * sd, struct v4l2_control * ctrl);
static int m5moLS_set_white_balance(struct v4l2_subdev * sd, struct v4l2_control * ctrl);
static int m5moLS_wait_interrupt(struct v4l2_subdev *sd, int interrupt, int timeout);
static int m5moLS_i2c_update_firmware(struct v4l2_subdev *sd);
static int m5moLS_init(struct v4l2_subdev *sd, u32 val);
static int m5moLS_enable_interrupt_pin(struct v4l2_subdev *sd);
static void m5moLS_disable_interrupt_pin(struct v4l2_subdev *sd);
static int m5moLS_detect(struct v4l2_subdev *sd);
static int m5moLS_set_vintage_mode(struct v4l2_subdev *sd);
static int m5moLS_set_face_beauty(struct v4l2_subdev *sd);


enum m5moLS_frame_size {
	M5MO_LS_PREVIEW_QCIF = 0,
	M5MO_LS_PREVIEW_QVGA,
	M5MO_LS_PREVIEW_592x480,
	M5MO_LS_PREVIEW_VGA,
	M5MO_LS_PREVIEW_D1, /* 720x480 */
	M5MO_LS_PREVIEW_WVGA,
	M5MO_LS_PREVIEW_720P,
	M5MO_LS_PREVIEW_VERTICAL_QCIF,
	
	M5MO_LS_CAPTURE_VGA, /* 640 x 480 */	
	M5MO_LS_CAPTURE_WVGA, /* 800 x 480 */
	M5MO_LS_CAPTURE_W1_5MP, /* 1600 x 960 */
	M5MO_LS_CAPTURE_2MP, /* 1600 x 1200 */
	M5MO_LS_CAPTURE_W4MP, /* 2560 x 1536 */
	M5MO_LS_CAPTURE_5MP,    /* 2560 x 1920 */
	M5MO_LS_CAPTURE_W6MP, /* 3264 x 1960 */
	M5MO_LS_CAPTURE_8MP, /* 3264 x 2448 */
};

enum M5MO_LS_CMD{
        M5MO_LS_CAM_START,
        M5MO_LS_INT_STATUS_MODE,
        M5MO_LS_MONITOR_SIZE,
        M5MO_LS_MONITOR_ROTATION,
        M5MO_LS_INT_EN_MODE,
        M5MO_LS_SYS_MODE,
        M5MO_LS_CAP_MODE,
        M5MO_LS_WDR_LVL,
        M5MO_LS_CAP_TRANSFER_START,
        M5MO_LS_INFO_EXPTIME,
        M5MO_LS_CMD_MAX
};

unsigned char m5moLS_test_buf[2] = {0x12,0x01};
unsigned int m5moLS_test_len = 2;


struct m5moLS_i2c_data{
        unsigned int category;
        unsigned int addr;
        unsigned int data;
};

struct m5moLS_enum_framesize {
	/* mode is 0 for preview, 1 for capture */
	enum m5moLS_oprmode mode;
	unsigned int index;
	unsigned int width;
	unsigned int height;	
};

typedef struct{
    char dummy;
    char module_company;
    char module_version;
    char year;
    char month;
    char update_times[2];
    unsigned int macro_af;
    unsigned int infinite_af;
}m5moLS_fw_info_t;

static struct m5moLS_enum_framesize m5moLS_framesize_list[] = {
	{ M5MO_LS_OPRMODE_VIDEO, M5MO_LS_PREVIEW_QCIF,       176,  144 },
	{ M5MO_LS_OPRMODE_VIDEO, M5MO_LS_PREVIEW_QVGA,       320,  240 },
	{ M5MO_LS_OPRMODE_VIDEO, M5MO_LS_PREVIEW_592x480,    592,  480 },
	{ M5MO_LS_OPRMODE_VIDEO, M5MO_LS_PREVIEW_VGA,		 640,  480 },
	{ M5MO_LS_OPRMODE_VIDEO, M5MO_LS_PREVIEW_D1,         720,  480 },
	{ M5MO_LS_OPRMODE_VIDEO, M5MO_LS_PREVIEW_WVGA,       800,  480 },
	{ M5MO_LS_OPRMODE_VIDEO, M5MO_LS_PREVIEW_720P,      1280,  720 },
	{ M5MO_LS_OPRMODE_VIDEO, M5MO_LS_PREVIEW_VERTICAL_QCIF,	144,	176},
	{ M5MO_LS_OPRMODE_IMAGE, M5MO_LS_CAPTURE_VGA,		 640,  480 },
	{ M5MO_LS_OPRMODE_IMAGE, M5MO_LS_CAPTURE_WVGA,		 800,  480 },
	{ M5MO_LS_OPRMODE_IMAGE, M5MO_LS_CAPTURE_W1_5MP,   	1600,  960 },
	{ M5MO_LS_OPRMODE_IMAGE, M5MO_LS_CAPTURE_2MP,       1600, 1200 },
	{ M5MO_LS_OPRMODE_IMAGE, M5MO_LS_CAPTURE_W4MP,		2560, 1536 },
	{ M5MO_LS_OPRMODE_IMAGE, M5MO_LS_CAPTURE_5MP,       2560, 1920 },
	{ M5MO_LS_OPRMODE_IMAGE, M5MO_LS_CAPTURE_W6MP,      3264, 1960 },
	{ M5MO_LS_OPRMODE_IMAGE, M5MO_LS_CAPTURE_8MP,       3264, 2448 },
};

struct m5moLS_version {
	unsigned int major;
	unsigned int minor;
};

struct m5moLS_date_info {
	unsigned int year;
	unsigned int month;
	unsigned int date;
};

enum m5moLS_runmode {
	M5MO_LS_RUNMODE_NOTREADY,
	M5MO_LS_RUNMODE_IDLE, 
	M5MO_LS_RUNMODE_READY,
	M5MO_LS_RUNMODE_RUNNING, 
};

struct m5moLS_firmware {
	unsigned int addr;
	unsigned int size;
};

/* Camera functional setting values configured by user concept */
struct m5moLS_jpeg_param {
	unsigned int enable;
	unsigned int quality;
	unsigned int main_size;  /* Main JPEG file size */
	unsigned int thumb_size; /* Thumbnail file size */
	unsigned int main_offset;
	unsigned int thumb_offset;
	unsigned int postview_offset;
} ; 

struct m5moLS_position {
	int x;
	int y;
} ; 

struct m5moLS_gps_info{
	unsigned char m5moLS_gps_buf[8];
	unsigned char m5moLS_altitude_buf[4];
	long gps_timeStamp;
};

struct m5moLS_sensor_maker{
	unsigned int maker;
	unsigned int optical;
};

struct m5moLS_version_af{
	unsigned int low;
	unsigned int high;
};

struct m5moLS_gamma{
	unsigned int rg_low;
	unsigned int rg_high;
	unsigned int bg_low;
	unsigned int bg_high;	
};

struct gps_info_common {
	unsigned int 	direction;
	unsigned int 	dgree;
	unsigned int	minute;
	unsigned int	second;
};

struct m5moLS_exif_info {
	unsigned int 	info_exptime_numer;
	unsigned int 	info_exptime_denumer;
	unsigned int 	info_tv_numer;
	unsigned int 	info_tv_denumer;
	unsigned int	info_av_numer;
	unsigned int	info_av_denumer;
	unsigned int	info_bv_numer;
	unsigned int	info_bv_denumer;
	unsigned int 	info_ebv_numer;
	unsigned int 	info_ebv_denumer;
	unsigned int	info_iso;
	unsigned int	info_flash;
	unsigned int	info_sdr;
	unsigned int	info_qval;	
};

struct m5moLS_state {
	struct m5moLS_platform_data *pdata;
	struct v4l2_subdev sd;
	struct v4l2_pix_format pix;
	struct v4l2_fract timeperframe;
	struct m5moLS_userset userset;
	struct m5moLS_jpeg_param jpeg;
	struct m5moLS_version fw;
	struct m5moLS_version prm;
	struct m5moLS_date_info dateinfo;
	struct m5moLS_firmware fw_info;
	struct m5moLS_position position;
	struct m5moLS_sensor_maker sensor_info;	
	struct m5moLS_version_af af_info;
	struct m5moLS_gamma gamma;
	struct v4l2_streamparm strm;
	struct m5moLS_gps_info gpsInfo;
	enum m5moLS_runmode runmode;
	enum m5moLS_oprmode oprmode;
	int framesize_index;
	int sensor_version;
	int freq;	/* MCLK in Hz */
	int fps;
	int ot_status;
	int sa_status;
	int anti_banding;	
	int preview_size;
	unsigned int fw_dump_size;
	int hd_preview_on;
	int pre_af_status;
	int cur_af_status;
       u8 customer_ver;
       u8 project_ver;
       u8 camera_fw_info[25];
       u8 vintage_mode;
       u8 beauty_mode;
       u8 touch_af_mode;
       u8 m_contrast_mode;
       u8 m_sharpness_mode;
       u8 m_wdr_mode;
       u8 m_anti_shake_mode;
       u8 m_white_balance_mode;
       u8 m_anti_banding_mode;
       u8 m_ev_mode;
       u8 m_iso_mode;
       u8 m_effect_mode;
       u8 m_metering_mode;
       u8 m_zoom_level;
       u8 m_movie_mode;
       u8 m_saturation_mode;
       u8 m_ev_program_mode;
       u8 m_mcc_mode;
       u8 m_focus_mode;
       u16 hw_ver;
       u16 prm_ver;
       u16 fw_ver;
       u16 mac_af_ver;
       u16 Inf_af_ver;
	struct m5moLS_version main_sw_fw;
	struct m5moLS_version main_sw_prm;
	struct m5moLS_date_info main_sw_dateinfo;
	int exif_orientation_info;
	int check_dataline;
	int hd_slow_ae;
	int hd_gamma;
	int iso;
	int metering;
	int ev;
	int effect;
	int wb;
       int irq;
       int not_detect;
	struct tm *exifTimeInfo;
	struct m5moLS_exif_info exif_info;

	struct work_struct m5moLS_firmup_work;
	struct work_struct m5moLS_firmdump_work;    
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

static inline struct m5moLS_state *to_state(struct v4l2_subdev *sd)
{
	return container_of(sd, struct m5moLS_state, sd);
}

static int 
m5moLS_read_mem(struct i2c_client *client, u16 data_length, u32 reg, u8 *val)
{
	struct i2c_msg msg[1];
	unsigned char data[8];
	unsigned char recv_data[data_length + 3];
	int err;

	if (!client->adapter)
		return -ENODEV;
	
	if (data_length < 1)
	{
		ERR("Data Length to read is out of range !\n");
		return -EINVAL;
	}
	
	msg->addr = client->addr;
	msg->flags = 0;
	msg->len = 8;
	msg->buf = data;

	/* high byte goes out first */
	data[0] = 0x00;
	data[1] = 0x03;
	data[2] = (u8) (reg >> 24);
	data[3] = (u8) ( (reg >> 16) & 0xff );
	data[4] = (u8) ( (reg >> 8) & 0xff );
	data[5] = (u8) (reg & 0xff);
	data[6] = (u8) (data_length >> 8);
	data[7] = (u8) (data_length & 0xff);
	
	err = i2c_transfer(client->adapter, msg, 1);
	if (err >= 0) 
	{
		msg->flags = I2C_M_RD;
		msg->len = data_length + 3;
		msg->buf = recv_data; 

		err = i2c_transfer(client->adapter, msg, 1);
	}
	if (err >= 0) 
	{
		memcpy(val, recv_data+3, data_length);
		return 0;
	}

	ERR( "read from offset 0x%x error %d", reg, err);
	return err;
}

static int 
m5moLS_write_mem(struct i2c_client *client, u16 data_length, u32 reg, u8* val)
{
	struct i2c_msg msg[1];
	u8 *data;
	int retry = 0, err;

	if (!client->adapter)
		return -ENODEV;

	data = kmalloc(8 + data_length, GFP_KERNEL);

again:
	msg->addr = client->addr;
	msg->flags = 0;
	msg->len = data_length + 8;
	msg->buf = data;

	/* high byte goes out first */
	data[0] = 0x00;
	data[1] = 0x04;
	data[2] = (u8) (reg >> 24);
	data[3] = (u8) ( (reg >> 16) & 0xff );
	data[4] = (u8) ( (reg >> 8) & 0xff );
	data[5] = (u8) (reg & 0xff);
	data[6] = (u8) (data_length >> 8);
	data[7] = (u8) (data_length & 0xff);
	memcpy(data+8 , val, data_length);

	err = i2c_transfer(client->adapter, msg, 1);
	if (err >= 0)
	{
		kfree(data);
		return 0;
	}

	ERR( "wrote to offset 0x%x error %d", reg, err);
	if (retry <= M5MO_LS_I2C_RETRY) 
	{
		ERR( "retry ... %d", retry);
		retry++;
		set_current_state(TASK_UNINTERRUPTIBLE);
		schedule_timeout(msecs_to_jiffies(20));
		goto again;
	}

	kfree(data);
	return err;
}


static int 
m5moLS_read_category_parm(struct i2c_client *client, 
								u8 data_length, u8 category, u8 byte, u32* val)
{
	struct i2c_msg msg[1];
	unsigned char data[5];
	unsigned char recv_data[data_length + 1];
	int err, retry=0;

	if (!client->adapter)
		return -ENODEV;
	if (data_length != M5MO_LS_8BIT && data_length != M5MO_LS_16BIT	&& data_length != M5MO_LS_32BIT)		
		return -EINVAL;

read_again:
	msg->addr = client->addr;
	msg->flags = 0;
	msg->len = 5;
	msg->buf = data;

	/* high byte goes out first */
	data[0] = 0x05;
	data[1] = 0x01;
	data[2] = category;
	data[3] = byte;
	data[4] = data_length;
	
	err = i2c_transfer(client->adapter, msg, 1);
	if (err >= 0) 
	{
		msg->flags = I2C_M_RD;
		msg->len = data_length + 1;
		msg->buf = recv_data; 
		
		err = i2c_transfer(client->adapter, msg, 1);
	}
	if (err >= 0) 
	{
		/* high byte comes first */		
		if (data_length == M5MO_LS_8BIT)			
			*val = recv_data[1];		
		else if (data_length == M5MO_LS_16BIT)			
			*val = recv_data[2] + (recv_data[1] << 8)+(recv_data[0]<<16);		
		else			
			*val = recv_data[4] + (recv_data[3] << 8) + (recv_data[2] << 16) + (recv_data[1] << 24);

		m5moLS_msg(&client->dev, "read value from [category:0x%x], [Byte:0x%x] is 0x%x\n", category, byte, *val);
		//print_currenttime();
		
		return 0;
	}

	m5moLS_msg(&client->dev, "read from offset [category:0x%x], [Byte:0x%x] error %d\n", category, byte, err);
	if (retry <= M5MO_LS_I2C_RETRY) 
	{
		m5moLS_msg(&client->dev,"retry ... %d\n", retry);
		retry++;
		set_current_state(TASK_UNINTERRUPTIBLE);
		schedule_timeout(msecs_to_jiffies(20));
		goto read_again;
	}

	return err;
}

static int 
m5moLS_write_category_parm(struct i2c_client *client,  
										u8 data_length, u8 category, u8 byte, u32 val)
{
	struct i2c_msg msg[1];
	unsigned char data[data_length+4];
	int retry = 0, err;

	if (!client->adapter)
		return -ENODEV;

	if (data_length != M5MO_LS_8BIT && data_length != M5MO_LS_16BIT && data_length != M5MO_LS_32BIT)
		return -EINVAL; 

again:
	m5moLS_msg(&client->dev, "write value from [category:0x%x], [Byte:0x%x] is value:0x%x\n", category, byte, val);
	//print_currenttime();
	m5moLS_msg(&client->dev, "write value from [address:0x%x]\n", client->addr);
	msg->addr = client->addr;
	msg->flags = 0;
	msg->len = data_length + 4;
	msg->buf = data;

	/* high byte goes out first */
	data[0] = data_length + 4;
	data[1] = 0x02;
	data[2] = category;
	data[3] = byte;
	if (data_length == M5MO_LS_8BIT)
		data[4] = (u8) (val & 0xff);
	else if (data_length == M5MO_LS_16BIT) 
	{
		data[4] = (u8) (val >> 8);
		data[5] = (u8) (val & 0xff);
	} 
	else 
	{
		data[4] = (u8) (val >> 24);
		data[5] = (u8) ( (val >> 16) & 0xff );
		data[6] = (u8) ( (val >> 8) & 0xff );
		data[7] = (u8) (val & 0xff);
	}
	
	err = i2c_transfer(client->adapter, msg, 1);
	if (err >= 0)
		return 0;

	m5moLS_msg(&client->dev,"wrote 0x%x to offset [category:0x%x], [Byte:0x%x] error %d", val, category, byte, err);
	if (retry <= M5MO_LS_I2C_RETRY) 
	{
		m5moLS_msg(&client->dev,"retry ... %d", retry);
		retry++;
		set_current_state(TASK_UNINTERRUPTIBLE);
		schedule_timeout(msecs_to_jiffies(20));
		goto again;
	}
	return err;
}

int m5moLS_write_fw_status(u8* status)
{
	struct file *file = NULL;
	int fd;
	loff_t pos = 0;
	mm_segment_t old_fs = get_fs ();

	int err = 0;

	set_fs (KERNEL_DS);

	fd = sys_open ("/mnt/.lfs/cam_fw_status", O_WRONLY | O_CREAT | O_TRUNC , 0644);
	if(fd < 0 )
	{
		err = -EFAULT;
		goto STATUS_ERR;
	}
	file = fget (fd);
	if(!file)
	{
		err = -EFAULT;
		goto STATUS_ERR;
	}
		
	vfs_write (file, status, 1, &pos);

STATUS_ERR:
	if(file)
		fput (file);
	if(fd)
		sys_close (fd);

	set_fs (old_fs);

	return err;
}

#ifdef M5MO_LS_CAM_POWER
static void m5moLS_ldo_en(bool onoff)
{
        int err;

        printk(KERN_ERR "m5moLS_ldo_en\n");        

	 printk("m5moLS_ldo_en\n"); 
		

        //For Emul Rev0.1
        // Because B4, B5 do not use this GPIO, this GPIO is enabled in all HW version
        /* CAM_IO_EN - GPB(7) */
#if 0        
        err = gpio_request(GPIO_CAM_IO_EN, "CAM_IO_EN");

        if(err) {
                printk(KERN_ERR "failed to request GPB7 for camera control\n");

                return err;
        }
#endif
        if(onoff == TRUE) { //power on 

            /* 1. CAM_VDIG 1.2V(SENSOR DIGITAL POWER)
                         2. CAM_I_CORE 1.2V
                         3. CAM_VANA_2.8V(SENSOR ANALOG POWER)
                         4. CAM_VIF 1.8V(SENSOR I/O POWER)
                         5. CAM_I_HOST 1.8V(I/O POWER SUPPLY FOR YUV I/F)
                     */
            /* STEALTH REV0.1
                        LDO11 = CAM_AF_2.8V
                        LDO12 = CAM_SENSOR_CORE_1.2V
                        LDO13 = CAM_SENSOR_A2.8V
                        LDO14 = CAM_DVDD_1.5V
                        LDO15 = CAM_HOST_1.8V
                        LDO16 = CAM_SENSOR_IO_1.8V
                        LX4(BUCK4)  = CAM_ISP_CORE_1.2V
                        ON SEQUENCE : LDO12->LX4->LDO13->LDO14->LDO15->LDO16->LDO11
                    */
            
            Set_MAX8998_PM_OUTPUT_Voltage(LDO12, VCC_1p300);
            Set_MAX8998_PM_REG(ELDO12, 1);

            Set_MAX8998_PM_OUTPUT_Voltage(BUCK4, VCC_1p300);
            Set_MAX8998_PM_REG(EN4, 1);

            if(HWREV>=8)
            {
                if(gpio_request(GPIO_CAM_IO_EN, "GPB") != 0)
                {
                        printk(KERN_ERR "failed to request GPB7 for camera control = %d\n",HWREV);
                        return err;
                }
                gpio_direction_output(GPIO_CAM_IO_EN, 0);
                gpio_set_value(GPIO_CAM_IO_EN, 1);                
            }
            else
            {
                Set_MAX8998_PM_OUTPUT_Voltage(LDO13, VCC_2p800);
                Set_MAX8998_PM_REG(ELDO13, 1);
            }
            
            Set_MAX8998_PM_OUTPUT_Voltage(LDO14, VCC_1p500);
            Set_MAX8998_PM_REG(ELDO14, 1);

            Set_MAX8998_PM_OUTPUT_Voltage(LDO15, VCC_1p800);
            Set_MAX8998_PM_REG(ELDO15, 1);

            Set_MAX8998_PM_OUTPUT_Voltage(LDO16, VCC_1p800);
            Set_MAX8998_PM_REG(ELDO16, 1);

            Set_MAX8998_PM_OUTPUT_Voltage(LDO11, VCC_2p800);
            Set_MAX8998_PM_REG(ELDO11, 1);
        }
        else { // power off
            /*
                    1. CAM_I_HOST 1.8V
                    2. CAM_VIF 1.8V
                    3. CAM_VANA_2.8V
                    4. CAM_I_CORE_1.2V
                    5. CAM_VDIG 1.2V
                    */
            //OFF SEQUENCE : LDO11->LDO15->LDO16->LDO13->LDO14->LX4->LDO12

            Set_MAX8998_PM_REG(ELDO11, 0);

            Set_MAX8998_PM_REG(ELDO15, 0);

            Set_MAX8998_PM_REG(ELDO16, 0);

            if(HWREV>=8)
            {
                if(gpio_request(GPIO_CAM_IO_EN, "GPB") != 0)
                {
                        printk(KERN_ERR "failed to request GPB7 for camera control = %d\n",HWREV);
                        return err;
                }            
                gpio_direction_output(GPIO_CAM_IO_EN, 1);    
                gpio_set_value(GPIO_CAM_IO_EN, 0);
            }
            else
            {
                Set_MAX8998_PM_REG(ELDO13, 0);
            }
            
            Set_MAX8998_PM_REG(ELDO14, 0);                

            Set_MAX8998_PM_REG(EN4, 0);

            Set_MAX8998_PM_REG(ELDO12, 0);
        }

        if(HWREV >= 8)
            gpio_free(GPIO_CAM_IO_EN);

};

static int m5moLS_power_on(struct v4l2_subdev *sd)
{   
    int err, err2, err3, err4;

    printk("m5moLS_power_on::::: Camera Driver\n");

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

    /* GPIO_CAM_VT_RST_28V - GPJ4(1) */
    err = gpio_request(GPIO_CAM_VT_RST_28V, "GPJ4");
    if(err) {
        printk(KERN_ERR "failed to request GPJ1 for camera control\n");
        return err;
    }
    // Power On Sequence

    /* CAM_ISP_CORE_1.2V */
    Set_MAX8998_PM_OUTPUT_Voltage(BUCK4, VCC_1p300);
    Set_MAX8998_PM_REG(EN4, 1);

    udelay(20);
    
    /* CAM_SENSOR_CORE_1.2V */
    Set_MAX8998_PM_OUTPUT_Voltage(LDO12, VCC_1p300);
    Set_MAX8998_PM_REG(ELDO12, 1);

    udelay(150);
    
    /* CAM_1.3M_nSTBY  HIGH */
    gpio_direction_output(GPIO_CAM_VT_EN_28V, 1);
    gpio_set_value(GPIO_CAM_VT_EN_28V, 1);

    udelay(20);

    /* CAM_DVDD_1.5V */
    Set_MAX8998_PM_OUTPUT_Voltage(LDO14, VCC_1p500);
    Set_MAX8998_PM_REG(ELDO14, 1);    

    udelay(20);
    
    /* CAM_SENSOR_A2.8V */
    if(HWREV>=8)
    {
        if(gpio_request(GPIO_CAM_IO_EN, "GPB") != 0)
        {
                printk(KERN_ERR "failed to request GPB7 for camera control = %d\n",HWREV);
                return err;
        }
        gpio_direction_output(GPIO_CAM_IO_EN, 0);
        gpio_set_value(GPIO_CAM_IO_EN, 1);                
    }
    else
    {
        Set_MAX8998_PM_OUTPUT_Voltage(LDO13, VCC_2p800);
        Set_MAX8998_PM_REG(ELDO13, 1);
    }

    /* CAM_HOST_1.8V */
    Set_MAX8998_PM_OUTPUT_Voltage(LDO15, VCC_1p800);
    Set_MAX8998_PM_REG(ELDO15, 1);

    udelay(20);

    /* CAM_SENSOR_IO_1.8V */
    Set_MAX8998_PM_OUTPUT_Voltage(LDO16, VCC_1p800);
    Set_MAX8998_PM_REG(ELDO16, 1);

    udelay(100);
    
    // Mclk enable
    s3c_gpio_cfgpin(GPIO_CAM_MCLK, S5PV210_GPE1_3_CAM_A_CLKOUT);

    mdelay(1);

    /* CAM_AF_2.8V */
    Set_MAX8998_PM_OUTPUT_Voltage(LDO11, VCC_2p800);
    Set_MAX8998_PM_REG(ELDO11, 1);
            
    // CAM_VGA_nRST  HIGH       
    gpio_direction_output(GPIO_CAM_VT_RST_28V, 0);
    gpio_set_value(GPIO_CAM_VT_RST_28V, 1);

    mdelay(85);

    // CAM_1.3M_nSTBY  LOW  
    gpio_direction_output(GPIO_CAM_VT_EN_28V, 1);
    gpio_set_value(GPIO_CAM_VT_EN_28V, 0);

    udelay(20);
       
    // CAM_MEGA_nRST HIGH
    gpio_direction_output(GPIO_CAM_MEGA_nRST, 0);
    gpio_set_value(GPIO_CAM_MEGA_nRST, 1);    
    

    gpio_free(GPIO_CAM_MEGA_nRST);

    gpio_free(GPIO_CAM_VT_EN_28V);

    gpio_free(GPIO_CAM_VT_RST_28V); 

    if(HWREV >= 8)
        gpio_free(GPIO_CAM_IO_EN);

    mdelay(5);

    return 0;
}


/** 
 * m5moLS_power_off: Power off the camera sensor 
 * Use only for updating camera firmware
 * Returns 0 forever
 */
static int m5moLS_power_off(struct v4l2_subdev *sd)
{
    int err;

    printk(KERN_ERR "m5moLS_power_off\n");

    /* CAM_MEGA_nRST - GPJ1(5) */
    err = gpio_request(GPIO_CAM_MEGA_nRST, "GPJ1");
    if(err) {
        printk(KERN_ERR "failed to request GPJ1 for camera control\n");
        return err;
    }

    /* GPIO_CAM_VT_RST_28V - GPJ4(1) */
    err = gpio_request(GPIO_CAM_VT_RST_28V, "GPJ4");
    if(err) {
        printk(KERN_ERR "failed to request GPJ1 for camera control\n");
        return err;
    }

    // Power Off Sequence
    mdelay(5);

    // CAM_MEGA_nRST - GPJ1(5) LOW
    gpio_direction_output(GPIO_CAM_MEGA_nRST, 1);
    gpio_set_value(GPIO_CAM_MEGA_nRST, 0);

    // CAM_VGA_nRST  LOW
    gpio_direction_output(GPIO_CAM_VT_RST_28V, 0);
    gpio_set_value(GPIO_CAM_VT_RST_28V, 0);

    /* CAM_AF_2.8V */
    Set_MAX8998_PM_OUTPUT_Voltage(LDO11, VCC_2p800);
    Set_MAX8998_PM_REG(ELDO11, 0);

    mdelay(1);
    
    // Mclk disable
    s3c_gpio_cfgpin(GPIO_CAM_MCLK, 0);

    /* CAM_SENSOR_IO_1.8V */
    Set_MAX8998_PM_OUTPUT_Voltage(LDO16, VCC_1p800);
    Set_MAX8998_PM_REG(ELDO16, 0);    

     /* CAM_HOST_1.8V */
     Set_MAX8998_PM_OUTPUT_Voltage(LDO15, VCC_1p800);
     Set_MAX8998_PM_REG(ELDO15, 0);

     /* CAM_SENSOR_A2.8V */
    if(HWREV>=8)
    {
        if(gpio_request(GPIO_CAM_IO_EN, "GPB") != 0)
        {
                printk(KERN_ERR "failed to request GPB7 for camera control = %d\n",HWREV);
                return err;
        }            
        gpio_direction_output(GPIO_CAM_IO_EN, 1);    
        gpio_set_value(GPIO_CAM_IO_EN, 0);
    }
    else
    {
        Set_MAX8998_PM_REG(ELDO13, 0);
    }    

    /* CAM_DVDD_1.5V */
    Set_MAX8998_PM_OUTPUT_Voltage(LDO14, VCC_1p500);
    Set_MAX8998_PM_REG(ELDO14, 0);

    /* CAM_SENSOR_CORE_1.2V */
    Set_MAX8998_PM_OUTPUT_Voltage(LDO12, VCC_1p300);
    Set_MAX8998_PM_REG(ELDO12, 0);

    /* CAM_ISP_CORE_1.2V */
    Set_MAX8998_PM_OUTPUT_Voltage(BUCK4, VCC_1p300);
    Set_MAX8998_PM_REG(EN4, 0);    
    

    gpio_free(GPIO_CAM_MEGA_nRST);
    
    gpio_free(GPIO_CAM_VT_RST_28V);

    if(HWREV >= 8)
        gpio_free(GPIO_CAM_IO_EN);

    return 0;
}



/** 
 * m5moLS_power_en: Enable or disable the camera sensor power 
 * Use only for updating camera firmware
 * Returns 0 forever
 */
static int m5moLS_power_en(struct v4l2_subdev *sd,int onoff)
{
	if(onoff == 1)
	{
		m5moLS_power_on(sd);
	}

	else 
	{
		m5moLS_power_off(sd);
	}
	return 0;
}

#endif //ifdef M5MO_LS_CAM_POWER

static int 
m5moLS_i2c_verify(struct i2c_client *client, u8 category, u8 byte, u32 value)
{
	u32 val = 0, i;

	for(i= 0; i < M5MO_LS_I2C_VERIFY_RETRY; i++) 
	{
		m5moLS_read_category_parm(client, M5MO_LS_8BIT, category, byte, &val);
		msleep(1);
		if(val == value) 
		{
			m5moLS_msg(&client->dev, "[m5moLS] i2c verified [c,b,v] [%02x, %02x, %02x] (try = %d)\n", category, byte, value, i);
			return 0;
		}
	}

	return -EBUSY;
}


static int 
m5moLS_get_mode(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	
	int err, mode;
	u32 val;

	err = m5moLS_read_category_parm(client, M5MO_LS_8BIT, 0x00, 0x0B, &val);
	if(err)
	{
		m5moLS_msg(&client->dev,  "Can not read I2C command\n");
		return -EPERM;
	}

	mode = val;

	return mode;
}

static int 
m5moLS_set_mode_common(struct v4l2_subdev *sd, u32 mode )
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	
	int err = 0;

	m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x00, 0x0B, mode);

	if(mode != M5MO_LS_MONITOR_MODE)
	{
		err = m5moLS_i2c_verify(client, 0x00, 0x0B, mode);
		if(err)
		{
			m5moLS_msg(&client->dev,  "Can not Set Sensor Mode\n");
			return err;
		}
	}

	return err;
}


static int 
m5moLS_set_mode(struct v4l2_subdev *sd, u32 mode )
{	
	int org_value;
	int err = -EINVAL;
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    
	org_value = m5moLS_get_mode(sd);

    m5moLS_msg(&client->dev, "%s ::: get_mode = %d\n", __func__, org_value);

    
	switch(org_value)
	{
		case M5MO_LS_PARMSET_MODE :
			if(mode == M5MO_LS_PARMSET_MODE)
			{
				return 0;
			}
			else if(mode == M5MO_LS_MONITOR_MODE)
			{
				if((err = m5moLS_set_mode_common(sd, M5MO_LS_MONITOR_MODE)))
					return err;
			}
			else if(mode == M5MO_LS_STILLCAP_MODE)
			{
				if((err = m5moLS_set_mode_common(sd, M5MO_LS_MONITOR_MODE)))
					return err;
				if((err = m5moLS_set_mode_common(sd, M5MO_LS_STILLCAP_MODE)))
					return err;
			}
			else
			{
				ERR( "Requested Sensor Mode is incorrect!\n");
				return -EINVAL;
			}

			break;

		case M5MO_LS_MONITOR_MODE :
			if(mode == M5MO_LS_PARMSET_MODE)
			{
				if((err = m5moLS_set_mode_common(sd, M5MO_LS_PARMSET_MODE)))
					return err;
			}
			else if(mode == M5MO_LS_MONITOR_MODE)
			{
				return 0;
			}
			else if(mode == M5MO_LS_STILLCAP_MODE)
			{
				if((err = m5moLS_set_mode_common(sd, M5MO_LS_STILLCAP_MODE)))
					return err;
			}
			else
			{
				ERR( "Requested Sensor Mode is incorrect!\n");
				return -EINVAL;
			}

			break;

		case M5MO_LS_STILLCAP_MODE :
			if(mode == M5MO_LS_PARMSET_MODE)
			{
				if((err = m5moLS_set_mode_common(sd, M5MO_LS_MONITOR_MODE)))
					return err;
				if((err = m5moLS_set_mode_common(sd, M5MO_LS_PARMSET_MODE)))
					return err;
			}
			else if(mode == M5MO_LS_MONITOR_MODE)
			{
				if((err = m5moLS_set_mode_common(sd, M5MO_LS_MONITOR_MODE)))
					return err;
			}
			else if(mode == M5MO_LS_STILLCAP_MODE)
			{
				return 0;
			}
			else
			{
				ERR( "Requested Sensor Mode is incorrect!\n");
				return -EINVAL;
			}
			break;
            
                default:
                       break;
	}
	
	return err;
}

static int
m5moLS_get_version_in_flash(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	int i, j, flash_addr, err = 0;
	u8 readdata[40];
	u8 pin1 = 0x7E;
    
       m5moLS_msg(&client->dev,"%s \n",__func__);		
	/* Set M-5MoLS Pins */
	m5moLS_write_mem(client, 0x0001, 0x50000308, &pin1);

	flash_addr = 0x1016FF00;	
	if(m5moLS_read_mem(client, 20, flash_addr, &readdata) < 0)
       {
            m5moLS_msg(&client->dev,"read_mem failed!!\n");
            return -1;
       }
       readdata[40] = NULL;
       m5moLS_msg(&client->dev,"%s :: DATA = %s\n",__func__, readdata);
	return err;

}

static int
m5moLS_get_version(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct m5moLS_state *state = to_state(sd);

    u8 err = 0, temp_8=0,i=0,temp_af=0;
    u16 temp_16=0;
    u8 FW_INFO_VER[26];
    
    m5moLS_fw_info_t module_fw_info;

    if(copy_from_user(&module_fw_info, (void *)ctrl->value, sizeof(m5moLS_fw_info_t) ) != 0)
        return -1;

    if(state->not_detect != 1)
    {
        err = m5moLS_detect(sd);
        if(err)
            return err;
    }
    
    /*Macro AF Version*/
    temp_16 = 0;
    m5moLS_read_category_parm(client, M5MO_LS_16BIT, 0x0A, 0x22, &temp_16);
    state->mac_af_ver = (temp_af & 0x02ff);
    /*Infinite AF Version*/
    temp_16 = 0;
    m5moLS_read_category_parm(client, M5MO_LS_16BIT, 0x0A, 0x24, &temp_16);
    state->Inf_af_ver = (temp_af & 0x02ff);

    if(state->not_detect != 1)
    {
        for(i=0;i<30;i++)
        {
            m5moLS_read_category_parm(client, M5MO_LS_8BIT, 0x00, 0x0A, &temp_8);
            state->camera_fw_info[i] = temp_8;
            if(state->camera_fw_info[i] == NULL)
            {
                break;
            }
            m5moLS_msg(&client->dev, "FW_INFO_VER[%d] = 0x%02x[%c]\n", i,state->camera_fw_info[i],state->camera_fw_info[i]);
            temp_8=0;
        }
    }

    module_fw_info.dummy                 = 'F';
    module_fw_info.module_company   = state->camera_fw_info[0];
    module_fw_info.module_version      = state->camera_fw_info[1];
    module_fw_info.year                       = state->camera_fw_info[2];
    module_fw_info.month                    = state->camera_fw_info[3];
    module_fw_info.update_times[0]      = state->camera_fw_info[4];
    module_fw_info.update_times[1]      = state->camera_fw_info[5];
    module_fw_info.macro_af                = state->mac_af_ver;
    module_fw_info.infinite_af                = state->Inf_af_ver;

    m5moLS_msg(&client->dev, "Customer Version: 0x%04x\n", state->customer_ver);
    m5moLS_msg(&client->dev, "Project Version: 0x%04x\n", state->project_ver);
    m5moLS_msg(&client->dev, "Firmware Version: 0x%04x\n", state->fw_ver);
    m5moLS_msg(&client->dev, "Hardware Version: 0x%04x\n", state->hw_ver);
    m5moLS_msg(&client->dev, "Parameter Version: 0x%04x\n", state->prm_ver);
    m5moLS_msg(&client->dev, "Macro AF Version: 0x%04x\n", state->mac_af_ver);
    m5moLS_msg(&client->dev, "Infinite AF Version: 0x%04x\n", state->Inf_af_ver);    
    m5moLS_msg(&client->dev, "Full FW Infomation : %s\n", &state->camera_fw_info);
    //m5moLS_get_version_in_flash(sd);  

    if(copy_to_user((void *)ctrl->value, &module_fw_info, sizeof(m5moLS_fw_info_t)) != 0)
        return -1;
    
    return 0;
}

static void 
m5moLS_wq_fw_dump(struct work_struct *work)
{
	return;
}

static void 
m5moLS_wq_fw_update(struct work_struct *work)
{
	return;
}

static int m5moLS_read_fw_bin(const char *path, char *fwBin, int *fwSize)
{
	char*       buffer = NULL;
	unsigned int file_size = 0;

	struct file *filep = NULL;
	mm_segment_t old_fs;

	printk("m5moLS_read_main_SW_fw_version is called...\n");

	filep = filp_open(path, O_RDONLY, 0) ;

	if (filep && (filep!= 0xfffffffe))
	{
		old_fs = get_fs();
		set_fs(KERNEL_DS);
		
		file_size = filep->f_op->llseek(filep, 0, SEEK_END);
		filep->f_op->llseek(filep, 0, SEEK_SET);
		
		buffer = (char*)kmalloc(file_size+1, GFP_KERNEL);
		
		filep->f_op->read(filep, buffer, file_size, &filep->f_pos);
		buffer[file_size] = '\0';
		
		filp_close(filep, current->files);

		set_fs(old_fs);

		printk("File size : %d\n", file_size);
	}
	else
	{
		return -EINVAL;
	}

	memcpy(fwBin, buffer, file_size);
	*fwSize = file_size;

	kfree(buffer);

	return 0;
}

static int m5moLS_load_fw(struct v4l2_subdev *sd)
{
        struct i2c_client *client = v4l2_get_subdevdata(sd);    
        struct m5moLS_state *state = to_state(sd);

        fw_update_status = 0;

        return 0;
}

/*
 * IRQ handler
 */
static irqreturn_t m5moLS_irq_handler(int irq, void *dev_id)
{
	//printk("m5moLS_irq_handler!!!!!!!!!!!!!!!\n");
	cam_interrupted = 1;
	wake_up_interruptible(&cam_wait);
    
	return IRQ_HANDLED;
}

static int m5moLS_dump_fw(struct v4l2_subdev *sd)
{
	/*[5B] Need to do later*/
	return 0;
}

static int m5moLS_get_capture_size(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	ctrl->value = main_jpeg_size;
	return 0;
}

static int m5moLS_get_thumb_size(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	ctrl->value = thumb_jpeg_size;
	return 0;
}


static int m5moLS_set_preview_resolution(struct v4l2_subdev *sd)
{
    int err, OrgMode = 0x00,CheckMode= 0x00 ;
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct m5moLS_state *state = to_state(sd);

    int index = state->framesize_index;
    u8 capture_resolution = 0, preview_resolution=0;

    m5moLS_msg(&client->dev, "%s: start, size = %d\n", __func__, index);

    switch(index){
        case M5MO_LS_PREVIEW_QCIF: 
            preview_resolution = M5MO_LS_QCIF_SIZE;
            break;

        case M5MO_LS_PREVIEW_QVGA:
            preview_resolution = M5MO_LS_QVGA_SIZE;            
            break;

        case M5MO_LS_PREVIEW_592x480: 
            preview_resolution = M5MO_LS_VGA_SIZE;            
            break; 

        case M5MO_LS_PREVIEW_VGA:
            preview_resolution = M5MO_LS_VGA_SIZE;            
            break;

        case M5MO_LS_PREVIEW_D1: 
            preview_resolution = M5MO_LS_NTSC_SIZE;            
            break;

        case M5MO_LS_PREVIEW_WVGA:
            preview_resolution = M5MO_LS_WVGA_SIZE;            
            break; 

        case M5MO_LS_PREVIEW_720P:
            preview_resolution = M5MO_LS_HD_SIZE;
            break;

        case M5MO_LS_PREVIEW_VERTICAL_QCIF:
            preview_resolution = M5MO_LS_LQVGA_SIZE;
            break;
            
        default:
            /* The framesize index was not set properly. 
            * Check s_fmt call - it must be for video mode. */
            return -EINVAL;
    }

     OrgMode = m5moLS_get_mode(sd); 
     if( OrgMode == M5MO_LS_STILLCAP_MODE)
     {
          m5moLS_msg(&client->dev, " %s : done. and skip to set preview image \n",__func__);
          return 0;
     }
     else
     {
         if(OrgMode != M5MO_LS_PARMSET_MODE)
         {
             err = m5moLS_set_mode(sd, M5MO_LS_PARMSET_MODE);
             if(err)
             {
                 ERR("Can not set operation mode\n");
                 return err;
             }
             while(CheckMode != M5MO_LS_PARMSET_MODE)
             {
                  CheckMode = m5moLS_get_mode(sd);
                  msleep(1);
                  m5moLS_msg(&client->dev,"Check camera module mode \n");
             }
         }

         err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x01, 0x01,preview_resolution);         
         if(err < 0){
             dev_err(&client->dev, "%s: failed: i2c_write for Preview_image_resoltuion\n", __func__);
             return -EIO; 
         }
     
         if(OrgMode != M5MO_LS_PARMSET_MODE)
         {
             err = m5moLS_set_mode(sd, OrgMode);
             if(err)
             {
                 ERR("Can not set operation mode\n");
                 return err;
             }
         }
     
         m5moLS_msg(&client->dev, "%s: done\n", __func__);
         return 0;    
     }

}

static int m5moLS_set_frame_rate(struct v4l2_subdev *sd)
{
	/*[5B] Need to do later*/
	return 0;
}

static int m5moLS_set_anti_banding(struct v4l2_subdev *sd)
{
	int err;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct m5moLS_state *state = to_state(sd);	

       if(state->m_anti_banding_mode == state->anti_banding)
            return 0;
       
	switch(state->anti_banding)
	{
		case ANTI_BANDING_OFF:
                err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x06, 0x04);
	    	  break;

		case ANTI_BANDING_AUTO:
                err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x06, 0x00);
                break;

		case ANTI_BANDING_50HZ:
                err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x06, 0x01);
                break;

		case ANTI_BANDING_60HZ:
		default:
                err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x06, 0x02);
                break;
	}

	if(err < 0){
		dev_err(&client->dev, "%s: failed: i2c_write for anti_banding\n", __func__);
		return -EIO;
	}
    
       state->m_anti_banding_mode = state->anti_banding;
	m5moLS_msg(&client->dev, "%s: done\n", __func__);
	
	return 0;
}

static int m5moLS_set_preview_stop(struct v4l2_subdev *sd)
{
	/*[5B] Need to do later*/
	return 0;
}

static int m5moLS_set_dzoom(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    int err = -1,zoom_level=0;
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct m5moLS_state *state = to_state(sd);

    zoom_level = ctrl->value;
	m5moLS_msg(&client->dev, "%s: Enter : Digital Zoom = %d\n", __func__, zoom_level);
    
    if(state->m_zoom_level == zoom_level)
        return 0;
    
    if((zoom_level < M5MO_LS_ZOOM_LEVEL_0) || (zoom_level > M5MO_LS_ZOOM_LEVEL_MAX))
    {
        m5moLS_msg(&client->dev,"Error, Zoom Level is out of range!\n");
        return -EINVAL;
    }
    
    /* Start Digital Zoom */
    err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x02, 0x01,m5moLS_zoom_table[zoom_level]); 
    msleep(30);

    if(err == 0){
        state->m_zoom_level = zoom_level;
        m5moLS_msg(&client->dev, "%s: done\n", __func__);
    }
    return err;
}

static int m5moLS_set_dtp(struct v4l2_subdev *sd)
{
    int err, rval;
    struct i2c_client *client = v4l2_get_subdevdata(sd);

	/* Set Parameter Mode */
	rval = m5moLS_set_mode(sd, M5MO_LS_PARMSET_MODE);
	if(rval)
	{
		m5moLS_msg(&client->dev,"Can not set parameter mode\n");
		return rval;
	}

	/* MON_SIZE(QVGA size) */
	m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x01, 0x01, 0x09);
	/* YUVOUT_MAIN(YUV422) */
	m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x00, 0x00);
	/* MAIN_IMAGE_SIZE(QVGA) */
	m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x01, 0x02);
	/* PREVIEW_IMAGE_SIZE(QVGA size) */
	m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x06, 0x01);
	/* MON_FPS(AUTO) */
	m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x01, 0x02, 0x01);
	
	/* Set Monitor Mode */
	rval = m5moLS_set_mode(sd, M5MO_LS_MONITOR_MODE);
	if(rval)
	{
		m5moLS_msg(&client->dev,"Can not set monitor mode\n");
		return rval;
	}

	/* Start output test data */
	m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0D, 0x1B, 0x01);
	
    return rval;  
}

static int m5moLS_set_dtp_stop(struct v4l2_subdev *sd)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);

	/* MON_SIZE(QVGA size) */
	m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0D, 0x1B, 0x00);
	
    return 0;  
}

static int m5moLS_set_preview_start(struct v4l2_subdev *sd)
{
        int err;
        struct i2c_client *client = v4l2_get_subdevdata(sd);
        struct m5moLS_state *state = to_state(sd);

        int mode, rval;
        u32 intr_value;
        int output_if = 0xff;

        m5moLS_msg(&client->dev,"%s\n",__func__);
        
		m5moLS_msg(&client->dev,"state->check_dataline: %d\n", state->check_dataline);

		/*This is for dtp*/
		if(state->check_dataline)
		{
			rval = m5moLS_set_dtp(sd);
			if(rval < 0){
				dev_err(&client->dev, "%s: failed: Could not check data line.\n", __func__);
				return rval;
			}
		}

        /* If mode is already Monitor, return now */
        mode = m5moLS_get_mode(sd);
        if(mode == M5MO_LS_MONITOR_MODE)
            return 0;
        
        /* MONITER / MOVIE */
        m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x01, 0x32, state->m_movie_mode);
        /* Interrupt Clear */
        m5moLS_read_category_parm(client, M5MO_LS_8BIT, 0x00, 0x10, &intr_value);
        /* Enable YUV-Output Interrupt */
        m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x00, 0x11, M5MO_LS_INT_MODE);
        /* Enable Interrupt Control */
        //m5mo_write_category_parm(client, M5MO_8BIT, 0x00, 0x12, 0x01);

        /* Set Monitor Mode */
        rval = m5moLS_set_mode(sd, M5MO_LS_MONITOR_MODE);
        if(rval)
        {
                m5moLS_msg(&client->dev,"Can not set operation mode\n");
                m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x00, 0x11, 0x00);
                return rval;
        }
	/* Anti-Banding 60Hz*/
	m5moLS_set_anti_banding(sd);

        /* AE/AWB Unlock */
        if(m5moLS_set_ae_lock(sd,M5MO_LS_AE_UNLOCK) != 0)
            return -EINVAL;
        if(m5moLS_set_awb_lock(sd,M5MO_LS_AWB_UNLOCK) != 0)
            return -EINVAL;


        /* Wait 'YUV-Output' interrupt */
        m5moLS_msg(&client->dev,"Waiting 'YUV-Output' interrupt... \n");
        rval = m5moLS_wait_interrupt(sd, M5MO_LS_INT_MODE, 2000);
        if(rval)
            m5moLS_msg(&client->dev,"%s: m5moLS_wait_interrupt error!\n", __FUNCTION__);

        /* OUTPUT_IF_SEL(YUV-IF) */
        m5moLS_read_category_parm(client, M5MO_LS_8BIT, 0x01, 0x00, &output_if);
    	m5moLS_msg( &client->dev,"OUTPUT INTERFACE SEL PARM : 0x%02x.\n", output_if);
    
        return rval;
     
}

static int m5moLS_set_capture_resolution(struct v4l2_subdev *sd)
{
    int err, OrgMode = 0x00, CheckMode = 0x00;
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct m5moLS_state *state = to_state(sd);

    int index = state->framesize_index;
    u8 capture_resolution = 0, preview_resolution=0, postview_resolution=0;

    m5moLS_msg(&client->dev, "%s: start\n", __func__);

    switch(index){
        case M5MO_LS_CAPTURE_VGA: /* 640x480 */
            capture_resolution = M5MO_LS_SHOT_VGA_SIZE;
            postview_resolution = 0x0B;
            break;

        case M5MO_LS_CAPTURE_WVGA: /* 800x480 */
            capture_resolution = M5MO_LS_SHOT_WVGA_SIZE;
            postview_resolution = 0x0C;            
            break;

        case M5MO_LS_CAPTURE_W1_5MP://  1600x 960
            capture_resolution = M5MO_LS_SHOT_1600_960;
            postview_resolution = 0x0C;            
            break;

        case M5MO_LS_CAPTURE_2MP:// 1600 x 1200
            capture_resolution = M5MO_LS_SHOT_1600_1200;
            postview_resolution = 0x0B;            
            break;

        case M5MO_LS_CAPTURE_W4MP: // 2560 x 1536
            capture_resolution = M5MO_LS_SHOT_2560_1536;
            postview_resolution = 0x0C;            
            break;

        case M5MO_LS_CAPTURE_5MP:// 2560 x 1920
            capture_resolution = M5MO_LS_SHOT_2560_1920;
            postview_resolution = 0x0B;            
            break;

        case M5MO_LS_CAPTURE_W6MP: // 3264 x 1960
            capture_resolution = M5MO_LS_SHOT_3264_1960;
            postview_resolution = 0x0C;            
            break;

        case M5MO_LS_CAPTURE_8MP: // 3264 x 2448
            capture_resolution = M5MO_LS_SHOT_3264_2448;
            postview_resolution = 0x0B;            
            break;

        default:
            /* The framesize index was not set properly. 
            * Check s_fmt call - it must be for video mode. */
            return -EINVAL;
    }

	/* Set capture image resolution */
	err =     m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x01, capture_resolution);
	if(err < 0){
		dev_err(&client->dev, "%s: failed: i2c_write for capture_resolution\n", __func__);
		return -EIO; 
	}

       /* Set Postview image resolution */
       err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x0B, postview_resolution);            
       if(err < 0){
           dev_err(&client->dev, "%s: failed: i2c_write for Preview_image_resoltuion\n", __func__);
           return -EIO; 
       }
#if 0
       /* Set Preview image resolution */
       OrgMode = m5moLS_get_mode(sd); 
       if( OrgMode == M5MO_LS_STILLCAP_MODE)
       {
            m5moLS_msg(&client->dev, " %s : done. and skip to set preview image \n",__func__);
            return 0;
       }
       else
       {
           if(OrgMode != M5MO_LS_PARMSET_MODE)
           {
               err = m5moLS_set_mode(sd, M5MO_LS_PARMSET_MODE);
               if(err)
               {
                   ERR("Can not set operation mode\n");
                   return err;
               }
               while(CheckMode != M5MO_LS_PARMSET_MODE)
               {
                    CheckMode = m5moLS_get_mode(sd);
                    msleep(1);
                    m5moLS_msg(&client->dev,"Check camera module mode \n");
               }
           }
           
           err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x06, preview_resolution);            
           if(err < 0){
               dev_err(&client->dev, "%s: failed: i2c_write for Preview_image_resoltuion\n", __func__);
               return -EIO; 
           }

           if(OrgMode != M5MO_LS_PARMSET_MODE)
           {
               err = m5moLS_set_mode(sd, OrgMode);
               if(err)
               {
                   ERR("Can not set operation mode\n");
                   return err;
               }
           }
           m5moLS_msg(&client->dev, "%s: done\n", __func__);
           return 0;	
       }
#endif       
      m5moLS_msg(&client->dev, "%s: done\n", __func__);
      return 0;    
}


static int m5moLS_set_awb_lock(struct v4l2_subdev *sd, int lock)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int ret = -EINVAL;
    
    if(lock)
        ret = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x06, 0x00, 0x01);
    else
        ret = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x06, 0x00, 0x00);

    if(ret < 0)
        m5moLS_info(&client->dev,"%s :: failed\n",__func__);
    
    return ret;
}

static int m5moLS_set_ae_lock(struct v4l2_subdev *sd, int lock)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int ret = -EINVAL;

    if(lock)
        ret = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x00, 0x01); 
    else
        ret = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x00, 0x00);  
    
    if(ret < 0)
        m5moLS_info(&client->dev,"%s :: failed\n",__func__);
    
    return ret;
}

static int m5moLS_set_jpeg_quality(struct v4l2_subdev *sd, int quality)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct m5moLS_state *state = to_state(sd);
    int compressionRatio = 0;

    m5moLS_msg(&client->dev, "%s : Enter ::::: quality = %d\n",__func__,quality);

    m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x17, 0x62);    
    
    if(quality > 70)    // superfine
    {
        m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x34, M5MO_LS_JPEG_SUPERFINE);
    }
    else if(quality > 40)   // fine
    {
        m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x34, M5MO_LS_JPEG_FINE);
    }
    else    // normal
    {
        m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x34, M5MO_LS_JPEG_NORMAL);    
    }
    
    return 0;
}
    
static int m5moLS_convert_iso(struct v4l2_subdev *sd, int value)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int i;
    int ret_val = 0;

    for(i=0;i<ISO_TABLE_MAX;i++)
    {
        if(value < iso_reference_table[i])
        {
            ret_val = iso_conversion_table[i];
            break;
        }
    }
    m5moLS_msg(&client->dev,"%s::: value = %d\n",__func__,ret_val);

    if(ret_val == 0)
        return value;
    else
        return ret_val;
    
}

static int m5moLS_read_exif_info(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct m5moLS_state *state = to_state(sd);

    int err = -1;
    int val = 0;

	m5moLS_read_category_parm(client, M5MO_LS_32BIT, 0x07, 0x00, &val);
	state->exif_info.info_exptime_numer = val;
	
	m5moLS_msg(&client->dev, "%s: exp_time_val_numer is (%d)\n", __func__, val);

    m5moLS_read_category_parm(client, M5MO_LS_32BIT, 0x07, 0x04, &val);		
	state->exif_info.info_exptime_denumer = val;
	
	m5moLS_msg(&client->dev, "%s: exp_time_val_denumer is (%d)\n", __func__, val);

	m5moLS_read_category_parm(client, M5MO_LS_32BIT, 0x07, 0x08, &val);
	state->exif_info.info_tv_numer = val;
	
	m5moLS_msg(&client->dev, "%s: tv_numer is (%d)\n", __func__, val);

	m5moLS_read_category_parm(client, M5MO_LS_32BIT, 0x07, 0x0C, &val);
	state->exif_info.info_tv_denumer = val;
	
	m5moLS_msg(&client->dev, "%s: tv_denumer is (%d)\n", __func__, val);

	m5moLS_read_category_parm(client, M5MO_LS_32BIT, 0x07, 0x10, &val);
	state->exif_info.info_av_numer = val;
	
	m5moLS_msg(&client->dev, "%s: av_numer is (%d)\n", __func__, val);

	m5moLS_read_category_parm(client, M5MO_LS_32BIT, 0x07, 0x14, &val);
	state->exif_info.info_av_denumer = val;
	
	m5moLS_msg(&client->dev, "%s: av_denumer is (%d)\n", __func__, val);

	m5moLS_read_category_parm(client, M5MO_LS_32BIT, 0x07, 0x18, &val);
	state->exif_info.info_bv_numer = val;
	
	m5moLS_msg(&client->dev, "%s: bv_numer is (%d)\n", __func__, val);

	m5moLS_read_category_parm(client, M5MO_LS_32BIT, 0x07, 0x1C, &val);
	state->exif_info.info_bv_denumer = val;
	
	m5moLS_msg(&client->dev, "%s: bv_denumer is (%d)\n", __func__, val);

	m5moLS_read_category_parm(client, M5MO_LS_32BIT, 0x07, 0x20, &val);
	state->exif_info.info_ebv_numer = val;
	
	m5moLS_msg(&client->dev, "%s: ebv_numer is (%d)\n", __func__, val);

	m5moLS_read_category_parm(client, M5MO_LS_32BIT, 0x07, 0x24, &val);
	state->exif_info.info_ebv_denumer = val;
	
	m5moLS_msg(&client->dev, "%s: ebv_denumer is (%d)\n", __func__, val);
	
    err = m5moLS_read_category_parm(client, M5MO_LS_16BIT, 0x07, 0x28, &val);
	if(err < 0)
	{
		m5moLS_info(&client->dev, "%s: i2c_read failed for iso\n", __func__);
		return -EIO;
	}
	else
	{
		state->exif_info.info_iso = m5moLS_convert_iso(sd, (val&0xffff));
		m5moLS_msg(&client->dev,"m5moLS_read_exif_info: iso is (%d)\n", (val&0xffff));
	}	
	
    err = m5moLS_read_category_parm(client, M5MO_LS_16BIT, 0x07, 0x2A, &val);
    
       val = val&0xffff;
	if(err < 0)
	{
		m5moLS_info(&client->dev, "%s: i2c_read failed for flash\n", __func__);
		return -EIO;
	}
	else
	{
		if(val == 24) //means (auto) Flash did not fire
		{
			state->exif_info.info_flash = 0;
		}
		else
		{
			state->exif_info.info_flash = 1;		
		}
		m5moLS_msg(&client->dev,"m5moLS_read_exif_info: flash is (%d)\n", val);
	}
	m5moLS_msg(&client->dev, "%s: is finished successfully\n", __func__);

	return err;
}	


static int 
m5moLS_start_capture_transfer(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct m5moLS_state *state = to_state(sd);
	struct v4l2_pix_format pix = state->pix;

	int rval, err;
	u32 val = 0, intr_value;
	u32 jpeg_max_size =0, temp_val =0,thumbnail_max_size=0;

	/* wait interrupt sequence is added from m5moLS_start_capture_transfer() because of shutter sound timing*/
	//mdelay(200);

        /* Set Capture JPEG Quality */
        m5moLS_set_jpeg_quality(sd, state->jpeg.quality);

	/* Select main frame image*/
	m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0C, 0x06, 0x01);
       
	/* Select main image size */	
        m5moLS_set_capture_resolution(sd);

	/* Start capture transfering */
	m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0C, 0x09, 0x01);
	/* Wait 'Capture Transfer Complete' interrupt */
	DBG("Waiting 'Capture Transfer Complete' interrupt... \n");
	rval = m5moLS_wait_interrupt(sd, M5MO_LS_INT_CAPTURE, 10000);
	if(rval != 0)
		m5moLS_info(&client->dev,"%s: m5mo_wait_interrupt error :::: return = %d!\n", __FUNCTION__,rval);
	else
		DBG("Capture Image Transfer Complete!!\n");

	/*Read Exif info*/
	err = m5moLS_read_exif_info(sd);
	if(err < 0){
		m5moLS_info(&client->dev, "%s: failed: m5moLS_read_exif_info\n", __func__);
		return -EIO;
	}

	/*Read main image JPEG size */
	m5moLS_read_category_parm(client, M5MO_LS_32BIT, 0x0C, 0x0D, &val);
	main_jpeg_size = val;
	m5moLS_read_category_parm(client, M5MO_LS_32BIT, 0x0C, 0x11, &val);
	m5moLS_msg(&client->dev, "%s: JPEG thumbnail is %d\n", __func__, val);\
	thumb_jpeg_size = val;

	temp_val = 0;
	m5moLS_read_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x12, &temp_val);
	jpeg_max_size |= temp_val;
	temp_val =0;
	m5moLS_read_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x11, &temp_val);
	jpeg_max_size |= (temp_val<<8);
	temp_val =0;
	m5moLS_read_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x10, &temp_val);
	jpeg_max_size |= (temp_val<<16);
	temp_val =0;
	m5moLS_read_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x0F, &temp_val);
	jpeg_max_size |= (temp_val<<24);
	temp_val =0;
       
	temp_val = 0;
	m5moLS_read_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x3F, &temp_val);
	thumbnail_max_size |= temp_val;
	temp_val =0;
	m5moLS_read_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x3E, &temp_val);
	thumbnail_max_size |= (temp_val<<8);
	temp_val =0;
	m5moLS_read_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x3D, &temp_val);
	thumbnail_max_size |= (temp_val<<16);
	temp_val =0;
	m5moLS_read_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x3C, &temp_val);
	thumbnail_max_size |= (temp_val<<24);
	temp_val =0;       
	state->jpeg.main_offset = 0;
	state->jpeg.thumb_offset = M5MO_LS_THUMB_OFFSET;
	state->jpeg.postview_offset = M5MO_LS_THUMB_OFFSET+M5MO_LS_THUMB_MAX_SIZE;

	m5moLS_msg(&client->dev, "%s: done :::: main_jpeg_size=%d Kbyte, thumb_jpeg_size=%d Kbyte\n", __func__,main_jpeg_size/1024, thumb_jpeg_size/1024);
	m5moLS_msg(&client->dev, "jpeg_max_size = 0x%x\n",jpeg_max_size);
	m5moLS_msg(&client->dev, "thumbnail_max_size = 0x%x\n",thumbnail_max_size);       
       
	//capture_transfer_completed = 1;

	return rval;
}


static int m5moLS_set_capture_start(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct m5moLS_state *state = to_state(sd);
    struct v4l2_pix_format pix = state->pix;

    int rval = 0;
    u32 intr_value;
    
    /* Select main image format */
    if(pix.pixelformat == V4L2_PIX_FMT_JPEG || pix.pixelformat == V4L2_PIX_FMT_MJPEG)
    	m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x00, 0x10); // JPEG(with header)
    else
    	m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x00, 0x00); // YUV422

    /* Set Thumbnail Max Size */
    m5moLS_write_category_parm(client,M5MO_LS_32BIT, 0x0B,0x3C, M5MO_LS_THUMB_MAX_SIZE);
    
    /* AE/AWB Lock */
    if(m5moLS_set_ae_lock(sd,M5MO_LS_AE_LOCK) != 0)
        return -EINVAL;
    if(m5moLS_set_awb_lock(sd,M5MO_LS_AWB_LOCK) != 0)
        return -EINVAL;

    if(state->vintage_mode > VINTAGE_MODE_BASE)
    {
        if(m5moLS_set_vintage_mode(sd) < 0)
        {
            m5moLS_info(&client->dev,"failed set_vintage_mode\n");
            return -EINVAL;
        }
    }

    if(m5moLS_set_face_beauty(sd) < 0)
    {
        m5moLS_info(&client->dev,"failed set_face_beauty\n");
        return -EINVAL;
    }
    
    /* Enable Capture Transfer Interrupt */
    m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x00, 0x11, M5MO_LS_INT_CAPTURE);

    /* Set Still Capture Mode */
    m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x00, 0x0B, M5MO_LS_STILLCAP_MODE);

    /* Wait 'Capture Complete' interrupt */
    DBG("Waiting 'Capture Complete' interrupt... \n");
    rval = m5moLS_wait_interrupt(sd, M5MO_LS_INT_CAPTURE, 10000);
    if(rval != 0)
    	m5moLS_info(&client->dev,"%s: m5moLS_wait_interrupt error ::: return = %d!\n", __FUNCTION__,rval);
    else
    	DBG("Capture Image Storing and Processing Complete!!\n");
  
    return 0; 
}


//static int m5moLS_get_focus_mode(struct i2c_client *client, unsigned char cmd, unsigned char *value)
static int m5moLS_get_focus_mode(struct i2c_client *client, struct v4l2_control *ctrl)

{
    int err = -1,count;
    u32 val = 0;

    for(count = 600; count > 0; count--)
    {
        msleep(10);
        m5moLS_read_category_parm(client, M5MO_LS_8BIT, 0x0A, 0x03, &val);
        if(val == 2)
            err = 0;
            
        if(val == 1)
            ctrl->value = M5MO_LS_AF_STATUS_MOVING;
        else if(val == 0){
            ctrl->value = M5MO_LS_AF_STATUS_FAILED; 
        }
        else if(val == 2){
            ctrl->value = M5MO_LS_AF_STATUS_SUCCESS;
            break;
        }

        m5moLS_msg(&client->dev,"AF status reading... %d \n", ctrl->value);
    }
    
    if(ctrl->value == M5MO_LS_AF_STATUS_FAILED || ctrl->value == M5MO_LS_AF_STATUS_SUCCESS) 
        m5moLS_msg(&client->dev, "AF result = %d (0.success, -1.fail)\n", ctrl->value);

	return ctrl->value;
}


static int m5moLS_set_af_softlanding(struct v4l2_subdev *sd)
{
	/*[5B] Need to do later*/
	return 0;
}

static int 
m5moLS_set_flash_capture(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct m5moLS_state *state = to_state(sd);

    m5moLS_msg(&client->dev, "%s\n", __func__);

    switch(ctrl->value)
    {
        case FLASH_MODE_OFF:
        case FLASH_MODE_TORCH_OFF:
            m5moLS_msg(&client->dev,"Flash Off is set\n");
            m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x40, 0x00);
            m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x41, 0x00);
            break;

        case FLASH_MODE_ON:
            m5moLS_msg(&client->dev,"Flash On is set\n");
            m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x40, 0x01);
            m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x41, 0x01);
            break;

        case FLASH_MODE_AUTO:
            m5moLS_msg(&client->dev,"Flash Auto is set\n");
            m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x40, 0x02);
            m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x41, 0x02);
            break;

        case FLASH_MODE_TORCH_ON:
        case FLASH_MODE_TORCH_ALWAYS_ON:
            m5moLS_msg(&client->dev,"Flash torch On is set\n");
            m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x40, 0x03);
            m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x41, 0x00);
            break;           

        case FLASH_MODE_BACKLIGHT_ON:
            m5moLS_msg(&client->dev,"Flash Backlight On is set\n");
            m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x40, 0x01);
            m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x41, 0x01);
            break;

        default:
            m5moLS_msg(&client->dev,"[FLASH CAPTURE]Invalid value is ordered!!!\n");
            return -EINVAL;
    }

    m5moLS_msg(&client->dev, "%s: Done!!\n", __func__);

    return 0;
}

static int m5moLS_set_effect_gamma(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	int rval, orgmode;
	u32 readval = 0;

	/* Read Color Effect ON/OFF */
	m5moLS_read_category_parm(client, M5MO_LS_8BIT, 0x02, 0x0B, &readval);
	/* If Color Effect is on, turn it off */
	if(readval)
		m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x02, 0x0B, 0x00); 
    
	/* Check Original Mode */
	orgmode =m5moLS_get_mode(sd);
    
	/* Set Parameter Mode */
	rval = m5moLS_set_mode(sd, M5MO_LS_PARMSET_MODE);
	if(rval)
	{
		m5moLS_msg(&client->dev,"Can not set operation mode\n");
		return rval;
	}
	
	switch(ctrl->value)
	{
		case IMAGE_EFFECT_NEGATIVE:
			rval = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x01, 0x0B, 0x01);
			break;
#if 0 // sun ggeun : stealth : not used this effect mode
		case M5MO_EFFECT_EMBOSS:
			m5mo_write_category_parm(client, M5MO_LS_8BIT, 0x01, 0x0B, 0x06);
			break;
		case M5MO_EFFECT_OUTLINE:
			m5mo_write_category_parm(client, M5MO_LS_8BIT, 0x01, 0x0B, 0x07);
			break;
#endif            
		case IMAGE_EFFECT_AQUA:
			rval = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x01, 0x0B, 0x08);
			break;
            
		default:
            rval = -EINVAL;
			break;	
	}

	/* Set Monitor Mode */
	if(orgmode == M5MO_LS_MONITOR_MODE)
	{
		rval = m5moLS_set_mode(sd, M5MO_LS_MONITOR_MODE);
		if(rval)
		{
			m5moLS_msg(&client->dev,"Can not start preview\n");
			return rval;
		}
	}

	return rval;
}


static int m5moLS_set_effect_color(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	int rval, orgmode;
	u32 readval = 0;

	/* Read Gamma Effect ON/OFF */
	m5moLS_read_category_parm(client, M5MO_LS_8BIT, 0x01, 0x0B, &readval);
	/* If Gamma Effect is on, turn it off */
	if(readval)
	{
		orgmode = m5moLS_get_mode(sd);
		rval = m5moLS_set_mode(sd, M5MO_LS_PARMSET_MODE);
		if(rval)
		{
			ERR("Can not set operation mode\n");
			return rval;
		}
		m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x01, 0x0B, 0x00); 

		if(orgmode == M5MO_LS_MONITOR_MODE)
		{
			rval = m5moLS_set_mode(sd, M5MO_LS_MONITOR_MODE);
			if(rval)
			{
				ERR("Can not start preview\n");
				return rval;
			}
		}
	}

	switch(ctrl->value)
	{
		case IMAGE_EFFECT_NONE:
			rval = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x02, 0x0B, 0x00);
			break;
            
		case IMAGE_EFFECT_SEPIA:
			rval = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x02, 0x0B, 0x01); 
			rval = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x02, 0x09, 0xD8);
			rval = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x02, 0x0A, 0x18);
			break;
            
		case IMAGE_EFFECT_BNW:
			rval = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x02, 0x0B, 0x01); 
			rval = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x02, 0x09, 0x00);
			rval = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x02, 0x0A, 0x00);
			break;
            
		case IMAGE_EFFECT_ANTIQUE:
			rval = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x02, 0x0B, 0x01); 
			rval = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x02, 0x09, 0xD0);
			rval = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x02, 0x0A, 0x30);
			break;	
            
		default:
            rval = -EINVAL;
			break;
	}
		
	return rval;
}


static int m5moLS_set_effect(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	int err = -1;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
       struct m5moLS_state *state = to_state(sd);
        
       if(state->m_effect_mode == ctrl->value)
            return 0;
       
	switch(ctrl->value)
	{
		case IMAGE_EFFECT_NONE:
		case IMAGE_EFFECT_BNW:            
		case IMAGE_EFFECT_SEPIA:                  
		case IMAGE_EFFECT_ANTIQUE:            
		case IMAGE_EFFECT_SHARPEN:            
			err = m5moLS_set_effect_color(sd, ctrl);
		    break;

		case IMAGE_EFFECT_NEGATIVE:
		case IMAGE_EFFECT_AQUA:                  
			err = m5moLS_set_effect_gamma(sd, ctrl);
		    break;

		default:
			m5moLS_msg(&client->dev,,"%s : Invalid value is ordered!!\n", __func__);
            err = -EINVAL;
    		break;
	}

       state->m_effect_mode = ctrl->value;
	m5moLS_msg(&client->dev, "%s: done\n", __func__);

	return err;
}

static int m5moLS_set_saturation(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	int err = -1;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
       struct m5moLS_state *state = to_state(sd);  

	m5moLS_msg(&client->dev, "%s: Enter : saturation %d\n", __func__, ctrl->value);

       if(state->m_saturation_mode == ctrl->value)
            return 0;

	err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x02, 0x10, 0x01);

	if(err != 0) {
		m5moLS_info(&client->dev, "%s: Chroma gain control failed to be on, %d\n", __func__, err);

		return err;
	}

	switch(ctrl->value)
	{
		case SATURATION_MINUS_2:
			err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x02, 0x0F, 0x01);
    		break;

		case SATURATION_MINUS_1:
			err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x02, 0x0F, 0x02);
    		break;

		case SATURATION_DEFAULT:
			err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x02, 0x0F, 0x03);
    		break;

		case SATURATION_PLUS_1:
			err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x02, 0x0F, 0x04);
    		break;

		case SATURATION_PLUS_2:
			err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x02, 0x0F, 0x05);
    		break;

		default:
			m5moLS_msg(&client->dev,,"%s : Invalid value is ordered!!\n", __func__);
            err = -EINVAL;
    		break;

	}

       state->m_saturation_mode = ctrl->value;
       
	m5moLS_msg(&client->dev, "%s: done, saturation: %d\n", __func__, ctrl->value);

	return err;
}

static int m5moLS_set_contrast(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    int err = -1;
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct m5moLS_state *state = to_state(sd);

    if(state->m_contrast_mode == ctrl->value)
        return 0;
    
    switch(ctrl->value)
    {
        case CONTRAST_MINUS_2:
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x02, 0x25, 0x03);
            break;
            
        case CONTRAST_MINUS_1:
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x02, 0x25, 0x04);
            break;
            
        case CONTRAST_DEFAULT:
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x02, 0x25, 0x05);
            break;
            
        case CONTRAST_PLUS_1:
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x02, 0x25, 0x06);
            break;
            
        case CONTRAST_PLUS_2:
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x02, 0x25, 0x07);
            break;
            
        default:
            m5moLS_msg(&client->dev,"[Contrast]Invalid value is ordered!!!\n");
            err =  -EINVAL;
            break;
    }
    state->m_contrast_mode = ctrl->value;
    
    return err;
}


static int m5moLS_set_sharpness(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    int err =-1;
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct m5moLS_state *state = to_state(sd);

    if(state->m_sharpness_mode == ctrl->value)
        return 0;
    
    err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x02, 0x12, 0x01);

    if(err != 0) {
    	m5moLS_info(&client->dev, "%s: Edge enhancement gain control failed to be on, %d\n", __func__, err);

    	return err;
    }
	
    switch(ctrl->value)
    {
        case SHARPNESS_MINUS_2:
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x02, 0x11, 0x03);
            break;
        
        case SHARPNESS_MINUS_1:
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x02, 0x11, 0x04);
            break;
        
        case SHARPNESS_DEFAULT:
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x02, 0x11, 0x05);
            break;
        
        case SHARPNESS_PLUS_1:
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x02, 0x11, 0x06);
            break;
        
        case SHARPNESS_PLUS_2:
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x02, 0x11, 0x07);
            break;
        
        default:
            m5moLS_msg(&client->dev,"[Sharpness]Invalid value is ordered!!!\n");
            return -EINVAL;
    }
    state->m_sharpness_mode = ctrl->value;
    
    return 0;
}


static int m5moLS_set_wdr(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    int err = -1;
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct m5moLS_state *state = to_state(sd);

    if(state->m_wdr_mode == ctrl->value)
        return 0;

    switch(ctrl->value)
    {
    case WDR_OFF:
        err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x02, 0x25, 0x05);
        err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x2C, 0x00);
        break;
    
    case WDR_ON:
        err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x02, 0x25, 0x09);
        err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x2C, 0x02);
        break;
        
    default:
        m5moLS_msg(&client->dev,"[WDR]Invalid value is ordered!!!\n");
        err = -EINVAL;
        break;
    }
    state->m_wdr_mode = ctrl->value;
    
    return err;
}


static int m5moLS_set_anti_shake(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    int err = -1;
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct m5moLS_state *state = to_state(sd);

    if(state->m_anti_shake_mode == ctrl->value)
        return 0;
    
    /* WDR & ISC are orthogonal!*/
    switch(ctrl->value)
    {
        case ANTI_SHAKE_OFF:
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x0A, 0x00);
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x0B, 0x00);
            break;
        
        case ANTI_SHAKE_STILL_ON:
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x0B, 0x0E);
            break;
        
        case ANTI_SHAKE_MOVIE_ON:
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x0A, 0x0E);
            break;
        
        default:
            m5moLS_msg(&client->dev,"[Anti_Shake]Invalid value is ordered!!!\n");
            err =  -EINVAL;
            break;
    }
    state->m_anti_shake_mode = ctrl->value;
        
    return err;
}

static int m5moLS_set_continous_af(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{   
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct m5moLS_state *state = to_state(sd);
    int err = -1;
    
    switch(ctrl->value)
    {
        case M5MO_LS_AF_CONTINUOUS_OFF:
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0A, 0x05, 0x00);
            break;
            
        case M5MO_LS_AF_CONTINUOUS_ON:
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0A, 0x05, 0x01);
            break;
            
        default:
            m5moLS_msg(&client->dev,"[CONTINUOUS AF]Invalid value is ordered!!!\n");
            err = -EINVAL;
            break;
    }

    if(err < 0)
        m5moLS_info(&client->dev,"%s ::: failed\n",__func__);
    
    return err;
}

static int m5moLS_set_face_detection(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    int err = -1;
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct m5moLS_state *state = to_state(sd);

    switch(ctrl->value)
    {
        case FACE_DETECTION_OFF:
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x09, 0x00, 0x00);
            break;
        
        case FACE_DETECTION_ON:
        case FACE_DETECTION_NOLINE:
        case FACE_DETECTION_ON_BEAUTY:
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x09, 0x00, 0x11);
            break;
        
        default:
            m5moLS_msg(&client->dev,"[Face Detection]Invalid value is ordered!!!\n");
            err =  -EINVAL;
            break;
    }

    if(err < 0)
        m5moLS_info(&client->dev,"%s ::: failed\n",__func__);

    return err;
}

static int m5moLS_set_touch_auto_focus(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    int err = -1; 
    u16 x=0, y =0;

    struct m5moLS_state *state = to_state(sd);
    struct i2c_client *client = v4l2_get_subdevdata(sd);

    m5moLS_msg(&client->dev, "%s: Enter : Touch_AF Location= %d, %d\n", __func__, x, y);

    if(ctrl->value)     /* Start */
    {
        /* get x,y touch position */
        x = state->position.x;
        y = state->position.y;
        state->touch_af_mode = 1;
        
        m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0A, 0x01, 0x0B);         // Touch AF mode

        m5moLS_msg(&client->dev, "%s: Enter : Touch_AF = %d, %d\n", __func__, x, y);
        m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0A, 0x30, x >> 8); 
        m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0A, 0x31, x & 0xFF); 

        m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0A, 0x32, y >> 8); 
        m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0A, 0x33, y & 0xFF);   
    }
    else    /* Stop */
    {
        /* AE/AWB Unlock */
        if(m5moLS_set_ae_lock(sd,M5MO_LS_AE_UNLOCK) != 0)
            return -EINVAL;
        if(m5moLS_set_awb_lock(sd,M5MO_LS_AWB_UNLOCK) != 0)
            return -EINVAL;

        msleep(5);    
    }

    return 0;
}

static int m5moLS_set_focus_mode(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    int err = -1, af_status = 0;

    struct m5moLS_state *state = to_state(sd);
    struct i2c_client *client = v4l2_get_subdevdata(sd); 

    m5moLS_msg(&client->dev, "%s: Enter :::: Focus Mode = %d(%d)\n",__func__,ctrl->value,state->m_focus_mode);
    
    af_status = state->userset.lens.af_status;

    switch(ctrl->value)
    {
        case FOCUS_MODE_AUTO:
            /*Set Auto(Normal) AF mode */
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0A, 0x01, 0x00);
            state->userset.lens.af_status = M5MO_LS_AF_INIT_NORMAL;
            break;
            
        case FOCUS_MODE_AUTO_DEFAULT:            
            /* AE/AWB Unlock */
            if(m5moLS_set_ae_lock(sd,M5MO_LS_AE_UNLOCK) != 0)
                return -EINVAL;
            if(m5moLS_set_awb_lock(sd,M5MO_LS_AWB_UNLOCK) != 0)
                return -EINVAL;       
        
            /*Set Auto(Normal) AF mode */
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0A, 0x01, 0x00);
            /* AF initial position */
            m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0A, 0x00, 0x01);            
            state->userset.lens.af_status = M5MO_LS_AF_INIT_NORMAL;
            break;
            
        case FOCUS_MODE_MACRO: 
            /*Set Macro AF mode */
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0A, 0x01, 0x01);
            state->userset.lens.af_status = M5MO_LS_AF_INIT_MACRO;
            break;
            
        case FOCUS_MODE_MACRO_DEFAULT:
            /* AE/AWB Unlock */
            if(m5moLS_set_ae_lock(sd,M5MO_LS_AE_UNLOCK) != 0)
                return -EINVAL;
            if(m5moLS_set_awb_lock(sd,M5MO_LS_AWB_UNLOCK) != 0)
                return -EINVAL;       
            
            /*Set Macro AF mode */
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0A, 0x01, 0x01);
            /* AF initial position */
            m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0A, 0x00, 0x01);            
            state->userset.lens.af_status = M5MO_LS_AF_INIT_MACRO;
            break;
            
        case FOCUS_MODE_FD:
            /*Set Face Detection AF mode */
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0A, 0x01, 0x03);
            state->userset.lens.af_status = M5MO_LS_AF_INIT_FD;
            break;
			
        case FOCUS_MODE_FD_DEFAULT:
            /* AE/AWB Unlock */
            if(m5moLS_set_ae_lock(sd,M5MO_LS_AE_UNLOCK) != 0)
                return -EINVAL;
            if(m5moLS_set_awb_lock(sd,M5MO_LS_AWB_UNLOCK) != 0)
                return -EINVAL;        

            /* AF Stop Operation */
            m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0A, 0x02, 0x00);                    
            /*Set Face Detection AF mode */
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0A, 0x01, 0x03);
            /* AF initial position */
            m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0A, 0x00, 0x01);            
            state->userset.lens.af_status = M5MO_LS_AF_INIT_FD;
            break;

        default:
		m5moLS_msg(&client->dev, "%s: Invalid Focus mode\n",__func__, ctrl->value);
		err = 0;
		break;
    }

    if((ctrl->value != state->m_focus_mode)&&
        (ctrl->value+3 != state->m_focus_mode))
    {
        /* AF initial position */
        m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0A, 0x00, 0x01);           
    }
    
    state->m_focus_mode = ctrl->value;
    
    if(err == 0)
        m5moLS_msg(&client->dev, "%s: Done\n",__func__);
    else
        m5moLS_info(&client->dev,"%s: Failed :::: err = %d\n",__func__,err);
        return err;  
}

static int m5moLS_set_vintage_mode(struct v4l2_subdev *sd)
{
    struct m5moLS_state *state = to_state(sd);
    struct i2c_client *client = v4l2_get_subdevdata(sd); 
    m5moLS_msg(&client->dev, "%s: Enter : vintage mode %d\n", __func__, state->vintage_mode);
    int err = -1;
    
    switch(state->vintage_mode)
    {
        case VINTAGE_MODE_OFF:
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x50, 0x00);
            break;

        case VINTAGE_MODE_NORMAL:
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x51, 0x00);            
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x50, 0x01);
            //err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x50, 0x00);
            break;

        case VINTAGE_MODE_WARM:
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x51, 0x01);            
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x50, 0x01);            
            break;

        case VINTAGE_MODE_COOL:
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x51, 0x02);            
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x50, 0x01);            
            break;            

        case VINTAGE_MODE_BNW:
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x51, 0x03);            
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x50, 0x01);            
            break;        

        default:
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x50, 0x00);
            m5moLS_info(&client->dev," not setting\n");
            break;
    }
    return 0;
}

static int m5moLS_set_face_beauty(struct v4l2_subdev *sd)
{
    struct m5moLS_state *state = to_state(sd);
    struct i2c_client *client = v4l2_get_subdevdata(sd); 
    m5moLS_msg(&client->dev, "%s: Enter : face beauty %d\n", __func__, state->beauty_mode);
    int err = -1;

    if(state->beauty_mode)
        err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x53, 0x01);
    else
        err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x53, 0x00);
    
    return err;
}

static int m5moLS_set_white_balance(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    int err = -1;
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct m5moLS_state *state = to_state(sd);

	m5moLS_msg(&client->dev, "%s: Enter : white balance %d\n", __func__, ctrl->value);

    if(state->m_white_balance_mode == ctrl->value)
        return 0;
    
	switch(ctrl->value)
	{
		case WHITE_BALANCE_AUTO:
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x06, 0x02, 0x01);
            break;

		case WHITE_BALANCE_SUNNY:
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x06, 0x02, 0x02);
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x06, 0x03, 0x04);
            break;

		case WHITE_BALANCE_CLOUDY:
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x06, 0x02, 0x02);
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x06, 0x03, 0x05);
            break;

		case WHITE_BALANCE_TUNGSTEN:
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x06, 0x02, 0x02);
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x06, 0x03, 0x01);
            break;

		case WHITE_BALANCE_FLUORESCENT:
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x06, 0x02, 0x02);
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x06, 0x03, 0x02);
            break;

		default:
			m5moLS_info(&client->dev, "%s: failed: to set_white_balance, enum: %d\n", __func__, ctrl->value);
			err =  -EINVAL;
            break;
	}

    if(err == 0){
            state->m_white_balance_mode = ctrl->value;
	    m5moLS_msg(&client->dev, "%s: done\n", __func__);
    }
    else
        m5moLS_info(&client->dev,"%s ::: failed\n",__func__);

	return err;
}

static int m5moLS_set_ev(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    int err = -1;
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct m5moLS_state *state = to_state(sd);

	m5moLS_msg(&client->dev, "%s: Enter : brightness = %d\n", __func__, ctrl->value);

    if(state->m_ev_mode == ctrl->value)
        return 0;
    
    switch(ctrl->value)
    {
    case EV_MINUS_4:
        err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x38, 0x00);
        break;
        
    case EV_MINUS_3:
        err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x38, 0x01);
        break;
    
    case EV_MINUS_2:
        err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x38, 0x02);
        break;
    
    case EV_MINUS_1:
        err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x38, 0x03);
        break;
    
    case EV_DEFAULT:
        err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x38, 0x04);
        break;
    
    case EV_PLUS_1:
        err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x38, 0x05);
        break;
    
    case EV_PLUS_2:
        err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x38, 0x06);
        break;  
    
    case EV_PLUS_3:
        err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x38, 0x07);
        break;
    
    case EV_PLUS_4:
        err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x38, 0x08);
        break;
        
    default:
        m5moLS_msg(&client->dev,"[EV]Invalid value is ordered!!!\n");
        err = -EINVAL;
        break;
    }

    if(err == 0){
       state->m_ev_mode = ctrl->value;
    	m5moLS_msg(&client->dev, "%s: done, brightness: %d\n", __func__, ctrl->value);
    }
    else
        m5moLS_info(&client->dev,"%s ::: failed\n",__func__);

    return err;
}


static int m5moLS_set_metering(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    int err = -1;
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct m5moLS_state *state = to_state(sd);

    m5moLS_msg(&client->dev, "%s: Enter : metering %d\n", __func__, ctrl->value);
    
    if((state->m_metering_mode == ctrl->value)||
        (state->m_movie_mode == 0x01)) // 0x01: movie_mode, 0x00 :  Moniter_mode
        return 0;
    
    switch(ctrl->value)
    {
        case METERING_MATRIX:
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x01, 0x01);
            break;

        case METERING_CENTER:
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x01, 0x03);
            break;

        case METERING_SPOT:
            err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x01, 0x06);
            break;

        default:
            dev_err(&client->dev, "%s: failed: to set_photometry, enum: %d\n", __func__, ctrl->value);
            err = -EINVAL;
            break;
    }

    if(err == 0){
        state->m_metering_mode = ctrl->value;
        m5moLS_msg(&client->dev, "%s: done\n", __func__);
    }
    else
        m5moLS_info(&client->dev,"%s ::: failed\n",__func__);

    return err;
}

static int m5moLS_set_iso(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	int err = -1;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
       struct m5moLS_state *state = to_state(sd);

	m5moLS_msg(&client->dev, "%s: Enter : iso %d\n", __func__, ctrl->value);

       if(state->m_iso_mode == ctrl->value)
            return 0;
       
	switch(ctrl->value)
	{
		case ISO_AUTO:
			err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x05, 0x00);
	        	break;

		case ISO_50:
			err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x05, 0x01);
	    		break;

		case ISO_100:
			err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x05, 0x02);
	    		break;

		case ISO_200:
			err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x05, 0x03);
	    		break;

		case ISO_400:
			err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x05, 0x04);
	    		break;

		case ISO_800:
			err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x05, 0x05);
	    		break;

		/* sunggeun : temp */
		case ISO_SPORTS:
		case ISO_NIGHT:
		case ISO_MOVIE:
			err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x05, 0x00);;
			break;
			
		default:
			dev_err(&client->dev, "%s: failed: to set_iso, enum: %d\n", __func__, ctrl->value);
			err =  -EINVAL;
        		break;
	}

    if(err == 0){
        state->m_iso_mode = ctrl->value;
    	m5moLS_msg(&client->dev, "%s: done, iso: 0x%02x\n", __func__, ctrl->value);
    }
    else
        m5moLS_info(&client->dev,"%s ::: failed\n",__func__);

	return err;
}

static int m5moLS_set_gamma(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	/*[5b] Need to do later*/
	return 0;
}

static int m5moLS_set_slow_ae(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	/*[5b] Need to do later*/
	return 0;
}

static void m5moLS_start_auto_focus(struct v4l2_subdev *sd)
{
    int err;
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct m5moLS_state *state = to_state(sd);

    int orgmode = 0, af_status = 0, intr_value=0;
    af_status = state->userset.lens.af_status;

    orgmode = m5moLS_get_mode(sd);
    if(orgmode != M5MO_LS_MONITOR_MODE)
    {
        state->userset.lens.af_status = af_status;
        m5moLS_msg(&client->dev," camera module mode is not preview!! Don't working AF!n");
        return 0;
    }

    m5moLS_msg(&client->dev,"Start AF !!!\n");
    state->userset.lens.af_status = M5MO_LS_AF_START;

    /* Interrupt Clear */
    m5moLS_read_category_parm(client, M5MO_LS_8BIT, 0x00, 0x10, &intr_value);        
    /* AF interrupt enable */
    m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x00, 0x11, M5MO_LS_INT_AF); 
    /* AF Start Operation */
    m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0A, 0x02, 0x01);
    
    m5moLS_wait_interrupt(sd, M5MO_LS_INT_AF, 1000);

}

static void m5moLS_stop_auto_focus(struct v4l2_subdev *sd)
{
    int err;
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct m5moLS_state *state = to_state(sd);

    int orgmode = 0, af_status = 0, intr_value=0;
    af_status = state->userset.lens.af_status;

    orgmode = m5moLS_get_mode(sd);
    if(orgmode != M5MO_LS_MONITOR_MODE)
    {
        state->userset.lens.af_status = af_status;
        m5moLS_info(&client->dev," camera module mode is not preview!! Don't working AF!n");
        return 0;
    }

    state->userset.lens.af_status = M5MO_LS_AF_STOP;

    /* AF Stop Operation */
    m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0A, 0x02, 0x00);
    /* disable AF interrupt */
    m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x00, 0x11, 0x00); 
    /* Clear Interrupt flag */
    m5moLS_read_category_parm(client, M5MO_LS_8BIT, 0x00, 0x10, &intr_value);        

    //cam_interrupted = 1;
    //wake_up_interruptible(&cam_wait);

    /* AE/AWB Unlock */
    if(m5moLS_set_ae_lock(sd,M5MO_LS_AE_UNLOCK) != 0)
        return -EINVAL;
    if(m5moLS_set_awb_lock(sd,M5MO_LS_AWB_UNLOCK) != 0)
        return -EINVAL;  

    msleep(5);

    m5moLS_msg(&client->dev,"Stop AF !!!\n");
}

static int m5moLS_get_auto_focus_status(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct m5moLS_state *state = to_state(sd);
    u32 val = 0;

    if(state->userset.lens.af_status == M5MO_LS_AF_STOP)
    {
        return M5MO_LS_AF_RELEASE;
    }
    
    m5moLS_read_category_parm(client, M5MO_LS_8BIT, 0x0A, 0x03, &val);
    if(val == 1)
        ctrl->value = M5MO_LS_AF_STATUS_MOVING;
    else if(val == 0)
        ctrl->value = M5MO_LS_AF_STATUS_FAILED; 
    else if(val == 2)
        ctrl->value = M5MO_LS_AF_STATUS_SUCCESS;

    m5moLS_msg(&client->dev,"AF status reading... %d \n", ctrl->value);

    if(ctrl->value == M5MO_LS_AF_STATUS_FAILED || ctrl->value == M5MO_LS_AF_STATUS_SUCCESS)
    { 
        if((ctrl->value == M5MO_LS_AF_STATUS_SUCCESS)&&(state->touch_af_mode == 0))
        {
            /* AE/AWB Lock */
            if(m5moLS_set_ae_lock(sd,M5MO_LS_AE_LOCK) != 0)
                return -EINVAL;
            if(m5moLS_set_awb_lock(sd,M5MO_LS_AWB_LOCK) != 0)
                return -EINVAL;
        } 

        if(state->touch_af_mode)
            state->touch_af_mode = 0;
        
        m5moLS_msg(&client->dev, "AF result = %d (2.success, 1. moving, 0.fail), af_status = %d\n", ctrl->value,state->userset.lens.af_status);
    }

    return ctrl->value;
}


static void m5moLS_init_parameters(struct v4l2_subdev *sd)
{
        struct i2c_client *client = v4l2_get_subdevdata(sd);
        struct m5moLS_state *state = to_state(sd);

        /* Set initial values for the sensor stream parameters */
        state->strm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        state->strm.parm.capture.timeperframe.numerator = 1;
        state->strm.parm.capture.capturemode = 0;

        //state->framesize_index = M5MO_LS_PREVIEW_VGA;
        state->fps = 30; /* Default value */
        state->anti_banding = ANTI_BANDING_60HZ;
        
        state->jpeg.enable = 0;
        state->jpeg.quality = 100;
        state->jpeg.main_offset = 0;
        state->jpeg.main_size = 0;
        state->jpeg.thumb_offset = 0;
        state->jpeg.thumb_size = 0;
        state->jpeg.postview_offset = 0;
        
        cam_interrupted = 0;
        
        m5moLS_msg(&client->dev, "%s:Done!!!\n", __func__);
}

//s1_camera [ Defense process by ESD input ] _[
static int m5moLS_reset(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = -EINVAL;

	dev_err(&client->dev, "%s: Enter \n", __func__);

	err = m5moLS_power_en(sd, 0);
	if(err < 0){	
		dev_err(&client->dev, "%s: failed: m5moLS_power_en(off)\n", __func__);
		return -EIO;
    	}

	mdelay(5);
	
	err = m5moLS_power_en(sd, 1);
	if(err < 0){	
		dev_err(&client->dev, "%s: failed: m5moLS_power_en(off)\n", __func__);
		return -EIO;
    	}

	err = m5moLS_init(sd,NULL);
	if(err < 0){
		dev_err(&client->dev, "%s: Failed: Camera Initialization\n", __func__);
		return -EIO;
	}
       err = m5moLS_set_preview_start(sd);
	return 0;
}
// _]

#if 0
/* Sample code */
static const char *m5moLS_querymenu_wb_preset[] = {
	"WB Tungsten", "WB Fluorescent", "WB sunny", "WB cloudy", NULL
};
#endif

static struct v4l2_queryctrl m5moLS_controls[] = {
#if 0
	/* Sample code */
	{
		.id = V4L2_CID_WHITE_BALANCE_PRESET,
		.type = V4L2_CTRL_TYPE_MENU,
		.name = "White balance preset",
		.minimum = 0,
		.maximum = ARRAY_SIZE(m5moLS_querymenu_wb_preset) - 2,
		.step = 1,
		.default_value = 0,
	},
#endif
};

static int m5moLS_enable_interrupt_pin(struct v4l2_subdev *sd)
{
    int err = -1;
    struct i2c_client *client = v4l2_get_subdevdata(sd);

    m5moLS_msg(&client->dev, "%s\n", __func__);		

    err =gpio_request(GPIO_CAM_MEGA_EN,"GPJ1");
    if(err)
        goto err_request_int;
    

    set_irq_type(IRQ_M5MO_LS_CAM_MEGA_INT, IRQ_TYPE_EDGE_RISING);
	s3c_gpio_cfgpin(GPIO_CAM_MEGA_EN, S3C_GPIO_SFN(0xf));
    return 0;
    
    err_request_int:
        gpio_free(GPIO_CAM_MEGA_EN);
        return err;
}

static void m5moLS_disable_interrupt_pin(struct v4l2_subdev *sd)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    m5moLS_msg(&client->dev, "%s\n", __func__);		

    s3c_gpio_setpull(GPIO_CAM_MEGA_EN, S3C_GPIO_PULL_DOWN);
    set_irq_type(IRQ_M5MO_LS_CAM_MEGA_INT, IRQ_TYPE_NONE);
    gpio_free(GPIO_CAM_MEGA_EN);

    free_irq(IRQ_M5MO_LS_CAM_MEGA_INT,NULL);    
}


const char **m5moLS_ctrl_get_menu(u32 id)
{
	switch (id) {
#if 0
	/* Sample code */
	case V4L2_CID_WHITE_BALANCE_PRESET:
		return m5moLS_querymenu_wb_preset;
#endif
	default:
		return v4l2_ctrl_get_menu(id);
	}
}

static inline struct v4l2_queryctrl const *m5moLS_find_qctrl(int id)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(m5moLS_controls); i++)
		if (m5moLS_controls[i].id == id)
			return &m5moLS_controls[i];

	return NULL;
}

static int m5moLS_queryctrl(struct v4l2_subdev *sd, struct v4l2_queryctrl *qc)
{
	int i;
         struct i2c_client *client = v4l2_get_subdevdata(sd);
	dev_err(&client->dev, "%s\n", __func__);

	for (i = 0; i < ARRAY_SIZE(m5moLS_controls); i++) {
		if (m5moLS_controls[i].id == qc->id) {
			memcpy(qc, &m5moLS_controls[i], sizeof(struct v4l2_queryctrl));
			return 0;
		}
	}

	return -EINVAL;
}

static int m5moLS_querymenu(struct v4l2_subdev *sd, struct v4l2_querymenu *qm)
{
	struct v4l2_queryctrl qctrl;
         struct i2c_client *client = v4l2_get_subdevdata(sd);
	dev_err(&client->dev, "%s\n", __func__);

	qctrl.id = qm->id;
	m5moLS_queryctrl(sd, &qctrl);

	return v4l2_ctrl_query_menu(qm, &qctrl, m5moLS_ctrl_get_menu(qm->id));
}

/*
 * Clock configuration
 * Configure expected MCLK from host and return EINVAL if not supported clock
 * frequency is expected
 * 	freq : in Hz
 * 	flag : not supported for now
 */
static int m5moLS_s_crystal_freq(struct v4l2_subdev *sd, u32 freq, u32 flags)
{
	int err = -EINVAL;

	return err;
}

static int m5moLS_g_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
	int err = 0;

	return err;
}

static int m5moLS_get_framesize_index(struct v4l2_subdev *sd);
static int m5moLS_set_framesize_index(struct v4l2_subdev *sd, unsigned int index);
/* Information received: 
 * width, height
 * pixel_format -> to be handled in the upper layer 
 *
 * */
static int m5moLS_s_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
	int err = 0;
	struct m5moLS_state *state = to_state(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int framesize_index = -1;

	dev_err(&client->dev, "%s\n", __func__);

	if(fmt->fmt.pix.pixelformat == V4L2_PIX_FMT_JPEG && fmt->fmt.pix.colorspace != V4L2_COLORSPACE_JPEG){
		dev_err(&client->dev, "%s: mismatch in pixelformat and colorspace\n", __func__);
		return -EINVAL;
	}

	if(fmt->fmt.pix.width == 800 && fmt->fmt.pix.height == 448) {
		state->pix.width = 1280;
		state->pix.height = 720;
	}

	else {
		state->pix.width = fmt->fmt.pix.width;
		state->pix.height = fmt->fmt.pix.height;
	}
	
	state->pix.pixelformat = fmt->fmt.pix.pixelformat;

	if(fmt->fmt.pix.colorspace == V4L2_COLORSPACE_JPEG)
		state->oprmode = M5MO_LS_OPRMODE_IMAGE;
	else
		state->oprmode = M5MO_LS_OPRMODE_VIDEO; 


	framesize_index = m5moLS_get_framesize_index(sd);

       if(framesize_index < M5MO_LS_PREVIEW_QCIF || framesize_index >  M5MO_LS_CAPTURE_8MP)
       {
            m5moLS_info(&client->dev,"framesize_index value is invalid : %d\n",framesize_index);
            if(fmt->fmt.pix.colorspace == V4L2_COLORSPACE_JPEG)
                framesize_index = M5MO_LS_CAPTURE_8MP;
       
       }
	m5moLS_msg(&client->dev, "%s:framesize_index = %d\n", __func__, framesize_index);
	
	err = m5moLS_set_framesize_index(sd, framesize_index);
	if(err < 0){
		dev_err(&client->dev, "%s: set_framesize_index failed : frameszie_index = %d,\n", __func__,framesize_index);
		return -EINVAL;
	}

        m5moLS_set_preview_resolution(sd);
       
	if(state->pix.pixelformat == V4L2_PIX_FMT_JPEG){
		state->jpeg.enable = 1;
	} else {
		state->jpeg.enable = 0;
	}
/*[5b] Need to do later*/
/*
	if(state->oprmode == M5MO_LS_OPRMODE_VIDEO)
	{
		if(framesize_index == M5MO_LS_PREVIEW_720P)
		{
			state->hd_preview_on = 1;
		}
		else
		{
			state->hd_preview_on = 0;
		}
	}
*/	
	return 0;
}

static int m5moLS_enum_framesizes(struct v4l2_subdev *sd, \
					struct v4l2_frmsizeenum *fsize)
{
	struct m5moLS_state *state = to_state(sd);
	int num_entries = sizeof(m5moLS_framesize_list)/sizeof(struct m5moLS_enum_framesize);	
	struct m5moLS_enum_framesize *elem;	
	int index = 0;
	int i = 0;

	/* The camera interface should read this value, this is the resolution
 	 * at which the sensor would provide framedata to the camera i/f
 	 *
 	 * In case of image capture, this returns the default camera resolution (VGA)
 	 */
	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;

	if(state->pix.pixelformat == V4L2_PIX_FMT_JPEG){
		index = M5MO_LS_PREVIEW_VGA;
	} else {
		index = state->framesize_index;
	}

	for(i = 0; i < num_entries; i++){
		elem = &m5moLS_framesize_list[i];
		if(elem->index == index){
			fsize->discrete.width = m5moLS_framesize_list[index].width;
			fsize->discrete.height = m5moLS_framesize_list[index].height;
			return 0;
		}
	}

	return -EINVAL;
}

static int m5moLS_enum_frameintervals(struct v4l2_subdev *sd, 
					struct v4l2_frmivalenum *fival)
{
	int err = 0;

	return err;
}

static int m5moLS_enum_fmt(struct v4l2_subdev *sd, struct v4l2_fmtdesc *fmtdesc)
{
	int num_entries;
         struct i2c_client *client = v4l2_get_subdevdata(sd);

	num_entries = sizeof(capture_fmts)/sizeof(struct v4l2_fmtdesc);
    
	dev_err(&client->dev, "%s\n", __func__);

	if(fmtdesc->index >= num_entries)
		return -EINVAL;

        memset(fmtdesc, 0, sizeof(*fmtdesc));
        memcpy(fmtdesc, &capture_fmts[fmtdesc->index], sizeof(*fmtdesc));

	return 0;
}

static int m5moLS_try_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
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
static int m5moLS_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct m5moLS_state *state = to_state(sd);
	int err = 0;

	state->strm.parm.capture.timeperframe.numerator = 1;
	state->strm.parm.capture.timeperframe.denominator = state->fps;

	memcpy(param, &state->strm, sizeof(param));

	return err;
}

/** Sets the FPS value */
static int m5moLS_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	int err = 0;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct m5moLS_state *state = to_state(sd);

	dev_err(&client->dev, "%s\n", __func__);

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
 * It returns the index (enum m5moLS_frame_size) of the framesize entry.
 */
static int m5moLS_get_framesize_index(struct v4l2_subdev *sd)
{
	int i = 0;
	struct m5moLS_state *state = to_state(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct m5moLS_enum_framesize *frmsize;

	m5moLS_info(&client->dev, "%s: Requested Res: %dx%d\n", __func__, state->pix.width, state->pix.height);

	/* Check for video/image mode */
	for(i = 0; i < (sizeof(m5moLS_framesize_list)/sizeof(struct m5moLS_enum_framesize)); i++)
	{
		frmsize = &m5moLS_framesize_list[i];

		if(frmsize->mode != state->oprmode)
			continue;

		if(state->oprmode == M5MO_LS_OPRMODE_IMAGE)
		{
			/* In case of image capture mode, if the given image resolution is not supported,
 			 * return the next higher image resolution. */
 			//printk("frmsize->width(%d) state->pix.width(%d) frmsize->height(%d) state->pix.height(%d)\n", frmsize->width, state->pix.width, frmsize->height, state->pix.height);
			if(frmsize->width == state->pix.width && frmsize->height == state->pix.height)
			{
				//printk("frmsize->index(%d) \n", frmsize->index);
				return frmsize->index;
			}
		}
		else 
		{
			/* In case of video mode, if the given video resolution is not matching, use
 			 * the default rate (currently M5MO_LS_PREVIEW_VGA).
 			 */		 
 			//printk("frmsize->width(%d) state->pix.width(%d) frmsize->height(%d) state->pix.height(%d)\n", frmsize->width, state->pix.width, frmsize->height, state->pix.height);
			if(frmsize->width == state->pix.width && frmsize->height == state->pix.height)
			{
				//printk("frmsize->index(%d) \n", frmsize->index);
				return frmsize->index;
		}
	} 
	} 
	/* If it fails, return the default value. */
	return (state->oprmode == M5MO_LS_OPRMODE_IMAGE) ? M5MO_LS_CAPTURE_8MP : M5MO_LS_PREVIEW_VGA;
}


/* This function is called from the s_ctrl api
 * Given the index, it checks if it is a valid index.
 * On success, it returns 0.
 * On Failure, it returns -EINVAL
 */
static int m5moLS_set_framesize_index(struct v4l2_subdev *sd, unsigned int index)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct m5moLS_state *state = to_state(sd);
	int i = 0;

	for(i = 0; i < (sizeof(m5moLS_framesize_list)/sizeof(struct m5moLS_enum_framesize)); i++)
	{
		if(m5moLS_framesize_list[i].index == index && m5moLS_framesize_list[i].mode == state->oprmode){
			state->framesize_index = m5moLS_framesize_list[i].index;	
			state->pix.width = m5moLS_framesize_list[i].width;
			state->pix.height = m5moLS_framesize_list[i].height;
			m5moLS_info(&client->dev, "%s: Camera Res: %dx%d\n", __func__, state->pix.width, state->pix.height);
			return 0;
		} 
	} 
	
       m5moLS_info(&client->dev, "index = %d, state->oprmode = %d\n",index,state->oprmode);
	return -EINVAL;
}



static int m5moLS_set_ev_program_mode(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	int mon_err = -1;
	int cap_err = -1;

	struct i2c_client *client = v4l2_get_subdevdata(sd); 
       struct m5moLS_state *state = to_state(sd);

	m5moLS_msg(&client->dev, "%s: Scene Mode = %d\n",__func__,ctrl->value);

       if((state->m_ev_program_mode == ctrl->value)||
          (state->m_movie_mode == 0x01))    // 0x01 : movie_mode, 0x00 : moniter_mode
            return 0;
       
	switch(ctrl->value) {
		case SCENE_MODE_NONE:
			mon_err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x0A, 0x00);	// Monitor EV program mode setting
			cap_err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x0B, 0x00);	// Still Capture EV program mode setting
			break;

		case SCENE_MODE_PORTRAIT:            
			mon_err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x0A, 0x01);	// Monitor EV program mode setting
			cap_err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x0B, 0x01);	// Still Capture EV program mode setting
			break;

		case SCENE_MODE_LANDSCAPE:            
			mon_err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x0A, 0x02);	// Monitor EV program mode setting
			cap_err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x0B, 0x02);	// Still Capture EV program mode setting
			break;

		case SCENE_MODE_SPORTS:            
			mon_err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x0A, 0x03);	// Monitor EV program mode setting
			cap_err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x0B, 0x03);	// Still Capture EV program mode setting
			break;

		case SCENE_MODE_PARTY_INDOOR:            
			mon_err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x0A, 0x04);	// Monitor EV program mode setting
			cap_err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x0B, 0x04);	// Still Capture EV program mode setting
			break;

		case SCENE_MODE_BEACH_SNOW:            
			mon_err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x0A, 0x05);	// Monitor EV program mode setting
			cap_err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x0B, 0x05);	// Still Capture EV program mode setting
			break;

		case SCENE_MODE_SUNSET:            
			mon_err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x0A, 0x06);	// Monitor EV program mode setting
			cap_err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x0B, 0x06);	// Still Capture EV program mode setting
			break;

		case SCENE_MODE_DUSK_DAWN:            
			mon_err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x0A, 0x07);	// Monitor EV program mode setting
			cap_err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x0B, 0x07);	// Still Capture EV program mode setting
			break;
			
		case SCENE_MODE_FALL_COLOR:            
			mon_err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x0A, 0x08);	// Monitor EV program mode setting
			cap_err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x0B, 0x08);	// Still Capture EV program mode setting
			break;

		case SCENE_MODE_NIGHTSHOT:            
			mon_err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x0A, 0x09);	// Monitor EV program mode setting
			cap_err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x0B, 0x09);	// Still Capture EV program mode setting
			break;

		case SCENE_MODE_BACK_LIGHT:            
			mon_err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x0A, 0x0A);	// Monitor EV program mode setting
			cap_err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x0B, 0x0A);	// Still Capture EV program mode setting
			break;

		case SCENE_MODE_FIREWORKS:            
			mon_err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x0A, 0x0B);	// Monitor EV program mode setting
			cap_err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x0B, 0x0B);	// Still Capture EV program mode setting
			break;

		case SCENE_MODE_TEXT:            
			mon_err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x0A, 0x0C);	// Monitor EV program mode setting
			cap_err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x0B, 0x0C);	// Still Capture EV program mode setting
			break;

		case SCENE_MODE_CANDLE_LIGHT:            
			mon_err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x0A, 0x0D);	// Monitor EV program mode setting
			cap_err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x03, 0x0B, 0x0D);	// Still Capture EV program mode setting
			break;
			
		default:
			m5moLS_msg(&client->dev, "%s: Invalid Scene Mode\n",__func__, ctrl->value);
			mon_err = 0;
			cap_err = 0;
			break;
		}

		if(mon_err == 0 && cap_err == 0) {
                     state->m_ev_program_mode = ctrl->value;
			m5moLS_msg(&client->dev, "%s: Done\n",__func__);

			return 0;
		}

		else {
			m5moLS_msg(&client->dev,"%s: mon_err = %d, cap_err = %d\n", __func__, mon_err, cap_err);

			return -1;
		}
		
		return 0;  
}

static int m5moLS_set_movie_mode(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    int mode_err = -1, orgmode = 0, rval=0, mode_val = 0;
    struct i2c_client *client = v4l2_get_subdevdata(sd); 
    struct m5moLS_state *state = to_state(sd);
    
    m5moLS_msg(&client->dev,"%s ::: mode_value = %d\n",__func__, mode_val);

    mode_val = ctrl->value;

    if(state->m_movie_mode == mode_val)
        return 0;
    
    orgmode = m5moLS_get_mode(sd);
    rval = m5moLS_set_mode(sd, M5MO_LS_PARMSET_MODE);
    if(rval)
    {
        ERR("Can not set operation mode\n");
        return rval;
    }

    // 0 : Monitor_mode , 1 : Movie_mode
    mode_err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x01, 0x32, mode_val);
    
    if(orgmode == M5MO_LS_MONITOR_MODE)
    {
        rval = m5moLS_set_mode(sd, M5MO_LS_MONITOR_MODE);
        if(rval)
        {
            ERR("Can not start preview\n");
            return rval;
        }
    }
    state->m_movie_mode = mode_val;
    
    return mode_err;
}


static int m5moLS_set_mcc_mode(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	int err = -1;

	struct i2c_client *client = v4l2_get_subdevdata(sd); 
       struct m5moLS_state *state = to_state(sd);

	m5moLS_msg(&client->dev, "%s: Scene Mode = %d\n",__func__,ctrl->value);

       if(state->m_mcc_mode == ctrl->value)
            return 0;
       
	if(ctrl->value == SCENE_MODE_NONE) {
		err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x1D, 0x01);	// Still capture color effect using multi-axis color space conversio
	}

	else {
		err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x1D, 0x00);	// Still capture color effect using multi-axis color space conversio
	}

       state->m_mcc_mode = ctrl->value;
       
	m5moLS_msg(&client->dev,"%s: err = %d\n", __func__, err);

	return err;  
}



static int m5moLS_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct m5moLS_state *state = to_state(sd);
    int err = -ENOIOCTLCMD;
    u32 offset = V4L2_CID_PRIVATE_BASE;

	switch (ctrl->id) {
    	case V4L2_CID_EXPOSURE:
    		//ctrl->value = userset.exposure_bias;
    		err = 0;
    		break;

    	case V4L2_CID_AUTO_WHITE_BALANCE:
    		//ctrl->value = userset.auto_wb;
    		err = 0;
    		break;

    	case V4L2_CID_WHITE_BALANCE_PRESET:
    		//ctrl->value = userset.manual_wb;
    		err = 0;
    		break;

    	case V4L2_CID_COLORFX:
    		//ctrl->value = userset.effect;
    		err = 0;
    		break;

    	case V4L2_CID_CONTRAST:
    		//ctrl->value = userset.contrast;
    		err = 0;
    		break;

    	case V4L2_CID_SATURATION:
    		//ctrl->value = userset.saturation;
    		err = 0;
    		break;

    	case V4L2_CID_SHARPNESS:
    		//ctrl->value = userset.sharpness;
    		err = 0;
    		break;

    	case V4L2_CID_CAM_JPEG_MAIN_SIZE:
    		err = m5moLS_get_capture_size(sd, ctrl);
    		break;
    	
    	case V4L2_CID_CAM_JPEG_MAIN_OFFSET:
    	    ctrl->value = state->jpeg.main_offset;
    		err = 0;
    		break;

    	case V4L2_CID_CAM_JPEG_THUMB_SIZE:
    		err = m5moLS_get_thumb_size(sd, ctrl);
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
    		//ctrl->value = state->jpeg.quality;
    		err = 0;
    		break;

    	case V4L2_CID_CAMERA_OBJ_TRACKING_STATUS:
    		//err = m5moLS_get_object_tracking(sd, ctrl);
    		//ctrl->value = state->ot_status;
    		err = 0;
    		break;

    	case V4L2_CID_CAMERA_SMART_AUTO_STATUS:
    		//err = m5moLS_get_smart_auto_status(sd, ctrl);
    		//ctrl->value = state->sa_status;
    		err = 0;
    		break;

    	case V4L2_CID_CAMERA_AUTO_FOCUS_RESULT:
    		err = m5moLS_get_auto_focus_status(sd, ctrl);
    		break;

    	case V4L2_CID_CAM_MODULE_COMP:
    		ctrl->value = state->camera_fw_info[0];
    		err = 0;
    		break; 

    	case V4L2_CID_CAM_MODULE_VER:
    		ctrl->value = state->camera_fw_info[1];
    		err = 0;
    		break; 

    	case V4L2_CID_CAM_DATE_INFO_YEAR:
    		ctrl->value = state->camera_fw_info[2];
    		err = 0;
    		break; 

    	case V4L2_CID_CAM_DATE_INFO_MONTH:
    		ctrl->value = state->camera_fw_info[3];
    		err = 0;
    		break; 

    	case V4L2_CID_CAM_DATE_INFO_DATE1:
    		ctrl->value = state->camera_fw_info[4];
    		err = 0;
    		break; 

    	case V4L2_CID_CAM_DATE_INFO_DATE2:
    		ctrl->value = state->camera_fw_info[5];
    		err = 0;
    		break; 
            
       case V4L2_CID_FW_UPDATE:
            ctrl->value = fw_update_status;
            err = 0;
            break;
			
		case V4L2_CID_CAMERA_FLASH_CHECK:
			ctrl->value = state->exif_info.info_flash; 
			err = 0;		
			break;
			
		case V4L2_CID_CAMERA_GET_ISO:
			ctrl->value = state->exif_info.info_iso; 
			err = 0;		
			break;

		case V4L2_CID_CAMERA_GET_SHT_TIME_NUM:
			ctrl->value = state->exif_info.info_exptime_numer; 
			err = 0;		
			break;			

		case V4L2_CID_CAMERA_GET_SHT_TIME_DEN:
			ctrl->value = state->exif_info.info_exptime_denumer; 
			err = 0;		
			break;	

		case V4L2_CID_CAMERA_GET_TV_NUM:
			ctrl->value = state->exif_info.info_tv_numer; 
			err = 0;		
			break;

		case V4L2_CID_CAMERA_GET_TV_DEN:
			ctrl->value = state->exif_info.info_tv_denumer; 
			err = 0;		
			break;
		
		case V4L2_CID_CAMERA_GET_AV_NUM:
			ctrl->value = state->exif_info.info_av_numer; 
			err = 0;		
			break;	

		case V4L2_CID_CAMERA_GET_AV_DEN:
			ctrl->value = state->exif_info.info_av_denumer; 
			err = 0;		
			break;	

		case V4L2_CID_CAMERA_GET_BV_NUM:
			ctrl->value = state->exif_info.info_bv_numer; 
			err = 0;		
			break;	
			
		case V4L2_CID_CAMERA_GET_BV_DEN:
			ctrl->value = state->exif_info.info_bv_denumer; 
			err = 0;		
			break;	
				
		case V4L2_CID_CAMERA_GET_EBV_NUM:
			ctrl->value = state->exif_info.info_ebv_numer; 
			err = 0;		
			break;	
			
		case V4L2_CID_CAMERA_GET_EBV_DEN:
			ctrl->value = state->exif_info.info_ebv_denumer; 
			err = 0;		
			break;	

    	default:
            m5moLS_msg(&client->dev, "%s: no such control:::: g_ctrl:id(%d)\n", __func__, (ctrl->id - offset));
			err = 0;
    		break;
	}
       m5moLS_msg(&client->dev, "%s :::: ctrl->id=%d, ctrl->value = %d \n", __func__,(ctrl->id - offset),ctrl->value);
	
	return err;
}

static int m5moLS_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct m5moLS_state *state = to_state(sd);
    int err = -ENOIOCTLCMD;
    int offset = V4L2_CID_PRIVATE_BASE;

    m5moLS_msg(&client->dev, "%s :::: ctrl->id=%d, ctrl->value = %d \n", __func__,(ctrl->id - offset),ctrl->value);

    switch (ctrl->id) {
        case V4L2_CID_CAMERA_ANTI_SHAKE:
            err = m5moLS_set_anti_shake(sd, ctrl);
            break;
            
        case V4L2_CID_CAMERA_BRIGHTNESS:
            err = m5moLS_set_ev(sd, ctrl);
            break;

        case V4L2_CID_CAMERA_FLASH_MODE:
            err = m5moLS_set_flash_capture(sd, ctrl);
            break;

        case V4L2_CID_CAMERA_CONTRAST:
            err = m5moLS_set_contrast(sd, ctrl);
            break;

        case V4L2_CID_CAMERA_SHARPNESS:
            err = m5moLS_set_sharpness(sd, ctrl);
            break;

        case V4L2_CID_CAMERA_WDR:
            err = m5moLS_set_wdr(sd, ctrl);
            break;

        case V4L2_CID_CAMERA_FACE_DETECTION:
            err = m5moLS_set_face_detection(sd, ctrl);
            break;
        
        case V4L2_CID_CAMERA_ZOOM:
            err = m5moLS_set_dzoom(sd, ctrl);
            err =0;
            break;

        case V4L2_CID_CAM_JPEG_QUALITY:
            state->jpeg.quality = ctrl->value;
            err = 0;
            break;
            
        case V4L2_CID_CAMERA_WHITE_BALANCE:
            err = m5moLS_set_white_balance(sd, ctrl);
            break;

        case V4L2_CID_CAMERA_SATURATION:
            err = m5moLS_set_saturation(sd, ctrl);
            break;

        case V4L2_CID_CAMERA_EFFECT:
            err = m5moLS_set_effect(sd, ctrl);                 
            err = 0;
            break;      
        
        case V4L2_CID_CAMERA_ISO:
            err = m5moLS_set_iso(sd, ctrl);       
            break;
        
        case V4L2_CID_CAMERA_METERING:
            err = m5moLS_set_metering(sd, ctrl);        
            break;

        case V4L2_CID_CAMERA_CAPTURE:   // get_capture_image
            err = m5moLS_start_capture_transfer(sd);
            break;

        case V4L2_CID_CAM_CAPTURE:      // start_capture
            err = m5moLS_set_capture_start(sd, ctrl);
            break;
            
        /* Used to start / stop preview operation. 
                * This call can be modified to START/STOP operation, which can be used in image capture also */
        case V4L2_CID_CAM_PREVIEW_ONOFF:
            if(ctrl->value)
            err = m5moLS_set_preview_start(sd);
            else
            err = m5moLS_set_preview_stop(sd);
            break;

        case V4L2_CID_CAMERA_CAF_START_STOP:
            err = m5moLS_set_continous_af(sd, ctrl);
            break;  

        case V4L2_CID_CAMERA_OBJECT_POSITION_X:
            state->position.x = ctrl->value;
            err = 0;
            break;
        
        case V4L2_CID_CAMERA_OBJECT_POSITION_Y:
            state->position.y = ctrl->value;
            err = 0;
            break;

        case V4L2_CID_CAMERA_TOUCH_AF_START_STOP:
            err = m5moLS_set_touch_auto_focus(sd, ctrl);
            break;

        case V4L2_CID_CAMERA_SET_AUTO_FOCUS:
            if(ctrl->value)
                m5moLS_start_auto_focus(sd);
            else
                m5moLS_stop_auto_focus(sd);
            err = 0;
            break;

        case V4L2_CID_CAMERA_FOCUS_MODE:
            err = m5moLS_set_focus_mode(sd, ctrl);
            break;

        case V4L2_CID_CAM_UPDATE_FW:
            err = m5moLS_i2c_update_firmware(sd);
            break;

        case V4L2_CID_CAM_SET_FW_ADDR:
            m5moLS_msg(&client->dev,"V4L2_CID_CAM_SET_FW_ADDR = %d",ctrl->value);
            state->fw_info.addr = ctrl->value;
            err = 0;
            break;

        case V4L2_CID_CAM_SET_FW_SIZE:
            m5moLS_msg(&client->dev,"V4L2_CID_CAM_SET_FW_SIZE = %d",ctrl->value);
            state->fw_info.size = ctrl->value;
            err = 0;
            break;

        case V4L2_CID_CAM_FW_VER:
            state->not_detect = 0;
            err = m5moLS_get_version(sd,ctrl);
            break;

        case V4L2_CID_CAM_S_FW_VER:
            state->not_detect = 1;
            err = m5moLS_get_version(sd,ctrl);
            break;
            
        case V4L2_CID_CAM_DUMP_FW:
            err = 0;
            break;

        case V4L2_CID_CAMERA_BATCH_REFLECTION:
            err = 0;        
            break;

        case V4L2_CID_CAMERA_EXIF_ORIENTATION:
            err = 0;        
            break;

        case V4L2_CID_CAMERA_RESET:
            err = m5moLS_reset(sd);        
            break;

        case V4L2_CID_CAMERA_CHECK_DATALINE:
			state->check_dataline = ctrl->value;
            err = 0;        
            break;	

        case V4L2_CID_CAMERA_CHECK_DATALINE_STOP:
			err = m5moLS_set_dtp_stop(sd);       
            break;
			
		case V4L2_CID_CAMERA_ANTI_BANDING:
			state->anti_banding = ctrl->value;
			err = 0;		
			break;

		case V4L2_CID_CAMERA_FRAME_RATE:
			err = 0;		
			break;
			
		case V4L2_CID_CAMERA_VINTAGE_MODE:
			state->vintage_mode = ctrl->value;
                     err = 0;
			break;

		case V4L2_CID_CAMERA_BEAUTY_SHOT:
			state->beauty_mode = ctrl->value;
                     err = 0;
			break;

		case V4L2_CID_CAMERA_EV_PROGRAM_MODE:
			err = m5moLS_set_ev_program_mode(sd, ctrl);
			break;

		case V4L2_CID_CAMERA_MCC_MODE:
			err = m5moLS_set_mcc_mode(sd, ctrl);
			break;
            
	       case V4L2_CID_CAMERA_MON_MOVIE_SELECT:
                    state->m_movie_mode = ctrl->value;
                    err = 0;
                    break; 
            
        default:
            dev_err(&client->dev, "%s: no such control:::: s_ctrl:id(%d)\n", __func__, (ctrl->id - offset));
            err = 0; //SecFeature.Camera aswoogi
            break;
    }

    if (err < 0)
        m5moLS_info(&client->dev, "%s: vidioc_s_ctrl failed %d, s_ctrl: id(%d), value(%d)\n", __func__, err, (ctrl->id - offset), ctrl->value);

    m5moLS_msg(&client->dev, "%s : Done!!! \n", __func__);
    return err;
}

static int m5moLS_s_ext_ctrls(struct v4l2_subdev *sd, struct v4l2_ext_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct m5moLS_state *state = to_state(sd);
	int i =0;
	int err = -ENOIOCTLCMD;
	long temp = 0;
	struct gps_info_common * tempGPSType = NULL;
	

	switch (ctrl->id) {

	case V4L2_CID_CAMERA_GPS_LATITUDE:
		tempGPSType = (struct gps_info_common *)ctrl->reserved2;
		state->gpsInfo.m5moLS_gps_buf[0] = tempGPSType ->direction;
		state->gpsInfo.m5moLS_gps_buf[1] = tempGPSType ->dgree;
		state->gpsInfo.m5moLS_gps_buf[2] = tempGPSType ->minute;
		state->gpsInfo.m5moLS_gps_buf[3] = tempGPSType ->second;
		
		//dev_err(&client->dev,"gps_info_latiude NS: %d, dgree: %d, minute: %d, second: %d \n",state->gpsInfo.m5moLS_gps_buf[0], state->gpsInfo.m5moLS_gps_buf[1], state->gpsInfo.m5moLS_gps_buf[2], state->gpsInfo.m5moLS_gps_buf[3]);
		err = 0;
		break;

	case V4L2_CID_CAMERA_GPS_LONGITUDE:
		tempGPSType = (struct gps_info_common *)ctrl->reserved2;
		state->gpsInfo.m5moLS_gps_buf[4] = tempGPSType ->direction;
		state->gpsInfo.m5moLS_gps_buf[5] = tempGPSType ->dgree;
		state->gpsInfo.m5moLS_gps_buf[6] = tempGPSType ->minute;
		state->gpsInfo.m5moLS_gps_buf[7] = tempGPSType ->second;
		
		//dev_err(&client->dev,"gps_info_longitude EW: %d, dgree: %d, minute: %d, second: %d \n", state->gpsInfo.m5moLS_gps_buf[4], state->gpsInfo.m5moLS_gps_buf[5], state->gpsInfo.m5moLS_gps_buf[6], state->gpsInfo.m5moLS_gps_buf[7]);
		err = 0;
		break;

	case V4L2_CID_CAMERA_GPS_ALTITUDE:
		tempGPSType = (struct gps_info_common *)ctrl->reserved2;
		state->gpsInfo.m5moLS_altitude_buf[0] = tempGPSType ->direction;
		state->gpsInfo.m5moLS_altitude_buf[1] = (tempGPSType ->dgree)&0x00ff; //lower byte
		state->gpsInfo.m5moLS_altitude_buf[2] = ((tempGPSType ->dgree)&0xff00)>>8; //upper byte
		state->gpsInfo.m5moLS_altitude_buf[3] = tempGPSType ->minute;//float

		//dev_err(&client->dev,"gps_info_altitude PLUS_MINUS: %d, dgree_lower: %d, degree_lower: %d, minute: %d \n", state->gpsInfo.m5moLS_altitude_buf[0], state->gpsInfo.m5moLS_altitude_buf[1], state->gpsInfo.m5moLS_altitude_buf[2], state->gpsInfo.m5moLS_altitude_buf[3]);
		err = 0;
		break;

	case V4L2_CID_CAMERA_GPS_TIMESTAMP:
		temp = *((long *)ctrl->reserved2);
		//dev_err(&client->dev, "%s: V4L2_CID_CAMERA_GPS_TIMESTAMP: %ld\n", __func__, (*((long *)ctrl->reserved)));
		state->gpsInfo.gps_timeStamp = temp;
		err = 0;
		break;
		
	case V4L2_CID_CAMERA_EXIF_TIME_INFO:
		state->exifTimeInfo =(struct tm *)ctrl->reserved2;
		err = 0;
		break;
	}

	if (err < 0)
		dev_err(&client->dev, "%s: vidioc_s_ext_ctrl failed %d\n", __func__, err);

	return err;
}

static int 
m5moLS_i2c_update_firmware(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
       struct m5moLS_state *state = to_state(sd);
	struct device *dev = &client->adapter->dev;
	const struct firmware * fw_entry;
	
	int i, j, retry, ret;
	u32 val = 0, flash_addr;
	u8 pin1 = 0x7E;
	u8 status = '1';
	int ret_val;

        m5moLS_msg(&client->dev, "%s: start\n", __func__);
	/* Set M-5MoLS Pins */
	ret_val = m5moLS_write_mem(client, 0x0001, 0x50000308, &pin1);
	if(ret_val < 0)
	{
		m5moLS_msg(&client->dev,"return failed");
		return -1;
	}
	/* M-5MoLS flash memory select */   
	ret_val = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0F, 0x13, 0x01);
	if(ret_val < 0)
	{
		m5moLS_msg(&client->dev,"return failed");
		return -1;
	}
	msleep(30);

	m5moLS_msg(&client->dev,"Firmware Update Start!!\n");
	m5moLS_write_fw_status(&status);
	flash_addr = 0x10000000;	
	for(i = 0; i < 31; i++)
	{
		/* Set Flash ROM memory address */
		m5moLS_write_category_parm(client, M5MO_LS_32BIT, 0x0F, 0x00, flash_addr);
		msleep(30);
		/* Erase FLASH ROM entire memory */
		m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0F, 0x06, 0x01);
		msleep(30);
		retry = 0;
		while(1) 
		{
			/* Response while sector-erase is operating */
			m5moLS_read_category_parm(client, M5MO_LS_8BIT, 0x0F, 0x06, &val);
			if(val == 0)	//4
				break;

			//mdelay(50);
			msleep(200);
			retry++;
		}	

		/* Set FLASH ROM programming size */
		m5moLS_write_category_parm(client, M5MO_LS_16BIT, 0x0F, 0x04, 0x0000);
		msleep(30);
		/* Clear M-5MoLS internal RAM */
		m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0F, 0x08, 0x01);
		mdelay(10);

		printk("Firmware Update is processing... %d!!\n", i);
		/* Set Flash ROM programming address */
		m5moLS_write_category_parm(client, M5MO_LS_32BIT, 0x0F, 0x00, flash_addr);
		msleep(30);
		/* Send programmed firmware */
		for(j=0; j<0x10000; j+=0x1000)
		{
			m5moLS_write_mem(client, 0x1000, 0x68000000+j, 
								((state->fw_info.addr) + (i * 0x10000) + j));
			mdelay(10);
                     m5moLS_msg(&client->dev,"Send Programmed firmware = %d, 0x%x\n",i,((state->fw_info.addr) + (i * 0x10000) + j));
		}

		/* Start Programming */
		m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0F, 0x07, 0x01);						
		msleep(30);
		/* Confirm programming has been completed */
		retry = 0;
		while(1) 
		{
			m5moLS_read_category_parm(client, M5MO_LS_8BIT, 0x0F, 0x07, &val);
			if(val == 0)			//check programming process
				break;
			//mdelay(50);
			msleep(200);
			retry++;
		}
		mdelay(20);

		/* Increase Flash ROM memory address */
		flash_addr += 0x10000;		

		fw_update_status += 3;
	}

	for(i = 0; i < 4; i++)
	{
		/* Set FLASH ROM memory address */
		m5moLS_write_category_parm(client, M5MO_LS_32BIT, 0x0F, 0x00, flash_addr);
		msleep(30);
		/* Erase FLASH ROM entire memory */
		m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0F, 0x06, 0x01);
		msleep(30);
		retry = 0;
		while(1) 
		{
			/* Response while sector-erase is operating */
			m5moLS_read_category_parm(client, M5MO_LS_8BIT, 0x0F, 0x06, &val);
			if(val == 0)	//4
				break;

			//mdelay(20);
			msleep(200);
			retry++;
                     m5moLS_msg(&client->dev,"Response while sector-erase is operating :: retry = %d\n",retry);
		}	
		
		/* Set FLASH ROM programming size */
		m5moLS_write_category_parm(client, M5MO_LS_16BIT, 0x0F, 0x04, 0x2000);
		msleep(30); /* This isn't in EE4, but it was added at all other locations in the changes made, so I'm putting it here */
		/* Clear M-5MoLS internal RAM */
		m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0F, 0x08, 0x01);
		mdelay(10);

		/* Set Flash ROM programming address */
		m5moLS_write_category_parm(client, M5MO_LS_32BIT, 0x0F, 0x00, flash_addr);
		msleep(30);

		/* Send programmed firmware */
		for(j=0; j<0x2000; j+=0x1000)
		{
			m5moLS_write_mem(client, 0x1000, 0x68000000+j, 
								((state->fw_info.addr) + (31 * 0x10000) + (i * 0x2000) + j));
			mdelay(10);
                     m5moLS_msg(&client->dev,"Send Programmed firmware = %d\n",i);
		}

		/* Start Programming */
		m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0F, 0x07, 0x01);						
		/* Confirm programming has been completed */
		retry = 0;
		while(1) 
		{
			m5moLS_read_category_parm(client, M5MO_LS_8BIT, 0x0F, 0x07, &val);
			if(val == 0)			//check programming process
				break;

			//mdelay(10);
			msleep(200);
			retry++;
		}
		//mdelay(20);
		msleep(20);
		
		/* Increase Flash ROM memory address */
		flash_addr += 0x2000;	

		fw_update_status += 1;
	}

	m5moLS_msg(&client->dev,"Firmware Update Success!!\n");

	//release_firmware(fw_entry);

	fw_update_status = 100;
	if(camfw_update == 1)
		camfw_update = 2;

	status = '0';
	m5moLS_write_fw_status(&status);

     	return 0;

}

static int 
m5moLS_i2c_dump_firmware(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	int i, j, flash_addr, err = 0;
	u8* readdata;
	u8 pin1 = 0x7E;

	int fd;
	loff_t pos = 0;
	struct file *file = NULL;
	mm_segment_t old_fs = get_fs ();
	
	readdata = kmalloc(0x1000, GFP_KERNEL);

	set_fs (KERNEL_DS);

	fd = sys_open ("/tmp/cam_fw.bin", O_WRONLY | O_CREAT | O_TRUNC , 0644);
	if(fd < 0 )
	{
		err = -EFAULT;
		goto DUMP_ERR;
	}
	file = fget (fd);
	if(!file)
	{
		err = -EFAULT;
		goto DUMP_ERR;
	}
		
	/* Set M-5MoLS Pins */
	m5moLS_write_mem(client, 0x0001, 0x50000308, &pin1);

	DBG("Firmware Dump Start!!\n");
	flash_addr = 0x10000000;	

	for(i = 0; i< 0x1F0000; i+= 0x10000) 
	{
		DBG( "Firmware Dump is processing... %d!!\n", i/0x10000);
		/*Set FLASH ROM read address and size; read data*/	
		for(j=0; j<0x10000; j+=0x1000)
		{
			pos = i + j;
			m5moLS_read_mem(client, 0x1000, flash_addr + i + j, readdata);
			vfs_write (file, readdata, 0x1000, &pos);
		}
		fw_dump_status += 3;
	}
	for(i = 0x1F0000; i< 0x1F8000; i+= 0x2000) 
	{
		/*Set FLASH ROM read address and size; read data*/	
		for(j=0; j<0x2000; j+=0x1000)
		{
			pos = i + j;
			m5moLS_read_mem(client, 0x1000, flash_addr + i + j, readdata);
			vfs_write (file, readdata, 0x1000, &pos);
		}
		fw_dump_status += 1;
	}
	
	DBG( "Firmware Dump End!!\n");

DUMP_ERR:
	if(file)
		fput (file);
	if(fd)
		sys_close (fd);
      set_fs (old_fs);
	kfree(readdata);  

	fw_dump_status = 100;

	return err;

}


static int
m5moLS_check_firmware(struct v4l2_subdev *sd, u32 fw_version)
{
/*		sunggeun : stealth_Q
	if(hw_revision >= CONFIG_H2_REV02)
	{
		if (fw_version < 0x0A)
		{
			cam_fw_name = "RS_M5LS_8BIT.bin";
			cam_fw_version = 0x0009;
			cam_parm_version = 0x504C;
		}
		else if (fw_version >= 0x0A) 
		{
			cam_fw_name = "RS_M5LS_10BIT.bin";
			cam_fw_version  = 0x000A;	
			cam_parm_version = 0x5055;
		}
	}
	else if (hw_revision < CONFIG_H2_REV02)
	{
		cam_fw_name = "RS_M5LS_SE.bin";
		cam_fw_version = 0xA000;
	}
*/
	return 0;
}


static int 
m5moLS_read_interrupt(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
         unsigned char m5moLS_interrupt_addr[1] = {0x10};
    
	int int_value = 0;

	if(cam_interrupted != 1) 
	{
		m5moLS_msg( &client->dev,"wait interrupt error.\n");
		return -EPERM;
	}

	m5moLS_read_category_parm(client, M5MO_LS_8BIT, 0x00, 0x10, &int_value);
	m5moLS_msg( &client->dev,"Interrupt : 0x%02x.\n", int_value);

	cam_interrupted = 0;

	return int_value;
}


static int 
m5moLS_wait_interrupt(struct v4l2_subdev *sd, int interrupt, int timeout)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    u32 intr_value;         
    int err = 1, int_value = 0xff;
    cam_interrupted = 0;
    
    m5moLS_msg(&client->dev, "%s\n", __func__);
    
    while(cam_interrupted == 0 && err != 0) 
    {
    	err = wait_event_interruptible_timeout(cam_wait,cam_interrupted == 1,msecs_to_jiffies(timeout));
    }
    m5moLS_msg(&client->dev,"=======================>cam_interruted = %d\n",cam_interrupted);

    int_value = m5moLS_read_interrupt(sd);
    if(int_value == interrupt) 
    {
    	m5moLS_msg(&client->dev, "OK... %02x interrupt is occured!.\n", interrupt);
    	return 0;
    }
    else
    {
    	m5moLS_info(&client->dev, "Error! Interrupt signal is not %02x signal and read interrupt signal is %02x signal!!!.\n", interrupt, int_value);
    	return -EPERM;
    }	
}

int m5moLS_check_fw_status(void)
{
        struct file *filp;
        char *dp;
        long l;
        loff_t pos;
        int ret;
        mm_segment_t fs = get_fs();

        printk("%s %d\n", __func__, __LINE__);

        set_fs(get_ds());

        filp = filp_open("/mnt/.lfs/cam_fw_status", O_RDONLY, 0);
        if (filp < 0) {
                ERR("file open error\n");
                set_fs(fs);
                return -EFAULT;
        }

        l = filp->f_path.dentry->d_inode->i_size;	
        dp = kmalloc(l, GFP_KERNEL);	
        if (dp == NULL) {
                ERR("Out of Memory\n");
                filp_close(filp, current->files);
                set_fs(fs);
                return -ENOMEM;
        }

        pos = 0;
        memset(dp, 0, l);
        ret = vfs_read(filp, (char __user *)dp, l, &pos);	
        if (ret != l) {
                ERR("Failed to read file ret = %d\n", ret);
                kfree(dp);
                filp_close(filp, current->files);
                set_fs(fs);
                return -EINVAL;
        }

        ret = strncmp(dp, "0", 1);

        kfree(dp);
        filp_close(filp, current->files);
        set_fs(fs);

        if(ret)
                return -EPERM;
        else
                return 0;
}

static int
m5moLS_detect(struct v4l2_subdev *sd)
{
        struct i2c_client *client = v4l2_get_subdevdata(sd);

        unsigned char m5moLS_test_buf[2] = {0x12, 0x01};
        unsigned int m5moLS_test_len = 2;
        
        u16 fw_version = 0, hw_version = 0, parm_version = 0, val = 0;
        u16 customer_version = 0, project_version=0;
        int err,i;

        cam_fw_name = "RS_M5LS.bin";
#if 0
        err = m5moLS_check_fw_status();
        if(err)
        {
                ERR("Camera Firmware is not ended!!!\n");
                return -EPERM;
        }
#endif
        /* Start Camera Program */
        err = m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0F, 0x12, 0x01);
        if(err)
        {
                ERR("There is no Camera Module!!!\n");
                return -ENODEV;
        }
     
        /* Wait 'System initialization completion' interrupt */
        m5moLS_msg(&client->dev,  "Waiting 'System initialization completion' interrupt... \n");
        
        err = m5moLS_wait_interrupt(sd, M5MO_LS_INT_MODE, 1000);
        if(err)
        {
                m5moLS_msg(&client->dev,"%s: m5moLS_wait_interrupt error!\n", __FUNCTION__);	
                return 0;
        }
        return 0;
}

static int m5moLS_init(struct v4l2_subdev *sd, u32 val)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct m5moLS_state *state = to_state(sd);
    int err = -EINVAL;
    int i = 0;
    u8 temp_8=0;

    m5moLS_msg(&client->dev, "%s\n", __func__);

    m5moLS_init_parameters(sd);

    err = m5moLS_detect(sd);
    if(err)
        return err;

    for(i=0;i<30;i++)
    {
        m5moLS_read_category_parm(client, M5MO_LS_8BIT, 0x00, 0x0A, &temp_8);
        state->camera_fw_info[i] = temp_8;
        if(state->camera_fw_info[i] == NULL)
        {
            break;
        }
        m5moLS_msg(&client->dev, "FW_INFO_VER[%d] = 0x%02x[%c]\n", i,state->camera_fw_info[i],state->camera_fw_info[i]);
        temp_8=0;
    }

    /* Parameter Setting Mode */
    err = m5moLS_set_mode(sd, M5MO_LS_PARMSET_MODE);
    if(err)
    {
        m5moLS_info(&client->dev,"Can not set operation mode\n");
        return err;
    }
       /* OUTPUT_IF_SEL(YUV-IF) */
       m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x01, 0x00, 0x00);

	/* YUVOUT_MAIN(YUV422) */
	m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x00, 0x00);

        /* PREVIEW_IMAGE RESOLUTION */
        m5moLS_set_preview_resolution(sd);
    
	/* MON_FPS( Auto) */
	m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x01, 0x02, 0x01);
 
    /* Set the Initialization Value */
    /* Set Thumbnail Image Size to QVGA*/
    m5moLS_write_category_parm(client, M5MO_LS_8BIT, 0x0B, 0x0B, 0x04); 

    m5moLS_msg(&client->dev, "%s:Done!!!\n", __func__);

    return 0;
}

/*
 * s_config subdev ops
 * With camera device, we need to re-initialize every single opening time therefor,
 * it is not necessary to be initialized on probe time. except for version checking
 * NOTE: version checking is optional
 */
static int m5moLS_s_config(struct v4l2_subdev *sd, int irq, void *platform_data)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct m5moLS_state *state = to_state(sd);
    struct m5moLS_platform_data *pdata;
    int ret = -ENODEV;

    m5moLS_msg(&client->dev, "%s\n", __func__);

    pdata = client->dev.platform_data;

    if (!pdata) {
        m5moLS_info(&client->dev, "%s: no platform data\n", __func__);
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

    ret = m5moLS_enable_interrupt_pin(sd);


    /* Check for interrupt pin assignment */
    ret = request_irq(IRQ_M5MO_LS_CAM_MEGA_INT, m5moLS_irq_handler,IRQF_IRQPOLL,"m5moLS irq",NULL);
    if (ret) {
    	m5moLS_msg(&client->dev,"Failed to get IRQ\n");
    	return ret;
    }
    else
        m5moLS_msg(&client->dev,"M5MO_LS IRQ registered as %d\n", state->irq); 

    if(ret<0){
        m5moLS_msg(&client->dev, "failed to enable interrupt :::: ret = %d\n",ret);
    }

#if 0
    ret = device_create_file(sd->v4l2_dev->dev, &dev_attr_m5moLS_debug);
    if (ret < 0)
    ERR("Failed to add object sysfs files(dev_attr_m5moLS_debug)\n");
#endif
    return 0;
}

static const struct v4l2_subdev_core_ops m5moLS_core_ops = {
	.init = m5moLS_init,	/* initializing API */
	.s_config = m5moLS_s_config,	/* Fetch platform data */
	.queryctrl = m5moLS_queryctrl,
	.querymenu = m5moLS_querymenu,
	.g_ctrl = m5moLS_g_ctrl,
	.s_ctrl = m5moLS_s_ctrl,
	.s_ext_ctrls = m5moLS_s_ext_ctrls,
};

static const struct v4l2_subdev_video_ops m5moLS_video_ops = {
	.s_crystal_freq = m5moLS_s_crystal_freq,
	.g_fmt = m5moLS_g_fmt,
	.s_fmt = m5moLS_s_fmt,
	.enum_framesizes = m5moLS_enum_framesizes,
	.enum_frameintervals = m5moLS_enum_frameintervals,
	.enum_fmt = m5moLS_enum_fmt,
	.try_fmt = m5moLS_try_fmt,
	.g_parm = m5moLS_g_parm,
	.s_parm = m5moLS_s_parm,
};

static const struct v4l2_subdev_ops m5moLS_ops = {
	.core = &m5moLS_core_ops,
	.video = &m5moLS_video_ops,
};

/*
 * m5moLS_probe
 * Fetching platform data is being done with s_config subdev call.
 * In probe routine, we just register subdev device
 */
static int m5moLS_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	printk("8MP camera M5MO_LS loaded.\n");
	struct m5moLS_state *state;
	struct v4l2_subdev *sd;

	state = kzalloc(sizeof(struct m5moLS_state), GFP_KERNEL);
	if (state == NULL)
		return -ENOMEM;

	state->runmode = M5MO_LS_RUNMODE_NOTREADY;
	state->pre_af_status = -1;
	state->cur_af_status = -2;

	sd = &state->sd;
	strcpy(sd->name, M5MO_LS_DRIVER_NAME);

	/* Registering subdev */
	v4l2_i2c_subdev_init(sd, client, &m5moLS_ops);

	m5moLS_info(&client->dev, "8MP camera M5MO_LS loaded.\n");

	return 0;
}

static int m5moLS_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
    
    m5moLS_disable_interrupt_pin(sd);
	//m5moLS_set_af_softlanding(sd);

	v4l2_device_unregister_subdev(sd);
	kfree(to_state(sd));

	m5moLS_info(&client->dev, "Unloaded camera sensor M5MO_LS.\n");

	return 0;
}

static const struct i2c_device_id m5moLS_id[] = {
	{ M5MO_LS_DRIVER_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, m5moLS_id);

static struct v4l2_i2c_driver_data v4l2_i2c_data = {
	.name = M5MO_LS_DRIVER_NAME,
	.probe = m5moLS_probe,
	.remove = m5moLS_remove,
	.id_table = m5moLS_id,
};

MODULE_DESCRIPTION("FUJITSU M5MO_LS-FUJITSU 8MP camera driver");
MODULE_AUTHOR("Tushar Behera <tushar.b@samsung.com>");
MODULE_LICENSE("GPL");
