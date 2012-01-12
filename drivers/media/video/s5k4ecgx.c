/* drivers/media/video/s5k4ecgx.c
 *
 * Driver for s5k4ecgx (5MP Camera) from SEC
 *
 * Copyright (C) 2010, SAMSUNG ELECTRONICS
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
#include <linux/completion.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-i2c-drv.h>
#include <media/s5k4ecgx_platform.h>
#include <linux/videodev2_samsung.h>

#if 0/* For debug */
#ifdef dev_dbg(dev, format, arg...)
#undef dev_dbg(dev, format, arg...)
#endif
#define dev_dbg(dev, format, arg...)   dev_printk(KERN_DEBUG , dev , format , ## arg)
#endif

#define NEW_CAMERA /* For alternative method */
#define MAX_ZOOMSTEP 8

#ifdef CONFIG_VIDEO_S5K4ECGX_V_1_0
#include "s5k4ecgx_regs_1_0.h"
#define S5K4ECGX_VERSION_1_0	0x00
#endif

#ifdef CONFIG_VIDEO_S5K4ECGX_V_1_1
#include "s5k4ecgx_regs_1_1.h"
#define S5K4ECGX_VERSION_1_1	0x11
#endif
//#define CONFIG_LOAD_FILE	//For tunning binary

#define FORMAT_FLAGS_COMPRESSED		0x3
#define SENSOR_JPEG_SNAPSHOT_MEMSIZE	0x410580

#define DEFAULT_PIX_FMT		V4L2_PIX_FMT_UYVY	/* YUV422 */
#define DEFAULT_MCLK		24000000
#define POLL_TIME_MS		10
#define CAPTURE_POLL_TIME_MS    1500

static const u16 Zoom_X2[MAX_ZOOMSTEP+1] = {256, 288, 320, 352, 384, 416, 448, 480, 512};
static const u16 Zoom_X1_77[MAX_ZOOMSTEP+1] = {256, 280, 304, 328, 352, 376, 400, 424, 453};
static const u16 Zoom_X1_6[MAX_ZOOMSTEP+1] = {256, 275, 294, 313, 332, 351, 370, 389, 410};
static const u16 Zoom_X1_25[MAX_ZOOMSTEP+1] = {256, 264, 272, 280, 288, 296, 304, 312, 320};
static u16 cnZoomDefaultValue = 256;

#ifdef NEW_CAMERA
static int camcorder_check_flag;
#endif

#ifdef NEW_CAMERA
/* maximum time for one frame at minimum fps (10fps) in normal mode */
#define NORMAL_MODE_MAX_ONE_FRAME_DELAY_MS    100
/* maximum time for one frame at minimum fps (4fps) in night mode */
#define NIGHT_MODE_MAX_ONE_FRAME_DELAY_MS     250
#else
/* maximum time for one frame at minimum fps (15fps) in normal mode */
#define NORMAL_MODE_MAX_ONE_FRAME_DELAY_MS     67
/* maximum time for one frame at minimum fps (4fps) in night mode */
#define NIGHT_MODE_MAX_ONE_FRAME_DELAY_MS     250
#endif

/* time to move lens to target position before last af mode register write */
#define LENS_MOVE_TIME_MS       100

/* level at or below which we need to enable flash when in auto mode */
#define LOW_LIGHT_LEVEL		0x94

/* level at or below which we need to use low light capture mode */
#define HIGH_LIGHT_LEVEL	0x50//0x80

#define FIRST_AF_SEARCH_COUNT   80
#define SECOND_AF_SEARCH_COUNT  80
#define AE_STABLE_SEARCH_COUNT  4

#define FIRST_SETTING_FOCUS_MODE_DELAY_MS	100
#define SECOND_SETTING_FOCUS_MODE_DELAY_MS   	 200

#ifdef CONFIG_VIDEO_S5K4ECGX_DEBUG
enum {
	S5K4ECGX_DEBUG_I2C		= 1U << 0,
	S5K4ECGX_DEBUG_I2C_BURSTS	= 1U << 1,
};

static uint32_t s5k4ecgx_debug_mask = S5K4ECGX_DEBUG_I2C_BURSTS;
module_param_named(debug_mask, s5k4ecgx_debug_mask, uint, S_IWUSR | S_IRUGO);

#define s5k4ecgx_debug(mask, x...) \
	do { \
		if (s5k4ecgx_debug_mask & mask) \
			pr_info(x);	\
	} while (0)
#else

#define s5k4ecgx_debug(mask, x...)

#endif

#define S5K4ECGX_VERSION_1_1	0x11

/* result values returned to HAL */
enum {
	AUTO_FOCUS_FAILED,
	AUTO_FOCUS_DONE,
	AUTO_FOCUS_CANCELLED,
};

enum af_operation_status {
	AF_NONE = 0,
	AF_START,
	AF_CANCEL,
};

enum s5k4ecgx_oprmode {
	S5K4ECGX_OPRMODE_VIDEO = 0,
	S5K4ECGX_OPRMODE_IMAGE = 1,
};

enum s5k4ecgx_preview_frame_size {
	S5K4ECGX_PREVIEW_QCIF = 0,	/* 176x144 */
	S5K4ECGX_PREVIEW_QVGA,		/* 320x240 */
	S5K4ECGX_PREVIEW_CIF,		/* 352x288 */
	S5K4ECGX_PREVIEW_528,           /* 528x432 */
	S5K4ECGX_PREVIEW_VGA,		/* 640x480 */
	S5K4ECGX_PREVIEW_D1,		/* 720x480 */
	S5K4ECGX_PREVIEW_WVGA,		/* 800x480 */
	S5K4ECGX_PREVIEW_SVGA,		/* 800x600 */
	S5K4ECGX_PREVIEW_WSVGA,		/* 1024x600*/
	S5K4ECGX_PREVIEW_720P,		/* 1280x720*/
	S5K4ECGX_PREVIEW_MAX,
};

#ifdef NEW_CAMERA
enum s5k4ecgx_camcorder_preview_frame_size {
	S5K4ECGX_CAMCORDER_PREVIEW_QCIF = 0,	/* 176x144 */
	S5K4ECGX_CAMCORDER_PREVIEW_QVGA,		/* 320x240 */
	S5K4ECGX_CAMCORDER_PREVIEW_CIF,		/* 352x288 */
	S5K4ECGX_CAMCORDER_PREVIEW_528,		/* 528x432 */			
	S5K4ECGX_CAMCORDER_PREVIEW_VGA,		/* 640x480 */
	S5K4ECGX_CAMCORDER_PREVIEW_D1,		/* 720x480 */
	S5K4ECGX_CAMCORDER_PREVIEW_WVGA,		/* 800x480 */
	S5K4ECGX_CAMCORDER_PREVIEW_SVGA,		/* 800x600 */
	S5K4ECGX_CAMCORDER_PREVIEW_WSVGA,		/* 1024x600*/
	S5K4ECGX_CAMCORDER_PREVIEW_720P,		/* 1280x720*/
	S5K4ECGX_CAMCORDER_PREVIEW_MAX,
};
#endif

enum s5k4ecgx_capture_frame_size {
	S5K4ECGX_CAPTURE_VGA = 0,	/* 640x480 */
	S5K4ECGX_CAPTURE_WVGA,		/* 800x480 */
	S5K4ECGX_CAPTURE_SVGA,		/* 800x600 */
	S5K4ECGX_CAPTURE_WSVGA,		/* 1024x600 */
	S5K4ECGX_CAPTURE_1MP,		/* 1280x960 */
	S5K4ECGX_CAPTURE_W1MP,		/* 1600x960 */
	S5K4ECGX_CAPTURE_2MP,		/* UXGA  - 1600x1200 */
	S5K4ECGX_CAPTURE_W2MP,		/* 35mm Academy Offset Standard 1.66 */
					/* 2048x1232, 2.4MP */
	S5K4ECGX_CAPTURE_3MP,		/* QXGA  - 2048x1536 */
	S5K4ECGX_CAPTURE_W4MP,		/* WQXGA - 2560x1536 */
	S5K4ECGX_CAPTURE_5MP,		/* 2560x1920 */
	S5K4ECGX_CAPTURE_MAX,
};

struct s5k4ecgx_framesize {
	u32 index;
	u32 width;
	u32 height;
};

static const struct s5k4ecgx_framesize s5k4ecgx_preview_framesize_list[] = {
	{ S5K4ECGX_PREVIEW_QCIF,	176,  144 },
	{ S5K4ECGX_PREVIEW_QVGA,	320,  240 },
	{ S5K4ECGX_PREVIEW_CIF,		352,  288 },
	{ S5K4ECGX_PREVIEW_528,		528,  432 },
	{ S5K4ECGX_PREVIEW_VGA,		640,  480 },
	{ S5K4ECGX_PREVIEW_D1,		720,  480 },
	{ S5K4ECGX_PREVIEW_WVGA,	800,  480 },
	{ S5K4ECGX_PREVIEW_720P,	1280, 720 },
};

#ifdef NEW_CAMERA
static const struct s5k4ecgx_framesize s5k4ecgx_camcorder_preview_framesize_list[] = {
	{ S5K4ECGX_CAMCORDER_PREVIEW_QCIF,	176,  144 },
	{ S5K4ECGX_CAMCORDER_PREVIEW_QVGA,	320,  240 },
	{ S5K4ECGX_CAMCORDER_PREVIEW_CIF,	352,  288 },
	{ S5K4ECGX_CAMCORDER_PREVIEW_528,	528,  432 },		
	{ S5K4ECGX_CAMCORDER_PREVIEW_VGA,	640,  480 },
	{ S5K4ECGX_CAMCORDER_PREVIEW_D1,	720,  480 },
	{ S5K4ECGX_CAMCORDER_PREVIEW_WVGA,	800,  480 },
	{ S5K4ECGX_CAMCORDER_PREVIEW_720P,	1280, 720 },

	};
#endif

static const struct s5k4ecgx_framesize s5k4ecgx_capture_framesize_list[] = {
	{ S5K4ECGX_CAPTURE_VGA,		640,  480 },
	{ S5K4ECGX_CAPTURE_WVGA,	800,  480 },
	{ S5K4ECGX_CAPTURE_SVGA,	800,  600 },
	{ S5K4ECGX_CAPTURE_WSVGA,	1024, 600 },
	{ S5K4ECGX_CAPTURE_1MP,		1280, 960 },
	{ S5K4ECGX_CAPTURE_W1MP,	1600, 960 },
	{ S5K4ECGX_CAPTURE_2MP,		1600, 1200 },
	{ S5K4ECGX_CAPTURE_W2MP,	2048, 1232 },
	{ S5K4ECGX_CAPTURE_3MP,		2048, 1536 },
	{ S5K4ECGX_CAPTURE_W4MP,	2560, 1536 },
	{ S5K4ECGX_CAPTURE_5MP,		2560, 1920 },
};

struct s5k4ecgx_version {
	u32 major;
	u32 minor;
};

struct s5k4ecgx_date_info {
	u32 year;
	u32 month;
	u32 date;
};

enum s5k4ecgx_runmode {
	S5K4ECGX_RUNMODE_NOTREADY,
	S5K4ECGX_RUNMODE_IDLE,
	S5K4ECGX_RUNMODE_RUNNING,
	S5K4ECGX_RUNMODE_CAPTURE,
	S5K4ECGX_RUNMODE_ERROR,
};

struct s5k4ecgx_firmware {
	u32 addr;
	u32 size;
};

struct s5k4ecgx_jpeg_param {
	u32 enable;
	u32 quality;
	u32 main_size;		/* Main JPEG file size */
	u32 thumb_size;		/* Thumbnail file size */
	u32 main_offset;
	u32 thumb_offset;
	u32 postview_offset;
};

struct s5k4ecgx_position {
	int x;
	int y;
};

struct gps_info_common {
	u32 direction;
	u32 dgree;
	u32 minute;
	u32 second;
};

struct s5k4ecgx_gps_info {
	unsigned char gps_buf[8];
	unsigned char altitude_buf[4];
	int gps_timeStamp;
};

struct s5k4ecgx_regset {
	u32 size;
	u8 *data;
};

#ifdef CONFIG_LOAD_FILE
struct s5k4ecgx_regset_table {
	const u32	*reg;
	int		array_size;
	char		*name;
};

#define S5K4ECGX_REGSET(x, y)		\
	[(x)] = {					\
		.reg		= (y),			\
		.array_size	= ARRAY_SIZE((y)),	\
		.name		= #y,			\
}

#define S5K4ECGX_REGSET_TABLE(y)		\
	{					\
		.reg		= (y),			\
		.array_size	= ARRAY_SIZE((y)),	\
		.name		= #y,			\
}
#else
struct s5k4ecgx_regset_table {
	const u32	*reg;
	int		array_size;
};

#define S5K4ECGX_REGSET(x, y)		\
	[(x)] = {					\
		.reg		= (y),			\
		.array_size	= ARRAY_SIZE((y)),	\
}

#define S5K4ECGX_REGSET_TABLE(y)		\
	{					\
		.reg		= (y),			\
		.array_size	= ARRAY_SIZE((y)),	\
}
#endif

struct s5k4ecgx_regs {
	struct s5k4ecgx_regset_table ev[EV_MAX];
	struct s5k4ecgx_regset_table metering[METERING_MAX];
	struct s5k4ecgx_regset_table iso[ISO_MAX];
	struct s5k4ecgx_regset_table effect[IMAGE_EFFECT_MAX];
	struct s5k4ecgx_regset_table white_balance[WHITE_BALANCE_MAX];
	struct s5k4ecgx_regset_table preview_size[S5K4ECGX_PREVIEW_MAX];
#ifdef NEW_CAMERA
	struct s5k4ecgx_regset_table camcorder_preview_size[S5K4ECGX_CAMCORDER_PREVIEW_MAX];
#endif
	struct s5k4ecgx_regset_table capture_size[S5K4ECGX_CAPTURE_MAX];
	struct s5k4ecgx_regset_table scene_mode[SCENE_MODE_MAX];
	struct s5k4ecgx_regset_table saturation[SATURATION_MAX];
	struct s5k4ecgx_regset_table contrast[CONTRAST_MAX];
	struct s5k4ecgx_regset_table sharpness[SHARPNESS_MAX];
	struct s5k4ecgx_regset_table fps[FRAME_RATE_MAX];
	struct s5k4ecgx_regset_table preview_return;
	struct s5k4ecgx_regset_table night_preview_return;
	struct s5k4ecgx_regset_table beach_fireworks_preview_return;
	struct s5k4ecgx_regset_table jpeg_quality_high;
	struct s5k4ecgx_regset_table jpeg_quality_normal;
	struct s5k4ecgx_regset_table jpeg_quality_low;
	struct s5k4ecgx_regset_table flash_start;
	struct s5k4ecgx_regset_table flash_end;
	struct s5k4ecgx_regset_table af_assist_flash_start;
	struct s5k4ecgx_regset_table af_assist_flash_end;
	struct s5k4ecgx_regset_table af_low_light_mode_on;
	struct s5k4ecgx_regset_table af_low_light_mode_off;
#ifndef NEW_CAMERA
	struct s5k4ecgx_regset_table ae_awb_lock_on;
	struct s5k4ecgx_regset_table ae_awb_lock_off;
#else
	struct s5k4ecgx_regset_table ae_lock;
	struct s5k4ecgx_regset_table ae_unlock;
	struct s5k4ecgx_regset_table awb_lock;
	struct s5k4ecgx_regset_table awb_unlock;
#endif
	struct s5k4ecgx_regset_table low_cap_on;
	struct s5k4ecgx_regset_table low_cap_off;
	struct s5k4ecgx_regset_table night_cap;
	struct s5k4ecgx_regset_table beach_snow_fire_cap;
	struct s5k4ecgx_regset_table wdr_on;
	struct s5k4ecgx_regset_table wdr_off;
	struct s5k4ecgx_regset_table face_detection_on;
	struct s5k4ecgx_regset_table face_detection_off;
	struct s5k4ecgx_regset_table capture_start;
	struct s5k4ecgx_regset_table af_macro_mode_1;
	struct s5k4ecgx_regset_table af_macro_mode_2;
	struct s5k4ecgx_regset_table af_macro_mode_3;
	struct s5k4ecgx_regset_table af_low_light_macro_1;
	struct s5k4ecgx_regset_table af_low_light_macro_2;
	struct s5k4ecgx_regset_table af_low_light_macro_3;
	struct s5k4ecgx_regset_table af_normal_mode_1;
	struct s5k4ecgx_regset_table af_normal_mode_2;
	struct s5k4ecgx_regset_table af_normal_mode_3;
#ifdef NEW_CAMERA
	struct s5k4ecgx_regset_table af_normal_mode_4;
#endif
	struct s5k4ecgx_regset_table af_low_light_normal_1;
	struct s5k4ecgx_regset_table af_low_light_normal_2;
	struct s5k4ecgx_regset_table af_low_light_normal_3;
	struct s5k4ecgx_regset_table af_return_infinite_position;
	struct s5k4ecgx_regset_table af_return_macro_position;
	struct s5k4ecgx_regset_table single_af_start;
	struct s5k4ecgx_regset_table single_af_off_1;
	struct s5k4ecgx_regset_table single_af_off_2;
	struct s5k4ecgx_regset_table dtp_start;
	struct s5k4ecgx_regset_table dtp_stop;
	struct s5k4ecgx_regset_table init_reg_1;
	struct s5k4ecgx_regset_table init_reg_2;
#ifdef NEW_CAMERA
	struct s5k4ecgx_regset_table init_reg_1_qik;
	struct s5k4ecgx_regset_table init_reg_2_qik;
	struct s5k4ecgx_regset_table s5k4ecgx_640_preview_qik;
#endif
	struct s5k4ecgx_regset_table flash_init;
	struct s5k4ecgx_regset_table reset_crop;
	struct s5k4ecgx_regset_table get_ae_stable_status;
	struct s5k4ecgx_regset_table get_light_level;
	struct s5k4ecgx_regset_table get_1st_af_search_status;
	struct s5k4ecgx_regset_table get_2nd_af_search_status;
	struct s5k4ecgx_regset_table get_capture_status;
	struct s5k4ecgx_regset_table get_esd_status;
	struct s5k4ecgx_regset_table get_iso;
	struct s5k4ecgx_regset_table get_shutterspeed;
	struct s5k4ecgx_regset_table set_fix_fps;
	struct s5k4ecgx_regset_table set_15_fps;
	struct s5k4ecgx_regset_table set_auto_fps;
	struct s5k4ecgx_regset_table set_auto_contrast_on;
	struct s5k4ecgx_regset_table set_auto_contrast_off;  
       struct s5k4ecgx_regset_table set_af_window_center;
       struct s5k4ecgx_regset_table set_softlanding;
	struct s5k4ecgx_regset_table set_x1_25_zoom[ZOOM_LEVEL_MAX];
	struct s5k4ecgx_regset_table set_x1_6_zoom[ZOOM_LEVEL_MAX];
	struct s5k4ecgx_regset_table set_x1_77_zoom[ZOOM_LEVEL_MAX];
	struct s5k4ecgx_regset_table set_x2_zoom[ZOOM_LEVEL_MAX];
	struct s5k4ecgx_regset_table set_x4_zoom[ZOOM_LEVEL_MAX];
};

#ifdef CONFIG_VIDEO_S5K4ECGX_V_1_0
static const struct s5k4ecgx_regs regs_for_fw_version_1_0 = {
	.ev = {
		S5K4ECGX_REGSET(EV_MINUS_4, s5k4ecgx_EV_Minus_4_v1),
		S5K4ECGX_REGSET(EV_MINUS_3, s5k4ecgx_EV_Minus_3_v1),
		S5K4ECGX_REGSET(EV_MINUS_2, s5k4ecgx_EV_Minus_2_v1),
		S5K4ECGX_REGSET(EV_MINUS_1, s5k4ecgx_EV_Minus_1_v1),
		S5K4ECGX_REGSET(EV_DEFAULT, s5k4ecgx_EV_Default_v1),
		S5K4ECGX_REGSET(EV_PLUS_1, s5k4ecgx_EV_Plus_1_v1),
		S5K4ECGX_REGSET(EV_PLUS_2, s5k4ecgx_EV_Plus_2_v1),
		S5K4ECGX_REGSET(EV_PLUS_3, s5k4ecgx_EV_Plus_3_v1),
		S5K4ECGX_REGSET(EV_PLUS_4, s5k4ecgx_EV_Plus_4_v1),
	},
	.metering = {
		S5K4ECGX_REGSET(METERING_MATRIX, s5k4ecgx_Metering_Matrix_v1),
		S5K4ECGX_REGSET(METERING_CENTER, s5k4ecgx_Metering_Center_v1),
		S5K4ECGX_REGSET(METERING_SPOT, s5k4ecgx_Metering_Spot_v1),
	},
	.iso = {
		S5K4ECGX_REGSET(ISO_AUTO, s5k4ecgx_ISO_Auto_v1),
		S5K4ECGX_REGSET(ISO_50, s5k4ecgx_ISO_100_v1),     /* use 100 */
		S5K4ECGX_REGSET(ISO_100, s5k4ecgx_ISO_100_v1),
		S5K4ECGX_REGSET(ISO_200, s5k4ecgx_ISO_200_v1),
		S5K4ECGX_REGSET(ISO_400, s5k4ecgx_ISO_400_v1),
		S5K4ECGX_REGSET(ISO_800, s5k4ecgx_ISO_400_v1),    /* use 400 */
		S5K4ECGX_REGSET(ISO_1600, s5k4ecgx_ISO_400_v1),   /* use 400 */
		S5K4ECGX_REGSET(ISO_SPORTS, s5k4ecgx_ISO_Auto_v1),/* use auto */
		S5K4ECGX_REGSET(ISO_NIGHT, s5k4ecgx_ISO_Auto_v1), /* use auto */
		S5K4ECGX_REGSET(ISO_MOVIE, s5k4ecgx_ISO_Auto_v1), /* use auto */
	},
	.effect = {
		S5K4ECGX_REGSET(IMAGE_EFFECT_NONE, s5k4ecgx_Effect_Normal_v1),
		S5K4ECGX_REGSET(IMAGE_EFFECT_BNW,
				s5k4ecgx_Effect_Black_White_v1),
		S5K4ECGX_REGSET(IMAGE_EFFECT_SEPIA, s5k4ecgx_Effect_Sepia_v1),
		S5K4ECGX_REGSET(IMAGE_EFFECT_NEGATIVE,
				s5k4ecgx_Effect_Negative_v1),
	},
	.white_balance = {
		S5K4ECGX_REGSET(WHITE_BALANCE_AUTO, s5k4ecgx_WB_Auto_v1),
		S5K4ECGX_REGSET(WHITE_BALANCE_SUNNY, s5k4ecgx_WB_Sunny_v1),
		S5K4ECGX_REGSET(WHITE_BALANCE_CLOUDY, s5k4ecgx_WB_Cloudy_v1),
		S5K4ECGX_REGSET(WHITE_BALANCE_TUNGSTEN,
				s5k4ecgx_WB_Tungsten_v1),
		S5K4ECGX_REGSET(WHITE_BALANCE_FLUORESCENT,
				s5k4ecgx_WB_Fluorescent_v1),
	},
	.preview_size = {
		S5K4ECGX_REGSET(S5K4ECGX_PREVIEW_QCIF, s5k4ecgx_176_Preview_v1),
		S5K4ECGX_REGSET(S5K4ECGX_PREVIEW_CIF, s5k4ecgx_352_Preview_v1),
		S5K4ECGX_REGSET(S5K4ECGX_PREVIEW_VGA, s5k4ecgx_640_Preview_v1),
		S5K4ECGX_REGSET(S5K4ECGX_PREVIEW_D1, s5k4ecgx_720_Preview_v1),
	},
	.capture_size = {
		S5K4ECGX_REGSET(S5K4ECGX_CAPTURE_VGA, s5k4ecgx_VGA_Capture_v1),
		S5K4ECGX_REGSET(S5K4ECGX_CAPTURE_1MP, s5k4ecgx_1M_Capture_v1),
		S5K4ECGX_REGSET(S5K4ECGX_CAPTURE_2MP, s5k4ecgx_2M_Capture_v1),
		S5K4ECGX_REGSET(S5K4ECGX_CAPTURE_3MP, s5k4ecgx_3M_Capture_v1),
		S5K4ECGX_REGSET(S5K4ECGX_CAPTURE_5MP, s5k4ecgx_5M_Capture_v1),
	},
	.scene_mode = {
		S5K4ECGX_REGSET(SCENE_MODE_NONE, s5k4ecgx_Scene_Default_v1),
		S5K4ECGX_REGSET(SCENE_MODE_PORTRAIT,
				s5k4ecgx_Scene_Portrait_v1),
		S5K4ECGX_REGSET(SCENE_MODE_NIGHTSHOT,
				s5k4ecgx_Scene_Nightshot_v1),
		S5K4ECGX_REGSET(SCENE_MODE_LANDSCAPE,
				s5k4ecgx_Scene_Landscape_v1),
		S5K4ECGX_REGSET(SCENE_MODE_SPORTS, s5k4ecgx_Scene_Sports_v1),
		S5K4ECGX_REGSET(SCENE_MODE_PARTY_INDOOR,
				s5k4ecgx_Scene_Party_Indoor_v1),
		S5K4ECGX_REGSET(SCENE_MODE_BEACH_SNOW,
				s5k4ecgx_Scene_Beach_Snow_v1),
		S5K4ECGX_REGSET(SCENE_MODE_SUNSET, s5k4ecgx_Scene_Sunset_v1),
		S5K4ECGX_REGSET(SCENE_MODE_FIREWORKS,
				s5k4ecgx_Scene_Fireworks_v1),
		S5K4ECGX_REGSET(SCENE_MODE_CANDLE_LIGHT,
				s5k4ecgx_Scene_Candle_Light_v1),
	},
	.saturation = {
		S5K4ECGX_REGSET(SATURATION_MINUS_2,
				s5k4ecgx_Saturation_Minus_2_v1),
		S5K4ECGX_REGSET(SATURATION_MINUS_1,
				s5k4ecgx_Saturation_Minus_1_v1),
		S5K4ECGX_REGSET(SATURATION_DEFAULT,
				s5k4ecgx_Saturation_Default_v1),
		S5K4ECGX_REGSET(SATURATION_PLUS_1,
				s5k4ecgx_Saturation_Plus_1_v1),
		S5K4ECGX_REGSET(SATURATION_PLUS_2,
				s5k4ecgx_Saturation_Plus_2_v1),
	},
	.contrast = {
		S5K4ECGX_REGSET(CONTRAST_MINUS_2, s5k4ecgx_Contrast_Minus_2_v1),
		S5K4ECGX_REGSET(CONTRAST_MINUS_1, s5k4ecgx_Contrast_Minus_1_v1),
		S5K4ECGX_REGSET(CONTRAST_DEFAULT, s5k4ecgx_Contrast_Default_v1),
		S5K4ECGX_REGSET(CONTRAST_PLUS_1, s5k4ecgx_Contrast_Plus_1_v1),
		S5K4ECGX_REGSET(CONTRAST_PLUS_2, s5k4ecgx_Contrast_Plus_2_v1),
	},
	.sharpness = {
		S5K4ECGX_REGSET(SHARPNESS_MINUS_2,
				s5k4ecgx_Sharpness_Minus_2_v1),
		S5K4ECGX_REGSET(SHARPNESS_MINUS_1,
				s5k4ecgx_Sharpness_Minus_1_v1),
		S5K4ECGX_REGSET(SHARPNESS_DEFAULT,
				s5k4ecgx_Sharpness_Default_v1),
		S5K4ECGX_REGSET(SHARPNESS_PLUS_1, s5k4ecgx_Sharpness_Plus_1_v1),
		S5K4ECGX_REGSET(SHARPNESS_PLUS_2, s5k4ecgx_Sharpness_Plus_2_v1),
	},
	.preview_return = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Preview_Return_v1),
	.jpeg_quality_high =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_Jpeg_Quality_High_v1),
	.jpeg_quality_normal =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_Jpeg_Quality_Normal_v1),
	.jpeg_quality_low = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Jpeg_Quality_Low_v1),
	.flash_start = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Flash_Start_v1),
	.flash_end = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Flash_End_v1),
	.af_assist_flash_start =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_Pre_Flash_Start_v1),
	.af_assist_flash_end =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_Pre_Flash_End_v1),
	.af_low_light_mode_on =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Low_Light_Mode_On_v1),
	.af_low_light_mode_off =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Low_Light_Mode_Off_v1),
	.ae_awb_lock_on =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_AE_AWB_Lock_On_v1),
	.ae_awb_lock_off =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_AE_AWB_Lock_Off_v1),
	.low_cap_on = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Low_Cap_On_v1),
	.low_cap_off = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Low_Cap_Off_v1),
	.wdr_on = S5K4ECGX_REGSET_TABLE(s5k4ecgx_WDR_on_v1),
	.wdr_off = S5K4ECGX_REGSET_TABLE(s5k4ecgx_WDR_off_v1),
	.face_detection_on =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_Face_Detection_On_v1),
	.face_detection_off =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_Face_Detection_Off_v1),
	.capture_start = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Capture_Start_v1),
	.af_macro_mode_1 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Macro_mode_1_v1),
	.af_macro_mode_2 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Macro_mode_2_v1),
	.af_macro_mode_3 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Macro_mode_3_v1),
	.af_low_light_macro_1 =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Low_Light_Macro_mode_1_v1),
	.af_low_light_macro_2 =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Low_Light_Macro_mode_2_v1),
	.af_low_light_macro_3 =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Low_Light_Macro_mode_3_v1),
	.af_normal_mode_1 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Normal_mode_1_v1),
	.af_normal_mode_2 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Normal_mode_2_v1),
	.af_normal_mode_3 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Normal_mode_3_v1),
	.af_low_light_normal_1 =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Low_Light_normal_mode_1_v1),
	.af_low_light_normal_2 =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Low_Light_normal_mode_2_v1),
	.af_low_light_normal_3 =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Low_Light_normal_mode_3_v1),
	.af_return_infinite_position =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Return_Inf_pos_v1),
	.af_return_macro_position =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Return_Macro_pos_v1),
	.single_af_start = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Single_AF_Start_v1),
	.single_af_off_1 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Single_AF_Off_1_v1),
	.single_af_off_2 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Single_AF_Off_2_v1),
	.dtp_start = S5K4ECGX_REGSET_TABLE(s5k4ecgx_DTP_init_v1),
	.dtp_stop = S5K4ECGX_REGSET_TABLE(s5k4ecgx_DTP_stop_v1),
	.init_reg_1 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_init_reg1_v1),
	.init_reg_2 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_init_reg2_v1),
	.flash_init = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Flash_init_v1),
//	.reset_crop = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Reset_Crop_v1),
	.get_ae_stable_status =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_Get_AE_Stable_Status_v1),
	.get_light_level = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Get_Light_Level_v1),
	.get_1st_af_search_status =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_get_1st_af_search_status_v1),
	.get_2nd_af_search_status =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_get_2nd_af_search_status_v1),
	.get_capture_status =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_get_capture_status_v1),
	.get_esd_status = S5K4ECGX_REGSET_TABLE(s5k4ecgx_get_esd_status_v1),
	.get_iso = S5K4ECGX_REGSET_TABLE(s5k4ecgx_get_iso_reg_v1),
	.get_shutterspeed =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_get_shutterspeed_reg_v1),
#if 0
	.night_capture = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Night_Capture_v1),
	.return_night_preview =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_return_night_preview_v1),
	.beach_snow_fire_capture =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_Beach_Snow_Fire_Capture_v1),
#endif
};
#endif

#ifdef CONFIG_VIDEO_S5K4ECGX_V_1_1
static const struct s5k4ecgx_regs regs_for_fw_version_1_1 = {
	.ev = {
		S5K4ECGX_REGSET(EV_MINUS_4, s5k4ecgx_EV_Minus_4),
		S5K4ECGX_REGSET(EV_MINUS_3, s5k4ecgx_EV_Minus_3),
		S5K4ECGX_REGSET(EV_MINUS_2, s5k4ecgx_EV_Minus_2),
		S5K4ECGX_REGSET(EV_MINUS_1, s5k4ecgx_EV_Minus_1),
		S5K4ECGX_REGSET(EV_DEFAULT, s5k4ecgx_EV_Default),
		S5K4ECGX_REGSET(EV_PLUS_1, s5k4ecgx_EV_Plus_1),
		S5K4ECGX_REGSET(EV_PLUS_2, s5k4ecgx_EV_Plus_2),
		S5K4ECGX_REGSET(EV_PLUS_3, s5k4ecgx_EV_Plus_3),
		S5K4ECGX_REGSET(EV_PLUS_4, s5k4ecgx_EV_Plus_4),
	},
	.metering = {
		S5K4ECGX_REGSET(METERING_MATRIX, s5k4ecgx_Metering_Matrix),
		S5K4ECGX_REGSET(METERING_CENTER, s5k4ecgx_Metering_Center),
		S5K4ECGX_REGSET(METERING_SPOT, s5k4ecgx_Metering_Spot),
	},
	.iso = {
		S5K4ECGX_REGSET(ISO_AUTO, s5k4ecgx_ISO_Auto),
		S5K4ECGX_REGSET(ISO_50, s5k4ecgx_ISO_100),     /* map to 100 */
		S5K4ECGX_REGSET(ISO_100, s5k4ecgx_ISO_100),
		S5K4ECGX_REGSET(ISO_200, s5k4ecgx_ISO_200),
		S5K4ECGX_REGSET(ISO_400, s5k4ecgx_ISO_400),
		S5K4ECGX_REGSET(ISO_800, s5k4ecgx_ISO_400),    /* map to 400 */
		S5K4ECGX_REGSET(ISO_1600, s5k4ecgx_ISO_400),   /* map to 400 */
		S5K4ECGX_REGSET(ISO_SPORTS, s5k4ecgx_ISO_Auto),/* map to auto */
		S5K4ECGX_REGSET(ISO_NIGHT, s5k4ecgx_ISO_Auto), /* map to auto */
		S5K4ECGX_REGSET(ISO_MOVIE, s5k4ecgx_ISO_Auto), /* map to auto */
	},
	.effect = {
		S5K4ECGX_REGSET(IMAGE_EFFECT_NONE, s5k4ecgx_Effect_Normal),
		S5K4ECGX_REGSET(IMAGE_EFFECT_BNW, s5k4ecgx_Effect_Black_White),
		S5K4ECGX_REGSET(IMAGE_EFFECT_SEPIA, s5k4ecgx_Effect_Sepia),
		S5K4ECGX_REGSET(IMAGE_EFFECT_NEGATIVE,
				s5k4ecgx_Effect_Negative),
	},
	.white_balance = {
		S5K4ECGX_REGSET(WHITE_BALANCE_AUTO, s5k4ecgx_WB_Auto),
		S5K4ECGX_REGSET(WHITE_BALANCE_SUNNY, s5k4ecgx_WB_Sunny),
		S5K4ECGX_REGSET(WHITE_BALANCE_CLOUDY, s5k4ecgx_WB_Cloudy),
		S5K4ECGX_REGSET(WHITE_BALANCE_TUNGSTEN, s5k4ecgx_WB_Tungsten),
		S5K4ECGX_REGSET(WHITE_BALANCE_FLUORESCENT,
				s5k4ecgx_WB_Fluorescent),
	},
	.preview_size = {
		S5K4ECGX_REGSET(S5K4ECGX_PREVIEW_QCIF, s5k4ecgx_176_Preview),
		S5K4ECGX_REGSET(S5K4ECGX_PREVIEW_QVGA, s5k4ecgx_320_Preview), 
		S5K4ECGX_REGSET(S5K4ECGX_PREVIEW_CIF, s5k4ecgx_352_Preview),
		S5K4ECGX_REGSET(S5K4ECGX_PREVIEW_528, s5k4ecgx_528_Preview),
		S5K4ECGX_REGSET(S5K4ECGX_PREVIEW_VGA, s5k4ecgx_640_Preview),
		S5K4ECGX_REGSET(S5K4ECGX_PREVIEW_D1, s5k4ecgx_720_Preview),
		S5K4ECGX_REGSET(S5K4ECGX_PREVIEW_WVGA, s5k4ecgx_800_Preview),
		S5K4ECGX_REGSET(S5K4ECGX_PREVIEW_720P, s5k4ecgx_1280_Preview),

	},
#ifdef NEW_CAMERA
	.camcorder_preview_size = {
		S5K4ECGX_REGSET(S5K4ECGX_CAMCORDER_PREVIEW_QCIF, s5k4ecgx_176_Camcorder),
		S5K4ECGX_REGSET(S5K4ECGX_CAMCORDER_PREVIEW_QVGA, s5k4ecgx_320_Camcorder), 
		S5K4ECGX_REGSET(S5K4ECGX_CAMCORDER_PREVIEW_CIF, s5k4ecgx_352_Camcorder),
		S5K4ECGX_REGSET(S5K4ECGX_CAMCORDER_PREVIEW_528, s5k4ecgx_528_Camcorder),
		S5K4ECGX_REGSET(S5K4ECGX_CAMCORDER_PREVIEW_VGA, s5k4ecgx_640_Camcorder),
		S5K4ECGX_REGSET(S5K4ECGX_CAMCORDER_PREVIEW_D1, s5k4ecgx_720_Camcorder),
		S5K4ECGX_REGSET(S5K4ECGX_CAMCORDER_PREVIEW_WVGA, s5k4ecgx_800_Camcorder),
		S5K4ECGX_REGSET(S5K4ECGX_CAMCORDER_PREVIEW_720P, s5k4ecgx_1280_Camcorder),
	},
#endif
	.capture_size = {
		S5K4ECGX_REGSET(S5K4ECGX_CAPTURE_VGA, s5k4ecgx_VGA_Capture),
		S5K4ECGX_REGSET(S5K4ECGX_CAPTURE_WVGA, s5k4ecgx_800_Capture), 
		S5K4ECGX_REGSET(S5K4ECGX_CAPTURE_1MP, s5k4ecgx_1M_Capture),
		S5K4ECGX_REGSET(S5K4ECGX_CAPTURE_W1MP, s5k4ecgx_1600_Capture),
		S5K4ECGX_REGSET(S5K4ECGX_CAPTURE_2MP, s5k4ecgx_2M_Capture),
		S5K4ECGX_REGSET(S5K4ECGX_CAPTURE_W2MP, s5k4ecgx_2048_Capture),
		S5K4ECGX_REGSET(S5K4ECGX_CAPTURE_3MP, s5k4ecgx_3M_Capture),
		S5K4ECGX_REGSET(S5K4ECGX_CAPTURE_W4MP, s5k4ecgx_2560_Capture),
		S5K4ECGX_REGSET(S5K4ECGX_CAPTURE_5MP, s5k4ecgx_5M_Capture),
	},
	.scene_mode = {
		S5K4ECGX_REGSET(SCENE_MODE_NONE, s5k4ecgx_Scene_Default),
		S5K4ECGX_REGSET(SCENE_MODE_PORTRAIT, s5k4ecgx_Scene_Portrait),
		S5K4ECGX_REGSET(SCENE_MODE_NIGHTSHOT, s5k4ecgx_Scene_Nightshot),
		S5K4ECGX_REGSET(SCENE_MODE_BACK_LIGHT, s5k4ecgx_Scene_Backlight),
		S5K4ECGX_REGSET(SCENE_MODE_LANDSCAPE, s5k4ecgx_Scene_Landscape),
		S5K4ECGX_REGSET(SCENE_MODE_SPORTS, s5k4ecgx_Scene_Sports),
		S5K4ECGX_REGSET(SCENE_MODE_PARTY_INDOOR,
				s5k4ecgx_Scene_Party_Indoor),
		S5K4ECGX_REGSET(SCENE_MODE_BEACH_SNOW,
				s5k4ecgx_Scene_Beach_Snow),
		S5K4ECGX_REGSET(SCENE_MODE_SUNSET, s5k4ecgx_Scene_Sunset),
		S5K4ECGX_REGSET(SCENE_MODE_DUSK_DAWN, s5k4ecgx_Scene_Duskdawn),
		S5K4ECGX_REGSET(SCENE_MODE_FALL_COLOR, s5k4ecgx_Scene_Fall_Color),
		S5K4ECGX_REGSET(SCENE_MODE_TEXT, s5k4ecgx_Scene_Text),
		S5K4ECGX_REGSET(SCENE_MODE_FIREWORKS, s5k4ecgx_Scene_Fireworks),
		S5K4ECGX_REGSET(SCENE_MODE_CANDLE_LIGHT,
				s5k4ecgx_Scene_Candle_Light),
	},
	.saturation = {
		S5K4ECGX_REGSET(SATURATION_MINUS_2,
				s5k4ecgx_Saturation_Minus_2),
		S5K4ECGX_REGSET(SATURATION_MINUS_1,
				s5k4ecgx_Saturation_Minus_1),
		S5K4ECGX_REGSET(SATURATION_DEFAULT,
				s5k4ecgx_Saturation_Default),
		S5K4ECGX_REGSET(SATURATION_PLUS_1, s5k4ecgx_Saturation_Plus_1),
		S5K4ECGX_REGSET(SATURATION_PLUS_2, s5k4ecgx_Saturation_Plus_2),
	},
	.contrast = {
		S5K4ECGX_REGSET(CONTRAST_MINUS_2, s5k4ecgx_Contrast_Minus_2),
		S5K4ECGX_REGSET(CONTRAST_MINUS_1, s5k4ecgx_Contrast_Minus_1),
		S5K4ECGX_REGSET(CONTRAST_DEFAULT, s5k4ecgx_Contrast_Default),
		S5K4ECGX_REGSET(CONTRAST_PLUS_1, s5k4ecgx_Contrast_Plus_1),
		S5K4ECGX_REGSET(CONTRAST_PLUS_2, s5k4ecgx_Contrast_Plus_2),
	},
	.sharpness = {
		S5K4ECGX_REGSET(SHARPNESS_MINUS_2, s5k4ecgx_Sharpness_Minus_2),
		S5K4ECGX_REGSET(SHARPNESS_MINUS_1, s5k4ecgx_Sharpness_Minus_1),
		S5K4ECGX_REGSET(SHARPNESS_DEFAULT, s5k4ecgx_Sharpness_Default),
		S5K4ECGX_REGSET(SHARPNESS_PLUS_1, s5k4ecgx_Sharpness_Plus_1),
		S5K4ECGX_REGSET(SHARPNESS_PLUS_2, s5k4ecgx_Sharpness_Plus_2),
	},
	.preview_return = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Preview_Return),
	.night_preview_return = S5K4ECGX_REGSET_TABLE(s5k4ecgx_return_night_preview),
	.beach_fireworks_preview_return = S5K4ECGX_REGSET_TABLE(s5k4ecgx_return_beach_Fireworks_preview),
	.jpeg_quality_high = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Jpeg_Quality_High),
	.jpeg_quality_normal =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_Jpeg_Quality_Normal),
	.jpeg_quality_low = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Jpeg_Quality_Low),
	.flash_start = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Flash_Start),
	.flash_end = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Flash_End),
	.af_assist_flash_start =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_Pre_Flash_Start),
	.af_assist_flash_end =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_Pre_Flash_End),
	.af_low_light_mode_on =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Low_Light_Mode_On),
	.af_low_light_mode_off =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Low_Light_Mode_Off),
#ifndef NEW_CAMERA
	.ae_awb_lock_on =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_AE_AWB_Lock_On),
	.ae_awb_lock_off =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_AE_AWB_Lock_Off),
#else
	.ae_lock = S5K4ECGX_REGSET_TABLE(s5k4ecgx_ae_lock),
	.ae_unlock = S5K4ECGX_REGSET_TABLE(s5k4ecgx_ae_unlock),
	.awb_lock = S5K4ECGX_REGSET_TABLE(s5k4ecgx_awb_lock),
	.awb_unlock = S5K4ECGX_REGSET_TABLE(s5k4ecgx_awb_unlock),
#endif
	.low_cap_on = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Low_Cap_On),
	.low_cap_off = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Low_Cap_Off),
	.night_cap = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Night_Capture),
	.beach_snow_fire_cap = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Beach_Snow_Fire_Capture),
	.wdr_on = S5K4ECGX_REGSET_TABLE(s5k4ecgx_WDR_on),
	.wdr_off = S5K4ECGX_REGSET_TABLE(s5k4ecgx_WDR_off),
	.face_detection_on = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Face_Detection_On),
	.face_detection_off =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_Face_Detection_Off),
	.capture_start = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Capture_Start),
	.af_macro_mode_1 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Macro_mode_1),
	.af_macro_mode_2 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Macro_mode_2),
	.af_macro_mode_3 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Macro_mode_3),
	.af_low_light_macro_1 =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Low_Light_Macro_mode_1),
	.af_low_light_macro_2 =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Low_Light_Macro_mode_2),
	.af_low_light_macro_3 =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Low_Light_Macro_mode_3),
	.af_normal_mode_1 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Normal_mode_1),
	.af_normal_mode_2 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Normal_mode_2),
	.af_normal_mode_3 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Normal_mode_3),
#ifdef NEW_CAMERA
	.af_normal_mode_4 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Normal_mode_4),
#endif
	.af_low_light_normal_1 =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Low_Light_normal_mode_1),
	.af_low_light_normal_2 =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Low_Light_normal_mode_2),
	.af_low_light_normal_3 =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Low_Light_normal_mode_3),
	.af_return_infinite_position =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Return_Inf_pos),
	.af_return_macro_position =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_AF_Return_Macro_pos),
	.single_af_start = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Single_AF_Start),
	.single_af_off_1 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Single_AF_Off_1),
	.single_af_off_2 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Single_AF_Off_2),
	.dtp_start = S5K4ECGX_REGSET_TABLE(s5k4ecgx_DTP_init),
	.dtp_stop = S5K4ECGX_REGSET_TABLE(s5k4ecgx_DTP_stop),
	.init_reg_1 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_init_reg1),
	.init_reg_2 = S5K4ECGX_REGSET_TABLE(s5k4ecgx_init_reg2),
#ifdef NEW_CAMERA /* For Qik */
	.init_reg_1_qik = S5K4ECGX_REGSET_TABLE(s5k4ecgx_init_reg1_Qik),
	.init_reg_2_qik = S5K4ECGX_REGSET_TABLE(s5k4ecgx_init_reg2_Qik),
	.s5k4ecgx_640_preview_qik = S5K4ECGX_REGSET_TABLE(s5k4ecgx_640_Preview_Qik),
#endif
	.flash_init = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Flash_init),
	.get_ae_stable_status =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_Get_AE_Stable_Status),
	.get_light_level = S5K4ECGX_REGSET_TABLE(s5k4ecgx_Get_Light_Level),
	.get_1st_af_search_status =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_get_1st_af_search_status),
	.get_2nd_af_search_status =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_get_2nd_af_search_status),
	.get_capture_status =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_get_capture_status),
	.get_esd_status = S5K4ECGX_REGSET_TABLE(s5k4ecgx_get_esd_status),
	.get_iso = S5K4ECGX_REGSET_TABLE(s5k4ecgx_get_iso_reg),
	.get_shutterspeed =
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_get_shutterspeed_reg),
	.set_fix_fps = 
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_FPS_30),		
	.set_15_fps = 
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_FPS_15),		
	.set_auto_fps = 
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_FPS_Auto),
	.set_auto_contrast_on = 
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_Auto_Contrast_ON),		
	.set_auto_contrast_off = 
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_Auto_Contrast_OFF),				
	.set_af_window_center = 
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_afwin_center),
       .set_softlanding = 
		S5K4ECGX_REGSET_TABLE(s5k4ecgx_softlanding_setting),       
	.set_x1_25_zoom = {
		S5K4ECGX_REGSET(ZOOM_LEVEL_0, s5k4ecgx_x1_25_zoom_0),
		S5K4ECGX_REGSET(ZOOM_LEVEL_1, s5k4ecgx_x1_25_zoom_1),
		S5K4ECGX_REGSET(ZOOM_LEVEL_2, s5k4ecgx_x1_25_zoom_2),
		S5K4ECGX_REGSET(ZOOM_LEVEL_3, s5k4ecgx_x1_25_zoom_3),
		S5K4ECGX_REGSET(ZOOM_LEVEL_4, s5k4ecgx_x1_25_zoom_4),
		S5K4ECGX_REGSET(ZOOM_LEVEL_5, s5k4ecgx_x1_25_zoom_5),
		S5K4ECGX_REGSET(ZOOM_LEVEL_6, s5k4ecgx_x1_25_zoom_6),
		S5K4ECGX_REGSET(ZOOM_LEVEL_7, s5k4ecgx_x1_25_zoom_7),
		S5K4ECGX_REGSET(ZOOM_LEVEL_8, s5k4ecgx_x1_25_zoom_8),		
	},
	.set_x1_6_zoom = {
		S5K4ECGX_REGSET(ZOOM_LEVEL_0, s5k4ecgx_x1_6_zoom_0),
		S5K4ECGX_REGSET(ZOOM_LEVEL_1, s5k4ecgx_x1_6_zoom_1),
		S5K4ECGX_REGSET(ZOOM_LEVEL_2, s5k4ecgx_x1_6_zoom_2),
		S5K4ECGX_REGSET(ZOOM_LEVEL_3, s5k4ecgx_x1_6_zoom_3),
		S5K4ECGX_REGSET(ZOOM_LEVEL_4, s5k4ecgx_x1_6_zoom_4),
		S5K4ECGX_REGSET(ZOOM_LEVEL_5, s5k4ecgx_x1_6_zoom_5),
		S5K4ECGX_REGSET(ZOOM_LEVEL_6, s5k4ecgx_x1_6_zoom_6),
		S5K4ECGX_REGSET(ZOOM_LEVEL_7, s5k4ecgx_x1_6_zoom_7),
		S5K4ECGX_REGSET(ZOOM_LEVEL_8, s5k4ecgx_x1_6_zoom_8),		
	},
	.set_x1_77_zoom = {
		S5K4ECGX_REGSET(ZOOM_LEVEL_0, s5k4ecgx_x1_77_zoom_0),
		S5K4ECGX_REGSET(ZOOM_LEVEL_1, s5k4ecgx_x1_77_zoom_1),
		S5K4ECGX_REGSET(ZOOM_LEVEL_2, s5k4ecgx_x1_77_zoom_2),
		S5K4ECGX_REGSET(ZOOM_LEVEL_3, s5k4ecgx_x1_77_zoom_3),
		S5K4ECGX_REGSET(ZOOM_LEVEL_4, s5k4ecgx_x1_77_zoom_4),
		S5K4ECGX_REGSET(ZOOM_LEVEL_5, s5k4ecgx_x1_77_zoom_5),
		S5K4ECGX_REGSET(ZOOM_LEVEL_6, s5k4ecgx_x1_77_zoom_6),
		S5K4ECGX_REGSET(ZOOM_LEVEL_7, s5k4ecgx_x1_77_zoom_7),
		S5K4ECGX_REGSET(ZOOM_LEVEL_8, s5k4ecgx_x1_77_zoom_8),		
	},
	.set_x2_zoom = {
		S5K4ECGX_REGSET(ZOOM_LEVEL_0, s5k4ecgx_x2_zoom_0),
		S5K4ECGX_REGSET(ZOOM_LEVEL_1, s5k4ecgx_x2_zoom_1),
		S5K4ECGX_REGSET(ZOOM_LEVEL_2, s5k4ecgx_x2_zoom_2),
		S5K4ECGX_REGSET(ZOOM_LEVEL_3, s5k4ecgx_x2_zoom_3),
		S5K4ECGX_REGSET(ZOOM_LEVEL_4, s5k4ecgx_x2_zoom_4),
		S5K4ECGX_REGSET(ZOOM_LEVEL_5, s5k4ecgx_x2_zoom_5),
		S5K4ECGX_REGSET(ZOOM_LEVEL_6, s5k4ecgx_x2_zoom_6),
		S5K4ECGX_REGSET(ZOOM_LEVEL_7, s5k4ecgx_x2_zoom_7),
		S5K4ECGX_REGSET(ZOOM_LEVEL_8, s5k4ecgx_x2_zoom_8),		
	},
	.set_x4_zoom = {
		S5K4ECGX_REGSET(ZOOM_LEVEL_0, s5k4ecgx_x4_zoom_0),
		S5K4ECGX_REGSET(ZOOM_LEVEL_1, s5k4ecgx_x4_zoom_1),
		S5K4ECGX_REGSET(ZOOM_LEVEL_2, s5k4ecgx_x4_zoom_2),
		S5K4ECGX_REGSET(ZOOM_LEVEL_3, s5k4ecgx_x4_zoom_3),
		S5K4ECGX_REGSET(ZOOM_LEVEL_4, s5k4ecgx_x4_zoom_4),
		S5K4ECGX_REGSET(ZOOM_LEVEL_5, s5k4ecgx_x4_zoom_5),
		S5K4ECGX_REGSET(ZOOM_LEVEL_6, s5k4ecgx_x4_zoom_6),
		S5K4ECGX_REGSET(ZOOM_LEVEL_7, s5k4ecgx_x4_zoom_7),
		S5K4ECGX_REGSET(ZOOM_LEVEL_8, s5k4ecgx_x4_zoom_8),		
	},	
};
#endif

struct s5k4ecgx_state {
	struct s5k4ecgx_platform_data *pdata;
	struct v4l2_subdev sd;
	struct v4l2_pix_format pix;
	struct v4l2_fract timeperframe;
	struct s5k4ecgx_jpeg_param jpeg;
	struct s5k4ecgx_version fw;
	struct s5k4ecgx_version prm;
	struct s5k4ecgx_date_info dateinfo;
	struct s5k4ecgx_position position;
	struct v4l2_streamparm strm;
	struct s5k4ecgx_gps_info gps_info;
	struct mutex ctrl_lock;
	struct completion af_complete;
	enum s5k4ecgx_runmode runmode;
	enum s5k4ecgx_oprmode oprmode;
	enum af_operation_status af_status;
	int framesize_index;
	int sensor_version;
	int freq;		/* MCLK in Hz */
	int check_dataline;
	int check_previewdata;
	bool flash_on;
       bool af_preflash_on;
	bool sensor_af_in_low_light_mode;
	bool flash_state_on_previous_capture;
	bool touch_ae_af_state; /* 0: never 1: ever */
	int one_frame_delay_ms;
	int vt_mode; /* vt mode, 1:Qik */
	int snapshot_width;
	int zoom_ratio;
	int zoom_level;
	const struct s5k4ecgx_regs *regs;
};

static const struct v4l2_fmtdesc capture_fmts[] = {
	{
		.index		= 0,
		.type		= V4L2_BUF_TYPE_VIDEO_CAPTURE,
		.flags		= FORMAT_FLAGS_COMPRESSED,
		.description	= "JPEG + Postview",
		.pixelformat	= V4L2_PIX_FMT_JPEG,
	},
};

static bool bStartFineSearch = false;

/**
 * s5k4ecgx_i2c_read_twobyte: Read 2 bytes from sensor
 */
static int s5k4ecgx_i2c_read_twobyte(struct i2c_client *client,
				  u16 subaddr, u16 *data)
{
	int err;
	unsigned char buf[2];
	struct i2c_msg msg[2];

	cpu_to_be16s(&subaddr);

	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = 2;
	msg[0].buf = (u8 *)&subaddr;

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 2;
	msg[1].buf = buf;

	err = i2c_transfer(client->adapter, msg, 2);
	if (unlikely(err != 2)) {
		dev_err(&client->dev,
			"%s: register read fail\n", __func__);
		return -EIO;
	}

	*data = ((buf[0] << 8) | buf[1]);

	return 0;
}

/**
 * s5k4ecgx_i2c_write_twobyte: Write (I2C) multiple bytes to the camera sensor
 * @client: pointer to i2c_client
 * @cmd: command register
 * @w_data: data to be written
 * @w_len: length of data to be written
 *
 * Returns 0 on success, <0 on error
 */
static int s5k4ecgx_i2c_write_twobyte(struct i2c_client *client,
					 u16 addr, u16 w_data)
{
	int retry_count = 5;
	unsigned char buf[4];
	struct i2c_msg msg = {client->addr, 0, 4, buf};
	int ret = 0;

	buf[0] = addr >> 8;
	buf[1] = addr;
	buf[2] = w_data >> 8;
	buf[3] = w_data & 0xff;

	s5k4ecgx_debug(S5K4ECGX_DEBUG_I2C, "%s : W(0x%02X%02X%02X%02X)\n",
		__func__, buf[0], buf[1], buf[2], buf[3]);

	do {
		ret = i2c_transfer(client->adapter, &msg, 1);
		if (likely(ret == 1))
			break;
		msleep(POLL_TIME_MS);
		dev_err(&client->dev, "%s: I2C err %d, retry %d.\n",
			__func__, ret, retry_count);
	} while (retry_count-- > 0);
	if (ret != 1) {
		dev_err(&client->dev, "%s: I2C is not working.\n", __func__);
		return -EIO;
	}

	return 0;
}

#ifdef CONFIG_LOAD_FILE
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
/*#include <asm/uaccess.h>*/

static char *s5k4ecgx_regs_table;

static int s5k4ecgx_regs_table_size;

int s5k4ecgx_regs_table_init(int rev)
{
	struct file *filp;
	char *dp;
	long l;
	loff_t pos;
	int ret;
	mm_segment_t fs = get_fs();

	set_fs(get_ds());
#if 1
	if (rev)
#ifdef NEW_CAMERA
		filp = filp_open("/sdcard/s5k4ecgx_regs_1_1.h", O_RDONLY, 0);
#endif
	else
		filp = filp_open("/sdcard/s5k4ecgx_regs_1_0.h", O_RDONLY, 0);
#else
	filp = filp_open("/mnt/internal_sd/external_sd/s5k4ecgx.h",
			O_RDONLY, 0);
#endif

	if (IS_ERR_OR_NULL(filp)) {
		printk(KERN_DEBUG "file open error\n");
		return PTR_ERR(filp);
	}

	l = filp->f_path.dentry->d_inode->i_size;
	printk(KERN_DEBUG "l = %ld\n", l);
	//dp = kmalloc(l, GFP_KERNEL);
	dp = vmalloc(l);
	if (dp == NULL) {
		printk(KERN_DEBUG "Out of Memory\n");
		filp_close(filp, current->files);
	}

	pos = 0;
	memset(dp, 0, l);
	ret = vfs_read(filp, (char __user *)dp, l, &pos);

	if (ret != l) {
		printk(KERN_DEBUG "Failed to read file ret = %d\n", ret);
		/*kfree(dp);*/
		vfree(dp);
		filp_close(filp, current->files);
		return -EINVAL;
	}

	filp_close(filp, current->files);

	set_fs(fs);

	s5k4ecgx_regs_table = dp;

	s5k4ecgx_regs_table_size = l;

	*((s5k4ecgx_regs_table + s5k4ecgx_regs_table_size) - 1) = '\0';

	printk(KERN_DEBUG "s5k4ecgx_regs_table 0x%p, %ld\n", dp, l);

	return 0;
}

void s5k4ecgx_regs_table_exit(void)
{
	if (s5k4ecgx_regs_table) {
		s5k4ecgx_regs_table = NULL;
		kfree(s5k4ecgx_regs_table);
	}
}

static int s5k4ecgx_regs_table_write(struct i2c_client *client, char *name)
{
	char *start, *end, *reg;
	unsigned int value;
	char reg_buf[11];

	*(reg_buf + 10) = '\0';

	start = strstr(s5k4ecgx_regs_table, name);
	end = strstr(start, "};");

	while (1) {
		/* Find Address */
		reg = strstr(start, "0x");
		if (reg)
			start = strstr(reg, "\n");

		if ((reg == NULL) || (reg > end))
			break;
		/* Write Value to Address */
		if (reg != NULL)  {
			memcpy(reg_buf, (reg), 10);
			value =
				(unsigned int)simple_strtoul(reg_buf,
						NULL, 16);

			/*	printk("==== value 0x%08x=======\n", value); */

			s5k4ecgx_i2c_write_twobyte(client,
					((value >> 16) & 0xFFFF), (value & 0xFFFF));
		}
	}

	return 0;
}

static int s5k4ecgx_write_regs(struct v4l2_subdev *sd, const u32 regs[],
			     int size, char *name)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err;

	err = s5k4ecgx_regs_table_write(client, name);
	if (unlikely(err < 0)) {
		v4l_info(client,
				"%s: s5k4ecgx_regs_table_write failed\n", __func__);
		return err;
	}
	return 0;
}
#else
static int s5k4ecgx_write_regs(struct v4l2_subdev *sd, const u32 regs[],
			     int size)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int i, err;

	for (i = 0; i < size; i++) {
		//printk("==== value 0x%08x=======\n", regs[i]); 
		err = s5k4ecgx_i2c_write_twobyte(client,
			(regs[i] >> 16), regs[i]);
		if (unlikely(err != 0)) {
			dev_err(&client->dev,
				"%s: register write failed\n", __func__);
			return err;
		}
	}

	return 0;
}
#endif

static int s5k4ecgx_set_from_table(struct v4l2_subdev *sd,
				const char *setting_name,
				const struct s5k4ecgx_regset_table *table,
				int table_size, int index)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	dev_dbg(&client->dev, "%s: set %s index %d\n",
		__func__, setting_name, index);
	if ((index < 0) || (index >= table_size)) {
		dev_err(&client->dev,
			"%s: index(%d) out of range[0:%d] for table for %s\n",
			__func__, index, table_size, setting_name);
		return -EINVAL;
	}
	table += index;
	if (table->reg == NULL)
		return -EINVAL;
#ifdef CONFIG_LOAD_FILE
	return s5k4ecgx_write_regs(sd, table->reg, table->array_size, table->name);
#else
	return s5k4ecgx_write_regs(sd, table->reg, table->array_size);
#endif
}

static int s5k4ecgx_set_parameter(struct v4l2_subdev *sd,
				int *current_value_ptr,
				int new_value,
				const char *setting_name,
				const struct s5k4ecgx_regset_table *table,
				int table_size)
{
	int err;

	if (*current_value_ptr == new_value)
		return 0;

	err = s5k4ecgx_set_from_table(sd, setting_name, table,
				table_size, new_value);

	if (!err)
		*current_value_ptr = new_value;
	return err;
}

static int s5k4ecgx_set_af_window_center(struct v4l2_subdev *sd)
{
       struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct sec_cam_parm *parms =
		(struct sec_cam_parm *)&state->strm.parm.raw_data;    
        int err = 0;
        dev_err(&client->dev, "%s", __func__);
        
        err =  s5k4ecgx_set_from_table(sd, "afwin center",
                    &state->regs->set_af_window_center,1,0);
        if(err < 0)
        {
            dev_err(&client->dev,"%s: Not support 'saturation' value %d\n",__func__, SATURATION_DEFAULT);
        }
        return err;
}

#ifdef NEW_CAMERA
static void s5k4ecgx_set_REG_TC_DBG_AutoAlgEnBits(struct v4l2_subdev *sd, int bit, int set)
{

	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state = container_of(sd, struct s5k4ecgx_state, sd);
	struct sec_cam_parm *parms =
		(struct sec_cam_parm *)&state->strm.parm.raw_data;
	int REG_TC_DBG_AutoAlgEnBits = 0;

       //dev_err(&client->dev,"%s : bit = %d",__func__,bit);
	/* Read 04E6 */
	s5k4ecgx_i2c_write_twobyte(client, 0x002C, 0x7000);
	s5k4ecgx_i2c_write_twobyte(client, 0x002E, 0x04E6);
	s5k4ecgx_i2c_read_twobyte(client, 0x0F12, (unsigned short*)&REG_TC_DBG_AutoAlgEnBits);
	
	if(bit == 3 && set == true)
	{
		if(REG_TC_DBG_AutoAlgEnBits & 0x8 == 1)return;
		if(parms->scene_mode == SCENE_MODE_NIGHTSHOT)msleep(250);
		else msleep(100);
		REG_TC_DBG_AutoAlgEnBits = REG_TC_DBG_AutoAlgEnBits | 0x8;
		s5k4ecgx_i2c_write_twobyte(client, 0x0028, 0x7000);
		s5k4ecgx_i2c_write_twobyte(client, 0x002A, 0x04E6);
		s5k4ecgx_i2c_write_twobyte(client, 0x0F12, REG_TC_DBG_AutoAlgEnBits);
	}
	else if(bit == 3 && set == false)
	{
		if(REG_TC_DBG_AutoAlgEnBits & 0x8 == 0)return;
		if(parms->scene_mode == SCENE_MODE_NIGHTSHOT)msleep(250);
		else msleep(100);
		REG_TC_DBG_AutoAlgEnBits = REG_TC_DBG_AutoAlgEnBits & 0xFFF7;
		s5k4ecgx_i2c_write_twobyte(client, 0x0028, 0x7000);
		s5k4ecgx_i2c_write_twobyte(client, 0x002A, 0x04E6);
		s5k4ecgx_i2c_write_twobyte(client, 0x0F12, REG_TC_DBG_AutoAlgEnBits);
	}
	else if(bit == 5 && set == true)
	{
		if(REG_TC_DBG_AutoAlgEnBits & 0x20 == 1)return;
		if(parms->scene_mode == SCENE_MODE_NIGHTSHOT)msleep(250);
		else msleep(100);
		REG_TC_DBG_AutoAlgEnBits = REG_TC_DBG_AutoAlgEnBits | 0x20;
		s5k4ecgx_i2c_write_twobyte(client, 0x0028, 0x7000);
		s5k4ecgx_i2c_write_twobyte(client, 0x002A, 0x04E6);
		s5k4ecgx_i2c_write_twobyte(client, 0x0F12, REG_TC_DBG_AutoAlgEnBits);
	}
	else if(bit == 5 && set == false)
	{
		if(REG_TC_DBG_AutoAlgEnBits & 0x20 == 0)return;
		if(parms->scene_mode == SCENE_MODE_NIGHTSHOT)msleep(250);
		else msleep(100);
		REG_TC_DBG_AutoAlgEnBits = REG_TC_DBG_AutoAlgEnBits & 0xFFDF;
		s5k4ecgx_i2c_write_twobyte(client, 0x0028, 0x7000);
		s5k4ecgx_i2c_write_twobyte(client, 0x002A, 0x04E6);
		s5k4ecgx_i2c_write_twobyte(client, 0x0F12, REG_TC_DBG_AutoAlgEnBits);
	}

#if 0
	/* restore write mode */
	s5k4ecgx_i2c_write_twobyte(client, 0x0028, 0x7000);
#endif

	return;
}

static void s5k4ecgx_check_REG_TC_GP_EnablePreviewChanged(struct v4l2_subdev *sd) /* Check the preview frame */
{
	int cnt = 0;
	int REG_TC_GP_EnablePreviewChanged = 0;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
//	printk("[S5K4ECGX]s5k4ecgx_check_REG_TC_GP_EnablePreviewChanged\n");
	while(cnt < 100) /* Max duration : 1000msec */
	{
		s5k4ecgx_i2c_write_twobyte(client, 0x002C, 0x7000);
		s5k4ecgx_i2c_write_twobyte(client, 0x002E, 0x0244);
		s5k4ecgx_i2c_read_twobyte(client, 0x0F12, (unsigned short*)&REG_TC_GP_EnablePreviewChanged);
		if(!REG_TC_GP_EnablePreviewChanged)break;
		mdelay(10);
		cnt++;
	}
	if(cnt){
	//	printk("[S5K4ECGX] wait time for preview frame : %dms\n",cnt*10);
	}
	if(REG_TC_GP_EnablePreviewChanged)printk("[S5K4ECGX] start preview failed.\n");
}


#endif

static int s5k4ecgx_set_preview_stop(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);

	if (state->runmode == S5K4ECGX_RUNMODE_RUNNING)
		state->runmode = S5K4ECGX_RUNMODE_IDLE;

	dev_dbg(&client->dev, "%s: =====change preview mode=====\n",
		__func__);

	return 0;
}

static int s5k4ecgx_set_preview_start(struct v4l2_subdev *sd)
{
	int err;
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct sec_cam_parm *parms =
		(struct sec_cam_parm *)&state->strm.parm.raw_data;
	bool set_size = true;

	dev_dbg(&client->dev, "%s: runmode = %d\n",
		__func__, state->runmode);

#if 1
	if(state->runmode == S5K4ECGX_RUNMODE_ERROR)
	{
		state->runmode = S5K4ECGX_RUNMODE_NOTREADY;
		return -ENODEV; /* No device or not ready */

	}
#endif

	if (!state->pix.width || !state->pix.height ||
		!state->strm.parm.capture.timeperframe.denominator)
		return -EINVAL;

	if (state->check_dataline) {
		if (s5k4ecgx_set_from_table(sd, "dtp start",
					&state->regs->dtp_start, 1, 0) < 0)
			return -EIO;
	}

	if (state->runmode == S5K4ECGX_RUNMODE_CAPTURE) {

		state->touch_ae_af_state = 0; /* Initialization touch AF & AE */
              err = s5k4ecgx_set_af_window_center(sd);
              if(err<0){
                    dev_err(&client->dev,
                                "%s: failed: s5k4ecgx_set_af_window_center\n",
                                __func__);
                            return -EIO;
              }

		dev_dbg(&client->dev, "%s: sending Preview_Return cmd\n",
			__func__);

		err = s5k4ecgx_set_from_table(sd, "preview return",
					&state->regs->preview_return, 1, 0);

		if (err < 0) {
			dev_err(&client->dev,
				"%s: failed: s5k4ecgx_Preview_Return\n",
				__func__);
			return -EIO;
		}
	
#ifndef	NEW_CAMERA
		if (parms->scene_mode == SCENE_MODE_NIGHTSHOT)
		{
			err = s5k4ecgx_set_from_table(sd, "return night preview",
					&state->regs->night_preview_return, 1, 0);

        		if (err < 0) {
        			dev_err(&client->dev,
        				"%s: failed: s5k4ecgx_return_night_preview\n",
        				__func__);
        			return -EIO;
        		}
		}

		/* Beach/Snow/Fireworks Scene mode add needed */
		if( (parms->scene_mode == SCENE_MODE_BEACH_SNOW) || (parms->scene_mode == SCENE_MODE_FIREWORKS) )
		{
			err = s5k4ecgx_set_from_table(sd, "return beach fireworks preview",
					&state->regs->beach_fireworks_preview_return, 1, 0);

		if (err < 0) {
			dev_err(&client->dev,
						"%s: failed: s5k4ecgx_return_night_preview\n",
				__func__);
			return -EIO;
		}
	}
#else
		/* Nothing needed */
#endif

	} else {
#ifdef NEW_CAMERA
		if(camcorder_check_flag == 1){ /* 1: Movie mode */
		//	printk("[%s:%d] Check in\n", __func__, __LINE__);
			err = s5k4ecgx_set_from_table(sd, "camcorder_preview_size",
					state->regs->camcorder_preview_size,
					ARRAY_SIZE(state->regs->camcorder_preview_size),
					state->framesize_index);
		} else {

			if( state->vt_mode == 1) {
			//	printk("[%s:%d] Qik for preview_size\n",__func__, __LINE__);
				if( s5k4ecgx_set_from_table(sd, "640 Preview Qik",
						&state->regs->s5k4ecgx_640_preview_qik, 1, 0) < 0)
					return -EIO;
			}
			else
			{
		//		printk("[%s:%d] Check in\n", __func__, __LINE__);
		err = s5k4ecgx_set_from_table(sd, "preview_size",
					state->regs->preview_size,
					ARRAY_SIZE(state->regs->preview_size),
						state->framesize_index);

		if (err < 0) {
			dev_err(&client->dev,
				"%s: failed: Could not set preview size\n",
				__func__);
			return -EIO;
		}
	}
		}
#else
		err = s5k4ecgx_set_from_table(sd, "preview_size",
				state->regs->preview_size,
				ARRAY_SIZE(state->regs->preview_size),
				state->framesize_index);

		if (err < 0) {
			dev_err(&client->dev,
					"%s: failed: Could not set preview size\n",
					__func__);
			return -EIO;
		}


#endif
	}

#ifdef NEW_CAMERA 
	/* Camera Preview delay by using polling for low fps */
	s5k4ecgx_check_REG_TC_GP_EnablePreviewChanged(sd);
#endif

	dev_dbg(&client->dev, "%s: runmode now RUNNING\n", __func__);
	state->runmode = S5K4ECGX_RUNMODE_RUNNING;

	return 0;
}

#if 1 /* For the Touch AF implemention  */
#if 0
// 416 * 320 Based window
#define AF_OUTER_WINDOW_WIDTH   208
#define AF_OUTER_WINDOW_HEIGHT  177
#define AF_INNER_WINDOW_WIDTH   93
#define AF_INNER_WINDOW_HEIGHT  95
#else
// 640 * 480 Based window
#define AF_OUTER_WINDOW_WIDTH  320
#define AF_OUTER_WINDOW_HEIGHT 266
#define AF_INNER_WINDOW_WIDTH  72
#define AF_INNER_WINDOW_HEIGHT 143

#define AE_WINDOW_WIDTH 143
#define AE_WINDOW_HEIGHT 143

#endif
static int s5k4ecgx_touch_auto_focus(struct v4l2_subdev *sd, int value)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct sec_cam_parm *parms =
		(struct sec_cam_parm *)&state->strm.parm.raw_data;
	int err;
	unsigned short FirstWinStartX, FirstWinStartY, SecondWinStartX, SecondWinStartY;
       unsigned short FirstWinEndX, FirstWinEndY, ConvFirstWinStartX, ConvFirstWinStartY;
       unsigned short SecondWinEndX, SecondWinEndY, ConvSecondWinStartX, ConvSecondWinStartY;
	unsigned short DefaultFirstWinStartX, DefaultFirstWinStartY, DefaultSecondWinStartX, DefaultSecondWinStartY;
	int preview_width, preview_height;
	int count, read_value;
	unsigned long zoom_value,zoom_ratio;
	int touch_position_x, touch_position_y;
	int zoom_width,zoom_height;
	int width_delta, height_delta;

	preview_width = state->pix.width;
	preview_height = state->pix.height;

	DefaultFirstWinStartX = preview_width - ((preview_width - AF_OUTER_WINDOW_WIDTH) / 2); /* 640x480 => 160, 800x480 => 240 */
	DefaultFirstWinStartY = preview_height - ((preview_height - AF_OUTER_WINDOW_HEIGHT) / 2); /* 640x480 => 107, 800x480 => 107 */
	DefaultSecondWinStartX = preview_width - ((preview_width - AF_INNER_WINDOW_WIDTH) / 2); /* 640x480 => 248, 800x480 => 160 */
	DefaultSecondWinStartY = preview_height - ((preview_height - AF_INNER_WINDOW_HEIGHT) / 2); /* 640x480 => 328, 800x480 => 160 */

	//printk("[%s:%d] preview_width %d, preview_height %d\n",__func__, __LINE__, preview_width, preview_height);
	//printk("[%s:%d] DefaultFirstWinStartX %d, DefaultFirstWinStartY %d\n",__func__, __LINE__, DefaultFirstWinStartX, DefaultFirstWinStartY);
	//printk("[%s:%d] DefaultSecondWinStartX %d, DefaultSecondWinStartY %d\n",__func__, __LINE__,DefaultSecondWinStartX ,DefaultSecondWinStartY );


	/* value 0 is Touch AF Stop, 1 is Touch AF start */
	if(value == 0)	{ /* AF stop */
		s5k4ecgx_i2c_write_twobyte(client, 0xFCFC, 0xD000);
		s5k4ecgx_i2c_write_twobyte(client, 0x0028, 0x7000);
		s5k4ecgx_i2c_write_twobyte(client, 0x002A, 0x0294);
		s5k4ecgx_i2c_write_twobyte(client, 0x0F12, (unsigned short)((DefaultFirstWinStartX << 10) / preview_width)); /* FstWinStartX */
		s5k4ecgx_i2c_write_twobyte(client, 0x0F12, (unsigned short)((DefaultFirstWinStartY << 10) / preview_height)); /* FstWinStartY */
		s5k4ecgx_i2c_write_twobyte(client, 0x0F12, (320 << 10) / preview_width  ); /* FstWinSizeX : 320 */
		s5k4ecgx_i2c_write_twobyte(client, 0x0F12, (266 << 10) / preview_height );  /* FstWinSizeY : 266 */
		s5k4ecgx_i2c_write_twobyte(client, 0x0F12, (unsigned short)((DefaultSecondWinStartX << 10) / preview_width)); /* ScndWinStartX */
		s5k4ecgx_i2c_write_twobyte(client, 0x0F12, (unsigned short)((DefaultSecondWinStartY << 10) / preview_height)); /* ScndWinStartY */
		s5k4ecgx_i2c_write_twobyte(client, 0x0F12, (143 << 10) / preview_width  ); /* ScndWinSizeX : 143 */
		s5k4ecgx_i2c_write_twobyte(client, 0x0F12, (143 << 10) / preview_height ); /* ScndWinSizeY : 143  */
		s5k4ecgx_i2c_write_twobyte(client, 0x0F12, 0x0001); /* WinSizesUpdated */		
	}
	else if(value == 1) { /* AF start */
		// Prevent divided-by-zero.
		if(preview_width == 0 || preview_height == 0) {
			dev_err(&client->dev, "%s: Either preview_width or preview_height is zero\n", __func__);
			return -EIO;
		}
        
		// caculate zoom
		switch(state->zoom_ratio)
		{
			case 125:
				zoom_value = Zoom_X1_25[state->zoom_level]<<10;
				break;
				
			case 160:
				zoom_value = Zoom_X1_6[state->zoom_level]<<10;
				break;
				
			case 177:
				zoom_value = Zoom_X1_77[state->zoom_level]<<10;
				break;
				
			case 200:
				zoom_value = Zoom_X2[state->zoom_level]<<10;
				break;
				
			case 100:
			default:
				zoom_value = cnZoomDefaultValue<<10;
				break;
		}

		zoom_ratio = zoom_value/cnZoomDefaultValue;
		
		zoom_width = (preview_width<<10)/zoom_ratio;
		zoom_height =(preview_height<<10)/zoom_ratio;

		//printk("zoom_value = %u, zoom_ratio = %u\n",zoom_value, zoom_ratio);
		//printk("zoom width %d, zoom height %d\n",zoom_width, zoom_height);

		if(preview_width - zoom_width <= 0){
			width_delta = 0;
		}else{
			width_delta = (preview_width - zoom_width)/2;
		}

		if(preview_height - zoom_height <= 0){
			height_delta = 0;
		}else{
			height_delta = (preview_height - zoom_height)/2;
		}
			
		if(state->position.x == 0){
			touch_position_x = width_delta;
		}else{
			touch_position_x = (state->position.x<<10)/zoom_ratio + width_delta;
		}

		if(state->position.y == 0){
			touch_position_y = height_delta;
		}else{
			touch_position_y = (state->position.y<<10)/zoom_ratio + height_delta;
		}
		
		//printk("touch X %d, touch Y %d\n",state->position.x,  state->position.y);
		//printk("zoom_touch X %d, zoom_touch Y %d\n", touch_position_x, touch_position_y);		
		
		FirstWinStartX = SecondWinStartX = touch_position_x;
		FirstWinStartY = SecondWinStartY = touch_position_y;
		//printk("[%s:%d]touch X %d, touch Y %d\n",__func__, __LINE__,  FirstWinStartX, FirstWinStartY);

		// AF Position(Round Down)
		if(FirstWinStartX > AF_OUTER_WINDOW_WIDTH/2) {
			FirstWinStartX -= AF_OUTER_WINDOW_WIDTH/2;

			if(FirstWinStartX + AF_OUTER_WINDOW_WIDTH > preview_width) {
				dev_err(&client->dev, "%s: X Position Overflow : [%d, %d] \n", __func__, FirstWinStartX, AF_OUTER_WINDOW_WIDTH);

				FirstWinStartX = preview_width - AF_OUTER_WINDOW_WIDTH - 1;
			}
		}
		else
			FirstWinStartX = 0;


		if(FirstWinStartY > AF_OUTER_WINDOW_HEIGHT/2)	{
			FirstWinStartY -= AF_OUTER_WINDOW_HEIGHT/2;

			if(FirstWinStartY + AF_OUTER_WINDOW_HEIGHT > preview_height) {
				dev_err(&client->dev, "%s: Y Position Overflow : [%d, %d] \n", __func__, FirstWinStartY, AF_OUTER_WINDOW_HEIGHT);

				FirstWinStartY = preview_height - AF_OUTER_WINDOW_HEIGHT - 1;
			}
		}
		else
			FirstWinStartY = 0;
                //printk("[%s:%d] FirstWinStartX %d, FirstWinStartY %d\n",__func__, __LINE__, FirstWinStartX, FirstWinStartY);

		// AF Position(Round Down)
		if(SecondWinStartX > AF_INNER_WINDOW_WIDTH/2) {
			SecondWinStartX -= AF_INNER_WINDOW_WIDTH/2;

			if(SecondWinStartX + AF_INNER_WINDOW_WIDTH > preview_width) {
				dev_err(&client->dev, "%s: X Position Overflow : [%d, %d] \n", __func__, SecondWinStartX, AF_INNER_WINDOW_WIDTH);

				SecondWinStartX = preview_width - AF_INNER_WINDOW_WIDTH - 1;
			}
		}
		else
			SecondWinStartX = 0;

		if(SecondWinStartY > AF_INNER_WINDOW_HEIGHT/2)	{
			SecondWinStartY -= AF_INNER_WINDOW_HEIGHT/2;

			if(SecondWinStartY + AF_INNER_WINDOW_HEIGHT > preview_height) {
				dev_err(&client->dev, "%s: Y Position Overflow : [%d, %d] \n", __func__, SecondWinStartY, AF_INNER_WINDOW_HEIGHT);

				SecondWinStartY = preview_height - AF_INNER_WINDOW_HEIGHT - 1;
			}
		}
		else
			SecondWinStartY = 0;
                //printk("[%s:%d] SecondWinStartX %d, SecondWinStartY %d\n",__func__, __LINE__, SecondWinStartX, SecondWinStartY);
                
                // if use mirror/flip, need this code.
                FirstWinEndX = FirstWinStartX + AF_OUTER_WINDOW_WIDTH;
                FirstWinEndY = FirstWinStartY + AF_OUTER_WINDOW_HEIGHT;
                
                if(preview_width - FirstWinEndX <= 0)
                    ConvFirstWinStartX = 0;
                else
                    ConvFirstWinStartX = preview_width - FirstWinEndX -1;
                
                if(preview_height - FirstWinEndY <= 0)
                    ConvFirstWinStartY = 0;
                else
                    ConvFirstWinStartY = preview_height - FirstWinEndY -1;
                //printk("[%s:%d] Conv::FirstWinStartX %d, FirstWinStartY %d\n",__func__, __LINE__, ConvFirstWinStartX, ConvFirstWinStartY);

                SecondWinEndX = SecondWinStartX + AF_INNER_WINDOW_WIDTH;
                SecondWinEndY = SecondWinStartY + AF_INNER_WINDOW_HEIGHT;
                
                if(preview_width - SecondWinEndX <= 0)
                    ConvSecondWinStartX = 0;
                else
                    ConvSecondWinStartX = preview_width - SecondWinEndX -1;
                
                if(preview_height - SecondWinEndY <= 0)
                    ConvSecondWinStartY = 0;
                else
                    ConvSecondWinStartY = preview_height - SecondWinEndY -1;
                //printk("[%s:%d] Conv::ConvSecondWinStartX %d, ConvSecondWinStartY %d\n",__func__, __LINE__, ConvSecondWinStartX, ConvSecondWinStartY);

		ConvFirstWinStartX = (unsigned short)((ConvFirstWinStartX << 10) / preview_width);
		ConvFirstWinStartY = (unsigned short)((ConvFirstWinStartY << 10) / preview_height);

		//SecondWinStartX = ConvFirstWinStartX + 140;
		//SecondWinStartY = ConvFirstWinStartY + 131;                 
		ConvSecondWinStartX = (unsigned short)((ConvSecondWinStartX << 10) / preview_width);
		ConvSecondWinStartY = (unsigned short)((ConvSecondWinStartY << 10) / preview_height);

		s5k4ecgx_i2c_write_twobyte(client, 0xFCFC, 0xD000);
		s5k4ecgx_i2c_write_twobyte(client, 0x0028, 0x7000);
		s5k4ecgx_i2c_write_twobyte(client, 0x002A, 0x0294);
		s5k4ecgx_i2c_write_twobyte(client, 0x0F12, ConvFirstWinStartX); /* FstWinStartX */
		s5k4ecgx_i2c_write_twobyte(client, 0x0F12, ConvFirstWinStartY); /* FstWinStartY */
		s5k4ecgx_i2c_write_twobyte(client, 0x0F12, (320 << 10) / preview_width  ); /* FstWinSizeX : 320 */
		s5k4ecgx_i2c_write_twobyte(client, 0x0F12, (266 << 10) / preview_height );  /* FstWinSizeY : 266 */
		s5k4ecgx_i2c_write_twobyte(client, 0x0F12, ConvSecondWinStartX); /* ScndWinStartX */
		s5k4ecgx_i2c_write_twobyte(client, 0x0F12, ConvSecondWinStartY); /* ScndWinStartY */
		s5k4ecgx_i2c_write_twobyte(client, 0x0F12, (72 << 10) / preview_width  ); /* ScndWinSizeX : 72 */
		s5k4ecgx_i2c_write_twobyte(client, 0x0F12, (143 << 10) / preview_height ); /* ScndWinSizeY : 143  */
		s5k4ecgx_i2c_write_twobyte(client, 0x0F12, 0x0001); /* WinSizesUpdated */

		state->touch_ae_af_state = 1;

		/* Are we need "Touch AE Weight"? */
		/* FIXME */
	} else if(value == 2){ /* stop touch AE */
#if 0
		s5k4ecgx_i2c_write_twobyte(client, 0xFCFC, 0xD000);
		s5k4ecgx_i2c_write_twobyte(client, 0x0028, 0x7000);
		s5k4ecgx_i2c_write_twobyte(client, 0x002A, 0x395C);
		s5k4ecgx_i2c_write_twobyte(client, 0x0F12, 0x0000); /* FDA_bUseFaceAlg, Touched AE&AF support on/off */
		s5k4ecgx_i2c_write_twobyte(client, 0x0F12, 0x0000); /* FDA_bUseConfigChange, Change config */
	
		state->touch_ae_af_state = 0;
#endif        
	} else if(value == 3){ /* start touch AE */

		unsigned int aeX = 0, aeY = 0;

		aeX = state->position.x;
		aeY = state->position.y; 

		// AE Position(Round Down)
		if(aeX > AE_WINDOW_WIDTH/2) {
			aeX -= AE_WINDOW_WIDTH/2;

			if(aeX + AE_WINDOW_WIDTH > preview_width) {
				dev_err(&client->dev, "%s:[Touch AE] X Position Overflow : [%d, %d] \n", __func__, aeX, AE_WINDOW_WIDTH);
				aeX = preview_width - AE_WINDOW_WIDTH - 1;
			}
		}
		else
			aeX = 0;

		if(aeY > AE_WINDOW_HEIGHT/2) {
			aeY -= AE_WINDOW_HEIGHT/2;

			if(aeY + AE_WINDOW_HEIGHT > preview_height) {
				dev_err(&client->dev, "%s:[Touch AE] Y Position Overflow : [%d, %d] \n", __func__, aeY, AE_WINDOW_HEIGHT);
				aeY = preview_width - AE_WINDOW_HEIGHT - 1;
			}
		}
		else
			aeY = 0;
#if 0        
		if(state->touch_ae_af_state == 0) /* Default setting */
		{
			s5k4ecgx_i2c_write_twobyte(client, 0xFCFC, 0xD000);
			s5k4ecgx_i2c_write_twobyte(client, 0x0028, 0x7000);
			s5k4ecgx_i2c_write_twobyte(client, 0x002A, 0x395C);
			s5k4ecgx_i2c_write_twobyte(client, 0x0F12, 0x0001); /* FDA_bUseFaceAlg, Touched AE&AF support on/off */
			s5k4ecgx_i2c_write_twobyte(client, 0x0F12, 0x0001); /* FDA_bUseConfigChange, Change config */
			s5k4ecgx_i2c_write_twobyte(client, 0x002A, 0x3962); /* FDA_FaceArr */
			s5k4ecgx_i2c_write_twobyte(client, 0x0F12, 0x0000); /* FDA_FaceArr_0_X_Start, region start x position */
			s5k4ecgx_i2c_write_twobyte(client, 0x0F12, 0x0000); /* FDA_FaceArr_0_Y_Start, region start y position */
			s5k4ecgx_i2c_write_twobyte(client, 0x0F12, preview_width ); /* FDA_FaceArr_0_X_End, region end x position */
			s5k4ecgx_i2c_write_twobyte(client, 0x0F12, preview_height ); /* FDA_FaceArr_0_Y_End, region end y position */
			s5k4ecgx_i2c_write_twobyte(client, 0x0F12, 0x003D); /* FDA_FaceArr_0_ABR, region target brightness */
			s5k4ecgx_i2c_write_twobyte(client, 0x0F12, 0x0000); /* FDA_FaceArr_0__Weigt_Ratio, Weight ratio between region and backtround */
			s5k4ecgx_i2c_write_twobyte(client, 0x0F12, 0x0001); /* FDA_FaceArr_0__UpdateState, region change update */
			s5k4ecgx_i2c_write_twobyte(client, 0x0F12, 0x0001); /* FDA_FaceArr_0__bUpdate, use or not use*/
		}
#endif        
//		printk("[%s:%d] state->touch_ae_af_state %d\n", __func__, __LINE__, state->touch_ae_af_state);
		s5k4ecgx_i2c_write_twobyte(client, 0xFCFC, 0xD000);
		s5k4ecgx_i2c_write_twobyte(client, 0x0028, 0x7000);
		s5k4ecgx_i2c_write_twobyte(client, 0x002A, 0x3962); /* FDA_FaceArr */
		s5k4ecgx_i2c_write_twobyte(client, 0x0F12, aeX ); /* FDA_FaceArr_0_X_Start, region start x position */
		s5k4ecgx_i2c_write_twobyte(client, 0x0F12, aeY ); /* FDA_FaceArr_0_Y_Start, region start y position */
		s5k4ecgx_i2c_write_twobyte(client, 0x0F12, aeX + AE_WINDOW_WIDTH ); /* FDA_FaceArr_0_X_End, region end x position */
		s5k4ecgx_i2c_write_twobyte(client, 0x0F12, aeY + AE_WINDOW_HEIGHT ); /* FDA_FaceArr_0_Y_End, region end y position */
		s5k4ecgx_i2c_write_twobyte(client, 0x002A, 0x396C); /* FDA_FaceArr_0__Weigt_Ratio, Weight ratio between region and backtround */
		s5k4ecgx_i2c_write_twobyte(client, 0x0F12, 0x0100); /* FDA_FaceArr_0__Weigt_Ratio, Weight ratio between region and backtround */
		s5k4ecgx_i2c_write_twobyte(client, 0x0F12, 0x0001); /* FDA_FaceArr_0__UpdateState, region change update */
	}

	return 0;
}
#endif

static int s5k4ecgx_set_capture_size(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	int err;

	dev_dbg(&client->dev, "%s: index:%d\n", __func__,
		state->framesize_index);

#if 0
	printk("Capture size is %d framesize_index is %d\n", state->regs->capture_size, state->framesize_index);
#endif
	err = s5k4ecgx_set_from_table(sd, "capture_size",
				state->regs->capture_size,
				ARRAY_SIZE(state->regs->capture_size),
				state->framesize_index);
	if (err < 0) {
		dev_err(&client->dev,
			"%s: failed: i2c_write for capture_resolution\n",
			__func__);
	}
	state->runmode = S5K4ECGX_RUNMODE_CAPTURE;

	return err;
}

static int s5k4ecgx_set_jpeg_quality(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);

	dev_dbg(&client->dev,
		"%s: jpeg.quality %d\n", __func__, state->jpeg.quality);

	switch(state->jpeg.quality) 
	{
		case JPEG_QUALITY_SUPERFINE:
		{
    		return s5k4ecgx_set_from_table(sd, "jpeg quality high",
    				&state->regs->jpeg_quality_high, 1, 0);
        }

		case JPEG_QUALITY_FINE:
		{
    		return s5k4ecgx_set_from_table(sd, "jpeg quality normal",
    				&state->regs->jpeg_quality_normal, 1, 0);
        }
        
		case JPEG_QUALITY_NORMAL:
		{
    		return s5k4ecgx_set_from_table(sd, "jpeg quality low",
    				&state->regs->jpeg_quality_low, 1, 0);
        }

        default:
        {
    		return s5k4ecgx_set_from_table(sd, "jpeg quality high",
    				&state->regs->jpeg_quality_high, 1, 0);
        }
    }
}

static u16 s5k4ecgx_get_light_level(struct v4l2_subdev *sd)
{
	int err;
	u16 read_value1 = 0;
	u16 read_value2 = 0;
	int read_value;

	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);

	err = s5k4ecgx_set_from_table(sd, "get light level",
				&state->regs->get_light_level, 1, 0);
	if (err) {
		dev_err(&client->dev,
			"%s: write cmd failed, returning 0\n", __func__);
		goto out;
	}
	err |= s5k4ecgx_i2c_read_twobyte(client, 0x0F12, &read_value1);
	err |= s5k4ecgx_i2c_read_twobyte(client, 0x0F12, &read_value2);
       read_value = (read_value2<<16)|(read_value1&0xFFFF);
       
	if (err) {
		dev_err(&client->dev,
			"%s: read cmd failed, returning 0\n", __func__);
		goto out;
	}

	dev_dbg(&client->dev, "%s: read_value = %d (0x%X)\n",
		__func__, read_value, read_value);

out:
	/* restore write mode */
	s5k4ecgx_i2c_write_twobyte(client, 0x0028, 0x7000);
	return read_value;
}

static int s5k4ecgx_start_capture(struct v4l2_subdev *sd)
{
	int err;
	u16 read_value;
	u16 light_level;
	int poll_time_ms;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct sec_cam_parm *parms =
		(struct sec_cam_parm *)&state->strm.parm.raw_data;
	struct s5k4ecgx_platform_data *pdata = client->dev.platform_data;

       //dev_err(&client->dev,"%s",__func__);
       
	light_level = s5k4ecgx_get_light_level(sd);

	dev_dbg(&client->dev, "%s: light_level = %d\n", __func__,
		light_level);
	state->flash_state_on_previous_capture = false;
	if (parms->scene_mode != SCENE_MODE_NIGHTSHOT) {
		switch (parms->flash_mode) {
		case FLASH_MODE_AUTO:
			if (light_level > LOW_LIGHT_LEVEL) {
				/* light level bright enough
				 * that we don't need flash
				 */
				break;
			}
			/* fall through to flash start */
		case FLASH_MODE_ON:
			if (parms->focus_mode == FOCUS_MODE_INFINITY) {
				s5k4ecgx_set_from_table(sd,
					"AF assist flash start",
					&state->regs->af_assist_flash_start,
					1, 0);
				s5k4ecgx_set_from_table(sd,
					"AF assist flash end",
					&state->regs->af_assist_flash_end,
					1, 0);
				msleep(10);
			}
			s5k4ecgx_set_from_table(sd, "flash start",
					&state->regs->flash_start, 1, 0);
			state->flash_on = true;
			state->flash_state_on_previous_capture = true;
			pdata->flash_onoff(1);
			break;
		default:
			break;
		}
	}

	/* if light is low, use low light capture settings, EXCEPT
	 * if scene mode set to NIGHTSHOT or SPORTS because they
	 * have their own settings (though a low light sport setting
	 * could be useful)
	 */

			if ((light_level <= HIGH_LIGHT_LEVEL) &&
		(parms->scene_mode != SCENE_MODE_NIGHTSHOT) &&
				(parms->scene_mode != SCENE_MODE_SPORTS)) {
				s5k4ecgx_set_from_table(sd, "low cap on",
							&state->regs->low_cap_on, 1, 0);
			}


#ifdef NEW_CAMERA
	/* Nothing needed */
#else
	/* NIGHT Scene capture setting */
	/* Night shot has no concerned Low Light status */
	if( parms->scene_mode == SCENE_MODE_NIGHTSHOT )
	{
		s5k4ecgx_set_from_table(sd, "night cap",
					&state->regs->night_cap, 1, 0);
	}


	/* BEACH, SNOW, FIRE capture setting */
	if ( (light_level > HIGH_LIGHT_LEVEL) && 
			(parms->scene_mode == SCENE_MODE_BEACH_SNOW || 
			 parms->scene_mode == SCENE_MODE_FIREWORKS) )
	{
		s5k4ecgx_set_from_table(sd, "beach snow fire cap",
					&state->regs->beach_snow_fire_cap, 1, 0);
		}
#endif

	err = s5k4ecgx_set_capture_size(sd);
	if (err < 0) {
		dev_err(&client->dev,
			"%s: failed: i2c_write for capture_resolution\n",
			__func__);
		return -EIO;
	}

	dev_dbg(&client->dev, "%s: send Capture_Start cmd\n", __func__);
	s5k4ecgx_set_from_table(sd, "capture start",
				&state->regs->capture_start, 1, 0);

	/* a shot takes takes at least 50ms so sleep that amount first
	 * and then start polling for completion.
	 */
	if ((parms->scene_mode != SCENE_MODE_NIGHTSHOT) && (state->flash_on))
	{
		msleep(300);
        pdata->flash_onoff(0);
	}
	else
	{
		msleep(50);
		/* Enter read mode */
		s5k4ecgx_i2c_write_twobyte(client, 0x002C, 0x7000);
		poll_time_ms = 50;
		do {
			s5k4ecgx_set_from_table(sd, "get capture status",
						&state->regs->get_capture_status, 1, 0);
			s5k4ecgx_i2c_read_twobyte(client, 0x0F12, &read_value);
			dev_dbg(&client->dev,
				"%s: s5k4ecgx_Capture_Start check = %#x\n",
				__func__, read_value);
			if (read_value != 0x00)
				break;
			msleep(POLL_TIME_MS);
			poll_time_ms += POLL_TIME_MS;
		} while (poll_time_ms < CAPTURE_POLL_TIME_MS);

		dev_dbg(&client->dev, "%s: capture done check finished after %d ms\n",
			__func__, poll_time_ms);

		/* restore write mode */
		s5k4ecgx_i2c_write_twobyte(client, 0x0028, 0x7000);

		if ((light_level <= HIGH_LIGHT_LEVEL) &&
			(parms->scene_mode != SCENE_MODE_NIGHTSHOT) &&
			(parms->scene_mode != SCENE_MODE_SPORTS)) {
			s5k4ecgx_set_from_table(sd, "low cap off",
						&state->regs->low_cap_off, 1, 0);
		}
	}

	if ((parms->scene_mode != SCENE_MODE_NIGHTSHOT) && (state->flash_on)) {
		state->flash_on = false;
		pdata->flash_onoff(0);
		s5k4ecgx_set_from_table(sd, "flash end",
					&state->regs->flash_end, 1, 0);
	}

       if((state->flash_on == false)&&(state->af_preflash_on == true))
       {
           state->af_preflash_on = false;
           s5k4ecgx_set_from_table(sd, "flash end",
                       &state->regs->flash_end, 1, 0);
       }
	return 0;
}

static int s5k4ecgx_set_softlanding(struct v4l2_subdev *sd)
{
       struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct sec_cam_parm *parms =
		(struct sec_cam_parm *)&state->strm.parm.raw_data;    
        int err = 0;
        dev_dbg(&client->dev, "%s : focus_mode = %d\n", __func__,parms->focus_mode);
#if 0        
        err |=  s5k4ecgx_set_from_table(sd, "softlanding",
                    &state->regs->set_softlanding,1,0);
        if(err < 0)
        {
            dev_err(&client->dev,"%s: Not support 'softlanding' value\n",__func__);
        }
        
        if(parms->focus_mode = FOCUS_MODE_MACRO)
            msleep(200);
        else        
            msleep(100);    // skip 1 frame.
#endif
        err |= s5k4ecgx_set_from_table(sd,"af normal mode 1",
        	&state->regs->af_normal_mode_1, 1, 0);
        if(err < 0)
        {
            dev_err(&client->dev,"%s: Not support 'softlanding' value\n",__func__);
        }

        err |= s5k4ecgx_set_from_table(sd,"af normal mode 2",
        	&state->regs->af_normal_mode_2, 1, 0);
        if(err < 0)
        {
            dev_err(&client->dev,"%s: Not support 'softlanding' value\n",__func__);
        }

        if(parms->focus_mode = FOCUS_MODE_MACRO)
            msleep(550);
        else
            msleep(300);
        
        return err;
}

/* wide dynamic range support */
static int s5k4ecgx_set_wdr(struct v4l2_subdev *sd, int value)
{
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);

	if (value == WDR_ON)
		return s5k4ecgx_set_from_table(sd, "wdr on",
					&state->regs->wdr_on, 1, 0);
	return s5k4ecgx_set_from_table(sd, "wdr off",
				&state->regs->wdr_off, 1, 0);
}

static int s5k4ecgx_set_auto_contrast(struct v4l2_subdev *sd, int onoff)
{
       struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct sec_cam_parm *parms =
		(struct sec_cam_parm *)&state->strm.parm.raw_data;    
        int err = 0;
        dev_err(&client->dev, "%s : value = %d", __func__,onoff);

        if (onoff == 1){
            // default adjust setting
            err |=  s5k4ecgx_set_from_table(sd, "contrast",
                        &state->regs->contrast,
                        ARRAY_SIZE(state->regs->contrast), 
                        CONTRAST_DEFAULT);
            if(err < 0)
            {
                dev_err(&client->dev,"%s:Not support 'contrast' value %d\n",__func__, CONTRAST_DEFAULT);
            }

            err |=  s5k4ecgx_set_from_table(sd, "saturation",
                        &state->regs->saturation,
                        ARRAY_SIZE(state->regs->saturation), 
                        SATURATION_DEFAULT);
            if(err < 0)
            {
                dev_err(&client->dev,"%s: Not support 'saturation' value %d\n",__func__, SATURATION_DEFAULT);
            }

            err |=  s5k4ecgx_set_from_table(sd, "sharpness",
                        &state->regs->sharpness,
                        ARRAY_SIZE(state->regs->sharpness), 
                        SHARPNESS_DEFAULT);
            if(err < 0)
            {
                dev_err(&client->dev,"%s: Not support 'sharpness' value %d\n",__func__, SHARPNESS_DEFAULT);
            }
            // ON
            err |=  s5k4ecgx_set_from_table(sd, "auto contrast on",
                        &state->regs->set_auto_contrast_on, 1, 0);
            if(err < 0)
            {
                dev_err(&client->dev,"%s:Not support 'auto contrast on' value %d\n",__func__,onoff);
            }
        }else{ 
            err |=  s5k4ecgx_set_from_table(sd, "contrast",
                        &state->regs->contrast,
                        ARRAY_SIZE(state->regs->contrast), 
                        parms->contrast);
            if(err < 0)
            {
                dev_err(&client->dev,"%s:Not support 'contrast' value %d\n",__func__, CONTRAST_DEFAULT);
            }

            err |=  s5k4ecgx_set_from_table(sd, "saturation",
                        &state->regs->saturation,
                        ARRAY_SIZE(state->regs->saturation), 
                        parms->saturation);
            if(err < 0)
            {
                dev_err(&client->dev,"%s: Not support 'saturation' value %d\n",__func__, SATURATION_DEFAULT);
            }

            err |=  s5k4ecgx_set_from_table(sd, "sharpness",
                        &state->regs->sharpness,
                        ARRAY_SIZE(state->regs->sharpness), 
                        parms->sharpness);
            if(err < 0)
            {
                dev_err(&client->dev,"%s: Not support 'sharpness' value %d\n",__func__, SHARPNESS_DEFAULT);
            }
           
            // OFF
            err |=  s5k4ecgx_set_from_table(sd, "auto contrast off",
                        &state->regs->set_auto_contrast_off, 1, 0);
            if(err < 0)
            {
                dev_err(&client->dev,"%s:Not support 'auto contrast off' value %d\n",__func__,onoff);
            }
        }
        return err;
}

static int s5k4ecgx_set_face_detection(struct v4l2_subdev *sd, int value)
{
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);

	if (value == FACE_DETECTION_ON)
		return s5k4ecgx_set_from_table(sd, "face detection on",
				&state->regs->face_detection_on, 1, 0);
	return s5k4ecgx_set_from_table(sd, "face detection off",
				&state->regs->face_detection_off, 1, 0);
}


#if 1 /* For fix 30fps */
static int s5k4ecgx_set_sensor_mode(struct v4l2_subdev *sd, int value)
{
	//struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
			container_of(sd, struct s5k4ecgx_state, sd);
	//struct sec_cam_parm *parms =	(struct sec_cam_parm *)&state->strm.parm.raw_data;
	int err;

	if(value == 1) /* 1 MOVIE MODE */
	{
//		dev_dbg("[%s:%d] Set fix fpx 30 for MOVIE MODE\n", __func__, __LINE__);
		err = s5k4ecgx_set_from_table(sd, "fix 30fps",
				&state->regs->set_fix_fps, 1, 0);
	}
	else
	{
//		dev_dbg("[%s:%d] Set Camera Mode, fps variation\n", __func__, __LINE__);
	}
	return 0;
}
#endif

static int s5k4ecgx_set_zoom(struct v4l2_subdev *sd, int zoom_level)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state = container_of(sd, struct s5k4ecgx_state, sd);
	struct sec_cam_parm *parms = (struct sec_cam_parm *)&state->strm.parm.raw_data;
	int err=0;

	dev_dbg(&client->dev,"%s: zoom_level = %d, snapshot_width = %d\n",
		 __func__,zoom_level, state->snapshot_width);

	if(camcorder_check_flag)
	{
		if(state->pix.width == 720)
		{
			state->zoom_ratio = 177;
			err = s5k4ecgx_set_from_table(sd, "x1_77 Zoom",
				state->regs->set_x1_77_zoom,
				ARRAY_SIZE(state->regs->set_x1_77_zoom),
				zoom_level);
		}
		else
		{
			state->zoom_ratio = 200;
			err = s5k4ecgx_set_from_table(sd, "x2 Zoom",
				state->regs->set_x2_zoom,
				ARRAY_SIZE(state->regs->set_x2_zoom),
				zoom_level);	
		}
		
	}
	else
	{
		switch(state->snapshot_width)
		{
			case 2560:
				if(state->pix.width == 640){
					state->zoom_ratio = 200;
					if(zoom_level == 0){
						err = s5k4ecgx_set_from_table(sd, "x2 Zoom",
							state->regs->set_x2_zoom,
							ARRAY_SIZE(state->regs->set_x2_zoom),
							zoom_level);
					}
				}else{
					state->zoom_ratio = 160;
					if(zoom_level == 0){
						err = s5k4ecgx_set_from_table(sd, "x1_6 Zoom",
							state->regs->set_x1_6_zoom,
							ARRAY_SIZE(state->regs->set_x1_6_zoom),
							zoom_level);
					}
				}
				break;
				
			case 2048:
				state->zoom_ratio = 125;
				err = s5k4ecgx_set_from_table(sd, "x1_25 Zoom",
					state->regs->set_x1_25_zoom,
					ARRAY_SIZE(state->regs->set_x1_25_zoom),
					zoom_level);
				break;
				
			case 1600:
				state->zoom_ratio = 160;
				err = s5k4ecgx_set_from_table(sd, "x1_6 Zoom",
					state->regs->set_x1_6_zoom,
					ARRAY_SIZE(state->regs->set_x1_6_zoom),
					zoom_level);			
				break;
				
			case 1280:
			case 800:
			case 640:
				if(state->pix.width == 640){
					state->zoom_ratio = 200;
					err = s5k4ecgx_set_from_table(sd, "x2 Zoom",
						state->regs->set_x2_zoom,
						ARRAY_SIZE(state->regs->set_x2_zoom),
						zoom_level);			
				}else{
					state->zoom_ratio = 160;
					err = s5k4ecgx_set_from_table(sd, "x1_6 Zoom",
						state->regs->set_x1_6_zoom,
						ARRAY_SIZE(state->regs->set_x1_6_zoom),
						zoom_level);
				}
				break;

			case 720:
				state->zoom_ratio = 177;
				err = s5k4ecgx_set_from_table(sd, "x1_77 Zoom",
					state->regs->set_x1_77_zoom,
					ARRAY_SIZE(state->regs->set_x1_77_zoom),
					zoom_level);			
				break;

			case 320:
			case 176:
				state->zoom_ratio = 200;
				err = s5k4ecgx_set_from_table(sd, "x2 Zoom",
					state->regs->set_x2_zoom,
					ARRAY_SIZE(state->regs->set_x2_zoom),
					zoom_level);			
				break;
			
			default:
				break;
		} 
	}

	state->zoom_level = zoom_level;
	return 0;
}

static int s5k4ecgx_AE_AWB_lock(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    struct s5k4ecgx_state *state = container_of(sd, struct s5k4ecgx_state, sd);
	struct sec_cam_parm *parms = (struct sec_cam_parm *)&state->strm.parm.raw_data;
    int err = 0;
    
//    CAM_INFO_MSG(&client->dev, "%s: setting value =%d\n", __func__, ctrl->value);

    // Lock, Unlock only AE for LSI 5CC sensor. Don't change AWB.
	switch(ctrl->value) 
	{
		case AE_UNLOCK_AWB_UNLOCK:
		{
		    // AE Unlock
        	s5k4ecgx_set_from_table(sd, "ae unlock", &state->regs->ae_unlock, 1, 0);

            // AWB Unlock
        	if( parms->white_balance == WHITE_BALANCE_AUTO )
        	{
        		s5k4ecgx_set_from_table(sd, "awb unlock", &state->regs->awb_unlock, 1, 0);
        	}
            break;
        }

		case AE_LOCK_AWB_UNLOCK:
		{
		    // AE Lock
        	s5k4ecgx_set_from_table(sd, "ae lock", &state->regs->ae_lock, 1, 0);

            // AWB Unlock
        	if(parms->white_balance == WHITE_BALANCE_AUTO)
        	{
        		s5k4ecgx_set_from_table(sd, "awb unlock", &state->regs->awb_unlock, 1, 0);
        	}
            break;
        }

		case AE_UNLOCK_AWB_LOCK:
		{
		    // AE UnLock
        	s5k4ecgx_set_from_table(sd, "ae unlock", &state->regs->ae_unlock, 1, 0);
            
            // AWB lock
        	if(parms->white_balance == WHITE_BALANCE_AUTO)
        	{
        		s5k4ecgx_set_from_table(sd, "awb lock", &state->regs->awb_lock, 1, 0);
        	}
            break;
        }

		case AE_LOCK_AWB_LOCK:
		{
		    // AE Lock
        	s5k4ecgx_set_from_table(sd, "ae lock", &state->regs->ae_lock, 1, 0);

            // AWB lock
        	if(parms->white_balance == WHITE_BALANCE_AUTO)
        	{
        		s5k4ecgx_set_from_table(sd, "awb lock",	&state->regs->awb_lock, 1, 0);
        	}
            break;
        }

        default:
        {
//            CAM_WARN_MSG(&client->dev, "[%s : %d] WARNING! Unsupported AE, AWB lock setting(%d)\n", __FILE__, __LINE__, ctrl->value);
            break;			
        }			
    }

    if(err < 0)
    {
//        CAM_ERROR_MSG(&client->dev, "[%s : %d] ERROR! AE, AWB lock failed\n", __FILE__, __LINE__);
        return -EIO;
    }
	
    return 0;
}

static int s5k4ecgx_set_focus_mode(struct v4l2_subdev *sd, int value)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct sec_cam_parm *parms =
		(struct sec_cam_parm *)&state->strm.parm.raw_data;
	int err;

	dev_dbg(&client->dev, "%s value(%d)\n", __func__, value);

	switch (value) {
	case FOCUS_MODE_MACRO:
		dev_dbg(&client->dev,
				"%s: FOCUS_MODE_MACRO\n", __func__);
		err = s5k4ecgx_set_from_table(sd, "af macro mode 1",
				&state->regs->af_macro_mode_1, 1, 0);
		if (err < 0)
			goto fail;

		err = s5k4ecgx_set_from_table(sd, "af macro mode 2",
				&state->regs->af_macro_mode_2, 1, 0);
		if (err < 0)
			goto fail;

		err = s5k4ecgx_set_from_table(sd, "af macro mode 3",
				&state->regs->af_macro_mode_3, 1, 0);
		if (err < 0)
			goto fail;
		parms->focus_mode = FOCUS_MODE_MACRO;
		break;

	case FOCUS_MODE_INFINITY:
		err = s5k4ecgx_set_from_table(sd,
			"af return infinite position",
			&state->regs->af_return_infinite_position, 1, 0);
		if (err < 0)
			goto fail;
		parms->focus_mode = FOCUS_MODE_INFINITY;
		break;
		
#ifdef NEW_CAMERA
	case FOCUS_MODE_POWEROFF:
		dev_dbg(&client->dev,
			"%s: AF POWER OFF function for noise reduction\n", __func__);
		err = s5k4ecgx_set_from_table(sd,
			"af normal mode 1",
			&state->regs->af_normal_mode_1, 1, 0);
		if (err < 0)
			goto fail;
		
		err = s5k4ecgx_set_from_table(sd,
			"af normal mode 2",
			&state->regs->af_normal_mode_2, 1, 0);
		if (err < 0)
			goto fail;
		break;
#endif

	case FOCUS_MODE_AUTO:
	default:
		dev_dbg(&client->dev,
			"%s: FOCUS_MODE_AUTO\n", __func__);
		err = s5k4ecgx_set_from_table(sd,
			"af normal mode 1",
			&state->regs->af_normal_mode_1, 1, 0);
		if (err < 0)
			goto fail;

		err = s5k4ecgx_set_from_table(sd,
			"af normal mode 2",
			&state->regs->af_normal_mode_2, 1, 0);
		if (err < 0)
			goto fail;

		err = s5k4ecgx_set_from_table(sd,
			"af normal mode 3",
			&state->regs->af_normal_mode_3, 1, 0);
		if (err < 0)
			goto fail;
		parms->focus_mode = FOCUS_MODE_AUTO;
		break;
	}

	return 0;
fail:
	dev_err(&client->dev,
		"%s: i2c_write failed\n", __func__);
	return -EIO;
}

static void s5k4ecgx_auto_focus_flash_start(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct s5k4ecgx_platform_data *pdata = client->dev.platform_data;
	int count;
	u16 read_value;

	s5k4ecgx_set_from_table(sd, "AF assist flash start",
				&state->regs->af_assist_flash_start, 1, 0);
	state->flash_on = true;
       state->af_preflash_on = true;
       
	pdata->af_assist_onoff(1);

	/* delay 200ms (SLSI value) and then poll to see if AE is stable.
	 * once it is stable, lock it and then return to do AF
	 */
	mdelay(200);

	/* enter read mode */
	s5k4ecgx_i2c_write_twobyte(client, 0x002C, 0x7000);
	for (count = 0; count < AE_STABLE_SEARCH_COUNT; count++) {
		if (state->af_status == AF_CANCEL)
			break;
		s5k4ecgx_set_from_table(sd, "get ae stable status",
				&state->regs->get_ae_stable_status, 1, 0);
		s5k4ecgx_i2c_read_twobyte(client, 0x0F12, &read_value);
		dev_dbg(&client->dev, "%s: ae stable status = %#x\n",
			__func__, read_value);
		if (read_value == 0x1)
			break;
		msleep(state->one_frame_delay_ms);
	}

	/* restore write mode */
	s5k4ecgx_i2c_write_twobyte(client, 0x0028, 0x7000);

	/* if we were cancelled, turn off flash */
	if (state->af_status == AF_CANCEL) {
		dev_dbg(&client->dev,
			"%s: AF cancelled\n", __func__);
		s5k4ecgx_set_from_table(sd, "AF assist flash end",
				&state->regs->af_assist_flash_end, 1, 0);
		state->flash_on = false;
		pdata->af_assist_onoff(0);
	}
}

static int s5k4ecgx_start_auto_focus(struct v4l2_subdev *sd)
{
        int light_level;
        struct i2c_client *client = v4l2_get_subdevdata(sd);
        struct s5k4ecgx_state *state =
        	container_of(sd, struct s5k4ecgx_state, sd);
        struct sec_cam_parm *parms =
        	(struct sec_cam_parm *)&state->strm.parm.raw_data;
        dev_dbg(&client->dev, "%s: start SINGLE AF operation, flash mode %d\n",
        	__func__, parms->flash_mode);

        // Initialize fine search value.
        bStartFineSearch = false;        
        s5k4ecgx_set_from_table(sd, "single af start",
        			&state->regs->single_af_start, 1, 0);
        state->af_status = AF_START;
        return 0;
}

static int s5k4ecgx_stop_auto_focus(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct sec_cam_parm *parms =
		(struct sec_cam_parm *)&state->strm.parm.raw_data;
	int focus_mode = parms->focus_mode;

	dev_dbg(&client->dev, "%s: single AF Off command Setting\n", __func__);

	/* auto focus was in progress.  the other thread
	 * is either in the middle of get_auto_focus_result()
	 * or will call it shortly.  set a flag to have
	 * it abort it's polling.  that thread will
	 * also do cleanup like restore focus position.
	 *
	 * it might be enough to just send sensor commands
	 * to abort auto focus and the other thread would get
	 * that state from it's polling calls, but I'm not sure.
	 */
	state->af_status = AF_CANCEL;
	dev_dbg(&client->dev,
		"%s: sending Single_AF_Off commands to sensor\n", __func__);
	s5k4ecgx_set_from_table(sd, "single af off 1",
				&state->regs->single_af_off_1, 1, 0);

	s5k4ecgx_set_from_table(sd, "single af off 2",
				&state->regs->single_af_off_2, 1, 0);
	return 0;
}

/* called by HAL after auto focus was started to get the result.
 * it might be aborted asynchronously by a call to set_auto_focus
 */
static int s5k4ecgx_get_auto_focus_result(struct v4l2_subdev *sd,
					struct v4l2_control *ctrl)
{
        struct i2c_client *client = v4l2_get_subdevdata(sd);
        struct s5k4ecgx_state *state =
        container_of(sd, struct s5k4ecgx_state, sd);
        int err = 0, count;
        u16 AF_status;

        if(!bStartFineSearch){
            err  = s5k4ecgx_i2c_write_twobyte(client, 0xFCFC, 0xD000);
            err += s5k4ecgx_i2c_write_twobyte(client, 0x002C, 0x7000);
            err += s5k4ecgx_i2c_write_twobyte(client, 0x002E, 0x2EEE);
            err += s5k4ecgx_i2c_read_twobyte(client, 0x0F12, &AF_status);

            if(err < 0){
                dev_err(&client->dev, "%s: AF is Failure~~~~~~~(I2C Failed) \n", __func__);
                ctrl->value = CAMERA_AF_STATUS_FAIL;
                goto routine_end;
            }

            if(AF_status & 0x0001){   // Check if AF is in progress
                ctrl->value = CAMERA_AF_STATUS_IN_PROGRESS;
            }else{
                if(AF_status & 0x0002){
                    dev_err(&client->dev, "%s: AF is success~~~~~~~(Single Search) \n", __func__);

#if 1 // Use Fine Search Algorithm.
                    ctrl->value = CAMERA_AF_STATUS_1ST_SUCCESS; // fine search algorithm.
                    bStartFineSearch = true;
#else
                    ctrl->value = CAMERA_AF_STATUS_SUCCESS; // single search algorithm.
#endif
                }else{
                    dev_err(&client->dev, "%s: AF is Failure~~~~~~~(Single Search) \n", __func__);
                    ctrl->value = CAMERA_AF_STATUS_FAIL;
                }
            }
        }        else{ // Fine Search
            err  = s5k4ecgx_i2c_write_twobyte(client, 0xFCFC, 0xD000);
            err += s5k4ecgx_i2c_write_twobyte(client, 0x002C, 0x7000);
            err += s5k4ecgx_i2c_write_twobyte(client, 0x002E, 0x2207);
            err += s5k4ecgx_i2c_read_twobyte(client, 0x0F12, &AF_status);

            if(err < 0){
                dev_err(&client->dev, "%s: AF is Failure~~~~~~~(I2C Failed) \n", __func__);

                ctrl->value = CAMERA_AF_STATUS_FAIL;
                goto routine_end;
            }

            if((AF_status & 0xFF00) == 0x0000){
                dev_err(&client->dev, "%s: AF is success~~~~~~~(Fine Search) \n", __func__);

                ctrl->value = CAMERA_AF_STATUS_SUCCESS;
            }else{
                ctrl->value = CAMERA_AF_STATUS_1ST_SUCCESS;
            }
        }

        routine_end:
        return 0;    
}


static void s5k4ecgx_init_parameters(struct v4l2_subdev *sd)
{
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct sec_cam_parm *parms =
		(struct sec_cam_parm *)&state->strm.parm.raw_data;

	state->strm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	parms->capture.capturemode = 0;
	parms->capture.timeperframe.numerator = 1;
	parms->capture.timeperframe.denominator = 30;
	parms->contrast = CONTRAST_DEFAULT;
	parms->effects = IMAGE_EFFECT_NONE;
	parms->brightness = EV_DEFAULT;
	parms->flash_mode = FLASH_MODE_AUTO;
	parms->focus_mode = FOCUS_MODE_AUTO;
	parms->iso = ISO_AUTO;
	parms->metering = METERING_CENTER;
	parms->saturation = SATURATION_DEFAULT;
	parms->scene_mode = SCENE_MODE_NONE;
	parms->sharpness = SHARPNESS_DEFAULT;
	parms->white_balance = WHITE_BALANCE_AUTO;
	parms->sensor_mode = 0;

	state->jpeg.enable = 0;
	state->jpeg.quality = 100;
	state->jpeg.main_offset = 0;
	state->jpeg.main_size = 0;
	state->jpeg.thumb_offset = 0;
	state->jpeg.thumb_size = 0;
	state->jpeg.postview_offset = 0;

	state->fw.major = 1;

	state->one_frame_delay_ms = NORMAL_MODE_MAX_ONE_FRAME_DELAY_MS;
}

static void s5k4ecgx_set_framesize(struct v4l2_subdev *sd,
				const struct s5k4ecgx_framesize *frmsize,
				int frmsize_count, bool exact_match);
static int s5k4ecgx_s_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	dev_dbg(&client->dev, "%s: pixelformat = 0x%x (%c%c%c%c),"
		" colorspace = 0x%x, width = %d, height = %d\n",
		__func__, fmt->fmt.pix.pixelformat,
		fmt->fmt.pix.pixelformat,
		fmt->fmt.pix.pixelformat >> 8,
		fmt->fmt.pix.pixelformat >> 16,
		fmt->fmt.pix.pixelformat >> 24,
		fmt->fmt.pix.colorspace,
		fmt->fmt.pix.width, fmt->fmt.pix.height);

	if (fmt->fmt.pix.pixelformat == V4L2_PIX_FMT_JPEG &&
		fmt->fmt.pix.colorspace != V4L2_COLORSPACE_JPEG) {
		dev_err(&client->dev,
			"%s: mismatch in pixelformat and colorspace\n",
			__func__);
		return -EINVAL;
	}

	state->pix.width = fmt->fmt.pix.width;
	state->pix.height = fmt->fmt.pix.height;
	state->pix.pixelformat = fmt->fmt.pix.pixelformat;

	if (fmt->fmt.pix.colorspace == V4L2_COLORSPACE_JPEG) {
		state->oprmode = S5K4ECGX_OPRMODE_IMAGE;
		/*
		 * In case of image capture mode,
		 * if the given image resolution is not supported,
		 * use the next higher image resolution. */
		s5k4ecgx_set_framesize(sd, s5k4ecgx_capture_framesize_list,
				ARRAY_SIZE(s5k4ecgx_capture_framesize_list),
				false);

	} else {
		state->oprmode = S5K4ECGX_OPRMODE_VIDEO;
		/*
		 * In case of video mode,
		 * if the given video resolution is not matching, use
		 * the default rate (currently S5K4ECGX_PREVIEW_WVGA).
		 */
#ifdef NEW_CAMERA
		s5k4ecgx_set_framesize(sd, s5k4ecgx_camcorder_preview_framesize_list,
				ARRAY_SIZE(s5k4ecgx_camcorder_preview_framesize_list),
				true);
#else
		s5k4ecgx_set_framesize(sd, s5k4ecgx_preview_framesize_list,
				ARRAY_SIZE(s5k4ecgx_preview_framesize_list),
				true);
#endif
	}

	state->jpeg.enable = state->pix.pixelformat == V4L2_PIX_FMT_JPEG;

	return 0;
}

static int s5k4ecgx_enum_framesizes(struct v4l2_subdev *sd,
				  struct v4l2_frmsizeenum *fsize)
{
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);

	/* The camera interface should read this value, this is the resolution
	 * at which the sensor would provide framedata to the camera i/f
	 *
	 * In case of image capture,
	 * this returns the default camera resolution (SVGA)
	 */
	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
	fsize->discrete.width = state->pix.width;
	fsize->discrete.height = state->pix.height;
	return 0;
}

static int s5k4ecgx_enum_fmt(struct v4l2_subdev *sd,
			struct v4l2_fmtdesc *fmtdesc)
{
	pr_debug("%s: index = %d\n", __func__, fmtdesc->index);

	if (fmtdesc->index >= ARRAY_SIZE(capture_fmts))
		return -EINVAL;

	memcpy(fmtdesc, &capture_fmts[fmtdesc->index], sizeof(*fmtdesc));

	return 0;
}

static int s5k4ecgx_try_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
	int num_entries;
	int i;

	num_entries = ARRAY_SIZE(capture_fmts);

	pr_debug("%s: pixelformat = 0x%x (%c%c%c%c), num_entries = %d\n",
		__func__, fmt->fmt.pix.pixelformat,
		fmt->fmt.pix.pixelformat,
		fmt->fmt.pix.pixelformat >> 8,
		fmt->fmt.pix.pixelformat >> 16,
		fmt->fmt.pix.pixelformat >> 24,
		num_entries);

	for (i = 0; i < num_entries; i++) {
		if (capture_fmts[i].pixelformat == fmt->fmt.pix.pixelformat) {
			pr_debug("%s: match found, returning 0\n", __func__);
			return 0;
		}
	}

	pr_debug("%s: no match found, returning -EINVAL\n", __func__);
	return -EINVAL;
}

static void s5k4ecgx_enable_torch(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct s5k4ecgx_platform_data *pdata = client->dev.platform_data;

	s5k4ecgx_set_from_table(sd, "flash start",
				&state->regs->flash_start, 1, 0);
	state->flash_on = true;
	pdata->torch_onoff(1);
}

static void s5k4ecgx_disable_torch(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct s5k4ecgx_platform_data *pdata = client->dev.platform_data;

	if (state->flash_on) {
		state->flash_on = false;
		pdata->torch_onoff(0);
		s5k4ecgx_set_from_table(sd, "flash end",
					&state->regs->flash_end, 1, 0);
	}
}
static int s5k4ecgx_set_af_preflash_mode(struct v4l2_subdev *sd, int onoff)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct s5k4ecgx_state *state =
        container_of(sd, struct s5k4ecgx_state, sd);
    struct s5k4ecgx_platform_data *pdata = client->dev.platform_data;
    struct sec_cam_parm *parms =
        (struct sec_cam_parm *)&state->strm.parm.raw_data;

    pr_debug("%s = %d",__func__,onoff);
    
    if((parms->flash_mode == FLASH_MODE_AUTO)&&(onoff != 0)){
        if(s5k4ecgx_get_light_level(sd) > LOW_LIGHT_LEVEL)
            return 0;
    }
    
    state->af_preflash_on = true;
        
    if(onoff){
        s5k4ecgx_set_from_table(sd, "AF assist flash start",
                    &state->regs->af_assist_flash_start, 1, 0);
        state->flash_on = true;
        pdata->af_assist_onoff(1);
    }else{
        s5k4ecgx_set_from_table(sd, "AF assist flash end",
            &state->regs->af_assist_flash_end, 1, 0);
        state->flash_on = false;
        pdata->af_assist_onoff(0);    
	    msleep(300);
    }
    
    return 0;
}

static int s5k4ecgx_set_flash_mode(struct v4l2_subdev *sd, int value)
{
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct sec_cam_parm *parms =
		(struct sec_cam_parm *)&state->strm.parm.raw_data;

	if (parms->flash_mode == value)
		return 0;

	if ((value >= FLASH_MODE_OFF) && (value <= FLASH_MODE_BACKLIGHT_ON)) {
		pr_debug("%s: setting flash mode to %d\n",
			__func__, value);
		parms->flash_mode = value;
		if (parms->flash_mode == FLASH_MODE_TORCH_ON)
			s5k4ecgx_enable_torch(sd);
		else
			s5k4ecgx_disable_torch(sd);
		return 0;
	}
	pr_debug("%s: trying to set invalid flash mode %d\n",
		__func__, value);
	return -EINVAL;
}

static int s5k4ecgx_set_all_parameter(struct v4l2_subdev *sd,
                    struct v4l2_streamparm *param)
{
	int err = 0;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct sec_cam_parm *new_parms =
		(struct sec_cam_parm *)&param->parm.raw_data;
	struct sec_cam_parm *parms =
		(struct sec_cam_parm *)&state->strm.parm.raw_data;

        /* we return an error if one happened but don't stop trying to
         * set all parameters passed
         
         */
#if 0        
        //FOCUS
        err |= s5k4ecgx_set_focus_mode(sd, new_parms->focus_mode);
        if(err < 0)
        {
            dev_err(&client->dev,"%s: Not support 'focus_mode' value %d\n",__func__, new_parms->focus_mode);
        }         
#endif
        //FLASH
        err |= s5k4ecgx_set_flash_mode(sd, new_parms->flash_mode);
        dev_dbg(&client->dev,"'flash_mode' value %d\n",new_parms->flash_mode);
        if(err < 0)
        {
            dev_err(&client->dev,"%s: Not support 'flash_mode' value %d\n",__func__, new_parms->flash_mode);
        }         
        // SCENE MODE
        if( new_parms->scene_mode != SCENE_MODE_NONE) /* Scene mode initialization */
        {
    
            err |= s5k4ecgx_set_parameter(sd, &parms->scene_mode,
                    SCENE_MODE_NONE, "scene_mode",
                    state->regs->scene_mode,
                    ARRAY_SIZE(state->regs->scene_mode));
            if(err < 0)
            {
                dev_err(&client->dev,"%s: Not support 'scene_mode' value %d\n",__func__, SCENE_MODE_NONE);
            }
    
        }

	// set portrait -> candleLight -> none  change issue fix
	// refrence from Cooper driver
	if( new_parms->scene_mode == SCENE_MODE_CANDLE_LIGHT ||
           new_parms->scene_mode == SCENE_MODE_DUSK_DAWN||
           new_parms->scene_mode == SCENE_MODE_SUNSET ){
		s5k4ecgx_set_REG_TC_DBG_AutoAlgEnBits(sd,3,0);
	}else if( new_parms->scene_mode == SCENE_MODE_PARTY_INDOOR||
	    new_parms->scene_mode == SCENE_MODE_BEACH_SNOW){
		s5k4ecgx_set_REG_TC_DBG_AutoAlgEnBits(sd,5,0);
       }
        
        err |= s5k4ecgx_set_parameter(sd, &parms->scene_mode,
                (new_parms->scene_mode < 0)?SCENE_MODE_NONE:new_parms->scene_mode, "scene_mode",
                state->regs->scene_mode,
                ARRAY_SIZE(state->regs->scene_mode));
        dev_dbg(&client->dev,"'scene_mode' value %d\n",new_parms->scene_mode);
        if(err < 0)
        {
            dev_err(&client->dev,"%s: Not support 'scene_mode' value %d\n",__func__, new_parms->scene_mode);
        }

        // white balance
        if( new_parms->white_balance == WHITE_BALANCE_AUTO )
        {
            s5k4ecgx_set_REG_TC_DBG_AutoAlgEnBits(sd,3,1);
        }
        else
        {
            s5k4ecgx_set_REG_TC_DBG_AutoAlgEnBits(sd,3,0);
        }
#if 0        
        err |= s5k4ecgx_set_parameter(sd, &parms->white_balance,
                new_parms->white_balance, "white balance",
                state->regs->white_balance,
                ARRAY_SIZE(state->regs->white_balance));
#endif
        err |=  s5k4ecgx_set_from_table(sd, "white balance",
                    &state->regs->white_balance,
                    ARRAY_SIZE(state->regs->white_balance), 
                    new_parms->white_balance);
        parms->white_balance = new_parms->white_balance;
        dev_dbg(&client->dev,"'white_balance' value %d\n",new_parms->white_balance);
        if(err < 0)
        {
            dev_err(&client->dev,"%s: Not support 'white balance' value %d\n",__func__, new_parms->white_balance);
        }
        // EFFECT
#if 0        
        err |= s5k4ecgx_set_parameter(sd, &parms->effects, new_parms->effects,
                    "effect", state->regs->effect,
                    ARRAY_SIZE(state->regs->effect));
#endif
        err |=  s5k4ecgx_set_from_table(sd, "effects",
                    &state->regs->effect,
                    ARRAY_SIZE(state->regs->effect), 
                    new_parms->effects);
        parms->effects = new_parms->effects;
        dev_dbg(&client->dev,"'effects' value %d\n",new_parms->effects);
        if(err < 0)
        {
            dev_err(&client->dev,"%s: Not support 'effect' value %d\n",__func__, new_parms->effects);
        }           
        // ISO
         if(new_parms->scene_mode != SCENE_MODE_NIGHTSHOT &&
	      new_parms->scene_mode != SCENE_MODE_SPORTS)
     	 {
	        if( new_parms->iso == ISO_AUTO)
	        {
	            s5k4ecgx_set_REG_TC_DBG_AutoAlgEnBits(sd,5,1);
	        }
	        else
	        {
	            s5k4ecgx_set_REG_TC_DBG_AutoAlgEnBits(sd,5,0);
	        }
#if 0
	        err |= s5k4ecgx_set_parameter(sd, &parms->iso, new_parms->iso,
	                    "iso", state->regs->iso,
	                    ARRAY_SIZE(state->regs->iso));
#endif
	        err |=  s5k4ecgx_set_from_table(sd, "iso",
	                    &state->regs->iso,
	                    ARRAY_SIZE(state->regs->iso), 
	                    new_parms->iso);
	        parms->iso = new_parms->iso;
	        dev_dbg(&client->dev,"'effects' value %d\n",new_parms->iso);
	        if(err < 0)
	        {
	            dev_err(&client->dev,"%s: Not support 'iso' value %d\n",__func__, new_parms->iso);
	        }
      }
        // METERING
#if 0        
        err |= s5k4ecgx_set_parameter(sd, &parms->metering, new_parms->metering,
                    "metering", state->regs->metering,
                    ARRAY_SIZE(state->regs->metering));
        dev_err(&client->dev,"'metering' value %d\n",new_parms->metering);
#endif        
        err |=  s5k4ecgx_set_from_table(sd, "metering",
            &state->regs->metering,
            ARRAY_SIZE(state->regs->metering), 
            new_parms->metering);
        parms->metering = new_parms->metering;
        if(err < 0)
        {
            dev_err(&client->dev,"%s: Not support 'metering' value %d\n",__func__, new_parms->metering);
        }
        // EV
#if 0        
        err |= s5k4ecgx_set_parameter(sd, &parms->brightness,
                    new_parms->brightness, "brightness",
                    state->regs->ev, ARRAY_SIZE(state->regs->ev));
#endif
        err |=  s5k4ecgx_set_from_table(sd, "brightness",
                    &state->regs->ev,
                    ARRAY_SIZE(state->regs->ev), 
                    new_parms->brightness);
        parms->brightness = new_parms->brightness;
        dev_dbg(&client->dev,"'brightness' value %d\n",new_parms->brightness);
        if(err < 0)
        {
            dev_err(&client->dev,"%s: Not support 'brightness' value %d\n",__func__, new_parms->brightness);
        }
        
        // 4~5 ETC
        // CONTRAST
        if( new_parms->scene_mode <= SCENE_MODE_NONE){ /* Scene mode initialization */
            err = s5k4ecgx_set_parameter(sd, &parms->contrast,
                        new_parms->contrast,"contrast",
                        state->regs->contrast,
                        ARRAY_SIZE(state->regs->contrast));
            dev_dbg(&client->dev,"'contrast' value %d\n",new_parms->contrast);
            if(err < 0)
            {
                dev_err(&client->dev,"%s: Not support 'contrast' value %d\n",__func__, new_parms->contrast);
            }
            
            err |= s5k4ecgx_set_parameter(sd, &parms->saturation,
                        new_parms->saturation, "saturation",
                        state->regs->saturation,
                        ARRAY_SIZE(state->regs->saturation));
            dev_dbg(&client->dev,"'saturation' value %d\n",new_parms->saturation);
            if(err < 0)
            {
                dev_err(&client->dev,"%s: Not support 'saturation' value %d\n",__func__, new_parms->saturation);
            }
        
            err |= s5k4ecgx_set_parameter(sd, &parms->sharpness,
                        new_parms->sharpness, "sharpness",
                        state->regs->sharpness,
                        ARRAY_SIZE(state->regs->sharpness));
            dev_dbg(&client->dev,"'sharpness' value %d\n",new_parms->sharpness);
            if(err < 0)
            {
                dev_err(&client->dev,"%s: Not support 'sharpness' value %d\n",__func__, new_parms->sharpness);
            }   
        }
        if (parms->scene_mode == SCENE_MODE_NIGHTSHOT)
            state->one_frame_delay_ms = NIGHT_MODE_MAX_ONE_FRAME_DELAY_MS;
        else
            state->one_frame_delay_ms = NORMAL_MODE_MAX_ONE_FRAME_DELAY_MS;
    
    
        if(err <0)
        {
            dev_err(&client->dev, "%s: Setting value error", __func__);
            err = 0;    
        }
        dev_dbg(&client->dev, "%s: returning %d\n", __func__, err);
        return err;

}

static int s5k4ecgx_g_parm(struct v4l2_subdev *sd,
			struct v4l2_streamparm *param)
{
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);

	memcpy(param, &state->strm, sizeof(param));
	return 0;
}

static int s5k4ecgx_s_parm(struct v4l2_subdev *sd,
			struct v4l2_streamparm *param)
{
	int err = 0;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
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
		int fps_max = 30;

		if (param->parm.capture.timeperframe.numerator &&
			param->parm.capture.timeperframe.denominator)
			fps =
			    (int)(param->parm.capture.timeperframe.denominator /
				  param->parm.capture.timeperframe.numerator);
		else
			fps = 0;

		if (fps <= 0 || fps > fps_max) {
			dev_err(&client->dev,
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
	}
    dev_dbg(&client->dev, " set frame rate %d\n",parms->capture.timeperframe.denominator);

    err = s5k4ecgx_set_all_parameter(sd,param);
    
    if(parms->capture.timeperframe.denominator == 15){
          err = s5k4ecgx_set_from_table(sd, " 15 fps",
                                      &state->regs->set_15_fps, 1, 0);
    }else{         
        if(camcorder_check_flag == 1){    
            err = s5k4ecgx_set_from_table(sd, " fixed fps",
                                        &state->regs->set_fix_fps, 1, 0);
            mdelay(NORMAL_MODE_MAX_ONE_FRAME_DELAY_MS*2);
        }else{
            if(state->regs->scene_mode <= SCENE_MODE_NONE){
                err = s5k4ecgx_set_from_table(sd, " auto fps",
                                            &state->regs->set_auto_fps, 1, 0);
            }
        }
    }

        return 0;
}

/* This function is called from the g_ctrl api
 *
 * This function should be called only after the s_fmt call,
 * which sets the required width/height value.
 *
 * It checks a list of available frame sizes and sets the
 * most appropriate frame size.
 *
 * The list is stored in an increasing order (as far as possible).
 * Hence the first entry (searching from the beginning) where both the
 * width and height is more than the required value is returned.
 * In case of no perfect match, we set the last entry (which is supposed
 * to be the largest resolution supported.)
 */
static void s5k4ecgx_set_framesize(struct v4l2_subdev *sd,
				const struct s5k4ecgx_framesize *frmsize,
				int frmsize_count, bool exact_match)
{
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	const struct s5k4ecgx_framesize *last_frmsize =
		&frmsize[frmsize_count - 1];
       int preview_ratio;

	dev_dbg(&client->dev, "%s: Requested Res: %dx%d\n", __func__,
		state->pix.width, state->pix.height);

	do {
		/*
		 * In case of image capture mode,
		 * if the given image resolution is not supported,
		 * return the next higher image resolution. */
		if (exact_match) {
			if (frmsize->width == state->pix.width &&
				frmsize->height == state->pix.height) {
				break;
			}
		} else {
			dev_dbg(&client->dev,
				"%s: compare frmsize %dx%d to %dx%d\n",
				__func__,
				frmsize->width, frmsize->height,
				state->pix.width, state->pix.height);
			if (frmsize->width >= state->pix.width &&
				frmsize->height >= state->pix.height) {
				dev_dbg(&client->dev,
					"%s: select frmsize %dx%d, index=%d\n",
					__func__,
					frmsize->width, frmsize->height,
					frmsize->index);
				break;
			}
		}

		frmsize++;
	} while (frmsize <= last_frmsize);

	if (frmsize > last_frmsize){
            preview_ratio = state->pix.width * 10 / state->pix.height;

            if(preview_ratio == 12){
                if( state->oprmode == S5K4ECGX_OPRMODE_VIDEO){
                    state->framesize_index = S5K4ECGX_CAMCORDER_PREVIEW_528;
                }else{
                    state->framesize_index = S5K4ECGX_PREVIEW_528;
                }
                state->pix.width = 528;
                state->pix.height = 432;
            }else{
                state->framesize_index = last_frmsize->index;
                state->pix.width = last_frmsize->width;
                state->pix.height = last_frmsize->height;                
            }
       }else{
        	state->framesize_index = frmsize->index;
        	state->pix.width = frmsize->width;
        	state->pix.height = frmsize->height;
       }
	dev_err(&client->dev, "%s: Camera Res Set: %dx%d\n",
		__func__, state->pix.width, state->pix.height);
}

static int s5k4ecgx_check_dataline_stop(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	int err;

	dev_dbg(&client->dev, "%s\n", __func__);

	err = s5k4ecgx_set_from_table(sd, "DTP stop",
				&state->regs->dtp_stop, 1, 0);
	if (err < 0) {
		v4l_info(client, "%s: register set failed\n", __func__);
		return -EIO;
	}

	state->check_dataline = 0;

	return err;
}

static void s5k4ecgx_get_esd_int(struct v4l2_subdev *sd,
				struct v4l2_control *ctrl)
{
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	u16 read_value;
	int err;

//	printk("[%s:%d] ESD CHECK routine\n", __func__, __LINE__);

	if ((S5K4ECGX_RUNMODE_RUNNING == state->runmode) &&
		(state->af_status != AF_START)) {
		err = s5k4ecgx_set_from_table(sd, "get esd status",
					&state->regs->get_esd_status,
					1, 0);
		err |= s5k4ecgx_i2c_read_twobyte(client, 0x0F12, &read_value);
		dev_dbg(&client->dev,
			"%s: read_value == 0x%x\n", __func__, read_value);
		/* return to write mode */
		err |= s5k4ecgx_i2c_write_twobyte(client, 0x0028, 0x7000);

		if (err < 0) {
			v4l_info(client,
				"Failed I2C for getting ESD information\n");
			ctrl->value = 0x01;
		} else {
			if (read_value != 0x0000) {
				v4l_info(client, "ESD interrupt happened!!\n");
				ctrl->value = 0x01;
			} else {
				dev_dbg(&client->dev,
					"%s: No ESD interrupt!!\n", __func__);
				ctrl->value = 0x00;
			}
		}
	} else
		ctrl->value = 0x00;
}

/* returns the real iso currently used by sensor due to lighting
 * conditions, not the requested iso we sent using s_ctrl.
 */
static int s5k4ecgx_get_iso(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	int err;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	u16 read_value1 = 0;
	u16 read_value2 = 0;
	int read_value;

	err = s5k4ecgx_set_from_table(sd, "get iso",
				&state->regs->get_iso, 1, 0);
	err |= s5k4ecgx_i2c_read_twobyte(client, 0x0F12, &read_value1);
	err |= s5k4ecgx_i2c_read_twobyte(client, 0x0F12, &read_value2);

	/* restore write mode */
	s5k4ecgx_i2c_write_twobyte(client, 0x0028, 0x7000);

	read_value = (read_value1 * read_value2 / 0x100)/2; /* divide by 384(decimal) */

	if (read_value > 0x300)
		ctrl->value = ISO_400;
	else if (read_value > 0x180)
		ctrl->value = ISO_200;
	else if (read_value > 0xC0)
		ctrl->value = ISO_100;
	else
		ctrl->value = ISO_50;

	dev_dbg(&client->dev, "%s: get iso == %d (0x%x, 0x%x, 0x%x)\n",
		__func__, ctrl->value, read_value1, read_value2,read_value);

	return err;
}

static int s5k4ecgx_get_shutterspeed(struct v4l2_subdev *sd,
	struct v4l2_control *ctrl)
{
	int err;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	u16 read_value_1;
	u16 read_value_2;
	u32 read_value;

	err = s5k4ecgx_set_from_table(sd, "get shutterspeed",
				&state->regs->get_shutterspeed, 1, 0);
	err |= s5k4ecgx_i2c_read_twobyte(client, 0x0F12, &read_value_1);
	err |= s5k4ecgx_i2c_read_twobyte(client, 0x0F12, &read_value_2);

	read_value = (read_value_2 << 16) | (read_value_1 & 0xffff);
	/* restore write mode */
	s5k4ecgx_i2c_write_twobyte(client, 0x0028, 0x7000);


	ctrl->value = read_value * 1000 / 400;

	dev_dbg(&client->dev,
			"%s: get shutterspeed == %d\n", __func__, ctrl->value);
        //pr_info("%s: read1= %d(0x%x),read2 = %d(0x%x), read_value=%d(0x%x)\n", __func__, read_value_1,read_value_1,read_value_2,read_value_2,read_value);
        //pr_info("%s: get shutterspeed == %d\n", __func__, ctrl->value);

	return err;

}

static int s5k4ecgx_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct sec_cam_parm *parms =
		(struct sec_cam_parm *)&state->strm.parm.raw_data;
	int err = 0;

	mutex_lock(&state->ctrl_lock);

	switch (ctrl->id) {
	case V4L2_CID_CAMERA_WHITE_BALANCE:
		ctrl->value = parms->white_balance;
		break;
	case V4L2_CID_CAMERA_EFFECT:
		ctrl->value = parms->effects;
		break;
	case V4L2_CID_CAMERA_CONTRAST:
		ctrl->value = parms->contrast;
		break;
	case V4L2_CID_CAMERA_SATURATION:
		ctrl->value = parms->saturation;
		break;
	case V4L2_CID_CAMERA_SHARPNESS:
		ctrl->value = parms->sharpness;
		break;
	case V4L2_CID_CAM_JPEG_MAIN_SIZE:
		ctrl->value = state->jpeg.main_size;
		break;
	case V4L2_CID_CAM_JPEG_MAIN_OFFSET:
		ctrl->value = state->jpeg.main_offset;
		break;
	case V4L2_CID_CAM_JPEG_THUMB_SIZE:
		ctrl->value = state->jpeg.thumb_size;
		break;
	case V4L2_CID_CAM_JPEG_THUMB_OFFSET:
		ctrl->value = state->jpeg.thumb_offset;
		break;
	case V4L2_CID_CAM_JPEG_POSTVIEW_OFFSET:
		ctrl->value = state->jpeg.postview_offset;
		break;
	case V4L2_CID_CAM_JPEG_MEMSIZE:
		ctrl->value = SENSOR_JPEG_SNAPSHOT_MEMSIZE;
		break;
	case V4L2_CID_CAM_JPEG_QUALITY:
		ctrl->value = state->jpeg.quality;
		break;
	case V4L2_CID_CAMERA_AUTO_FOCUS_RESULT:
		err = s5k4ecgx_get_auto_focus_result(sd, ctrl);
		break;
	case V4L2_CID_CAM_DATE_INFO_YEAR:
		ctrl->value = 2010;
		break;
	case V4L2_CID_CAM_DATE_INFO_MONTH:
		ctrl->value = 2;
		break;
	case V4L2_CID_CAM_DATE_INFO_DATE:
		ctrl->value = 25;
		break;
	case V4L2_CID_CAM_SENSOR_VER:
		ctrl->value = 1;
		break;
	case V4L2_CID_CAM_FW_MINOR_VER:
		ctrl->value = state->fw.minor;
		break;
	case V4L2_CID_CAM_FW_MAJOR_VER:
		ctrl->value = state->fw.major;
		break;
	case V4L2_CID_CAM_PRM_MINOR_VER:
		ctrl->value = state->prm.minor;
		break;
	case V4L2_CID_CAM_PRM_MAJOR_VER:
		ctrl->value = state->prm.major;
		break;
	case V4L2_CID_ESD_INT:
		s5k4ecgx_get_esd_int(sd, ctrl);
		break;
	case V4L2_CID_CAMERA_GET_ISO:
		err = s5k4ecgx_get_iso(sd, ctrl);
		break;
	case V4L2_CID_CAMERA_GET_SHT_TIME:
		err = s5k4ecgx_get_shutterspeed(sd, ctrl);
		break;
	case V4L2_CID_CAMERA_GET_FLASH_ONOFF:
		ctrl->value = state->flash_state_on_previous_capture;
		break;

	
	case V4L2_CID_CAMERA_OBJ_TRACKING_STATUS:
	case V4L2_CID_CAMERA_SMART_AUTO_STATUS:
		break;
	default:
//		err = -ENOIOCTLCMD;
		err = 0;

		dev_dbg(&client->dev, "%s: unknown ctrl id 0x%x\n",
			__func__, ctrl->id);
		break;
	}

	mutex_unlock(&state->ctrl_lock);

	return err;
}

static int s5k4ecgx_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct sec_cam_parm *parms =
		(struct sec_cam_parm *)&state->strm.parm.raw_data;
	int err = 0;
	int value = ctrl->value;

	dev_dbg(&client->dev, "%s: V4l2 control ID =%d, val = %d\n",
		__func__, ctrl->id - V4L2_CID_PRIVATE_BASE, value);

	/* No device or damaged device */
	if( state->runmode == S5K4ECGX_RUNMODE_ERROR )
		return -ENODEV;

	mutex_lock(&state->ctrl_lock);

	switch (ctrl->id) {
       case V4L2_CID_CAMERA_SET_AF_PREFLASH:
              err = s5k4ecgx_set_af_preflash_mode(sd,value);
              break;
              
	case V4L2_CID_CAMERA_FLASH_MODE:
		err = s5k4ecgx_set_flash_mode(sd, value);
		break;
        
	case V4L2_CID_CAMERA_BRIGHTNESS:
		err = s5k4ecgx_set_parameter(sd, &parms->brightness,
					value, "brightness",
					state->regs->ev,
					ARRAY_SIZE(state->regs->ev));
		break;
        
	case V4L2_CID_CAMERA_WHITE_BALANCE:
#ifdef NEW_CAMERA
		if( value == WHITE_BALANCE_AUTO )
		{
			s5k4ecgx_set_REG_TC_DBG_AutoAlgEnBits(sd,3,1);
		}
		else
		{
			s5k4ecgx_set_REG_TC_DBG_AutoAlgEnBits(sd,3,0);
		}
#endif
		err = s5k4ecgx_set_parameter(sd, &parms->white_balance,
				value, "white balance",
				state->regs->white_balance,
				ARRAY_SIZE(state->regs->white_balance));
		break;
        
	case V4L2_CID_CAMERA_EFFECT:
		err = s5k4ecgx_set_parameter(sd, &parms->effects,
				value, "effects", state->regs->effect,
				ARRAY_SIZE(state->regs->effect));
		break;
        
	case V4L2_CID_CAMERA_ISO:
#ifdef	NEW_CAMERA
		if( value == ISO_AUTO)
		{
			s5k4ecgx_set_REG_TC_DBG_AutoAlgEnBits(sd,5,1);
		}
		else
		{
			s5k4ecgx_set_REG_TC_DBG_AutoAlgEnBits(sd,5,0);
		}
#endif
		err = s5k4ecgx_set_parameter(sd, &parms->iso,
					value, "iso",
					state->regs->iso,
					ARRAY_SIZE(state->regs->iso));
		break;
        
	case V4L2_CID_CAMERA_METERING:
		err = s5k4ecgx_set_parameter(sd, &parms->metering,
				value, "metering",
				state->regs->metering,
				ARRAY_SIZE(state->regs->metering));
		break;
        
	case V4L2_CID_CAMERA_CONTRAST:
		err = s5k4ecgx_set_parameter(sd, &parms->contrast,
					value, "contrast",
					state->regs->contrast,
					ARRAY_SIZE(state->regs->contrast));
		break;
        
	case V4L2_CID_CAMERA_SATURATION:
		err = s5k4ecgx_set_parameter(sd, &parms->saturation,
					value, "saturation",
					state->regs->saturation,
					ARRAY_SIZE(state->regs->saturation));
		break;
        
	case V4L2_CID_CAMERA_SHARPNESS:
		err = s5k4ecgx_set_parameter(sd, &parms->sharpness,
					value, "sharpness",
					state->regs->sharpness,
					ARRAY_SIZE(state->regs->sharpness));
		break;
        
	case V4L2_CID_CAMERA_WDR:
		//err = s5k4ecgx_set_wdr(sd, value);
              err = s5k4ecgx_set_auto_contrast(sd, value);
		break;

	case V4L2_CID_CAMERA_FACE_DETECTION:
		//err = s5k4ecgx_set_face_detection(sd, value);
		err = 0;
		break;
        
	case V4L2_CID_CAMERA_FOCUS_MODE:
		err = s5k4ecgx_set_focus_mode(sd, value);
		break;
        
    case V4L2_CID_CAMERA_DEFAULT_FOCUS_POSITION:
        err = s5k4ecgx_set_focus_mode(sd, parms->focus_mode);
        break;
        
	case V4L2_CID_CAM_JPEG_QUALITY:
		state->jpeg.quality = value;
		err = s5k4ecgx_set_jpeg_quality(sd);
		break;
        
	case V4L2_CID_CAMERA_SCENE_MODE:
		err = s5k4ecgx_set_parameter(sd, &parms->scene_mode,
					SCENE_MODE_NONE, "scene_mode",
					state->regs->scene_mode,
					ARRAY_SIZE(state->regs->scene_mode));
		if (err < 0) {
			dev_err(&client->dev,
				"%s: failed to set scene-mode default value\n",
				__func__);
			break;
		}

		// set portrait -> candleLight -> none  change issue fix
		// refrence from Cooper driver
		if( value == SCENE_MODE_CANDLE_LIGHT || value == SCENE_MODE_DUSK_DAWN|| value == SCENE_MODE_SUNSET )
			s5k4ecgx_set_REG_TC_DBG_AutoAlgEnBits(sd,3,0);
		else if( value == SCENE_MODE_PARTY_INDOOR||value == SCENE_MODE_BEACH_SNOW)
			s5k4ecgx_set_REG_TC_DBG_AutoAlgEnBits(sd,5,0);


		if (value != SCENE_MODE_NONE) {
			err = s5k4ecgx_set_parameter(sd, &parms->scene_mode,
					value, "scene_mode",
					state->regs->scene_mode,
					ARRAY_SIZE(state->regs->scene_mode));
		}
		if (parms->scene_mode == SCENE_MODE_NIGHTSHOT) {
			state->one_frame_delay_ms =
				NIGHT_MODE_MAX_ONE_FRAME_DELAY_MS;
		} else {
			state->one_frame_delay_ms =
				NORMAL_MODE_MAX_ONE_FRAME_DELAY_MS;
		}


		break;
        
	case V4L2_CID_CAMERA_GPS_LATITUDE:
		dev_err(&client->dev,
			"%s: V4L2_CID_CAMERA_GPS_LATITUDE: not implemented\n",
			__func__);
		break;
        
	case V4L2_CID_CAMERA_GPS_LONGITUDE:
		dev_err(&client->dev,
			"%s: V4L2_CID_CAMERA_GPS_LONGITUDE: not implemented\n",
			__func__);
		break;
        
	case V4L2_CID_CAMERA_GPS_TIMESTAMP:
		dev_err(&client->dev,
			"%s: V4L2_CID_CAMERA_GPS_TIMESTAMP: not implemented\n",
			__func__);
		break;
        
	case V4L2_CID_CAMERA_GPS_ALTITUDE:
		dev_err(&client->dev,
			"%s: V4L2_CID_CAMERA_GPS_ALTITUDE: not implemented\n",
			__func__);
		break;
        
	case V4L2_CID_CAMERA_OBJECT_POSITION_X:
		state->position.x = value;
		break;
        
	case V4L2_CID_CAMERA_OBJECT_POSITION_Y:
		state->position.y = value;
		break;
        
	case V4L2_CID_CAMERA_TOUCH_AF_START_STOP:
#ifdef NEW_CAMERA
            if( value == 0 ){ /* AF stop */ 		
                s5k4ecgx_touch_auto_focus(sd,2); /* AE stop */
            }else{ /* AF start */
                s5k4ecgx_touch_auto_focus(sd,3); /* AE start */
            }        			
#endif
            s5k4ecgx_touch_auto_focus(sd,value);
            break;
        
	case V4L2_CID_CAMERA_SET_AUTO_FOCUS:
		if (value == AUTO_FOCUS_ON)
			err = s5k4ecgx_start_auto_focus(sd);
		else if (value == AUTO_FOCUS_OFF)
			err = s5k4ecgx_stop_auto_focus(sd);
		else {
			err = -EINVAL;
			dev_err(&client->dev,
				"%s: bad focus value requestion %d\n",
				__func__, value);
		}
		break;
        
        case V4L2_CID_CAMERA_AE_AWB_LOCKUNLOCK:
            err = s5k4ecgx_AE_AWB_lock(sd, ctrl);
            break;
        
	case V4L2_CID_CAMERA_FRAME_RATE:
		dev_dbg(&client->dev,
			"%s: camera frame rate request for %d fps\n",
			__func__, value);
		state->strm.parm.capture.timeperframe.denominator = value;
		break;
        
	case V4L2_CID_CAM_CAPTURE:
		err = s5k4ecgx_start_capture(sd);
		break;

	/* Used to start / stop preview operation.
	 * This call can be modified to START/STOP operation,
	 * which can be used in image capture also
	 */
	case V4L2_CID_CAM_PREVIEW_ONOFF:
		if (value)
			err = s5k4ecgx_set_preview_start(sd);
		else
			err = s5k4ecgx_set_preview_stop(sd);
		break;
        
	case V4L2_CID_CAMERA_CHECK_DATALINE:
		dev_dbg(&client->dev, "%s: check_dataline set to %d\n",
			__func__, value);
		state->check_dataline = value;
		break;
        
	case V4L2_CID_CAMERA_CHECK_DATALINE_STOP:
		err = s5k4ecgx_check_dataline_stop(sd);
		break;
        
	case V4L2_CID_CAMERA_ZOOM:
		err = s5k4ecgx_set_zoom(sd,ctrl->value);
		break;
        
	case V4L2_CID_CAMERA_VT_MODE:
#ifdef 		NEW_CAMERA
		state->vt_mode = value;
	//	printk("[%s:%d] V4L2_CID_CAMERA_VT_MODE, state->vt_mode %d\n", __func__,__LINE__,state->vt_mode);
#endif
		err = 0;
		break;
        
	case V4L2_CID_CAMERA_SENSOR_MODE:
#ifdef NEW_CAMERA
	//	printk("[%s:%d] Camera sensor mode is %d\n", __func__, __LINE__, value);
		if(value == 1)
			camcorder_check_flag = value;
		else
			camcorder_check_flag = value;
#endif
		err = 0;
		break;
        
	case V4L2_CID_CAMERA_SNAPSHOT_WIDTH:
		state->snapshot_width = ctrl->value;
		err = 0;
		break;
		
	default:
		dev_err(&client->dev, "%s: unknown set ctrl id 0x%x\n",
			__func__, ctrl->id - V4L2_CID_PRIVATE_BASE);
		err = 0;
		break;
	}

	if (err < 0)
		dev_err(&client->dev, "%s: videoc_s_ctrl failed %d\n", __func__,
			err);

	mutex_unlock(&state->ctrl_lock);

	dev_dbg(&client->dev, "%s: videoc_s_ctrl returning %d\n",
		__func__, err);

	return err;
}

static int s5k4ecgx_s_ext_ctrl(struct v4l2_subdev *sd,
			      struct v4l2_ext_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	int err = 0;
	struct gps_info_common *tempGPSType = NULL;

	switch (ctrl->id) {

	case V4L2_CID_CAMERA_GPS_LATITUDE:
		tempGPSType = (struct gps_info_common *)ctrl->reserved2[1];
		state->gps_info.gps_buf[0] = tempGPSType->direction;
		state->gps_info.gps_buf[1] = tempGPSType->dgree;
		state->gps_info.gps_buf[2] = tempGPSType->minute;
		state->gps_info.gps_buf[3] = tempGPSType->second;
		break;
	case V4L2_CID_CAMERA_GPS_LONGITUDE:
		tempGPSType = (struct gps_info_common *)ctrl->reserved2[1];
		state->gps_info.gps_buf[4] = tempGPSType->direction;
		state->gps_info.gps_buf[5] = tempGPSType->dgree;
		state->gps_info.gps_buf[6] = tempGPSType->minute;
		state->gps_info.gps_buf[7] = tempGPSType->second;
		break;
	case V4L2_CID_CAMERA_GPS_ALTITUDE:
		tempGPSType = (struct gps_info_common *)ctrl->reserved2[1];
		state->gps_info.altitude_buf[0] = tempGPSType->direction;
		state->gps_info.altitude_buf[1] =
					(tempGPSType->dgree) & 0x00ff;
		state->gps_info.altitude_buf[2] =
					((tempGPSType->dgree) & 0xff00) >> 8;
		state->gps_info.altitude_buf[3] = tempGPSType->minute;
		break;
	case V4L2_CID_CAMERA_GPS_TIMESTAMP:
		state->gps_info.gps_timeStamp = *((int *)ctrl->reserved2[1]);
		err = 0;
		break;
	default:
		dev_err(&client->dev, "%s: unknown ctrl->id %d\n",
			__func__, ctrl->id);
//		err = -ENOIOCTLCMD;
		err = 0;

		break;
	}

	if (err < 0)
		dev_err(&client->dev, "%s: vidioc_s_ext_ctrl failed %d\n",
			__func__, err);

	return err;
}

static int s5k4ecgx_s_ext_ctrls(struct v4l2_subdev *sd,
				struct v4l2_ext_controls *ctrls)
{
	struct v4l2_ext_control *ctrl = ctrls->controls;
	int ret = 0;
	int i;

	for (i = 0; i < ctrls->count; i++, ctrl++) {
		ret = s5k4ecgx_s_ext_ctrl(sd, ctrl);

		if (ret) {
			ctrls->error_idx = i;
			break;
		}
	}

	return ret;
}

#ifdef CONFIG_VIDEO_S5K4ECGX_DEBUG
static void s5k4ecgx_dump_regset(struct s5k4ecgx_regset *regset)
{
	if ((regset->data[0] == 0x00) && (regset->data[1] == 0x2A)) {
		if (regset->size <= 6)
			pr_err("odd regset size %d\n", regset->size);
		pr_info("regset: addr = 0x%02X%02X, data[0,1] = 0x%02X%02X,"
			" total data size = %d\n",
			regset->data[2], regset->data[3],
			regset->data[6], regset->data[7],
			regset->size-6);
	} else {
		pr_info("regset: 0x%02X%02X%02X%02X\n",
			regset->data[0], regset->data[1],
			regset->data[2], regset->data[3]);
		if (regset->size != 4)
			pr_err("odd regset size %d\n", regset->size);
	}
}
#endif

static int s5k4ecgx_i2c_write_block(struct v4l2_subdev *sd, u8 *buf, int size)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int retry_count = 5;
	int ret;
	struct i2c_msg msg = {client->addr, 0, size, buf};

#ifdef CONFIG_VIDEO_S5K4ECGX_DEBUG
	if (s5k4ecgx_debug_mask & S5K4ECGX_DEBUG_I2C_BURSTS) {
		if ((buf[0] == 0x0F) && (buf[1] == 0x12))
			pr_info("%s : data[0,1] = 0x%02X%02X,"
				" total data size = %d\n",
				__func__, buf[2], buf[3], size-2);
		else
			pr_info("%s : 0x%02X%02X%02X%02X\n",
				__func__, buf[0], buf[1], buf[2], buf[3]);
	}
#endif

	do {
		ret = i2c_transfer(client->adapter, &msg, 1);
		if (likely(ret == 1))
			break;
		msleep(POLL_TIME_MS);
	} while (retry_count-- > 0);
	if (ret != 1) {
		dev_err(&client->dev, "%s: I2C is not working.\n", __func__);
		return -EIO;
	}

#ifdef CONFIG_VIDEO_S5K4ECGX_V_1_0
	{
		struct s5k4ecgx_state *state =
			container_of(sd, struct s5k4ecgx_state, sd);
		if (state->fw.minor == 0) {
			/* v1.0 sensor have problems sometimes if we write
			 * too much data too fast, so add a sleep.  I've
			 * tried various combinations of size/delay.  Checking
			 * for a larger size doesn't seem to work reliably, and
			 * a delay of 1ms sometimes isn't enough either.
			 */
			if (size > 16)
				msleep(2);
		}
	}
#endif
	return 0;
}

/*
 * Parse the init_reg2 array into a number of register sets that
 * we can send over as i2c burst writes instead of writing each
 * entry of init_reg2 as a single 4 byte write.  Write the
 * new data structures and then free them.
 */
static int s5k4ecgx_write_init_reg2_burst(struct v4l2_subdev *sd)
{
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct s5k4ecgx_regset *regset_table;
	struct s5k4ecgx_regset *regset;
	struct s5k4ecgx_regset *end_regset;
	u8 *regset_data;
	u8 *dst_ptr;
	const u32 *end_src_ptr;
	bool flag_copied;
	int init_reg_2_array_size = state->regs->init_reg_2.array_size;
	int init_reg_2_size = init_reg_2_array_size * sizeof(u32);
	const u32 *src_ptr = state->regs->init_reg_2.reg;
	u32 src_value;
	int err;

	pr_debug("%s : start\n", __func__);

	regset_data = vmalloc(init_reg_2_size);
	if (regset_data == NULL)
		return -ENOMEM;
	regset_table = vmalloc(sizeof(struct s5k4ecgx_regset) *
			init_reg_2_size);
	if (regset_table == NULL) {
		kfree(regset_data);
		return -ENOMEM;
	}

	dst_ptr = regset_data;
	regset = regset_table;
	end_src_ptr = &state->regs->init_reg_2.reg[init_reg_2_array_size];

	src_value = *src_ptr++;
	while (src_ptr <= end_src_ptr) {
		/* initial value for a regset */
		regset->data = dst_ptr;
		flag_copied = false;
		*dst_ptr++ = src_value >> 24;
		*dst_ptr++ = src_value >> 16;
		*dst_ptr++ = src_value >> 8;
		*dst_ptr++ = src_value;

		/* check subsequent values for a data flag (starts with
		   0x0F12) or something else */
		do {
			src_value = *src_ptr++;
			if ((src_value & 0xFFFF0000) != 0x0F120000) {
				/* src_value is start of next regset */
				regset->size = dst_ptr - regset->data;
				regset++;
				break;
			}
			/* copy the 0x0F12 flag if not done already */
			if (!flag_copied) {
				*dst_ptr++ = src_value >> 24;
				*dst_ptr++ = src_value >> 16;
				flag_copied = true;
			}
			/* copy the data part */
			*dst_ptr++ = src_value >> 8;
			*dst_ptr++ = src_value;
		} while (src_ptr < end_src_ptr);
	}
	pr_debug("%s : finished creating table\n", __func__);

	end_regset = regset;
	pr_debug("%s : first regset = %p, last regset = %p, count = %d\n",
		__func__, regset_table, regset, end_regset - regset_table);
	pr_debug("%s : regset_data = %p, end = %p, dst_ptr = %p\n", __func__,
		regset_data, regset_data + (init_reg_2_size * sizeof(u32)),
		dst_ptr);

#ifdef CONFIG_VIDEO_S5K4ECGX_DEBUG
	if (s5k4ecgx_debug_mask & S5K4ECGX_DEBUG_I2C_BURSTS) {
		int last_regset_end_addr = 0;
		regset = regset_table;
		do {
			s5k4ecgx_dump_regset(regset);
			if (regset->size > 4) {
				int regset_addr = (regset->data[2] << 8 |
						regset->data[3]);
				if (last_regset_end_addr == regset_addr)
					pr_info("%s : this regset can be"
						" combined with previous\n",
						__func__);
				last_regset_end_addr = (regset_addr +
							regset->size - 6);
			}
			regset++;
		} while (regset < end_regset);
	}
#endif
	regset = regset_table;
	pr_debug("%s : start writing init reg 2 bursts\n", __func__);
	do {
		if (regset->size > 4) {
			/* write the address packet */
			err = s5k4ecgx_i2c_write_block(sd, regset->data, 4);
			if (err)
				break;
			/* write the data in a burst */
			err = s5k4ecgx_i2c_write_block(sd, regset->data+4,
						regset->size-4);

		} else
			err = s5k4ecgx_i2c_write_block(sd, regset->data,
						regset->size);
		if (err)
			break;
		regset++;
	} while (regset < end_regset);

	pr_debug("%s : finished writing init reg 2 bursts\n", __func__);

	vfree(regset_data);
	vfree(regset_table);

	return err;
}

static int s5k4ecgx_init_regs(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	u16 read_value;

	/* enter read mode */
	s5k4ecgx_i2c_write_twobyte(client, 0x002C, 0x7000);
	s5k4ecgx_i2c_write_twobyte(client, 0x002E, 0x01A6);
	s5k4ecgx_i2c_read_twobyte(client, 0x0F12, &read_value);

	pr_info("%s : revision %08X\n", __func__, read_value);

	/* restore write mode */
	s5k4ecgx_i2c_write_twobyte(client, 0x0028, 0x7000);

#ifdef CONFIG_VIDEO_S5K4ECGX_V_1_0
	if (read_value == S5K4ECGX_VERSION_1_0) {
		state->regs = &regs_for_fw_version_1_0;
		return 0;
	}
#endif
#ifdef CONFIG_VIDEO_S5K4ECGX_V_1_1
	if (read_value == S5K4ECGX_VERSION_1_1) {
		state->fw.minor = 1;
		state->regs = &regs_for_fw_version_1_1;

		return 0;
	}
	else
		state->runmode = S5K4ECGX_RUNMODE_ERROR;
#endif

	dev_err(&client->dev, "%s: unknown fw version 0x%x\n",
		__func__, read_value);
	return -ENODEV;
}

/* venturi add by park dong yun for issue camera status to audio driver */
//extern void mc1n2_voip_set_camera_status(int status);

static int s5k4ecgx_init(struct v4l2_subdev *sd, u32 val)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);

	dev_dbg(&client->dev, "%s: start\n", __func__);

	s5k4ecgx_init_parameters(sd);

	if (s5k4ecgx_init_regs(&state->sd) < 0)
		return -ENODEV;

#ifdef CONFIG_LOAD_FILE
	if (s5k4ecgx_regs_table_init(state->fw.minor)) {
		printk( "%s: config file read fail\n",
				__func__);
		return -EIO;
	}
	msleep(100);
#endif		

	dev_dbg(&client->dev, "%s: state->check_dataline : %d\n",
		__func__, state->check_dataline);

#ifdef	NEW_CAMERA
	if(state->vt_mode == 1){  /* For Qik */
//		printk("[%s:%d] init_reg1_Qik\n", __func__, __LINE__);
		if(s5k4ecgx_set_from_table(sd, "init reg 1 Qik",
				&state->regs->init_reg_1_qik, 1, 0) < 0)
			return -EIO;
	}
	else 
#endif
	{
	if (s5k4ecgx_set_from_table(sd, "init reg 1",
					&state->regs->init_reg_1, 1, 0) < 0)
		return -EIO;
	}
	/* delay 10ms after wakeup of SOC processor */
	msleep(10);

#ifdef CONFIG_LOAD_FILE
	if (s5k4ecgx_set_from_table(sd, "init reg 2",
					&state->regs->init_reg_2, 1, 0) < 0)
		return -EIO;	
#else

#ifdef	NEW_CAMERA
	if(state->vt_mode == 1){  /* For Qik */
		printk("[%s:%d] init_reg2_Qik\n", __func__, __LINE__);
		if(s5k4ecgx_set_from_table(sd, "init reg 2 Qik",
				&state->regs->init_reg_2_qik, 1, 0) < 0)
			return -EIO;
	}
	else 
#endif
	{
	if (s5k4ecgx_write_init_reg2_burst(sd) < 0)
		return -EIO;
	}
#endif

#ifdef	NEW_CAMERA
	if(state->vt_mode == 1){  /* For Qik */
//		printk("[%s:%d] flash init\n", __func__, __LINE__);
		/* We don't have to do anyting */
	}
	else 
#endif
	{
	if (s5k4ecgx_set_from_table(sd, "flash init",
				&state->regs->flash_init, 1, 0) < 0)
		return -EIO;
	}

	dev_dbg(&client->dev, "%s: end\n", __func__);

     /* add by park dong yun for venturi set flag  for voip */
//     mc1n2_voip_set_camera_status(1);
     
	return 0;
}

/*
 * s_config subdev ops
 * With camera device, we need to re-initialize
 * every single opening time therefor,
 * it is not necessary to be initialized on probe time.
 * except for version checking
 * NOTE: version checking is optional
 */
static int s5k4ecgx_s_config(struct v4l2_subdev *sd,
			int irq, void *platform_data)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct s5k4ecgx_platform_data *pdata = client->dev.platform_data;

	/*
	 * Assign default format and resolution
	 * Use configured default information in platform data
	 * or without them, use default information in driver
	 */
	state->pix.width = pdata->default_width;
	state->pix.height = pdata->default_height;

	if (!pdata->pixelformat)
		state->pix.pixelformat = DEFAULT_PIX_FMT;
	else
		state->pix.pixelformat = pdata->pixelformat;

	if (!pdata->freq)
		state->freq = DEFAULT_MCLK;	/* 24MHz default */
	else
		state->freq = pdata->freq;

	return 0;
}

static const struct v4l2_subdev_core_ops s5k4ecgx_core_ops = {
	.init = s5k4ecgx_init,	/* initializing API */
	.s_config = s5k4ecgx_s_config,	/* Fetch platform data */
	.g_ctrl = s5k4ecgx_g_ctrl,
	.s_ctrl = s5k4ecgx_s_ctrl,
	.s_ext_ctrls = s5k4ecgx_s_ext_ctrls,
};

static const struct v4l2_subdev_video_ops s5k4ecgx_video_ops = {
	.s_fmt = s5k4ecgx_s_fmt,
	.enum_framesizes = s5k4ecgx_enum_framesizes,
	.enum_fmt = s5k4ecgx_enum_fmt,
	.try_fmt = s5k4ecgx_try_fmt,
	.g_parm = s5k4ecgx_g_parm,
	.s_parm = s5k4ecgx_s_parm,
};

static const struct v4l2_subdev_ops s5k4ecgx_ops = {
	.core = &s5k4ecgx_core_ops,
	.video = &s5k4ecgx_video_ops,
};


/*
 * s5k4ecgx_probe
 * Fetching platform data is being done with s_config subdev call.
 * In probe routine, we just register subdev device
 */
static int s5k4ecgx_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct v4l2_subdev *sd;
	struct s5k4ecgx_state *state;
	struct s5k4ecgx_platform_data *pdata = client->dev.platform_data;

	if ((pdata == NULL) || (pdata->flash_onoff == NULL)) {
		dev_err(&client->dev, "%s: bad platform data\n", __func__);
		return -ENODEV;
	}

	state = kzalloc(sizeof(struct s5k4ecgx_state), GFP_KERNEL);
	if (state == NULL)
		return -ENOMEM;

	mutex_init(&state->ctrl_lock);
	init_completion(&state->af_complete);

	state->runmode = S5K4ECGX_RUNMODE_NOTREADY;
	sd = &state->sd;
	strcpy(sd->name, S5K4ECGX_DRIVER_NAME);

	/* Registering subdev */
	v4l2_i2c_subdev_init(sd, client, &s5k4ecgx_ops);

	dev_dbg(&client->dev, "5MP camera S5K4ECGX loaded.\n");

	return 0;
}

static int s5k4ecgx_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct s5k4ecgx_state *state =
		container_of(sd, struct s5k4ecgx_state, sd);
	struct s5k4ecgx_platform_data *pdata = client->dev.platform_data;
	
        if((state->runmode != S5K4ECGX_RUNMODE_NOTREADY)&&
            (state->runmode != S5K4ECGX_RUNMODE_ERROR)){
		    state->flash_on = false;
		    pdata->flash_onoff(0);            
            s5k4ecgx_set_softlanding(sd);
        }
	v4l2_device_unregister_subdev(sd);
	mutex_destroy(&state->ctrl_lock);
	kfree(state);

	dev_dbg(&client->dev, "Unloaded camera sensor S5K4ECGX.\n");

     /* add by park dong yun for venturi set flag  for voip */
//     mc1n2_voip_set_camera_status(0);
     
	return 0;
}

static const struct i2c_device_id s5k4ecgx_id[] = {
	{ S5K4ECGX_DRIVER_NAME, 0 },
	{}
};

MODULE_DEVICE_TABLE(i2c, s5k4ecgx_id);

static struct v4l2_i2c_driver_data v4l2_i2c_data = {
	.name = S5K4ECGX_DRIVER_NAME,
	.probe = s5k4ecgx_probe,
	.remove = s5k4ecgx_remove,
	.id_table = s5k4ecgx_id,
};

MODULE_DESCRIPTION("LSI S5K4ECGX 5MP SOC camera driver");
MODULE_AUTHOR("Seok-Young Jang <quartz.jang@samsung.com>");
MODULE_LICENSE("GPL");



