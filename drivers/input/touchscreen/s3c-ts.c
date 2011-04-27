/* linux/drivers/input/touchscreen/s3c-ts.c
 *
 * $Id: s3c-ts.c,v 1.13 2008/11/20 06:00:55 ihlee215 Exp $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
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
#include <linux/clk.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif /* CONFIG_HAS_EARLYSUSPEND */

#include <asm/io.h>
#include <asm/irq.h>
#include <mach/hardware.h>

#include <plat/regs-adc.h>
#include <mach/adcts.h>
#include <mach/ts.h>
#include <mach/irqs.h>

#ifdef CONFIG_CPU_FREQ
#include <plat/s5pc11x-dvfs.h>
#endif /* CONFIG_CPU_FREQ */

#define CONFIG_TOUCHSCREEN_S3C_DEBUG
#undef CONFIG_TOUCHSCREEN_S3C_DEBUG

#define S3C_TSVERSION   0x0101

struct s3c_ts_data {
	struct input_dev        *dev;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend	early_suspend;
#endif /* CONFIG_HAS_EARLYSUSPEND */
};

#ifdef CONFIG_HAS_EARLYSUSPEND
void s3c_ts_early_suspend(struct early_suspend *h);
void s3c_ts_late_resume(struct early_suspend *h);
#endif /* CONFIG_HAS_EARLYSUSPEND */

/*
 * Definitions & global arrays.
 */
static char *s3c_ts_name = "S3C TouchScreen";
static struct s3c_ts_data	*data;
static struct s3c_ts_mach_info 	*ts;

static u32 touch_count = 0;
static void s3c_ts_done_callback (struct s3c_adcts_value *ts_value)
{
	long i;
	int x, y;
	int x_sum = 0, y_sum = 0;

	if (ts_value->status == TS_STATUS_UP) {
		input_report_key(data->dev, BTN_TOUCH, 0);
		input_sync(data->dev);

		touch_count = 0;

		return;
	}

	for (i = 0; i < ts->sampling_time; i++) {
		x_sum += ts_value->xp[i];
		y_sum += ts_value->yp[i];
	}

	x = (int) x_sum / (ts->sampling_time);
	y = (int) y_sum / (ts->sampling_time);
#ifdef CONFIG_FB_S3C_LTE480WV
	y = 4000 - y;
#endif /* CONFIG_FB_S3C_LTE480WV */
	
	touch_count++;

	if (touch_count == 1) { //check first touch
#ifdef CONFIG_CPU_FREQ
		set_dvfs_perf_level();
#endif /* CONFIG_CPU_FREQ */
#ifdef CONFIG_TOUCHSCREEN_S3C_DEBUG
		printk(KERN_INFO "\nFirst BTN_TOUCH Event(%03d, %03d) is discard\n", x, y);
#endif
		return;
	}

	input_report_abs(data->dev, ABS_X, x);
	input_report_abs(data->dev, ABS_Y, y);
	input_report_abs(data->dev, ABS_Z, 0);
	input_report_key(data->dev, BTN_TOUCH, 1);
	input_sync(data->dev);
}

/*
 * The functions for inserting/removing us as a module.
 */
static int __init s3c_ts_probe(struct platform_device *pdev)
{
	struct device *dev;
	struct input_dev *input_dev;
	struct s3c_ts_mach_info * s3c_ts_cfg;
	int ret;

	dev = &pdev->dev;

	s3c_ts_cfg = (struct s3c_ts_mach_info *) dev->platform_data;
	if (s3c_ts_cfg == NULL)
		return -EINVAL;

	ts = kzalloc(sizeof(struct s3c_ts_mach_info), GFP_KERNEL);
	data = kzalloc(sizeof(struct s3c_ts_data), GFP_KERNEL);

	memcpy (ts, s3c_ts_cfg, sizeof(struct s3c_ts_mach_info));

	input_dev = input_allocate_device();

	if (!input_dev) {
		ret = -ENOMEM;
		goto input_dev_fail;
	}

	data->dev = input_dev;

	data->dev->evbit[0] = data->dev->evbit[0] = BIT_MASK(EV_SYN) | BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
	data->dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);

	input_set_abs_params(data->dev, ABS_X, ts->x_coor_min, ts->x_coor_max, ts->x_coor_fuzz, 0);
	input_set_abs_params(data->dev, ABS_Y, ts->y_coor_min, ts->y_coor_max, ts->y_coor_fuzz, 0);

	input_set_abs_params(data->dev, ABS_PRESSURE, 0, 1, 0, 0);

	data->dev->name = s3c_ts_name;
	data->dev->id.bustype = BUS_RS232;
	data->dev->id.vendor = 0xDEAD;
	data->dev->id.product = 0xBEEF;
	data->dev->id.version = S3C_TSVERSION;

#ifdef CONFIG_HAS_EARLYSUSPEND
	data->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	data->early_suspend.suspend = s3c_ts_early_suspend;
	data->early_suspend.resume = s3c_ts_late_resume;
	register_early_suspend(&data->early_suspend);
#endif /* CONFIG_HAS_EARLYSUSPEND */

	ret = s3c_adcts_register_ts(ts, s3c_ts_done_callback);
	if (ret) {
		dev_err(dev, "s3c_ts.c: Could not register adcts device(touchscreen)!\n");
		ret = -EIO;
		goto s3c_adcts_register_fail;
	}

	/* All went ok, so register to the input system */
	ret = input_register_device(data->dev);

	if (ret) {
		dev_err(dev, "s3c_ts.c: Could not register input device(touchscreen)!\n");
		ret = -EIO;
		goto input_register_fail;
	}
	return 0;

input_register_fail:
	s3c_adcts_unregister_ts();

s3c_adcts_register_fail:
	input_free_device (data->dev);

input_dev_fail:
	kfree (ts);
	kfree (data);

	return ret;
}

static int s3c_ts_remove(struct platform_device *dev)
{
	printk(KERN_INFO "s3c_ts_remove() of TS called !\n");

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&data->early_suspend);
#endif /* CONFIG_HAS_EARLYSUSPEND */

	input_unregister_device(data->dev);
	s3c_adcts_unregister_ts();
	input_free_device (data->dev);
	kfree (ts);
	kfree (data);

	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
void s3c_ts_early_suspend(struct early_suspend *h)
{
	s3c_adcts_unregister_ts();
}

void s3c_ts_late_resume(struct early_suspend *h)
{
	s3c_adcts_register_ts(ts, s3c_ts_done_callback);
}
#endif /* CONFIG_HAS_EARLYSUSPEND */

static struct platform_driver s3c_ts_driver = {
	.probe          = s3c_ts_probe,
	.remove         = s3c_ts_remove,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "s3c-ts",
	},
};

static char banner[] __initdata = KERN_INFO "S3C Touchscreen driver, (c) 2009 Samsung Electronics\n";

static int __init s3c_ts_init(void)
{
	printk(banner);
	return platform_driver_register(&s3c_ts_driver);
}

static void __exit s3c_ts_exit(void)
{
	platform_driver_unregister(&s3c_ts_driver);
}

module_init(s3c_ts_init);
module_exit(s3c_ts_exit);

MODULE_AUTHOR("Samsung AP");
MODULE_DESCRIPTION("Samsung touchscreen driver");
MODULE_LICENSE("GPL");
