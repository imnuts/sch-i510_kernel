/*
 *  Copyright (C) 2009 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __FAN5403_CHARGER_H_
#define __FAN5403_CHARGER_H_

/* Charge current */
/* Argument value of "set_charge_current" */
enum fan5403_iocharge_t {
	FAN5403_CURR_550 = 0,
	FAN5403_CURR_650,
	FAN5403_CURR_750,
	FAN5403_CURR_850,
	FAN5403_CURR_950,
	FAN5403_CURR_1050,
	FAN5403_CURR_1150,
	FAN5403_CURR_1250,
};

#define CHG_CURR_TA		FAN5403_CURR_950
#define CHG_CURR_TA_EVENT	FAN5403_CURR_750
#define CHG_CURR_USB		FAN5403_CURR_550

/* Charging fault */
/* Argument value of "tx_charge_fault" */
enum fan5403_fault_t {
	FAN5403_FAULT_NONE = 0,
	FAN5403_FAULT_VBUS_OVP,
	FAN5403_FAULT_SLEEP_MODE,
	FAN5403_FAULT_POOR_INPUT,
	FAN5403_FAULT_BATT_OVP,
	FAN5403_FAULT_THERMAL,
	FAN5403_FAULT_TIMER,
	FAN5403_FAULT_NO_BATT,
};

struct fan5403_platform_data {
	/* Rx functions from host */
	int (*start_charging)(enum fan5403_iocharge_t curr);
	int (*stop_charging)(void);
	int (*get_vbus_status)(void);
	int (*set_host_status)(bool isAlive);
	int (*get_monitor_bits)(void);	/* support battery test mode (*#0228#) */

	/* Tx functions to host */
	void (*tx_charge_done)(void);
	void (*tx_charge_fault)(enum fan5403_fault_t reason);
};

/*

 Host(AP) --> Client(Charger)	: "Rx" from host
 Host(AP) <-- Client(Charger)	: "Tx" to host

*/

#endif
