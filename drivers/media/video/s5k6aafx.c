/*
 * Driver for S5K6AAFX from Samsung Electronics
 * 
 * 1/6" 1.3Mp CMOS Image Sensor SoC with an Embedded Image Processor
 *
 * Copyright (C) 2010, Dongsoo Nathaniel Kim<dongsoo45.kim@samsung.com>
 * Copyright (C) 2010, HeungJun Kim<riverful.kim@samsung.com>
 * Copyright (C) 2010, Arun c <arun.c@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-i2c-drv.h>
#ifdef CONFIG_VIDEO_SAMSUNG_V4L2
#include <linux/videodev2_samsung.h>
#endif
#include <media/s5k6aafx_platform.h>
#include "s5k6aafx.h"

#define FUNC_DEBUG
#ifdef FUNC_DEBUG
#define FUNC_ENTR() printk("[~~~~ S5K6AAFX ~~~~] %s entered\n", __func__)
#else
#define FUNC_ENTR() 
#endif

/*
 * s5k6aafx sensor i2c write routine
 * <start>--<Device address><2Byte Subaddr><2Byte Value>--<stop>
 */
static inline int s5k6aafx_write(struct i2c_client *client,
		unsigned long packet)
{
	unsigned char buf[4];
	int ret;

	struct i2c_msg msg = {
		.addr	= client->addr,
		.flags	= 0,
		.buf	= buf,
		.len	= 4,
	};

	*(unsigned long *)buf = cpu_to_be32(packet);
	ret = i2c_transfer(client->adapter, &msg, 1);

	if (unlikely(ret < 0)) {
		dev_err(&client->dev, "%s: 0x%08x write failed\n", __func__, packet);
		return ret;
	}
	
	return (ret != 1) ? -1 : 0;
}

/* program multiple registers */
static int s5k6aafx_write_regs(struct v4l2_subdev *sd,
		unsigned long *packet, unsigned int num)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret = -EAGAIN;	/* FIXME */
	unsigned long temp;
	char delay = 0;

	while (num--) {
		temp = *packet++;

	/*	if ((temp & S5K6AAFX_DELAY) == S5K6AAFX_DELAY) {
			if (temp & 0x1) {
				dev_info(&client->dev, "delay for 100msec\n");
				msleep(100);
				continue;
			} else {
				dev_info(&client->dev, "delay for 10msec\n");
				msleep(10);
				continue;
			}
	
	}*/
	if ((temp & S5K6AAFX_DELAY) == S5K6AAFX_DELAY) {                                                    
		delay = temp & 0xFFFF;                                                                              
		printk("[kidggang]:func(%s):line(%d):delay(0x%x):delay(%d)\n",__func__,__LINE__,delay,delay);       
		msleep(delay);                                                                                      
		continue;                                                                                           
	}
		ret = s5k6aafx_write(client, temp);

		/* In error circumstances */
		/* Give second shot */
		if (unlikely(ret)) {
			dev_info(&client->dev,
				"s5k6aafx i2c retry one more time\n");
			ret = s5k6aafx_write(client, temp);

			/* Give it one more shot */
			if (unlikely(ret)) {
				dev_info(&client->dev,
					"s5k6aafx i2c retry twice\n");
				ret = s5k6aafx_write(client, temp);
			}
		}
	}

	dev_info(&client->dev, "S5K6AAFX register programming ends up\n");
	return ret;	/* FIXME */
}

#if 0
static int s5k6aafx_s_crystal_freq(struct v4l2_subdev *sd, u32 freq, u32 flags)
{
	int err = -EINVAL;
	
	FUNC_ENTR();
	return err;
}
#endif

static int s5k6aafx_g_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
	FUNC_ENTR();
	return 0;
}

static int s5k6aafx_enum_framesizes(struct v4l2_subdev *sd, \
					struct v4l2_frmsizeenum *fsize)
{
	struct s5k6aafx_state *state = to_state(sd);
	/*
	 * Return the actual output settings programmed to the camera
	 */
	FUNC_ENTR();
	fsize->discrete.width = state->set_fmt.width;
	fsize->discrete.height = state->set_fmt.height;
	
	return 0;
}

#if 0
static int s5k6aafx_enum_fmt(struct v4l2_subdev *sd, struct v4l2_fmtdesc *fmtdesc)
{
	int err = 0;
	
	FUNC_ENTR();
	return err;
}

static int s5k6aafx_enum_frameintervals(struct v4l2_subdev *sd, 
					struct v4l2_frmivalenum *fival)
{
	int err = 0;
	
	FUNC_ENTR();
	return err;
}
#endif

static int s5k6aafx_try_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
	int err = 0;

	FUNC_ENTR();
	return err;
}

static int s5k6aafx_s_fmt(struct v4l2_subdev *sd, struct v4l2_format *fmt)
{
	struct s5k6aafx_state *state = to_state(sd);

	FUNC_ENTR();
	
	/*
	 * Just copying the requested format as of now.
	 * We need to check here what are the formats the camera support, and
	 * set the most appropriate one according to the request from FIMC
	 */
	state->req_fmt.width = fmt->fmt.pix.width;
	state->req_fmt.height = fmt->fmt.pix.height;
	state->req_fmt.pixelformat = fmt->fmt.pix.pixelformat;

	return 0;
}

static int s5k6aafx_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *parms)
{
	int err = 0;

	FUNC_ENTR();
	return err;
}

static int s5k6aafx_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *parms)
{
	int err = 0;

	FUNC_ENTR();
	return err;
}

static int s5k6aafx_init(struct v4l2_subdev *sd, u32 val)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k6aafx_state *state = to_state(sd);
	int err = -EINVAL;

	FUNC_ENTR();
	dev_info(&client->dev, "%s: camera initialization start\n", __func__);

	/* set initial regster value */
	err = s5k6aafx_write_regs(sd, s5k6aafx_common,
				       	sizeof(s5k6aafx_common) / sizeof(s5k6aafx_common[0]));
	if (unlikely(err)) {
		printk("%s: failed to init\n", __func__);
		return err;
	}

	state->set_fmt.width = DEFAULT_WIDTH;
	state->set_fmt.height = DEFAULT_HEIGHT;

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
static int s5k6aafx_s_config(struct v4l2_subdev *sd,
		int irq, void *platform_data)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct s5k6aafx_state *state = to_state(sd);
	struct s5k6aafx_platform_data *pdata;

	FUNC_ENTR();
	dev_info(&client->dev, "fetching platform data\n");

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
		state->req_fmt.width = DEFAULT_WIDTH;
		state->req_fmt.height = DEFAULT_HEIGHT;
	} else {
		state->req_fmt.width = pdata->default_width;
		state->req_fmt.height = pdata->default_height;
	}

	if (!pdata->pixelformat)
		state->req_fmt.pixelformat = DEFAULT_FMT;
	else
		state->req_fmt.pixelformat = pdata->pixelformat;

	return 0;
}

#if 0
static int s5k6aafx_queryctrl(struct v4l2_subdev *sd, struct v4l2_queryctrl *qc)
{
	FUNC_ENTR();
	return 0;
}

static int s5k6aafx_querymenu(struct v4l2_subdev *sd, struct v4l2_querymenu *qm)
{
	FUNC_ENTR();
	return 0;
}
#endif

static int s5k6aafx_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	FUNC_ENTR();
	printk("ctrl->id : %d \n", ctrl->id - V4L2_CID_PRIVATE_BASE);
	return 0;
}

static int s5k6aafx_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	FUNC_ENTR();
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = 0;

	printk("ctrl->id : %d \n", ctrl->id - V4L2_CID_PRIVATE_BASE);

	switch (ctrl->id) {
	case V4L2_CID_CAM_PREVIEW_ONOFF:
		printk("V4L2_CID_CAM_PREVIEW_ONOFF [%d] \n", ctrl->value);
		break;
	default:
		dev_err(&client->dev, "%s: no such control\n", __func__);
		break;
	}
	return err;
}

static const struct v4l2_subdev_core_ops s5k6aafx_core_ops = {
	.init = s5k6aafx_init,		/* initializing API */
	.s_config = s5k6aafx_s_config,	/* Fetch platform data */
#if 0
	.queryctrl = s5k6aafx_queryctrl,
	.querymenu = s5k6aafx_querymenu,
#endif
	.g_ctrl = s5k6aafx_g_ctrl,
	.s_ctrl = s5k6aafx_s_ctrl,
};

static const struct v4l2_subdev_video_ops s5k6aafx_video_ops = {
	/*.s_crystal_freq = s5k6aafx_s_crystal_freq,*/
	.g_fmt	= s5k6aafx_g_fmt,
	.s_fmt	= s5k6aafx_s_fmt,
	.enum_framesizes = s5k6aafx_enum_framesizes,
	/*.enum_frameintervals = s5k6aafx_enum_frameintervals,*/
	/*.enum_fmt = s5k6aafx_enum_fmt,*/
	.try_fmt = s5k6aafx_try_fmt,
	.g_parm	= s5k6aafx_g_parm,
	.s_parm	= s5k6aafx_s_parm,
};

static const struct v4l2_subdev_ops s5k6aafx_ops = {
	.core = &s5k6aafx_core_ops,
	.video = &s5k6aafx_video_ops,
};

/*
 * s5k6aafx_probe
 * Fetching platform data is being done with s_config subdev call.
 * In probe routine, we just register subdev device
 */
static int s5k6aafx_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct s5k6aafx_state *state;
	struct v4l2_subdev *sd;

	FUNC_ENTR();
	state = kzalloc(sizeof(struct s5k6aafx_state), GFP_KERNEL);
	if (state == NULL)
		return -ENOMEM;

	sd = &state->sd;
	strcpy(sd->name, S5K6AAFX_DRIVER_NAME);

	/* Registering subdev */
	v4l2_i2c_subdev_init(sd, client, &s5k6aafx_ops);

	dev_info(&client->dev, "s5k6aafx has been probed\n");
	return 0;
}

static int s5k6aafx_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);

	FUNC_ENTR();
	v4l2_device_unregister_subdev(sd);
	kfree(to_state(sd));
	return 0;
}

static const struct i2c_device_id s5k6aafx_id[] = {
	{ S5K6AAFX_DRIVER_NAME, 0 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, s5k6aafx_id);

static struct v4l2_i2c_driver_data v4l2_i2c_data = {
	.name = S5K6AAFX_DRIVER_NAME,
	.probe = s5k6aafx_probe,
	.remove = s5k6aafx_remove,
	.id_table = s5k6aafx_id,
};

MODULE_DESCRIPTION("S5K6AAFX ISP driver");
MODULE_AUTHOR("Heungjun Kim<riverful.kim@samsung.com>");
MODULE_LICENSE("GPL");
