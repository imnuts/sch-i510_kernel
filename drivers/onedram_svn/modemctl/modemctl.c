/**
 * header for modem control
 *
 * Copyright (C) 2010 Samsung Electronics. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

//#define DEBUG

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/kdev_t.h>
#include <linux/workqueue.h>

#include <linux/modemctl.h>


#define DRVNAME "modemctl"

struct modemctl;

struct modemctl_ops {
	void (*modem_on)(struct modemctl *);
	void (*modem_off)(struct modemctl *);
	void (*modem_reset)(struct modemctl *);
	void (*modem_boot)(struct modemctl *);
};

struct modemctl_info {
	const char *name;
	struct modemctl_ops ops;
};

struct modemctl {
	int irq;

	unsigned gpio_phone_on;
	unsigned gpio_phone_active;
	unsigned gpio_pda_active;
	unsigned gpio_cp_reset;
	unsigned gpio_usim_boot;
	unsigned gpio_flm_sel;

	struct modemctl_ops *ops;

	struct class *class;
	struct device *dev;
	const struct attribute_group *group;

	struct work_struct work;
};


static void xmm_on(struct modemctl *);
static void xmm_off(struct modemctl *);
static void xmm_reset(struct modemctl *);
static void xmm_boot(struct modemctl *);

static void lte_on(struct modemctl *);
static void lte_reset(struct modemctl *);

static struct modemctl_info mdmctl_info[] = {
	{
		.name = "xmm",
		.ops = {
			.modem_on = xmm_on,
			.modem_off = xmm_off,
			.modem_reset = xmm_reset,
			.modem_boot = xmm_boot,
		},
	},
	{
		.name = "lte",
		.ops = {
			.modem_on = lte_on,
			.modem_reset = lte_reset,
		},
	},
};


static ssize_t show_control(struct device *d,
		struct device_attribute *attr, char *buf);
static ssize_t store_control(struct device *d,
		struct device_attribute *attr, const char *buf, size_t count);
static ssize_t show_status(struct device *d,
		struct device_attribute *attr, char *buf);
static ssize_t show_debug(struct device *d,
		struct device_attribute *attr, char *buf);

static DEVICE_ATTR(control, S_IRUGO | S_IWUGO, show_control, store_control);
static DEVICE_ATTR(status, S_IRUGO, show_status, NULL);
static DEVICE_ATTR(debug, S_IRUGO, show_debug, NULL);

static struct attribute *modemctl_attributes[] = {
	&dev_attr_control.attr,
	&dev_attr_status.attr,
	&dev_attr_debug.attr,
	NULL
};

static const struct attribute_group modemctl_group = {
	.attrs = modemctl_attributes,
};

static void lte_on(struct modemctl *mc)
{
	dev_dbg(mc->dev, "%s\n", __func__);
	if(!mc->gpio_phone_on)
		return;

	gpio_set_value(mc->gpio_phone_on, 1);
	msleep(100);
	/* PWRON keep HIGH */
}

static void lte_reset(struct modemctl *mc)
{
	int r;

	dev_dbg(mc->dev, "%s\n", __func__);
	if(!mc->gpio_phone_on || !mc->gpio_cp_reset)
		return;

//	r = gpio_get_value(mc->gpio_phone_on);
//	if (r == 0)
//		lte_on(mc);

	gpio_set_value(mc->gpio_phone_on, 0);
	gpio_set_value(mc->gpio_cp_reset, 0);
	msleep(10);
	gpio_set_value(mc->gpio_phone_on, 1);
	msleep(20);
	gpio_set_value(mc->gpio_cp_reset, 1);
}

static void xmm_on(struct modemctl *mc)
{
	dev_dbg(mc->dev, "%s\n", __func__);
	if(!mc->gpio_cp_reset || !mc->gpio_phone_on)
		return;

	gpio_set_value(mc->gpio_cp_reset, 1);
	gpio_set_value(mc->gpio_phone_on, 1);
	gpio_set_value(mc->gpio_cp_reset, 0);
	msleep(10);  /* no spec, confirm later exactly how much time
			needed to initialize CP with RESET_PMU_N */

	gpio_set_value(mc->gpio_cp_reset, 1);
	/* Follow RESET timming delay not Power-On timming,
	   because CP_RST & PHONE_ON have been set high already. */
	// msleep(30);  /* > 26.6 + 2 msec */
	msleep(40);     /* > 37.2 + 2 msec */
}

static void xmm_off(struct modemctl *mc)
{
	dev_dbg(mc->dev, "%s\n", __func__);
	if(!mc->gpio_cp_reset || !mc->gpio_phone_on)
		return;

	gpio_set_value(mc->gpio_phone_on, 0);
	gpio_set_value(mc->gpio_cp_reset, 0);
}

static void xmm_reset(struct modemctl *mc)
{
	dev_dbg(mc->dev, "%s\n", __func__);
	if(!mc->gpio_cp_reset || !mc->gpio_phone_on)
		return;

	/* To Do :
	 * hard_reset(RESET_PMU_N) and soft_reset(RESET_REQ_N)
	 * should be divided later.
	 * soft_reset is used for CORE_DUMP
	 */
	gpio_set_value(mc->gpio_cp_reset, 0);
	msleep(10); /* no spec, confirm later exactly how much time
		       needed to initialize CP with RESET_PMU_N */
	gpio_set_value(mc->gpio_cp_reset, 1);
	msleep(40); /* > 37.2 + 2 msec */

	gpio_set_value(mc->gpio_phone_on, 0);
	gpio_set_value(mc->gpio_cp_reset, 0);
}

static void xmm_boot(struct modemctl *mc)
{
	dev_dbg(mc->dev, "%s\n", __func__);

	if(mc->gpio_usim_boot)
		gpio_set_value(mc->gpio_usim_boot, 1);

	if(mc->gpio_flm_sel)
		gpio_set_value(mc->gpio_flm_sel, 0);
}

static int modem_on(struct modemctl *mc)
{
	dev_dbg(mc->dev, "%s\n", __func__);
	if(!mc->ops || !mc->ops->modem_on) {
		// 
		return -ENXIO;
	}

	mc->ops->modem_on(mc);

	return 0;
}

static int modem_off(struct modemctl *mc)
{
	dev_dbg(mc->dev, "%s\n", __func__);
	if(!mc->ops || !mc->ops->modem_off) {
		// 
		return -ENXIO;
	}

	mc->ops->modem_off(mc);

	return 0;
}

static int modem_reset(struct modemctl *mc)
{
	dev_dbg(mc->dev, "%s\n", __func__);
	if(!mc->ops || !mc->ops->modem_reset) {
		// 
		return -ENXIO;
	}

	mc->ops->modem_reset(mc);

	return 0;
}


static int modem_boot(struct modemctl *mc)
{
	dev_dbg(mc->dev, "%s\n", __func__);
	if(!mc->ops || !mc->ops->modem_boot) {
		// 
		return -ENXIO;
	}

	mc->ops->modem_boot(mc);

	return 0;
}

static int pda_on(struct modemctl *mc)
{
	dev_dbg(mc->dev, "%s\n", __func__);
	if(!mc->gpio_pda_active) {

		return -ENXIO;
	}

	gpio_set_value(mc->gpio_pda_active, 1);
	return 0;
}

static int pda_off(struct modemctl *mc)
{
	dev_dbg(mc->dev, "%s\n", __func__);
	if(!mc->gpio_pda_active) {

		return -ENXIO;
	}

	gpio_set_value(mc->gpio_pda_active, 0);
	return 0;
}

static int modem_get_active(struct modemctl *mc)
{
	dev_dbg(mc->dev, "%s\n", __func__);
	if(!mc->gpio_phone_active || !mc->gpio_cp_reset)
		return -ENXIO;

	dev_dbg(mc->dev, "cp %d phone %d\n",
			gpio_get_value(mc->gpio_cp_reset),
			gpio_get_value(mc->gpio_phone_active));

	if(gpio_get_value(mc->gpio_cp_reset))
		return !!gpio_get_value(mc->gpio_phone_active);

	return 0;
}

static ssize_t show_control(struct device *d,
		struct device_attribute *attr, char *buf)
{
	char *p = buf;
	struct modemctl *mc = dev_get_drvdata(d);
	struct modemctl_ops *ops = mc->ops;

	if(ops) {
		if(ops->modem_on)
			p += sprintf(p, "on ");
		if(ops->modem_off)
			p += sprintf(p, "off ");
		if(ops->modem_reset)
			p += sprintf(p, "reset ");
		if(ops->modem_boot)
			p += sprintf(p, "boot ");
	} else {
		p += sprintf(p, "(No ops)");
	}

	p += sprintf(p, "\n");
	return p - buf;
}

static ssize_t store_control(struct device *d,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct modemctl *mc = dev_get_drvdata(d);

	if(!strncmp(buf, "on", 2)) {
		modem_on(mc);
		return count;
	}

	if(!strncmp(buf, "off", 3)) {
		modem_off(mc);
		return count;
	}

	if(!strncmp(buf, "reset", 5)) {
		modem_reset(mc);
		return count;
	}

	if(!strncmp(buf, "boot", 4)) {
		modem_boot(mc);
		return count;
	}

	return count;
}

static ssize_t show_status(struct device *d,
		struct device_attribute *attr, char *buf)
{
	char *p = buf;
	struct modemctl *mc = dev_get_drvdata(d);

	p += sprintf(p, "%d\n", modem_get_active(mc));

	return p - buf;
}

static ssize_t show_debug(struct device *d,
		struct device_attribute *attr, char *buf)
{
	char *p = buf;
	int i;
	struct modemctl *mc = dev_get_drvdata(d);

	if(mc->irq)
		p += sprintf(p, "Irq: %d\n", mc->irq);

	p += sprintf(p, "GPIO ---- \n");

	if(mc->gpio_phone_on)
		p += sprintf(p, "\t%3d %d : phone on\n", mc->gpio_phone_on,
				gpio_get_value(mc->gpio_phone_on));
	if(mc->gpio_phone_active)
		p += sprintf(p, "\t%3d %d : phone active\n", mc->gpio_phone_active,
				gpio_get_value(mc->gpio_phone_active));
	if(mc->gpio_pda_active)
		p += sprintf(p, "\t%3d %d : pda active\n", mc->gpio_pda_active,
				gpio_get_value(mc->gpio_pda_active));
	if(mc->gpio_cp_reset)
		p += sprintf(p, "\t%3d %d : CP reset\n", mc->gpio_cp_reset,
				gpio_get_value(mc->gpio_cp_reset));
	if(mc->gpio_usim_boot)
		p += sprintf(p, "\t%3d %d : USIM boot\n", mc->gpio_usim_boot,
				gpio_get_value(mc->gpio_usim_boot));
	if(mc->gpio_flm_sel)
		p += sprintf(p, "\t%3d %d : FLM sel\n", mc->gpio_flm_sel,
				gpio_get_value(mc->gpio_flm_sel));

	p += sprintf(p, "Support types --- \n");
	for(i=0;i<ARRAY_SIZE(mdmctl_info);i++) {
		if(mc->ops == &mdmctl_info[i].ops) {
			p += sprintf(p, "\t * ");
		} else {
			p += sprintf(p, "\t   ");
		}
		p += sprintf(p, "%s\n", mdmctl_info[i].name);
	}

	return p - buf;
}

static void mc_work(struct work_struct *work)
{
	struct modemctl *mc = container_of(work, struct modemctl, work);
	int r;

	r = modem_get_active(mc);
	if (r < 0) {
		dev_err(mc->dev, "Not initialized\n");
		return;
	}

	dev_info(mc->dev, "PHONE ACTIVE: %d\n", r);

	if (r)
		kobject_uevent(&mc->dev->kobj, KOBJ_ONLINE);
	else
		kobject_uevent(&mc->dev->kobj, KOBJ_OFFLINE);
}

static irqreturn_t modemctl_irq_handler(int irq, void *dev_id)
{
	struct modemctl *mc = (struct modemctl *)dev_id;

	if (!work_pending(&mc->work))
		schedule_work(&mc->work);

	return IRQ_HANDLED;
}

static struct modemctl_ops* _find_ops(const char *name)
{
	int i;
	struct modemctl_ops *ops = NULL;

	for(i=0;i<ARRAY_SIZE(mdmctl_info);i++) {
		if(mdmctl_info[i].name && !strcmp(name, mdmctl_info[i].name))
			ops = &mdmctl_info[i].ops;
	}

	return ops;
}

static void _free_all(struct modemctl *mc)
{
	if(mc) {
		if(mc->ops)
			mc->ops = NULL;

		if(mc->group)
			sysfs_remove_group(&mc->dev->kobj, mc->group);

		if(mc->irq)
			free_irq(mc->irq, mc);

		if(mc->dev)
			device_destroy(mc->class, mc->dev->devt);

		if(mc->class)
			class_destroy(mc->class);

		kfree(mc);
	}
}

static int __devinit modemctl_probe(struct platform_device *pdev)
{
	struct modemctl *mc = NULL;
	struct modemctl_platform_data *pdata;
	struct resource *res;
	int r = 0;
	int irq;

	dev_info(&pdev->dev, "[%s]\n", __func__);
	pdata = pdev->dev.platform_data;
	if(!pdata || !pdata->cfg_gpio) {
		dev_err(&pdev->dev, "No platform data\n");
		r = -EINVAL;
		goto err;
	}

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if(!res)  {
		dev_err(&pdev->dev, "failed to get irq number\n");
		r = -EINVAL;
		goto err;
	}
	irq = res->start;

	mc = kzalloc(sizeof(struct modemctl), GFP_KERNEL);
	if(!mc) {
		dev_err(&pdev->dev, "failed to allocate device\n");
		r = -ENOMEM;
		goto err;
	}

	mc->gpio_phone_on = pdata->gpio_phone_on;
	mc->gpio_phone_active = pdata->gpio_phone_active;
	mc->gpio_pda_active = pdata->gpio_pda_active;
	mc->gpio_cp_reset = pdata->gpio_cp_reset;
	mc->gpio_usim_boot = pdata->gpio_usim_boot;
	mc->gpio_flm_sel = pdata->gpio_flm_sel;

	mc->ops = _find_ops(pdata->name);
	if(!mc->ops) {
		dev_err(&pdev->dev, "can't find operations: %s\n", pdata->name);
		goto err;
	}

	mc->class = class_create(THIS_MODULE, "modemctl");
	if(IS_ERR(mc->class)) {
		dev_err(&pdev->dev, "failed to create sysfs class\n");
		r = PTR_ERR(mc->class);
		mc->class = NULL;
		goto err;
	}

	pdata->cfg_gpio();

	mc->dev = device_create(mc->class, &pdev->dev, MKDEV(0, 0), NULL, "%s", pdata->name);
	if(IS_ERR(mc->dev)) {
		dev_err(&pdev->dev, "failed to create device\n");
		r = PTR_ERR(mc->dev);
		goto err;
	}
	dev_set_drvdata(mc->dev, mc);

	r = sysfs_create_group(&mc->dev->kobj, &modemctl_group);
	if(r) {
		dev_err(&pdev->dev, "failed to create sysfs files\n");
		goto err;
	}
	mc->group = &modemctl_group;

	INIT_WORK(&mc->work, mc_work);

	r = request_irq(irq, modemctl_irq_handler,
			IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
			"phone_active", mc);
	if(r) {
		dev_err(&pdev->dev, "failed to allocate an interrupt(%d) (%d)\n",
				irq, r);
		goto err;
	}
	mc->irq = irq;

	platform_set_drvdata(pdev, mc);
	return 0;

err:
	_free_all(mc);
	return r;
}

static int __devexit modemctl_remove(struct platform_device *pdev)
{
	struct modemctl *mc = platform_get_drvdata(pdev);

	flush_work(&mc->work);
	platform_set_drvdata(pdev, NULL);
	_free_all(mc);
	return 0;
}

#ifdef CONFIG_PM
static int modemctl_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct modemctl *mc = platform_get_drvdata(pdev);

	pda_off(mc);

	return 0;
}

static int modemctl_resume(struct platform_device *pdev)
{
	struct modemctl *mc = platform_get_drvdata(pdev);

	pda_on(mc);

	return 0;
}
#else
#  define modemctl_suspend NULL
#  define modemctl_resume NULL
#endif

static struct platform_driver modemctl_driver = {
	.probe = modemctl_probe,
	.remove = __devexit_p(modemctl_remove),
	.suspend = modemctl_suspend,
	.resume = modemctl_resume,
	.driver = {
		.name = DRVNAME,
	},
};

static int __init modemctl_init(void)
{
	return platform_driver_register(&modemctl_driver);
}

static void __exit modemctl_exit(void)
{
	platform_driver_unregister(&modemctl_driver);
}

module_init(modemctl_init);
module_exit(modemctl_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Suchang Woo <suchang.woo@samsung.com>");
MODULE_DESCRIPTION("Modem control");
