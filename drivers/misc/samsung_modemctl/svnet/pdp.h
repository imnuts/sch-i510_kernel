/**
 * SAMSUNG MODEM IPC header
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

#ifndef __PACKET_DATA_PROTOCOL_H__
#define __PACKET_DATA_PROTOCOL_H__

#include <linux/netdevice.h>

#ifdef CONFIG_MACH_CHIEF
#undef SUSPEND_RESUME_BRIDGE
#undef SVNET_PDP_ETHER
#else
#define SUSPEND_RESUME_BRIDGE
#define SVNET_PDP_ETHER
#endif

struct pdp_priv {
	int channel;
	struct net_device *parent;
};

extern struct net_device* create_pdp(int channel, struct net_device *parent);
extern void destroy_pdp(struct net_device **);

#ifdef SUSPEND_RESUME_BRIDGE
extern  ssize_t store_bridge_suspend(struct device *d,
                         struct device_attribute *attr, const char *buf, size_t count);


extern  ssize_t store_bridge_resume(struct device *d,
                         struct device_attribute *attr, const char *buf, size_t count);
#endif
#ifdef SET_INTERFACE_ID
extern ssize_t store_intf_id(struct device *d,
                         struct device_attribute *attr, const char *buf, size_t count);
#endif

#endif /* __PACKET_DATA_PROTOCOL_H__ */
