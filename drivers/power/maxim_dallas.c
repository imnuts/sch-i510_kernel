#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/power_supply.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/irq.h>
#include <linux/wakelock.h>
#include <asm/mach-types.h>
#include <mach/hardware.h>
#include <plat/gpio-cfg.h>

#include <linux/types.h>
#include <linux/pci.h>
#include <linux/uio.h>
#include <linux/skbuff.h>
#include <linux/atmdev.h>
#include <linux/atm_nicstar.h>


#define READ_ROM_MAXIM  0x33
#define SKIP_ROM        0xCC
#define READ_MEMORY     0xF0
#define MAXIM_ADDRESS1  0x1F
#define MAXIM_ADDRESS2  0x00

spinlock_t intlock;
static int intlock_init = 0;	// hanapark
static int delay_time = 100;	// hanapark

int CRC_protection(void);
int rom_code_protection(void);
static inline int gpio_get_value_ex(unsigned int);
static inline void gpio_set_value_ex(unsigned int, unsigned int);


/*==============================================================================

                                PORTING LAYER

==============================================================================*/

static inline int gpio_get_value_ex(unsigned int pin)
{
#ifndef __USE_EGPIO__
	return gpio_get_value(pin);
#else
	return egpio_get_value(pin);
#endif /* __USE_EGPIO__ */
}

static inline void gpio_set_value_ex(unsigned int pin, unsigned int level)
{
#ifndef __USE_EGPIO__
	gpio_set_value(pin, level);
#else
	egpio_set_value(pin, level);
#endif /* __USE_EGPIO__ */
}


#define usleep(t)  udelay(t)

void Waitx(int microseconds)	//The Waitx routine creates a delay of length microseconds
{
     usleep(microseconds);
}

void port_out_low(void)		//Routine that drives the DQ line low
{
#if 0
#ifdef FEATURE_BATT_ID_USE_MPP
    (void)pm_mpp_config_digital_output(PM_MPP_2,
                                   PM_MPP__DLOGIC__LVL_MSMP,
                                   PM_MPP__DLOGIC_OUT__CTRL_LOW);
#else
   gpio_tlmm_config(BATT_COMM_OUT);
   gpio_out(BATT_COMM_OUT,0);
#endif   

#else

//    gpio_direction_output(GPIO_BATT_ID_OUTPUT, GPIO_LEVEL_LOW);
//    s3c_gpio_setpull(GPIO_BATT_ID_OUTPUT, S3C_GPIO_PULL_NONE);
//    gpio_set_value(GPIO_BATT_ID_OUTPUT, GPIO_LEVEL_LOW);
    s3c_gpio_cfgpin(GPIO_BATT_ID, S3C_GPIO_OUTPUT);
    gpio_set_value(GPIO_BATT_ID, GPIO_LEVEL_LOW);

#endif
}

void port_out_high(void)		//Routine that drives the DQ line to 12V
{
#if 0
#ifdef FEATURE_BATT_ID_USE_MPP
    (void)pm_mpp_config_digital_output(PM_MPP_2,
                                   PM_MPP__DLOGIC__LVL_MSMP,
                                   PM_MPP__DLOGIC_OUT__CTRL_HIGH);
#else
   gpio_tlmm_config(BATT_COMM_OUT);
   gpio_out(BATT_COMM_OUT,1);
#endif   

#else
//    gpio_direction_output(GPIO_BATT_ID_OUTPUT, GPIO_LEVEL_HIGH);
//    s3c_gpio_setpull(GPIO_BATT_ID_OUTPUT, S3C_GPIO_PULL_NONE);
//    gpio_set_value(GPIO_BATT_ID_OUTPUT, GPIO_LEVEL_HIGH);
    s3c_gpio_cfgpin(GPIO_BATT_ID, S3C_GPIO_OUTPUT);
    gpio_set_value(GPIO_BATT_ID, GPIO_LEVEL_HIGH);

#endif
}

void port_input_config(void)		//Routine that disables the DQ output driver and sets the DQ pin as an input
{
#if 0
#ifdef FEATURE_BATT_ID_USE_MPP
{
    pm_err_flag_type    err = PM_ERR_FLAG__SUCCESS;
    
    err = pm_mpp_config_digital_input( PM_MPP_2, 
                                       PM_MPP__DLOGIC__LVL_MSMP,
                                       PM_MPP__DLOGIC_IN__DBUS_NONE );

    if( err != PM_ERR_FLAG__SUCCESS )
        MSG_ERROR("PMIC API ERROR(0x%x) DETECTED",err,0,0);
}
#else
   gpio_tlmm_config(BATT_COMM_IN);
#endif

#else
//    gpio_direction_input(GPIO_BATT_ID);
    s3c_gpio_cfgpin(GPIO_BATT_ID, S3C_GPIO_INPUT);
#endif

}

int port_input(void)		//Routine that samples DQ, returns 1 if DQ is high, 0 if DQ is low
{
    int input_value;

    port_input_config();

#if 0    
#ifdef FEATURE_BATT_ID_USE_MPP
{
    pm_err_flag_type    err       = PM_ERR_FLAG__SUCCESS;
    boolean             status;

    err = pm_get_rt_status( PM_MPP02_CHGED_RT_ST, &status );

    if( err != PM_ERR_FLAG__SUCCESS )
    {
        MSG_ERROR(" PMIC API ERROR(0x%x).",err,0,0);
    }
    else
        input_value = (int)status;
}
#else 
    input_value = gpio_in(BATT_COMM_IN);
#endif

#else
   input_value = gpio_get_value(GPIO_BATT_ID);
#endif

   if (input_value)
      return 1;
   else 
      return 0;
}

/*============================================================================*/


// Generate a 1-Wire reset, return 1 if no
// presence detect was found, return 0 otherwise.
int Reset(void)  
{
    int result;
    unsigned long flags;

		// hanapark BEGIN[
		if (intlock_init == 0)
		{
				spin_lock_init(&intlock);
				intlock_init = 1;
		}
		// ] hanapark END

    spin_lock_irqsave(&intlock, flags);
    
    port_out_low(); //Drives DQ low
    Waitx(500);
    port_out_high(); //Releases the bus
    Waitx(67);
    //Sample and return the Presence Detect    
    result = port_input() & 0x01;
    Waitx(420);

    spin_unlock_irqrestore(&intlock, flags);

	Waitx(delay_time); // hanapark
    return result;
}

int Reset_TA(void)  
{
    int result;
    unsigned long flags;

	// hanapark BEGIN[
	if (intlock_init == 0)
	{
	    spin_lock_init(&intlock);
	    intlock_init = 1;
	}
	// ] hanapark END

    spin_lock_irqsave(&intlock, flags);
    
    port_out_low(); //Drives DQ low
    Waitx(500);
    port_out_high(); //Releases the bus
    Waitx(67);
    //Sample and return the Presence Detect
    result = port_input() & 0x01;

    spin_unlock_irqrestore(&intlock, flags);

	Waitx(delay_time); // hanapark
    return result;
}

// Generate a Write1.
void Write1(void)
{
    unsigned long flags;

		// hanapark BEGIN[
		if (intlock_init == 0)
		{
				spin_lock_init(&intlock);
				intlock_init = 1;
		}
		// ] hanapark END

    spin_lock_irqsave(&intlock, flags);
    
    port_out_low(); //Drives DQ low
    Waitx(1);
    port_out_high(); //Releases the bus
    Waitx(59);

    spin_unlock_irqrestore(&intlock, flags);
}

//Generate a Write0. I'm giving a 5us recovery 
//time in case the rise time of your system is 
//slower than 1us. This will notaffect system 
//performance.
void Write0(void)
{
    unsigned long flags;

		// hanapark BEGIN[
		if (intlock_init == 0)
		{
				spin_lock_init(&intlock);
				intlock_init = 1;
		}
		// ] hanapark END

    spin_lock_irqsave(&intlock, flags);
    port_out_low(); //Drives DQ low
    Waitx(55);
    port_out_high(); //Releases the bus
    Waitx(5);

    spin_unlock_irqrestore(&intlock, flags);
}

// Read 1 bit from the bus and return it
int Readx(void)
{
    int result;
    unsigned long flags;

		// hanapark BEGIN[
		if (intlock_init == 0)
		{
				spin_lock_init(&intlock);
				intlock_init = 1;
		}
		// ] hanapark END

    spin_lock_irqsave(&intlock, flags);

    port_out_low(); //Drives DQ low
#if 0	// hanapark_DF01 from GLUON2
    Waitx(1);
    port_out_high(); //Releases the bus
    Waitx(15);   
    //Sample after 15us
#endif	// hanapark_DF01 from GLUON2
    result = port_input() & 0x01;
    Waitx(45);

    spin_unlock_irqrestore(&intlock, flags);
    return result;
}

// Write data byte
void maxim_WriteByte(int Data)
{
   int loop;
   //Do 1 complete byte
   for(loop=0; loop<8; loop++)
   {
	   //0x01,0x02,0x04,0x08,0x10,ect.
	   if(Data & (0x01<<loop))
		   Write1();
	   else
		   Write0();
   }
}

// Read data byte and return it
int maxim_ReadByte(void)
{
   int loop;
   int result=0;
   for(loop=0; loop<8; loop++)
   {
	   result = result + (Readx()<<loop);
   }
   return result;
}

// CRC calculator
// This function takes an N byte serial data input, runs the standard ROM CRC algorithm,
// X8 + X5 + X4 + 1, on it and generates the result. A result of 0 is expected for any valid
// data string ending with a CRC.
//The data is passed into the function LSB first using int* data of specified length int length.
//CRC returns the CRC for the entire data array.
int CRC(int* data, int length)
{
     int CRC=0;
     int byte,bit;
     for(byte=0;byte<length;byte++)
     {
         for(bit=0;bit<=7;bit++)
         {
             if((data[byte]>>bit & 0x01) == (CRC & 0x01))
             {
                CRC = CRC >> 1;
             }
             else
             {
                 CRC = CRC >> 1;
                 if(CRC & 0x04) CRC = CRC - 4;
                 else CRC = CRC + 4;
                 if(CRC & 0x08) CRC = CRC - 8;
                 else CRC = CRC + 8;
                 if(CRC & 0x80) CRC = CRC - 128;
                 else CRC = CRC + 128;
             }
         }
     }
     return CRC;
}


//ROM code protection function
int rom_code_protection()
{
	int i;
	int result[8];
	int retVal;	// hanapark

	// hanapark BEGIN[
	if (intlock_init == 0)
	{
		spin_lock_init(&intlock);
		intlock_init = 1;
	}
	// ] hanapark END

	spin_lock_init(&intlock);    

	if(!Reset())
	{
		maxim_WriteByte(READ_ROM_MAXIM);
		for (i=0; i<8; i++)
		{
			result[i]=maxim_ReadByte();
		}
#if 1    //batt customer id(0x414) checking added namil YK28
		if ((result[7]==CRC(result,7))&&((((result[5]>>4)&0x0F) == 0x04)&& (result[6] == 0x41)))
#else
		if (result[7]==CRC(result,7))
#endif 
			retVal = 1; //return 1;
		else retVal = 0; //return 0;
	}
	else 
		retVal = 0; //return 0;

	Waitx(delay_time); // hanapark
	return retVal;
}

//CRC protection function
int CRC_protection()
{
	int result;
	int protection_data[3];
	int retVal;	// hanapark

	protection_data[0]=READ_MEMORY;
	protection_data[1]=MAXIM_ADDRESS1;
	protection_data[2]=MAXIM_ADDRESS2;

	if(!Reset())
	{
		maxim_WriteByte(SKIP_ROM);
		maxim_WriteByte(READ_MEMORY);
		maxim_WriteByte(MAXIM_ADDRESS1);
		maxim_WriteByte(MAXIM_ADDRESS2);

		result=maxim_ReadByte();

		if (result==CRC(protection_data,3))
			retVal = 1; //return 1;
		else retVal = 0; //return 0;
	}
	else 
		retVal = 0; //return 0;

	Waitx(delay_time); // hanapark
	return retVal;
}

void BattAuth_Finish(void)	// hanapark_Garnett
{
	// Set BATT_ID high !
	s3c_gpio_cfgpin(GPIO_BATT_ID, GPIO_OUTPUT);
	s3c_gpio_setpull(GPIO_BATT_ID, S3C_GPIO_PULL_NONE);
	s3c_gpio_setpin(GPIO_BATT_ID, GPIO_LEVEL_HIGH);
}

