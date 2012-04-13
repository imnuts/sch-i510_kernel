/*
 * linux/drivers/power/maxim_dallas.c
 *
 * Battery measurement code for S5PC11x platform.
 *
 * Copyright (C) 2010 Samsung Electronics.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/spinlock.h>

#include <mach/gpio.h>

#define READ_ROM_MAXIM  0x33
#define SKIP_ROM        0xCC
#define READ_MEMORY     0xF0
#define MAXIM_ADDRESS1  0x1F
#define MAXIM_ADDRESS2  0x00

#define GPIO_LEVEL_LOW	0
#define GPIO_LEVEL_HIGH	1

#define WAIT_X(microsec)	udelay(microsec)
#define DELAY_TIME	100

#define DEBUG

spinlock_t intlock;
static int intlock_init = 0;

static void port_out_low(void)
{
	s3c_gpio_cfgpin(GPIO_BATT_ID, S3C_GPIO_OUTPUT);
	gpio_set_value(GPIO_BATT_ID, GPIO_LEVEL_LOW);
}

static void port_out_high(void)
{
	s3c_gpio_cfgpin(GPIO_BATT_ID, S3C_GPIO_OUTPUT);
	gpio_set_value(GPIO_BATT_ID, GPIO_LEVEL_HIGH);
}

static void port_input_config(void)
{
	s3c_gpio_cfgpin(GPIO_BATT_ID, S3C_GPIO_INPUT);
}

static int port_input(void)
{
	int input_value;

	port_input_config();
	input_value = gpio_get_value(GPIO_BATT_ID);

	if (input_value)
		return 1;
	else 
		return 0;
}

// Generate a 1-Wire reset, return 1 if no
// presence detect was found, return 0 otherwise.
int Reset(void)  
{
	int result;
	unsigned long flags;

	if (intlock_init == 0) {
		spin_lock_init(&intlock);
		intlock_init = 1;
	}

	spin_lock_irqsave(&intlock, flags);
	port_out_low();
	WAIT_X(500);
	port_out_high();
	WAIT_X(67);
	result = port_input() & 0x01;
	WAIT_X(420);
	spin_unlock_irqrestore(&intlock, flags);
	WAIT_X(DELAY_TIME);

	return result;
}

int Reset_TA(void)  
{
	int result;
	unsigned long flags;

	if (intlock_init == 0) {
		spin_lock_init(&intlock);
		intlock_init = 1;
	}

	spin_lock_irqsave(&intlock, flags);
	port_out_low();
	WAIT_X(500);
	port_out_high();
	WAIT_X(67);
	result = port_input() & 0x01;
	spin_unlock_irqrestore(&intlock, flags);
	WAIT_X(DELAY_TIME);

	return result;
}

static void Write1(void)
{
	unsigned long flags;

	if (intlock_init == 0) {
		spin_lock_init(&intlock);
		intlock_init = 1;
	}

	spin_lock_irqsave(&intlock, flags);
	port_out_low();
	WAIT_X(1);
	port_out_high();
	WAIT_X(59);
	spin_unlock_irqrestore(&intlock, flags);
}

static void Write0(void)
{
	unsigned long flags;

	if (intlock_init == 0) {
		spin_lock_init(&intlock);
		intlock_init = 1;
	}

	spin_lock_irqsave(&intlock, flags);
	port_out_low();
	WAIT_X(55);
	port_out_high();
	WAIT_X(5);
	spin_unlock_irqrestore(&intlock, flags);
}

static int Readx(void)
{
	int result;
	unsigned long flags;

	if (intlock_init == 0) {
		spin_lock_init(&intlock);
		intlock_init = 1;
	}

	spin_lock_irqsave(&intlock, flags);
	port_out_low();
	WAIT_X(1);
	port_out_high();
	WAIT_X(15);   
	result = port_input() & 0x01;
	WAIT_X(45);
	spin_unlock_irqrestore(&intlock, flags);

	return result;
}

static void maxim_WriteByte(int Data)
{
	int i;

	for (i = 0; i < 8; i++) {
		if (Data & (0x01 << i))
			Write1();
		else
			Write0();
	}
}

static int maxim_ReadByte(void)
{
	int i;
	int result = 0;

	for (i = 0; i < 8; i++)
		result = result + (Readx() << i);

	return result;
}

static int CRC(int* data, int length)
{
	int CRC = 0;
	int byte;
	int bit;

	for (byte = 0; byte < length; byte++) {
		for (bit = 0; bit <= 7; bit++) {
			if ((data[byte] >> bit & 0x01) == (CRC & 0x01))
				CRC = CRC >> 1;
			else {
				CRC = CRC >> 1;
				if (CRC & 0x04)
					CRC = CRC - 4;
				else
					CRC = CRC + 4;

				if (CRC & 0x08)
					CRC = CRC - 8;
				else
					CRC = CRC + 8;

				if (CRC & 0x80)
					CRC = CRC - 128;
				else
					CRC = CRC + 128;
			}
		}
	}

	return CRC;
}

int vzw_rcode[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };	// info
int vzw_crc1 = 0;	// info
int vzw_crc2 = 0;	// info

//#define AUTH_TEST

#ifdef AUTH_TEST
int test_rcode[8] = { 159, 215, 255, 79, 15, 64, 65, 234 };
#endif

static int rom_code_protection(void)
{
	int i;
	int result[8];
	int retVal;

	if (intlock_init == 0) {
		spin_lock_init(&intlock);
		intlock_init = 1;
	}

	spin_lock_init(&intlock);    

	if (!Reset()) {
		maxim_WriteByte(READ_ROM_MAXIM);
		for (i = 0; i < 8; i++) {
			result[i] = maxim_ReadByte();
			vzw_rcode[i] = result[i];
#ifdef AUTH_TEST
			result[i] = test_rcode[i];
			vzw_rcode[i] = test_rcode[i];
#endif
		}

		vzw_crc1 = CRC(result, 7);

#ifdef DEBUG
		printk("/BATT_ID/ rom code =");
		for (i = 0; i < 8; i++)
			printk(" %d", result[i]);
		printk("\n");
		printk("/BATT_ID/ rom code crc = %d\n", vzw_crc1);
#endif

		if ((result[7] == vzw_crc1/*CRC(result, 7)*/) &&
			(((result[5] >> 4) & 0x0F) == 0x04) && (result[6] == 0x41))
			retVal = 1;
		else
			retVal = 0;
	}
	else 
		retVal = 0;

	WAIT_X(DELAY_TIME);
	return retVal;
}

static int CRC_protection(void)
{
	int result;
	int protection_data[3];
	int retVal;

	protection_data[0] = READ_MEMORY;
	protection_data[1] = MAXIM_ADDRESS1;
	protection_data[2] = MAXIM_ADDRESS2;

	if (!Reset()) {
		maxim_WriteByte(SKIP_ROM);
		maxim_WriteByte(READ_MEMORY);
		maxim_WriteByte(MAXIM_ADDRESS1);
		maxim_WriteByte(MAXIM_ADDRESS2);

		result = maxim_ReadByte();
		vzw_crc2 = CRC(protection_data, 3);
#ifdef DEBUG
		printk("/BATT_ID/ crc = %d\n", vzw_crc2);
#endif
		if (result == vzw_crc2/*CRC(protection_data, 3)*/)
			retVal = 1;
		else
			retVal = 0;
	}
	else
		retVal = 0;

	WAIT_X(DELAY_TIME);
	return retVal;
}

int verizon_batt_auth_full_check(void)
{
	int i;
	int retval = 0;

	/* Retry 3 times */
	for (i = 0; i < 4; i++) {

		msleep(100);

		if (Reset_TA()) {
			retval = 0;
			printk("/BATT_ID/ full check retry = %d\n", i + 1);
			continue;
		}

		if (rom_code_protection() == 0)
			retval = 1;
		else if (CRC_protection() == 0)
			retval = 1;
		else {
			retval = 1;
			break;
		}

	}

	return retval;
}

static int _verizon_batt_auth_multi_check(void)
{
	int i;
	int retval = 0;

	/* Retry 3 times */
	for (i = 0; i < 3 ; i++) {
		if (!Reset_TA())
			retval = 1;

		if(retval)
			break;
	}

	return retval;
}

int verizon_batt_auth_check(void)
{
	int result = 0;

	if (!Reset_TA())
		return 1;
	else
		result = _verizon_batt_auth_multi_check();

	return result;
}

