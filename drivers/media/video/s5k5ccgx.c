/***********************************************************************
* Driver for S5K5CCGX (3MP Camera) from SAMSUNG SYSTEM LSI
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
#include <media/common_camera_platform.h>

#ifdef CONFIG_VIDEO_SAMSUNG_V4L2
#include <linux/videodev2_samsung.h>
#endif // CONFIG_VIDEO_SAMSUNG_V4L2

#include <linux/rtc.h>
#include <mach/gpio.h>
#include <mach/gpio-chief.h>
#include <plat/gpio-cfg.h>
#include <mach/regs-gpio.h>
#include <mach/regs-clock.h>

#include "s5k5ccgx_regs_evt_1.h"

// #define CONFIG_LOAD_FILE	//For tunning binary

/**************************************************************************
* DEFINES
***************************************************************************/

#define S5K5CCGX_DRIVER_NAME	"S5K5CCGX"

#define FORMAT_FLAGS_COMPRESSED			0x3
#define SENSOR_JPEG_SNAPSHOT_MEMSIZE	0x33F000     //3403776 //2216 * 1536

#define SENSOR_MAX_WIDTH        2048
#define SENSOR_MAX_HEIGHT       1536

#define PREVIEW_BASE_WIDTH      1024
#define PREVIEW_BASE_HEIGHT      768

// 416 * 320 Based window
#define AF_OUTER_WINDOW_WIDTH   208
#define AF_OUTER_WINDOW_HEIGHT  177
#define AF_INNER_WINDOW_WIDTH   93
#define AF_INNER_WINDOW_HEIGHT  95

#define MAX_BUFFER			(2048)
#define DELAY_SEQ           0xFFFF

/* Default resolution & pixelformat. */
#define DEFAULT_PIX_FMT		V4L2_PIX_FMT_UYVY	/* YUV422 */
#define DEFUALT_MCLK		24000000
#define POLL_TIME_MS		10

#define INIT_NUM_OF_REGS                        (sizeof(init_regs) / sizeof(regs_short_t))
#define PREVIEW_CAMERA_NUM_OF_REGS              (sizeof(preview_camera_regs) / sizeof(regs_short_t))
#define SNAPSHOT_NORMAL_NUM_OF_REGS             (sizeof(snapshot_normal_regs) / sizeof(regs_short_t))
#define SNAPSHOT_LOWLIGHT_NUM_OF_REGS           (sizeof(snapshot_lowlight_regs) / sizeof(regs_short_t))
#define SNAPSHOT_HIGHLIGHT_NUM_OF_REGS          (sizeof(snapshot_highlight_regs) / sizeof(regs_short_t))
#define SNAPSHOT_NIGHTMODE_NUM_OF_REGS          (sizeof(snapshot_nightmode_regs) / sizeof(regs_short_t))

#define SNAPSHOT_FLASH_ON_NUM_OF_REGS           (sizeof(snapshot_flash_on_regs) / sizeof(regs_short_t))
#define SNAPSHOT_AF_PREFLASH_ON_NUM_OF_REGS     (sizeof(snapshot_af_preflash_on_regs) / sizeof(regs_short_t))
#define SNAPSHOT_AF_PREFLASH_OFF_NUM_OF_REGS    (sizeof(snapshot_af_preflash_off_regs) / sizeof(regs_short_t))

#define AF_MACRO_NUM_OF_REGS	                (sizeof(af_macro_mode_regs) / sizeof(regs_short_t))
#define AF_NORMAL_NUM_OF_REGS	                (sizeof(af_normal_mode_regs) / sizeof(regs_short_t))
#define SINGLE_AF_START_NUM_OF_REGS	            (sizeof(single_af_start_regs) / sizeof(regs_short_t))
#define SINGLE_AF_STOP_NUM_OF_REGS	            (sizeof(single_af_stop_regs) / sizeof(regs_short_t))

#define EFFECT_NORMAL_NUM_OF_REGS	            (sizeof(effect_normal_regs) / sizeof(regs_short_t))
#define EFFECT_NEGATIVE_NUM_OF_REGS	            (sizeof(effect_negative_regs) / sizeof(regs_short_t))
#define EFFECT_SEPIA_NUM_OF_REGS	            (sizeof(effect_sepia_regs) / sizeof(regs_short_t))
#define EFFECT_MONO_NUM_OF_REGS                 (sizeof(effect_mono_regs) / sizeof(regs_short_t))

#define WB_AUTO_NUM_OF_REGS	                    (sizeof(wb_auto_regs) / sizeof(regs_short_t))
#define WB_SUNNY_NUM_OF_REGS	                (sizeof(wb_sunny_regs) / sizeof(regs_short_t))
#define WB_CLOUDY_NUM_OF_REGS	                (sizeof(wb_cloudy_regs) / sizeof(regs_short_t))
#define WB_TUNSTEN_NUM_OF_REGS	                (sizeof(wb_tungsten_regs) / sizeof(regs_short_t))
#define WB_FLUORESCENT_NUM_OF_REGS	            (sizeof(wb_fluorescent_regs) / sizeof(regs_short_t))

#define METERING_MATRIX_NUM_OF_REGS	            (sizeof(metering_matrix_regs) / sizeof(regs_short_t))
#define METERING_CENTER_NUM_OF_REGS	            (sizeof(metering_center_regs) / sizeof(regs_short_t))
#define METERING_SPOT_NUM_OF_REGS	            (sizeof(metering_spot_regs) / sizeof(regs_short_t))

#define EV_M4_NUM_OF_REGS	                    (sizeof(ev_minus_4_regs) / sizeof(regs_short_t))
#define EV_M3_NUM_OF_REGS	                    (sizeof(ev_minus_3_regs) / sizeof(regs_short_t))
#define EV_M2_NUM_OF_REGS	                    (sizeof(ev_minus_2_regs) / sizeof(regs_short_t))
#define EV_M1_NUM_OF_REGS	                    (sizeof(ev_minus_1_regs) / sizeof(regs_short_t))
#define EV_DEFAULT_NUM_OF_REGS	                (sizeof(ev_default_regs) / sizeof(regs_short_t))
#define EV_P1_NUM_OF_REGS	                    (sizeof(ev_plus_1_regs) / sizeof(regs_short_t))
#define EV_P2_NUM_OF_REGS	                    (sizeof(ev_plus_2_regs) / sizeof(regs_short_t))
#define EV_P3_NUM_OF_REGS	                    (sizeof(ev_plus_3_regs) / sizeof(regs_short_t))
#define EV_P4_NUM_OF_REGS	                    (sizeof(ev_plus_4_regs) / sizeof(regs_short_t))

#define CONTRAST_M2_NUM_OF_REGS	                (sizeof(contrast_minus_2_regs) / sizeof(regs_short_t))
#define CONTRAST_M1_NUM_OF_REGS	                (sizeof(contrast_minus_1_regs) / sizeof(regs_short_t))
#define CONTRAST_DEFAULT_NUM_OF_REGS	        (sizeof(contrast_default_regs) / sizeof(regs_short_t))
#define CONTRAST_P1_NUM_OF_REGS	                (sizeof(contrast_plus_1_regs) / sizeof(regs_short_t))
#define CONTRAST_P2_NUM_OF_REGS	                (sizeof(contrast_plus_2_regs) / sizeof(regs_short_t))

#define SHARPNESS_M2_NUM_OF_REGS	            (sizeof(sharpness_minus_2_regs) / sizeof(regs_short_t))
#define SHARPNESS_M1_NUM_OF_REGS	            (sizeof(sharpness_minus_1_regs) / sizeof(regs_short_t))
#define SHARPNESS_DEFAULT_NUM_OF_REGS	        (sizeof(sharpness_default_regs) / sizeof(regs_short_t))
#define SHARPNESS_P1_NUM_OF_REGS	            (sizeof(sharpness_plus_1_regs) / sizeof(regs_short_t))
#define SHARPNESS_P2_NUM_OF_REGS	            (sizeof(sharpness_plus_2_regs) / sizeof(regs_short_t))

#define SATURATION_M2_NUM_OF_REGS	            (sizeof(saturation_minus_2_regs) / sizeof(regs_short_t))
#define SATURATION_M1_NUM_OF_REGS	            (sizeof(saturation_minus_1_regs) / sizeof(regs_short_t))
#define SATURATION_DEFAULT_NUM_OF_REGS	        (sizeof(saturation_default_regs) / sizeof(regs_short_t))
#define SATURATION_P1_NUM_OF_REGS	            (sizeof(saturation_plus_1_regs) / sizeof(regs_short_t))
#define SATURATION_P2_NUM_OF_REGS	            (sizeof(saturation_plus_2_regs) / sizeof(regs_short_t))

#define ZOOM_00_NUM_OF_REGS	                    (sizeof(zoom_00_regs) / sizeof(regs_short_t))
#define ZOOM_01_NUM_OF_REGS	                    (sizeof(zoom_01_regs) / sizeof(regs_short_t))
#define ZOOM_02_NUM_OF_REGS	                    (sizeof(zoom_02_regs) / sizeof(regs_short_t))
#define ZOOM_03_NUM_OF_REGS	                    (sizeof(zoom_03_regs) / sizeof(regs_short_t))
#define ZOOM_04_NUM_OF_REGS	                    (sizeof(zoom_04_regs) / sizeof(regs_short_t))
#define ZOOM_05_NUM_OF_REGS	                    (sizeof(zoom_05_regs) / sizeof(regs_short_t))
#define ZOOM_06_NUM_OF_REGS	                    (sizeof(zoom_06_regs) / sizeof(regs_short_t))
#define ZOOM_07_NUM_OF_REGS	                    (sizeof(zoom_07_regs) / sizeof(regs_short_t))
#define ZOOM_08_NUM_OF_REGS	                    (sizeof(zoom_08_regs) / sizeof(regs_short_t))

#define SCENE_NONE_NUM_OF_REGS	                (sizeof(scene_none_regs) / sizeof(regs_short_t))
#define SCENE_PORTRAIT_NUM_OF_REGS	            (sizeof(scene_portrait_regs) / sizeof(regs_short_t))
#define SCENE_NIGHTSHOT_NUM_OF_REGS	            (sizeof(scene_nightshot_regs) / sizeof(regs_short_t))
#define SCENE_BACKLIGHT_NUM_OF_REGS	            (sizeof(scene_backlight_regs) / sizeof(regs_short_t))
#define SCENE_LANDSCAPE_NUM_OF_REGS	            (sizeof(scene_landscape_regs) / sizeof(regs_short_t))
#define SCENE_SPORTS_NUM_OF_REGS	            (sizeof(scene_sports_regs) / sizeof(regs_short_t))
#define SCENE_INDOOR_NUM_OF_REGS	            (sizeof(scene_party_indoor_regs) / sizeof(regs_short_t))
#define SCENE_BEACH_NUM_OF_REGS	                (sizeof(scene_beach_snow_regs) / sizeof(regs_short_t))
#define SCENE_SUNSET_NUM_OF_REGS	            (sizeof(scene_sunset_regs) / sizeof(regs_short_t))
#define SCENE_DUSKDAWN_NUM_OF_REGS	            (sizeof(scene_duskdawn_regs) / sizeof(regs_short_t))
#define SCENE_FALLCOLOR_NUM_OF_REGS	            (sizeof(scene_fall_color_regs) / sizeof(regs_short_t))
#define SCENE_FIREWORKS_NUM_OF_REGS	            (sizeof(scene_fireworks_regs) / sizeof(regs_short_t))
#define SCENE_TEXT_NUM_OF_REGS	                (sizeof(scene_text_regs) / sizeof(regs_short_t))
#define SCENE_CANDLELIGHT_NUM_OF_REGS	        (sizeof(scene_candle_light_regs) / sizeof(regs_short_t))

#define FPS_AUTO_NUM_OF_REGS	                (sizeof(fps_auto_regs) / sizeof(regs_short_t))
#define FPS_7_NUM_OF_REGS	                    (sizeof(fps_7_regs) / sizeof(regs_short_t))
#define FPS_10_NUM_OF_REGS	                    (sizeof(fps_10_regs) / sizeof(regs_short_t))
#define FPS_15_NUM_OF_REGS	                    (sizeof(fps_15_regs) / sizeof(regs_short_t))
#define FPS_30_NUM_OF_REGS	                    (sizeof(fps_30_regs) / sizeof(regs_short_t))

#define QUALITY_SUPERFINE_NUM_OF_REGS           (sizeof(quality_superfine_regs) / sizeof(regs_short_t))
#define QUALITY_FINE_NUM_OF_REGS                (sizeof(quality_fine_regs) / sizeof(regs_short_t))
#define QUALITY_NORMAL_NUM_OF_REGS              (sizeof(quality_normal_regs) / sizeof(regs_short_t))
#define QUALITY_ECONOMY_NUM_OF_REGS             (sizeof(quality_economy_regs) / sizeof(regs_short_t))

#define PREVIEW_SIZE_1024X768_NUM_OF_REGS       (sizeof(preview_size_1024x768_regs) / sizeof(regs_short_t))
#define PREVIEW_SIZE_720X480_NUM_OF_REGS        (sizeof(preview_size_720x480_regs) / sizeof(regs_short_t))
#define PREVIEW_SIZE_704X576_NUM_OF_REGS        (sizeof(preview_size_704x576_regs) / sizeof(regs_short_t))
#define PREVIEW_SIZE_640X480_NUM_OF_REGS        (sizeof(preview_size_640x480_regs) / sizeof(regs_short_t))

#define CAPTURE_SIZE_2048X1536_NUM_OF_REGS      (sizeof(capture_size_2048x1536_regs) / sizeof(regs_short_t))
#define CAPTURE_SIZE_2048X1368_NUM_OF_REGS      (sizeof(capture_size_2048x1368_regs) / sizeof(regs_short_t))
#define CAPTURE_SIZE_1600X1200_NUM_OF_REGS      (sizeof(capture_size_1600x1200_regs) / sizeof(regs_short_t))
#define CAPTURE_SIZE_1280X960_NUM_OF_REGS       (sizeof(capture_size_1280x960_regs) / sizeof(regs_short_t))
#define CAPTURE_SIZE_1024X768_NUM_OF_REGS       (sizeof(capture_size_1024x768_regs) / sizeof(regs_short_t))
#define CAPTURE_SIZE_800X600_NUM_OF_REGS        (sizeof(capture_size_800x600_regs) / sizeof(regs_short_t))
#define CAPTURE_SIZE_720X480_NUM_OF_REGS        (sizeof(capture_size_720x480_regs) / sizeof(regs_short_t))
#define CAPTURE_SIZE_640X480_NUM_OF_REGS        (sizeof(capture_size_640x480_regs) / sizeof(regs_short_t))
#define CAPTURE_SIZE_416X320_NUM_OF_REGS        (sizeof(capture_size_416x320_regs) / sizeof(regs_short_t))

#define PATTERN_ON_NUM_OF_REGS	                (sizeof(pattern_on_regs) / sizeof(regs_short_t))
#define PATTERN_OFF_NUM_OF_REGS	                (sizeof(pattern_off_regs) / sizeof(regs_short_t))

// #define S5K5CCGX_DEBUG 

#ifdef S5K5CCGX_DEBUG
#define CAM_ERROR_MSG(dev, format, arg...)  dev_err(dev, format, ## arg)
#define CAM_WARN_MSG(dev, format, arg...)   dev_warn(dev, format, ## arg)
#define CAM_INFO_MSG(dev, format, arg...)   dev_warn(dev, format, ## arg)
#define CAM_PRINTK(format, arg...)          printk(format, ## arg)
#else
#define CAM_ERROR_MSG(dev, format, arg...)  dev_err(dev, format, ## arg)
#define CAM_WARN_MSG(dev, format, arg...)   dev_warn(dev, format, ## arg)
#define CAM_INFO_MSG(dev, format, arg...)
#define CAM_PRINTK(format, arg...)
#endif // S5K5CCGX_DEBUG

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
	PREVIEW_SIZE_4CIF,		    /* 4CIF :   704x576 */
	PREVIEW_SIZE_D1,			/* D1   :   720x480 */
	PREVIEW_SIZE_SVGA,		    /* SVGA :   800x600 */
	PREVIEW_SIZE_XGA,		    /* XGA  :  1024x768 */
//	PREVIEW_SIZE_HD,		    /* HD   :  1280x720 */
	CAPTURE_SIZE_VGA,		    /* VGA  :   640x480 */
	CAPTURE_SIZE_D1,		    /* D1   :   720x480 */
	CAPTURE_SIZE_SVGA,		    /* SVGA :   800x600 */
	CAPTURE_SIZE_XGA,		    /* XGA  :  1024x768 */
	CAPTURE_SIZE_4VGA,		    /* 4VGA :  1280x960 */
	CAPTURE_SIZE_UXGA,			/* UXGA : 1600x1200 */
	CAPTURE_SIZE_QXGA,			/* QXGA : 2048x1536 */
	CAPTURE_SIZE_USER1,		    /* USER1 : 2048x1368 */
	CAPTURE_SIZE_PREVIEW,		/* Preview Size */
};

enum running_mode 
{
	RUNNING_MODE_NOTREADY,
	RUNNING_MODE_IDLE, 
	RUNNING_MODE_RUNNING, 
};

/**************************************************************************
* STRUCTURES
***************************************************************************/

struct sensor_date_info 
{
	unsigned int year;
	unsigned int month;
	unsigned int date;
};

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

struct sensor_version {
	unsigned int major;
	unsigned int minor;
};

struct sensor_firmware 
{
	unsigned int addr;
	unsigned int size;
};

struct sensor_af_info 
{
	int x;
	int y;
	int preview_width;
	int preview_height;
} ; 

struct sensor_image_info {
	int width;
	int height;
} ; 

struct sensor_state 
{
	struct camera_platform_data *platform_data;
	struct v4l2_subdev sd;
	struct v4l2_pix_format pix;
	struct v4l2_fract time_per_frame;
	struct sensor_userset userset;
	struct sensor_jpeg_param jpeg_param;
	struct sensor_date_info date_info;
	struct sensor_version fw_ver;
	struct sensor_af_info af_info;
	struct sensor_image_info postview_info;
	struct v4l2_streamparm strm;
	enum running_mode runmode;
	enum operation_mode op_mode;
	int sensor_mode;
	int framesize_index;
	int sensor_version;
	int mclk_freq;	            /* MCLK in Hz */
	int fps;
    int capture_mode;
    int check_dataline;
    int current_flash_mode;
    int camera_flash_fire;
    int camera_af_flash_fire;    
};

/**************************************************************************
* GLOBAL, STATIC VARIABLES
***************************************************************************/

/* protect s_ctrl calls */
static DEFINE_MUTEX(sensor_s_ctrl);
static DEFINE_MUTEX(af_cancel_op);

/* Save the focus mode value, it can be marco or auto */
static int af_mode;
static int gCurrentScene = SCENE_MODE_NONE;
static int gCurrentWB = WHITE_BALANCE_AUTO;
static int gCurrentMetering = METERING_CENTER;
static bool bStartFineSearch = false;
static int gISOSpeedRating = 0;
static int gExposureTime = 0;
static unsigned char pBurstData[MAX_BUFFER];
static unsigned short previous_WB_read = 0x001E;    // Default : 0x001E

const static struct sensor_enum_framesize s5k5ccgx_framesize_list[] = 
{
	{ OP_MODE_VIDEO, PREVIEW_SIZE_VGA,	    640,  480 },
	{ OP_MODE_VIDEO, PREVIEW_SIZE_4CIF,	    704,  576 },
	{ OP_MODE_VIDEO, PREVIEW_SIZE_D1,	    720,  480 },
	{ OP_MODE_VIDEO, PREVIEW_SIZE_SVGA,	    800,  600 },
	{ OP_MODE_VIDEO, PREVIEW_SIZE_XGA,	   1024,  768 },
//	{ OP_MODE_VIDEO, PREVIEW_SIZE_HD,	   1280,  720 },
	{ OP_MODE_IMAGE, CAPTURE_SIZE_VGA,	    640,  480 },
	{ OP_MODE_IMAGE, CAPTURE_SIZE_D1,	    720,  480 },
	{ OP_MODE_IMAGE, CAPTURE_SIZE_SVGA,	    800,  600 },
	{ OP_MODE_IMAGE, CAPTURE_SIZE_XGA,     1024,  768 },	
	{ OP_MODE_IMAGE, CAPTURE_SIZE_4VGA,	   1280,  960 },
	{ OP_MODE_IMAGE, CAPTURE_SIZE_UXGA,	   1600, 1200 },
	{ OP_MODE_IMAGE, CAPTURE_SIZE_QXGA,	   2048, 1536 },
	{ OP_MODE_IMAGE, CAPTURE_SIZE_USER1,   2048, 1368 },
	{ OP_MODE_IMAGE, CAPTURE_SIZE_PREVIEW,  416,  320 },
};

const static struct v4l2_fmtdesc capture_fmts[] = 
{
    {
        .index          = 0,
        .type           = V4L2_BUF_TYPE_VIDEO_CAPTURE,
        .flags          = FORMAT_FLAGS_COMPRESSED,
        .description    = "JPEG + Postview",
        .pixelformat    = V4L2_PIX_FMT_JPEG,
    },
};

/**************************************************************************
* EXTERN VARIABLES
***************************************************************************/

/**************************************************************************
* FUNCTION DECLARE
***************************************************************************/
static int s5k5ccgx_i2c_read_word(struct i2c_client *client,  
                                     unsigned short subaddr, 
                                     unsigned short *data);

static int s5k5ccgx_i2c_write_word(struct i2c_client *client, 
                                     unsigned short subaddr, 
                                     unsigned short data);

static int s5k5ccgx_i2c_set_data_burst(struct i2c_client *client, 
                                         regs_short_t reg_buffer[], 
          				                 int num_of_regs); 

static int s5k5ccgx_i2c_set_config_register(struct i2c_client *client, 
                                         regs_short_t reg_buffer[], 
          				                 int num_of_regs,
          				                 char *name);
          				                 
static int s5k5ccgx_check_dataline_onoff(struct v4l2_subdev *sd, int onoff);
static int s5k5ccgx_AE_AWB_lock(struct v4l2_subdev *sd, struct v4l2_control *ctrl);
static int s5k5ccgx_set_framesize_index(struct v4l2_subdev *sd, unsigned int index);
static int s5k5ccgx_get_iso_speed_rate(struct v4l2_subdev *sd);
static int s5k5ccgx_get_shutterspeed(struct v4l2_subdev *sd);

static inline struct sensor_state *to_state(struct v4l2_subdev *sd)
{
	return container_of(sd, struct sensor_state, sd);
}

/**************************************************************************
* TUNING CONFIGURATION FUNCTIONS, DATAS
***************************************************************************/
#ifdef CONFIG_LOAD_FILE

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

static char *regs_buf_ptr = NULL; 
static char *curr_pos_ptr = NULL;
static char current_line[MAX_ONE_LINE_LEN];
static regs_short_t reg_table[MAX_REG_TABLE_LEN];
static int reg_num_of_element = 0;

// Warning!! : Register order is very important in aspect of performance of loading regs.
// Place regs by the order as described in register header file.
static reg_hash_t reg_hash_table[] =
{
    {"init_regs",                           NULL},
    {"preview_camera_regs",                 NULL},
    {"snapshot_normal_regs",                NULL},
    {"snapshot_lowlight_regs",              NULL},
    {"snapshot_highlight_regs",             NULL},
    {"snapshot_nightmode_regs",             NULL},
    {"snapshot_flash_on_regs",              NULL},
    {"snapshot_af_preflash_on_regs",        NULL},
    {"snapshot_af_preflash_off_regs",       NULL},    
    {"af_macro_mode_regs",                  NULL},
    {"af_normal_mode_regs",                 NULL},
    {"single_af_start_regs",                NULL},
    {"single_af_stop_regs",                 NULL},
    {"effect_normal_regs",                  NULL},
    {"effect_negative_regs",                NULL},
    {"effect_sepia_regs",                   NULL},
    {"effect_mono_regs",                    NULL},
    {"wb_auto_regs",                        NULL},
    {"wb_sunny_regs",                       NULL},
    {"wb_cloudy_regs",                      NULL},
    {"wb_tungsten_regs",                    NULL},
    {"wb_fluorescent_regs",                 NULL},
    {"metering_matrix_regs",                NULL},
    {"metering_center_regs",                NULL},
    {"metering_spot_regs",                  NULL},
    {"ev_minus_4_regs",                     NULL},
    {"ev_minus_3_regs",                     NULL},
    {"ev_minus_2_regs",                     NULL},
    {"ev_minus_1_regs",                     NULL},
    {"ev_default_regs",                     NULL},
    {"ev_plus_1_regs",                      NULL},
    {"ev_plus_2_regs",                      NULL},
    {"ev_plus_3_regs",                      NULL},
    {"ev_plus_4_regs",                      NULL},
    {"contrast_minus_2_regs",               NULL},
    {"contrast_minus_1_regs",               NULL},
    {"contrast_default_regs",               NULL},
    {"contrast_plus_1_regs",                NULL},
    {"contrast_plus_2_regs",                NULL},
    {"sharpness_minus_2_regs",              NULL},
    {"sharpness_minus_1_regs",              NULL},
    {"sharpness_default_regs",              NULL},
    {"sharpness_plus_1_regs",               NULL},
    {"sharpness_plus_2_regs",               NULL},
    {"saturation_minus_2_regs",             NULL},
    {"saturation_minus_1_regs",             NULL},
    {"saturation_default_regs",             NULL},
    {"saturation_plus_1_regs",              NULL},
    {"saturation_plus_2_regs",              NULL},
    {"zoom_00_regs",                        NULL},
    {"zoom_01_regs",                        NULL},
    {"zoom_02_regs",                        NULL},
    {"zoom_03_regs",                        NULL},
    {"zoom_04_regs",                        NULL},
    {"zoom_05_regs",                        NULL},
    {"zoom_06_regs",                        NULL},
    {"zoom_07_regs",                        NULL},
    {"zoom_08_regs",                        NULL},
    {"scene_none_regs",                     NULL},
    {"scene_portrait_regs",                 NULL},
    {"scene_landscape_regs",                NULL},
    {"scene_nightshot_regs",                NULL},
    {"scene_backlight_regs",                NULL},
    {"scene_sports_regs",                   NULL},
    {"scene_party_indoor_regs",             NULL},
    {"scene_beach_snow_regs",               NULL},
    {"scene_sunset_regs",                   NULL},
    {"scene_duskdawn_regs",                 NULL},
    {"scene_fall_color_regs",               NULL},
    {"scene_fireworks_regs",                NULL},
    {"scene_text_regs",                     NULL},
    {"scene_candle_light_regs",             NULL},
    {"fps_auto_regs",                       NULL},
    {"fps_7_regs",                          NULL},
    {"fps_10_regs",                         NULL},
    {"fps_15_regs",                         NULL},
    {"fps_30_regs",                         NULL},
    {"quality_superfine_regs",              NULL},
    {"quality_fine_regs",                   NULL},
    {"quality_normal_regs",                 NULL},
    {"quality_economy_regs",                NULL},    
    {"preview_size_1024x768_regs",          NULL},    
    {"preview_size_720x480_regs",           NULL},    
    {"preview_size_704x576_regs",           NULL},    
    {"preview_size_640x480_regs",           NULL},    
    {"capture_size_2048x1536_regs",         NULL},    
    {"capture_size_2048x1368_regs",         NULL},    
    {"capture_size_1600x1200_regs",         NULL},    
    {"capture_size_1280x960_regs",          NULL},    
    {"capture_size_1024x768_regs",          NULL},    
    {"capture_size_800x600_regs",           NULL},    
    {"capture_size_720x480_regs",           NULL},    
    {"capture_size_640x480_regs",           NULL},    
    {"capture_size_416x320_regs",           NULL},    
    {"pattern_on_regs",                     NULL},
    {"pattern_off_regs",                    NULL},
};

static bool s5k5ccgx_regs_get_line(char *line_buf)
{
    int i;
    char *r_n_ptr = NULL;

    memset(line_buf, 0, MAX_ONE_LINE_LEN);

    r_n_ptr = strstr(curr_pos_ptr, "\n"); // Wr

    //\n exists.
    if(r_n_ptr )
    {
        for(i = 0; i < MAX_ONE_LINE_LEN; i++)
        {
            if(curr_pos_ptr + i == r_n_ptr)
            {
                curr_pos_ptr = r_n_ptr + 1;
                break;
            }
            line_buf[i] = curr_pos_ptr[i];
        }
        line_buf[i] = '\0';

        return true;
    }
    //\n doesn't exist.
    else
    {
        if(strlen(curr_pos_ptr) > 0)
        {
            strcpy(line_buf, curr_pos_ptr);
            return true;
        }
        else
        {
            return false;
        }            
    }
}

static bool s5k5ccgx_regs_trim(char *line_buf)
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

static int s5k5ccgx_regs_parse_table(void)
{
	char reg_buf[7], data_buf[7];
	int reg_index = 0;

	reg_buf[6] = '\0';
	data_buf[6] = '\0';

    while(s5k5ccgx_regs_get_line(current_line))
    {
        if(s5k5ccgx_regs_trim(current_line) == false)
        {
            continue;
        }

        // Check End line of a table.
        if((current_line[0] == '}') && (current_line[1] == ';'))
        {
            break;
        }

        // Parsing a register format : {0x0000, 0x0000},
        if((current_line[0] == '{') && (current_line[1] == '0') && (current_line[15] == '}'))
        {
            memcpy(reg_buf, (const void *)&current_line[1], 6);
            memcpy(data_buf, (const void *)&current_line[9], 6);

            reg_table[reg_index].subaddr = (unsigned short)simple_strtoul(reg_buf, NULL, 16); 
            reg_table[reg_index].value = (unsigned int)simple_strtoul(data_buf, NULL, 16); 

            reg_index++;
        }
    }

    return reg_index;
}

static int s5k5ccgx_regs_table_write(struct i2c_client *client, char *name)
{
    bool bFound_table = false;
    int i, err = 0;
    
    reg_num_of_element = 0;

    for(i = 0; i < sizeof(reg_hash_table)/sizeof(reg_hash_t); i++)
    {
        if(strcmp(name, reg_hash_table[i].name) == 0)
        {
            bFound_table = true;

            curr_pos_ptr = reg_hash_table[i].location_ptr;
            break;
        }
    }
    
    if(bFound_table)
    {
        reg_num_of_element = s5k5ccgx_regs_parse_table();
    }
    else
    {
        CAM_ERROR_MSG(&client->dev, "[%s: %d] %s reg_table doesn't exist\n", __FILE__, __LINE__, name);	
        return -EIO;
    }

    err = s5k5ccgx_i2c_set_data_burst(client, reg_table, reg_num_of_element);
	if(err < 0)
	{
		CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! s5k5ccgx_i2c_set_data_burst failed\n", __FILE__, __LINE__);	
		return -EIO;
	}
    
	return err;
}

int s5k5ccgx_regs_table_init(void)
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
	
	filp = filp_open("/sdcard/s5k5ccgx_regs_evt_1.h", O_RDONLY, 0);

	if(IS_ERR(filp)) 
	{
		CAM_PRINTK(KERN_ERR "file open error\n");
		return -EIO;
	}
	
	l = filp->f_path.dentry->d_inode->i_size;	
	CAM_PRINTK("%s file size = %ld\n", __func__, l);

	msleep(50);

    for(retry_cnt = 5; retry_cnt > 0; retry_cnt--)
    {
	    dp = vmalloc(l);
	    
	    if(dp != NULL)
	    {
	        break;
	    }
	    
	    msleep(50);
    }
    
	if(dp == NULL) 
	{
		CAM_PRINTK(KERN_ERR "Out of Memory\n");
		filp_close(filp, current->files);
		return -ENOMEM;
	}
	
	memset(dp, 0, l);

	pos = 0;
	ret = vfs_read(filp, (char __user *)dp, l, &pos);
	
	if(ret != l) 
	{
		CAM_PRINTK(KERN_ERR "Failed to read file ret = %d\n", ret);
		vfree(dp);
		filp_close(filp, current->files);
		return -EINVAL;
	}

	filp_close(filp, current->files);
		
	set_fs(fs);
	
	regs_buf_ptr = dp;
		
	*((regs_buf_ptr + l) - 1) = '\0';

    // Make hash table to enhance speed.
    curr_pos_ptr = regs_buf_ptr;
    location_ptr = curr_pos_ptr;
    
    for(i = 0; i < sizeof(reg_hash_table)/sizeof(reg_hash_t); i++)
    {
        reg_hash_table[i].location_ptr = NULL;
        bFound_name = false;
        
        while(s5k5ccgx_regs_get_line(current_line))	
        {
            if(strstr(current_line, reg_hash_table[i].name) != NULL)
            {
                bFound_name = true;
                reg_hash_table[i].location_ptr = location_ptr;
                break;
            }

            location_ptr = curr_pos_ptr;
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
                curr_pos_ptr = reg_hash_table[i-1].location_ptr;
            }
            location_ptr = curr_pos_ptr;
            
            CAM_PRINTK(KERN_ERR "[%s : %d] ERROR! Couldn't find the reg name in hash table\n", __FILE__, __LINE__);	
        }
    }
    
	CAM_PRINTK("s5k5ccgx_reg_table_init\n");

	return 0;
}

void s5k5ccgx_regs_table_exit(void)
{
	CAM_PRINTK("%s start\n", __func__);

	if(regs_buf_ptr) 
	{
		vfree(regs_buf_ptr);
		regs_buf_ptr = NULL;
	}

	CAM_PRINTK("%s done\n", __func__);
}
#endif // CONFIG_LOAD_FILE

/**************************************************************************
* FUNCTIONS
***************************************************************************/

/**************************************************************************
 * s5k5ccgx_i2c_read_word: Read (I2C) multiple bytes to the camera sensor 
 * @client: pointer to i2c_client
 * @subaddr: data to be written
 * @*data: buffer where data is read
 *
 * Returns 0 on success, <0 on error
 ***************************************************************************/
static int s5k5ccgx_i2c_read_word(struct i2c_client *client,  
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
 * s5k5ccgx_i2c_write_word: Write (I2C) multiple bytes to the camera sensor 
 * @client: pointer to i2c_client
 * @subaddr: register address
 * @data: data to be written
 *
 * Returns 0 on success, <0 on error
 ***************************************************************************/
static int s5k5ccgx_i2c_write_word(struct i2c_client *client, 
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
 * s5k5ccgx_i2c_set_data_burst: Write (I2C) multiple bytes to the camera sensor 
 * @client: pointer to i2c_client
 * @reg_buffer: buffer which includes all registers to be written.
 * @num_of_regs: number of registers to be written.
 * @name : This will be used for tuning.
 * Returns 0 on success, <0 on error
 ***************************************************************************/
static int s5k5ccgx_i2c_set_data_burst(struct i2c_client *client, 
                                         regs_short_t reg_buffer[], 
          				                 int num_of_regs)
{
	struct i2c_msg msg = {client->addr, 0, 0, 0};	
    unsigned short subaddr, data_value, next_subaddr;
    int i, index = 0, err = 0;

    memset(pBurstData, 0, sizeof(pBurstData));
    
    for(i = 0; i < num_of_regs; i++)
    {
        subaddr = reg_buffer[i].subaddr;
        data_value = reg_buffer[i].value;

        switch(subaddr)
        {
            case 0x0F12:
            {
                // Start Burst datas
                if(index == 0)
                {
                    pBurstData[index++] = subaddr >> 8;
                    pBurstData[index++] = subaddr & 0xFF;	
                }	

                pBurstData[index++] = data_value >> 8;
                pBurstData[index++] = data_value & 0xFF;	

                // Get Next Address
                if((i+1) == num_of_regs)  // The last code
                {
                    next_subaddr = 0xFFFF;   // Dummy
                }
                else
                {
                    next_subaddr = reg_buffer[i+1].subaddr;
                }

                // If next subaddr is different from the current subaddr
                // In other words, if burst mode ends, write the all of the burst datas which were gathered until now
                if(next_subaddr != subaddr) 
                {
                    msg.buf = pBurstData;
                    msg.len = index;

                    err = i2c_transfer(client->adapter, &msg, 1);
                	if(err < 0)
                	{
                		CAM_ERROR_MSG(&client->dev, "[%s: %d] i2c burst write fail\n", __FILE__, __LINE__);	
                		return -EIO;
                	}

                    // Intialize and gather busrt datas again.
                    index = 0;
                    memset(pBurstData, 0, sizeof(pBurstData));
                }
                break;
            }

            case DELAY_SEQ:
            {
                msleep(data_value);
                break;
            }

            case 0xFCFC:
            case 0x0028:
            case 0x002A:
            default:
            {
                err = s5k5ccgx_i2c_write_word(client, subaddr, data_value);
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
 * s5k5ccgx_i2c_set_config_register: Write (I2C) multiple bytes to the camera sensor 
 * @client: pointer to i2c_client
 * @reg_buffer: buffer which includes all registers to be written.
 * @num_of_regs: number of registers to be written.
 * @name : This will be used for tuning.
 * Returns 0 on success, <0 on error
 ***************************************************************************/
static int s5k5ccgx_i2c_set_config_register(struct i2c_client *client, 
                                         regs_short_t reg_buffer[], 
          				                 int num_of_regs, 
          				                 char *name)
{
    int err = 0;

#ifdef CONFIG_LOAD_FILE 
	err = s5k5ccgx_regs_table_write(client, name);
#else
    err = s5k5ccgx_i2c_set_data_burst(client, reg_buffer, num_of_regs);
#endif // CONFIG_LOAD_FILE

    return err;
}

#define AAT_PULS_HI_TIME    1
#define AAT_PULS_LO_TIME    1
#define AAT_LATCH_TIME      500

// AAT1271 flash control driver.
static void s5k5ccgx_AAT_flash_write_data(unsigned char count)
{
    unsigned long flags;

    local_irq_save(flags);

    if(count)
    {
        do 
        {
            gpio_set_value(GPIO_CAM_FLASH_SET, 0);
            udelay(AAT_PULS_LO_TIME);

            gpio_set_value(GPIO_CAM_FLASH_SET, 1);
            udelay(AAT_PULS_HI_TIME);
        } while (--count);

        udelay(AAT_LATCH_TIME);
    }

    local_irq_restore(flags);
}

// R920 : Rset 160K ohm : Max Flash Current 1012mA, Max Movie Current 139mA
static int s5k5ccgx_AAT_flash_control(struct v4l2_subdev *sd, int control_mode)
{
    switch(control_mode)
    {
        // USE FLASH MODE
        case FLASH_CONTROL_MAX_LEVEL:
        {
            gpio_set_value(GPIO_CAM_FLASH_SET, 0);
            gpio_set_value(GPIO_CAM_FLASH_EN, 0);
            udelay(1);

            gpio_set_value(GPIO_CAM_FLASH_EN, 1);
            break;
        }
    
        // USE FLASH MODE
        case FLASH_CONTROL_HIGH_LEVEL:
        {
            gpio_set_value(GPIO_CAM_FLASH_SET, 0);
            gpio_set_value(GPIO_CAM_FLASH_EN, 0);
            udelay(1);

            gpio_set_value(GPIO_CAM_FLASH_EN, 1);
            udelay(10);    // Flash Mode Set time

            s5k5ccgx_AAT_flash_write_data(3);
            break;
        }

        // USE MOVIE MODE : AF Pre-Flash Mode(Torch Mode)
        case FLASH_CONTROL_MIDDLE_LEVEL:
        {
            gpio_set_value(GPIO_CAM_FLASH_SET, 0);
            gpio_set_value(GPIO_CAM_FLASH_EN, 0);
            udelay(1);

            s5k5ccgx_AAT_flash_write_data(1);
            break;
        }

        // USE MOVIE MODE : Movie Mode(Torch Mode)
        case FLASH_CONTROL_LOW_LEVEL:
        {
            gpio_set_value(GPIO_CAM_FLASH_SET, 0);
            gpio_set_value(GPIO_CAM_FLASH_EN, 0); 
            udelay(1);

            s5k5ccgx_AAT_flash_write_data(3);
            break;
        }

        case FLASH_CONTROL_OFF:
        default:
        {
            gpio_set_value(GPIO_CAM_FLASH_SET, 0);
            gpio_set_value(GPIO_CAM_FLASH_EN, 0);
            break;
        }        
    }

    return 0;
}

static int s5k5ccgx_set_flash_mode(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    struct sensor_state *state = to_state(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
    
    CAM_INFO_MSG(&client->dev, "%s : flash mode(%d) ~~~~~~~~~~~~~~\n", __func__, ctrl->value);	
    
	switch(ctrl->value)
    {
        case FLASH_MODE_ON:
        {
            state->current_flash_mode = FLASH_MODE_ON;
            break;
        }

        case FLASH_MODE_AUTO:
        {
            state->current_flash_mode = FLASH_MODE_AUTO;
            break;
        }

        case FLASH_MODE_TORCH_ON:
        {
            s5k5ccgx_AAT_flash_control(sd, FLASH_CONTROL_LOW_LEVEL);
            break;
        }

        case FLASH_MODE_TORCH_OFF:
        {
            s5k5ccgx_AAT_flash_control(sd, FLASH_CONTROL_OFF);
            break;
        }
        
        case FLASH_MODE_OFF:
        default:
        {
            s5k5ccgx_AAT_flash_control(sd, FLASH_CONTROL_OFF);
            state->current_flash_mode = FLASH_MODE_OFF;
            break;
        }        
    }

    return 0;
}

/**************************************************************************
 * s5k5ccgx_set_frame_rate
 *
 * Returns 0 on success, <0 on error
 ***************************************************************************/
static int s5k5ccgx_set_frame_rate(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct sensor_state *state = to_state(sd);
	int err = 0;

    CAM_INFO_MSG(&client->dev, "%s : %d fps ~~~~~~~~~~~~~~\n", __func__, ctrl->value);	
    
	switch(ctrl->value)
	{
		case FRAME_RATE_AUTO:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  fps_auto_regs, 
			                                  FPS_AUTO_NUM_OF_REGS, 
			                                  "fps_auto_regs");
			break;
        }
        
		case FRAME_RATE_7:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  fps_7_regs, 
			                                  FPS_7_NUM_OF_REGS, 
			                                  "fps_7_regs");
			break;
        }
        
		case FRAME_RATE_10:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  fps_10_regs, 
			                                  FPS_10_NUM_OF_REGS, 
			                                  "fps_10_regs");
			break;
        }
        
		case FRAME_RATE_15:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  fps_15_regs, 
			                                  FPS_15_NUM_OF_REGS, 
			                                  "fps_15_regs");
			break;
        }
        
		case FRAME_RATE_30:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  fps_30_regs, 
			                                  FPS_30_NUM_OF_REGS, 
			                                  "fps_30_regs");
			break;
        }
        
		default:
		{
			CAM_WARN_MSG(&client->dev, "[%s : %d] WARNING! unsupported %d fps\n", 
			            __FILE__, __LINE__, ctrl->value);
            err = -EIO;       
			break;
        }			
	}

	if(err < 0)
	{
		CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! frame rate set failed\n", 
		                __FILE__, __LINE__);
		return -EIO;
	}

	if(ctrl->value == FRAME_RATE_AUTO)
	{
	    state->fps = 30;    // default 30
    }	
    else
    {
	    state->fps = ctrl->value;
    }
	
	return 0;
}

/**************************************************************************
 * s5k5ccgx_set_preview_stop
 *
 * Returns 0 on success, <0 on error
 ***************************************************************************/
static int s5k5ccgx_set_preview_stop(struct v4l2_subdev *sd)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct sensor_state *state = to_state(sd);

	if(state->runmode == RUNNING_MODE_RUNNING)
	{
		state->runmode = RUNNING_MODE_IDLE;
	}

	CAM_INFO_MSG(&client->dev, "%s ~~~~~~~~~~~~~~\n", __func__);	
	return 0;
}

/**************************************************************************
 * s5k5ccgx_set_dzoom
 *
 * Returns 0 on success, <0 on error
 ***************************************************************************/
static int s5k5ccgx_set_dzoom(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;

	CAM_INFO_MSG(&client->dev, "%s : %d level ~~~~~~~~~~~~~~\n", __func__, ctrl->value);	

	switch (ctrl->value) 
	{
		case ZOOM_LEVEL_0:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  zoom_00_regs, 
			                                  ZOOM_00_NUM_OF_REGS, 
			                                  "zoom_00_regs");
			break;
        }
        
		case ZOOM_LEVEL_1:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  zoom_01_regs, 
			                                  ZOOM_01_NUM_OF_REGS, 
			                                  "zoom_01_regs");
			break;
        }
        
		case ZOOM_LEVEL_2:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  zoom_02_regs, 
			                                  ZOOM_02_NUM_OF_REGS, 
			                                  "zoom_02_regs");
			break;
        }
        
		case ZOOM_LEVEL_3:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  zoom_03_regs, 
			                                  ZOOM_03_NUM_OF_REGS, 
			                                  "zoom_03_regs");
			break;
        }
        
		case ZOOM_LEVEL_4:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  zoom_04_regs, 
			                                  ZOOM_04_NUM_OF_REGS, 
			                                  "zoom_04_regs");
			break;
        }
        
		case ZOOM_LEVEL_5:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  zoom_05_regs, 
			                                  ZOOM_05_NUM_OF_REGS, 
			                                  "zoom_05_regs");
			break;
        }
        
		case ZOOM_LEVEL_6:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  zoom_06_regs, 
			                                  ZOOM_06_NUM_OF_REGS, 
			                                  "zoom_06_regs");
			break;
        }
        
		case ZOOM_LEVEL_7:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  zoom_07_regs, 
			                                  ZOOM_07_NUM_OF_REGS, 
			                                  "zoom_07_regs");
			break;
        }
        
		case ZOOM_LEVEL_8:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  zoom_08_regs, 
			                                  ZOOM_08_NUM_OF_REGS, 
			                                  "zoom_08_regs");
			break;
        }
        
		default:
		{
		    CAM_WARN_MSG(&client->dev, "[%s : %d] WARNING! unsupported zoom(%d) value\n", __FILE__, __LINE__, ctrl->value);
			break;			
        }			
	}

	if(err < 0)
	{
	    CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! zoom set failed\n", __FILE__, __LINE__);
		return -EIO;
	}
	
	return 0;
}

/**************************************************************************
 * s5k5ccgx_set_preview_size
 *
 * Returns 0 on success, <0 on error
 ***************************************************************************/
static int s5k5ccgx_set_preview_size(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct sensor_state *state = to_state(sd);
	int err = 0, index;	
	
	index = state->framesize_index;

    CAM_INFO_MSG(&client->dev, "%s : framesize_index %d  ~~~~~~~~~~~~~~\n", __func__, index);

	switch(index)
	{
        case PREVIEW_SIZE_XGA:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  preview_size_1024x768_regs, 
			                                  PREVIEW_SIZE_1024X768_NUM_OF_REGS, 
			                                  "preview_size_1024x768_regs");
			break;
        }		

		case PREVIEW_SIZE_D1:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  preview_size_720x480_regs, 
			                                  PREVIEW_SIZE_720X480_NUM_OF_REGS, 
			                                  "preview_size_720x480_regs");
			break;
        }

		case PREVIEW_SIZE_4CIF:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  preview_size_704x576_regs, 
			                                  PREVIEW_SIZE_704X576_NUM_OF_REGS, 
			                                  "preview_size_704x576_regs");
			break;
        }

        case PREVIEW_SIZE_VGA:
        {
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  preview_size_640x480_regs, 
			                                  PREVIEW_SIZE_640X480_NUM_OF_REGS, 
			                                  "preview_size_640x480_regs");
            break;			                                  
        }

		default:
		{
            CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! not support preview size(%d)\n", __FILE__, __LINE__, index);
            err = -EINVAL;
		    break;
		}
        
	}

    /* Set preview image size */
    if(err < 0)
    {
        CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! preview size set failed\n", __FILE__, __LINE__);
        return err; 
    }

	return 0;	
}

static int s5k5ccgx_set_preview_start(struct v4l2_subdev *sd)
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

	err = s5k5ccgx_set_preview_size(sd);
	if(err < 0)
	{
		CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! Could not set preview size\n", __FILE__, __LINE__);
		return -EIO;
	}
    
	err = s5k5ccgx_i2c_set_config_register(client, 
	                                  preview_camera_regs, 
	                                  PREVIEW_CAMERA_NUM_OF_REGS, 
	                                  "preview_camera_regs");
	if(err < 0)
	{
		CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! Could not start preview\n", __FILE__, __LINE__);
		return -EIO;
	}

	state->runmode = RUNNING_MODE_RUNNING;
	
    state->camera_flash_fire = 0;
    state->camera_af_flash_fire = 0;
    
	if(state->check_dataline) // Output Test Pattern
	{
        err = s5k5ccgx_check_dataline_onoff(sd, 1);	
        
		if(err < 0)
		{
			CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! Pattern setting failed\n", __FILE__, __LINE__);	
			return -EIO;
		}		
	}

	return 0;
}

static int s5k5ccgx_set_jpeg_quality(struct v4l2_subdev *sd)
{
	struct sensor_state *state = to_state(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err, quality;

    quality = state->jpeg_param.quality;

	CAM_INFO_MSG(&client->dev, "%s: setting value =%d\n", __func__, quality);

	switch(quality) 
	{
		case JPEG_QUALITY_SUPERFINE:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  quality_superfine_regs, 
			                                  QUALITY_SUPERFINE_NUM_OF_REGS, 
			                                  "quality_superfine_regs");
			break;
        }

		case JPEG_QUALITY_FINE:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  quality_fine_regs, 
			                                  QUALITY_FINE_NUM_OF_REGS, 
			                                  "quality_fine_regs");
			break;
        }
        
		case JPEG_QUALITY_NORMAL:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  quality_normal_regs, 
			                                  QUALITY_NORMAL_NUM_OF_REGS, 
			                                  "quality_normal_regs");
			break;
        }
        
		case JPEG_QUALITY_ECONOMY:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  quality_economy_regs, 
			                                  QUALITY_ECONOMY_NUM_OF_REGS, 
			                                  "quality_economy_regs");
			break;
        }

        default:
        {
            CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! not support quality(%d)\n", __FILE__, __LINE__, quality);
            err = -EINVAL;
            break;                
        }
    }        
    
	if(err < 0)
	{
		CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! quality set failed\n", __FILE__, __LINE__);
		return -EIO;
	}

    CAM_INFO_MSG(&client->dev, "%s : quality %d set done~~~~~~~~~~~~~~\n", __func__, quality);

	return 0;
}

static int s5k5ccgx_get_low_light_condition(struct v4l2_subdev *sd, int *Result)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);	
    unsigned short read_value1, read_value2;
    int NB_value = 0;

    s5k5ccgx_i2c_write_word(client, 0xFCFC, 0xD000);
    s5k5ccgx_i2c_write_word(client, 0x002C, 0x7000);
    s5k5ccgx_i2c_write_word(client, 0x002E, 0x2A3C);
    s5k5ccgx_i2c_read_word(client, 0x0F12, &read_value1);   // LSB (0x2A3C)
    s5k5ccgx_i2c_read_word(client, 0x0F12, &read_value2);   // MSB (0x2A3E)

    NB_value = (int)read_value2;
    NB_value = ((NB_value << 16) | (read_value1 & 0xFFFF));
    
    if(NB_value > 0xFFFE)
    {
        *Result = CAM_HIGH_LIGHT;
	    CAM_INFO_MSG(&client->dev,"%s : Highlight Read(0x%X) \n", __func__, NB_value);
    }
    else if(NB_value > 0x0020)
    {
        *Result = CAM_NORMAL_LIGHT;
	    CAM_INFO_MSG(&client->dev,"%s : Normallight Read(0x%X) \n", __func__, NB_value);
    }
    else
    {
        *Result = CAM_LOW_LIGHT;
	    CAM_INFO_MSG(&client->dev,"%s : Lowlight Read(0x%X) \n", __func__, NB_value);
    }
    
	return 0;
}

static int s5k5ccgx_set_capture_size(struct v4l2_subdev *sd)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct sensor_state *state = to_state(sd);
    int err = 0, index;
    
    index = state->framesize_index;

    CAM_INFO_MSG(&client->dev, "%s : framesize_index %d  ~~~~~~~~~~~~~~\n", __func__, index);

    switch(index)
    {
        // ======================= 1.333 Ratio =======================================
        case CAPTURE_SIZE_PREVIEW: /* 416x320 */
        {
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  capture_size_416x320_regs, 
			                                  CAPTURE_SIZE_416X320_NUM_OF_REGS, 
			                                  "capture_size_416x320_regs");

            state->postview_info.width = 320;
            state->postview_info.height = 240;			                                  
            break;		
        }
        
        case CAPTURE_SIZE_VGA: /* 640x480 */
        {
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  capture_size_640x480_regs, 
			                                  CAPTURE_SIZE_640X480_NUM_OF_REGS, 
			                                  "capture_size_640x480_regs");

            state->postview_info.width = 320;
            state->postview_info.height = 240;			                                  
            break;
        }

        case CAPTURE_SIZE_SVGA: /* 800x600 */
        {
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  capture_size_800x600_regs, 
			                                  CAPTURE_SIZE_800X600_NUM_OF_REGS, 
			                                  "capture_size_800x600_regs");

            state->postview_info.width = 320;
            state->postview_info.height = 240;			                                  
            break;
        }

        case CAPTURE_SIZE_XGA: /* 1024x768 */
        {
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  capture_size_1024x768_regs, 
			                                  CAPTURE_SIZE_1024X768_NUM_OF_REGS, 
			                                  "capture_size_1024x768_regs");

            state->postview_info.width = 320;
            state->postview_info.height = 240;			                                  
            break;		
        }            
 
        case CAPTURE_SIZE_4VGA: /* 1280x960 */
        {
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  capture_size_1280x960_regs, 
			                                  CAPTURE_SIZE_1280X960_NUM_OF_REGS, 
			                                  "capture_size_1280x960_regs");

            state->postview_info.width = 320;
            state->postview_info.height = 240;			                                  
            break;		
        }
        
        case CAPTURE_SIZE_UXGA: /* 1600x1200 */
        {
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  capture_size_1600x1200_regs, 
			                                  CAPTURE_SIZE_1600X1200_NUM_OF_REGS, 
			                                  "capture_size_1600x1200_regs");

            state->postview_info.width = 320;
            state->postview_info.height = 240;			                                  
            break;		
        }
        
        case CAPTURE_SIZE_QXGA: /* 2048x1536 */
        {
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  capture_size_2048x1536_regs, 
			                                  CAPTURE_SIZE_2048X1536_NUM_OF_REGS, 
			                                  "capture_size_2048x1536_regs");

            state->postview_info.width = 320;
            state->postview_info.height = 240;			                                  
            break;		
        }

        // ======================= 1.5 Ratio =======================================
        case CAPTURE_SIZE_D1:  /* 720x480 */
        {
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  capture_size_720x480_regs, 
			                                  CAPTURE_SIZE_720X480_NUM_OF_REGS, 
			                                  "capture_size_720x480_regs");

            state->postview_info.width = 336;
            state->postview_info.height = 224;			                                  
            break;
        }
        
        case CAPTURE_SIZE_USER1: /* 2048x1368 */
        {
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  capture_size_2048x1368_regs, 
			                                  CAPTURE_SIZE_2048X1368_NUM_OF_REGS, 
			                                  "capture_size_2048x1368_regs");

            state->postview_info.width = 336;
            state->postview_info.height = 224;			                                  
            break;		
        }

        default:
        {
            CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! not support capture size(%d)\n", __FILE__, __LINE__, index);
            err = -EINVAL;
            break;                
        }            
    }

    /* Set capture image size */
    if(err < 0)
    {
        CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! capture size set failed\n", __FILE__, __LINE__);
        return err; 
    }

    return 0;	
}

static int s5k5ccgx_set_capture_start(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct sensor_state *state = to_state(sd);
    int err = 0, light_state;

    // Initialize...
	gISOSpeedRating = 100;
	gExposureTime = 0;
	
	// Set image size	
	err = s5k5ccgx_set_capture_size(sd);
	if(err < 0)
	{
		dev_err(&client->dev, "[%s : %d] ERROR! Couldn't set capture size\n", __FILE__, __LINE__);
		return -EIO; 
	}

    if(gCurrentScene == SCENE_MODE_NIGHTSHOT || gCurrentScene == SCENE_MODE_FIREWORKS)
    {
        /* Set Snapshot registers */ 
    	err = s5k5ccgx_i2c_set_config_register(client, 
    	                                  snapshot_nightmode_regs, 
    	                                  SNAPSHOT_NIGHTMODE_NUM_OF_REGS, 
    	                                  "snapshot_nightmode_regs");
    	if(err < 0)
    	{
            CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! Could not take a picture\n", __FILE__, __LINE__);
    		return -EIO;
    	}
    }
    else
    {
        s5k5ccgx_get_low_light_condition(sd, &light_state);

        state->camera_flash_fire = 0;
        
        if(state->current_flash_mode == FLASH_MODE_ON)
        {
            state->camera_flash_fire = 1;
        }
        else if(state->current_flash_mode == FLASH_MODE_AUTO)
        {
            if(state->camera_af_flash_fire)
            {
                state->camera_flash_fire = 1;
            }
            else
            {
                if(light_state == CAM_LOW_LIGHT)
                {
                    state->camera_flash_fire = 1;
                }
            }                
        }

        if(state->camera_flash_fire)
        {
            /* Set Snapshot registers */ 
        	err = s5k5ccgx_i2c_set_config_register(client, 
        	                                  snapshot_flash_on_regs, 
        	                                  SNAPSHOT_FLASH_ON_NUM_OF_REGS, 
        	                                  "snapshot_flash_on_regs");
        	if(err < 0)
        	{
                CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! Couldn't Set Flash_on_regs \n", __FILE__, __LINE__);
        	}
        	
            s5k5ccgx_AAT_flash_control(sd, FLASH_CONTROL_MAX_LEVEL);
        }
        
        if(light_state == CAM_LOW_LIGHT)
        {    
            /* Set Snapshot registers */ 
        	err = s5k5ccgx_i2c_set_config_register(client, 
        	                                  snapshot_lowlight_regs, 
        	                                  SNAPSHOT_LOWLIGHT_NUM_OF_REGS, 
        	                                  "snapshot_lowlight_regs");

        	if(err < 0)
        	{
                CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! Could not take a picture\n", __FILE__, __LINE__);
        		return -EIO;
        	}
        }
        else if(light_state == CAM_HIGH_LIGHT)
        {
            /* Set Snapshot registers */ 
        	err = s5k5ccgx_i2c_set_config_register(client, 
        	                                  snapshot_highlight_regs, 
        	                                  SNAPSHOT_HIGHLIGHT_NUM_OF_REGS, 
        	                                  "snapshot_highlight_regs");
        	if(err < 0)
        	{
                CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! Could not take a picture\n", __FILE__, __LINE__);
        		return -EIO;
        	}
        }
        else // CAM_NORMAL LIGHT
        {
            /* Set Snapshot registers */ 
        	err = s5k5ccgx_i2c_set_config_register(client, 
        	                                  snapshot_normal_regs, 
        	                                  SNAPSHOT_NORMAL_NUM_OF_REGS, 
        	                                  "snapshot_normal_regs");
        	if(err < 0)
        	{
                CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! Could not take a picture\n", __FILE__, __LINE__);
        		return -EIO;
        	}
        }
    }

    // Get iso speed rating and exposure time for EXIF.
    gISOSpeedRating = s5k5ccgx_get_iso_speed_rate(sd);
    gExposureTime = s5k5ccgx_get_shutterspeed(sd);
    
	return 0;
}

static int s5k5ccgx_set_capture_done(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct sensor_state *state = to_state(sd);
	int err = 0;
    
	CAM_INFO_MSG(&client->dev, "%s\n", __func__);

    if(state->camera_af_flash_fire)
    {
		err = s5k5ccgx_i2c_set_config_register(client, 
		                                  snapshot_af_preflash_off_regs, 
		                                  SNAPSHOT_AF_PREFLASH_OFF_NUM_OF_REGS, 
		                                  "snapshot_af_preflash_off_regs");

    	if(err < 0)
    	{
		    CAM_ERROR_MSG(&client->dev, "[%s: %d] ERROR! Setting af_preflash_off_regs\n", __FILE__, __LINE__);
    	}
    }
    
    s5k5ccgx_AAT_flash_control(sd, FLASH_CONTROL_OFF);
    
    return 0;
}

static int s5k5ccgx_set_scene_mode(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;

	CAM_INFO_MSG(&client->dev, "%s: CurrentScene(%d), NextScene(%d)\n", __func__, gCurrentScene, ctrl->value);

	if(ctrl->value != SCENE_MODE_NONE)
	{
	    // Clear the previous scene mode settings.
		err = s5k5ccgx_i2c_set_config_register(client, 
		                                  scene_none_regs, 
		                                  SCENE_NONE_NUM_OF_REGS, 
		                                  "scene_none_regs");

    	if(err < 0)
    	{
    	    CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! scene mode failed\n", __FILE__, __LINE__);
    		return -EIO;
    	}
	}
	
	switch(ctrl->value)
	{
		case SCENE_MODE_NONE:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  scene_none_regs, 
			                                  SCENE_NONE_NUM_OF_REGS, 
			                                  "scene_none_regs");
			break;
        }
        
		case SCENE_MODE_PORTRAIT:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  scene_portrait_regs, 
			                                  SCENE_PORTRAIT_NUM_OF_REGS, 
			                                  "scene_portrait_regs");
			break;
        }

		case SCENE_MODE_NIGHTSHOT:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  scene_nightshot_regs, 
			                                  SCENE_NIGHTSHOT_NUM_OF_REGS, 
			                                  "scene_nightshot_regs");
			break;
        }

		case SCENE_MODE_BACK_LIGHT:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  scene_backlight_regs, 
			                                  SCENE_BACKLIGHT_NUM_OF_REGS, 
			                                  "scene_backlight_regs");
			break;
        }

		case SCENE_MODE_LANDSCAPE:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  scene_landscape_regs, 
			                                  SCENE_LANDSCAPE_NUM_OF_REGS, 
			                                  "scene_landscape_regs");
			break;
        }

		case SCENE_MODE_SPORTS:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  scene_sports_regs, 
			                                  SCENE_SPORTS_NUM_OF_REGS, 
			                                  "scene_sports_regs");
			break;
        }

		case SCENE_MODE_PARTY_INDOOR:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  scene_party_indoor_regs, 
			                                  SCENE_INDOOR_NUM_OF_REGS, 
			                                  "scene_party_indoor_regs");
			break;
        }

		case SCENE_MODE_BEACH_SNOW:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  scene_beach_snow_regs, 
			                                  SCENE_BEACH_NUM_OF_REGS, 
			                                  "scene_beach_snow_regs");
			break;
        }

		case SCENE_MODE_SUNSET:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  scene_sunset_regs, 
			                                  SCENE_SUNSET_NUM_OF_REGS, 
			                                  "scene_sunset_regs");
			break;
        }

		case SCENE_MODE_DUSK_DAWN:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  scene_duskdawn_regs, 
			                                  SCENE_DUSKDAWN_NUM_OF_REGS, 
			                                  "scene_duskdawn_regs");
			break;
        }

		case SCENE_MODE_FALL_COLOR:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  scene_fall_color_regs, 
			                                  SCENE_FALLCOLOR_NUM_OF_REGS, 
			                                  "scene_fall_color_regs");
			break;
        }

		case SCENE_MODE_FIREWORKS:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  scene_fireworks_regs, 
			                                  SCENE_FIREWORKS_NUM_OF_REGS, 
			                                  "scene_fireworks_regs");
			break;
        }

		case SCENE_MODE_TEXT:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  scene_text_regs, 
			                                  SCENE_TEXT_NUM_OF_REGS, 
			                                  "scene_text_regs");
			break;
        }

		case SCENE_MODE_CANDLE_LIGHT:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  scene_candle_light_regs, 
			                                  SCENE_CANDLELIGHT_NUM_OF_REGS, 
			                                  "scene_candle_light_regs");
			break;
        }
			
		default:
		{
		    CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! not support scene(%d)\n", __FILE__, __LINE__, ctrl->value);
		    err = -EINVAL;
			break;
        }			
	}

	if(err < 0)
	{
	    CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! scene mode failed\n", __FILE__, __LINE__);
		return -EIO;
	}
	
	gCurrentScene = ctrl->value;	
	
	return 0;
}

static int s5k5ccgx_set_effect(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;

	CAM_INFO_MSG(&client->dev, "%s: setting value =%d\n", __func__, ctrl->value);

	switch(ctrl->value) 
	{
		case IMAGE_EFFECT_NONE:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  effect_normal_regs, 
			                                  EFFECT_NORMAL_NUM_OF_REGS, 
			                                  "effect_normal_regs");
			break;
		}
		
		case IMAGE_EFFECT_BNW:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  effect_mono_regs, 
			                                  EFFECT_MONO_NUM_OF_REGS, 
			                                  "effect_mono_regs");
			break;
		}
			
		case IMAGE_EFFECT_SEPIA:
		case IMAGE_EFFECT_ANTIQUE:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  effect_sepia_regs, 
			                                  EFFECT_SEPIA_NUM_OF_REGS, 
			                                  "effect_sepia_regs");
			break;
		}
			
		case IMAGE_EFFECT_NEGATIVE:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  effect_negative_regs, 
			                                  EFFECT_NEGATIVE_NUM_OF_REGS, 
			                                  "effect_negative_regs");
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

static int s5k5ccgx_set_white_balance(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;

	CAM_INFO_MSG(&client->dev, "%s: setting value =%d\n", __func__, ctrl->value);

	switch(ctrl->value)
	{
		case WHITE_BALANCE_AUTO:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  wb_auto_regs, 
			                                  WB_AUTO_NUM_OF_REGS, 
			                                  "wb_auto_regs");
			break;
		}

		case WHITE_BALANCE_SUNNY:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  wb_sunny_regs, 
			                                  WB_SUNNY_NUM_OF_REGS, 
			                                  "wb_sunny_regs");
			break;
		}

		case WHITE_BALANCE_CLOUDY:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  wb_cloudy_regs, 
			                                  WB_CLOUDY_NUM_OF_REGS, 
			                                  "wb_cloudy_regs");
			break;
		}

		case WHITE_BALANCE_TUNGSTEN:  // WHITE_BALANCE_INCANDESCENT:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  wb_tungsten_regs, 
			                                  WB_TUNSTEN_NUM_OF_REGS, 
			                                  "wb_tungsten_regs");
			break;
		}
		
		case WHITE_BALANCE_FLUORESCENT:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  wb_fluorescent_regs, 
			                                  WB_FLUORESCENT_NUM_OF_REGS, 
			                                  "wb_fluorescent_regs");
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

    gCurrentWB = ctrl->value;
    
	return 0;
}

static int s5k5ccgx_set_metering(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;

	CAM_INFO_MSG(&client->dev, "%s: setting value =%d\n", __func__, ctrl->value);

	switch(ctrl->value)
	{
		case METERING_MATRIX:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  metering_matrix_regs, 
			                                  METERING_MATRIX_NUM_OF_REGS, 
			                                  "metering_matrix_regs");
			break;
        }
        
		case METERING_CENTER:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  metering_center_regs, 
			                                  METERING_CENTER_NUM_OF_REGS, 
			                                  "metering_center_regs");
			break;
        }

		case METERING_SPOT:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  metering_spot_regs, 
			                                  METERING_SPOT_NUM_OF_REGS, 
			                                  "metering_spot_regs");
			break;
        }

		default:
		{
		    CAM_WARN_MSG(&client->dev, "[%s : %d] WARNING! unsupported metering(%d)\n", __FILE__, __LINE__, ctrl->value);
			break;
        }			
	}
	
	if(err < 0)
	{
	    CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! metering set failed\n", __FILE__, __LINE__);
		return -EIO;
	}

    gCurrentMetering = ctrl->value;

	return 0;
}

static int s5k5ccgx_set_iso(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    
    CAM_INFO_MSG(&client->dev, "[%s] Not support set ISO\n", __func__);
	return 0;
}

static int s5k5ccgx_set_ev(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;

    CAM_INFO_MSG(&client->dev, "%s: setting value =%d\n", __func__, ctrl->value);
    
	switch(ctrl->value)
	{
		case EV_MINUS_4:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  ev_minus_4_regs, 
			                                  EV_M4_NUM_OF_REGS, 
			                                  "ev_minus_4_regs");
			break;
        }

		case EV_MINUS_3:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  ev_minus_3_regs, 
			                                  EV_M3_NUM_OF_REGS, 
			                                  "ev_minus_3_regs");
			break;
        }

		case EV_MINUS_2:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  ev_minus_2_regs, 
			                                  EV_M2_NUM_OF_REGS, 
			                                  "ev_minus_2_regs");
			break;
        }

		case EV_MINUS_1:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  ev_minus_1_regs, 
			                                  EV_M1_NUM_OF_REGS, 
			                                  "ev_minus_1_regs");
			break;
        }

		case EV_DEFAULT:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  ev_default_regs, 
			                                  EV_DEFAULT_NUM_OF_REGS, 
			                                  "ev_default_regs");
			break;
        }

		case EV_PLUS_1:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  ev_plus_1_regs, 
			                                  EV_P1_NUM_OF_REGS, 
			                                  "ev_plus_1_regs");
			break;
        }

		case EV_PLUS_2:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  ev_plus_2_regs, 
			                                  EV_P2_NUM_OF_REGS, 
			                                  "ev_plus_2_regs");
			break;
        }
		
		case EV_PLUS_3:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  ev_plus_3_regs, 
			                                  EV_P3_NUM_OF_REGS, 
			                                  "ev_plus_3_regs");
			break;
        }

		case EV_PLUS_4:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  ev_plus_4_regs, 
			                                  EV_P4_NUM_OF_REGS, 
			                                  "ev_plus_4_regs");
			break;
        }

		default:
		{
			CAM_WARN_MSG(&client->dev, "[%s : %d] WARNING! unsupported ev(%d)\n", __FILE__, __LINE__, ctrl->value);
			break;
        }			
	}

	if(err < 0)
	{
		CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! ev set failed\n", __FILE__, __LINE__);
		return -EIO;
	}

	return 0;
}

static int s5k5ccgx_set_saturation(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;

	CAM_INFO_MSG(&client->dev, "%s: setting value =%d\n", __func__, ctrl->value);

	switch(ctrl->value)
	{
		case SATURATION_MINUS_2:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  saturation_minus_2_regs, 
			                                  SATURATION_M2_NUM_OF_REGS, 
			                                  "saturation_minus_2_regs");
			break;
        }

		case SATURATION_MINUS_1:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  saturation_minus_1_regs, 
			                                  SATURATION_M1_NUM_OF_REGS, 
			                                  "saturation_minus_1_regs");
			break;
        }

		case SATURATION_DEFAULT:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  saturation_default_regs, 
			                                  SATURATION_DEFAULT_NUM_OF_REGS, 
			                                  "saturation_default_regs");
			break;
        }

		case SATURATION_PLUS_1:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  saturation_plus_1_regs, 
			                                  SATURATION_P1_NUM_OF_REGS, 
			                                  "saturation_plus_1_regs");
			break;
        }

		case SATURATION_PLUS_2:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  saturation_plus_2_regs, 
			                                  SATURATION_P2_NUM_OF_REGS, 
			                                  "saturation_plus_2_regs");
			break;
        }
		
		default:
		{
		    CAM_WARN_MSG(&client->dev, "[%s : %d] WARNING! unsupported saturation(%d)\n", __FILE__, __LINE__, ctrl->value);
			break;
        }			
	}

	if(err < 0)
	{
        CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! saturation set failed\n", __FILE__, __LINE__);
		return -EIO;
	}

	return 0;
}

static int s5k5ccgx_set_contrast(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;

	CAM_INFO_MSG(&client->dev, "%s: setting value =%d\n", __func__, ctrl->value);

	switch(ctrl->value)
	{
		case CONTRAST_MINUS_2:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  contrast_minus_2_regs, 
			                                  CONTRAST_M2_NUM_OF_REGS, 
			                                  "contrast_minus_2_regs");
			break;
        }

		case CONTRAST_MINUS_1:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  contrast_minus_1_regs, 
			                                  CONTRAST_M1_NUM_OF_REGS, 
			                                  "contrast_minus_1_regs");
			break;
        }

		case CONTRAST_DEFAULT:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  contrast_default_regs, 
			                                  CONTRAST_DEFAULT_NUM_OF_REGS, 
			                                  "contrast_default_regs");
			break;
        }

		case CONTRAST_PLUS_1:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  contrast_plus_1_regs, 
			                                  CONTRAST_P1_NUM_OF_REGS, 
			                                  "contrast_plus_1_regs");
			break;
        }

		case CONTRAST_PLUS_2:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  contrast_plus_2_regs, 
			                                  CONTRAST_P2_NUM_OF_REGS, 
			                                  "contrast_plus_2_regs");
			break;
        }

		default:
		{
			CAM_WARN_MSG(&client->dev, "[%s : %d] WARNING! unsupported constrast(%d)\n", __FILE__, __LINE__, ctrl->value);
			break;			
        }   			
	}

	if(err < 0)
	{
	    CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! constrast set failed\n", __FILE__, __LINE__);
		return -EIO;
	}

	return 0;
}

static int s5k5ccgx_set_sharpness(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;

	CAM_INFO_MSG(&client->dev, "%s: setting value =%d\n", __func__, ctrl->value);

	switch(ctrl->value)
	{
		case SHARPNESS_MINUS_2:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  sharpness_minus_2_regs, 
			                                  SHARPNESS_M2_NUM_OF_REGS, 
			                                  "sharpness_minus_2_regs");
			break;
        }

		case SHARPNESS_MINUS_1:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  sharpness_minus_1_regs, 
			                                  SHARPNESS_M1_NUM_OF_REGS, 
			                                  "sharpness_minus_1_regs");
			break;
        }

		case SHARPNESS_DEFAULT:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  sharpness_default_regs, 
			                                  SHARPNESS_DEFAULT_NUM_OF_REGS, 
			                                  "sharpness_default_regs");
			break;
        }

		case SHARPNESS_PLUS_1:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  sharpness_plus_1_regs, 
			                                  SHARPNESS_P1_NUM_OF_REGS, 
			                                  "sharpness_plus_1_regs");
			break;
        }

		case SHARPNESS_PLUS_2:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  sharpness_plus_2_regs, 
			                                  SHARPNESS_P2_NUM_OF_REGS, 
			                                  "sharpness_plus_2_regs");
			break;
        }

		default:
		{
		    CAM_WARN_MSG(&client->dev, "[%s : %d] WARNING! unsupported sharpness(%d)\n", __FILE__, __LINE__, ctrl->value);
			break;			
        }			
	}

	if(err < 0)
	{
	    CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! sharpness set failed\n", __FILE__, __LINE__);
		return -EIO;
	}

	return 0;
}

static int s5k5ccgx_set_focus_mode(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;

	CAM_INFO_MSG(&client->dev, "%s: setting value =%d\n", __func__, ctrl->value);

	switch(ctrl->value) 
	{
		case FOCUS_MODE_MACRO:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  af_macro_mode_regs, 
			                                  AF_MACRO_NUM_OF_REGS, 
			                                  "af_macro_mode_regs");
			break;
        }

		case FOCUS_MODE_AUTO:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  af_normal_mode_regs, 
			                                  AF_NORMAL_NUM_OF_REGS, 
			                                  "af_normal_mode_regs");
			break;
        }

		default:
		{
		    CAM_WARN_MSG(&client->dev, "[%s : %d] WARNING! unsupported focus(%d)\n", __FILE__, __LINE__, ctrl->value);
			break;			
        }			
	}

	if(err < 0)
	{
        CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! focus set failed\n", __FILE__, __LINE__);
		return -EIO;
	}

    af_mode = ctrl->value;
    
	return 0;
}

// cmk 2010.09.29 Apply new AF routine.
static int s5k5ccgx_set_AF_default_position(struct v4l2_subdev *sd)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int err = 0;

    CAM_INFO_MSG(&client->dev, "%s: setting value =%d\n", __func__, af_mode);

	switch(af_mode) 
	{
		case FOCUS_MODE_MACRO:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  af_macro_mode_regs, 
			                                  AF_MACRO_NUM_OF_REGS, 
			                                  "af_macro_mode_regs");
			break;
        }

		case FOCUS_MODE_AUTO:
		{
			err = s5k5ccgx_i2c_set_config_register(client, 
			                                  af_normal_mode_regs, 
			                                  AF_NORMAL_NUM_OF_REGS, 
			                                  "af_normal_mode_regs");
			break;
        }

		default:
		{
		    CAM_WARN_MSG(&client->dev, "[%s : %d] WARNING! unsupported default focus(%d)\n", __FILE__, __LINE__, af_mode);
			break;			
        }			
	}

	if(err < 0)
	{
        CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! default focus set failed\n", __FILE__, __LINE__);
		return -EIO;
	}
	
    return 0;
}

static int s5k5ccgx_set_touch_auto_focus(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct sensor_state *state = to_state(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	unsigned short FirstWinStartX, FirstWinStartY, SecondWinStartX, SecondWinStartY;
    unsigned int aeX = 0, aeY = 0;
    unsigned int pos = 0;
	int preview_width, preview_height;
    int err = 0;
    regs_short_t ae_weight[METERING_CENTER_NUM_OF_REGS];

	if(ctrl->value == TOUCH_AF_START)
	{
        preview_width = state->af_info.preview_width;
        preview_height = state->af_info.preview_height;

        // Prevent divided-by-zero.
        if(preview_width == 0 || preview_height == 0)
        {
            CAM_ERROR_MSG(&client->dev, "%s: Either preview_width or preview_height is zero\n", __func__);
            return -EIO;
        }

        FirstWinStartX = state->af_info.x;
        FirstWinStartY = state->af_info.y;
        
        // AF Position(Round Down)
        if(FirstWinStartX > AF_OUTER_WINDOW_WIDTH/2)
        {
            FirstWinStartX -= AF_OUTER_WINDOW_WIDTH/2;

    	    if(FirstWinStartX + AF_OUTER_WINDOW_WIDTH > preview_width)
    	    {
    	        CAM_ERROR_MSG(&client->dev, "%s: X Position Overflow : [%d, %d] \n", __func__, FirstWinStartX, AF_OUTER_WINDOW_WIDTH);
    	        
    	        FirstWinStartX = preview_width - AF_OUTER_WINDOW_WIDTH - 1;
    	    }
        }
        else
        {
            FirstWinStartX = 0;
        }

        if(FirstWinStartY > AF_OUTER_WINDOW_HEIGHT/2)
        {
            FirstWinStartY -= AF_OUTER_WINDOW_HEIGHT/2;

    	    if(FirstWinStartY + AF_OUTER_WINDOW_HEIGHT > preview_height)
    	    {
    	        CAM_ERROR_MSG(&client->dev, "%s: Y Position Overflow : [%d, %d] \n", __func__, FirstWinStartY, AF_OUTER_WINDOW_HEIGHT);
    	        
    	        FirstWinStartY = preview_height - AF_OUTER_WINDOW_HEIGHT - 1;
    	    }
        }
        else
        {
            FirstWinStartY = 0;
        }

    	FirstWinStartX = (unsigned short)((FirstWinStartX * 1024) / preview_width);
    	FirstWinStartY = (unsigned short)((FirstWinStartY * 1024) / preview_height);

        SecondWinStartX = FirstWinStartX + 140;
        SecondWinStartY = FirstWinStartY + 131;

        mutex_lock(&af_cancel_op);
        
        err  = s5k5ccgx_i2c_write_word(client, 0xFCFC, 0xD000);
        err += s5k5ccgx_i2c_write_word(client, 0x0028, 0x7000);
        err += s5k5ccgx_i2c_write_word(client, 0x002A, 0x022C);
        err += s5k5ccgx_i2c_write_word(client, 0x0F12, FirstWinStartX);         // FirstWinStartX
        err += s5k5ccgx_i2c_write_word(client, 0x0F12, FirstWinStartY);         // FirstWinStartY
        err += s5k5ccgx_i2c_write_word(client, 0x0F12, 0x0200);                 // FirstWinSizeX
        err += s5k5ccgx_i2c_write_word(client, 0x0F12, 0x0238);                 // FirstWinSizeY
        err += s5k5ccgx_i2c_write_word(client, 0x0F12, SecondWinStartX);        // SecondWinStartX
        err += s5k5ccgx_i2c_write_word(client, 0x0F12, SecondWinStartY);        // SecondWinStartY
        err += s5k5ccgx_i2c_write_word(client, 0x0F12, 0x00E6);                 // SecondWinSizeX
        err += s5k5ccgx_i2c_write_word(client, 0x0F12, 0x0132);                 // SecondWinSizeY
        err += s5k5ccgx_i2c_write_word(client, 0x0F12, 0x0001);                 // WindowSizeUpdated

        mutex_unlock(&af_cancel_op);
        
        // Apply Touch AE Weight. (Use center-weighted metering table.)
        memcpy(ae_weight, metering_center_regs, sizeof(metering_center_regs));

        aeX = state->af_info.x / (state->af_info.preview_width / 8);
        aeY = state->af_info.y / (state->af_info.preview_height / 8);

        // Find the corresponding index of ae_weight array.
        pos = (((aeY * 8) + aeX) / 2) + 3;  // +3 means {0xFCFC, 0xD000}, {0x0028, 0x7000}, {0x002A, 0x1316}

        if(pos < METERING_CENTER_NUM_OF_REGS)
        {
            if(aeX % 2 == 0)
            {
                ae_weight[pos].value |= 0x0020;  // 0x000F => 0x0020
            }            
            else
            {
                ae_weight[pos].value |= 0x2000;  // 0x0F00 => 0x2000
            }
        }
        
		err += s5k5ccgx_i2c_set_data_burst(client, 
		                                  ae_weight, 
		                                  METERING_CENTER_NUM_OF_REGS);

	    CAM_INFO_MSG(&client->dev, "%s: Start AF Pos[%d %d]\n", __func__, FirstWinStartX, FirstWinStartY);
    }
    else
    {
        mutex_lock(&af_cancel_op);
    
        err  = s5k5ccgx_i2c_write_word(client, 0xFCFC, 0xD000);
        err += s5k5ccgx_i2c_write_word(client, 0x0028, 0x7000);
        err += s5k5ccgx_i2c_write_word(client, 0x002A, 0x022C);
        err += s5k5ccgx_i2c_write_word(client, 0x0F12, 0x0100);    // FirstWinStartX
        err += s5k5ccgx_i2c_write_word(client, 0x0F12, 0x00E3);    // FirstWinStartY
        err += s5k5ccgx_i2c_write_word(client, 0x0F12, 0x0200);    // FirstWinSizeX
        err += s5k5ccgx_i2c_write_word(client, 0x0F12, 0x0238);    // FirstWinSizeY
        err += s5k5ccgx_i2c_write_word(client, 0x0F12, 0x018C);    // SecondWinStartX
        err += s5k5ccgx_i2c_write_word(client, 0x0F12, 0x0166);    // SecondWinStartY
        err += s5k5ccgx_i2c_write_word(client, 0x0F12, 0x00E6);    // SecondWinSizeX
        err += s5k5ccgx_i2c_write_word(client, 0x0F12, 0x0132);    // SecondWinSizeY
        err += s5k5ccgx_i2c_write_word(client, 0x0F12, 0x0001);    // WindowSizeUpdated

        mutex_unlock(&af_cancel_op);
        
        // Apply Touch AE Weight.
        // Touch AE restoration. Set to previous metering values.            
    	switch(gCurrentMetering)
    	{
    		case METERING_MATRIX:
    		{
    			err += s5k5ccgx_i2c_set_config_register(client, 
    			                                  metering_matrix_regs, 
    			                                  METERING_MATRIX_NUM_OF_REGS, 
    			                                  "metering_matrix_regs");
    			break;
            }
            
    		case METERING_CENTER:
    		{
    			err += s5k5ccgx_i2c_set_config_register(client, 
    			                                  metering_center_regs, 
    			                                  METERING_CENTER_NUM_OF_REGS, 
    			                                  "metering_center_regs");
    			break;
            }

    		case METERING_SPOT:
    		{
    			err += s5k5ccgx_i2c_set_config_register(client, 
    			                                  metering_spot_regs, 
    			                                  METERING_SPOT_NUM_OF_REGS, 
    			                                  "metering_spot_regs");
    			break;
            }

    		default:
    		{
    		    CAM_WARN_MSG(&client->dev, "[%s : %d] WARNING! unsupported metering(%d)\n", __FILE__, __LINE__, ctrl->value);
    			break;
            }			
    	}
	}

	if(err < 0)
	{
		CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! touch AF set failed\n", __FILE__, __LINE__);
		return -EIO;
	}

    CAM_INFO_MSG(&client->dev, "%s: Stop AF Pos\n", __func__);

	return 0;
}

// cmk 2010.09.29 Apply new AF routine.
static int s5k5ccgx_set_auto_focus(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct sensor_state *state = to_state(sd);
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int err = 0;

    mutex_lock(&af_cancel_op);

    // Initialize fine search value.
    bStartFineSearch = false;
    
    if(ctrl->value == AUTO_FOCUS_ON) 
    {
        CAM_INFO_MSG(&client->dev, "%s: AF Start~~~~~~~\n", __func__);		

		err = s5k5ccgx_i2c_set_config_register(client, 
		                                  single_af_start_regs, 
		                                  SINGLE_AF_START_NUM_OF_REGS, 
		                                  "single_af_start_regs");

    	if(err < 0)
    	{
    		CAM_ERROR_MSG(&client->dev, "[%s: %d] ERROR! Starting AF Failed\n", __FILE__, __LINE__);
    		mutex_unlock(&af_cancel_op);
    		return -EIO;
    	}		
    }
    else if(ctrl->value == AUTO_FOCUS_OFF) 
    {
        CAM_INFO_MSG(&client->dev, "%s: AF Stop~~~~~~~\n", __func__);	

		err = s5k5ccgx_i2c_set_config_register(client, 
		                                  single_af_stop_regs, 
		                                  SINGLE_AF_STOP_NUM_OF_REGS, 
		                                  "single_af_stop_regs");

    	if(err < 0)
    	{
    		CAM_ERROR_MSG(&client->dev, "[%s: %d] ERROR! Stopping AF Failed\n", __FILE__, __LINE__);
    		mutex_unlock(&af_cancel_op);
    		return -EIO;
    	}		
    }

    mutex_unlock(&af_cancel_op);

    return 0;
}

// cmk 2010.09.29 Apply new AF routine.
static int s5k5ccgx_get_auto_focus_status(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{   
    struct sensor_state *state = to_state(sd);
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    unsigned short AF_status = 0;
    int err = 0;

    mutex_lock(&af_cancel_op);

    if(!bStartFineSearch)
    {
        err  = s5k5ccgx_i2c_write_word(client, 0xFCFC, 0xD000);
        err += s5k5ccgx_i2c_write_word(client, 0x002C, 0x7000);
        err += s5k5ccgx_i2c_write_word(client, 0x002E, 0x2D12);
        err += s5k5ccgx_i2c_read_word(client, 0x0F12, &AF_status);

        if(err < 0)
        {
    		CAM_ERROR_MSG(&client->dev, "%s: AF is Failure~~~~~~~(I2C Failed) \n", __func__);
    		ctrl->value = CAMERA_AF_STATUS_FAIL;
    		goto routine_end;
        }
        
    	if(AF_status & 0x0001)   // Check if AF is in progress
    	{
    	    ctrl->value = CAMERA_AF_STATUS_IN_PROGRESS;
    	}
    	else
    	{
        	if(AF_status & 0x0002) 
        	{
        		CAM_WARN_MSG(&client->dev, "%s: AF is success~~~~~~~(Single Search) \n", __func__);

#if 1 // Use Fine Search Algorithm.
        		ctrl->value = CAMERA_AF_STATUS_1ST_SUCCESS; // fine search algorithm.
        		bStartFineSearch = true;
#else
        		ctrl->value = CAMERA_AF_STATUS_SUCCESS; // single search algorithm.
#endif
        	}
        	else
        	{
        		CAM_WARN_MSG(&client->dev, "%s: AF is Failure~~~~~~~(Single Search) \n", __func__);
        		ctrl->value = CAMERA_AF_STATUS_FAIL;
        	}
        }
    }
    else // Fine Search
    {
        err  = s5k5ccgx_i2c_write_word(client, 0xFCFC, 0xD000);
        err += s5k5ccgx_i2c_write_word(client, 0x002C, 0x7000);
        err += s5k5ccgx_i2c_write_word(client, 0x002E, 0x1F2F);
        err += s5k5ccgx_i2c_read_word(client, 0x0F12, &AF_status);

        if(err < 0)
        {
    		CAM_ERROR_MSG(&client->dev, "%s: AF is Failure~~~~~~~(I2C Failed) \n", __func__);

    		ctrl->value = CAMERA_AF_STATUS_FAIL;
    		goto routine_end;
        }

    	if((AF_status & 0xFF00) == 0x0000) 
    	{
    		CAM_WARN_MSG(&client->dev, "%s: AF is success~~~~~~~(Fine Search) \n", __func__);

    		ctrl->value = CAMERA_AF_STATUS_SUCCESS;
    	}
    	else
    	{
    	    CAM_INFO_MSG(&client->dev, "%s: AF is in progress~~~~~~~(Fine Search) \n", __func__);
    		ctrl->value = CAMERA_AF_STATUS_1ST_SUCCESS;
    	}
    }

routine_end:
    mutex_unlock(&af_cancel_op);
    return 0;    
}

// cmk 2010.09.29 Apply new AF routine.
static int s5k5ccgx_set_af_preflash(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    struct sensor_state *state = to_state(sd);
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int err = 0;
    
    if(ctrl->value == PREFLASH_ON)
    {
        state->camera_af_flash_fire = 0;
        
        if(state->current_flash_mode == FLASH_MODE_ON)
        {
            state->camera_af_flash_fire = 1;
        }
        else if(state->current_flash_mode == FLASH_MODE_AUTO)
        {
            int light_state;

            s5k5ccgx_get_low_light_condition(sd, &light_state);
            
    		if(light_state == CAM_LOW_LIGHT)
    		{
    			state->camera_af_flash_fire = 1;
    		}
        }

        if(state->camera_af_flash_fire)
        {
    		err = s5k5ccgx_i2c_set_config_register(client, 
    		                                  snapshot_af_preflash_on_regs, 
    		                                  SNAPSHOT_AF_PREFLASH_ON_NUM_OF_REGS, 
    		                                  "snapshot_af_preflash_on_regs");

        	if(err < 0)
        	{
    		    CAM_ERROR_MSG(&client->dev, "[%s: %d] ERROR! Setting af_preflash_on_regs\n", __FILE__, __LINE__);
        	}
        	
            s5k5ccgx_AAT_flash_control(sd, FLASH_CONTROL_MIDDLE_LEVEL);
        }
        else
        {
            return -1; // Must return a non-zero value, when flash is not fired.
        }
    }
    else // PREFLASH_OFF
    {
        if(state->camera_af_flash_fire)
        {
    		err = s5k5ccgx_i2c_set_config_register(client, 
    		                                  snapshot_af_preflash_off_regs, 
    		                                  SNAPSHOT_AF_PREFLASH_OFF_NUM_OF_REGS, 
    		                                  "snapshot_af_preflash_off_regs");

        	if(err < 0)
        	{
    		    CAM_ERROR_MSG(&client->dev, "[%s: %d] ERROR! Setting af_preflash_off_regs\n", __FILE__, __LINE__);
        	}

            s5k5ccgx_AAT_flash_control(sd, FLASH_CONTROL_OFF);
            state->camera_af_flash_fire = 0;
        }
    }
    
    return 0;    
}

static int s5k5ccgx_get_ae_stable_status(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{   
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int err = 0;
    unsigned short AE_stable = 0x0000;
    
    //Check AE stable
    err = s5k5ccgx_i2c_write_word(client, 0xFCFC, 0xD000);
    err += s5k5ccgx_i2c_write_word(client, 0x002C, 0x7000);
    err += s5k5ccgx_i2c_write_word(client, 0x002E, 0x1E3C);
    err += s5k5ccgx_i2c_read_word(client, 0x0F12, &AE_stable);

    if(err < 0)
    {
        CAM_ERROR_MSG(&client->dev, "[%s: %d] ERROR! AE stable check\n", __FILE__, __LINE__);
    }

    if(AE_stable == 0x0001)
    {
        ctrl->value = AE_STABLE;
    }
    else
    {
        ctrl->value = AE_UNSTABLE;
    }
    
    return 0;
}        
// end cmk

static int s5k5ccgx_AE_AWB_lock(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    struct sensor_state *state = to_state(sd);
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int err = 0;
    
    CAM_INFO_MSG(&client->dev, "%s: setting value =%d\n", __func__, ctrl->value);

    // Lock, Unlock only AE for LSI 5CC sensor. Don't change AWB.
	switch(ctrl->value) 
	{
		case AE_UNLOCK_AWB_UNLOCK:
		{
		    // AE Unlock
            err  = s5k5ccgx_i2c_write_word(client, 0xFCFC, 0xD000);
            err += s5k5ccgx_i2c_write_word(client, 0x0028, 0x7000);
            err += s5k5ccgx_i2c_write_word(client, 0x002A, 0x2A5A);
            err += s5k5ccgx_i2c_write_word(client, 0x0F12, 0x0001);

            // AWB Unlock
            err += s5k5ccgx_i2c_write_word(client, 0xFCFC, 0xD000);
            err += s5k5ccgx_i2c_write_word(client, 0x0028, 0x7000);
            err += s5k5ccgx_i2c_write_word(client, 0x002A, 0x11D6);
            err += s5k5ccgx_i2c_write_word(client, 0x0F12, previous_WB_read);
            break;
        }

		case AE_LOCK_AWB_UNLOCK:
		{
		    // AE Lock
            err  = s5k5ccgx_i2c_write_word(client, 0xFCFC, 0xD000);
            err += s5k5ccgx_i2c_write_word(client, 0x0028, 0x7000);
            err += s5k5ccgx_i2c_write_word(client, 0x002A, 0x2A5A);
            err += s5k5ccgx_i2c_write_word(client, 0x0F12, 0x0000);

            // AWB Unlock
            err += s5k5ccgx_i2c_write_word(client, 0xFCFC, 0xD000);
            err += s5k5ccgx_i2c_write_word(client, 0x0028, 0x7000);
            err += s5k5ccgx_i2c_write_word(client, 0x002A, 0x11D6);
            err += s5k5ccgx_i2c_write_word(client, 0x0F12, previous_WB_read);
            break;
        }

		case AE_UNLOCK_AWB_LOCK:
		{
		    // AE UnLock
            err  = s5k5ccgx_i2c_write_word(client, 0xFCFC, 0xD000);
            err += s5k5ccgx_i2c_write_word(client, 0x0028, 0x7000);
            err += s5k5ccgx_i2c_write_word(client, 0x002A, 0x2A5A);
            err += s5k5ccgx_i2c_write_word(client, 0x0F12, 0x0001);
            
            // AWB lock
            s5k5ccgx_i2c_write_word(client, 0xFCFC, 0xD000);
            s5k5ccgx_i2c_write_word(client, 0x002C, 0x7000);
            s5k5ccgx_i2c_write_word(client, 0x002E, 0x11D6);
            s5k5ccgx_i2c_read_word(client, 0x0F12, &previous_WB_read);

            err += s5k5ccgx_i2c_write_word(client, 0xFCFC, 0xD000);
            err += s5k5ccgx_i2c_write_word(client, 0x0028, 0x7000);
            err += s5k5ccgx_i2c_write_word(client, 0x002A, 0x11D6);
            err += s5k5ccgx_i2c_write_word(client, 0x0F12, 0xFFFF);
            break;
        }

		case AE_LOCK_AWB_LOCK:
		{
		    // AE Lock
            err  = s5k5ccgx_i2c_write_word(client, 0xFCFC, 0xD000);
            err += s5k5ccgx_i2c_write_word(client, 0x0028, 0x7000);
            err += s5k5ccgx_i2c_write_word(client, 0x002A, 0x2A5A);
            err += s5k5ccgx_i2c_write_word(client, 0x0F12, 0x0000);

            // AWB lock
            s5k5ccgx_i2c_write_word(client, 0xFCFC, 0xD000);
            s5k5ccgx_i2c_write_word(client, 0x002C, 0x7000);
            s5k5ccgx_i2c_write_word(client, 0x002E, 0x11D6);
            s5k5ccgx_i2c_read_word(client, 0x0F12, &previous_WB_read);

            // 5CC requirement : Don't Lock AWB when pre-flash was fired.
            if(!state->camera_af_flash_fire)
            {
                err += s5k5ccgx_i2c_write_word(client, 0xFCFC, 0xD000);
                err += s5k5ccgx_i2c_write_word(client, 0x0028, 0x7000);
                err += s5k5ccgx_i2c_write_word(client, 0x002A, 0x11D6);
                err += s5k5ccgx_i2c_write_word(client, 0x0F12, 0xFFFF);
            }
            break;
        }

        default:
        {
            CAM_WARN_MSG(&client->dev, "[%s : %d] WARNING! Unsupported AE, AWB lock setting(%d)\n", __FILE__, __LINE__, ctrl->value);
            break;			
        }			
    }

    if(err < 0)
    {
        CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! AE, AWB lock failed\n", __FILE__, __LINE__);
        return -EIO;
    }
	
    return 0;
}

static int s5k5ccgx_get_iso_speed_rate(struct v4l2_subdev *sd)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    unsigned short read_value = 0;
    int GainValue = 0;
    int isospeedrating = 100;
    
    s5k5ccgx_i2c_write_word(client, 0xFCFC, 0xD000);
    s5k5ccgx_i2c_write_word(client, 0x002C, 0x7000);
    s5k5ccgx_i2c_write_word(client, 0x002E, 0x2A18);
    s5k5ccgx_i2c_read_word(client, 0x0F12, &read_value);

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
}

static int s5k5ccgx_get_shutterspeed(struct v4l2_subdev *sd)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);	
    unsigned short read_value1 = 0, read_value2 = 0;
    int ShutterSpeed = 0;

    s5k5ccgx_i2c_write_word(client, 0xFCFC, 0xD000);
    s5k5ccgx_i2c_write_word(client, 0x002C, 0x7000);
    s5k5ccgx_i2c_write_word(client, 0x002E, 0x2A14);
    s5k5ccgx_i2c_read_word(client, 0x0F12, &read_value1);   // LSB (0x2A14)
    s5k5ccgx_i2c_read_word(client, 0x0F12, &read_value2);   // MSB (0x2A16)

    ShutterSpeed = (int)read_value2;
    ShutterSpeed = (ShutterSpeed << 16) | (read_value1 & 0xFFFF);

    return ((ShutterSpeed * 1000) / 400); // us
}

static void s5k5ccgx_init_parameters(struct v4l2_subdev *sd)
{
	struct sensor_state *state = to_state(sd);

    /* Default value */	
    state->framesize_index = PREVIEW_SIZE_XGA;
    state->fps = 30;
    state->capture_mode = V4L2_MODE_PREVIEW;
    state->sensor_mode = SENSOR_MODE_CAMERA;
    
	/* Set initial values for the sensor stream parameters */
	state->strm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	state->strm.parm.capture.timeperframe.numerator = 1;
	state->strm.parm.capture.timeperframe.denominator = state->fps;
	state->strm.parm.capture.capturemode = 0;

	state->jpeg_param.enable = 0;
	state->jpeg_param.quality = 100;
	state->jpeg_param.main_offset = 0;
	state->jpeg_param.main_size = 0;
	state->jpeg_param.thumb_offset = 0;
	state->jpeg_param.thumb_size = 0;
	state->jpeg_param.postview_offset = 0;

	state->postview_info.width = 320;
	state->postview_info.height = 240;

    state->current_flash_mode = FLASH_MODE_OFF;
    state->camera_flash_fire = 0;
    state->camera_af_flash_fire = 0;
    
    af_mode = FOCUS_MODE_AUTO;
	gCurrentScene = SCENE_MODE_NONE;
	gCurrentWB = WHITE_BALANCE_AUTO;
}

#if 0
/* Sample code */
static const char *s5k5ccgx_querymenu_wb_preset[] = {
	"WB Tungsten", "WB Fluorescent", "WB sunny", "WB cloudy", NULL
};
#endif

static struct v4l2_queryctrl s5k5ccgx_controls[] = {
#if 0
	/* Sample code */
	{
		.id = V4L2_CID_WHITE_BALANCE_PRESET,
		.type = V4L2_CTRL_TYPE_MENU,
		.name = "White balance preset",
		.minimum = 0,
		.maximum = ARRAY_SIZE(s5k5ccgx_querymenu_wb_preset) - 2,
		.step = 1,
		.default_value = 0,
	},
#endif
};

const char **s5k5ccgx_ctrl_get_menu(u32 id)
{
	switch (id) 
	{
#if 0
		/* Sample code */
		case V4L2_CID_WHITE_BALANCE_PRESET:
			return s5k5ccgx_querymenu_wb_preset;
#endif
		default:
			return v4l2_ctrl_get_menu(id);
	}
}

static inline struct v4l2_queryctrl const *s5k5ccgx_find_qctrl(int id)
{
	int i = 0;

	for (i = 0; i < ARRAY_SIZE(s5k5ccgx_controls); i++)
	{
		if (s5k5ccgx_controls[i].id == id)
		{
			return &s5k5ccgx_controls[i];
		}
	}

	return NULL;
}

static int s5k5ccgx_queryctrl(struct v4l2_subdev *sd, struct v4l2_queryctrl *qc)
{
	int i = 0;

	for (i = 0; i < ARRAY_SIZE(s5k5ccgx_controls); i++) 
	{
		if (s5k5ccgx_controls[i].id == qc->id) 
		{
			memcpy(qc, &s5k5ccgx_controls[i], sizeof(struct v4l2_queryctrl));
			return 0;
		}
	}

	return -EINVAL;
}

static int s5k5ccgx_querymenu(struct v4l2_subdev *sd, struct v4l2_querymenu *qm)
{
	struct v4l2_queryctrl qctrl;

	qctrl.id = qm->id;
	s5k5ccgx_queryctrl(sd, &qctrl);

	return v4l2_ctrl_query_menu(qm, &qctrl, s5k5ccgx_ctrl_get_menu(qm->id));
}

/*
 * Clock configuration
 * Configure expected MCLK from host and return EINVAL if not supported clock
 * frequency is expected
 * 	freq : in Hz
 * 	flag : not supported for now
 */
static int s5k5ccgx_s_crystal_freq(struct v4l2_subdev *sd, u32 freq, u32 flags)
{
	int err = -EINVAL;

	return err;
}

static int s5k5ccgx_g_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
	int err = 0;

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
 * It returns the index (enum s5k5ccgx_frame_size) of the framesize entry.
 */
static int s5k5ccgx_get_framesize_index(struct v4l2_subdev *sd)
{
	struct sensor_state *state = to_state(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct sensor_enum_framesize *frmsize;
	int i = 0, size_index;
	int preview_ratio;

	CAM_ERROR_MSG(&client->dev, "%s: Requested Res: %dx%d\n", __func__, state->pix.width, state->pix.height);

	/* Check for video/image mode */
	for(i = 0; i < (sizeof(s5k5ccgx_framesize_list)/sizeof(struct sensor_enum_framesize)); i++)
	{
		frmsize = (struct sensor_enum_framesize *)&s5k5ccgx_framesize_list[i];

		if(frmsize->op_mode != state->op_mode)
		{
			continue;
		}

		if(state->op_mode == OP_MODE_IMAGE)
		{
			/* In case of image capture mode, if the given image resolution is not supported,
 			 * return the next higher image resolution. */
			if(frmsize->width == state->pix.width && frmsize->height == state->pix.height)
			{
				return frmsize->size_index;
			}
		} 
		else 
		{
			/* In case of video mode, if the given video resolution is not matching, use
 			 * the default rate (currently S5K5CCGX_PREVIEW_VGA).
 			 */		 
			if(frmsize->width == state->pix.width && frmsize->height == state->pix.height)
			{
				return frmsize->size_index;
			}
		}
	} 

    // In case that there is no match in s5k5ccgx_framesize_list.
    if(state->op_mode == OP_MODE_IMAGE)
    {
        size_index = CAPTURE_SIZE_QXGA;
    }
    else
    {
        preview_ratio = state->pix.width * 10 / state->pix.height;
        
    	if(preview_ratio == 17)
    	{
    		size_index = PREVIEW_SIZE_D1;  // Not support PREVIEW_SIZE_HD
    	}
    	else if(preview_ratio == 15)
    	{
    	    size_index = PREVIEW_SIZE_D1;
    	}
    	else if(preview_ratio == 12)
    	{
    	    // In camcorder mode, VGA size will be used instead of XGA to enhance performance.
    	    size_index = (state->sensor_mode == SENSOR_MODE_MOVIE) ? PREVIEW_SIZE_4CIF : PREVIEW_SIZE_XGA;
    	}
    	else // (preview_ratio == 13) and others.
    	{
    	    // In camcorder mode, VGA size will be used instead of XGA to enhance performance.
    	    size_index = (state->sensor_mode == SENSOR_MODE_MOVIE) ? PREVIEW_SIZE_VGA : PREVIEW_SIZE_XGA;
    	}
    }
    
    return size_index;
}

/* This function is called from the s_ctrl api
 * Given the index, it checks if it is a valid index.
 * On success, it returns 0.
 * On Failure, it returns -EINVAL
 */
static int s5k5ccgx_set_framesize_index(struct v4l2_subdev *sd, unsigned int index)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct sensor_state *state = to_state(sd);	
	int i = 0;

	for(i = 0; i < (sizeof(s5k5ccgx_framesize_list)/sizeof(struct sensor_enum_framesize)); i++)
	{
		if(s5k5ccgx_framesize_list[i].size_index == index && s5k5ccgx_framesize_list[i].op_mode == state->op_mode)
		{
			state->framesize_index 	= s5k5ccgx_framesize_list[i].size_index;	
			state->pix.width = s5k5ccgx_framesize_list[i].width;
			state->pix.height = s5k5ccgx_framesize_list[i].height;
			
			CAM_ERROR_MSG(&client->dev, "%s: Camera Res: %dx%d\n", __func__, state->pix.width, state->pix.height);
			return 0;
		} 
	} 
	
	CAM_ERROR_MSG(&client->dev, "%s: not support frame size\n", __func__);
	
	return -EINVAL;
}

/* Information received: 
 * width, height
 * pixel_format -> to be handled in the upper layer 
 *
 * */
static int s5k5ccgx_s_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
	struct sensor_state *state = to_state(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	
	int err = 0;
	int framesize_index = -1;

	if(fmt->fmt.pix.pixelformat == V4L2_PIX_FMT_JPEG && fmt->fmt.pix.colorspace != V4L2_COLORSPACE_JPEG)
	{
		CAM_ERROR_MSG(&client->dev, "%s: mismatch in pixelformat and colorspace\n", __func__);
		return -EINVAL;
	}

	state->pix.width = fmt->fmt.pix.width;
	state->pix.height = fmt->fmt.pix.height;

	state->pix.pixelformat = fmt->fmt.pix.pixelformat;

	if(state->capture_mode == V4L2_MODE_CAPTURE)
	{
		state->op_mode = OP_MODE_IMAGE;
	}
	else
	{
		state->op_mode = OP_MODE_VIDEO; 
	}

	// cmk 2010.09.29 Touch AF Setup
    if(state->op_mode == OP_MODE_VIDEO)
    {
    	state->af_info.preview_width = fmt->fmt.pix.width;
    	state->af_info.preview_height = fmt->fmt.pix.height;
    }
    
	framesize_index = s5k5ccgx_get_framesize_index(sd);

	CAM_INFO_MSG(&client->dev, "%s:framesize_index = %d\n", __func__, framesize_index);
	
	err = s5k5ccgx_set_framesize_index(sd, framesize_index);
	if(err < 0)
	{
		CAM_ERROR_MSG(&client->dev, "%s: set_framesize_index failed\n", __func__);
		return -EINVAL;
	}

	if(state->pix.pixelformat == V4L2_PIX_FMT_JPEG)
	{
		state->jpeg_param.enable = 1;
	} 
	else 
	{
		state->jpeg_param.enable = 0;
	}

	return 0;
}

static int s5k5ccgx_enum_framesizes(struct v4l2_subdev *sd, struct v4l2_frmsizeenum *fsize)
{
	struct sensor_state *state = to_state(sd);
	const struct sensor_enum_framesize *elem;	
	
	int num_entries = sizeof(s5k5ccgx_framesize_list)/sizeof(struct sensor_enum_framesize);	
	int index = 0;
	int i = 0;

	/* The camera interface should read this value, this is the resolution
 	 * at which the sensor would provide framedata to the camera i/f
 	 *
 	 * In case of image capture, this returns the default camera resolution (SVGA)
 	 */
	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;

	if(state->capture_mode == V4L2_MODE_CAPTURE)
	{
		index = CAPTURE_SIZE_QXGA;
	} 
	else 
	{
		index = state->framesize_index;
	}

	for(i = 0; i < num_entries; i++)
	{
		elem = &s5k5ccgx_framesize_list[i];
		
		if(elem->size_index == index)
		{
			fsize->discrete.width = s5k5ccgx_framesize_list[index].width;
			fsize->discrete.height = s5k5ccgx_framesize_list[index].height;
			return 0;
		}
	}

	return -EINVAL;
}

static int s5k5ccgx_enum_frameintervals(struct v4l2_subdev *sd, 
					struct v4l2_frmivalenum *fival)
{
	int err = 0;

	return err;
}

static int s5k5ccgx_enum_fmt(struct v4l2_subdev *sd, struct v4l2_fmtdesc *fmtdesc)
{
	int num_entries;

	num_entries = sizeof(capture_fmts)/sizeof(struct v4l2_fmtdesc);

	if(fmtdesc->index >= num_entries)
	{
		return -EINVAL;
	}

    memset(fmtdesc, 0, sizeof(*fmtdesc));
    memcpy(fmtdesc, &capture_fmts[fmtdesc->index], sizeof(*fmtdesc));

	return 0;
}

static int s5k5ccgx_try_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
	int num_entries = 0;
	int i = 0;

	num_entries = sizeof(capture_fmts)/sizeof(struct v4l2_fmtdesc);

	for(i = 0; i < num_entries; i++)
	{
		if(capture_fmts[i].pixelformat == fmt->fmt.pix.pixelformat)
			return 0;
	} 

	return -EINVAL;
}

/** Gets current FPS value */
static int s5k5ccgx_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct sensor_state *state = to_state(sd);
	
	int err = 0;

	state->strm.parm.capture.timeperframe.numerator = 1;
	state->strm.parm.capture.timeperframe.denominator = state->fps;

	memcpy(param, &state->strm, sizeof(param));

	return err;
}

/** Sets the FPS value */
static int s5k5ccgx_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct sensor_state *state = to_state(sd);

	int err = 0;

    state->capture_mode = param->parm.capture.capturemode;

	if(param->parm.capture.timeperframe.numerator != state->strm.parm.capture.timeperframe.numerator ||
	   param->parm.capture.timeperframe.denominator != state->strm.parm.capture.timeperframe.denominator)
	{
		
		int fps = 0;
		int fps_max = 30;

		if(param->parm.capture.timeperframe.numerator && param->parm.capture.timeperframe.denominator)
		{
			fps = (int)(param->parm.capture.timeperframe.denominator/param->parm.capture.timeperframe.numerator);
		}
		else
		{
			fps = 0;
		}

		if(fps <= 0 || fps > fps_max)
		{
			CAM_ERROR_MSG(&client->dev, "%s: Framerate %d not supported, setting it to %d fps.\n",__func__, fps, fps_max);
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

static int s5k5ccgx_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
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
        
		case V4L2_CID_CAM_JPEG_MAIN_SIZE:
		{
			ctrl->value = state->jpeg_param.main_size;
			break;
		}
		
		case V4L2_CID_CAM_JPEG_MAIN_OFFSET:
		{
			ctrl->value = state->jpeg_param.main_offset;
			break;
        }
        
		case V4L2_CID_CAM_JPEG_THUMB_SIZE:
		{
			ctrl->value = state->jpeg_param.thumb_size;
			break;
		}
		
		case V4L2_CID_CAM_JPEG_THUMB_OFFSET:
		{
			ctrl->value = state->jpeg_param.thumb_offset;
			break;
        }
        
		case V4L2_CID_CAM_JPEG_POSTVIEW_OFFSET:
		{
			ctrl->value = state->jpeg_param.postview_offset;
			break; 
		}
		
		case V4L2_CID_CAM_JPEG_MEMSIZE:
		{
			ctrl->value = SENSOR_JPEG_SNAPSHOT_MEMSIZE;
			break;
        }
        
		case V4L2_CID_CAM_JPEG_QUALITY:
		{
			ctrl->value = state->jpeg_param.quality;
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
			err = s5k5ccgx_get_auto_focus_status(sd, ctrl);
			break;
        }

		case V4L2_CID_CAMERA_AE_STABLE_RESULT:
		{
			err = s5k5ccgx_get_ae_stable_status(sd, ctrl);
			break;
        }

		case V4L2_CID_CAM_DATE_INFO_YEAR:
		{
			ctrl->value = 2010;//state->dateinfo.year;//bestiq 
			break; 
		}
		
		case V4L2_CID_CAM_DATE_INFO_MONTH:
		{
			ctrl->value = 2;//state->dateinfo.month;
			break; 
		}
		
		case V4L2_CID_CAM_DATE_INFO_DATE:
		{
			ctrl->value = 25;//state->dateinfo.date;
			break; 
		}
		
		case V4L2_CID_CAM_SENSOR_VER:
		{
			ctrl->value = state->sensor_version;
			break; 
		}
		
		case V4L2_CID_CAM_FW_MINOR_VER:
		{
			ctrl->value = state->fw_ver.minor;
			break; 
		}
		
		case V4L2_CID_CAM_FW_MAJOR_VER:
		{
			ctrl->value = state->fw_ver.major;
			break; 
		}
		
		case V4L2_CID_CAM_PRM_MINOR_VER:
		{
			break; 
		}
		
		case V4L2_CID_CAM_PRM_MAJOR_VER:
		{
			break; 
        }
        
		case V4L2_CID_CAMERA_FLASH_CHECK:
		{
			ctrl->value = state->camera_flash_fire;
			break;
		}

        case V4L2_CID_CAMERA_POSTVIEW_WIDTH:
        {
            ctrl->value = state->postview_info.width;
            break;
        }

        case V4L2_CID_CAMERA_POSTVIEW_HEIGHT:
        {
            ctrl->value = state->postview_info.height;
            break;
        }

        case V4L2_CID_CAMERA_SENSOR_ID:
        {
            ctrl->value = SYSTEMLSI_S5K5CCGX;
            break;
        }

        case V4L2_CID_CAMERA_GET_ISO:
        {
            ctrl->value = gISOSpeedRating;
            break;
        }

        case V4L2_CID_CAMERA_GET_SHT_TIME:
        {
            ctrl->value = gExposureTime;
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

static int s5k5ccgx_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct sensor_state *state = to_state(sd);
	int err = 0;

	CAM_ERROR_MSG(&client->dev, "%s: V4l2 control ID =%d\n", __func__, ctrl->id - V4L2_CID_PRIVATE_BASE);

	if(state->check_dataline)
	{
        if((ctrl->id != V4L2_CID_CAM_PREVIEW_ONOFF) &&
           (ctrl->id != V4L2_CID_CAMERA_CHECK_DATALINE_STOP) &&
           (ctrl->id != V4L2_CID_CAMERA_CHECK_DATALINE))
        {
            return 0;
        }
	}
    
	mutex_lock(&sensor_s_ctrl);

	switch(ctrl->id) 
	{
		case V4L2_CID_CAMERA_VT_MODE:
		{
			break;
		}
		
		case V4L2_CID_CAMERA_AE_AWB_LOCKUNLOCK:
		{
			err = s5k5ccgx_AE_AWB_lock(sd, ctrl);
			break;
		}
		
		case V4L2_CID_CAMERA_FLASH_MODE:
		{
		    err = s5k5ccgx_set_flash_mode(sd, ctrl);
			break;
		}

		case V4L2_CID_CAMERA_FLASH_CONTROL:
		{
		    err = s5k5ccgx_AAT_flash_control(sd, ctrl->value);
			break;
		}
		
		case V4L2_CID_CAMERA_BRIGHTNESS:
		{
			err = s5k5ccgx_set_ev(sd, ctrl);
			break;
		}
		
		case V4L2_CID_CAMERA_WHITE_BALANCE:
		{
			err = s5k5ccgx_set_white_balance(sd, ctrl);
			break;
		}
		
		case V4L2_CID_CAMERA_EFFECT:
		{
			err = s5k5ccgx_set_effect(sd, ctrl);
			break;
		}
		
		case V4L2_CID_CAMERA_ISO:
		{
			err = s5k5ccgx_set_iso(sd, ctrl);
			break;
		}
		
		case V4L2_CID_CAMERA_METERING:
		{
			err = s5k5ccgx_set_metering(sd, ctrl);
			break;
		}
		
		case V4L2_CID_CAMERA_CONTRAST:
		{
			err = s5k5ccgx_set_contrast(sd, ctrl);
			break;
        }
        
		case V4L2_CID_CAMERA_SATURATION:
		{
			err = s5k5ccgx_set_saturation(sd, ctrl);
			break;
        }
		
		case V4L2_CID_CAMERA_SHARPNESS:
		{
			err = s5k5ccgx_set_sharpness(sd, ctrl);
			break;
        }
        
		/*Camcorder fix fps*/
		case V4L2_CID_CAMERA_SENSOR_MODE:
		{
		    state->sensor_mode = ctrl->value;
		    
			CAM_INFO_MSG(&client->dev, "sensor mode = %d\n", ctrl->value);
			break;
        }
        
		case V4L2_CID_CAMERA_WDR:
		{
			break;
        }
        
		case V4L2_CID_CAMERA_ANTI_SHAKE:
		{
			break;
        }
        
		case V4L2_CID_CAMERA_FACE_DETECTION:
		{
			break;
        }
        
		case V4L2_CID_CAMERA_SMART_AUTO:
		{
			break;
        }
        
		case V4L2_CID_CAMERA_FOCUS_MODE:
		{
			err = s5k5ccgx_set_focus_mode(sd, ctrl);
			break;
        }
        
		// cmk 2010.09.29 Touch AF Setup
		case V4L2_CID_CAMERA_DEFAULT_FOCUS_POSITION:
		{
			err = s5k5ccgx_set_AF_default_position(sd);
			break;
		}
		
		case V4L2_CID_CAMERA_VINTAGE_MODE:
		{
			break;
		}
		
		case V4L2_CID_CAMERA_BEAUTY_SHOT:
		{
			break;
        }
        
		case V4L2_CID_CAMERA_FACEDETECT_LOCKUNLOCK:
		{
			break;		
        }
        
		//need to be modified
		case V4L2_CID_CAM_JPEG_QUALITY:
		{
			if(ctrl->value < 0 || ctrl->value > 100)
			{
				err = -EINVAL;
			} 
			else 
			{
				state->jpeg_param.quality = ctrl->value;
				err = s5k5ccgx_set_jpeg_quality(sd);
			}
			break;
        }
        
		case V4L2_CID_CAMERA_SCENE_MODE:
		{
			err = s5k5ccgx_set_scene_mode(sd, ctrl);
			break;
        }
        
		case V4L2_CID_CAMERA_GPS_LATITUDE:
		{
			CAM_ERROR_MSG(&client->dev, "%s: V4L2_CID_CAMERA_GPS_LATITUDE: not implemented\n", __func__);
			break;
        }
        
		case V4L2_CID_CAMERA_GPS_LONGITUDE:
		{
			CAM_ERROR_MSG(&client->dev, "%s: V4L2_CID_CAMERA_GPS_LONGITUDE: not implemented\n", __func__);
			break;
        }
        
		case V4L2_CID_CAMERA_GPS_TIMESTAMP:
		{
			CAM_ERROR_MSG(&client->dev, "%s: V4L2_CID_CAMERA_GPS_TIMESTAMP: not implemented\n", __func__);
			break;
        }
        
		case V4L2_CID_CAMERA_GPS_ALTITUDE:
		{
			CAM_ERROR_MSG(&client->dev, "%s: V4L2_CID_CAMERA_GPS_ALTITUDE: not implemented\n", __func__);
			break;
        }
        
		case V4L2_CID_CAMERA_ZOOM:
		{
			err = s5k5ccgx_set_dzoom(sd, ctrl);
			break;
        }
        
		case V4L2_CID_CAMERA_TOUCH_AF_START_STOP:
		{
		    err = s5k5ccgx_set_touch_auto_focus(sd, ctrl);
			break;
		}
		
		case V4L2_CID_CAMERA_CAF_START_STOP:
		{
			break;	
        }
        
		case V4L2_CID_CAMERA_OBJECT_POSITION_X:
		{
			state->af_info.x = ctrl->value;
			err = 0;
			break;
        }
        
		case V4L2_CID_CAMERA_OBJECT_POSITION_Y:
		{
			state->af_info.y = ctrl->value;
			err = 0;
			break;
        }
        
		case V4L2_CID_CAMERA_OBJ_TRACKING_START_STOP:
		{
			break;
        }
        
		case V4L2_CID_CAMERA_SET_AUTO_FOCUS:
		{
			err = s5k5ccgx_set_auto_focus(sd, ctrl);
			break;		
        }

        case V4L2_CID_CAMERA_SET_AF_PREFLASH:
        {
            err = s5k5ccgx_set_af_preflash(sd, ctrl);
            break;
        }
        
		case V4L2_CID_CAMERA_FRAME_RATE:
		{
			err = s5k5ccgx_set_frame_rate(sd, ctrl);
			break;
		}
		
		case V4L2_CID_CAMERA_ANTI_BANDING:
		{
			break;
        }
        
		case V4L2_CID_CAM_CAPTURE:
		{
			err = s5k5ccgx_set_capture_start(sd, ctrl);
			break;
		}

		case V4L2_CID_CAM_CAPTURE_DONE:
		{
			err = s5k5ccgx_set_capture_done(sd, ctrl);
			break;
		}
		
		/* Used to start / stop preview operation. 
	 	 * This call can be modified to START/STOP operation, which can be used in image capture also */
		case V4L2_CID_CAM_PREVIEW_ONOFF:
		{
			if(ctrl->value)
			{
				err = s5k5ccgx_set_preview_start(sd);
            }				
			else
			{
				err = s5k5ccgx_set_preview_stop(sd);
            }				
			break;
        }
        
		case V4L2_CID_CAM_UPDATE_FW:
		{
			break;
        }
        
		case V4L2_CID_CAM_SET_FW_ADDR:
		{
			break;
        }
        
		case V4L2_CID_CAM_SET_FW_SIZE:
		{
			break;
        }
        
		case V4L2_CID_CAM_FW_VER:
		{
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
			err = s5k5ccgx_check_dataline_onoff(sd, 0);
			break;	
        }

        case V4L2_CID_CAMERA_SET_SLOW_AE:
        case V4L2_CID_CAMERA_SET_GAMMA:
        case V4L2_CID_CAMERA_BATCH_REFLECTION:
        {
            break;
        }
        
		default:
		{
		    err = -ENOTSUPP;
		    break;
        }
	}

	if(err < 0)
	{
	    CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! control %d failed\n", 
	                                __FILE__, __LINE__, ctrl->id - V4L2_CID_PRIVATE_BASE);
	}

	mutex_unlock(&sensor_s_ctrl);
	
	return err;
}

static int s5k5ccgx_check_dataline_onoff(struct v4l2_subdev *sd, int onoff)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct sensor_state *state = to_state(sd);
	int err = 0;

    if(onoff)
    {
        // data line on
		CAM_INFO_MSG(&client->dev, "pattern on setting~~~~~~~~~~~~~~\n");	
		
    	err = s5k5ccgx_i2c_set_config_register(client, 
    	                                  pattern_on_regs, 
    	                                  PATTERN_ON_NUM_OF_REGS, 
    	                                  "pattern_on_regs");        
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
		
    	err = s5k5ccgx_i2c_set_config_register(client, 
    	                                  pattern_off_regs, 
    	                                  PATTERN_OFF_NUM_OF_REGS, 
    	                                  "pattern_off_regs");
        
    	if(err < 0)
    	{
    		CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! Pattern off failed\n", __FILE__, __LINE__);	
    		return -EIO;
    	}

	    state->check_dataline = CHK_DATALINE_OFF;
    }

	msleep(100);
	
	CAM_INFO_MSG(&client->dev, "pattern on setting done~~~~~~~~~~~~~~\n");	
	
	return err;
}

static int s5k5ccgx_init(struct v4l2_subdev *sd, u32 val)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = -EINVAL;

	CAM_INFO_MSG(&client->dev, "%s ~~~~~~~~~~~~~~\n", __func__);
	
	s5k5ccgx_init_parameters(sd);

#ifdef CONFIG_LOAD_FILE
	err = s5k5ccgx_regs_table_init();
	if(err < 0) 
	{
		CAM_ERROR_MSG(&client->dev, "%s: s5k5ccgx_regs_table_init failed\n", __func__);
		return -ENOIOCTLCMD;
	}
#endif // CONFIG_LOAD_FILE

	err = s5k5ccgx_i2c_set_config_register(client,
	                                  init_regs,
	                                  INIT_NUM_OF_REGS,
	                                  "init_regs");
	if(err < 0)
	{
		CAM_ERROR_MSG(&client->dev, "[%s: %d] ERROR! Sensor Init Failed\n", __FILE__, __LINE__);
		return -ENOIOCTLCMD;
	}		

	return 0;
}

/**************************************************************************
 * s5k5ccgx_s_config
 * With camera device, we need to re-initialize every single opening time therefor,
 * it is not necessary to be initialized on probe time. except for version checking
 * NOTE: version checking is optional
 ***************************************************************************/
static int s5k5ccgx_s_config(struct v4l2_subdev *sd, int irq, void *platform_data)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct sensor_state *state = to_state(sd);
	struct camera_platform_data *pdata;

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

	if(!pdata->mclk_freq)
	{
		state->mclk_freq = DEFUALT_MCLK;
	}
	else
	{
		state->mclk_freq = pdata->mclk_freq;
	}

	return 0;
}

/**************************************************************************
 * DRIVER REGISTRATION FACTORS
 ***************************************************************************/
static const struct v4l2_subdev_core_ops s5k5ccgx_core_ops = 
{
    .init       = s5k5ccgx_init,	    /* initializing API */
    .s_config   = s5k5ccgx_s_config,	/* Fetch platform data */
    .queryctrl  = s5k5ccgx_queryctrl,
    .querymenu  = s5k5ccgx_querymenu,
    .g_ctrl     = s5k5ccgx_g_ctrl,
    .s_ctrl     = s5k5ccgx_s_ctrl,
};

static const struct v4l2_subdev_video_ops s5k5ccgx_video_ops = 
{
    .s_crystal_freq     = s5k5ccgx_s_crystal_freq,
    .g_fmt              = s5k5ccgx_g_fmt,
    .s_fmt              = s5k5ccgx_s_fmt,
    .enum_framesizes    = s5k5ccgx_enum_framesizes,
    .enum_frameintervals = s5k5ccgx_enum_frameintervals,
    .enum_fmt           = s5k5ccgx_enum_fmt,
    .try_fmt            = s5k5ccgx_try_fmt,
    .g_parm             = s5k5ccgx_g_parm,
    .s_parm             = s5k5ccgx_s_parm,
};

static const struct v4l2_subdev_ops s5k5ccgx_ops = 
{
	.core = &s5k5ccgx_core_ops,
	.video = &s5k5ccgx_video_ops,
};

/**************************************************************************
 * s5k5ccgx_probe
 * Fetching platform data is being done with s_config sd call.
 * In probe routine, we just register sd device
 ***************************************************************************/
static int s5k5ccgx_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct sensor_state *state;
	struct v4l2_subdev *sd;

	CAM_INFO_MSG(&client->dev, "s5k5ccgx_probe......................................... \n");
	
	state = kzalloc(sizeof(struct sensor_state), GFP_KERNEL);
	if (state == NULL)
	{
		return -ENOMEM;
	}

	state->runmode = RUNNING_MODE_NOTREADY;

	sd = &state->sd;
	strcpy(sd->name, S5K5CCGX_DRIVER_NAME);

	/* Registering sd */
	v4l2_i2c_subdev_init(sd, client, &s5k5ccgx_ops);

	CAM_ERROR_MSG(&client->dev, "3MP camera S5K5CCGX loaded.\n");

    // FLASH
    gpio_request(GPIO_CAM_FLASH_SET, "GPJ0[2]");
    gpio_request(GPIO_CAM_FLASH_EN, "GPJ0[7]");

    gpio_direction_output(GPIO_CAM_FLASH_SET, 0);
    gpio_direction_output(GPIO_CAM_FLASH_EN, 0);
	
	return 0;
}

/**************************************************************************
 * s5k5ccgx_remove
 ***************************************************************************/
static int s5k5ccgx_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);

	CAM_INFO_MSG(&client->dev, "s5k5ccgx_remove.......................................... \n");

#ifdef CONFIG_LOAD_FILE
	s5k5ccgx_regs_table_exit();
#endif // CONFIG_LOAD_FILE

    // FLASH
    s5k5ccgx_AAT_flash_control(sd, FLASH_CONTROL_OFF);
    
	gpio_free(GPIO_CAM_FLASH_SET);
	gpio_free(GPIO_CAM_FLASH_EN);

	v4l2_device_unregister_subdev(sd);
	kfree(to_state(sd));

	CAM_ERROR_MSG(&client->dev, "Unloaded camera sensor S5K5CCGX.\n");

	return 0;
}

static const struct i2c_device_id s5k5ccgx_id[] = 
{
	{ S5K5CCGX_DRIVER_NAME, 0 },
	{ }
};

static struct v4l2_i2c_driver_data v4l2_i2c_data = 
{
	.name = S5K5CCGX_DRIVER_NAME,
	.probe = s5k5ccgx_probe,
	.remove = s5k5ccgx_remove,
	.id_table = s5k5ccgx_id,
};

MODULE_DEVICE_TABLE(i2c, s5k5ccgx_id);
MODULE_DESCRIPTION("SYSTEM LSI S5K5CCGX 3MP camera driver");
MODULE_AUTHOR("Minkeun Cho <mk76.cho@samsung.com>");
MODULE_LICENSE("GPL");

