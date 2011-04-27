/* linux/arch/arm/mach-s5pv210/include/mach/powergating.h
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *              http://www.samsung.com/
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/


void s5pc110_lock_power_domain(unsigned int nToken);
void s5pc110_unlock_power_domain(unsigned int nToken);
int s5p_domain_off_check(unsigned int power_domain);
int s5p_pmic_gating(unsigned int power_domain, unsigned int on_off);
int s5p_power_gating(unsigned int power_domain, unsigned int on_off);

