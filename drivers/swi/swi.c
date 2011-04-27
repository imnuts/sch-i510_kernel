/*
 * linux/driver/swi/swi.c 
 *
 * Single-Wired Interface Driver.
 *
 * Copyright (C) 2009 Samsung Electronics Co.Ltd
 *	InKi Dae <inki.dae@samsung.com>
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/cache.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/gpio.h>
#include <linux/swi.h>

#include <mach/map.h>

#include <plat/gpio-cfg.h>


/* 
 * manage boardinfo registered by board file as link list.
 *
 * @list : linked list head.
 * @n_board_info : boardinfo count.
 * @board_info: instance of boardinfo registered.
 */
struct boardinfo {
	struct list_head	list;
	unsigned int		n_board_info;
	struct swi_board_info	board_info[0];
};

static LIST_HEAD(board_list);
static DEFINE_MUTEX(board_lock);

static int swi_transfer_command(struct swi_device *swi_dev,
	unsigned int command)
{
	struct swi_board_info *bi = NULL;

	bi = swi_dev->bi;

	switch (command) {
	case SWI_LOW:
		gpio_set_value(bi->controller_data, 0);
		break;
	case SWI_HIGH:
		gpio_set_value(bi->controller_data, 1);
		break;
	case SWI_CHANGE:
		gpio_direction_output(bi->controller_data, 0);
		udelay(bi->low_period);

		gpio_set_value(bi->controller_data, 1);
		udelay(bi->high_period);
		break;
	default:
		printk("invalid command.\n");
		return 0;
	}

	return 1;
}

void swi_set_drvdata(struct swi_device *swi_dev, void *data)
{
	dev_set_drvdata(&swi_dev->dev, data);
}

void *swi_get_drvdata(struct swi_device *swi_dev)
{
	return dev_get_drvdata(&swi_dev->dev);
}

static ssize_t
modalias_show(struct device *dev, struct device_attribute *a, char *buf)
{
	const struct swi_device *swi_dev = to_swi_device(dev);

	return sprintf(buf, "%s\n", swi_dev->modalias);
}

static struct device_attribute swi_dev_attrs[] = {
	__ATTR_RO(modalias),
	__ATTR_NULL,
};

static int swi_match_device(struct device *dev, struct device_driver *drv)
{
	const struct swi_device	*swi_dev = to_swi_device(dev);

	return strcmp(swi_dev->modalias, drv->name) == 0;
}

#ifdef CONFIG_PM
static int swi_bus_suspend(struct device *dev, pm_message_t state)
{
	int ret = 0;
	struct swi_device	*swi_dev = to_swi_device(dev);
	const struct swi_driver *swi_drv = swi_dev->swi_drv;

	if (swi_drv)
		if (swi_drv->suspend)
			ret = swi_drv->suspend(swi_dev, state);
		else
			dev_err(dev, "failed to suspend.\n");
	else
		dev_warn(dev, "swi_drv is null.\n");

	return ret;
}

static int swi_bus_resume(struct device *dev)
{
	int ret = 0;
	struct swi_device	*swi_dev = to_swi_device(dev);
	const struct swi_driver *swi_drv = swi_dev->swi_drv;

	if (swi_drv)
		if (swi_drv->resume)
			ret = swi_drv->resume(swi_dev);
		else
			dev_err(dev, "failed to resume.\n");
	else
		dev_warn(dev, "swi_drv is null.\n");

	return ret;
}
#endif

struct bus_type swi_bus_type = {
	.name		= "swi",
	.dev_attrs	= swi_dev_attrs,
	.match		= swi_match_device,
#ifdef CONFIG_PM
	.suspend	= swi_bus_suspend,
	.resume		= swi_bus_resume,
#endif
};


static int swi_drv_probe(struct device *dev)
{
	const struct swi_driver *swi_drv = to_swi_driver(dev->driver);

	return swi_drv->probe(to_swi_device(dev));
}

static int swi_drv_remove(struct device *dev)
{
	const struct swi_driver	*swi_drv = to_swi_driver(dev->driver);

	return swi_drv->remove(to_swi_device(dev));
}

static void swi_drv_shutdown(struct device *dev)
{
	const struct swi_driver	*swi_drv = to_swi_driver(dev->driver);

	swi_drv->shutdown(to_swi_device(dev));
}

/*
 * register SWI devices for a given board.
 *
 * @info : array of chip descriptors.
 * @n : how many descriptors are provided.
 */
int __init swi_register_board_info(struct swi_board_info const *info, unsigned int n)
{
	struct boardinfo	*bi;

	bi = kmalloc(sizeof(*bi) + n * sizeof *info, GFP_KERNEL);
	if (!bi)
		return -ENOMEM;
	bi->n_board_info = n;
	memcpy(bi->board_info, info, n * sizeof *info);

	mutex_lock(&board_lock);
	list_add_tail(&bi->list, &board_list);
	mutex_unlock(&board_lock);

	return 0;
}

/*
 * search board_info registerd to list.
 *
 * @swi_drv : SWI driver.
 */
struct swi_board_info *scan_boardinfo(struct swi_driver *swi_drv)
{
	struct boardinfo	*bi;

	mutex_lock(&board_lock);
	list_for_each_entry(bi, &board_list, list) {
		struct swi_board_info	*chip = bi->board_info;
		unsigned		n;

		for (n = bi->n_board_info; n > 0; n--, chip++) {
			if ((strcmp(chip->name, swi_drv->driver.name)) == 0) {
				mutex_unlock(&board_lock);
				printk("it found swi_board_info(%s)\n",
					swi_drv->driver.name);
				return chip;
			}
		}
	}

	mutex_unlock(&board_lock);

	printk(KERN_WARNING "it can't find the swi_board_info(%s)\n",
		swi_drv->driver.name);

	return NULL;
}

/**
 * It registers a SWI driver to kernel and called by user-defined device driver.
 *
 * @swi_drv: the driver to register
 */
int swi_register_driver(struct swi_driver *swi_drv)
{
	struct swi_board_info *bi;
	struct swi_device *swi_dev;

	bi = scan_boardinfo(swi_drv);
	if (bi == NULL) {
		printk(KERN_ERR "failed to scan board_info.\n");

		return 0;
	}

	swi_dev = swi_new_device(bi);
	swi_dev->swi_drv = swi_drv;
	swi_dev->bi = bi;
	swi_dev->transfer = swi_transfer_command;

	swi_drv->driver.bus = &swi_bus_type;
	if (swi_drv->probe)
		swi_drv->driver.probe = swi_drv_probe;
	if (swi_drv->remove)
		swi_drv->driver.remove = swi_drv_remove;
	if (swi_drv->shutdown)
		swi_drv->driver.shutdown = swi_drv_shutdown;

	return driver_register(&swi_drv->driver);
}
EXPORT_SYMBOL_GPL(swi_register_driver);

struct swi_device *swi_alloc_device(void)
{
	struct swi_device	*swi_dev;

	swi_dev = kzalloc(sizeof *swi_dev, GFP_KERNEL);
	if (!swi_dev) {
		printk(KERN_ERR "cannot alloc swi_device\n");

		return NULL;
	}

	device_initialize(&swi_dev->dev);

	return swi_dev;
}

int swi_add_device(struct swi_device *swi_dev)
{
	static DEFINE_MUTEX(swi_add_lock);
	int status;

	swi_dev->dev.bus = &swi_bus_type;

	/* Set the bus ID string */
	dev_set_name(&swi_dev->dev, "swi.%d", swi_dev->id);

	mutex_lock(&swi_add_lock);

	/* Device may be bound to an active driver when this returns */
	status = device_add(&swi_dev->dev);
	if (status < 0)
		printk(KERN_ERR "can't %s %s, status %d\n",
				"add", dev_name(&swi_dev->dev), status);
	else
		printk(KERN_INFO "registered child %s\n", dev_name(&swi_dev->dev));

	mutex_unlock(&swi_add_lock);

	return status;
}

struct swi_device *swi_new_device(struct swi_board_info *chip)
{
	struct swi_device	*proxy;
	int			status;
	static unsigned int	id = 1;

	proxy = swi_alloc_device();
	if (!proxy)
		return NULL;

	proxy->id = id++;
	strcpy(proxy->modalias, chip->name);
	proxy->bi = chip;

	status = swi_add_device(proxy);
	if (status < 0) {
		swi_dev_put(proxy);
		return NULL;
	}

	return proxy;
}

static int __init swi_init(void)
{
	int status;

	status = bus_register(&swi_bus_type);
	if (status < 0)
		return status;

	return status;
}

postcore_initcall(swi_init);
