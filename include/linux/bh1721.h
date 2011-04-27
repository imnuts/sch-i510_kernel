/*
 * bh1721.h - BH1721 Ambient Light Sensors driver
 *
 * Copyright (c) 2009 Samsung Eletronics
 * Authors:
 *	Donggeun Kim <dg77.kim@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef _BH1721_H_
#define _BH1721_H_

struct bh1721_platform_data {
	void (*reset)(void);
};

#endif
