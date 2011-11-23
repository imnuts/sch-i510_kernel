/*
 *  max17040_battery.c
 *  fuel-gauge systems for lithium-ion (Li+) batteries
 *
 *  Copyright (C) 2009 Samsung Electronics
 *  Minkyu Kang <mk7.kang@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/power_supply.h>
#include <linux/max17040_battery.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/fs.h>
#ifdef CONFIG_BATTERY_MAX17043
#include <linux/irq.h>
#include <linux/interrupt.h>
#endif

#define MAX17040_VCELL_MSB	0x02
#define MAX17040_VCELL_LSB	0x03
#define MAX17040_SOC_MSB	0x04
#define MAX17040_SOC_LSB	0x05
#define MAX17040_MODE_MSB	0x06
#define MAX17040_MODE_LSB	0x07
#define MAX17040_VER_MSB	0x08
#define MAX17040_VER_LSB	0x09
#define MAX17040_RCOMP_MSB	0x0C
#define MAX17040_RCOMP_LSB	0x0D
#define MAX17040_CMD_MSB	0xFE
#define MAX17040_CMD_LSB	0xFF

#define MAX17040_DELAY		1000
#define MAX17040_BATTERY_FULL	95

#define MAX17040_MAJOR		174

struct max17040_chip {
	struct i2c_client		*client;
	struct power_supply		battery;
	struct max17040_platform_data	*pdata;
	struct timespec			next_update_time;
	struct device			*fg_atcmd;

	/* State Of Connect */
	int online;
	/* battery voltage */
	int vcell;
	/* battery capacity */
	int soc;
	/* State Of Charge */
	int status;
};

extern struct class *sec_class;
struct i2c_client *fg_i2c_client;

static void max17040_update_values(struct max17040_chip *chip);

static int max17040_get_property(struct power_supply *psy,
			    enum power_supply_property psp,
			    union power_supply_propval *val)
{
	struct max17040_chip *chip = container_of(psy,
				struct max17040_chip, battery);
	struct timespec now;

	ktime_get_ts(&now);
	monotonic_to_bootbased(&now);
	if (timespec_compare(&now, &chip->next_update_time) >= 0)
		max17040_update_values(chip);

	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = chip->status;
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = chip->online;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = chip->vcell;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		val->intval = chip->soc;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int max17040_write_reg(struct i2c_client *client, int reg, u8 value)
{
	int ret;

	ret = i2c_smbus_write_byte_data(client, reg, value);

	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	return ret;
}

static int max17040_read_reg(struct i2c_client *client, int reg)
{
	int ret;

	ret = i2c_smbus_read_byte_data(client, reg);

	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	return ret;
}

static void max17040_get_vcell(struct i2c_client *client)
{
	struct max17040_chip *chip = i2c_get_clientdata(client);
	u8 msb;
	u8 lsb;

	msb = max17040_read_reg(client, MAX17040_VCELL_MSB);
	lsb = max17040_read_reg(client, MAX17040_VCELL_LSB);

	chip->vcell = ((msb << 4) + (lsb >> 4)) * 1250;
}

static void max17040_get_soc(struct i2c_client *client)
{
	struct max17040_chip *chip = i2c_get_clientdata(client);
	u8 msb;
	u8 lsb;
	int pure_soc, adj_soc, soc;
	int full = chip->pdata->psoc_full;
	int empty = chip->pdata->psoc_empty;

	if (full == 0)
		full = 100;
	if (empty == 0)
		empty = 0;

	msb = max17040_read_reg(client, MAX17040_SOC_MSB);
	lsb = max17040_read_reg(client, MAX17040_SOC_LSB);

	pure_soc = msb * 100 + (lsb * 100) / 256;

	if (pure_soc >= 0)
		adj_soc = ((pure_soc * 10000) - empty) / (full - empty);
	else
		adj_soc = 0;

	soc = adj_soc / 100;

	if (adj_soc % 100 >= 50)	// ¹Ý¿Ã¸²
		soc += 1;

	if (soc > 100)
		soc = 100;

	chip->soc = min(soc, (uint)100);
}

static void max17040_get_version(struct i2c_client *client)
{
	u8 msb;
	u8 lsb;

	msb = max17040_read_reg(client, MAX17040_VER_MSB);
	lsb = max17040_read_reg(client, MAX17040_VER_LSB);

	dev_info(&client->dev, "MAX17040 Fuel-Gauge Ver %d%d\n", msb, lsb);
}

static void max17040_get_online(struct i2c_client *client)
{
	struct max17040_chip *chip = i2c_get_clientdata(client);

	if (chip->pdata && chip->pdata->battery_online)
		chip->online = chip->pdata->battery_online();
	else
		chip->online = 1;
}

static void max17040_get_status(struct i2c_client *client)
{
	struct max17040_chip *chip = i2c_get_clientdata(client);

	if (!chip->pdata || !chip->pdata->charger_online ||
		!chip->pdata->charger_enable) {
		chip->status = POWER_SUPPLY_STATUS_UNKNOWN;
		return;
	}

	if (chip->pdata->charger_online()) {
		if (chip->pdata->charger_enable())
			chip->status = POWER_SUPPLY_STATUS_CHARGING;
		else
			chip->status = POWER_SUPPLY_STATUS_NOT_CHARGING;
	} else {
		chip->status = POWER_SUPPLY_STATUS_DISCHARGING;
	}

	if (chip->soc > MAX17040_BATTERY_FULL)
		chip->status = POWER_SUPPLY_STATUS_FULL;
}

static int max17040_reset_chip(struct i2c_client *client)
{
	int ret;
	u16 rst_cmd = 0x4000;

	ret = i2c_smbus_write_word_data(client, MAX17040_MODE_MSB, swab16(rst_cmd));
	msleep(500);

	return ret;
}

static void max17040_update_values(struct max17040_chip *chip)
{
	max17040_get_vcell(chip->client);
	max17040_get_soc(chip->client);
	max17040_get_online(chip->client);
	max17040_get_status(chip->client);

	/* next update must be at least 1 second later */
	ktime_get_ts(&chip->next_update_time);
	monotonic_to_bootbased(&chip->next_update_time);
	chip->next_update_time.tv_sec++;
}

void max17040_reset_soc(void)
{
	struct i2c_client *client = fg_i2c_client;
	max17040_reset_chip(client);
}
EXPORT_SYMBOL(max17040_reset_soc);

unsigned int max17040_read_raw_soc(void)
{
	struct i2c_client *client = fg_i2c_client;
	u8 msb;
	u8 lsb;
	int pure_soc;

	msb = max17040_read_reg(client, MAX17040_SOC_MSB);
	lsb = max17040_read_reg(client, MAX17040_SOC_LSB);

	return msb;
}
EXPORT_SYMBOL(max17040_read_raw_soc);

unsigned int max17040_read_raw_vcell(void)
{
	struct i2c_client *client = fg_i2c_client;
	u8 msb;
	u8 lsb;
	int vcell = 0;

	msb = max17040_read_reg(client, MAX17040_VCELL_MSB);
	lsb = max17040_read_reg(client, MAX17040_VCELL_LSB);

	vcell = msb << 8 | lsb;
	vcell = (vcell >> 4) * 125 * 1000;

	return vcell;
}
EXPORT_SYMBOL(max17040_read_raw_vcell);

static ssize_t max17040_show_fg_soc(struct device *device,
				    struct device_attribute *attr, char *buf)
{
	struct max17040_chip *chip = (struct max17040_chip *)dev_get_drvdata(device);

	max17040_get_soc(chip->client);

	return sprintf(buf, "%d\n", chip->soc);	
}
static DEVICE_ATTR(set_fuel_gauage_read, 0664, max17040_show_fg_soc, NULL);

static ssize_t max17040_show_fg_reset(struct device *device,
				      struct device_attribute *attr, char *buf)
{
	struct max17040_chip *chip = (struct max17040_chip *)dev_get_drvdata(device);
	int ret;

	ret = max17040_reset_chip(chip->client);
	max17040_get_soc(chip->client);

	return sprintf(buf,"%d\n", ret);
}
static DEVICE_ATTR(set_fuel_gauage_reset, 0664, max17040_show_fg_reset, NULL);

static enum power_supply_property max17040_battery_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
};

#ifdef CONFIG_BATTERY_MAX17043
static irqreturn_t max17043_isr(int irq, void *data)
{
	struct max17040_chip *chip = data;
	struct i2c_client *client = chip->client;
	u8 msb;
	u8 lsb;
	int raw_soc;

	max17040_get_soc(client);

	msb = max17040_read_reg(client, MAX17040_SOC_MSB);
	lsb = max17040_read_reg(client, MAX17040_SOC_LSB);

	raw_soc = msb * 100 + (lsb * 100) / 256;

	msb = max17040_read_reg(client, MAX17040_RCOMP_MSB);
	lsb = max17040_read_reg(client, MAX17040_RCOMP_LSB);

	dev_info(&client->dev, "%s: msb=0x%x, lsb=0x%x (raw_soc=%d, adj_soc=%d)\n",
		__func__, msb, lsb, raw_soc, chip->soc);

	/* clear ALRT bit to enable next irq */
	lsb = lsb & ~(0x20);
	i2c_smbus_write_word_data(client, MAX17040_RCOMP_MSB,
				  swab16((msb << 8) + lsb));

	return IRQ_HANDLED;
}

static int max17043_irq_init(struct max17040_chip *chip)
{
	struct i2c_client *client = chip->client;
	int ret;

	if (client->irq) {
		ret = request_threaded_irq(client->irq, NULL,
			max17043_isr, IRQF_TRIGGER_FALLING,
			"max17043 fuel gauge", chip);
		if (ret) {
			dev_err(&client->dev, "failed to reqeust IRQ\n");
			return ret;
		}

		ret = enable_irq_wake(client->irq);
		if (ret < 0)
			dev_err(&client->dev,
				"failed to enable wakeup src %d\n", ret);
	}

	return 0;
}
#endif

static int __devinit max17040_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct max17040_chip *chip;
	int ret;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE))
		return -EIO;

	chip = kzalloc(sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->client = fg_i2c_client = client;
	chip->pdata = client->dev.platform_data;

	i2c_set_clientdata(client, chip);

	chip->battery.name		= "battery";
	chip->battery.type		= POWER_SUPPLY_TYPE_BATTERY;
	chip->battery.get_property	= max17040_get_property;
	chip->battery.properties	= max17040_battery_props;
	chip->battery.num_properties	= ARRAY_SIZE(max17040_battery_props);

	max17040_update_values(chip);

	if (chip->pdata && chip->pdata->power_supply_register)
		ret = chip->pdata->power_supply_register(&client->dev, &chip->battery);
	else
		ret = power_supply_register(&client->dev, &chip->battery);
	if (ret) {
		dev_err(&client->dev, "failed: power supply register\n");
		goto err_psy_register;
	}

	max17040_get_version(client);

	if (chip->pdata)
		i2c_smbus_write_word_data(client, MAX17040_RCOMP_MSB,
					  swab16(chip->pdata->rcomp_value));

	chip->fg_atcmd = device_create(sec_class, NULL, MKDEV(MAX17040_MAJOR, 0),
					NULL, "fg_atcom_test");
	if (IS_ERR(chip->fg_atcmd)) {
		dev_err(&client->dev, "failed to create fg_atcmd\n");
		goto err_fg_atcmd;
	}

	dev_set_drvdata(chip->fg_atcmd, chip);

	ret = device_create_file(chip->fg_atcmd, &dev_attr_set_fuel_gauage_read);
	if (ret)
		goto err_fg_read;

	ret = device_create_file(chip->fg_atcmd, &dev_attr_set_fuel_gauage_reset);
	if (ret)
		goto err_fg_reset;

#ifdef CONFIG_BATTERY_MAX17043
	max17043_irq_init(chip);
#endif

	return 0;

err_fg_reset:
	device_remove_file(chip->fg_atcmd, &dev_attr_set_fuel_gauage_read);
err_fg_read:
	device_destroy(sec_class, MKDEV(MAX17040_MAJOR, 0));
err_fg_atcmd:
	if (chip->pdata && chip->pdata->power_supply_unregister)
		chip->pdata->power_supply_unregister(&chip->battery);
	else
		power_supply_unregister(&chip->battery);

err_psy_register:
	kfree(chip);

	return ret;
}

static int __devexit max17040_remove(struct i2c_client *client)
{
	struct max17040_chip *chip = i2c_get_clientdata(client);

	device_remove_file(chip->fg_atcmd, &dev_attr_set_fuel_gauage_reset);
	device_remove_file(chip->fg_atcmd, &dev_attr_set_fuel_gauage_read);
	device_destroy(sec_class, MKDEV(MAX17040_MAJOR, 0));

	if (chip->pdata && chip->pdata->power_supply_unregister)
		chip->pdata->power_supply_unregister(&chip->battery);
	else
		power_supply_unregister(&chip->battery);
	kfree(chip);
	return 0;
}

static const struct i2c_device_id max17040_id[] = {
	{ "max17040", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, max17040_id);

static struct i2c_driver max17040_i2c_driver = {
	.driver	= {
		.name	= "max17040",
	},
	.probe		= max17040_probe,
	.remove		= __devexit_p(max17040_remove),
	.id_table	= max17040_id,
};

static int __init max17040_init(void)
{
	return i2c_add_driver(&max17040_i2c_driver);
}
module_init(max17040_init);

static void __exit max17040_exit(void)
{
	i2c_del_driver(&max17040_i2c_driver);
}
module_exit(max17040_exit);

MODULE_AUTHOR("Minkyu Kang <mk7.kang@samsung.com>");
MODULE_DESCRIPTION("MAX17040 Fuel Gauge");
MODULE_LICENSE("GPL");
