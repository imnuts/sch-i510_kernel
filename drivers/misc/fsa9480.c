/*
 * driver/misc/fsa9480.c - FSA9480 micro USB switch device driver
 *
 * Copyright (C) 2010 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Wonguk Jeong <wonguk.jeong@samsung.com>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/fsa9480.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <mach/param.h>

/* FSA9480 I2C registers */
#define FSA9480_REG_DEVID		0x01
#define FSA9480_REG_CTRL		0x02
#define FSA9480_REG_INT1		0x03
#define FSA9480_REG_INT2		0x04
#define FSA9480_REG_INT1_MASK		0x05
#define FSA9480_REG_INT2_MASK		0x06
#define FSA9480_REG_ADC			0x07
#define FSA9480_REG_TIMING1		0x08
#define FSA9480_REG_TIMING2		0x09
#define FSA9480_REG_DEV_T1		0x0a
#define FSA9480_REG_DEV_T2		0x0b
#define FSA9480_REG_BTN1		0x0c
#define FSA9480_REG_BTN2		0x0d
#define FSA9480_REG_CK			0x0e
#define FSA9480_REG_CK_INT1		0x0f
#define FSA9480_REG_CK_INT2		0x10
#define FSA9480_REG_CK_INTMASK1		0x11
#define FSA9480_REG_CK_INTMASK2		0x12
#define FSA9480_REG_MANSW1		0x13
#define FSA9480_REG_MANSW2		0x14

/* Control */
#define CON_SWITCH_OPEN		(1 << 4)
#define CON_RAW_DATA		(1 << 3)
#define CON_MANUAL_SW		(1 << 2)
#define CON_WAIT		(1 << 1)
#define CON_INT_MASK		(1 << 0)
#define CON_MASK		(CON_SWITCH_OPEN | CON_RAW_DATA | \
				CON_MANUAL_SW | CON_WAIT)

/* Device Type 1 */
#define DEV_USB_OTG		(1 << 7)
#define DEV_DEDICATED_CHG	(1 << 6)
#define DEV_USB_CHG		(1 << 5)
#define DEV_CAR_KIT		(1 << 4)
#define DEV_UART		(1 << 3)
#define DEV_USB			(1 << 2)
#define DEV_AUDIO_2		(1 << 1)
#define DEV_AUDIO_1		(1 << 0)

#define DEV_T1_USB_MASK		(DEV_USB_OTG | DEV_USB)
#define DEV_T1_UART_MASK	(DEV_UART)
#define DEV_T1_CHARGER_MASK	(DEV_DEDICATED_CHG | DEV_USB_CHG | DEV_CAR_KIT)

/* Device Type 2 */
#define DEV_AV			(1 << 6)
#define DEV_TTY			(1 << 5)
#define DEV_PPD			(1 << 4)
#define DEV_JIG_UART_OFF	(1 << 3)
#define DEV_JIG_UART_ON		(1 << 2)
#define DEV_JIG_USB_OFF		(1 << 1)
#define DEV_JIG_USB_ON		(1 << 0)

#define DEV_T2_USB_MASK		(DEV_JIG_USB_OFF | DEV_JIG_USB_ON)
#define DEV_T2_UART_MASK	(DEV_JIG_UART_OFF | DEV_JIG_UART_ON)
#define DEV_T2_JIG_MASK		(DEV_JIG_USB_OFF | DEV_JIG_USB_ON | \
				DEV_JIG_UART_OFF)

/*
 * Manual Switch
 * D- [7:5] / D+ [4:2]
 * 000: Open all / 001: USB / 010: AUDIO / 011: UART / 100: V_AUDIO
 */
#define SW_VAUDIO		((4 << 5) | (4 << 2))
#define SW_UART			((3 << 5) | (3 << 2))
#define SW_AUDIO		((2 << 5) | (2 << 2))
#define SW_DHOST		((1 << 5) | (1 << 2))
#define SW_AUTO			((0 << 5) | (0 << 2))

/* Interrupt 1 */
#define INT_OVP_EN			(1 << 5)
#define INT_LKR			(1 << 4)
#define INT_LKP			(1 << 3)
#define INT_KP			(1 << 2)
#define INT_DETACH		(1 << 1)
#define INT_ATTACH		(1 << 0)

/* Interrupt Mask 1 */
#define INT_MASK_LKR		(1 << 4)
#define INT_MASK_LKP		(1 << 3)
#define INT_MASK_KP		(1 << 2)
#define INT_MASK_DETACH		(1 << 1)
#define INT_MASK_ATTACH		(1 << 0)

/* Timing table for Timing Set 1 & 2 register */
#define ADC_DETECTION_TIME_50MS		(0x00)
#define ADC_DETECTION_TIME_100MS	(0x01)
#define ADC_DETECTION_TIME_150MS	(0x02)
#define ADC_DETECTION_TIME_200MS	(0x03)
#define ADC_DETECTION_TIME_300MS	(0x04)
#define ADC_DETECTION_TIME_400MS	(0x05)
#define ADC_DETECTION_TIME_500MS	(0x06)
#define ADC_DETECTION_TIME_600MS	(0x07)
#define ADC_DETECTION_TIME_700MS	(0x08)
#define ADC_DETECTION_TIME_800MS	(0x09)
#define ADC_DETECTION_TIME_900MS	(0x10)
#define ADC_DETECTION_TIME_1000MS	(0x11)

#define KEY_PRESS_TIME_200MS		(0x10)
#define KEY_PRESS_TIME_300MS		(0x20)
#define KEY_PRESS_TIME_700MS		(0x60)

#define LONGKEY_PRESS_TIME_500MS	(0x02)
#define LONGKEY_PRESS_TIME_1S		(0x07)
#define LONGKEY_PRESS_TIME_1_5S		(0x0C)


#if defined(CONFIG_MACH_STEALTHV) || defined(CONFIG_MACH_AEGIS) || defined(CONFIG_MACH_VIPER) || defined(CONFIG_MACH_CHIEF)
#define REG_INT1_MASK_VALUE     (0x3fe0)
#define REG_TIMING1_VALUE	(ADC_DETECTION_TIME_300MS \
				|KEY_PRESS_TIME_200MS)

#define REG_TIMING2_VALUE	(LONGKEY_PRESS_TIME_500MS)

#define INT_KEY_MASK		(INT_LKR|INT_LKP|INT_KP) 
#define BTN2_KEY_MASK		(0x18)
#define BTN2_KEY_SHIFT		(0x3)
#else
#define REG_INT1_MASK_VALUE	(0x1ffc)
#define REG_TIMING1_VALUE	(ADC_DETECTION_TIME_500MS)
#define REG_TIMING2_VALUE	(0x0)	
#define INT_KEY_MASK		(0x0) 
#define BTN2_KEY_MASK		(0x0)
#define BTN2_KEY_SHIFT		(0x0)
#endif

int usb_cable, uart_cable;

struct fsa9480_usbsw {
	struct i2c_client		*client;
	struct fsa9480_platform_data	*pdata;
	int				intr;
	int				dev1;
	int				dev2;
	int				mansw;
	bool				is_usb_ready;
	struct delayed_work		usb_work;
};

static struct fsa9480_usbsw *local_usbsw;

#ifdef CONFIG_USB_ANDROID_SAMSUNG_COMPOSITE
extern u16 askonstatus;
extern u16 inaskonstatus;
#endif

static ssize_t fsa9480_show_control(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	struct fsa9480_usbsw *usbsw = dev_get_drvdata(dev);
	struct i2c_client *client = usbsw->client;
	int value;

	value = i2c_smbus_read_byte_data(client, FSA9480_REG_CTRL);
	if (value < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, value);

	return sprintf(buf, "CONTROL: %02x\n", value);
}

static ssize_t fsa9480_show_device_type(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	struct fsa9480_usbsw *usbsw = dev_get_drvdata(dev);
	struct i2c_client *client = usbsw->client;
	int value;

	value = i2c_smbus_read_byte_data(client, FSA9480_REG_DEV_T1);
	if (value < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, value);

	return sprintf(buf, "DEVICE_TYPE: %02x\n", value);
}

static ssize_t fsa9480_show_manualsw(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct fsa9480_usbsw *usbsw = dev_get_drvdata(dev);
	struct i2c_client *client = usbsw->client;
	unsigned int value;

	value = i2c_smbus_read_byte_data(client, FSA9480_REG_MANSW1);
	if (value < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, value);

	if (value == SW_VAUDIO)
		return sprintf(buf, "VAUDIO\n");
	else if (value == SW_UART)
		return sprintf(buf, "UART\n");
	else if (value == SW_AUDIO)
		return sprintf(buf, "AUDIO\n");
	else if (value == SW_DHOST)
		return sprintf(buf, "DHOST\n");
	else if (value == SW_AUTO)
		return sprintf(buf, "AUTO\n");
	else
		return sprintf(buf, "%x", value);
}

static ssize_t fsa9480_set_manualsw(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	struct fsa9480_usbsw *usbsw = dev_get_drvdata(dev);
	struct i2c_client *client = usbsw->client;
	unsigned int value;
	unsigned int path = 0;
	unsigned int mansw1_value;
	int ret;

	value = i2c_smbus_read_byte_data(client, FSA9480_REG_CTRL);
	if (value < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, value);

	if ((value & ~(CON_SWITCH_OPEN | CON_MANUAL_SW)) != (CON_RAW_DATA | CON_WAIT))
		return 0;

	if (!strncmp(buf, "VAUDIO", 6)) {
		path = SW_VAUDIO;
		value &= ~CON_MANUAL_SW;
		value |= CON_SWITCH_OPEN;
	} else if (!strncmp(buf, "UART", 4)) {
		path = SW_UART;
		value &= ~CON_MANUAL_SW;
		value |= CON_SWITCH_OPEN;
	} else if (!strncmp(buf, "AUDIO", 5)) {
		path = SW_AUDIO;
		value &= ~CON_MANUAL_SW;
		value |= CON_SWITCH_OPEN;
	} else if (!strncmp(buf, "DHOST", 5)) {
		path = SW_DHOST;
		value &= ~CON_MANUAL_SW;
		value |= CON_SWITCH_OPEN;
	} else if (!strncmp(buf, "AUTO", 4)) {
		path = SW_AUTO;
		value |= CON_MANUAL_SW;
		value |= CON_SWITCH_OPEN;
	} else if (!strncmp(buf, "OPEN", 4)) {
		path = SW_AUTO;
		value |= CON_MANUAL_SW;
		value &= ~CON_SWITCH_OPEN;
	} else {
		dev_err(dev, "Wrong command\n");
		return 0;
	}

	usbsw->mansw = path;

	mansw1_value = i2c_smbus_read_byte_data(client, FSA9480_REG_MANSW1);
	if (mansw1_value < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, mansw1_value);

	mansw1_value = mansw1_value & (0x03);	// clear D+ and D- switching
	mansw1_value |= path;

	dev_info(&client->dev, "%s: manual sw1 write 0x%x\n", __func__, mansw1_value);

	ret = i2c_smbus_write_byte_data(client, FSA9480_REG_MANSW1, mansw1_value);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	ret = i2c_smbus_write_byte_data(client, FSA9480_REG_CTRL, value);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	usbsw->mansw = mansw1_value;

	return count;
}

static DEVICE_ATTR(control, S_IRUGO, fsa9480_show_control, NULL);
static DEVICE_ATTR(device_type, S_IRUGO, fsa9480_show_device_type, NULL);
static DEVICE_ATTR(switch, S_IRUGO | S_IWUSR,
		fsa9480_show_manualsw, fsa9480_set_manualsw);

static struct attribute *fsa9480_attributes[] = {
	&dev_attr_control.attr,
	&dev_attr_device_type.attr,
	&dev_attr_switch.attr,
	NULL
};

static const struct attribute_group fsa9480_group = {
	.attrs = fsa9480_attributes,
};

void fsa9480_manual_switching(int path)
{
	struct i2c_client *client = local_usbsw->client;
	unsigned int value;
	unsigned int data = 0;
	unsigned int mansw1_value;
	int ret;

	value = i2c_smbus_read_byte_data(client, FSA9480_REG_CTRL);
	if (value < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, value);

	if ((value & ~(CON_SWITCH_OPEN | CON_MANUAL_SW)) != (CON_RAW_DATA | CON_WAIT))
		return;

	if (path == SWITCH_PORT_VAUDIO) {
		data = SW_VAUDIO;
		value &= ~CON_MANUAL_SW;
		value |= CON_SWITCH_OPEN;
	} else if (path ==  SWITCH_PORT_UART) {
		data = SW_UART;
		value &= ~CON_MANUAL_SW;
		value |= CON_SWITCH_OPEN;
	} else if (path ==  SWITCH_PORT_AUDIO) {
		data = SW_AUDIO;
		value &= ~CON_MANUAL_SW;
		value |= CON_SWITCH_OPEN;
	} else if (path ==  SWITCH_PORT_USB) {
		data = SW_DHOST;
		value &= ~CON_MANUAL_SW;
		value |= CON_SWITCH_OPEN;
	} else if (path ==  SWITCH_PORT_AUTO) {
		data = SW_AUTO;
		value |= CON_MANUAL_SW;
		value |= CON_SWITCH_OPEN;
	} else if (path == SWITCH_PORT_OPEN) {
		data = SW_AUTO;
		value |= CON_MANUAL_SW;
		value &= ~CON_SWITCH_OPEN;
	} else {
		printk("%s: wrong path (%d)\n", __func__, path);
		return;
	}

	local_usbsw->mansw = data;

	mansw1_value = i2c_smbus_read_byte_data(client, FSA9480_REG_MANSW1);
	if (mansw1_value < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, mansw1_value);

	mansw1_value = mansw1_value & (0x03);	// clear D+ and D- switching
	mansw1_value |= data;

	dev_info(&client->dev, "%s: manual sw1 write 0x%x\n", __func__, mansw1_value);

	ret = i2c_smbus_write_byte_data(client, FSA9480_REG_MANSW1, mansw1_value);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	ret = i2c_smbus_write_byte_data(client, FSA9480_REG_CTRL, value);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	local_usbsw->mansw = mansw1_value;
}
EXPORT_SYMBOL(fsa9480_manual_switching);

#if defined(CONFIG_MACH_STEALTHV) || defined(CONFIG_MACH_AEGIS) || defined(CONFIG_MACH_VIPER) || defined(CONFIG_MACH_CHIEF)
extern void sec_usb_switch(void);
extern void sec_uart_switch(void);
#endif

/* check OCP and OVP */
static int fsa9480_check_valid_charger(struct fsa9480_usbsw *usbsw)
{
	//if ((usbsw->intr & INT_OCP_EN) || (usbsw->intr & INT_OVP_EN))
	if (usbsw->intr & INT_OVP_EN)
		return 0;

	return 1;
}

static void fsa9480_detect_dev(struct fsa9480_usbsw *usbsw)
{
	int device_type, ret;
	unsigned char val1, val2;
	struct fsa9480_platform_data *pdata = usbsw->pdata;
	struct i2c_client *client = usbsw->client;
	unsigned int value;

	device_type = i2c_smbus_read_word_data(client, FSA9480_REG_DEV_T1);
	if (device_type < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, device_type);

	val1 = device_type & 0xff;
	val2 = device_type >> 8;

	dev_info(&client->dev, "dev1: 0x%x, dev2: 0x%x\n", val1, val2);

	/* Attached */
	if (val1 || val2) {
		/* USB */
		if (val1 & DEV_T1_USB_MASK || val2 & DEV_T2_USB_MASK) {
			if (pdata->usb_charger_cb && fsa9480_check_valid_charger(usbsw))
				pdata->usb_charger_cb(FSA9480_ATTACHED);
			if (pdata->usb_cb && usbsw->is_usb_ready)
				pdata->usb_cb(FSA9480_ATTACHED);
#if defined(CONFIG_MACH_STEALTHV) || defined(CONFIG_MACH_AEGIS)
			/* For only LTE device */
			sec_usb_switch();
#else
			if (usbsw->mansw) {
				ret = i2c_smbus_write_byte_data(client,
					FSA9480_REG_MANSW1, usbsw->mansw);
				if (ret < 0)
					dev_err(&client->dev,
						"%s: err %d\n", __func__, ret);
			}
#endif
		/* UART */
		} else if (val1 & DEV_T1_UART_MASK || val2 & DEV_T2_UART_MASK) {
			if (pdata->uart_cb)
				pdata->uart_cb(FSA9480_ATTACHED);
#if defined(CONFIG_MACH_STEALTHV) || defined(CONFIG_MACH_AEGIS) 
			/* For only LTE device */
			sec_uart_switch();
#else
			if (usbsw->mansw) {
				ret = i2c_smbus_write_byte_data(client,
					FSA9480_REG_MANSW1, SW_UART);
				if (ret < 0)
					dev_err(&client->dev,
						"%s: err %d\n", __func__, ret);
			}
#endif
			if (val2 & DEV_T2_JIG_MASK) {
				if (pdata->jig_cb)
					pdata->jig_cb(FSA9480_ATTACHED);
			}
		/* CHARGER */
		} else if (val1 & DEV_T1_CHARGER_MASK) {
			if (pdata->charger_cb && fsa9480_check_valid_charger(usbsw))
				pdata->charger_cb(FSA9480_ATTACHED);
		/* Desk Dock (Stealth-V, Aegis) */
		} else if (val1 & DEV_AUDIO_1) {
#if defined (CONFIG_STEALTHV_USA) || defined (CONFIG_MACH_AEGIS) || defined(CONFIG_MACH_VIPER) || defined(CONFIG_MACH_CHIEF)
		if (pdata->deskdock_cb)
			pdata->deskdock_cb(FSA9480_ATTACHED);
				
			value = i2c_smbus_read_byte_data(client, FSA9480_REG_MANSW1);
			if (value < 0)
				dev_err(&client->dev, "%s: err %d\n", __func__, value);

			value = value | 0x1;	// vbus connected to charger

			dev_info(&client->dev, "DEV_AUDIO_1(ATTACH) -> manual sw1 write 0x%x\n", value);

			ret = i2c_smbus_write_byte_data(client,
					FSA9480_REG_MANSW1, value);
			if (ret < 0)
				dev_err(&client->dev,
					"%s: err %d\n", __func__, ret);

			ret = i2c_smbus_read_byte_data(client,
					FSA9480_REG_CTRL);
			if (ret < 0)
				dev_err(&client->dev,
					"%s: err %d\n", __func__, ret);

			ret = i2c_smbus_write_byte_data(client,
					FSA9480_REG_CTRL, ret & ~CON_MANUAL_SW);
			if (ret < 0)
				dev_err(&client->dev,
					"%s: err %d\n", __func__, ret);		
#endif
		/* JIG */
		} else if (val2 & DEV_T2_JIG_MASK) {
			if (pdata->jig_cb)
				pdata->jig_cb(FSA9480_ATTACHED);
		/* Desk Dock (Stealth-V, Aegis: Car Dock) */
		} else if (val2 & DEV_AV) {
#if defined (CONFIG_STEALTHV_USA) || defined (CONFIG_MACH_AEGIS) || defined(CONFIG_MACH_VIPER) || defined(CONFIG_MACH_CHIEF)
			if (pdata->cardock_cb)
				pdata->cardock_cb(FSA9480_ATTACHED);
#else
			if (pdata->deskdock_cb)
				pdata->deskdock_cb(FSA9480_ATTACHED);
#endif
			ret = i2c_smbus_write_byte_data(client,
					FSA9480_REG_MANSW1, SW_VAUDIO);
			if (ret < 0)
				dev_err(&client->dev,
					"%s: err %d\n", __func__, ret);

			ret = i2c_smbus_read_byte_data(client,
					FSA9480_REG_CTRL);
			if (ret < 0)
				dev_err(&client->dev,
					"%s: err %d\n", __func__, ret);

			ret = i2c_smbus_write_byte_data(client,
					FSA9480_REG_CTRL, ret & ~CON_MANUAL_SW);
			if (ret < 0)
				dev_err(&client->dev,
					"%s: err %d\n", __func__, ret);
		} 	/* Detached */
	} else {
		/* USB */
		if (usbsw->dev1 & DEV_T1_USB_MASK ||
				usbsw->dev2 & DEV_T2_USB_MASK) {
			if (pdata->usb_charger_cb)
				pdata->usb_charger_cb(FSA9480_DETACHED);
#ifndef CONFIG_USB_ANDROID_SAMSUNG_COMPOSITE
			if (pdata->usb_cb && usbsw->is_usb_ready)
				pdata->usb_cb(FSA9480_DETACHED);
#else
                        /* Mass storage icon bug fix : ask on mode */
			if (pdata->usb_cb && usbsw->is_usb_ready) {
                            if ( askonstatus == 0xabcd )
                            {
                                askonstatus=0;
                                inaskonstatus=0;
                                pdata->usb_cb(FSA9480_DETACHED);
                                askonstatus = 0xabcd;
                            }
                            else
                            {
                                 askonstatus=0;
                                inaskonstatus=0;
                                pdata->usb_cb(FSA9480_DETACHED);                               
                            }
			}
#endif
		/* UART */
		} else if (usbsw->dev1 & DEV_T1_UART_MASK ||
				usbsw->dev2 & DEV_T2_UART_MASK) {
			if (pdata->uart_cb)
				pdata->uart_cb(FSA9480_DETACHED);
			if (usbsw->dev2 & DEV_T2_JIG_MASK) {
				if (pdata->jig_cb)
					pdata->jig_cb(FSA9480_DETACHED);
			}
		/* CHARGER */
		} else if (usbsw->dev1 & DEV_T1_CHARGER_MASK) {
			if (pdata->charger_cb)
				pdata->charger_cb(FSA9480_DETACHED);
		/* Desk Dock (Stealth-V, Aegis) */
		} else if (usbsw->dev1 & DEV_AUDIO_1) {
#if defined (CONFIG_STEALTHV_USA) || defined (CONFIG_MACH_AEGIS) || defined(CONFIG_MACH_VIPER) || defined(CONFIG_MACH_CHIEF)
			if (pdata->deskdock_cb)
				pdata->deskdock_cb(FSA9480_DETACHED);

			value = i2c_smbus_read_byte_data(client, FSA9480_REG_MANSW1);
			if (value < 0)
				dev_err(&client->dev, "%s: err %d\n", __func__, value);

			value = value & ~(0x1);	// clear vbus switching to auto-switch

			dev_info(&client->dev, "DEV_AUDIO_1(DETACH) -> manual sw1 write 0x%x\n", value);

			ret = i2c_smbus_write_byte_data(client,
					FSA9480_REG_MANSW1, value);
			if (ret < 0)
				dev_err(&client->dev,
					"%s: err %d\n", __func__, ret);

			ret = i2c_smbus_read_byte_data(client,
					FSA9480_REG_CTRL);
			if (ret < 0)
				dev_err(&client->dev,
					"%s: err %d\n", __func__, ret);

			ret = i2c_smbus_write_byte_data(client,
					FSA9480_REG_CTRL, ret | CON_MANUAL_SW);
			if (ret < 0)
				dev_err(&client->dev,
					"%s: err %d\n", __func__, ret);			
#endif
		/* JIG */
		} else if (usbsw->dev2 & DEV_T2_JIG_MASK) {
			if (pdata->jig_cb)
				pdata->jig_cb(FSA9480_DETACHED);
		/* Desk Dock (Stealth-V, Aegis: Car Dock) */
		} else if (usbsw->dev2 & DEV_AV) {
#if defined (CONFIG_STEALTHV_USA) || defined (CONFIG_MACH_AEGIS) || defined(CONFIG_MACH_VIPER) || defined(CONFIG_MACH_CHIEF)
			if (pdata->cardock_cb)
				pdata->cardock_cb(FSA9480_DETACHED);
#else
			if (pdata->deskdock_cb)
				pdata->deskdock_cb(FSA9480_DETACHED);
#endif
			ret = i2c_smbus_read_byte_data(client,
					FSA9480_REG_CTRL);
			if (ret < 0)
				dev_err(&client->dev,
					"%s: err %d\n", __func__, ret);

			ret = i2c_smbus_write_byte_data(client,
					FSA9480_REG_CTRL, ret | CON_MANUAL_SW);
			if (ret < 0)
				dev_err(&client->dev,
					"%s: err %d\n", __func__, ret);
		}
	}

	usb_cable = val1;
	uart_cable = val2;
	usbsw->dev1 = val1;
	usbsw->dev2 = val2;
}

#if defined(CONFIG_MACH_STEALTHV) || defined(CONFIG_MACH_AEGIS) || defined(CONFIG_MACH_VIPER) || defined(CONFIG_MACH_CHIEF)
static void fsa9480_detect_key(struct fsa9480_usbsw *usbsw)
{
	int button2;
	bool is_press;
	bool is_long;
	unsigned char key_mask;
	unsigned int key_index;
	//struct fsa9480_platform_data *pdata = usbsw->pdata;
	struct i2c_client *client = usbsw->client;

	button2 = i2c_smbus_read_word_data(client, FSA9480_REG_BTN2);
	if (button2 < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, button2);

	if (usbsw->intr & INT_KP) {
		is_long = 0;
	} else {
		is_long = 1;

		if (usbsw->intr & INT_LKP)
			is_press = 1;
		else
			is_press = 0;
	}

	dev_info(&client->dev, "button2 0x%x, is_press %d, is_long %d\n",
			button2, is_press, is_long);

	key_mask = (button2 & BTN2_KEY_MASK) >> BTN2_KEY_SHIFT;
	key_index = 0;

	while (key_mask) {
		if( key_mask & 0x1 ) {
			if (usbsw->pdata->key_cb) {
				if (is_long) {
					usbsw->pdata->key_cb(key_index, is_press);
				}
				else {
					usbsw->pdata->key_cb(key_index, 1);
					usbsw->pdata->key_cb(key_index, 0);
				}
			}
		}

		key_mask >>= 1;
		key_index++;
	}				
}
#endif

static void fsa9480_usb_detect(struct work_struct *work)
{
	struct fsa9480_usbsw *usbsw = container_of(work,
			struct fsa9480_usbsw, usb_work.work);

	usbsw->is_usb_ready = true;

	fsa9480_detect_dev(usbsw);
}

static void fsa9480_reg_init(struct fsa9480_usbsw *usbsw)
{
	struct i2c_client *client = usbsw->client;
	unsigned int ctrl = CON_MASK;
	int ret;

	/* mask interrupts (unmask attach/detach only) */
	ret = i2c_smbus_write_word_data(client, FSA9480_REG_INT1_MASK, REG_INT1_MASK_VALUE);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	/* mask all car kit interrupts */
	ret = i2c_smbus_write_word_data(client, FSA9480_REG_CK_INTMASK1, 0x07ff);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	/* Timing1 - Keypress, ADC Dectet Time */
	ret = i2c_smbus_write_byte_data(client, FSA9480_REG_TIMING1, REG_TIMING1_VALUE);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	/* Timing2 - Long Key Press */
	ret = i2c_smbus_write_byte_data(client, FSA9480_REG_TIMING2, REG_TIMING2_VALUE);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	usbsw->mansw = i2c_smbus_read_byte_data(client, FSA9480_REG_MANSW1);
	if (usbsw->mansw < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, usbsw->mansw);

	if (usbsw->mansw)
		ctrl &= ~CON_MANUAL_SW;	/* Manual Switching Mode */

	ret = i2c_smbus_write_byte_data(client, FSA9480_REG_CTRL, ctrl);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);
}

static irqreturn_t fsa9480_irq_thread(int irq, void *data)
{
	struct fsa9480_usbsw *usbsw = data;
	struct i2c_client *client = usbsw->client;
	int intr;

	/* read and clear interrupt status bits */
	intr = i2c_smbus_read_word_data(client, FSA9480_REG_INT1);
	if (intr < 0) {
		dev_err(&client->dev, "%s: err %d\n", __func__, intr);
	} else if (intr == 0) {
		/* interrupt was fired, but no status bits were set,
		so device was reset. In this case, the registers were
		reset to defaults so they need to be reinitialised. */
		fsa9480_reg_init(usbsw);
	}

	usbsw->intr = intr;

	pr_info("%s: intr1 = 0x%x\n", __func__, intr);

#if defined(CONFIG_MACH_STEALTHV) || defined(CONFIG_MACH_AEGIS) || defined(CONFIG_MACH_VIPER) || defined(CONFIG_MACH_CHIEF)
	if (intr & INT_KEY_MASK)
		/* key detection */
		fsa9480_detect_key(usbsw);
	else
#endif
		/* device detection */
		fsa9480_detect_dev(usbsw);

	return IRQ_HANDLED;
}

static int fsa9480_irq_init(struct fsa9480_usbsw *usbsw)
{
	struct i2c_client *client = usbsw->client;
	int ret;

	if (client->irq) {
		ret = request_threaded_irq(client->irq, NULL,
			fsa9480_irq_thread, IRQF_TRIGGER_FALLING,
			"fsa9480 micro USB", usbsw);
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

static int __devinit fsa9480_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct fsa9480_usbsw *usbsw;
	int ret = 0;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
		return -EIO;

	usbsw = kzalloc(sizeof(struct fsa9480_usbsw), GFP_KERNEL);
	if (!usbsw) {
		dev_err(&client->dev, "failed to allocate driver data\n");
		return -ENOMEM;
	}

	usbsw->client = client;
	usbsw->pdata = client->dev.platform_data;
	if (!usbsw->pdata)
		goto fail1;

	i2c_set_clientdata(client, usbsw);

	local_usbsw = usbsw;  // temp

	if (usbsw->pdata->cfg_gpio)
		usbsw->pdata->cfg_gpio();

	fsa9480_reg_init(usbsw);

	ret = fsa9480_irq_init(usbsw);
	if (ret)
		goto fail1;

	ret = sysfs_create_group(&client->dev.kobj, &fsa9480_group);
	if (ret) {
		dev_err(&client->dev,
				"failed to create fsa9480 attribute group\n");
		goto fail2;
	}

	if (usbsw->pdata->reset_cb)
		usbsw->pdata->reset_cb();

	/* device detection */
	fsa9480_detect_dev(usbsw);

	// set fsa9480 init flag.
	if (usbsw->pdata->set_init_flag)
		usbsw->pdata->set_init_flag();

	INIT_DELAYED_WORK(&usbsw->usb_work, fsa9480_usb_detect);
	schedule_delayed_work(&usbsw->usb_work, msecs_to_jiffies(17000));

	return 0;

fail2:
	if (client->irq)
		free_irq(client->irq, usbsw);
fail1:
	i2c_set_clientdata(client, NULL);
	kfree(usbsw);
	return ret;
}

static int __devexit fsa9480_remove(struct i2c_client *client)
{
	struct fsa9480_usbsw *usbsw = i2c_get_clientdata(client);

	if (client->irq) {
		disable_irq_wake(client->irq);
		free_irq(client->irq, usbsw);
	}
	i2c_set_clientdata(client, NULL);

	cancel_delayed_work(&usbsw->usb_work);

	sysfs_remove_group(&client->dev.kobj, &fsa9480_group);
	kfree(usbsw);
	return 0;
}

#ifdef CONFIG_PM
static int fsa9480_resume(struct i2c_client *client)
{
	struct fsa9480_usbsw *usbsw = i2c_get_clientdata(client);

	/* device detection */
	fsa9480_detect_dev(usbsw);

	return 0;
}

#else

#define fsa9480_suspend NULL
#define fsa9480_resume NULL

#endif /* CONFIG_PM */

static const struct i2c_device_id fsa9480_id[] = {
	{"fsa9480", 0},
	{}
};
MODULE_DEVICE_TABLE(i2c, fsa9480_id);

static struct i2c_driver fsa9480_i2c_driver = {
	.driver = {
		.name = "fsa9480",
	},
	.probe = fsa9480_probe,
	.remove = __devexit_p(fsa9480_remove),
	.resume = fsa9480_resume,
	.id_table = fsa9480_id,
};

static int __init fsa9480_init(void)
{
	return i2c_add_driver(&fsa9480_i2c_driver);
}
module_init(fsa9480_init);

static void __exit fsa9480_exit(void)
{
	i2c_del_driver(&fsa9480_i2c_driver);
}
module_exit(fsa9480_exit);

MODULE_AUTHOR("Minkyu Kang <mk7.kang@samsung.com>");
MODULE_DESCRIPTION("FSA9480 USB Switch driver");
MODULE_LICENSE("GPL");
