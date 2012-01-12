/*
 * Copyright (c) 2010 Yamaha Corporation
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include "yas.h"
#if !defined(CONFIG_MACH_AEGIS) && !defined(CONFIG_MACH_VIPER) \
	&& !defined(CONFIG_MACH_TIKAL)
#if YAS_MAG_DRIVER == YAS_MAG_DRIVER_YAS529
#include "yas_mag_driver-yas529.c"
#elif YAS_MAG_DRIVER == YAS_MAG_DRIVER_YAS530
#include "yas_mag_driver-yas530.c"
#endif
#else
#include "yas_mag_driver-yas529.c"
#include "yas_mag_driver-yas530.c"
#endif
