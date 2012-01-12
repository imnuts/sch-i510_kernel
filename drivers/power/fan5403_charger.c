/* Switch mode charger to minimize single cell lithium ion charging time from a USB power source.
 *
 * Copyright (C) 2009 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/fan5403_charger.h>
#include <linux/fan5403_private.h>

/* Register address */
#define FAN5403_REG_CONTROL0	0x00
#define FAN5403_REG_CONTROL1	0x01
#define FAN5403_REG_OREG	0x02
#define FAN5403_REG_IC_INFO	0x03
#define FAN5403_REG_IBAT	0x04
#define FAN5403_REG_SP_CHARGER	0x05
#define FAN5403_REG_SAFETY	0x06
#define FAN5403_REG_MONITOR	0x10

static struct i2c_client	*fan5403_i2c_client;
static struct work_struct	chg_work;
static struct timer_list	chg_work_timer;
static struct workqueue_struct	*chg_wqueue;

#define FAN5403_POLLING_INTERVAL	10000	/* 10 sec */

static void _fan5403_print_register(void)
{
	u8 regs[8];
	regs[0] = i2c_smbus_read_byte_data(fan5403_i2c_client, FAN5403_REG_CONTROL0);
	regs[1] = i2c_smbus_read_byte_data(fan5403_i2c_client, FAN5403_REG_CONTROL1);
	regs[2] = i2c_smbus_read_byte_data(fan5403_i2c_client, FAN5403_REG_OREG);
	regs[3] = i2c_smbus_read_byte_data(fan5403_i2c_client, FAN5403_REG_IC_INFO);
	regs[4] = i2c_smbus_read_byte_data(fan5403_i2c_client, FAN5403_REG_IBAT);
	regs[5] = i2c_smbus_read_byte_data(fan5403_i2c_client, FAN5403_REG_SP_CHARGER);
	regs[6] = i2c_smbus_read_byte_data(fan5403_i2c_client, FAN5403_REG_SAFETY);
	regs[7] = i2c_smbus_read_byte_data(fan5403_i2c_client, FAN5403_REG_MONITOR);

	pr_info("/FAN5403/ 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", 
		regs[0], regs[1], regs[2], regs[3], regs[4], regs[5], regs[6], regs[7]);
}

static void _fan5403_customize_register_settings(void)
{
	u8 value;

	i2c_smbus_write_byte_data(fan5403_i2c_client, FAN5403_REG_SAFETY,
		(FAN5403_ISAFE_950	<< FAN5403_SHIFT_ISAFE) |	/* Maximum IOCHARGE */
		(FAN5403_VSAFE_420	<< FAN5403_SHIFT_VSAFE));	/* Maximum OREG */

	value = i2c_smbus_read_byte_data(fan5403_i2c_client, FAN5403_REG_CONTROL1);
	value = value & ~(0x3F);	// clear except input limit
	value = value | ((FAN5403_LOWV_34	<< FAN5403_SHIFT_LOWV) |	/* Weak battery voltage 3.4V */
		(FAN5403_TE_DISABLE	<< FAN5403_SHIFT_TE) |
		(FAN5403_CE_ENABLE	<< FAN5403_SHIFT_CE) |
		(FAN5403_HZ_MODE_NO	<< FAN5403_SHIFT_HZ_MODE) |
		(FAN5403_OPA_MODE_CHARGE	<< FAN5403_SHIFT_OPA_MODE));

	i2c_smbus_write_byte_data(fan5403_i2c_client, FAN5403_REG_OREG,
		(FAN5403_OREG_420	<< FAN5403_SHIFT_OREG) |	/* Full charging voltage */
		(FAN5403_OTG_PL_HIGH	<< FAN5403_SHIFT_OTG_PL) |
		(FAN5403_OTG_EN_DISABLE	<< FAN5403_SHIFT_OTG_EN));

	i2c_smbus_write_byte_data(fan5403_i2c_client, FAN5403_REG_SP_CHARGER,
		(FAN5403_RESERVED_2	<< FAN5403_SHIFT_RESERVED_2) |
		(FAN5403_DIS_VREG_ON	<< FAN5403_SHIFT_DIS_VREG) |
		(FAN5403_IO_LEVEL_0	<< FAN5403_SHIFT_IO_LEVEL) |	/* Controlled by IOCHARGE bits */
		(FAN5403_SP_RDONLY	<< FAN5403_SHIFT_SP) |
		(FAN5403_EN_LEVEL_RDONLY	<< FAN5403_SHIFT_EN_LEVEL) |
		(FAN5403_VSP_4533	<< FAN5403_SHIFT_VSP));
}

static int _fan5403_set_iocharge(enum fan5403_iocharge_t curr)
{
	u8 value;

	/* Set charge current */
	value = i2c_smbus_read_byte_data(fan5403_i2c_client, FAN5403_REG_IBAT);

	value = value & ~(FAN5403_MASK_RESET);	/* Clear control reset bit */
	value = value & ~(FAN5403_MASK_IOCHARGE);
	value = value | (curr << FAN5403_SHIFT_IOCHARGE);

	if (curr == FAN5403_CURR_550)
		pr_info("%s: IOCHARGE = 550mA\n", __func__);
	else if (curr == FAN5403_CURR_750)
		pr_info("%s: IOCHARGE = 750mA\n", __func__);
	else if (curr == FAN5403_CURR_850)
		pr_info("%s: IOCHARGE = 850mA\n", __func__);
	else if (curr == FAN5403_CURR_950)
		pr_info("%s: IOCHARGE = 950mA\n", __func__);
	else
		pr_info("%s: IOCHARGE = xxxmA (bit = %d)\n", __func__, curr);

	i2c_smbus_write_byte_data(fan5403_i2c_client, FAN5403_REG_IBAT, value);

	/* Set input current limit */
	value = i2c_smbus_read_byte_data(fan5403_i2c_client, FAN5403_REG_CONTROL1);

	value = value & ~(FAN5403_MASK_INLIM);

	if (curr == FAN5403_CURR_550) {
		value = value | (FAN5403_INLIM_500 << FAN5403_SHIFT_INLIM);
		pr_info("%s: INLIM = 500mA\n", __func__);
	} else {
		value = value | (FAN5403_INLIM_NO_LIMIT << FAN5403_SHIFT_INLIM);
		pr_info("%s: INLIM = No limit\n", __func__);
	}

	i2c_smbus_write_byte_data(fan5403_i2c_client, FAN5403_REG_CONTROL1, value);

	return curr;
}

static int _fan5403_set_iterm(void)
{
	u8 value;

	value = i2c_smbus_read_byte_data(fan5403_i2c_client, FAN5403_REG_IBAT);

	value = value & ~(FAN5403_MASK_RESET);	/* Clear control reset bit */
	value = value & ~(FAN5403_MASK_ITERM);
	value = value | (FAN5403_ITERM_194 << FAN5403_SHIFT_ITERM);

	i2c_smbus_write_byte_data(fan5403_i2c_client, FAN5403_REG_IBAT, value);

	pr_info("%s: ITERM 194mA\n", __func__);
	return 0;
}

static int _fan5403_charging_control(int enable)
{
	u8 value;

	value = i2c_smbus_read_byte_data(fan5403_i2c_client, FAN5403_REG_CONTROL1);

	value = value & ~(FAN5403_MASK_CE);
	value = value & ~(FAN5403_MASK_TE);

	/* Set CE and TE bit */
	if (enable)
		value = value |
			(FAN5403_TE_ENABLE << FAN5403_SHIFT_TE) |
			(FAN5403_CE_ENABLE << FAN5403_SHIFT_CE);
	else
		value = value |
			(FAN5403_TE_DISABLE << FAN5403_SHIFT_TE) |
			(FAN5403_CE_DISABLE << FAN5403_SHIFT_CE);

	i2c_smbus_write_byte_data(fan5403_i2c_client, FAN5403_REG_CONTROL1, value);

	return 0;
}

static void _fan5403_chg_work(struct work_struct *work)
{
	u8 value;
	static int log_cnt = 0;

	/* Reset 32 sec timer while host is alive */
	value = i2c_smbus_read_byte_data(fan5403_i2c_client, FAN5403_REG_CONTROL0);

	value = value & ~(FAN5403_MASK_TMR_RST);
	value = value | (FAN5403_TMR_RST_RESET << FAN5403_SHIFT_TMR_RST);

	i2c_smbus_write_byte_data(fan5403_i2c_client, FAN5403_REG_CONTROL0, value);

	/* print registers every 10 minutes */
	log_cnt++;
	if (log_cnt == 60) {
		_fan5403_print_register();
		log_cnt = 0;
	}

	pr_debug("fan5403 host is alive\n");

	mod_timer(&chg_work_timer, jiffies + msecs_to_jiffies(FAN5403_POLLING_INTERVAL));
}

static void _fan5403_chg_work_timer_func(unsigned long param)
{
	queue_work(chg_wqueue, &chg_work);
}

static int fan5403_start_charging(enum fan5403_iocharge_t curr)
{
	if (!fan5403_i2c_client)
		return 0;

	_fan5403_set_iocharge(curr);
	_fan5403_set_iterm();
	_fan5403_charging_control(1);

	pr_info("%s\n", __func__);
	
	_fan5403_print_register();
	return 0;
}

static int fan5403_stop_charging(void)
{
	if (!fan5403_i2c_client)
		return 0;

	_fan5403_charging_control(0);

	pr_info("%s\n", __func__);

	_fan5403_print_register();
	return 0;
}

static int fan5403_get_vbus_status(void)
{
	u8 value;

	if (!fan5403_i2c_client)
		return 0;

	value = i2c_smbus_read_byte_data(fan5403_i2c_client, FAN5403_REG_MONITOR);

	if (value & FAN5403_MASK_VBUS_VALID)
		return 1;
	else
		return 0;
}

static int fan5403_set_host_status(bool isAlive)
{
	if (!fan5403_i2c_client)
		return 0;

	if (isAlive) {
		/* Reset 32 sec timer every 10 sec to ensure the host is alive */
		queue_work(chg_wqueue, &chg_work);
	} else
		del_timer_sync(&chg_work_timer);

	return isAlive;
}

static int _fan5403_get_charging_fault(void)
{
	u8 value;

	if (!fan5403_i2c_client)
		return 0;

	value = i2c_smbus_read_byte_data(fan5403_i2c_client, FAN5403_REG_CONTROL0);

	switch (value & FAN5403_MASK_FAULT) {
	case FAN5403_FAULT_NONE:
		pr_info("%s: none\n", __func__);
		break;
	case FAN5403_FAULT_VBUS_OVP:
		pr_info("%s: vbus ovp\n", __func__);
		break;
	case FAN5403_FAULT_SLEEP_MODE:
		pr_info("%s: sleep mode\n", __func__);
		break;
	case FAN5403_FAULT_POOR_INPUT:
		pr_info("%s: poor input source\n", __func__);
		break;
	case FAN5403_FAULT_BATT_OVP:
		pr_info("%s: battery ovp\n", __func__);
		break;
	case FAN5403_FAULT_THERMAL:
		pr_info("%s: thermal shutdown\n", __func__);
		break;
	case FAN5403_FAULT_TIMER:
		pr_info("%s: timer fault\n", __func__);
		break;
	case FAN5403_FAULT_NO_BATT:
		pr_info("%s: no battery\n", __func__);
		break;
	}

	return (value & FAN5403_MASK_FAULT);
}

static int _fan5403_get_charging_status(void)
{
	u8 value;

	value = i2c_smbus_read_byte_data(fan5403_i2c_client, FAN5403_REG_CONTROL0);
	value = value & FAN5403_MASK_STAT;

	if (value == (FAN5403_STAT_READY << FAN5403_SHIFT_STAT))
		return FAN5403_STAT_READY;
	else if (value == (FAN5403_STAT_CHARGING << FAN5403_SHIFT_STAT))
		return FAN5403_STAT_CHARGING;
	else if (value == (FAN5403_STAT_CHG_DONE << FAN5403_SHIFT_STAT))
		return FAN5403_STAT_CHG_DONE;
	else
		return FAN5403_STAT_FAULT;
}

static irqreturn_t _fan5403_handle_stat(int irq, void *data)
{
	struct fan5403_platform_data *pdata = data;
	int stat;
	enum fan5403_fault_t fault_reason;

	msleep(500);

	stat = _fan5403_get_charging_status();

	switch (stat) {
	case FAN5403_STAT_READY:
		pr_info("%s: ready\n", __func__);
		break;
	case FAN5403_STAT_CHARGING:
		pr_info("%s: charging\n", __func__);
		break;
	case FAN5403_STAT_CHG_DONE:
		pdata->tx_charge_done();
		pr_info("%s: chg done\n", __func__);
		break;
	case FAN5403_STAT_FAULT:
		_fan5403_print_register();
		fault_reason = _fan5403_get_charging_fault();
		pdata->tx_charge_fault(fault_reason);
		_fan5403_customize_register_settings();
		break;
	}

	return IRQ_HANDLED;
}

/* Support battery test mode (*#0228#) */
static int fan5403_get_monitor_bits(void)
{
	/*
		VICHG not supported with FAN5403...

		0:	Discharging (no charger)
		1:	CC charging
		2:	CV charging
		3:	Not charging (charge termination) include fault
	*/

	u8 value = 0;

	if (fan5403_get_vbus_status()) {
		if (_fan5403_get_charging_status() == FAN5403_STAT_CHARGING) {
			/* Check CV bit */
			value = i2c_smbus_read_byte_data(fan5403_i2c_client, FAN5403_REG_MONITOR);
			if (value & FAN5403_MASK_CV)
				return 2;
			else
				return 1;
		}
		return 3;
	}
	return 0;
}

static int __devinit fan5403_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct fan5403_platform_data *pdata = client->dev.platform_data;
	int ret;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
		return -EIO;

	pr_info("+++++++++++++++ %s +++++++++++++++\n", __func__);

	fan5403_i2c_client = client;

	/* Init Rx functions from host */
	pdata->start_charging = fan5403_start_charging;
	pdata->stop_charging = fan5403_stop_charging;
	pdata->get_vbus_status = fan5403_get_vbus_status;
	pdata->set_host_status = fan5403_set_host_status;
	pdata->get_monitor_bits = fan5403_get_monitor_bits;

	INIT_WORK(&chg_work, _fan5403_chg_work);
	setup_timer(&chg_work_timer, _fan5403_chg_work_timer_func, (unsigned long)pdata);
	chg_wqueue = create_freezeable_workqueue("fan5403_wqueue");

	ret = request_threaded_irq(client->irq, NULL, _fan5403_handle_stat,
				   (IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING),
				   "fan5403-stat", pdata);

	_fan5403_customize_register_settings();
	_fan5403_print_register();

	return 0;
}

static int __devexit fan5403_remove(struct i2c_client *client)
{
        return 0;
}

static const struct i2c_device_id fan5403_id[] = {
	{"fan5403", 0},
	{}
};
MODULE_DEVICE_TABLE(i2c, fan5403_id);

static struct i2c_driver fan5403_i2c_driver = {
	.driver = {
		.name = "fan5403",
	},
	.probe = fan5403_probe,
	.remove = __devexit_p(fan5403_remove),
	.id_table = fan5403_id,
};

static int __init fan5403_init(void)
{
	return i2c_add_driver(&fan5403_i2c_driver);
}
module_init(fan5403_init);

static void __exit fan5403_exit(void)
{
	i2c_del_driver(&fan5403_i2c_driver);
}
module_exit(fan5403_exit);

