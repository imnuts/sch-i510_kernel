/*
 * s6e63m0 AMOLED Panel Driver for the Samsung Universal board
 *
 * Derived from drivers/video/omap/lcd-apollon.c
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <linux/wait.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/spi/spi.h>
#include <linux/lcd.h>
#include <linux/backlight.h>

#include <plat/gpio-cfg.h>

#include "s3cfb.h"
#include "s6e63m0.h"

#define DIM_BL	20
#define MIN_BL	30
#define MAX_BL	255

#define MAX_GAMMA_VALUE	24	// we have 25 levels. -> 16 levels -> 24 levels
#define ELVSS_GAMMA_VALUE1 4   //100CD
#define ELVSS_GAMMA_VALUE2 10  //160CD
#define ELVSS_GAMMA_VALUE3 14  //200CD
#define CRITICAL_BATTERY_LEVEL 5

#define GAMMASET_CONTROL //for 1.9/2.2 gamma control from platform


/*********** for debug **********************************************************/
#if 0 
#define gprintk(fmt, x... ) printk( "%s(%d): " fmt, __FUNCTION__ ,__LINE__, ## x)
#else
#define gprintk(x...) do { } while (0)
#endif
/*******************************************************************************/
atomic_t lcd_spi_read_flag = ATOMIC_INIT(0);
EXPORT_SYMBOL(lcd_spi_read_flag);
atomic_t lcd_panel_Id= ATOMIC_INIT(0);
EXPORT_SYMBOL(lcd_panel_Id);

static unsigned int lcd_type;
module_param_named(lcd_type, lcd_type, uint, 0444);
MODULE_PARM_DESC(lcd_type, "LCD Panel Type: default= M2 (HWREV < 8), 1= M2 (HWREV >= 8), 2= SM2");

//static lcd_panel_type lcd_panel;

static int ldi_enable = 0;

typedef enum {
	BACKLIGHT_LEVEL_OFF		= 0,
	BACKLIGHT_LEVEL_DIMMING	= 1,
	BACKLIGHT_LEVEL_NORMAL	= 6
} backlight_level_t;

backlight_level_t backlight_level = BACKLIGHT_LEVEL_OFF;
static int bd_brightness = 0;

int current_gamma_value = -1;
static int on_19gamma = 0;

static DEFINE_MUTEX(spi_use);

struct s5p_lcd {
	struct spi_device *g_spi;
	struct lcd_device *lcd_dev;
	struct backlight_device *bl_dev;
};

#ifdef GAMMASET_CONTROL
struct class *gammaset_class;
struct device *switch_gammaset_dev;
#endif

#ifdef CONFIG_FB_S3C_TL2796_ACL
static int acl_enable = 0;
static int cur_acl = 0;

struct class *acl_class;
struct device *switch_aclset_dev;
#endif

#ifdef CONFIG_FB_S3C_MDNIE
extern void init_mdnie_class(void);
#endif

static struct s5p_lcd lcd;

extern unsigned int HWREV;

#if !defined(CONFIG_ARIES_NTT)
static const unsigned short *p22Gamma_set[] = {        
    SEQ_GAMMA_50_SETTING, //0
    SEQ_GAMMA_70_SETTING, 
    SEQ_GAMMA_80_SETTING,
    SEQ_GAMMA_90_SETTING,
    SEQ_GAMMA_100_SETTING,
    SEQ_GAMMA_110_SETTING,
    SEQ_GAMMA_120_SETTING,
    SEQ_GAMMA_130_SETTING,
    SEQ_GAMMA_140_SETTING,
    SEQ_GAMMA_150_SETTING,
    SEQ_GAMMA_160_SETTING, //10
    SEQ_GAMMA_170_SETTING,
    SEQ_GAMMA_180_SETTING,    
    SEQ_GAMMA_190_SETTING,    
    SEQ_GAMMA_200_SETTING,    
    SEQ_GAMMA_210_SETTING,    
    SEQ_GAMMA_220_SETTING,    
    SEQ_GAMMA_230_SETTING,    
    SEQ_GAMMA_240_SETTING,    
    SEQ_GAMMA_250_SETTING,    
    SEQ_GAMMA_260_SETTING, //20 
    SEQ_GAMMA_270_SETTING,    
    SEQ_GAMMA_280_SETTING,    
    SEQ_GAMMA_290_SETTING,    
    SEQ_GAMMA_300_SETTING, //24            
};                                             
                                               
static const unsigned short *p19Gamma_set[] = {        
	s6e63m0_19gamma_30cd,	//0
	//s6e63m0_19gamma_50cd,                         
	s6e63m0_19gamma_40cd,     
	s6e63m0_19gamma_70cd,

	s6e63m0_19gamma_90cd,
	s6e63m0_19gamma_100cd,
	s6e63m0_19gamma_110cd,	//5
	s6e63m0_19gamma_120cd,
	s6e63m0_19gamma_130cd,
	s6e63m0_19gamma_140cd,
	s6e63m0_19gamma_150cd,
	s6e63m0_19gamma_160cd,	//10
	s6e63m0_19gamma_170cd,
	s6e63m0_19gamma_180cd,
	s6e63m0_19gamma_190cd,
	s6e63m0_19gamma_200cd,
	s6e63m0_19gamma_210cd,	//15
	s6e63m0_19gamma_220cd,
	s6e63m0_19gamma_230cd,
	s6e63m0_19gamma_240cd,
	s6e63m0_19gamma_250cd,
	s6e63m0_19gamma_260cd,	//20
	s6e63m0_19gamma_270cd,
	s6e63m0_19gamma_280cd,
	s6e63m0_19gamma_290cd,
	s6e63m0_19gamma_300cd,	//24
}; 

static const unsigned short *p22Gamma_set_sm2[] = {
    SEQ_GAMMA_30_SETTING_SM2, //0
    SEQ_GAMMA_40_SETTING_SM2,
    SEQ_GAMMA_70_SETTING_SM2,
    SEQ_GAMMA_90_SETTING_SM2,
    SEQ_GAMMA_100_SETTING_SM2,
    SEQ_GAMMA_110_SETTING_SM2,
    SEQ_GAMMA_120_SETTING_SM2,
    SEQ_GAMMA_130_SETTING_SM2,
    SEQ_GAMMA_140_SETTING_SM2,
    SEQ_GAMMA_150_SETTING_SM2,
    SEQ_GAMMA_160_SETTING_SM2, //10
    SEQ_GAMMA_170_SETTING_SM2,
    SEQ_GAMMA_180_SETTING_SM2,
    SEQ_GAMMA_190_SETTING_SM2,
    SEQ_GAMMA_200_SETTING_SM2,
    SEQ_GAMMA_210_SETTING_SM2,
    SEQ_GAMMA_220_SETTING_SM2,
    SEQ_GAMMA_230_SETTING_SM2,
    SEQ_GAMMA_240_SETTING_SM2,
    SEQ_GAMMA_250_SETTING_SM2,
    SEQ_GAMMA_260_SETTING_SM2, //20
    SEQ_GAMMA_270_SETTING_SM2,
    SEQ_GAMMA_280_SETTING_SM2,
    SEQ_GAMMA_290_SETTING_SM2,
    SEQ_GAMMA_300_SETTING_SM2, //24
};

static const unsigned short *p22Gamma_set_hwrev_8[] = {        
    SEQ_GAMMA_30_SETTING_HWREV_8, //0
    SEQ_GAMMA_40_SETTING_HWREV_8, 
    SEQ_GAMMA_70_SETTING_HWREV_8,
    SEQ_GAMMA_90_SETTING_HWREV_8,
    SEQ_GAMMA_100_SETTING_HWREV_8,
    SEQ_GAMMA_110_SETTING_HWREV_8,
    SEQ_GAMMA_120_SETTING_HWREV_8,
    SEQ_GAMMA_130_SETTING_HWREV_8,
    SEQ_GAMMA_140_SETTING_HWREV_8,
    SEQ_GAMMA_150_SETTING_HWREV_8,
    SEQ_GAMMA_160_SETTING_HWREV_8, //10
    SEQ_GAMMA_170_SETTING_HWREV_8,
    SEQ_GAMMA_180_SETTING_HWREV_8,    
    SEQ_GAMMA_190_SETTING_HWREV_8,    
    SEQ_GAMMA_200_SETTING_HWREV_8,    
    SEQ_GAMMA_210_SETTING_HWREV_8,    
    SEQ_GAMMA_220_SETTING_HWREV_8,    
    SEQ_GAMMA_230_SETTING_HWREV_8,    
    SEQ_GAMMA_240_SETTING_HWREV_8,    
    SEQ_GAMMA_250_SETTING_HWREV_8,    
    SEQ_GAMMA_260_SETTING_HWREV_8, //20 
    SEQ_GAMMA_270_SETTING_HWREV_8,    
    SEQ_GAMMA_280_SETTING_HWREV_8,    
    SEQ_GAMMA_290_SETTING_HWREV_8,    
    SEQ_GAMMA_300_SETTING_HWREV_8, //24            
};                                             


static const unsigned short *p19Gamma_set_sm2[] = {        
	s6e63m0_19gamma_30cd_sm2,	//0
	s6e63m0_19gamma_40cd_sm2,     
	s6e63m0_19gamma_70cd_sm2,
	s6e63m0_19gamma_90cd_sm2,
	s6e63m0_19gamma_100cd_sm2,
	s6e63m0_19gamma_110cd_sm2,	//5
	s6e63m0_19gamma_120cd_sm2,
	s6e63m0_19gamma_130cd_sm2,
	s6e63m0_19gamma_140cd_sm2,
	s6e63m0_19gamma_150cd_sm2,
	s6e63m0_19gamma_160cd_sm2,	//10
	s6e63m0_19gamma_170cd_sm2,
	s6e63m0_19gamma_180cd_sm2,
	s6e63m0_19gamma_190cd_sm2,
	s6e63m0_19gamma_200cd_sm2,
	s6e63m0_19gamma_210cd_sm2,	//15
	s6e63m0_19gamma_220cd_sm2,
	s6e63m0_19gamma_230cd_sm2,
	s6e63m0_19gamma_240cd_sm2,
	s6e63m0_19gamma_250cd_sm2,
	s6e63m0_19gamma_260cd_sm2,	//20
	s6e63m0_19gamma_270cd_sm2,
	s6e63m0_19gamma_280cd_sm2,
	s6e63m0_19gamma_290cd_sm2,
	s6e63m0_19gamma_300cd_sm2,	//24
}; 


static const unsigned short *p19Gamma_set_hwrev_8[] = {        
	s6e63m0_19gamma_30cd_hwrev_8,	//0
	s6e63m0_19gamma_40cd_hwrev_8,     
	s6e63m0_19gamma_70cd_hwrev_8,
	s6e63m0_19gamma_90cd_hwrev_8,
	s6e63m0_19gamma_100cd_hwrev_8,
	s6e63m0_19gamma_110cd_hwrev_8,	//5
	s6e63m0_19gamma_120cd_hwrev_8,
	s6e63m0_19gamma_130cd_hwrev_8,
	s6e63m0_19gamma_140cd_hwrev_8,
	s6e63m0_19gamma_150cd_hwrev_8,
	s6e63m0_19gamma_160cd_hwrev_8,	//10
	s6e63m0_19gamma_170cd_hwrev_8,
	s6e63m0_19gamma_180cd_hwrev_8,
	s6e63m0_19gamma_190cd_hwrev_8,
	s6e63m0_19gamma_200cd_hwrev_8,
	s6e63m0_19gamma_210cd_hwrev_8,	//15
	s6e63m0_19gamma_220cd_hwrev_8,
	s6e63m0_19gamma_230cd_hwrev_8,
	s6e63m0_19gamma_240cd_hwrev_8,
	s6e63m0_19gamma_250cd_hwrev_8,
	s6e63m0_19gamma_260cd_hwrev_8,	//20
	s6e63m0_19gamma_270cd_hwrev_8,
	s6e63m0_19gamma_280cd_hwrev_8,
	s6e63m0_19gamma_290cd_hwrev_8,
	s6e63m0_19gamma_300cd_hwrev_8,	//24
}; 

static const unsigned short *dynamic_elvss_sm2[] = {
    SEQ_ELVSS_SM2_LEVEL1,   //30CD to 100CD
    SEQ_ELVSS_SM2_LEVEL2,   //110CD to 160CD
    SEQ_ELVSS_SM2_LEVEL3,   //170CD to 200CD
    SEQ_ELVSS_SM2_LEVEL4,   //210CD to 300CD
};

static const unsigned short *dynamic_elvss[] = {
	SEQ_ELVSS_LEVEL1,	//30CD to 100CD
	SEQ_ELVSS_LEVEL2,	//110CD to 160CD
	SEQ_ELVSS_LEVEL3,	//170CD to 200CD
	SEQ_ELVSS_LEVEL4,	//210CD to 300CD
};

#else // Modify NTTS1
static const unsigned short *p22Gamma_set[] = {        
	s6e63m0_22gamma_30cd,	 //0                    
	s6e63m0_22gamma_40cd,  
	s6e63m0_22gamma_50cd,
	s6e63m0_22gamma_60cd,
	s6e63m0_22gamma_70cd,	
	s6e63m0_22gamma_80cd,	//5
	s6e63m0_22gamma_90cd,
	s6e63m0_22gamma_100cd,
	s6e63m0_22gamma_110cd,	
	s6e63m0_22gamma_120cd,	
	s6e63m0_22gamma_130cd,	//10
	s6e63m0_22gamma_140cd,
	s6e63m0_22gamma_150cd,
	s6e63m0_22gamma_160cd,	
	s6e63m0_22gamma_170cd,	
	s6e63m0_22gamma_180cd,	//15
	s6e63m0_22gamma_190cd,
	s6e63m0_22gamma_200cd,
	s6e63m0_22gamma_210cd,	
	s6e63m0_22gamma_220cd,	
	s6e63m0_22gamma_230cd,	//20
	s6e63m0_22gamma_240cd,
	s6e63m0_22gamma_260cd,
	s6e63m0_22gamma_280cd,
	s6e63m0_22gamma_300cd, 	//24                   
};                                             
                                               
                                                
static const unsigned short *p19Gamma_set[] = {     
	s6e63m0_19gamma_30cd,	//0                     
	s6e63m0_19gamma_40cd,  
	s6e63m0_19gamma_50cd,
	s6e63m0_19gamma_60cd,
	s6e63m0_19gamma_70cd,	
	s6e63m0_19gamma_80cd,	//5
	s6e63m0_19gamma_90cd,
	s6e63m0_19gamma_100cd,
	s6e63m0_19gamma_110cd,	
	s6e63m0_19gamma_120cd,	
	s6e63m0_19gamma_130cd,	//10
	s6e63m0_19gamma_140cd,
	s6e63m0_19gamma_150cd,
	s6e63m0_19gamma_160cd,	
	s6e63m0_19gamma_170cd,	
	s6e63m0_19gamma_180cd,	//15
	s6e63m0_19gamma_190cd,
	s6e63m0_19gamma_200cd,
	s6e63m0_19gamma_210cd,	
	s6e63m0_19gamma_220cd,	
	s6e63m0_19gamma_230cd,	//20
	s6e63m0_19gamma_240cd,
	s6e63m0_19gamma_260cd,
	s6e63m0_19gamma_280cd,
	s6e63m0_19gamma_300cd, 	//24
}; 
#endif

#ifdef CONFIG_FB_S3C_TL2796_ACL
static const unsigned short *ACL_cutoff_set[] = {
	acl_cutoff_off, //0
	acl_cutoff_8p,
	acl_cutoff_14p,
	acl_cutoff_20p,
	acl_cutoff_24p,
	acl_cutoff_28p, //5
	acl_cutoff_32p,
	acl_cutoff_35p,
	acl_cutoff_37p,
	acl_cutoff_40p, //9
	acl_cutoff_45p, //10
	acl_cutoff_47p, //11
	acl_cutoff_48p, //12
	acl_cutoff_50p, //13
	acl_cutoff_60p, //14
	acl_cutoff_75p, //15
	acl_cutoff_43p, //16
};
#endif


static struct s3cfb_lcd s6e63m0 = {
	.width = 480,
	.height = 800,
	.p_width = 56,
	.p_height = 93,
	.bpp = 24,
	.freq = 60,
	
	.timing = {
		.h_fp = 16,
		.h_bp = 14,
		.h_sw = 2,
		.v_fp = 10,
		.v_fpe = 1,
		.v_bp = 4,
		.v_bpe = 1,
		.v_sw = 2,
	},

	.polarity = {
		.rise_vclk = 1,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 1,
	},
};

static void wait_ldi_enable(void);
static void update_brightness(int gamma);

static int s6e63m0_spi_write_driver(int reg)
{
	u16 buf[1];
	int ret;
	struct spi_message msg;

	struct spi_transfer xfer = {
		.len	= 2,
		.tx_buf	= buf,
	};

	buf[0] = reg;

	spi_message_init(&msg);
	spi_message_add_tail(&xfer, &msg);


	ret = spi_sync(lcd.g_spi, &msg);

	if (ret < 0)
		pr_err("%s::%d -> spi_sync failed Err=%d\n",__func__,__LINE__,ret);
	return ret ;

}
#if 0
static unsigned char get_display_id(int reg)
{
    unsigned char id;

    s6e63m0_spi_write_driver(reg);
    id = (u8)(atomic_read(&lcd_panel_Id) & 0xff);
    gprintk("*** LCD panel id = 0x%x\n", id);
    return id;
}
#endif
static void s6e63m0_spi_write(unsigned short reg)
{
  	s6e63m0_spi_write_driver(reg);	
}

static void s6e63m0_panel_send_sequence(const unsigned short *wbuf)
{
	int i = 0;

	mutex_lock(&spi_use);

	gprintk("#################SPI start##########################\n");
	
	while ((wbuf[i] & DEFMASK) != ENDDEF) {
		if ((wbuf[i] & DEFMASK) != SLEEPMSEC){
			s6e63m0_spi_write(wbuf[i]);
			i+=1;}
		else{
			msleep(wbuf[i+1]);
			i+=2;}
	}
	
	gprintk("#################SPI end##########################\n");

	mutex_unlock(&spi_use);
}

int IsLDIEnabled(void)
{
	return ldi_enable;
}
EXPORT_SYMBOL(IsLDIEnabled);

static void SetLDIEnabledFlag(int OnOff)
{
	ldi_enable = OnOff;
}
#if 0
static void get_lcd_revision(void)
{
    unsigned char lcd_id_arr[3];

    atomic_set(&lcd_spi_read_flag,1);
    lcd_id_arr[0] = get_display_id(0x0DA);
    lcd_id_arr[1] = get_display_id(0x0DB);
    lcd_id_arr[2] = get_display_id(0x0DC);
    atomic_set(&lcd_spi_read_flag,0);

    if((lcd_id_arr[0] == M2_LCD_ID1) && (lcd_id_arr[1] == M2_LCD_ID2) && (lcd_id_arr[2] == M2_LCD_ID3))
    {
        lcd_panel = LCD_PANEL_M2;
        gprintk("*** LCD panel type is M2\n");
    }
    else if((lcd_id_arr[0] == SM2_LCD_ID1) && (lcd_id_arr[1] == SM2_LCD_ID2) && (lcd_id_arr[2] == SM2_LCD_ID3))
    {
        lcd_panel = LCD_PANEL_SM2;
        gprintk("*** LCD panel type is SM2\n");
    }
}
#endif
void tl2796_ldi_init(void)
{
    s6e63m0_panel_send_sequence(SEQ_ETC_CONDITION_SET);
    if(HWREV >= 8)
    {
        s6e63m0_panel_send_sequence(SEQ_DISPLAY_CONDITION_SET_HWREV_8);
        //s6e63m0_panel_send_sequence(SEQ_GTCON_CONTROL_SET);
        s6e63m0_panel_send_sequence(SEQ_PANEL_CONDITION_SET_HWREV_8);
    }
    else
    {
        s6e63m0_panel_send_sequence(SEQ_DISPLAY_CONDITION_SET);
        s6e63m0_panel_send_sequence(SEQ_PANEL_CONDITION_SET);
    }

    s6e63m0_panel_send_sequence(s6e63m0_SEQ_STANDBY_OFF);
    msleep(160);

//    get_lcd_revision();

    if(lcd_type == LCD_PANEL_SM2)
    {
        s6e63m0_panel_send_sequence(SEQ_ELVSS_SM2_CONTROL);
        s6e63m0_panel_send_sequence(SEQ_ELVSS_SM2_LEVEL4);
        s6e63m0_panel_send_sequence(SEQ_POWER_CONTROL_SET_SM2);
        s6e63m0_panel_send_sequence(SEQ_GAMMA_210_SETTING_SM2);
    }
    else if((lcd_type == LCD_PANEL_M2) || (HWREV >= 8))
    {
        s6e63m0_panel_send_sequence(SEQ_ELVSS_SM2_CONTROL);
        s6e63m0_panel_send_sequence(SEQ_ELVSS_LEVEL4);
        s6e63m0_panel_send_sequence(SEQ_POWER_CONTROL_SET_HWREV_8);
        s6e63m0_panel_send_sequence(SEQ_GAMMA_210_SETTING_HWREV_8);
    }
    else
    {
        s6e63m0_panel_send_sequence(SEQ_ELVSS_SETTING);
        s6e63m0_panel_send_sequence(SEQ_GAMMA_210_SETTING);
    }

	SetLDIEnabledFlag(1);
	printk(KERN_DEBUG "LDI enable ok\n");
	pr_info("%s::%d -> ldi initialized\n",__func__,__LINE__);	
}

void tl2796_ldi_enable(void)
{
	s6e63m0_panel_send_sequence(s6e63m0_SEQ_DISPLAY_ON);
}

void tl2796_ldi_disable(void)
{
	s6e63m0_panel_send_sequence(s6e63m0_SEQ_DISPLAY_OFF);

    s6e63m0_panel_send_sequence(s6e63m0_SEQ_STANDBY_ON);

	SetLDIEnabledFlag(0);
	printk(KERN_DEBUG "LDI disable ok\n");
	pr_info("%s::%d -> ldi disabled\n",__func__,__LINE__);	
}

// Display ON test command
void ld9040_sleep_off(void)
{
    printk(KERN_DEBUG "start %s\n", __func__); 

    tl2796_ldi_init();

    tl2796_ldi_enable();
 
    msleep(1);
}

// Display OFF test command
void ld9040_sleep_in(void)
{
    printk(KERN_DEBUG "start %s\n", __func__); 

    tl2796_ldi_disable();

    msleep(1);
}

void s3cfb_set_lcd_info(struct s3cfb_global *ctrl)
{
	s6e63m0.init_ldi = NULL;
	ctrl->lcd = &s6e63m0;
}

//mkh:lcd operations and functions
static int s5p_lcd_set_power(struct lcd_device *ld, int power)
{
	printk(KERN_DEBUG "s5p_lcd_set_power is called: %d", power);

	if (power)	{
		s6e63m0_panel_send_sequence(s6e63m0_SEQ_DISPLAY_ON);
	}
	else	{
		s6e63m0_panel_send_sequence(s6e63m0_SEQ_DISPLAY_OFF);
	}

	return 0;
}

static struct lcd_ops s5p_lcd_ops = {
	.set_power = s5p_lcd_set_power,
};

//mkh:backlight operations and functions

void bl_update_status_gamma(void)
{
	wait_ldi_enable();

	if (current_gamma_value != -1)	{
		gprintk("#################%dgamma start##########################\n", on_19gamma ? 19 : 22);
		update_brightness(current_gamma_value);
		gprintk("#################%dgamma end##########################\n", on_19gamma ? 19 : 22);
	}
}

#ifdef GAMMASET_CONTROL //for 1.9/2.2 gamma control from platform
static ssize_t gammaset_file_cmd_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
	gprintk("called %s \n",__func__);

	return sprintf(buf,"%u\n", bd_brightness);
}
static ssize_t gammaset_file_cmd_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t size)
{
	int value;
	
    sscanf(buf, "%d", &value);

	//printk(KERN_INFO "[gamma set] in gammaset_file_cmd_store, input value = %d \n",value);
	if (!IsLDIEnabled())	{
		printk(KERN_DEBUG "[gamma set] return because LDI is disabled, input value = %d \n", value);
		return size;
	}

	if ((value != 0) && (value != 1))	{
		printk(KERN_DEBUG "\ngammaset_file_cmd_store value(%d) on_19gamma(%d) \n", value,on_19gamma);
		return size;
	}

	if (value != on_19gamma)	{
		on_19gamma = value;
		bl_update_status_gamma();
	}

	return size;
}

static DEVICE_ATTR(gammaset_file_cmd,0666, gammaset_file_cmd_show, gammaset_file_cmd_store);
#endif

#ifdef CONFIG_FB_S3C_TL2796_ACL 
static ssize_t aclset_file_cmd_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	gprintk("called %s \n",__func__);

	return sprintf(buf,"%u\n", acl_enable);
}
static ssize_t aclset_file_cmd_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int value;

	sscanf(buf, "%d", &value);

	printk("[acl set] in aclset_file_cmd_store, input value = %d \n", value);

	if (!IsLDIEnabled())	{
		printk(KERN_DEBUG "[acl set] return because LDI is disabled, input value = %d \n", value);
		return size;
	}
#if 0
	if ((value != 0) && (value != 1))	{
		printk(KERN_DEBUG "\naclset_file_cmd_store value is same : value(%d)\n", value);
		return size;
	}

	if (acl_enable != value)	{
		acl_enable = value;

		if (acl_enable == 1)	{
			s6e63m0_panel_send_sequence(acl_cutoff_init);
			msleep(20);

			switch (current_gamma_value)	{
				case 1:
					s6e63m0_panel_send_sequence(ACL_cutoff_set[0]); //set 0% ACL
					cur_acl = 0;
					//printk(" ACL_cutoff_set Percentage : 0!!\n");
					break;
				case 2:
					s6e63m0_panel_send_sequence(ACL_cutoff_set[1]); //set 12% ACL
					cur_acl = 12;
					//printk(" ACL_cutoff_set Percentage : 12!!\n");
					break;
				case 3:
					s6e63m0_panel_send_sequence(ACL_cutoff_set[2]); //set 22% ACL
					cur_acl = 22;
					//printk(" ACL_cutoff_set Percentage : 22!!\n");
					break;
				case 4:
					s6e63m0_panel_send_sequence(ACL_cutoff_set[3]); //set 30% ACL
					cur_acl = 30;
					//printk(" ACL_cutoff_set Percentage : 30!!\n");
					break;
				case 5:
					s6e63m0_panel_send_sequence(ACL_cutoff_set[4]); //set 35% ACL
					cur_acl = 35;
					//printk(" ACL_cutoff_set Percentage : 35!!\n");
					break;
				default:
					s6e63m0_panel_send_sequence(ACL_cutoff_set[5]); //set 40% ACL
					cur_acl = 40;
					//printk(" ACL_cutoff_set Percentage : 40!!\n");
			}
		}
#else
	if ((value != 0) && (value != 1))	{
		printk(KERN_DEBUG "\naclset_file_cmd_store value is same : value(%d)\n", value);
		return size;
	}

	if (acl_enable != value)	{
		acl_enable = value;

		if (acl_enable == 1)	{
			s6e63m0_panel_send_sequence(acl_cutoff_init);
			msleep(20);

			switch (current_gamma_value)	{
				case 0:
				case 1:
					s6e63m0_panel_send_sequence(ACL_cutoff_set[0]); //set 0% ACL
					cur_acl = 0;
					printk(" ACL_cutoff_set Percentage : 0!!\n");
					break;
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
				case 9:
				case 10:
				case 11:
				case 12:
					s6e63m0_panel_send_sequence(ACL_cutoff_set[9]); //set 40% ACL
					cur_acl = 40;
					printk(" ACL_cutoff_set Percentage : 40!!\n");
					break;
				case 13:
					s6e63m0_panel_send_sequence(ACL_cutoff_set[16]); //set 43% ACL
					cur_acl = 43;
					printk(" ACL_cutoff_set Percentage : 43!!\n");
					break;
				case 14:
					s6e63m0_panel_send_sequence(ACL_cutoff_set[10]); //set 45% ACL
					cur_acl = 45;
					printk(" ACL_cutoff_set Percentage : 45!!\n");
					break;
				case 15:
					s6e63m0_panel_send_sequence(ACL_cutoff_set[11]); //set 47% ACL
					cur_acl = 47;
					printk(" ACL_cutoff_set Percentage : 47!!\n");
					break;
				case 16:
					s6e63m0_panel_send_sequence(ACL_cutoff_set[12]); //set 48% ACL
					cur_acl = 48;
					printk(" ACL_cutoff_set Percentage : 48!!\n");
					break;
		
				default:
					s6e63m0_panel_send_sequence(ACL_cutoff_set[13]); //set 50% ACL
					cur_acl = 50;
					printk(" ACL_cutoff_set Percentage : 50!!\n");
			}
		}
#endif
		else	{
			//ACL Off
			s6e63m0_panel_send_sequence(ACL_cutoff_set[0]); //ACL OFF
			cur_acl  = 0;
			printk(" ACL_cutoff_set Percentage : 0!!\n");
		}
	}

	return size;
}

static DEVICE_ATTR(aclset_file_cmd,0666, aclset_file_cmd_show, aclset_file_cmd_store);
#endif


#ifdef CONFIG_FB_S3C_MDNIE_TUNINGMODE_FOR_BACKLIGHT
extern void mDNIe_Mode_set_for_backlight(u16 *buf);
extern u16 *pmDNIe_Gamma_set[];
extern int pre_val;
extern int autobrightness_mode;
#endif

static void wait_ldi_enable(void)
{
	int i = 0;

	for (i = 0; i < 100; i++)	{
		gprintk("ldi_enable : %d \n", ldi_enable);

		if(IsLDIEnabled())
			break;
		
		msleep(10);
	};
}

static void off_display(void)
{
	msleep(20);
	s6e63m0_panel_send_sequence(s6e63m0_SEQ_DISPLAY_OFF);
	bd_brightness = 0;
	backlight_level = BACKLIGHT_LEVEL_OFF;
	current_gamma_value = -1;
}

static int get_gamma_value_from_bl(int bl)
{
	int gamma_value = 0;
	int gamma_val_x10 = 0;

	if (bl >= MIN_BL)		{
		gamma_val_x10 = 10*(MAX_GAMMA_VALUE-1)*bl/(MAX_BL-MIN_BL) + (10 - 10*(MAX_GAMMA_VALUE-1)*(MIN_BL)/(MAX_BL-MIN_BL)) ;
		gamma_value = (gamma_val_x10+5)/10;
	}	
	else		{
		gamma_value = 0;
	}

	return gamma_value;
}

static void update_elvss(int elvss)
{
    if(lcd_type == LCD_PANEL_SM2)
    {
        if(elvss <= MAX_GAMMA_VALUE)
        {
            if(elvss <= ELVSS_GAMMA_VALUE1)
                s6e63m0_panel_send_sequence(dynamic_elvss_sm2[0]);

            else if(elvss <= ELVSS_GAMMA_VALUE2)
                s6e63m0_panel_send_sequence(dynamic_elvss_sm2[1]);

            else if(elvss <= ELVSS_GAMMA_VALUE3)
                s6e63m0_panel_send_sequence(dynamic_elvss_sm2[2]);

            else
                s6e63m0_panel_send_sequence(dynamic_elvss_sm2[3]);
        }
    }
    else if((lcd_type == LCD_PANEL_M2) || (HWREV >= 8))
    {
        if(elvss <= MAX_GAMMA_VALUE)
        {
            if(elvss <= ELVSS_GAMMA_VALUE1)
                s6e63m0_panel_send_sequence(dynamic_elvss[0]);

            else if(elvss <= ELVSS_GAMMA_VALUE2)
                s6e63m0_panel_send_sequence(dynamic_elvss[1]);

            else if(elvss <= ELVSS_GAMMA_VALUE3)
                s6e63m0_panel_send_sequence(dynamic_elvss[2]);

            else
                s6e63m0_panel_send_sequence(dynamic_elvss[3]);
        }
    }
    else
    {
    }
}

static void update_brightness(int gamma)
{
	if(on_19gamma)
    {
        if(lcd_type == LCD_PANEL_SM2)
        {
		    //update_elvss(gamma);   //Update ELVSS according to brightness
		    s6e63m0_panel_send_sequence(p19Gamma_set_sm2[gamma]);
        }
        else if((lcd_type == LCD_PANEL_M2) || (HWREV >= 8))
        {
		    s6e63m0_panel_send_sequence(p19Gamma_set_hwrev_8[gamma]);
        }
        else
        {
		    s6e63m0_panel_send_sequence(p19Gamma_set[gamma]);
        }

    }
	else
    {
        if(lcd_type == LCD_PANEL_SM2)
        {
		    update_elvss(gamma);   //Update ELVSS according to brightness
    		s6e63m0_panel_send_sequence(p22Gamma_set_sm2[gamma]);
        }
        else if((lcd_type == LCD_PANEL_M2) || (HWREV >= 8))
        {
		    update_elvss(gamma);   //Update ELVSS according to brightness
    		s6e63m0_panel_send_sequence(p22Gamma_set_hwrev_8[gamma]);
        }
        else
        {
    		s6e63m0_panel_send_sequence(p22Gamma_set[gamma]);
        }
    }
		
	s6e63m0_panel_send_sequence(gamma_update); //gamma update
}

static int s5p_bl_update_status(struct backlight_device* bd)
{
	int bl = bd->props.brightness;
	backlight_level_t level = BACKLIGHT_LEVEL_OFF;
	int gamma_value = 0;
	
	wait_ldi_enable();
	
	gprintk("\nupdate status brightness[0~255] : (%d) \n",bd->props.brightness);

	if (!IsLDIEnabled())
		return 0;

	if(bl == 0)
		level = BACKLIGHT_LEVEL_OFF;	//lcd off
	else if((bl < MIN_BL) && (bl > 0))
		level = BACKLIGHT_LEVEL_DIMMING;	//dimming
	else
		level = BACKLIGHT_LEVEL_NORMAL;	//normal

	if (level == BACKLIGHT_LEVEL_OFF)	{
		off_display();
		gprintk("Update status brightness[0~255]:(%d) - LCD OFF \n", bl);
		return 0;
	}	

	gamma_value = get_gamma_value_from_bl(bl);

	bd_brightness = bd->props.brightness;
	backlight_level = level;

	if (current_gamma_value == gamma_value)
		return 0;
	
	gprintk("Update status brightness[0~255]:(%d) gamma_value:(%d) on_19gamma(%d)\n", bl,gamma_value,on_19gamma);

#ifdef CONFIG_FB_S3C_MDNIE_TUNINGMODE_FOR_BACKLIGHT
	if ((pre_val==1) && (gamma_value < 24) &&(autobrightness_mode))		{
		mDNIe_Mode_set_for_backlight(pmDNIe_Gamma_set[2]);
		gprintk("s5p_bl_update_status - pmDNIe_Gamma_set[2]\n" );
		pre_val = -1;
	}
#endif

	switch (level)	{
		case BACKLIGHT_LEVEL_DIMMING:
			update_brightness(0);

#ifdef CONFIG_FB_S3C_TL2796_ACL
			if (acl_enable)		{
#if 0
            if (cur_acl == 0)
						{
							s6e63m0_panel_send_sequence(acl_cutoff_init);
							msleep(20);
						}
#endif
			if (cur_acl != 0)	{
					s6e63m0_panel_send_sequence(ACL_cutoff_set[0]); //set 0% ACL
							gprintk(" ACL_cutoff_set Percentage : 0!!\n");
							cur_acl = 0;
				}
			}
#endif
			gprintk("call s5p_bl_update_status level : %d\n",level);
			break;
		case BACKLIGHT_LEVEL_NORMAL:


#ifdef CONFIG_FB_S3C_TL2796_ACL
			if (acl_enable)		{				
				if ((cur_acl == 0) && (gamma_value != 1))	{
					s6e63m0_panel_send_sequence(acl_cutoff_init);
					msleep(20);
				}

				switch (gamma_value)	{
					case 1:
						if (cur_acl != 0)	{
							s6e63m0_panel_send_sequence(ACL_cutoff_set[0]); //set 0% ACL
							cur_acl = 0;
							gprintk(" ACL_cutoff_set Percentage : 0!!\n");
						}
						break;
					case 2:
					case 3:
					case 4:
					case 5:
					case 6:
					case 7:
					case 8:
					case 9:
					case 10:
					case 11:
					case 12:
						if (cur_acl != 40)	{
							s6e63m0_panel_send_sequence(ACL_cutoff_set[9]); //set 40% ACL
							cur_acl = 40;
							gprintk(" ACL_cutoff_set Percentage : 40!!\n");
						}
						break;

					case 13:
						if (cur_acl != 43)	{
							s6e63m0_panel_send_sequence(ACL_cutoff_set[16]); //set 43% ACL
							cur_acl = 43;
							gprintk(" ACL_cutoff_set Percentage : 43!!\n");
						}
						break;

					case 14:
						if (cur_acl != 45)	{
							s6e63m0_panel_send_sequence(ACL_cutoff_set[10]); //set 45% ACL
							cur_acl = 45;
							gprintk(" ACL_cutoff_set Percentage : 45!!\n");
						}
						break;
					case 15:
						if (cur_acl != 47)	{
							s6e63m0_panel_send_sequence(ACL_cutoff_set[11]); //set 47% ACL
							cur_acl = 47;
							gprintk(" ACL_cutoff_set Percentage : 47!!\n");
						}
						break;
					case 16:
						if (cur_acl != 48)	{
							s6e63m0_panel_send_sequence(ACL_cutoff_set[12]); //set 48% ACL
							cur_acl = 48;
							gprintk(" ACL_cutoff_set Percentage : 48!!\n");
						}
						break;

					default:
						if (cur_acl !=50)	{
							s6e63m0_panel_send_sequence(ACL_cutoff_set[13]); //set 50% ACL
							cur_acl = 50;
							gprintk(" ACL_cutoff_set Percentage : 50!!\n");
						}
				}
			}
#endif
			update_brightness(gamma_value);

			gprintk("#################backlight end##########################\n");

			break;
		default:
			break;
	}

	current_gamma_value = gamma_value;

	return 0;
}

static int s5p_bl_get_brightness(struct backlight_device* bd)
{
	printk(KERN_DEBUG "\n reading brightness \n");
	return bd_brightness;
}

static struct backlight_ops s5p_bl_ops = {
	.update_status = s5p_bl_update_status,
	.get_brightness = s5p_bl_get_brightness,	
};

static int __init tl2796_probe(struct spi_device *spi)
{
	int ret;

	spi->bits_per_word = 9;
	ret = spi_setup(spi);
	lcd.g_spi = spi;
	lcd.lcd_dev = lcd_device_register("s5p_lcd",&spi->dev,&lcd,&s5p_lcd_ops);
	lcd.bl_dev = backlight_device_register("s5p_bl",&spi->dev,&lcd,&s5p_bl_ops);
	lcd.bl_dev->props.max_brightness = 255;
	dev_set_drvdata(&spi->dev,&lcd);

	SetLDIEnabledFlag(1);

#ifdef GAMMASET_CONTROL //for 1.9/2.2 gamma control from platform
	gammaset_class = class_create(THIS_MODULE, "gammaset");
	if (IS_ERR(gammaset_class))
		pr_err("Failed to create class(gammaset_class)!\n");

	switch_gammaset_dev = device_create(gammaset_class, NULL, 0, NULL, "switch_gammaset");
	if (IS_ERR(switch_gammaset_dev))
		pr_err("Failed to create device(switch_gammaset_dev)!\n");

	if (device_create_file(switch_gammaset_dev, &dev_attr_gammaset_file_cmd) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_gammaset_file_cmd.attr.name);
#endif	

#ifdef CONFIG_FB_S3C_TL2796_ACL //ACL On,Off
	acl_class = class_create(THIS_MODULE, "aclset");
	if (IS_ERR(acl_class))
		pr_err("Failed to create class(acl_class)!\n");

	switch_aclset_dev = device_create(acl_class, NULL, 0, NULL, "switch_aclset");
	if (IS_ERR(switch_aclset_dev))
		pr_err("Failed to create device(switch_aclset_dev)!\n");

	if (device_create_file(switch_aclset_dev, &dev_attr_aclset_file_cmd) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_aclset_file_cmd.attr.name);
#endif	

#ifdef CONFIG_FB_S3C_MDNIE
	init_mdnie_class();  //set mDNIe UI mode, Outdoormode
#endif

	if (ret < 0)	{
		pr_err("%s::%d-> s6e63m0 probe failed Err=%d\n",__func__,__LINE__,ret);
		return 0;
	}
	pr_info("%s::%d->s6e63m0 probed successfuly\n",__func__,__LINE__);
	return ret;
}

#if 0
#ifdef CONFIG_PM // add by ksoo (2009.09.07)
int tl2796_suspend(struct platform_device *pdev, pm_message_t state)
{
	pr_info("%s::%d->s6e63m0 suspend called\n",__func__,__LINE__);
	tl2796_ldi_disable();
	return 0;
}

int tl2796_resume(struct platform_device *pdev, pm_message_t state)
{
	pr_info("%s::%d -> s6e63m0 resume called\n",__func__,__LINE__);
	tl2796_ldi_init();
	tl2796_ldi_enable();

	return 0;
}
#endif	/* CONFIG_PM */
#endif

static struct spi_driver tl2796_driver = {
	.driver = {
		.name	= "tl2796",
		.owner	= THIS_MODULE,
	},
	.probe		= tl2796_probe,
	.remove		= __exit_p(tl2796_remove),
#if 0
#ifdef CONFIG_PM
	.suspend	= tl2796_suspend,
	.resume		= tl2796_resume,
#else
	.suspend	= NULL,
	.resume		= NULL,
#endif	/* CONFIG_PM */
#endif
};

static int __init tl2796_init(void)
{
	return spi_register_driver(&tl2796_driver);
}

static void __exit tl2796_exit(void)
{
	spi_unregister_driver(&tl2796_driver);
}


module_init(tl2796_init);
module_exit(tl2796_exit);


MODULE_AUTHOR("SAMSUNG");
MODULE_DESCRIPTION("s6e63m0 LDI driver");
MODULE_LICENSE("GPL");
