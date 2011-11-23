/****************************************************************************
 **
 ** COPYRIGHT(C) : Samsung Electronics Co.Ltd, 2006-2011
 **
 ** AUTHOR       : Song Wei  			@LDK@
 **                WTLFOTA_DPRAM Device Driver for Via6410
 **			Reference: Via6419 DPRAM driver (dpram.c/.h)
 *
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
#ifndef _HSDPA_WTLFOTA_DPRAM
#define _HSDPA_WTLFOTA_DPRAM
#endif
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/irq.h>
#include <linux/poll.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <mach/regs-gpio.h>
#include <plat/gpio-cfg.h>
#include <mach/hardware.h>
#include <linux/miscdevice.h>
#include <mach/hardware.h>
#include <mach/map.h>
#include <mach/regs-mem.h>
#include <mach/gpio.h>
#include <mach/param.h>
#include <linux/sched.h>
#include <linux/dma-mapping.h>
#include "dpram_uio_driver.h"
#include "wtlfota_dpram.h"




/***************************************************************************/
/*                              GPIO SETTING                               */
/***************************************************************************/
#include <mach/gpio.h>

#define GPIO_LEVEL_LOW				0
#define GPIO_LEVEL_HIGH				1

#define IRQ_WTLFOTA_DPRAM_INT_N		        IRQ_EINT8
#define IRQ_PHONE_ACTIVE                        IRQ_EINT15

void reg_config(void)
{

  /* LTE-STEALTH Related config. CS related settings are done in Machine Init*/
  unsigned int regVal;

  /* SINGALS
     1) C110_WTLFOTA_DPRAM_nCS --> XM0CSN_3  ( ie Xm0CSn[3] MP0_1[3])
     2) C110_OE_N -->XM0OEN
     3) C110_LB -> XM0BEN_0
     4) C110_UB --> XM0BEN_1
     5) C110_WTLFOTA_DPRAM_INT_N --> XEINT_8 : how to config this one?
     6) C110_WE_N --> XM0WEN
     7) DATA LINES --> XM0DATA_0 to XM0DATA_15
     8) Address Lines -->XM0ADDR_0 to XM0ADDR_12 */
    
  //ADDR LINES //0xE0200340  and 0xE0200360
  regVal = 0x22222222;
  writel(regVal, S5PV210_GPA0_BASE + 0x0340);
		
  regVal = __raw_readl (S5PV210_GPA0_BASE + 0x0360);
  regVal |= 0x00022222;
  writel(regVal, S5PV210_GPA0_BASE + 0x0360);
	
  //DATA LINES MP06 and MP07 //0xE0200380 and 0xE02003A0
  regVal = 0x22222222;
  writel(regVal, S5PV210_GPA0_BASE + 0x0380);
      
  regVal = 0x22222222;
  writel(regVal,S5PV210_GPA0_BASE + 0x03A0);
}

static int claim_output_gpio(unsigned gpio, const char *label, int pull){
  if (gpio_is_valid(gpio)) {
    if (gpio_request(gpio, label)){
      printk("Failed to request %s!\n", label);
      return -1;
    }
    if(gpio_direction_output(gpio, pull)){
      printk("gpio_direction_output(%u) fail!\n", gpio);
      gpio_free(gpio);
      return -1;
    }else{
      return 0;
    }
  }else{
    printk("gpio %u is not valid!\n", gpio);
    return -1;
  }
}

static int claim_input_gpio(unsigned gpio, const char *label){
  if (gpio_is_valid(gpio)) {
    if (gpio_request(gpio, label)){
      printk("Failed to request %s!\n", label);
      return -1;
    }
    if(gpio_direction_input(gpio)){
      printk("gpio_direction_input(%u) fail!\n", gpio);
      gpio_free(gpio);
      return -1;
    }else{
      return 0;
    }
  }else{
    printk("gpio %u is not valid!\n", gpio);
    return -1;
  }
}

static void release_gpio(unsigned gpio){
  if (gpio_is_valid(gpio)) {
    gpio_free(gpio);
  }else{
    printk("gpio %u is not valid!\n", gpio);
  }
}

static int init_hw_setting(void)
{
  //	u32 mask;
  /* initial pin settings - wtlfota_dpram driver control */
  if(claim_input_gpio(GPIO_PHONE_ACTIVE, "wfdpram/PHONE_ACTIVE_AF")){
    return -1;
  }
  s3c_gpio_cfgpin(GPIO_PHONE_ACTIVE, S3C_GPIO_SFN(GPIO_PHONE_ACTIVE_AF));
  s3c_gpio_setpull(GPIO_PHONE_ACTIVE, S3C_GPIO_PULL_NONE); 
  set_irq_type(IRQ_PHONE_ACTIVE, IRQ_TYPE_EDGE_BOTH);

  if(claim_input_gpio(GPIO_DPRAM_INT_N, "wfdpram/INT_N_AF")){
    return -1;
  }
  s3c_gpio_cfgpin(GPIO_DPRAM_INT_N, S3C_GPIO_SFN(GPIO_DPRAM_INT_N_AF));
  s3c_gpio_setpull(GPIO_DPRAM_INT_N, S3C_GPIO_PULL_NONE); 
  set_irq_type(IRQ_WTLFOTA_DPRAM_INT_N, IRQ_TYPE_EDGE_FALLING);

  if(claim_output_gpio(GPIO_PHONE_ON, "wfdpram/PHONE_ON", GPIO_LEVEL_LOW)){
    return -1;
  }  
  s3c_gpio_setpull(GPIO_PHONE_ON, S3C_GPIO_PULL_NONE); 
  gpio_set_value(GPIO_PHONE_ON, GPIO_LEVEL_LOW);

  if(claim_output_gpio(GPIO_CP_RST, "wfdpram/PHONE_RST_N", GPIO_LEVEL_HIGH)){
    return -1;
  }  
  s3c_gpio_setpull(GPIO_CP_RST, S3C_GPIO_PULL_NONE); 
  return 0;
}

static void fini_hw_setting(void)
{
  release_gpio(GPIO_PHONE_ACTIVE);
  release_gpio(GPIO_PHONE_ON);
  release_gpio(GPIO_CP_RST);
  release_gpio(GPIO_DPRAM_INT_N);
}




static int  wtlfota_dpram_probe(void)
{
  int retval = 0;

  /* @LDK@ H/W setting */
  reg_config();
  //wait for a while
  mdelay(200);
  retval = init_hw_setting();
  if(retval){
    return retval;
  }

  /* @LDK@ check out missing interrupt from the phone */
  //check_miss_interrupt();

  return retval;
}


/***************************************************************************/
/*                              IOCTL                                      */
/***************************************************************************/

typedef struct gpio_name_value_pair{
  char name[_WTLFOTA_GPIO_PARAM_NAME_LENGTH];
  int value;
}gpio_name_value_pair_t;

#define GPIO_TRANSLATION_LIST_LEN 4
static gpio_name_value_pair_t GPIO_TRANSLATION_LIST[GPIO_TRANSLATION_LIST_LEN]={
  {"GPIO_PHONE_ON", GPIO_PHONE_ON},
  {"GPIO_PHONE_RST_N", GPIO_CP_RST},
  {"GPIO_DPRAM_INT", GPIO_DPRAM_INT_N},
  {"GPIO_PHONE_ACTIVE", GPIO_PHONE_ACTIVE}, 
};

unsigned int gpio_name_to_value(const char *name){
  unsigned val = -1;
  int i;
  for(i=0; i<GPIO_TRANSLATION_LIST_LEN; i++){
    if(!strncmp(name, GPIO_TRANSLATION_LIST[i].name, _WTLFOTA_GPIO_PARAM_NAME_LENGTH)){
      val = GPIO_TRANSLATION_LIST[i].value;
      break;
    }
  }
  return val;
}

static void dpram_gpio_op(struct _gpio_param *param)
{
  unsigned int gpio;
  gpio = gpio_name_to_value(param->name);
  if(gpio < 0){
    printk("gpio name not recognized in dpram_gpio_op\n");
    return;
  }
  switch(param->op){
  case _WTLFOTA_GPIO_OP_READ:
    param->data = gpio_get_value(gpio);
    break;
  case _WTLFOTA_GPIO_OP_WRITE:
    gpio_set_value(gpio, param->data);
    break;
  default:
    break;
  }
}


static int dpramctl_ioctl(struct inode *inode, struct file *file,
			  unsigned int cmd, unsigned long l)
{

  int ret;
  unsigned char *arg = (unsigned char *)l;
  switch (cmd) {
  case DPRAM_GPIO_OP:
    {
      struct _gpio_param param;
      ret = copy_from_user((void *)&param, (void *)arg, sizeof(param));
      if(ret != 0){
	printk("copy_from_user in dpramctl_ioctl failed!\n");
	return -EINVAL;
      }
      dpram_gpio_op(&param);
      if (param.op == _WTLFOTA_GPIO_OP_READ){
	return copy_to_user((unsigned long *)arg, &param, sizeof(param));
      }
      return 0;
    }
  default:
    break;
  }
  return -EINVAL;
}

static struct file_operations dpramctl_fops = {
  .owner =	THIS_MODULE,
  .ioctl =	dpramctl_ioctl,
  .llseek =	no_llseek,
};

//use the minor number of dpramctl_dev in dev
static struct miscdevice dpramctl_dev = {
  .minor =	132, //MISC_DYNAMIC_MINOR,
  .name =		IOCTL_DEVICE_NAME,
  .fops =		&dpramctl_fops,
};

/**ioctl ends**/

static struct uio_info uinfo={
  .name = "wtlfota_dpram",
  .version = "0.0.1",
};

static irqreturn_t IRQ_WTLFOTA_DPRAM_INT_N_handler(int irq, struct uio_info *dev_info){
  return IRQ_HANDLED;
}

static int uio_register(struct uio_info * info ){
  int retval;
  info->mem[0].addr = WTLFOTA_DPRAM_START_ADDRESS_PHYS + WTLFOTA_DPRAM_SHARED_BANK;
  info->mem[0].size = WTLFOTA_DPRAM_SHARED_BANK_SIZE;
  info->mem[0].memtype = UIO_MEM_PHYS;

  info->irq = IRQ_WTLFOTA_DPRAM_INT_N;
  info->irq_flags=IRQF_DISABLED;
  //info->irq = UIO_IRQ_NONE;
  info->handler = IRQ_WTLFOTA_DPRAM_INT_N_handler;
  //todo: i should use global encapsulation here. just for debugging
  retval = dpram_uio_register_device(dpramctl_dev.this_device, info);
  return retval;
}

static void uio_unregister(struct uio_info * info ){
  dpram_uio_unregister_device(info);
}


/* init & cleanup. */
static int __init wtlfota_dpram_init(void)
{
  int ret;
  ret =  wtlfota_dpram_probe();
  if(ret != 0){
    printk("wtlfota_dpram_probe fail!\n");
    return -1;
  }
  ret = misc_register(&dpramctl_dev);
  if (ret < 0) {
    printk("misc_register() failed\n");
    return -1;
  }
  
  ret = uio_register(&uinfo);
  if (ret != 0) {
    printk("uio_register() failed\n");
    return -1;
  }
  
  printk("wtlfota_dpram_init returning %d\n", ret);
  return ret;
}
static void __exit wtlfota_dpram_exit(void)
{
  uio_unregister(&uinfo);
  misc_deregister(&dpramctl_dev);
  fini_hw_setting();
  printk("wtlfota_dpram_exit returning\n");
}

module_init(wtlfota_dpram_init);
module_exit(wtlfota_dpram_exit);

MODULE_AUTHOR("SAMSUNG ELECTRONICS CO., LTD");

MODULE_DESCRIPTION("WTLFOTA_DPRAM Device Driver.");

MODULE_LICENSE("GPL");
