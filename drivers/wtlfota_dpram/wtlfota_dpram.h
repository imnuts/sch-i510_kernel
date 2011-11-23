/****************************************************************************
**
** COPYRIGHT(C) : Samsung Electronics Co.Ltd, 2006-2011
**
** AUTHOR       : Song Wei  			@LDK@
** DESCRIPTION: wtlfota_dpram.h: copied from Viswanath, Puttagunta's dpram.h.  removed unused stuff  @LDK@
** REFERENCES: Stealth dpram driver (dpram.c/.h)
*   This program is free software; you can redistribute it and/or modify it
*   under the terms of the GNU General Public License version 2 as published
*   by the Free Software Foundation.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License along
*   with this program; if not, write to the Free Software Foundation, Inc., 59
*   Temple Place, Suite 330, Boston, MA 02111-1307, USA.
*
****************************************************************************/

#ifndef __WTLFOTA_DPRAM_H__
#define __WTLFOTA_DPRAM_H__

/* 16KB Size */
#define WTLFOTA_DPRAM_SIZE										0x4000
/* Memory Address */
#define WTLFOTA_DPRAM_START_ADDRESS 							0x0000


/*CHNGED FOR LTE - Swapped the address*/
#define WTLFOTA_DPRAM_PDA2PHONE_INTERRUPT_ADDRESS			(WTLFOTA_DPRAM_START_ADDRESS + 0x3FFE)
#define WTLFOTA_DPRAM_PHONE2PDA_INTERRUPT_ADDRESS			(WTLFOTA_DPRAM_START_ADDRESS + 0x3FFC)

#define WTLFOTA_DPRAM_INTERRUPT_PORT_SIZE						2

/*CHNGED FOR LTE -*/
//#define WTLFOTA_DPRAM_START_ADDRESS_PHYS 		    0x30000000		//Need to verify this.
#define WTLFOTA_DPRAM_START_ADDRESS_PHYS 		    0x98000000 
#define WTLFOTA_DPRAM_SHARED_BANK					0x00000000		// In InstincQ, this has an offset from WTLFOTA_DPRAM_START_ADDRESS_PHYS. In Via6410 no offset			

#define WTLFOTA_DPRAM_SHARED_BANK_SIZE				0x4000		// 16 * 1024 bytes


typedef struct wtlfota_dpram_device {
	/* WTLFOTA_DPRAM memory addresses */
	u_int16_t mask_req_ack;
	u_int16_t mask_res_ack;
	u_int16_t mask_send;
} wtlfota_dpram_device_t;


/* currently supported gpios: */
/* #define GPIO_TRANSLATION_LIST_LEN 4 */
/* static gpio_name_value_pair_t GPIO_TRANSLATION_LIST[GPIO_TRANSLATION_LIST_LEN]={ */
/*   {"GPIO_PHONE_ON", GPIO_PHONE_ON}, */
/*   {"GPIO_PHONE_RST_N", GPIO_PHONE_RST_N}, */
/*   {"GPIO_DPRAM_INT", GPIO_DPRAM_INT_N}, */
/*   {"GPIO_PHONE_ACTIVE", GPIO_PHONE_ACTIVE},  */
/* }; */
struct _gpio_param {
#define _WTLFOTA_GPIO_PARAM_NAME_LENGTH 20
  char name[_WTLFOTA_GPIO_PARAM_NAME_LENGTH];
  int data;
#define _WTLFOTA_GPIO_OP_WRITE 1
#define _WTLFOTA_GPIO_OP_READ 0
  int op;//1: write. 0: read. others could be added later
};

/*ioctl cmd numbers. we use the same magic number dpram is using for now since the two are not up at the same time.*/
#define IOC_SEC_MAGIC		(0xf0)
/* #define DPRAM_PHONE_ON		_IO(IOC_SEC_MAGIC, 0xc0) */
#define DPRAM_GPIO_OP		_IO(IOC_SEC_MAGIC, 0xc0)
/* #define DPRAM_PHONE_OFF		_IO(IOC_SEC_MAGIC, 0xc1) */
/* #define DPRAM_PHONE_GETSTATUS	_IOR(IOC_SEC_MAGIC, 0xc2, unsigned int) */
/* #define DPRAM_PHONE_RAMDUMP_ON	_IO(IOC_SEC_MAGIC, 0xc6) */
/* #define DPRAM_PHONE_RAMDUMP_OFF	_IO(IOC_SEC_MAGIC, 0xc7) */
/* #define DPRAM_EXTRA_MEM_RW	_IOWR(IOC_SEC_MAGIC, 0xc8, unsigned long) */
/* #define DPRAM_TEST		_IO(IOC_SEC_MAGIC, 0xcf) /\* TODO: reimplement. Dummy for now *\/ */

#define IOCTL_DEVICE "/dev/dpramctl"
#define IOCTL_DEVICE_NAME "dpramctl"
#define MMAP_DEVICE "/dev/uio0"


#endif	/* __WTLFOTA_DPRAM_H__ */
