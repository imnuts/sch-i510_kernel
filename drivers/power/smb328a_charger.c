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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/smb328a_charger.h>

extern unsigned int HWREV;

static struct i2c_client	*smb328a_i2c_client;

#define REGISTER_00	0x00
#define REGISTER_01	0x01
#define REGISTER_02	0x02
#define REGISTER_03	0x03
#define REGISTER_04	0x04
#define REGISTER_05	0x05
#define REGISTER_06	0x06
#define REGISTER_07	0x07
#define REGISTER_08	0x08
#define REGISTER_09	0x09
#define REGISTER_0A	0x0A
#define REGISTER_30	0x30
#define REGISTER_31	0x31
#define REGISTER_32	0x32
#define REGISTER_33	0x33
#define REGISTER_34	0x34
#define REGISTER_35	0x35
#define REGISTER_36	0x36
#define REGISTER_37	0x37
#define REGISTER_38	0x38
#define REGISTER_39	0x39

static void _smb328a_print_register(void)
{
	int i = 0;

	printk("------------- _smb328a_debug -------------\n");

	for (i = 0x00; i <= 0x0A; i++) {
		pr_info("%s: 0x%x\t0x%x\n", __func__, i,
			i2c_smbus_read_byte_data(smb328a_i2c_client, i));
	}

	for (i = 0x30; i <= 0x39; i++) {
		pr_info("%s: 0x%x\t0x%x\n", __func__, i,
			i2c_smbus_read_byte_data(smb328a_i2c_client, i));
	}
}

static void _smb328a_rw_register(int addr, u8 clear, int shift, u8 value)
{
	u8 data = 0;

	data = i2c_smbus_read_byte_data(smb328a_i2c_client, addr);
	data = (data & ~(clear << shift)) | (value << shift);
	i2c_smbus_write_byte_data(smb328a_i2c_client, addr, data);

	pr_debug("smb328a: write REGISTER_%x -> 0x%x\n", addr, data);
}

static void _smb328a_custom_charger(void)
{
	/* Allow volatile writes */
	_smb328a_rw_register(REGISTER_31, 0x1, 7, 0x1);

	/* AC mode */
	_smb328a_rw_register(REGISTER_31, 0x1, 2, 0x1);

	/* Enable(EN) control */
	_smb328a_rw_register(REGISTER_05, 0x3, 2, 0x0);

	/* Fast charge current control method */
	_smb328a_rw_register(REGISTER_05, 0x1, 1, 0x0);

	/* Input current limit control method */
	_smb328a_rw_register(REGISTER_05, 0x1, 0, 0x0);

	/* Enable IRQ */
	_smb328a_rw_register(REGISTER_04, 0x1, 0, 0x1);

	/* Disable auto recharge */
	_smb328a_rw_register(REGISTER_03, 0x1, 7, 0x1);

	/* Disable charge safety timers */
	_smb328a_rw_register(REGISTER_04, 0x1, 3, 0x0);
}

static int _smb328a_set_fast_charge_current(enum smb328a_current_t curr)
{
	/* Set fast charge current */
	if (curr == SMB328A_CURR_500)
		pr_info("%s: CURR = 500mA\n", __func__);
	else if (curr == SMB328A_CURR_700)
		pr_info("%s: CURR = 700mA\n", __func__);
	else if (curr == SMB328A_CURR_800)
		pr_info("%s: CURR = 800mA\n", __func__);
	else if (curr == SMB328A_CURR_900)
		pr_info("%s: CURR = 900mA\n", __func__);

	_smb328a_rw_register(REGISTER_00, 0x7, 5, curr);

	/* Set input current limit */
	if (curr == SMB328A_CURR_500)
		pr_info("%s: INLIM = 450mA\n", __func__);	/* 0x0: 450mA */
	else if (curr == SMB328A_CURR_700)
		pr_info("%s: INLIM = 700mA\n", __func__);
	else if (curr == SMB328A_CURR_800)
		pr_info("%s: INLIM = 800mA\n", __func__);
	else if (curr == SMB328A_CURR_900)
		pr_info("%s: INLIM = 900mA\n", __func__);

	_smb328a_rw_register(REGISTER_01, 0x7, 5, curr);

	return curr;
}

static int _smb328a_set_termination_current(void)
{
	_smb328a_rw_register(REGISTER_00, 0x7, 0, 0x6);	// 175mA
	_smb328a_rw_register(REGISTER_07, 0x7, 5, 0x6);	// 175mA (with STAT assertion)

	/* Termination and Taper charging state triggers irq signal */
	_smb328a_rw_register(REGISTER_09, 0x1, 4, 0x1);

	pr_info("%s: 175mA\n", __func__);
	return 0;
}

static int _smb328a_charging_control(int enable)
{
	/* Battery charge enable */
	if (enable)
		_smb328a_rw_register(REGISTER_31, 0x1, 4, 0x0);
	else
		_smb328a_rw_register(REGISTER_31, 0x1, 4, 0x1);

	return 0;
}

static int smb328a_start_charging(enum smb328a_current_t curr)
{
	_smb328a_custom_charger();
	_smb328a_set_fast_charge_current(curr);
	_smb328a_set_termination_current();
	_smb328a_charging_control(1);

	pr_info("%s\n", __func__);
	_smb328a_print_register();

	return 0;
}

static int smb328a_stop_charging(void)
{
	_smb328a_charging_control(0);
	_smb328a_custom_charger();
	pr_info("%s\n", __func__);

	return 0;
}

static int smb328a_get_vbus_status(void)
{
	int value;

	value = i2c_smbus_read_byte_data(smb328a_i2c_client, REGISTER_33);

	if (value & 0x02)
		return 1;
	else
		return 0;
}

static irqreturn_t _smb328a_isr(int irq, void *data)
{
	struct smb328a_platform_data *pdata = data;
	int value;

	printk("********** %s **********\n", __func__);

	value = i2c_smbus_read_byte_data(smb328a_i2c_client, REGISTER_32);

	/* Check current termination IRQ */
	if (value & 0x08) {
		printk("%s: Current Termination\n", __func__);
		pdata->tx_charge_done();
	}

	/* Clear IRQ */
	_smb328a_rw_register(REGISTER_30, 0x1, 1, 0x1);

	return IRQ_HANDLED;
}

static int __devinit smb328a_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct smb328a_platform_data *pdata = client->dev.platform_data;
	int ret;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
		return -EIO;

	printk("+++++++++++++++ %s +++++++++++++++\n", __func__);

	smb328a_i2c_client = client;

	pdata->start_charging = smb328a_start_charging;
	pdata->stop_charging = smb328a_stop_charging;
	pdata->get_vbus_status = smb328a_get_vbus_status;

#ifdef CONFIG_TIKAL_USCC
	ret = request_threaded_irq(client->irq, NULL, _smb328a_isr,
				   IRQF_TRIGGER_RISING,
				   "smb328a-isr", pdata);
#endif

#ifdef CONFIG_MACH_CHIEF
	/* TA_nCHG GPIO connected from Rev0.1 Chief LTE Hardware */
	/* LnT need to add HWREV check*/
	ret = request_threaded_irq(client->irq, NULL, _smb328a_isr,
				   IRQF_TRIGGER_RISING,
				   "smb328a-isr", pdata);
#endif

	_smb328a_print_register();
	return 0;
}

static int __devexit smb328a_remove(struct i2c_client *client)
{
        return 0;
}

static const struct i2c_device_id smb328a_id[] = {
	{"smb328a", 0},
	{}
};
MODULE_DEVICE_TABLE(i2c, smb328a_id);

static struct i2c_driver smb328a_i2c_driver = {
	.driver = {
		.name = "smb328a",
	},
	.probe = smb328a_probe,
	.remove = __devexit_p(smb328a_remove),
	.id_table = smb328a_id,
};

static int __init smb328a_init(void)
{
	return i2c_add_driver(&smb328a_i2c_driver);
}
module_init(smb328a_init);

static void __exit smb328a_exit(void)
{
	i2c_del_driver(&smb328a_i2c_driver);
}
module_exit(smb328a_exit);

