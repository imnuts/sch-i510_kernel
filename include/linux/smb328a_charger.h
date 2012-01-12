/*
 *  Copyright (C) 2009 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __SMB328A_CHARGER_H_
#define __SMB328A_CHARGER_H_

/* Charge current */
/* Argument value of "set_charge_current" */
enum smb328a_current_t {
	SMB328A_CURR_500 = 0,
	SMB328A_CURR_600,
	SMB328A_CURR_700,
	SMB328A_CURR_800,
	SMB328A_CURR_900,
	SMB328A_CURR_1000,
	SMB328A_CURR_1100,
	SMB328A_CURR_1200,
};

#define CHG_CURR_TA		SMB328A_CURR_900
#define CHG_CURR_TA_EVENT	SMB328A_CURR_700
#define CHG_CURR_USB		SMB328A_CURR_500

struct smb328a_platform_data {
	int (*start_charging)(enum smb328a_current_t curr);
	int (*stop_charging)(void);
	int (*get_vbus_status)(void);

	/* Tx functions to host */
	void (*tx_charge_done)(void);
};
#endif
