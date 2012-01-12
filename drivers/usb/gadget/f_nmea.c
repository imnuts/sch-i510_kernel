/*
 * f_nmea.c - generic USB serial function driver (modified from f_serial.c)
 * ttygs2
 *
 * Copyright (C) 2003 Al Borchers (alborchers@steinerpoint.com)
 * Copyright (C) 2008 by David Brownell
 * Copyright (C) 2008 by Nokia Corporation
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/usb/android_composite.h>

#include "u_serial.h"
#include "gadget_chips.h"

/*
 * This function packages a simple "generic serial" port with no real
 * control mechanisms, just raw data transfer over two bulk endpoints.
 *
 * Because it's not standardized, this isn't as interoperable as the
 * CDC ACM driver.  However, for many purposes it's just as functional
 * if you can arrange appropriate host side drivers.
 */

struct nmea_descs {
	struct usb_endpoint_descriptor	*in;
	struct usb_endpoint_descriptor	*out;
};

struct f_nmea {
	struct gserial			port;
	u8				data_id;
	u8				port_num;

	struct nmea_descs		fs;
	struct nmea_descs		hs;
};

static inline struct f_nmea *func_to_nmea(struct usb_function *f)
{
	return container_of(f, struct f_nmea, port.func);
}

/*-------------------------------------------------------------------------*/

/* interface descriptor: */

static struct usb_interface_descriptor nmea_interface_desc  = {
	.bLength =		USB_DT_INTERFACE_SIZE,
	.bDescriptorType =	USB_DT_INTERFACE,
	/* .bInterfaceNumber = DYNAMIC */
	.bNumEndpoints =	2,
	.bInterfaceClass =	USB_CLASS_VENDOR_SPEC,
	.bInterfaceSubClass =	0,
	.bInterfaceProtocol =	0,
	/* .iInterface = DYNAMIC */
};

/* full speed support: */

static struct usb_endpoint_descriptor nmea_fs_in_desc  = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

static struct usb_endpoint_descriptor nmea_fs_out_desc  = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

static struct usb_descriptor_header *nmea_fs_function[]  = {
	(struct usb_descriptor_header *) &nmea_interface_desc,
	(struct usb_descriptor_header *) &nmea_fs_in_desc,
	(struct usb_descriptor_header *) &nmea_fs_out_desc,
	NULL,
};

/* high speed support: */

static struct usb_endpoint_descriptor nmea_hs_in_desc  = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	__constant_cpu_to_le16(512),
};

static struct usb_endpoint_descriptor nmea_hs_out_desc  = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	__constant_cpu_to_le16(512),
};

static struct usb_descriptor_header *nmea_hs_function[]  = {
	(struct usb_descriptor_header *) &nmea_interface_desc,
	(struct usb_descriptor_header *) &nmea_hs_in_desc,
	(struct usb_descriptor_header *) &nmea_hs_out_desc,
	NULL,
};

/* string descriptors: */
#define F_nmea_IDX	0

static struct usb_string nmea_string_defs[] = {
//	[0].s = "Generic Serial",
	[F_nmea_IDX].s = "Samsung Android NMEA",		
	{  /* ZEROES END LIST */ },
};

static struct usb_gadget_strings nmea_string_table = {
	.language =		0x0409,	/* en-us */
	.strings =		nmea_string_defs,
};

static struct usb_gadget_strings *nmea_strings[] = {
	&nmea_string_table,
	NULL,
};

/*-------------------------------------------------------------------------*/

static int nmea_set_alt(struct usb_function *f, unsigned intf, unsigned alt)
{
	struct f_nmea		*nmea = func_to_nmea(f);
	struct usb_composite_dev *cdev = f->config->cdev;

	/* we know alt == 0, so this is an activation or a reset */

	if (nmea->port.in->driver_data) {
		DBG(cdev, "reset generic ttyGS%d\n", nmea->port_num);
		gserial_disconnect(&nmea->port);
	} else {
		DBG(cdev, "activate generic ttyGS%d\n", nmea->port_num);
	}
        nmea->port.in_desc = ep_choose(cdev->gadget,
                        nmea->hs.in, nmea->fs.in);
        nmea->port.out_desc = ep_choose(cdev->gadget,
                        nmea->hs.out, nmea->fs.out);             
	gserial_connect(&nmea->port, nmea->port_num);
	return 0;
}

static void nmea_disable(struct usb_function *f)
{
	struct f_nmea	*nmea = func_to_nmea(f);
	struct usb_composite_dev *cdev = f->config->cdev;

	DBG(cdev, "generic ttyGS%d deactivated\n", nmea->port_num);
	gserial_disconnect(&nmea->port);
}

/*-------------------------------------------------------------------------*/

/* serial function driver setup/binding */

static int __init
nmea_bind(struct usb_configuration *c, struct usb_function *f)
{
	struct usb_composite_dev *cdev = c->cdev;
	struct f_nmea		*nmea = func_to_nmea(f);
	int			status;
	struct usb_ep		*ep;

	/* allocate instance-specific interface IDs */
	status = usb_interface_id(c, f);
	if (status < 0)
		goto fail;
	nmea->data_id = status;
	nmea_interface_desc.bInterfaceNumber = status;

	status = -ENODEV;

	/* allocate instance-specific endpoints */
#ifdef CONFIG_USB_ANDROID_SHARE_ENDPOINT
        ep = usb_ep_fixedconfig_alloc(cdev->gadget, &nmea_fs_in_desc);  
#else
        ep = usb_ep_autoconfig(cdev->gadget, &nmea_fs_in_desc);
#endif
	if (!ep)
		goto fail;
	nmea->port.in = ep;
	ep->driver_data = cdev;	/* claim */
#ifdef  CONFIG_USB_ANDROID_SHARE_ENDPOINT
        ep = usb_ep_fixedconfig_alloc(cdev->gadget, &nmea_fs_out_desc);
#else
        ep = usb_ep_autoconfig(cdev->gadget, &nmea_fs_out_desc);
#endif
	if (!ep)
		goto fail;
	nmea->port.out = ep;
	ep->driver_data = cdev;	/* claim */

	/* copy descriptors, and track endpoint copies */
	f->descriptors = usb_copy_descriptors(nmea_fs_function);

	nmea->fs.in = usb_find_endpoint(nmea_fs_function,
			f->descriptors, &nmea_fs_in_desc);
	nmea->fs.out = usb_find_endpoint(nmea_fs_function,
			f->descriptors, &nmea_fs_out_desc);


	/* support all relevant hardware speeds... we expect that when
	 * hardware is dual speed, all bulk-capable endpoints work at
	 * both speeds
	 */
	if (gadget_is_dualspeed(c->cdev->gadget)) {
		nmea_hs_in_desc.bEndpointAddress =
				nmea_fs_in_desc.bEndpointAddress;
		nmea_hs_out_desc.bEndpointAddress =
				nmea_fs_out_desc.bEndpointAddress;

		/* copy descriptors, and track endpoint copies */
		f->hs_descriptors = usb_copy_descriptors(nmea_hs_function);

		nmea->hs.in = usb_find_endpoint(nmea_hs_function,
				f->hs_descriptors, &nmea_hs_in_desc);
		nmea->hs.out = usb_find_endpoint(nmea_hs_function,
				f->hs_descriptors, &nmea_hs_out_desc);
	}

	DBG(cdev, "generic ttyGS%d: %s speed IN/%s OUT/%s\n",
			nmea->port_num,
			gadget_is_dualspeed(c->cdev->gadget) ? "dual" : "full",
			nmea->port.in->name, nmea->port.out->name);
	return 0;

fail:
	/* we might as well release our claims on endpoints */
	if (nmea->port.out)
		nmea->port.out->driver_data = NULL;
	if (nmea->port.in)
		nmea->port.in->driver_data = NULL;

	ERROR(cdev, "%s: can't bind, err %d\n", f->name, status);

	return status;
}

static void
nmea_unbind(struct usb_configuration *c, struct usb_function *f)
{
	if (gadget_is_dualspeed(c->cdev->gadget))
		usb_free_descriptors(f->hs_descriptors);
	usb_free_descriptors(f->descriptors);
	kfree(func_to_nmea(f));
}

#ifdef CONFIG_USB_ANDROID_SAMSUNG_COMPOSITE
static int nmea_set_interface_id(struct usb_function *f, int intf_num, int index_num)
{
	int ret = 0;
	struct f_nmea		*nmea = func_to_nmea(f);

	if (gadget_is_dualspeed(f->config->cdev->gadget)) {
                if (usb_change_interface_num(nmea_hs_function,
                	f->hs_descriptors, &nmea_interface_desc,
                	intf_num)) {
                	nmea->data_id = intf_num;
                }
	} else {
                if (usb_change_interface_num(nmea_fs_function,
                	f->descriptors, &nmea_interface_desc,
                	intf_num)) {
                	nmea->data_id = intf_num;
                }
	}
	return ret;
}
#endif

/**
 * nmea_bind_config - add a generic serial function to a configuration
 * @c: the configuration to support the serial instance
 * @port_num: /dev/ttyGS* port this interface will use
 * Context: single threaded during gadget setup
 *
 * Returns zero on success, else negative errno.
 *
 * Caller must have called @gserial_setup() with enough ports to
 * handle all the ones it binds.  Caller is also responsible
 * for calling @gserial_cleanup() before module unload.
 */
int __init nmea_bind_config(struct usb_configuration *c, u8 port_num)
{
	struct f_nmea	*nmea;
	int		status;

	printk(KERN_INFO "nmea_bind_config +++++\n");

	/* REVISIT might want instance-specific strings to help
	 * distinguish instances ...
	 */

	/* maybe allocate device-global string ID */
	if (nmea_string_defs[F_nmea_IDX].id == 0) {
		status = usb_string_id(c->cdev);
		if (status < 0)
			return status;
		nmea_string_defs[F_nmea_IDX].id = status;
	}

	/* allocate and initialize one new instance */
	nmea = kzalloc(sizeof *nmea, GFP_KERNEL);
	if (!nmea)
		return -ENOMEM;

	nmea->port_num = port_num;

	nmea->port.func.name = "nmea";
	nmea->port.func.strings = nmea_strings;
	nmea->port.func.bind = nmea_bind;
	nmea->port.func.unbind = nmea_unbind;
	nmea->port.func.set_alt = nmea_set_alt;
	nmea->port.func.disable = nmea_disable;
#ifdef CONFIG_USB_ANDROID_SAMSUNG_COMPOSITE
        nmea->port.func.set_intf_num = nmea_set_interface_id;
#endif        

	status = usb_add_function(c, &nmea->port.func);
	if (status)
		kfree(nmea);
	return status;
}

int nmea_function_bind_config(struct usb_configuration *c)
{
	int ret = nmea_bind_config(c, 2);
	/*
	if (ret == 0)
		gserial_setup(c->cdev->gadget, 1);
	*/
	return ret;
}

static struct android_usb_function nmea_function = {
	.name = "nmea",
	.bind_config = nmea_function_bind_config,
};

static int __init init(void)
{
	printk(KERN_INFO "f_nmea init\n");
	android_register_function(&nmea_function);
	return 0;
}
module_init(init);
