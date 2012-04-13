/* linux/drivers/media/video/samsung/tv20/s5p_tv_base.c
 *
 * Entry file for Samsung TVOut driver
 *
 * Copyright (c) 2010 Samsung Electronics
 * http://www.samsungsemi.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/ioctl.h>
#include <linux/device.h>
#include <linux/clk.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/regulator/consumer.h>

#include <media/v4l2-common.h>
#include <media/v4l2-ioctl.h>
#include <mach/map.h>

#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <mach/regs-gpio.h>
/*#include <mach/max8998_function.h>*/
#include <linux/earlysuspend.h>
#if CONFIG_CPU_FREQ_S5PV210
#include <mach/cpu-freq-v210.h>
#endif /* CONFIG_CPU_FREQ_S5PV210 */

#ifdef CONFIG_S5PV210_PM
#include <mach/pd.h>
#endif

#include "s5p_tv.h"
#include "hpd.h"


#ifdef COFIG_TVOUT_DBG
#define S5P_TV_BASE_DEBUG 1
#endif

#ifdef S5P_TV_BASE_DEBUG
#define BASEPRINTK(fmt, args...) \
	printk(KERN_INFO "[TVBASE] %s: " fmt, __func__ , ## args)
#else
#define BASEPRINTK(fmt, args...)
#endif

#ifdef CONFIG_CPU_S5PV210
#define TVOUT_CLK_INIT(dev, clk, name)
#else
#define TVOUT_CLK_INIT(dev, clk, name)					\
	do {								\
		clk = clk_get(dev, name);				\
		if (clk == NULL) {					\
			printk(KERN_ERR					\
			"failed to find %s clock source\n", name);	\
			return -ENOENT;					\
		}							\
		clk_enable(clk)						\
	} while (0);
#endif

#define TVOUT_IRQ_INIT(x, ret, dev, num, jump, ftn, m_name)		\
	do {								\
		x = platform_get_irq(dev, num);				\
		if (x < 0) {						\
			printk(KERN_ERR					\
			"failed to get %s irq resource\n", m_name);	\
			ret = -ENOENT;					\
			goto jump;					\
		}							\
		ret = request_irq(x, ftn, IRQF_DISABLED,		\
			dev->name, dev);				\
		if (ret != 0) {						\
			printk(KERN_ERR					\
			"failed to install %s irq (%d)\n", m_name, ret);\
			goto jump;					\
		}							\
	} while (0);


#ifdef CONFIG_CPU_S5PC100
#define I2C_BASE
#endif

#define CABLE_CHECK 1

#ifdef CABLE_CHECK


struct class *sec_hdmi;
EXPORT_SYMBOL(sec_hdmi);

struct device *detect_cable;
EXPORT_SYMBOL(detect_cable);

static ssize_t detect_cable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int count;

#ifdef CONFIG_MACH_STEALTHV
	printk(KERN_ERR "HDMI_EN1:%d\n",gpio_get_value(GPIO_HDMI_EN1));

	if(gpio_get_value(GPIO_HD_HPD))
		 count = sprintf(buf,"1\n");
	else
		 count = sprintf(buf,"0\n");
#endif
	return count;
}

static ssize_t detect_cable_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int value;
	
	sscanf(buf, "%d", &value);
	if(value==1)
		value = 1;
	else
		value = 0;
#if 0
    	printk(KERN_ERR "detect_cable_store%d\n",value);
	printk(KERN_ERR "HDMI_EN1:%d\n",gpio_get_value(GPIO_HDMI_EN1));

	s3c_gpio_cfgpin(GPIO_HDMI_EN1, S3C_GPIO_OUTPUT);
	s3c_gpio_cfgpin(GPIO_HDMI_EN2, S3C_GPIO_OUTPUT);

	s3c_gpio_setpin(GPIO_HDMI_EN1, value);
	s3c_gpio_setpin(GPIO_HDMI_EN2, value);

	printk(KERN_ERR "recheck,HDMI_EN1:%d\n",gpio_get_value(GPIO_HDMI_EN1));
#endif
	return size;
}

static DEVICE_ATTR(connected, S_IRUGO | S_IWUSR, detect_cable_show, detect_cable_store);
#endif

static struct mutex	*mutex_for_fo;
static int suspend_resume_sync;

struct s5p_tv_status	s5ptv_status;
struct s5p_tv_vo	s5ptv_overlay[2];

int Isdrv_open = 0;
int IsPower_on = 0;

#ifdef I2C_BASE
static struct mutex	*mutex_for_i2c;
static struct work_struct ws_hpd;
spinlock_t slock_hpd;


static struct i2c_driver hdcp_i2c_driver;
static bool hdcp_i2c_drv_state;

const static u16 ignore[] = { I2C_CLIENT_END };
const static u16 normal_addr[] = {(S5P_HDCP_I2C_ADDR >> 1), I2C_CLIENT_END };
const static u16 *forces[] = { NULL };

static struct i2c_client_address_data addr_data = {
	.normal_i2c	= normal_addr,
	.probe		= ignore,
	.ignore		= ignore,
	.forces		= forces,
};

/*
 * i2c client drv.  - register client drv
 */
static int hdcp_i2c_attach(struct i2c_adapter *adap, int addr, int kind)
{

	struct i2c_client *c;

	c = kzalloc(sizeof(*c), GFP_KERNEL);

	if (!c)
		return -ENOMEM;

	strcpy(c->name, "s5p_ddc_client");

	c->addr = addr;

	c->adapter = adap;

	c->driver = &hdcp_i2c_driver;

	s5ptv_status.hdcp_i2c_client = c;

	dev_info(&adap->dev, "s5p_ddc_client attached "
		"into s5p_ddc_port successfully\n");

	return i2c_attach_client(c);
}

static int hdcp_i2c_attach_adapter(struct i2c_adapter *adap)
{
	int ret = 0;

	ret = i2c_probe(adap, &addr_data, hdcp_i2c_attach);

	if (ret) {
		dev_err(&adap->dev,
			"failed to attach s5p_hdcp_port driver\n");
		ret = -ENODEV;
	}

	return ret;
}

static int hdcp_i2c_detach(struct i2c_client *client)
{
	dev_info(&client->adapter->dev, "s5p_ddc_client detached "
		"from s5p_ddc_port successfully\n");

	i2c_detach_client(client);

	return 0;
}

static struct i2c_driver hdcp_i2c_driver = {
	.driver = {
		.name = "s5p_ddc_port",
	},
	.id = I2C_DRIVERID_S5P_HDCP,
	.attach_adapter = hdcp_i2c_attach_adapter,
	.detach_client = hdcp_i2c_detach,
};

static void set_ddc_port(void)
{
	mutex_lock(mutex_for_i2c);

	if (s5ptv_status.hpd_status) {

		if (!hdcp_i2c_drv_state)
			/* cable : plugged, drv : unregistered */
			if (i2c_add_driver(&hdcp_i2c_driver))
				printk(KERN_INFO "HDCP port add failed\n");

		/* changed drv. status */
		hdcp_i2c_drv_state = true;


		/* cable inserted -> removed */
		__s5p_set_hpd_detection(true, s5ptv_status.hdcp_en,
			s5ptv_status.hdcp_i2c_client);

	} else {

		if (hdcp_i2c_drv_state)
			/* cable : unplugged, drv : registered */
			i2c_del_driver(&hdcp_i2c_driver);

		/* changed drv. status */
		hdcp_i2c_drv_state = false;

		/* cable removed -> inserted */
		__s5p_set_hpd_detection(false, s5ptv_status.hdcp_en,
			s5ptv_status.hdcp_i2c_client);
		printk(KERN_INFO "%s cable removed\n", __func__);
	}

	mutex_unlock(mutex_for_i2c);
}
#endif

#ifdef CONFIG_CPU_S5PV210
bool is_tv_phy_enable;

int tv_phy_power(bool on)
{
	if (on) {
		if (!is_tv_phy_enable) {
		__s5p_tv_poweron();
		/* on */
		clk_enable(s5ptv_status.i2c_phy_clk);
		__s5p_hdmi_phy_power(true);
			is_tv_phy_enable = true;
		}
	} else {
		if (is_tv_phy_enable) {
		/*
		 * for preventing hdmi hang up when restart
		 * switch to internal clk - SCLK_DAC, SCLK_PIXEL
		 */
		clk_set_parent(s5ptv_status.sclk_mixer,
					s5ptv_status.sclk_dac);
		clk_set_parent(s5ptv_status.sclk_hdmi,
					s5ptv_status.sclk_pixel);

		__s5p_hdmi_phy_power(false);
		clk_disable(s5ptv_status.i2c_phy_clk);
		 __s5p_tv_poweroff();
			is_tv_phy_enable = false;
		}
	}

	return 0;
}

bool is_tv_clk_on;

int s5p_tv_clk_gate(bool on)
{
	if (on) {
		if (!is_tv_clk_on) {
#ifdef CONFIG_S5PV210_PM
		if (s5pv210_pd_enable("vp_pd") < 0) {
			printk(KERN_ERR "[Error]The power is not on for VP\n");
			goto err_pm;
		}
#endif
		clk_enable(s5ptv_status.vp_clk);
#ifdef CONFIG_S5PV210_PM
		if (s5pv210_pd_enable("mixer_pd") < 0) {
			printk(KERN_ERR "[Error]The power is not on for mixer\n");
			goto err_pm;
		}
#endif
		clk_enable(s5ptv_status.mixer_clk);
#ifdef CONFIG_S5PV210_PM
		if (s5pv210_pd_enable("tv_enc_pd") < 0) {
			printk(KERN_ERR "[Error]The power is not on for TV ENC\n");
			goto err_pm;
		}
#endif
		clk_enable(s5ptv_status.tvenc_clk);
#ifdef CONFIG_S5PV210_PM
		if (s5pv210_pd_enable("hdmi_pd") < 0) {
			printk(KERN_ERR "[Error]The power is not on for HDMI\n");
			goto err_pm;
		}
#endif
		clk_enable(s5ptv_status.hdmi_clk);
			is_tv_clk_on = true;
		}
	} else {
		if (is_tv_clk_on) {
		/* off */
		clk_disable(s5ptv_status.vp_clk);
#ifdef CONFIG_S5PV210_PM
		if (s5pv210_pd_disable("vp_pd") < 0) {
			printk(KERN_ERR "[Error]The power is not off for VP\n");
			goto err_pm;
		}
#endif
		clk_disable(s5ptv_status.mixer_clk);
#ifdef CONFIG_S5PV210_PM
		if (0 != s5pv210_pd_disable("mixer_pd")) {
			printk(KERN_ERR "[Error]The power is not off for mixer\n");
			goto err_pm;
		}
#endif
		clk_disable(s5ptv_status.tvenc_clk);
#ifdef CONFIG_S5PV210_PM
		if (s5pv210_pd_disable("tv_enc_pd") < 0) {
			printk(KERN_ERR "[Error]The power is not off for TV ENC\n");
			goto err_pm;
		}
#endif
		clk_disable(s5ptv_status.hdmi_clk);
#ifdef CONFIG_S5PV210_PM
		if (s5pv210_pd_disable("hdmi_pd") < 0) {
			printk(KERN_ERR "[Error]The power is not off for HDMI\n");
			goto err_pm;
		}
#endif
			is_tv_clk_on = false;
		}
	}

	return 0;
#ifdef CONFIG_S5PV210_PM
err_pm:
	return -1;
#endif
}
EXPORT_SYMBOL(s5p_tv_clk_gate);

#define TV_CLK_GET_WITH_ERR_CHECK(clk, pdev, clk_name)			\
	do {							        \
		clk = clk_get(&pdev->dev, clk_name);			\
		if (IS_ERR(clk)) {					\
			printk(KERN_ERR					\
			"failed to find clock \"%s\"\n", clk_name);	\
			return ENOENT;					\
		}							\
	} while (0);

static int __devinit tv_clk_get(struct platform_device *pdev,
	struct s5p_tv_status *ctrl)
{
	struct clk	*ext_xtal_clk,
			*mout_vpll_src,
			*fout_vpll,
			*mout_vpll;

	TV_CLK_GET_WITH_ERR_CHECK(ctrl->tvenc_clk,	pdev, "tvenc");
	TV_CLK_GET_WITH_ERR_CHECK(ctrl->vp_clk,		pdev, "vp");
	TV_CLK_GET_WITH_ERR_CHECK(ctrl->mixer_clk,	pdev, "mixer");
	TV_CLK_GET_WITH_ERR_CHECK(ctrl->hdmi_clk,	pdev, "hdmi");
	TV_CLK_GET_WITH_ERR_CHECK(ctrl->i2c_phy_clk,	pdev, "i2c-hdmiphy");

	TV_CLK_GET_WITH_ERR_CHECK(ctrl->sclk_dac,	pdev, "sclk_dac");
	TV_CLK_GET_WITH_ERR_CHECK(ctrl->sclk_mixer,	pdev, "sclk_mixer");
	TV_CLK_GET_WITH_ERR_CHECK(ctrl->sclk_hdmi,	pdev, "sclk_hdmi");

	TV_CLK_GET_WITH_ERR_CHECK(ctrl->sclk_pixel,	pdev, "sclk_pixel");
	TV_CLK_GET_WITH_ERR_CHECK(ctrl->sclk_hdmiphy,	pdev, "sclk_hdmiphy");

	TV_CLK_GET_WITH_ERR_CHECK(ext_xtal_clk,		pdev, "ext_xtal");
	TV_CLK_GET_WITH_ERR_CHECK(mout_vpll_src,	pdev, "mout_vpll_src");
	TV_CLK_GET_WITH_ERR_CHECK(fout_vpll,		pdev, "fout_vpll");
	TV_CLK_GET_WITH_ERR_CHECK(mout_vpll,		pdev, "mout_vpll");

	clk_set_parent(mout_vpll_src, ext_xtal_clk);
	clk_set_parent(mout_vpll, fout_vpll);

	/* sclk_dac's parent is fixed as mout_vpll */
	clk_set_parent(ctrl->sclk_dac, mout_vpll);

	clk_set_rate(fout_vpll, 54000000);
	clk_set_rate(ctrl->sclk_pixel, 54000000);

	clk_enable(ctrl->sclk_dac);
	clk_enable(ctrl->sclk_mixer);
	clk_enable(ctrl->sclk_hdmi);

	clk_enable(mout_vpll_src);
	clk_enable(fout_vpll);
	clk_enable(mout_vpll);

	clk_put(ext_xtal_clk);
	clk_put(mout_vpll_src);
	clk_put(fout_vpll);
	clk_put(mout_vpll);

	return 0;
}
#else
#define s5p_tv_clk_gate NULL
#define tv_phy_power NULL
#define tv_clk_get NULL
#endif

/*
 * ftn for irq
 */
static irqreturn_t s5p_tvenc_irq(int irq, void *dev_id)
{
	return IRQ_HANDLED;
}

#ifdef CONFIG_TV_FB
static int s5p_tv_open(struct file *file)
{
	return 0;
}

static int s5p_tv_release(struct file *file)
{
	s5ptv_status.hdcp_en = false;
	return 0;
}

static int s5p_tv_vid_open(struct file *file)
{
	int ret = 0;

	mutex_lock(mutex_for_fo);

	if (s5ptv_status.vp_layer_enable) {
		printk(KERN_ERR "video layer. already used !!\n");
		ret =  -EBUSY;
	}

	mutex_unlock(mutex_for_fo);
	return ret;
}

static int s5p_tv_vid_release(struct file *file)
{
	s5ptv_status.vp_layer_enable = false;

	_s5p_vlayer_stop();

	return 0;
}
#else

/*
 * ftn for video
 */
static int s5p_tv_v_open(struct file *file)
{
	int ret = 0;
	printk("[TVOUT]s5p_tv_v_open ++\n");
	TVout_LDO_ctrl(true);
	Isdrv_open = 1;
	mutex_lock(mutex_for_fo);
	
	if (s5ptv_status.tvout_output_enable) {
		BASEPRINTK("tvout drv. already used !!\n");
		ret =  -EBUSY;
		goto drv_used;
	}

	s5p_tv_clk_gate(true);


#ifdef CONFIG_CPU_S5PV210
#ifdef CONFIG_PM
	if((s5ptv_status.hpd_status) && !(s5ptv_status.suspend_status))
	{
		BASEPRINTK("tv is turned on\n");
#endif
#ifdef CONFIG_CPU_FREQ_S5PV210
		if ((s5ptv_status.hpd_status))
			s5pv210_set_cpufreq_level(RESTRICT_TABLE);
#endif /* CONFIG_CPU_FREQ_S5PV210 */

#ifdef CONFIG_PM
	}
	else
		BASEPRINTK("tv is off\n");
#endif
#endif
	_s5p_tv_if_init_param();

	s5p_tv_v4l2_init_param();

	mutex_unlock(mutex_for_fo);

#ifdef I2C_BASE
	mutex_lock(mutex_for_i2c);
	/* for ddc(hdcp port) */
	if (s5ptv_status.hpd_status) {
		if (i2c_add_driver(&hdcp_i2c_driver))
			BASEPRINTK("HDCP port add failed\n");
		hdcp_i2c_drv_state = true;
	} else
		hdcp_i2c_drv_state = false;

	mutex_unlock(mutex_for_i2c);
	/* for i2c probing */
	udelay(100);
#endif

	tv_phy_power( true );

	return 0;

drv_used:
	mutex_unlock(mutex_for_fo);
	return ret;
}

int s5p_tv_v_read(struct file *filp, char *buf, size_t count,
		  loff_t *f_pos)
{
	return 0;
}

int s5p_tv_v_write(struct file *filp, const char *buf, size_t
		   count, loff_t *f_pos)
{
	return 0;
}

int s5p_tv_v_mmap(struct file *filp, struct vm_area_struct *vma)
{
	return 0;
}

int s5p_tv_v_release(struct file *filp)
{
	TVout_LDO_ctrl(true);
#if defined(CONFIG_CPU_S5PV210) && defined(CONFIG_PM)
/*
	if(s5ptv_status.tvout_param.out_mode == TVOUT_OUTPUT_HDMI || s5ptv_status.tvout_param.out_mode == TVOUT_OUTPUT_HDMI_RGB
			|| s5ptv_status.tvout_param.out_mode == TVOUT_OUTPUT_DVI)
				tv_phy_power( false );
*/

	if((s5ptv_status.hpd_status) && !(s5ptv_status.suspend_status))
	{
#endif
		if (s5ptv_status.vp_layer_enable)
			_s5p_vlayer_stop();
		if (s5ptv_status.tvout_output_enable)
			_s5p_tv_if_stop();
#if defined(CONFIG_CPU_S5PV210) && defined(CONFIG_PM)
	} else
		s5ptv_status.vp_layer_enable = false;
#endif

	s5ptv_status.hdcp_en = false;

	s5ptv_status.tvout_output_enable = false;

	/*
	 * drv. release
	 *        - just check drv. state reg. or not.
	 */
#ifdef I2C_BASE
	mutex_lock(mutex_for_i2c);

	if (hdcp_i2c_drv_state) {
		i2c_del_driver(&hdcp_i2c_driver);
		hdcp_i2c_drv_state = false;
	}

	mutex_unlock(mutex_for_i2c);
#endif


#ifdef CONFIG_CPU_S5PV210
#ifdef CONFIG_PM
	if((s5ptv_status.hpd_status) && !(s5ptv_status.suspend_status))
	{
#endif

#ifdef CONFIG_CPU_FREQ_S5PV210
		if (s5ptv_status.hpd_status)
			s5pv210_set_cpufreq_level(NORMAL_TABLE);
#endif /* CONFIG_CPU_FREQ_S5PV210 */

#ifdef CONFIG_PM
	}
#endif
#endif

	Isdrv_open = 0;
	if(s5ptv_status.tvout_param.out_mode == TVOUT_OUTPUT_HDMI || s5ptv_status.tvout_param.out_mode == TVOUT_OUTPUT_HDMI_RGB
			|| s5ptv_status.tvout_param.out_mode == TVOUT_OUTPUT_DVI)
				tv_phy_power( false );
	s5p_tv_clk_gate(false);
	TVout_LDO_ctrl(false);
	printk("[TVOUT]s5p_tv_v_release --\n");
	return 0;
}

/*
 * ftn for graphic(video output overlay)
 */
 /*
static int check_layer(dev_t dev)
{
	int id = 0;
	int layer = 0;

	id = MINOR(dev);

	if (id < TVOUT_MINOR_GRP0 || id > TVOUT_MINOR_GRP1)
		BASEPRINTK("grp layer invalid\n");

	layer = (id == TVOUT_MINOR_GRP0) ? 0:1;

	return layer;

}
*/

static int vo_open(int layer, struct file *file)
{
	int ret = 0;

	mutex_lock(mutex_for_fo);

	/* check tvout path available!! */
	if (!s5ptv_status.tvout_output_enable) {
		BASEPRINTK("check tvout start !!\n");
		ret =  -EACCES;
		goto resource_busy;
	}

	if (s5ptv_status.grp_layer_enable[layer]) {
		BASEPRINTK("grp %d layer is busy!!\n", layer);
		ret =  -EBUSY;
		goto resource_busy;
	}

	/* set layer info.!! */
	s5ptv_overlay[layer].index = layer;

	/* set file private data.!! */
	file->private_data = &s5ptv_overlay[layer];

	mutex_unlock(mutex_for_fo);

	return 0;

resource_busy:
	mutex_unlock(mutex_for_fo);

	return ret;
}

int vo_release(int layer, struct file *filp)
{

	_s5p_grp_stop(layer);

	return 0;
}

/* modified for 2.6.29 v4l2-dev.c */
static int s5p_tv_vo0_open(struct file *file)
{
	vo_open(0, file);
	return 0;
}

static int s5p_tv_vo0_release(struct file *file)
{
	vo_release(0, file);
	return 0;
}

static int s5p_tv_vo1_open(struct file *file)
{
	vo_open(1, file);
	return 0;
}

static int s5p_tv_vo1_release(struct file *file)
{
	vo_release(1, file);
	return 0;
}
#endif

#ifdef CONFIG_TV_FB
static int s5ptvfb_alloc_framebuffer(void)
{
	int ret;

	/* alloc for each framebuffer */
	s5ptv_status.fb = framebuffer_alloc(sizeof(struct s5ptvfb_window),
					 s5ptv_status.dev_fb);
	if (!s5ptv_status.fb) {
		dev_err(s5ptv_status.dev_fb, "not enough memory\n");
		ret = -ENOMEM;
		goto err_alloc_fb;
	}

	ret = s5ptvfb_init_fbinfo(5);
	if (ret) {
		dev_err(s5ptv_status.dev_fb,
			"failed to allocate memory for fb for tv\n");
		ret = -ENOMEM;
		goto err_alloc_fb;
	}
#ifndef CONFIG_USER_ALLOC_TVOUT
	if (s5ptvfb_map_video_memory(s5ptv_status.fb)) {
		dev_err(s5ptv_status.dev_fb,
			"failed to map video memory "
			"for default window \n");
		ret = -ENOMEM;
		goto err_alloc_fb;
	}
#endif
	return 0;

err_alloc_fb:
	if (s5ptv_status.fb)
		framebuffer_release(s5ptv_status.fb);

	kfree(s5ptv_status.fb);

	return ret;
}

int s5ptvfb_free_framebuffer(void)
{
#ifndef CONFIG_USER_ALLOC_TVOUT
	if (s5ptv_status.fb)
		s5ptvfb_unmap_video_memory(s5ptv_status.fb);
#endif

	if (s5ptv_status.fb)
		framebuffer_release(s5ptv_status.fb);

	return 0;
}

int s5ptvfb_register_framebuffer(void)
{
	int ret;

	ret = register_framebuffer(s5ptv_status.fb);
	if (ret) {
		dev_err(s5ptv_status.dev_fb, "failed to register "
			"framebuffer device\n");
		return -EINVAL;
	}
#ifndef CONFIG_FRAMEBUFFER_CONSOLE
#ifndef CONFIG_USER_ALLOC_TVOUT
	s5ptvfb_check_var(&s5ptv_status.fb->var, s5ptv_status.fb);
	s5ptvfb_set_par(s5ptv_status.fb);
	s5ptvfb_draw_logo(s5ptv_status.fb);
#endif
#endif

	return 0;
}
#endif

/*
 * struct for video
 */
#ifdef CONFIG_TV_FB
static struct v4l2_file_operations s5p_tv_fops = {
	.owner		= THIS_MODULE,
	.open		= s5p_tv_open,
	.ioctl		= s5p_tv_ioctl,
	.release	= s5p_tv_release
};
static struct v4l2_file_operations s5p_tv_vid_fops = {
	.owner		= THIS_MODULE,
	.open		= s5p_tv_vid_open,
	.ioctl		= s5p_tv_vid_ioctl,
	.release	= s5p_tv_vid_release
};


 #else
static struct v4l2_file_operations s5p_tv_v_fops = {
	.owner		= THIS_MODULE,
	.open		= s5p_tv_v_open,
	.read		= s5p_tv_v_read,
	.write		= s5p_tv_v_write,
	.ioctl		= s5p_tv_v_ioctl,
	.mmap		= s5p_tv_v_mmap,
	.release	= s5p_tv_v_release
};

/*
 * struct for graphic0
 */
static struct v4l2_file_operations s5p_tv_vo0_fops = {
	.owner		= THIS_MODULE,
	.open		= s5p_tv_vo0_open,
	.ioctl		= s5p_tv_vo_ioctl,
	.release	= s5p_tv_vo0_release
};

/*
 * struct for graphic1
 */
static struct v4l2_file_operations s5p_tv_vo1_fops = {
	.owner		= THIS_MODULE,
	.open		= s5p_tv_vo1_open,
	.ioctl		= s5p_tv_vo_ioctl,
	.release	= s5p_tv_vo1_release
};
#endif

void s5p_tv_vdev_release(struct video_device *vdev)
{
	kfree(vdev);
}

struct video_device s5p_tvout[] = {

#ifdef CONFIG_TV_FB
	[0] = {
		.name = "S5PC1xx TVOUT ctrl",
		.fops = &s5p_tv_fops,
		.ioctl_ops = &s5p_tv_v4l2_ops,
		.release  = s5p_tv_vdev_release,
		.minor = TVOUT_MINOR_TVOUT,
		.tvnorms = V4L2_STD_ALL_HD,
	},
	[1] = {
		.name = "S5PC1xx TVOUT for Video",
		.fops = &s5p_tv_vid_fops,
		.ioctl_ops = &s5p_tv_v4l2_vid_ops,
		.release  = s5p_tv_vdev_release,
		.minor = TVOUT_MINOR_VID,
		.tvnorms = V4L2_STD_ALL_HD,
	},
#else
	[0] = {
		.name = "S5PC1xx TVOUT Video",
		.fops = &s5p_tv_v_fops,
		.ioctl_ops = &s5p_tv_v4l2_v_ops,
		.release  = s5p_tv_vdev_release,
		.minor = TVOUT_MINOR_VIDEO,
		.tvnorms = V4L2_STD_ALL_HD,
	},
	[1] = {
		.name = "S5PC1xx TVOUT Overlay0",
		.fops = &s5p_tv_vo0_fops,
		.ioctl_ops = &s5p_tv_v4l2_vo_ops,
		.release  = s5p_tv_vdev_release,
		.minor = TVOUT_MINOR_GRP0,
		.tvnorms = V4L2_STD_ALL_HD,
	},
	[2] = {
		.name = "S5PC1xx TVOUT Overlay1",
		.fops = &s5p_tv_vo1_fops,
		.ioctl_ops = &s5p_tv_v4l2_vo_ops,
		.release  = s5p_tv_vdev_release,
		.minor = TVOUT_MINOR_GRP1,
		.tvnorms = V4L2_STD_ALL_HD,
	},
#endif
};

void TVout_LDO_ctrl(int enable)
{
	int i=0;
	if(enable == true)
	{
		if(!IsPower_on)
		{
			regulator_enable(s5ptv_status.tv_tv);
		        regulator_enable(s5ptv_status.tv_tvout);
		       
			printk("%s: LDO3_8 is enabled by TV \n", __func__);
			IsPower_on = 1;
			//msleep(120);
	}
	}
	else if(enable == false)
	{
//		if(s5ptv_status.suspend_status)
//			msleep(520);
//			
//			for(i;i<50;i++) {
//				if(Isdrv_open)
//					msleep(50);
//			}
			if (IsPower_on)
			{
				//ldo 3,8 off
				regulator_disable(s5ptv_status.tv_tv);
			        regulator_disable(s5ptv_status.tv_tvout);
				printk("%s: LDO3_8 is disabled by TV \n", __func__);
				IsPower_on = 0;
			}
	}


}

EXPORT_SYMBOL(TVout_LDO_ctrl);

void s5p_handle_cable(void)
{
	char env_buf[120];
	char *envp[2];
	int env_offset = 0;

	msleep(200);
	int previous_hpd_status = s5ptv_status.hpd_status;
#ifdef CONFIG_HDMI_HPD
	if (s5p_hpd_get_state()) {
        s5ptv_status.hpd_status = 1;
        set_irq_type(IRQ_EINT13, IRQ_TYPE_EDGE_FALLING);
        	if (suspend_resume_sync == 2)
	            suspend_resume_sync = 0;

	} else {
        s5ptv_status.hpd_status = 0;
        set_irq_type(IRQ_EINT13, IRQ_TYPE_EDGE_RISING); 
	}

#else
	return;
#endif
    printk(KERN_INFO "HDMI handle_cable previous status is %d and current status is %d\n",previous_hpd_status,s5ptv_status.hpd_status);
    
    memset(env_buf, 0, sizeof(env_buf));

	if(previous_hpd_status == s5ptv_status.hpd_status)
	{
		BASEPRINTK("same hpd_status value: %d\n", previous_hpd_status);
		return;
	}

	if(s5ptv_status.hpd_status)
	{
	        //TVout_LDO_ctrl(true);
		BASEPRINTK("\n hdmi cable is connected \n");
		sprintf(env_buf, "HDMI_STATE=online");
		envp[env_offset++] = env_buf;
		envp[env_offset] = NULL;
		kobject_uevent_env(&(s5p_tvout[0].dev.kobj), KOBJ_CHANGE, envp);
		printk("[HDMI] s5p_handle_cable :: send uevent  connected\n");

        if(s5ptv_status.suspend_status)
        {
		    printk("[TVOUT]hdmi cable is connected Before Suspend \n");
            return;
        }

#ifdef CONFIG_CPU_FREQ_S5PV210
		s5pv210_set_cpufreq_level(RESTRICT_TABLE);
#endif /* CONFIG_CPU_FREQ_S5PV210 */
		TVout_LDO_ctrl(true);
		s5p_tv_clk_gate(true);
		tv_phy_power( true ); //wjyoo 0808
		mdelay(10);
		/* tv on */
		if (s5ptv_status.tvout_output_enable)
			_s5p_tv_if_start();

		/* video layer start */
		if (s5ptv_status.vp_layer_enable)
			_s5p_vlayer_start();

		/* grp0 layer start */
		if (s5ptv_status.grp_layer_enable[0])
			_s5p_grp_start(VM_GPR0_LAYER);

		/* grp1 layer start */
		if (s5ptv_status.grp_layer_enable[1])
			_s5p_grp_start(VM_GPR1_LAYER);


	} else {
		
		BASEPRINTK("\n hdmi cable is disconnected \n");

		#if 1
	        if(s5ptv_status.suspend_status || suspend_resume_sync == 2)
        	{
				TVout_LDO_ctrl(true);
				s5p_tv_clk_gate( true );
				tv_phy_power( true );
			}
		#endif 

	 if (suspend_resume_sync == 0) {
		if (s5ptv_status.vp_layer_enable) {
			_s5p_vlayer_stop();
			s5ptv_status.vp_layer_enable = true;

		}

		/* grp0 layer stop */
		if (s5ptv_status.grp_layer_enable[0]) {
			_s5p_grp_stop(VM_GPR0_LAYER);
			s5ptv_status.grp_layer_enable[VM_GPR0_LAYER] = true;
		}

		/* grp1 layer stop */
		if (s5ptv_status.grp_layer_enable[1]) {
			_s5p_grp_stop(VM_GPR1_LAYER);
			s5ptv_status.grp_layer_enable[VM_GPR0_LAYER] = true;
		}

		/* tv off */
		if (s5ptv_status.tvout_output_enable) {
			_s5p_tv_if_stop();
            s5ptv_status.tvout_output_enable = false;
            s5ptv_status.tvout_param_available = false;
		}

#ifdef CONFIG_PM
		/* clk & power off */
		tv_phy_power(false); //wjyoo 0808
		s5p_tv_clk_gate(false);
		TVout_LDO_ctrl(false);
		
#endif
		}

		sprintf(env_buf, "HDMI_STATE=offline");
		envp[env_offset++] = env_buf;
		envp[env_offset] = NULL;
		kobject_uevent_env(&(s5p_tvout[0].dev.kobj), KOBJ_CHANGE, envp);
		printk("[HDMI] s5p_handle_cable :: send uevent  diconnected\n");

#ifdef CONFIG_CPU_FREQ_S5PV210
	if (suspend_resume_sync == 0)
		s5pv210_set_cpufreq_level(NORMAL_TABLE);
#endif /* CONFIG_CPU_FREQ_S5PV210 */
	}
}

#define S5P_TVMAX_CTRLS		ARRAY_SIZE(s5p_tvout)
/*
 *  Probe
 */

static int __devinit s5p_tv_probe(struct platform_device *pdev)
{
	int	irq_num;
	int	ret;
	int 	i;

	s5ptv_status.dev_fb = &pdev->dev;
	 suspend_resume_sync = 0;
	is_tv_clk_on = false;
	is_tv_phy_enable = false;
	//enable power	
	/* Get csis power domain regulator */
	s5ptv_status.tv_regulator = regulator_get(&pdev->dev, "pd");
	if (IS_ERR(s5ptv_status.tv_regulator)) {
		printk(KERN_ERR "%s %d: failed to get resource %s\n",
				__func__, __LINE__, "s3c-tv20 pd");
		return PTR_ERR(s5ptv_status.tv_regulator);
	}

	s5ptv_status.tv_tvout = regulator_get(NULL, "tvout");
	if (IS_ERR(s5ptv_status.tv_tvout)) {
		printk(KERN_ERR "%s %d: failed to get resource %s\n",
				__func__, __LINE__, "s3c-tv20 tvout");
		return PTR_ERR(s5ptv_status.tv_tvout);
	}

	s5ptv_status.tv_tv = regulator_get(NULL, "hdmi");
	if (IS_ERR(s5ptv_status.tv_tv)) {
		printk(KERN_ERR "%s %d: failed to get resource %s\n",
				__func__, __LINE__, "s3c-tv20 tv");
		return PTR_ERR(s5ptv_status.tv_tv);
	}
	
	regulator_enable(s5ptv_status.tv_tv);
	regulator_enable(s5ptv_status.tv_tvout);
	regulator_enable(s5ptv_status.tv_regulator);

	s5ptv_status.dev_fb = &pdev->dev;

	__s5p_sdout_probe(pdev, 0);
	__s5p_vp_probe(pdev, 1);
	__s5p_mixer_probe(pdev, 2);

#ifdef CONFIG_CPU_S5PV210
	tv_clk_get(pdev, &s5ptv_status);
	s5p_tv_clk_gate(true);
#endif
#ifdef CONFIG_CPU_S5PV210	
	__s5p_hdmi_probe(pdev, 3, 4);
	__s5p_hdcp_init();
#endif

#ifdef FIX_27M_UNSTABLE_ISSUE /* for smdkc100 pop */
	writel(0x1, S5PC1XX_GPA0_BASE + 0x56c);
#endif

#ifdef I2C_BASE
	/* for dev_dbg err. */
	spin_lock_init(&slock_hpd);


	/* for bh */
	INIT_WORK(&ws_hpd, (void *)set_ddc_port);
#endif
	/* check EINT init state */
#ifdef CONFIG_MACH_STEALTHV
	gpio_set_value(GPIO_HDMI_EN1, 1);
#endif
	s5ptv_status.hpd_status= 0;
	
	dev_info(&pdev->dev, "hpd status is cable %s\n", 
		s5ptv_status.hpd_status ? "inserted":"removed");


	/* interrupt */
	TVOUT_IRQ_INIT(irq_num, ret, pdev, 0, out, __s5p_mixer_irq, "mixer");
	TVOUT_IRQ_INIT(irq_num, ret, pdev, 1, out_hdmi_irq, __s5p_hdmi_irq , "hdmi");
	TVOUT_IRQ_INIT(irq_num, ret, pdev, 2, out_tvenc_irq, s5p_tvenc_irq, "tvenc");
	
	/* v4l2 video device registration */
	for (i = 0; i < S5P_TVMAX_CTRLS; i++) {
		s5ptv_status.video_dev[i] = &s5p_tvout[i];

		if (video_register_device(s5ptv_status.video_dev[i],
				VFL_TYPE_GRABBER, s5p_tvout[i].minor) != 0) {

			dev_err(&pdev->dev,
				"Couldn't register tvout driver.\n");
			return 0;
		}
	}

#ifdef CONFIG_TV_FB
	mutex_init(&s5ptv_status.fb_lock);

	/* for default start up */
	_s5p_tv_if_init_param();

	s5ptv_status.tvout_param.disp_mode = TVOUT_720P_60;
	s5ptv_status.tvout_param.out_mode  = TVOUT_OUTPUT_HDMI;

#ifndef CONFIG_USER_ALLOC_TVOUT
	s5p_tv_clk_gate(true);
	if(s5ptv_status.tvout_param.out_mode == TVOUT_OUTPUT_HDMI || s5ptv_status.tvout_param.out_mode == TVOUT_OUTPUT_HDMI_RGB
		|| s5ptv_status.tvout_param.out_mode == TVOUT_OUTPUT_DVI)
		tv_phy_power(true);
	_s5p_tv_if_set_disp();
#endif

	s5ptvfb_set_lcd_info(&s5ptv_status);

	/* prepare memory */
	if (s5ptvfb_alloc_framebuffer())
		goto err_alloc;

	if (s5ptvfb_register_framebuffer())
		goto err_alloc;
#ifndef CONFIG_USER_ALLOC_TVOUT
	s5ptvfb_display_on(&s5ptv_status);
#endif
#endif

	mutex_for_fo = kmalloc(sizeof(struct mutex),
		GFP_KERNEL);

	if (mutex_for_fo == NULL) {
		dev_err(&pdev->dev,
			"failed to create mutex handle\n");
		goto out;
	}

#ifdef I2C_BASE
	mutex_for_i2c = kmalloc(sizeof(struct mutex),
		GFP_KERNEL);

	if (mutex_for_i2c == NULL) {
		dev_err(&pdev->dev,
			"failed to create mutex handle\n");
		goto out;
	}
	mutex_init(mutex_for_i2c);
#endif
	mutex_init(mutex_for_fo);

#ifdef CONFIG_CPU_S5PV210
	/* added for phy cut off when boot up */
	clk_enable(s5ptv_status.i2c_phy_clk);
	__s5p_hdmi_phy_power(false);
	clk_disable(s5ptv_status.i2c_phy_clk);
	s5p_tv_clk_gate(false);
#endif
	regulator_disable(s5ptv_status.tv_regulator);
	regulator_disable(s5ptv_status.tv_tv);
	regulator_disable(s5ptv_status.tv_tvout);
	printk("tv_base_probe is successfully\n");

	return 0;

#ifdef CONFIG_TV_FB
err_alloc:
#endif

out_tvenc_irq:
	free_irq(IRQ_HDMI, pdev);

out_hdmi_irq:
	free_irq(IRQ_MIXER, pdev);

out:
	printk(KERN_ERR "not found (%d). \n", ret);

	return ret;
}

/*
 *  Remove
 */
static int s5p_tv_remove(struct platform_device *pdev)
{
	__s5p_hdmi_release(pdev);
	__s5p_sdout_release(pdev);
	__s5p_mixer_release(pdev);
	__s5p_vp_release(pdev);
#ifdef I2C_BASE
	i2c_del_driver(&hdcp_i2c_driver);
#endif

	clk_disable(s5ptv_status.tvenc_clk);
	clk_disable(s5ptv_status.vp_clk);
	clk_disable(s5ptv_status.mixer_clk);
	clk_disable(s5ptv_status.hdmi_clk);
	clk_disable(s5ptv_status.sclk_hdmi);
	clk_disable(s5ptv_status.sclk_mixer);
	clk_disable(s5ptv_status.sclk_dac);

	clk_put(s5ptv_status.tvenc_clk);
	clk_put(s5ptv_status.vp_clk);
	clk_put(s5ptv_status.mixer_clk);
	clk_put(s5ptv_status.hdmi_clk);
	clk_put(s5ptv_status.sclk_hdmi);
	clk_put(s5ptv_status.sclk_mixer);
	clk_put(s5ptv_status.sclk_dac);
	clk_put(s5ptv_status.sclk_pixel);
	clk_put(s5ptv_status.sclk_hdmiphy);

	free_irq(IRQ_MIXER, pdev);
	free_irq(IRQ_HDMI, pdev);
	free_irq(IRQ_TVENC, pdev);

	mutex_destroy(mutex_for_fo);
#ifdef I2C_BASE
	mutex_destroy(mutex_for_i2c);
#endif
             TVout_LDO_ctrl(false);
	return 0;
}


#ifdef CONFIG_PM
/*
 *  Suspend
 */
int s5p_tv_early_suspend(struct platform_device *dev, pm_message_t state)
{
	BASEPRINTK("(hpd_status = %d)++\n", s5ptv_status.hpd_status);
	printk(KERN_INFO "s5p_tv_early suspend executing..\n");

	mutex_lock(mutex_for_fo);
	s5ptv_status.suspend_status = true;
	suspend_resume_sync = 1;
#ifdef CABLE_CHECK
#if 0    
    if (s5p_hpd_get_state()) {
        set_irq_type(IRQ_EINT13, IRQ_TYPE_EDGE_FALLING);
        hpd_sleep_state = 0;
	else{
       	set_irq_type(IRQ_EINT13, IRQ_TYPE_EDGE_RISING);
        hpd_sleep_state = 1;
    }
#endif
       	hpd_sleep_state = 1;
        set_irq_type(IRQ_EINT13, IRQ_TYPE_EDGE_BOTH);
#endif
	if(!(s5ptv_status.hpd_status))
    	{
	        printk(KERN_INFO "s5p_tv_early_suspend returned no HDMI connected\n");
		mutex_unlock(mutex_for_fo);
	        return 0;
	}
	else
	{
                //IsPower_on = false;
                //TVout_LDO_ctrl(true);
		/* video layer stop */
		if (s5ptv_status.vp_layer_enable) {
			_s5p_vlayer_stop();
			s5ptv_status.vp_layer_enable = true;

		}

		/* grp0 layer stop */
		if (s5ptv_status.grp_layer_enable[0]) {
			_s5p_grp_stop(VM_GPR0_LAYER);
			s5ptv_status.grp_layer_enable[VM_GPR0_LAYER] = true;
		}

		/* grp1 layer stop */
		if (s5ptv_status.grp_layer_enable[1]) {
			_s5p_grp_stop(VM_GPR1_LAYER);
			s5ptv_status.grp_layer_enable[VM_GPR0_LAYER] = true;
		}

		/* tv off */
		if (s5ptv_status.tvout_output_enable) {
			_s5p_tv_if_stop();
			s5ptv_status.tvout_output_enable = true;
			s5ptv_status.tvout_param_available = true;
		}

		if(s5ptv_status.tvout_param.out_mode == TVOUT_OUTPUT_HDMI || s5ptv_status.tvout_param.out_mode == TVOUT_OUTPUT_HDMI_RGB
			|| s5ptv_status.tvout_param.out_mode == TVOUT_OUTPUT_DVI)
			tv_phy_power( false );
	    
		/* clk & power off */
		s5p_tv_clk_gate(false);
		TVout_LDO_ctrl(false);


#ifdef CONFIG_CPU_FREQ_S5PV210
		s5pv210_set_cpufreq_level(NORMAL_TABLE);
#endif /* CONFIG_CPU_FREQ_S5PV210 */
	}
	mutex_unlock(mutex_for_fo);
	BASEPRINTK("()--\n");
	return 0;
}

/*
 *  Resume
 */
int s5p_tv_late_resume(struct platform_device *dev)
{
    BASEPRINTK("(hpd_status = %d)++\n", s5ptv_status.hpd_status);
	mutex_lock(mutex_for_fo);
	s5ptv_status.suspend_status = false;
    suspend_resume_sync = 2;
#ifdef CABLE_CHECK
    hpd_sleep_state =0;
#endif
    printk(KERN_INFO "s5p_tv_late_resume executing...\n");
	if(s5ptv_status.tvout_param.out_mode == TVOUT_OUTPUT_HDMI || s5ptv_status.tvout_param.out_mode == TVOUT_OUTPUT_HDMI_RGB
		|| s5ptv_status.tvout_param.out_mode == TVOUT_OUTPUT_DVI) {
		s5p_handle_cable();
    }
	
    if(!(s5ptv_status.hpd_status))
    {
		mutex_unlock(mutex_for_fo);
	    return 0;
    }
    else
    {
#ifdef CONFIG_CPU_FREQ_S5PV210
		s5pv210_set_cpufreq_level(RESTRICT_TABLE);
#endif /* CONFIG_CPU_FREQ_S5PV210 */

		/* clk & power on */
		TVout_LDO_ctrl(true);
		s5p_tv_clk_gate(true);
	if((s5ptv_status.tvout_param.out_mode == TVOUT_OUTPUT_HDMI || s5ptv_status.tvout_param.out_mode == TVOUT_OUTPUT_HDMI_RGB
		|| s5ptv_status.tvout_param.out_mode == TVOUT_OUTPUT_DVI)&&(s5ptv_status.hpd_status == HPD_HI))  
			tv_phy_power(true);

		/* tv on */
	if (s5ptv_status.tvout_output_enable) {
			_s5p_tv_if_start();
	}

		/* video layer start */
		if (s5ptv_status.vp_layer_enable)
			_s5p_vlayer_start();

		/* grp0 layer start */
		if (s5ptv_status.grp_layer_enable[0])
			_s5p_grp_start(VM_GPR0_LAYER);

		/* grp1 layer start */
		if (s5ptv_status.grp_layer_enable[1])
			_s5p_grp_start(VM_GPR1_LAYER);

#ifdef CONFIG_TV_FB
		if (s5ptv_status.tvout_output_enable) {
			s5ptvfb_display_on(&s5ptv_status);
			s5ptvfb_set_par(s5ptv_status.fb);
		}
#endif
	}
	mutex_unlock(mutex_for_fo);
	BASEPRINTK("()--\n");
    return 0;
}
#else
#define s5p_tv_suspend NULL
#define s5p_tv_resume NULL
#endif

#ifdef CONFIG_PM
#ifdef CONFIG_HAS_EARLYSUSPEND
static struct early_suspend s5p_tv_early_suspend_desc = {
     .level = EARLY_SUSPEND_LEVEL_STOP_DRAWING,
     .suspend = s5p_tv_early_suspend,
     .resume = s5p_tv_late_resume,
};
#endif
#endif

static struct platform_driver s5p_tv_driver = {
	.probe		= s5p_tv_probe,
	.remove		= s5p_tv_remove,
#ifdef CONFIG_PM
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend	= s5p_tv_early_suspend,
	.resume		= s5p_tv_late_resume,
#endif
#else
	.suspend	= NULL,
	.resume		= NULL,
#endif
	.driver		= {
		.name	= "s5p-tvout",
		.owner	= THIS_MODULE,
	},
};

static char banner[] __initdata =
	KERN_INFO "S5P TVOUT Driver, (c) 2010 Samsung Electronics\n";

int __init s5p_tv_init(void)
{
	int ret;
#if defined (CONFIG_TARGET_LOCALE_EUR) || defined (CONFIG_TARGET_LOCALE_HKTW) || defined (CONFIG_TARGET_LOCALE_HKTW_FET) || defined(CONFIG_TARGET_LOCALE_VZW) || defined (CONFIG_TARGET_LOCALE_USAGSM)
	if(HWREV < 0x8)
		return -1;
#endif	

	printk(banner);

	ret = platform_driver_register(&s5p_tv_driver);

	if (ret) {
		printk(KERN_ERR "Platform Device Register Failed %d\n", ret);
		return -1;
	}

#ifdef CONFIG_PM
#ifdef CONFIG_HAS_EARLYSUSPEND
    register_early_suspend(&s5p_tv_early_suspend_desc);
#endif
#endif

#ifdef CABLE_CHECK
	/* file creation at '/sys/class/sec_setprio/sec_setprio  */
	sec_hdmi = class_create(THIS_MODULE, "sec_hdmi");
	if (IS_ERR(sec_hdmi))
		pr_err("Failed to create class (sec_hdmi)\n");

	detect_cable = device_create(sec_hdmi, NULL, 0, NULL,
                                             "detect_cable");
	if (IS_ERR(detect_cable))
		pr_err("Failed to create device(detect_cable)!\n");
	if (device_create_file(detect_cable, &dev_attr_connected) < 0)
		pr_err("Failed to create device file(%s)!\n",
			dev_attr_connected.attr.name);

#endif
	return 0;
}

static void __exit s5p_tv_exit(void)
{
#ifdef CONFIG_PM
#ifdef CONFIG_HAS_EARLYSUSPEND
    unregister_early_suspend(&s5p_tv_early_suspend_desc);
#endif
#endif
	platform_driver_unregister(&s5p_tv_driver);
}

late_initcall(s5p_tv_init);
module_exit(s5p_tv_exit);

MODULE_AUTHOR("SangPil Moon");
MODULE_DESCRIPTION("SS5PC1XX TVOUT driver");
MODULE_LICENSE("GPL");
