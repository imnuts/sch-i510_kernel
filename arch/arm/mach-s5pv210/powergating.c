/* linux/arch/arm/mach-s5pv210/powergating.c
 *
 * Copyright (c) 2009 Samsung Electronics Co., Ltd.
 *              http://www.samsung.com/
 *
 * Base Power gating function
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/init.h>
#include <linux/suspend.h>
#include <linux/errno.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/crc32.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/serial_core.h>
#include <linux/io.h>
#include <linux/platform_device.h>

#include <linux/regulator/max8998.h>

#include <plat/map-base.h>
#include <plat/regs-serial.h>
#include <mach/regs-clock.h>
#include <mach/regs-power.h>
#include <mach/regs-gpio.h>
#include <plat/gpio-cfg.h>
#include <mach/regs-audss.h>
#include <mach/cpu-freq-v210.h>
#include <mach/regs-mem.h>
#include <mach/regs-irq.h>
#include <asm/gpio.h>

#define DBG(fmt...)
//#define DBG(fmt...) printk(fmt)

static DEFINE_MUTEX(power_lock);

static DEFINE_SPINLOCK(power_gating_lock);

unsigned int g_power_domain_lock_token = 0;
extern int dvs_initilized;

void s5pc110_lock_power_domain(unsigned int nToken)
{
	if(nToken > I2S0_DOMAIN_LOCK_TOKEN)
	{
		printk(KERN_ERR "Lock power called with invalid parameter\n");
		return;	
	}
	mutex_lock(&power_lock);
	g_power_domain_lock_token |= nToken;
	mutex_unlock(&power_lock);
	DBG("Lock power called with token %d\n", nToken);	
}

void s5pc110_unlock_power_domain(unsigned int nToken)
{
	if(nToken > I2S0_DOMAIN_LOCK_TOKEN)
	{
		printk(KERN_ERR "Unlock power called with invalid parameter\n");
		return;	
	}
	mutex_lock(&power_lock);
	g_power_domain_lock_token &= ~nToken;
	mutex_unlock(&power_lock);
	DBG("Unlock power called with token %d\n", nToken);
	
	/*check if we can power off this domain*/
	if(nToken == MFC_DOMAIN_LOCK_TOKEN)
		s5p_power_gating(S5PC110_POWER_DOMAIN_MFC, DOMAIN_LP_MODE);
	else if(nToken == G3D_DOMAIN_LOCK_TOKEN)	
		s5p_power_gating(S5PC110_POWER_DOMAIN_G3D, DOMAIN_LP_MODE);	
	else if((nToken >= FIMD_DOMAIN_LOCK_TOKEN) && (nToken <= DSIM_DOMAIN_LOCK_TOKEN))	
		s5p_power_gating(S5PC110_POWER_DOMAIN_LCD, DOMAIN_LP_MODE);	
	else if((nToken >= VP_DOMAIN_LOCK_TOKEN) && (nToken <= HDMI_DOMAIN_LOCK_TOKEN))	
		s5p_power_gating(S5PC110_POWER_DOMAIN_TV, DOMAIN_LP_MODE);	
	else if((nToken >= FIMC0_DOMAIN_LOCK_TOKEN) && (nToken <= CSIS_DOMAIN_LOCK_TOKEN))	
		s5p_power_gating(S5PC110_POWER_DOMAIN_CAM, DOMAIN_LP_MODE);	
	else if(nToken == I2S0_DOMAIN_LOCK_TOKEN)	
		s5p_power_gating(S5PC110_POWER_DOMAIN_AUDIO, DOMAIN_LP_MODE);
}

/*
 * s5p_domain_off_check()
 *
 * check if the power domain is off or not
 * 
*/
int s5p_domain_off_check(unsigned int power_domain)
{
	unsigned int poweroff = 0;

    switch( power_domain )
    {
	case S5PC110_POWER_DOMAIN_MFC: //MFC off
		if(!(readl(S5P_CLKGATE_BLOCK) & S5P_CLKGATE_BLOCK_MFC) && 
			!(readl(S5P_CLKGATE_IP0) & POWER_DOMAIN_MFC_CLOCK_SET) )
	        {	
			if(!(g_power_domain_lock_token & MFC_DOMAIN_LOCK_TOKEN))
				poweroff = 1;
		}
		break;

	case S5PC110_POWER_DOMAIN_G3D: //G3D off
		if(!(readl(S5P_CLKGATE_BLOCK) & S5P_CLKGATE_BLOCK_G3D) &&
			!(readl(S5P_CLKGATE_IP0) & POWER_DOMAIN_G3D_CLOCK_SET) )
	        {
			if(!(g_power_domain_lock_token & G3D_DOMAIN_LOCK_TOKEN))
				poweroff = 1;

		}
		break;

	case S5PC110_POWER_DOMAIN_LCD: //LCD off
		if(!(readl(S5P_CLKGATE_BLOCK) & S5P_CLKGATE_BLOCK_LCD) && 
			!(readl(S5P_CLKGATE_IP1) & (POWER_DOMAIN_LCD_CLOCK_SET)) )
	        {
			if(!(g_power_domain_lock_token & (LCD_DOMAIN_LOCK_TOKEN_SET)))
				poweroff = 1;
		}
		break;

	case S5PC110_POWER_DOMAIN_TV: //TV off
		if(!(readl(S5P_CLKGATE_BLOCK) & S5P_CLKGATE_BLOCK_TV) &&
			!(readl(S5P_CLKGATE_IP1) & (POWER_DOMAIN_TV_CLOCK_SET)) )
	        {
			if(!(g_power_domain_lock_token & (TV_DOMAIN_LOCK_TOKEN_SET)))
				poweroff = 1;
		}
		break;


	case S5PC110_POWER_DOMAIN_CAM: //CAM off
		if(!(readl(S5P_CLKGATE_BLOCK) & S5P_CLKGATE_BLOCK_IMG) &&
			!(readl(S5P_CLKGATE_IP0) & (POWER_DOMAIN_CAMERA_CLOCK_SET)) )
	        {
			if(!(g_power_domain_lock_token & (CAMERA_DOMAIN_LOCK_TOKEN_SET)))
				poweroff = 1;
		}
		break;

	case S5PC110_POWER_DOMAIN_AUDIO: //AUDIO off
#if 1 // for audio pwr gating is not support on chip -> enable for evt1 only
		if(!(readl(S5P_CLKGATE_IP3) &((S5P_CLKGATE_IP3_I2S0))) )
	        {
			if(!(g_power_domain_lock_token & I2S0_DOMAIN_LOCK_TOKEN))
		
				poweroff = 1;
		}
#endif	
		break;
	
	default :
		printk( "[SYSCON][Err] S5PC110_Power_Gating - power_domain: %d \n", power_domain );
		break;
    	}

	return poweroff;
}


/*
 * s5p_pmic_gating()
 *
 * To do turn on/off LDOs of pmic for power gating 
 * 
*/
int s5p_pmic_gating(unsigned int power_domain, unsigned int on_off)
{
    switch( power_domain )
    {
	case S5PC110_POWER_DOMAIN_MFC: //MFC off
		if (on_off) {
			// power on
		} else {
			// power off		
		}
		break;

	case S5PC110_POWER_DOMAIN_G3D: //G3D off
		if (on_off) {
			// power on
		} else {
			// power off		
		}
		break;

	case S5PC110_POWER_DOMAIN_LCD: //LCD: ldo7, 17
		if (on_off) {
			// power on
			max8998_ldo_enable_direct(MAX8998_LDO7);
			max8998_ldo_enable_direct(MAX8998_LDO17);
		} else {
			// power off
			max8998_ldo_disable_direct(MAX8998_LDO7);	
			max8998_ldo_set_voltage_direct(MAX8998_LDO7,1800000, 1800000);
			max8998_ldo_disable_direct(MAX8998_LDO17);
			max8998_ldo_set_voltage_direct(MAX8998_LDO17,3000000,3000000);
		}
		break;

	case S5PC110_POWER_DOMAIN_TV: //TV off
		if (on_off) {
			// power on
			//max8998_ldo_enable_direct(MAX8998_LDO8);
		} else {
			// power off		
			//max8998_ldo_disable_direct(MAX8998_LDO8);
		}
		break;

	case S5PC110_POWER_DOMAIN_CAM: //CAM: ldo 11,12,13,14,15, 16
		if (on_off) {
			// power on
			/*max8998_ldo_enable_direct(MAX8998_LDO11);
			max8998_ldo_enable_direct(MAX8998_LDO12);
			max8998_ldo_enable_direct(MAX8998_LDO13);
			max8998_ldo_enable_direct(MAX8998_LDO14);
			max8998_ldo_enable_direct(MAX8998_LDO15);
			max8998_ldo_enable_direct(MAX8998_LDO16);*/																		
		} else {
			// power off
			/*max8998_ldo_disable_direct(MAX8998_LDO7);
			max8998_ldo_disable_direct(MAX8998_LDO12);
			max8998_ldo_disable_direct(MAX8998_LDO13);
			max8998_ldo_disable_direct(MAX8998_LDO14);
			max8998_ldo_disable_direct(MAX8998_LDO15);
			max8998_ldo_disable_direct(MAX8998_LDO16);*/																	
		}
		break;

	case S5PC110_POWER_DOMAIN_AUDIO: //AUDIO off
#if 0 // for audio pwr gating is not supported on chip
		if (on_off) {
			// power on
		} else {
			// power off		
		}
#endif
		break;
		
	default :
		printk( "[SYSCON][Err] S5PC110_PMIC_Gating - power_domain: %d durning %s\n", power_domain, on_off ? "Turn on" : "Turn off" );
		break;
    	}
    	
    	return 1;
}


/*
 * s5p_power_gating()
 *
 * To do power gating
 * 
*/
extern int tvblk_turnon;
int s5p_power_gating(unsigned int power_domain, unsigned int on_off)
{
	unsigned int tmp;
	int retvalue = 0;
	u32 con;
	unsigned long flags;

	if (power_domain > S5PC110_POWER_DOMAIN_UNCANGIBLE_MASK) return retvalue;

	spin_lock_irqsave(&power_gating_lock, flags);

	//mutex_lock(&power_lock);
	if(on_off == DOMAIN_ACTIVE_MODE) {
	
		if(s5p_domain_off_check(power_domain)){
			tmp = readl(S5P_NORMAL_CFG);
			if(!(tmp & power_domain)) // enable only once
			{
			
				if (power_domain == S5PC110_POWER_DOMAIN_TV) {
					con = __raw_readl(S5P_CLKGATE_IP1);
					con |= 0x0f00; // enable VP, Mixer, TVEnc, HDMI
					__raw_writel(con, S5P_CLKGATE_IP1);
					tvblk_turnon=1;
					/*Enable Block gating also*/
					con = __raw_readl(S5P_CLKGATE_BLOCK);
					con |= S5P_CLKGATE_BLOCK_TV; 
					__raw_writel(con, S5P_CLKGATE_BLOCK);
				}	
						
				/*Check if we have to enable the ldo's*/
				if(dvs_initilized)
					s5p_pmic_gating(power_domain, 1);
				
				tmp = tmp | (power_domain);
				writel(tmp , S5P_NORMAL_CFG);
				while(!(readl(S5P_BLK_PWR_STAT) & (power_domain)));
				DBG("Requested domain-active mode:  %x \n",power_domain);
				
				if (tvblk_turnon) {
					con = __raw_readl(S5P_CLKGATE_IP1);
					con =  (con & ~(0x0f00)); // disable others in VP, Mixer, TVEnc, HDMI
					__raw_writel(con, S5P_CLKGATE_IP1);		
					tvblk_turnon = 0;
					/*Disable Block gating also*/
					con = __raw_readl(S5P_CLKGATE_BLOCK);
					con &= (~S5P_CLKGATE_BLOCK_TV); 
					__raw_writel(con, S5P_CLKGATE_BLOCK);
				}				
								
			}		
			retvalue = 1;			
		}		
	}
	else if(on_off == DOMAIN_LP_MODE) {
		
		if(s5p_domain_off_check(power_domain)){

			 tmp = readl(S5P_NORMAL_CFG);
			if((tmp & power_domain)) // disable only once
			{
				tmp = tmp & ~(power_domain);
				writel(tmp , S5P_NORMAL_CFG);
				while((readl(S5P_BLK_PWR_STAT) & (power_domain)));
				DBG("Requested domain-LP mode:  %x \n",power_domain);
				/*Check if we have to disable the ldo's*/
				if(dvs_initilized)
					s5p_pmic_gating(power_domain, 0);
			}
			retvalue = 1;
		}
	}	
	//mutex_unlock(&power_lock);

	spin_unlock_irqrestore(&power_gating_lock, flags);

	return retvalue;
}
EXPORT_SYMBOL(s5p_power_gating);


#if 0 // commenting function s5p_init_domain_power
/*
 * s5p_init_domain_power()
 *
 * Initailize power domain at booting 
 * 
*/
static void s5p_init_domain_power(void)
{
#if 0
	s5p_power_gating(S5PC110_POWER_DOMAIN_TV,  DOMAIN_LP_MODE);
	s5p_power_gating(S5PC110_POWER_DOMAIN_MFC, DOMAIN_LP_MODE);
	s5p_power_gating(S5PC110_POWER_DOMAIN_AUDIO, DOMAIN_LP_MODE);
	s5p_power_gating(S5PC110_POWER_DOMAIN_G3D, DOMAIN_LP_MODE);
#endif
}
#endif // commenting function s5p_init_domain_power
