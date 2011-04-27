/*
 * linux/swi.h
 *
 * Copyright (C) 2009 Samsung Electronics Co.Ltd
 *	InKi Dae <inki.dae@samsung.com>
 *
 * Single-Wired Interface header definitions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __LINUX_SWI_H_
#define __LINUX_SWI_H_

#include <linux/device.h>

/*
 * It controls the status of a device using single-wired interface.
 *
 * SWI_LOW - signal low.
 * SWI_HIGH - signal high.
 * SWI_CHANGE - toggle signal low to high.
 */
enum {
	SWI_LOW = 0,
	SWI_HIGH,
	SWI_CHANGE
};

/*
 * Single-Wired Interface board information definitions.
 *
 * @name : device name.
 * @controller_data : control pin for seding signal. (gpio pin)
 * @low_period : low period of signal. (usecond)
 * @high_period : high period of signal. (usecond)
 * @init : function pointer for initializing backlight driver.
 */
struct swi_board_info {
	char *name;
	unsigned int controller_data;
	unsigned int low_period;
	unsigned int high_period;
	void (*init) (void);
};

/*
 * Single-Wired Interface device definitions.
 *
 * @dev : Driver model representation of the device.
 * @swi_drv : Pointer of swi_driver.
 * @id : id of device registered. when device is registered, id is counted.
 * @controller_state : Controller's runtime state.
 * @modalias: Name of the driver to use with this device, or an alias
 * 	for that name.
 * @bi : hardware specific infomation.
 */
struct swi_device {
	struct device		dev;
	struct swi_driver	*swi_drv;
	int			id;
	void			*controller_state;
	char			modalias[64];

	struct swi_board_info	*bi;
	int (*transfer) (struct swi_device *swi_dev, unsigned int command);
};

static inline struct swi_device *to_swi_device(struct device *dev)
{
	return dev ? container_of(dev, struct swi_device, dev) : NULL;
}

static inline struct swi_device *swi_dev_get(struct swi_device *swi_dev)
{
	return (swi_dev && get_device(&swi_dev->dev)) ? swi_dev : NULL;
}

static inline void swi_dev_put(struct swi_device *swi_dev)
{
	if (swi_dev)
		put_device(&swi_dev->dev);
}

static inline void *swi_get_ctldata(struct swi_device *swi_dev)
{
	return swi_dev->controller_state;
}

static inline void swi_set_ctldata(struct swi_device *swi_dev, void *state)
{
	swi_dev->controller_state = state;
}

static inline void swi_unregister_device(struct swi_device *swi_dev)
{
	if (swi_dev)
		device_unregister(&swi_dev->dev);
}

extern struct swi_device *swi_alloc_device(void);

extern int swi_add_device(struct swi_device *swi_dev);

extern struct swi_device *swi_new_device(struct swi_board_info *);


/*
 * Single-Wired Interface driver definitions.
 *
 * @driver : SWI device driver should initialize the name and owner field
 * 	this structure.
 * @probe : binds tis driver to the swi device.
 * @remove : unbinds this driver from the swi device.
 * @shutdown : standard shutdown callback used during system state transitions.
 * @suspend : standard suspend callback used during system state transitions.
 * @resume : standard resume callback used during system state transitions.
 */
struct swi_driver {
	struct	device_driver driver;

	int	(*probe)(struct swi_device *swi_dev);
	int	(*remove)(struct swi_device *swi_dev);
	void	(*shutdown)(struct swi_device *swi_dev);
	int	(*suspend)(struct swi_device *swi_dev, pm_message_t mesg);
	int	(*resume)(struct swi_device *swi_dev);
};

void swi_set_drvdata(struct swi_device *swi_dev, void *data);
void *swi_get_drvdata(struct swi_device *swi_dev);


static inline struct swi_driver *to_swi_driver(struct device_driver *drv)
{
	return drv ? container_of(drv, struct swi_driver, driver) : NULL;
}

static inline void swi_unregister_driver(struct swi_driver *swi_drv)
{
	if (swi_drv)
		driver_unregister(&swi_drv->driver);
}

extern int swi_register_driver(struct swi_driver *swi_drv);

#ifdef CONFIG_SWI
extern int swi_register_board_info(struct swi_board_info const *info,
	unsigned int n);
#else
static int swi_register_board_info(struct swi_board_info const *info,
	unsigned int n)
	{ return 0; }
#endif


#endif /*__LINUX_SWI_H_*/
