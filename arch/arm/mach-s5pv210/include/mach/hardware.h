/* linux/arch/arm/mach-s5pv210/include/mach/hardware.h
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * S5PV210 - Hardware support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H __FILE__

#define pcibios_assign_all_busses()	1

#define PCIBIOS_MIN_IO			0x00001000
#define PCIBIOS_MIN_MEM			0x01000000
#define PCIMEM_BASE			0x0

#endif /* __ASM_ARCH_HARDWARE_H */
