/*inclue/linux/ld9040.h
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *              http://www.samsung.com/
 *
 * Header file for Samsung Display Panel(AMOLED) driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#include <linux/types.h>

struct s5p_panel_data {
	const u16 *seq_display_set;
	const u16 *seq_panel_set;
	const u16 *seq_display_set_hw8;
	const u16 *seq_panel_set_hw8;
	const u16 *seq_etc_set;
	const u16 *display_on;
	const u16 *display_off;
	const u16 *standby_on;
	const u16 *standby_off;
	const u16 **gamma19_table;
	const u16 **gamma22_table;
	const u16 **gamma19_table_sm2;
	const u16 **gamma22_table_sm2;
	const u16 **gamma19_table_hw8;
	const u16 **gamma22_table_hw8;
	const u16 *gamma_update;
	const u16 **acl_table;
	const u16 *acl_init;
	const u16 *elvss_set_sm2;
	const u16 *elvss_set;
	const u16 *elvss_level4_sm2;
	const u16 *elvss_power_set_sm2;
	const u16 *elvss_power_set_hw8;
	const u16 *elvss_level4;
	const u16 **elvss_table;
	const u16 **elvss_table_sm2;
	const u16 *gamma210_sm2;
	const u16 *gamma210_hw8;
	const u16 *gamma210;
	int gamma_table_size;
};


