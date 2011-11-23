/*
 * s6d16a0x HVGA TFT LCD Panel Driver
*/

#include <linux/wait.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/spi/spi.h>
#include <linux/lcd.h>
#include <linux/backlight.h>


#include <plat/gpio-cfg.h>
#include <mach/gpio-chief.h>
#include <mach/gpio-chief-settings.h>

// TODO:FORTE_FROYO #include <plat/regs-lcd.h>

#include "s3cfb.h"

#define CONFIG_FORTE_LCD_TUNING_00		// 20100904 SHARP LCD tuning - tearing issue

#define DIM_BL	20
#define MIN_BL	30
#define MAX_BL	255

#define MAX_GAMMA_VALUE	24	// we have 25 levels. -> 16 levels -> 24 levels
#define CRITICAL_BATTERY_LEVEL 5

#if !defined(CONFIG_FB_S3C_S6D16A0X)
#define GAMMASET_CONTROL //for 1.9/2.2 gamma control from platform
#define ACL_ENABLE
#endif

/*********** for debug **********************************************************/
#if 0
#define gprintk(fmt, x... ) printk( "%s(%d): " fmt, __FUNCTION__ ,__LINE__, ## x)
#else
#define gprintk(x...) do { } while (0)
#endif

#define FBDBG_TRACE()       printk(KERN_ERR "%s\n", __FUNCTION__)
/*******************************************************************************/

static int locked = 0;
static int ldi_enable = 0;
#if !defined(CONFIG_FB_S3C_S6D16A0X)
int backlight_level = 0;
#endif

int current_gamma_value = -1;
int spi_ing = 0;
int on_19gamma = 0;

static DEFINE_MUTEX(spi_use);

//extern unsigned int get_battery_level(void);
//extern unsigned int is_charging_enabled(void);

struct s5p_lcd{
	struct spi_device *g_spi;
	struct lcd_device *lcd_dev;
	// struct backlight_device *bl_dev;

};

#ifdef GAMMASET_CONTROL
struct class *gammaset_class;
struct device *switch_gammaset_dev;
#endif

#ifdef ACL_ENABLE
int acl_enable = 0;
int cur_acl = 0;

struct class *acl_class;
struct device *switch_aclset_dev;
#endif

#ifdef CONFIG_FB_S3C_MDNIE
extern void init_mdnie_class(void);
#endif


static struct s5p_lcd lcd;
#if 1 //for  chief
static struct s3cfb_lcd s6d16a0x = {
	.width = 320,
	.height = 480,
	.p_width = 52,
	.p_height = 74,
	.bpp = 32,
	.freq = 60,

	.timing = {
		.h_fp = 30,
		.h_bp = 22,
		.h_sw = 8,
		.v_fp = 32,
		.v_fpe = 1,
		.v_bp = 16,
		.v_bpe = 1,
		.v_sw = 16,
	},

	.polarity = {
		.rise_vclk = 0,
		.inv_hsync = 0,
		.inv_vsync = 0,
		.inv_vden = 0,
	},
};

#else
#ifdef CONFIG_FORTE_LCD_TUNING_00
static struct s3cfb_lcd s6d16a0x = {
	.width = 320,
	.height = 480,
	.p_width = 52,
	.p_height = 86,
	.bpp = 32,
	.freq = 95,

	.timing = {
		.h_fp = 16,
		.h_bp = 24,
		.h_sw = 16,
		.v_fp = 2,
		.v_fpe = 1,
		.v_bp = 8,
		.v_bpe = 1,
		.v_sw = 2,
	},

	.polarity = {
		.rise_vclk = 1,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 0,
	},
};
#else
static struct s3cfb_lcd s6d16a0x = {
	.width = 320,
	.height = 480,
	.p_width = 52,
	.p_height = 86,
	.bpp = 32,
	.freq = 60,

	.timing = {
		.h_fp = 64,
		.h_bp = 62,
		.h_sw = 2,
		.v_fp = 8,
		.v_fpe = 1,
		.v_bp = 6,
		.v_bpe = 1,
		.v_sw = 2,
	},

	.polarity = {
		.rise_vclk = 1,
		.inv_hsync = 0,
		.inv_vsync = 0,
		.inv_vden = 0,
	},
};
#endif
#endif
////////////////////////////////////////////////////////////////////////////////
#if 1//for chief
#define CASET		0x2A
#define PASET		0x2B
#define MADCTL		0x36
#define COLMOD		0x3A
#define TESCL		0x44
#define TEON		0x35
#define PASSWD1	0xF0	// level 2 command enable
#define DISCTL		0xF2
#define PWRCTL		0xF4
#define VCMCTL		0xF5
#define SRCCTL		0xF6
#define IFCTL		0xF7
#define PANELCTL	0xF8
#define GAMMASEL	0xF9
#define PGAMMACTL	0xFA
#define NGAMMACTL	0xFB
#define MIECTL1	0xC0
#define BCMODE 	0xC1
#define WRMIECTL2	0xC2
#define WRBLCTL	0xC3
#define PASSWD1	0xF0
#define SLPOUT		0x11
#define DISPON		0x29

#define DISPOFF	0x28
#define SLPIN		0x10
#define WRCABC		0x55	// backlight control mode 
#define PASSWD2	0xF0  // level 1 command enable(default)

#else
#define PASSWD2			0xF1
#define DISCTL			0xF2
#ifdef CONFIG_FORTE_LCD_TUNING_00
#define DISCTL_PWRON    0xF2
#define DISCTL_SLPOUT   0xF2
#define DISCTL_SLPIN    0xF2
#endif
#define POWCTL			0xF3
#define VCMCTL			0xF4
#define SRCCTL			0xF5
#define PANELCTL1		0xF6
#define PANELCTL2		0xF7
#define PANELCTL3		0xF8
#define PANELCTL4		0xF9
#define PGAMMACTL		0xFA
#define NGAMMACTL		0xFB
#define CLKCTL3			0xB7
#define HOSTCTL1		0xB8
#define HOSTCTL2		0xB9
#define TEON			0x35
#define MADCTL          0x36
#define CASET			0x2A
#define PASET			0x2B
#define COLMOD			0x3A
#define WRCTRLD			0x53
#define SLPOUT			0x11
#define DISPON			0x29
#define DISPOFF			0x28
#define SLPIN			0x10
#endif
#if 1// for chief
static const u16	  CASET_PARAM[]    =   { 0x0100, 0x0100, 0x0101, 0x013F } ;
static const u16	  PASET_PARAM[]    =   { 0x0100, 0x0100, 0x0101, 0x01DF } ;
static const u16	  MADCTL_PARAM[]   =   { 0x0100 } ;
static const u16	  COLMOD_PARAM[]   =   { 0x0177 } ;
static const u16	  TESCL_PARAM[]    =   { 0x0100 } ;
static const u16	  TEON_PARAM[]     =   { 0x0100 } ;
static const u16	  PASSWD1_PARAM[]  =   { 0x015A, 0x015A } ;
static const u16	  DISCTL_PARAM[]   =   { 0x013B, 0x014C, 0x010F, 0x0120, 0x0120, 0x0108, 0x0108, 0x0100, 0x0108, 0x0108, 0x0100, 0x0100, 0x0100, 0x0100, 0x014C, 0x0120, 0x0120, 0x0120, 0x0120 } ;
static const u16	  PWRCTL_PARAM[]   =   { 0x0107, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0144, 0x0105, 0x0100, 0x0144, 0x0105 } ;
static const u16	  VCMCTL_PARAM[]   =   { 0x0100, 0x0115, 0x0117, 0x0100, 0x0100, 0x0102, 0x0100, 0x0100, 0x0100, 0x0100, 0x0115, 0x0117 } ;
static const u16	  SRCCTL_PARAM[]   =   { 0x0101, 0x0100, 0x0108, 0x0103, 0x0101, 0x0100, 0x0101, 0x0100, 0x0100 } ;
static const u16	  IFCTL_PARAM[]    =   { 0x0148, 0x0181, 0x01F0, 0x0103, 0x0100 } ;
static const u16	  PANELCTL_PARAM[] =   { 0x0155, 0x0100 } ;
static const u16	  GAMMASEL_PARAM[] =   { 0x0127 } ;
static const u16	  PGAMMACTL_PARAM[]=   { 0x010C, 0x0104, 0x0106, 0x011E, 0x011F, 0x0121, 0x0124, 0x0121, 0x012D, 0x012F, 0x012E, 0x012E, 0x010F, 0x0100, 0x0100, 0x0100 } ;
static const u16	  NGAMMACTL_PARAM[]=   { 0x010C, 0x0104, 0x010F, 0x012E, 0x012E, 0x012F, 0x012D, 0x0121, 0x0124, 0x0121, 0x011F, 0x011E, 0x0106, 0x0100, 0x0100, 0x0100 } ;
static const u16	  MIECTL1_PARAM[]  =   { 0x0180, 0x0180, 0x0110 } ;
static const u16	  BCMODE_PARAM[]   =   { 0x0112 } ;
static const u16	  WRMIECTL2_PARAM[]=   { 0x0108, 0x0100, 0x0100, 0x0101, 0x01DF, 0x0100, 0x0100, 0x0101, 0x013F } ;
static const u16	  WRBLCTL_PARAM[]  =   { 0x0101, 0x0135, 0x0120 } ;
static const u16	  PASSWD2_PARAM[]  =   { 0x01A5, 0x01A5 } ;

static const u16	  WRCABC_PARAM[]   =   { 0x0100 } ;
static const u16	  SLPOUT_PARAM[]   =   { } ;
static const u16	  DISPON_PARAM[]   =   { } ;
static const u16	  SLPIN_PARAM[]    =   { } ;
static const u16	  DISPOFF_PARAM[]  =   { } ;

#else
#if 1    // heatup - reference EXCEL sheet
   #ifdef CONFIG_FORTE_LCD_TUNING_00
      static const u16    PASSWD2_PARAM[]   = { 0x015A, 0x015A } ;

      static const u16    DISCTL_PARAM[]        = { 0x0100, 0x0100, 0x0182, 0x0182, 0x0157, 0x0157, 0x0110, 0x0102, 0x0100 } ;
      static const u16    DISCTL_PWRON_PARAM[]  = { 0x0100, 0x0100, 0x0182, 0x0182, 0x0157, 0x0157, 0x0110, 0x0102, 0x0100 } ;
      static const u16    DISCTL_SLPOUT_PARAM[] = { 0x0100, 0x0100, 0x0162, 0x0162, 0x0157, 0x0157, 0x0110, 0x0102, 0x0100 } ;
      static const u16    DISCTL_SLPIN_PARAM[]  = { 0x0100, 0x0100, 0x0162, 0x0162, 0x0157, 0x0157, 0x0110, 0x0100, 0x0100 } ;

      static const u16    POWCTL_PARAM[]    = { 0x0100, 0x0110, 0x0125, 0x0101, 0x012D, 0x012D, 0x0124, 0x012D, 0x0114, 0x0114, 0x0112, 0x0178} ;
      static const u16    VCMCTL_PARAM[]    = { 0x0100, 0x0123, 0x0100, 0x0157, 0x0168, 0x0100, 0x0157, 0x0168, 0x0100, 0x0100 } ;
      static const u16    SRCCTL_PARAM[]    = { 0x0100, 0x0100, 0x0157, 0x0100, 0x010B, 0x0101, 0x0114, 0x0114, 0x0109, 0x0109 } ;
      static const u16    PANELCTL1_PARAM[] = { 0x0102, 0x0100, 0x0180, 0x0100, 0x0144, 0x0100 } ;
      static const u16    PANELCTL2_PARAM[] = { 0x0100, 0x0101, 0x0100, 0x01F2, 0x0108, 0x0100, 0x0107, 0x011C, 0x0107, 0x0108, 0x0123, 0x0100, 0x0107, 0x0100, 0x014B, 0x0100, 0x018C } ;
      static const u16    PANELCTL3_PARAM[] = { 0x0100, 0x0101, 0x0100, 0x01F2, 0x0108, 0x0100, 0x0107, 0x011C, 0x0107, 0x0108, 0x0123, 0x0100, 0x0107, 0x0100, 0x014B, 0x0100, 0x018C } ;
      static const u16    PANELCTL4_PARAM[] = { 0x0100, 0x0108, 0x0100, 0x0101, 0x0100, 0x0105, 0x0100, 0x0104, 0x0100, 0x010C, 0x0102, 0x010F, 0x0100, 0x0110, 0x0100, 0x0111, 0x0100, 0x0100, 0x0100, 0x011F, 0x01FF, 0x01C0 } ;
      static const u16    PGAMMACTL_PARAM[] = { 0x0121, 0x0137, 0x012C, 0x010F, 0x0113, 0x0117, 0x011A, 0x010F, 0x010B, 0x0123, 0x0119, 0x0121, 0x012A, 0x0129, 0x0127, 0x0119, 0x0137, 0x012C, 0x010F, 0x0114, 0x0117, 0x011A, 0x010D, 0x0109, 0x0122, 0x0116, 0x011A, 0x0121, 0x011D, 0x011C, 0x0100, 0x0137, 0x012C, 0x0112, 0x0116, 0x0117, 0x011C, 0x0110, 0x010E, 0x0125, 0x0112, 0x0110, 0x0107, 0x0100, 0x0100 } ;
      static const u16    NGAMMACTL_PARAM[] = { 0x0100, 0x011B, 0x0115, 0x0113, 0x0132, 0x013B, 0x0143, 0x0138, 0x0132, 0x014D, 0x0141, 0x0144, 0x0145, 0x0123, 0x010D, 0x0100, 0x0122, 0x011F, 0x011E, 0x013B, 0x0142, 0x0146, 0x0138, 0x0130, 0x014B, 0x0142, 0x0143, 0x0142, 0x0123, 0x010D, 0x0100, 0x013F, 0x013E, 0x013E, 0x0155, 0x014A, 0x0148, 0x0138, 0x0131, 0x014C, 0x0141, 0x0143, 0x0140, 0x011F, 0x010D } ;
      static const u16    CLKCTL3_PARAM[]   = { 0x0100, 0x0111, 0x0111 } ;
      static const u16    HOSTCTL1_PARAM[]  = { 0x0131, 0x0111 } ;
      static const u16    HOSTCTL2_PARAM[]  = { 0x0100, 0x0106 } ;
      static const u16    TEON_PARAM[]      = {} ;
      static const u16    MADCTL_PARAM[]    = { 0x0180 } ;    /* vertical flip */
      static const u16    CASET_PARAM[]     = { 0x0100, 0x0100, 0x0101, 0x013F } ;
      static const u16    PASET_PARAM[]     = { 0x0100, 0x0100, 0x0101, 0x01DF } ;
      static const u16    COLMOD_PARAM[]    = { 0x0177 } ;
      static const u16    WRCTRLD_PARAM[]   = { 0x0100 } ;
      static const u16    DISPON_PARAM[]    = {} ;
      static const u16    SLPOUT_PARAM[]    = {} ;
      static const u16    DISPOFF_PARAM[]   = { 0x0102, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100 } ;
      static const u16    SLPIN_PARAM[]     = { 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100 } ;
   #else
static const u16    PASSWD2_PARAM[]   = { 0x015A, 0x015A } ;
static const u16    DISCTL_PARAM[]    = { 0x0100, 0x0100, 0x0162, 0x0162, 0x0157, 0x0157, 0x0110, 0x0100, 0x0100 } ;
static const u16    POWCTL_PARAM[]    = { 0x0100, 0x0110, 0x0125, 0x0101, 0x012D, 0x012D, 0x0124, 0x012D, 0x0114, 0x0114, 0x0112, 0x0178} ;
static const u16    VCMCTL_PARAM[]    = { 0x0100, 0x0123, 0x0100, 0x0157, 0x0168, 0x0100, 0x0157, 0x0168, 0x0100, 0x0100 } ;
static const u16    SRCCTL_PARAM[]    = { 0x0100, 0x0100, 0x0157, 0x0100, 0x0108, 0x0101, 0x0114, 0x0114, 0x0109, 0x0109 } ;
static const u16    PANELCTL1_PARAM[] = { 0x0102, 0x0100, 0x0180, 0x0100, 0x0144, 0x0100 } ;
      static const u16    PANELCTL2_PARAM[] = { 0x0100, 0x0101, 0x0100, 0x01F2, 0x0108, 0x0100, 0x0108, 0x011F, 0x0108, 0x0108, 0x0123, 0x0100, 0x0107, 0x0100, 0x014B, 0x0100, 0x018C } ;
      static const u16    PANELCTL3_PARAM[] = { 0x0100, 0x0101, 0x0100, 0x01F2, 0x0108, 0x0100, 0x0108, 0x011F, 0x0108, 0x0108, 0x0123, 0x0100, 0x0107, 0x0100, 0x014B, 0x0100, 0x018C } ;
      static const u16    PANELCTL4_PARAM[] = { 0x0100, 0x0108, 0x0100, 0x0101, 0x0100, 0x0105, 0x0100, 0x0104, 0x0100, 0x010C, 0x0102, 0x010F, 0x0100, 0x0110, 0x0100, 0x0111, 0x0100, 0x0100, 0x0100, 0x011F, 0x01FF, 0x01C0 } ;
      static const u16    PGAMMACTL_PARAM[] = { 0x0119, 0x0137, 0x012C, 0x010F, 0x0113, 0x0116, 0x0118, 0x010C, 0x0107, 0x011F, 0x0113, 0x011A, 0x0122, 0x011F, 0x011E, 0x0113, 0x0137, 0x012C, 0x010F, 0x0114, 0x0116, 0x0119, 0x010C, 0x0108, 0x0120, 0x0112, 0x0116, 0x011D, 0x0118, 0x0117, 0x0100, 0x0137, 0x012C, 0x0112, 0x0116, 0x0117, 0x011C, 0x0110, 0x010E, 0x0125, 0x0112, 0x0110, 0x0107, 0x0100, 0x0100 } ;
      static const u16    NGAMMACTL_PARAM[] = { 0x0100, 0x0123, 0x011E, 0x011D, 0x013B, 0x0142, 0x0149, 0x013C, 0x0136, 0x014F, 0x0143, 0x0145, 0x0145, 0x0123, 0x010D, 0x0100, 0x0128, 0x0124, 0x0123, 0x013F, 0x0147, 0x014A, 0x013A, 0x0132, 0x014C, 0x0143, 0x0144, 0x0142, 0x0123, 0x010D, 0x0100, 0x013E, 0x013E, 0x013E, 0x0155, 0x014A, 0x0148, 0x0138, 0x0131, 0x014C, 0x0141, 0x0143, 0x0140, 0x011F, 0x010D } ;
      static const u16    CLKCTL3_PARAM[]   = { 0x0100, 0x0111, 0x0111 } ;
      static const u16    HOSTCTL1_PARAM[]  = { 0x0131, 0x0111 } ;
      static const u16    HOSTCTL2_PARAM[]  = { 0x0100, 0x0106 } ;
      static const u16    TEON_PARAM[]      = {} ;
      static const u16    MADCTL_PARAM[]    = { 0x0180 } ;    /* vertical flip */
      static const u16    CASET_PARAM[]     = { 0x0100, 0x0100, 0x0101, 0x013F } ;
      static const u16    PASET_PARAM[]     = { 0x0100, 0x0100, 0x0101, 0x01DF } ;
static const u16    COLMOD_PARAM[]    = { 0x0177 } ;
static const u16    WRCTRLD_PARAM[]   = { 0x0100 } ;
static const u16    DISPON_PARAM[]    = {} ;
static const u16    SLPOUT_PARAM[]    = {} ;
static const u16    DISPOFF_PARAM[]   = { 0x0102, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100 } ;
static const u16    SLPIN_PARAM[]     = { 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100 } ;
   #endif
#else
static const u16    PASSWD2_PARAM[]   = { 0x015A, 0x015A } ;
static const u16    DISCTL_PARAM[]    = { 0x0100, 0x0100, 0x0162, 0x0162, 0x0157, 0x0157, 0x0110, 0x0100, 0x0100 } ;
static const u16    POWCTL_PARAM[]    = { 0x0100, 0x0110, 0x0125, 0x0101, 0x012D, 0x012D, 0x0124, 0x012d, 0x0114, 0x0114, 0x0112, 0x0178} ;
static const u16    VCMCTL_PARAM[]    = { 0x0100, 0x0123, 0x0100, 0x0155, 0x0168, 0x0100, 0x0155, 0x0168, 0x0100, 0x0100 } ;
static const u16    SRCCTL_PARAM[]    = { 0x0100, 0x0100, 0x0157, 0x0100, 0x0108, 0x0101, 0x0114, 0x0114, 0x0109, 0x0109 } ;
static const u16    PANELCTL1_PARAM[] = { 0x0102, 0x0100, 0x0180, 0x0100, 0x0144, 0x0100 } ;
static const u16    PANELCTL2_PARAM[] = { 0x0100, 0x0101, 0x0100, 0x01F2, 0x0108, 0x0100, 0x0108, 0x011F, 0x0108, 0x0108, 0x0123, 0x0100, 0x0107, 0x0100, 0x014B, 0x0100, 0x018C } ;
static const u16    PANELCTL3_PARAM[] = { 0x0100, 0x0101, 0x0100, 0x01F2, 0x0108, 0x0100, 0x0108, 0x011F, 0x0108, 0x0108, 0x0123, 0x0100, 0x0107, 0x0100, 0x014B, 0x0100, 0x018C } ;
static const u16    PANELCTL4_PARAM[] = { 0x0100, 0x0108, 0x0100, 0x0101, 0x0100, 0x0105, 0x0100, 0x0104, 0x0100, 0x010C, 0x0100, 0x010F, 0x0100, 0x0110, 0x0100, 0x0111, 0x0100, 0x0100, 0x0100, 0x011F, 0x01FF, 0x01E0 } ;
static const u16    PGAMMACTL_PARAM[] = { 0x010D, 0x0135, 0x012C, 0x010F, 0x0111, 0x0112, 0x0111, 0x0106, 0x0102, 0x011C, 0x010D, 0x010D, 0x0113, 0x010F, 0x010D, 0x0112, 0x0135, 0x012C, 0x010F, 0x0111, 0x0113, 0x0115, 0x0108, 0x0104, 0x011C, 0x010E, 0x0112, 0x0119, 0x0114, 0x0112, 0x0123, 0x0135, 0x012C, 0x0112, 0x0114, 0x0115, 0x0118, 0x010D, 0x0108, 0x0122, 0x0113, 0x0119, 0x0123, 0x0122, 0x0123 } ;
static const u16    NGAMMACTL_PARAM[] = { 0x0101, 0x012D, 0x012D, 0x012A, 0x0145, 0x014C, 0x014E, 0x013F, 0x0138, 0x0153, 0x014A, 0x0147, 0x0145, 0x0123, 0x010D, 0x0101, 0x0129, 0x0129, 0x0127, 0x0142, 0x014A, 0x014E, 0x013E, 0x0137, 0x0150, 0x0147, 0x0147, 0x0144, 0x0123, 0x010D, 0x0101, 0x0117, 0x0117, 0x0118, 0x0137, 0x0141, 0x0147, 0x013A, 0x0133, 0x014F, 0x0145, 0x0145, 0x0142, 0x0120, 0x010D } ;
static const u16    CLKCTL3_PARAM[]   = { 0x0100, 0x0111, 0x0111 } ;
static const u16    HOSTCTL1_PARAM[]  = { 0x0131, 0x0111 } ;
static const u16    HOSTCTL2_PARAM[]  = { 0x0100, 0x0106 } ;
static const u16    TEON_PARAM[]      = {} ;
static const u16    CASET_PARAM[]     = { 0x0100, 0x0100, 0x0101, 0x013F } ;
static const u16    PASET_PARAM[]     = { 0x0100, 0x0100, 0x0101, 0x01DF } ;
static const u16    COLMOD_PARAM[]    = { 0x0177 } ;
static const u16    WRCTRLD_PARAM[]   = { 0x0100 } ;
static const u16    DISPON_PARAM[]    = {} ;
static const u16    SLPOUT_PARAM[]    = {} ;
static const u16    DISPOFF_PARAM[]   = { 0x0102, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100 } ;
static const u16    SLPIN_PARAM[]     = { 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100 } ;
#endif
#endif
#define DEF_LCD_CMD(_s_, _t_)           {_s_, ARRAY_SIZE(_s_##_PARAM), _s_##_PARAM, _t_}

struct setting_table {
	u16         command;
	u16         param_num ;
	const u16   *p_param;
	s32         wait_ms;
};

static const struct setting_table s6d16a0x_power_on_seq[] = {
#if 1
DEF_LCD_CMD(CASET, 0),
DEF_LCD_CMD(PASET, 0),
DEF_LCD_CMD(MADCTL, 0),
DEF_LCD_CMD(COLMOD, 0),
DEF_LCD_CMD(TESCL, 0),
DEF_LCD_CMD(TEON, 0),
DEF_LCD_CMD(PASSWD1, 0),
DEF_LCD_CMD(DISCTL, 0),
DEF_LCD_CMD(PWRCTL, 0),
DEF_LCD_CMD(VCMCTL, 0),
DEF_LCD_CMD(SRCCTL, 0),
DEF_LCD_CMD(IFCTL, 0),
DEF_LCD_CMD(PANELCTL, 0),
DEF_LCD_CMD(GAMMASEL, 0),
DEF_LCD_CMD(PGAMMACTL, 0),
DEF_LCD_CMD(NGAMMACTL, 0),
DEF_LCD_CMD(MIECTL1, 0),
DEF_LCD_CMD(BCMODE, 0),
DEF_LCD_CMD(WRMIECTL2, 0),
DEF_LCD_CMD(WRBLCTL, 0),
DEF_LCD_CMD(PASSWD2, 0),
DEF_LCD_CMD(WRCABC, 0)
#else
    DEF_LCD_CMD(PASSWD2, 0),
#ifdef CONFIG_FORTE_LCD_TUNING_00
	DEF_LCD_CMD(DISCTL_PWRON, 0),
#else
	DEF_LCD_CMD(DISCTL, 0),
#endif
	DEF_LCD_CMD(POWCTL, 0),
	DEF_LCD_CMD(VCMCTL, 0),
	DEF_LCD_CMD(SRCCTL, 0),
	DEF_LCD_CMD(PANELCTL1,0),
	DEF_LCD_CMD(PANELCTL2, 0),
	DEF_LCD_CMD(PANELCTL3, 0),
	DEF_LCD_CMD(PANELCTL4, 0),
	DEF_LCD_CMD(PGAMMACTL, 0),
	DEF_LCD_CMD(NGAMMACTL, 0),
	DEF_LCD_CMD(CLKCTL3, 0),
	DEF_LCD_CMD(HOSTCTL1, 0),
	DEF_LCD_CMD(HOSTCTL2, 0),
	DEF_LCD_CMD(TEON, 0),
	//DEF_LCD_CMD(MADCTL, 0),
	DEF_LCD_CMD(PASSWD2, 0),
	DEF_LCD_CMD(CASET, 0),
	DEF_LCD_CMD(PASET, 0),
	DEF_LCD_CMD(COLMOD, 0),
	DEF_LCD_CMD(WRCTRLD, 0)
#endif
};

static const struct setting_table s6d16a0x_disp_on_seq[] = {
	DEF_LCD_CMD(DISPON, 0)
} ;

static const struct setting_table s6d16a0x_sleep_out_seq[] = {
	DEF_LCD_CMD(SLPOUT, 120)
} ;

static const struct setting_table s6d16a0x_disp_off_seq[] = {
	DEF_LCD_CMD(DISPOFF, 50)
};

static const struct setting_table s6d16a0x_sleep_in_seq[] = {
	DEF_LCD_CMD(SLPIN, 120)
};
#if 0//for chief def CONFIG_FORTE_LCD_TUNING_00
static const struct setting_table s6d16a0x_sleep_in_pre_seq[] = {
   DEF_LCD_CMD(PASSWD2, 0),
   DEF_LCD_CMD(DISCTL_SLPIN, 0),
   DEF_LCD_CMD(PASSWD2, 5),
};

static const struct setting_table s6d16a0x_sleep_out_pre_seq[] = {
   DEF_LCD_CMD(PASSWD2, 0),
   DEF_LCD_CMD(DISCTL_SLPOUT, 0),
   DEF_LCD_CMD(PASSWD2, 5),
};
#endif

////////////////////////////////////////////////////////////////////////////////
static int  s6d16a0x_spi_write(const u16 data)
{
    int     ret;
    u16     buff = data ;
    struct spi_message msg;
    struct spi_transfer xfer = {
    	.len	= 2,
    	.tx_buf	= &buff,
    };

    spi_message_init(&msg);
    spi_message_add_tail(&xfer, &msg);
    ret = spi_sync(lcd.g_spi, &msg);

    if(ret < 0)
    	printk(KERN_ERR "%s::%d -> spi_sync failed Err=%d\n",__func__,__LINE__,ret);

    return ret ;
}

////////////////////////////////////////////////////////////////////////////////
static void s6d16a0x_write_cmd_params(const struct setting_table *table)
{
    int     n ;
    u16     *p_param ;

    s6d16a0x_spi_write(table->command) ;
    for (n = table->param_num, p_param = table->p_param ; n ; n--, p_param++)
        s6d16a0x_spi_write(*p_param) ;

	msleep(table->wait_ms);
}

////////////////////////////////////////////////////////////////////////////////
void s6d16a0x_write_seq(const struct setting_table *p_seq, int size)
{
  int    i;

  for (i = 0; i < size ; i++)
    s6d16a0x_write_cmd_params(&p_seq[i]);
}

#define s6d16a0x_write_sequence(_arr_)      s6d16a0x_write_seq(_arr_, ARRAY_SIZE(_arr_))

#define DISPLAY_CS	GPIO_DISPLAY_CS
#define DISPLAY_CLK	GPIO_DISPLAY_CLK
#define DISPLAY_SI	GPIO_DISPLAY_SI
////////////////////////////////////////////////////////////////////////////////
void s6d16a0x_lcd_cfg_gpio(void)
{
    int i;

    FBDBG_TRACE() ;
    /* Data Pin */
	for (i = 0; i < 8; i++) {
		s3c_gpio_cfgpin(S5PV210_GPF0(i), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV210_GPF0(i), S3C_GPIO_PULL_NONE);
	}

	for (i = 0; i < 8; i++) {
		s3c_gpio_cfgpin(S5PV210_GPF1(i), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV210_GPF1(i), S3C_GPIO_PULL_NONE);
	}

	for (i = 0; i < 8; i++) {
		s3c_gpio_cfgpin(S5PV210_GPF2(i), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV210_GPF2(i), S3C_GPIO_PULL_NONE);
	}

   /* HSYNC, VSYNC, DE, CLK */
	for (i = 0; i < 4; i++) {
		s3c_gpio_cfgpin(S5PV210_GPF3(i), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV210_GPF3(i), S3C_GPIO_PULL_NONE);
	}

	/* DISPLAY_CS */
	s3c_gpio_cfgpin(DISPLAY_CS, S3C_GPIO_OUTPUT);
	/* DISPLAY_CLK */
	s3c_gpio_cfgpin(DISPLAY_CLK, S3C_GPIO_OUTPUT);
	/* DISPLAY_SI */
	s3c_gpio_cfgpin(DISPLAY_SI, S3C_GPIO_OUTPUT);

	/* DISPLAY_CS */
	s3c_gpio_setpull(DISPLAY_CS, S3C_GPIO_PULL_NONE);
	/* DISPLAY_CLK */
	s3c_gpio_setpull(DISPLAY_CLK, S3C_GPIO_PULL_NONE);
	/* DISPLAY_SI */
	s3c_gpio_setpull(DISPLAY_SI, S3C_GPIO_PULL_NONE);

   /* MLCD_RST */
	// for chief
	s3c_gpio_cfgpin(GPIO_MLCD_RST, S3C_GPIO_SFN(1));
	s3c_gpio_setpull(GPIO_MLCD_RST, S3C_GPIO_PULL_NONE);

	msleep(1);

}

////////////////////////////////////////////////////////////////////////////////
void s6d16a0x_lcd_reset(void)
{
#if 1
    s3c_gpio_cfgpin(GPIO_MLCD_RST, S3C_GPIO_SFN(1));
	msleep(10);
	gpio_set_value(GPIO_MLCD_RST, 1);
	msleep(20);
	gpio_set_value(GPIO_MLCD_RST, 0);
	msleep(20); // more than 10msec
	gpio_set_value(GPIO_MLCD_RST, 1);
	msleep(20); // wait more than 30 ms after releasing the system reset( > 10msec) and input RGB interface signal (RGB pixel datga, sync signals,dot clock...) ( >= 20msec)

#else
   gpio_set_value(GPIO_MLCD_RST, GPIO_LEVEL_HIGH);
   msleep(1);

   gpio_set_value(GPIO_MLCD_RST, GPIO_LEVEL_LOW);
   msleep(1);
   gpio_set_value(GPIO_MLCD_RST, GPIO_LEVEL_HIGH);
   msleep(10);
#endif
}

int IsLDIEnabled(void)
{
	return ldi_enable;
}
EXPORT_SYMBOL(IsLDIEnabled);

////////////////////////////////////////////////////////////////////////////////
static void SetLDIEnabledFlag(int OnOff)
{
	ldi_enable = OnOff;
}

////////////////////////////////////////////////////////////////////////////////
void s6d16a0x_ldi_init(void)
{
    FBDBG_TRACE();

#if 1   // TODO:FORTE_FROYO
    //s6d16a0x_lcd_cfg_gpio();
//    s6d16a0x_lcd_reset();
printk(KERN_ERR "%s,THIS IS LCD ON \n", __FUNCTION__);

#endif
    msleep(20);   // heatup
    s6d16a0x_write_sequence(s6d16a0x_power_on_seq) ;
    s6d16a0x_write_sequence(s6d16a0x_sleep_out_seq) ;
    s6d16a0x_write_sequence(s6d16a0x_disp_on_seq) ;
    msleep(1);   // heatup

    SetLDIEnabledFlag(1);
}

////////////////////////////////////////////////////////////////////////////////
void s6d16a0x_ldi_wake_up(void)
{
    FBDBG_TRACE();

#ifdef CONFIG_FORTE_LCD_TUNING_00
//    s6d16a0x_write_sequence(s6d16a0x_sleep_out_pre_seq) ;
#endif
    s6d16a0x_write_sequence(s6d16a0x_sleep_out_seq) ;
    s6d16a0x_write_sequence(s6d16a0x_disp_on_seq) ;
}

////////////////////////////////////////////////////////////////////////////////
void s6d16a0x_ldi_stand_by(void)
{
    FBDBG_TRACE();

#ifdef CONFIG_FORTE_LCD_TUNING_00
//    s6d16a0x_write_sequence(s6d16a0x_sleep_in_pre_seq);		// heatup
#endif
    s6d16a0x_write_sequence(s6d16a0x_disp_off_seq) ;
    s6d16a0x_write_sequence(s6d16a0x_sleep_in_seq) ;
}

////////////////////////////////////////////////////////////////////////////////
void s6d16a0x_ldi_enable(void)
{
    FBDBG_TRACE();
}

////////////////////////////////////////////////////////////////////////////////
void s6d16a0x_ldi_disable(void)
{
    FBDBG_TRACE();

	s6d16a0x_ldi_stand_by() ;

	SetLDIEnabledFlag(0);
	printk( "LCD OFF !!! \n");
	dev_dbg(lcd.lcd_dev,"%s::%d -> ldi disabled\n",__func__,__LINE__);
}

////////////////////////////////////////////////////////////////////////////////
void s3cfb_set_lcd_info(struct s3cfb_global *ctrl)
{
    FBDBG_TRACE();

	s6d16a0x.init_ldi = NULL;
	ctrl->lcd = &s6d16a0x ;
}

//mkh:lcd operations and functions
////////////////////////////////////////////////////////////////////////////////
static int s5p_lcd_set_power(struct lcd_device *ld, int power)
{
    FBDBG_TRACE() ;

	if(power)
	{
        s6d16a0x_write_sequence(s6d16a0x_disp_on_seq) ;
	}
	else
    {
		s6d16a0x_write_sequence(s6d16a0x_disp_off_seq);
	}
}

////////////////////////////////////////////////////////////////////////////////

// cmk 2011.04.11 fixed camera & video player black screen issue. Add s5p_lcd_check_fb with return 0.
static int s5p_lcd_check_fb(struct lcd_device *lcddev, struct fb_info *fi)
{
	return 0;
}

static struct lcd_ops s5p_lcd_ops = {
	.set_power = s5p_lcd_set_power,
	.check_fb = s5p_lcd_check_fb,
};
// end cmk

//mkh:backlight operations and functions

void bl_update_status_22gamma(int bl)
{
    FBDBG_TRACE() ;
#if !defined(CONFIG_FB_S3C_S6D16A0X)

	int gamma_value = 0;
	int i;

	for(i=0; i<100; i++)
	{
		gprintk("ldi_enable : %d \n",ldi_enable);

		if(IsLDIEnabled())
			break;

		msleep(10);
	};

	if(!(current_gamma_value == -1))
	{
		gprintk("#################22gamma start##########################\n");
		s6e63m0_panel_send_sequence(p22Gamma_set[current_gamma_value]);
		gprintk("#################22gamma end##########################\n");
	}


	//printk("bl_update_status_22gamma : current_gamma_value(%d) \n",current_gamma_value);
#endif
}
EXPORT_SYMBOL(bl_update_status_22gamma);


void bl_update_status_19gamma(int bl)
{
    FBDBG_TRACE() ;
#if !defined(CONFIG_FB_S3C_S6D16A0X)
	int gamma_value = 0;
	int i;

	for(i=0; i<100; i++)
	{
		gprintk("ldi_enable : %d \n",ldi_enable);

		if(IsLDIEnabled())
			break;

		msleep(10);
	};

	if(!(current_gamma_value == -1))
	{
		gprintk("#################19gamma start##########################\n");
		s6e63m0_panel_send_sequence(p19Gamma_set[current_gamma_value]);
		gprintk("#################19gamma end##########################\n");
	}

	//printk("bl_update_status_19gamma : current_gamma_value(%d) \n",current_gamma_value);
#endif
}
EXPORT_SYMBOL(bl_update_status_19gamma);


#ifdef GAMMASET_CONTROL //for 1.9/2.2 gamma control from platform
static ssize_t gammaset_file_cmd_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
	gprintk("called %s \n",__func__);

	return sprintf(buf,"%u\n",bd_brightness);
}
static ssize_t gammaset_file_cmd_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t size)
{
	int value;

    sscanf(buf, "%d", &value);

	//printk(KERN_INFO "[gamma set] in gammaset_file_cmd_store, input value = %d \n",value);
	if(IsLDIEnabled()==0)
	{
		//printk(KERN_DEBUG "[gamma set] return because LDI is disabled, input value = %d \n",value);
		printk("[gamma set] return because LDI is disabled, input value = %d \n",value);
		return size;
	}

	if(value==1 && on_19gamma==0)
	{
		on_19gamma = 1;
		bl_update_status_19gamma(bd_brightness);
	}
	else if(value==0 && on_19gamma==1)
	{
		on_19gamma = 0;
		bl_update_status_22gamma(bd_brightness);
	}
	else
		printk("\ngammaset_file_cmd_store value(%d) on_19gamma(%d) \n",value,on_19gamma);

	return size;
}

static DEVICE_ATTR(gammaset_file_cmd,0666, gammaset_file_cmd_show, gammaset_file_cmd_store);
#endif

#ifdef ACL_ENABLE
static ssize_t aclset_file_cmd_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	gprintk("called %s \n",__func__);

	return sprintf(buf,"%u\n", acl_enable);
}
static ssize_t aclset_file_cmd_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int value;

	sscanf(buf, "%d", &value);

	//printk(KERN_INFO "[acl set] in aclset_file_cmd_store, input value = %d \n", value);

	if(IsLDIEnabled()==0)
	{
		printk(KERN_DEBUG "[acl set] return because LDI is disabled, input value = %d \n",value);
		return size;
	}

	if(value==1 && acl_enable == 0)
	{
		acl_enable = value;

		s6e63m0_panel_send_sequence(acl_cutoff_init);
		msleep(20);

		if (current_gamma_value ==1)
		{
			s6e63m0_panel_send_sequence(ACL_cutoff_set[0]); //set 0% ACL
			cur_acl = 0;
			//printk(" ACL_cutoff_set Percentage : 0!!\n");
		}
		else if(current_gamma_value ==2)
		{
			s6e63m0_panel_send_sequence(ACL_cutoff_set[1]); //set 12% ACL
			cur_acl = 12;
			//printk(" ACL_cutoff_set Percentage : 12!!\n");
		}
		else if(current_gamma_value ==3)
		{
			s6e63m0_panel_send_sequence(ACL_cutoff_set[2]); //set 22% ACL
			cur_acl = 22;
			//printk(" ACL_cutoff_set Percentage : 22!!\n");
		}
		else if(current_gamma_value ==4)
		{
			s6e63m0_panel_send_sequence(ACL_cutoff_set[3]); //set 30% ACL
			cur_acl = 30;
			//printk(" ACL_cutoff_set Percentage : 30!!\n");
		}
		else if(current_gamma_value ==5)
		{
			s6e63m0_panel_send_sequence(ACL_cutoff_set[4]); //set 35% ACL
			cur_acl = 35;
			//printk(" ACL_cutoff_set Percentage : 35!!\n");
		}
		else
		{
			s6e63m0_panel_send_sequence(ACL_cutoff_set[5]); //set 40% ACL
			cur_acl = 40;
			//printk(" ACL_cutoff_set Percentage : 40!!\n");
		}
	}
	else if(value==0 && acl_enable == 1)
	{
		acl_enable = value;

		//ACL Off
		s6e63m0_panel_send_sequence(ACL_cutoff_set[0]); //ACL OFF
		//printk(" ACL_cutoff_set Percentage : 0!!\n");
		cur_acl  = 0;
	}
	else
		printk("\naclset_file_cmd_store value is same : value(%d)\n",value);

	return size;
}

static DEVICE_ATTR(aclset_file_cmd,0666, aclset_file_cmd_show, aclset_file_cmd_store);
#endif


#undef MDNIE_TUNINGMODE_FOR_BACKLIGHT

#ifdef MDNIE_TUNINGMODE_FOR_BACKLIGHT
extern void mDNIe_Mode_set_for_backlight(u16 *buf);
extern u16 *pmDNIe_Gamma_set[];
extern int pre_val;
extern int autobrightness_mode;
#endif

#if !defined(CONFIG_FB_S3C_S6D16A0X)
static int s5p_bl_update_status(struct backlight_device* bd)
{
	int bl = bd->props.brightness;
	int level = 0;
	int gamma_value = 0;
	int gamma_val_x10 = 0;
	int i = 0;

    FBDBG_TRACE() ;

	for(i=0; i<100; i++)
	{
//		printk("ldi_enable : %d \n",ldi_enable);

		if(IsLDIEnabled())
			break;

		msleep(10);
	};

	if(IsLDIEnabled())
	{
	#if 0
		if (get_battery_level() <= CRITICAL_BATTERY_LEVEL && !is_charging_enabled())
		{
			if (bl > DIM_BL)
				bl = DIM_BL;
		}
	#endif
		if(bl == 0)
			level = 0;	//lcd off
		else if((bl < MIN_BL) && (bl > 0))
			level = 1;	//dimming
		else
			level = 6;	//normal

		if(level==0)
		{
			msleep(20);
			s6e63m0_panel_send_sequence(s6e63m0_SEQ_DISPLAY_OFF);
//			printk("Update status brightness[0~255]:(%d) - LCD OFF \n", bl);
			bd_brightness = 0;
			backlight_level = 0;
			current_gamma_value = -1;
			return 0;
		}

		if (bl >= MIN_BL)
		{
			gamma_val_x10 = 10*(MAX_GAMMA_VALUE-1)*bl/(MAX_BL-MIN_BL) + (10 - 10*(MAX_GAMMA_VALUE-1)*(MIN_BL)/(MAX_BL-MIN_BL)) ;
			gamma_value = (gamma_val_x10+5)/10;
		}
		else
		{
			gamma_value = 0;
		}

		pr_err("brightness =  %d, gamma = %d\n",bd->props.brightness, gamma_value);

		bd_brightness = bd->props.brightness;
		backlight_level = level;

		if(current_gamma_value == gamma_value)
		{
			return 0;
		}

	 	if(level)
		{
			#ifdef MDNIE_TUNINGMODE_FOR_BACKLIGHT
			if((pre_val==1)&&(gamma_value < 24)&&(autobrightness_mode))
			{
				mDNIe_Mode_set_for_backlight(pmDNIe_Gamma_set[2]);
//				printk("s5p_bl_update_status - pmDNIe_Gamma_set[2]\n" );
				pre_val = -1;
			}
			#endif

			switch(level)
			{
				case  5:
				case  4:
				case  3:
				case  2:
				case  1: //dimming
				{
					if(on_19gamma)
						s6e63m0_panel_send_sequence(p19Gamma_set[0]);
					else
						s6e63m0_panel_send_sequence(p22Gamma_set[0]);


				#ifdef ACL_ENABLE
					if (acl_enable)
					{
						if (cur_acl != 0)
						{
							s6e63m0_panel_send_sequence(ACL_cutoff_set[0]); //set 0% ACL
//							printk(" ACL_cutoff_set Percentage : 0!!\n");
							cur_acl = 0;
						}
					}
				#endif
					//printk("call s5p_bl_update_status level : %d\n",level);
					break;
				}
				case  6:
				{
				#ifdef ACL_ENABLE
					if (acl_enable)
					{
						if (gamma_value ==1)
						{
							if (cur_acl != 0)
							{
								s6e63m0_panel_send_sequence(ACL_cutoff_set[0]); //set 0% ACL
								cur_acl = 0;
								//printk(" ACL_cutoff_set Percentage : 0!!\n");
							}
						}
						else
						{
							if (cur_acl == 0)
							{
								s6e63m0_panel_send_sequence(acl_cutoff_init);
								msleep(20);
							}

							if(gamma_value ==2)
							{
								if (cur_acl != 12)
								{
									s6e63m0_panel_send_sequence(ACL_cutoff_set[1]); //set 12% ACL
									cur_acl = 12;
									//printk(" ACL_cutoff_set Percentage : 12!!\n");
								}
							}
							else if(gamma_value ==3)
							{
								if (cur_acl != 22)
								{
									s6e63m0_panel_send_sequence(ACL_cutoff_set[2]); //set 22% ACL
									cur_acl = 22;
									//printk(" ACL_cutoff_set Percentage : 22!!\n");
								}
							}
							else if(gamma_value ==4)
							{
								if (cur_acl != 30)
								{
									s6e63m0_panel_send_sequence(ACL_cutoff_set[3]); //set 30% ACL
									cur_acl = 30;
									//printk(" ACL_cutoff_set Percentage : 30!!\n");
								}
							}
							else if(gamma_value ==5)
							{
								if (cur_acl != 35)
								{
									s6e63m0_panel_send_sequence(ACL_cutoff_set[4]); //set 35% ACL
									cur_acl = 35;
									//printk(" ACL_cutoff_set Percentage : 35!!\n");
								}
							}
							else
							{
								if(cur_acl !=40)
								{
									s6e63m0_panel_send_sequence(ACL_cutoff_set[5]); //set 40% ACL
									cur_acl = 40;
									//printk(" ACL_cutoff_set Percentage : 40!!\n");
								}
							}
						}
					}
				#endif

					if(on_19gamma)
						s6e63m0_panel_send_sequence(p19Gamma_set[gamma_value]);
					else
						s6e63m0_panel_send_sequence(p22Gamma_set[gamma_value]);

					//printk("#################backlight end##########################\n");

					break;
				}
			}

			current_gamma_value = gamma_value;
		}
	}

	return 0;
}

static int s5p_bl_get_brightness(struct backlilght_device* bd)
{
    FBDBG_TRACE() ;
	return bd_brightness;
}

static struct backlight_ops s5p_bl_ops = {
	.update_status = s5p_bl_update_status,
	.get_brightness = s5p_bl_get_brightness,
};

#endif /* FORTE BD60910 */
////////////////////////////////////////////////////////////////////////////////
static int __init s6d16a0x_probe(struct spi_device *spi)
{
	int ret;

    FBDBG_TRACE() ;

	spi->bits_per_word = 9;
	ret = spi_setup(spi);
	lcd.g_spi = spi;
	lcd.lcd_dev = lcd_device_register("s5p_lcd",&spi->dev,&lcd,&s5p_lcd_ops);
#if !defined(CONFIG_FB_S3C_S6D16A0X)
	lcd.bl_dev = backlight_device_register("s5p_bl",&spi->dev,&lcd,&s5p_bl_ops);
	lcd.bl_dev->props.max_brightness = 255;
#endif
	dev_set_drvdata(&spi->dev,&lcd);

    SetLDIEnabledFlag(1);

    //s6d16a0x_ldi_init() ; // TODO:FORTE_TEST
    //if (ret)
    //   printk(KERN_ERR "s6d16a0x not found\n") ;

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

#ifdef ACL_ENABLE //ACL On,Off
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

	if (ret < 0){
		printk(KERN_ERR "%s::%d-> s6d16a0x probe failed Err=%d\n",__func__,__LINE__,ret);
		return 0;
	}
	printk(KERN_INFO "%s::%d->s6d16a0x probed successfuly(ret=%d)\n",__func__,__LINE__, ret);
	return ret;
}

#ifdef CONFIG_PM // add by ksoo (2009.09.07)
int s6d16a0x_suspend(struct platform_device *pdev, pm_message_t state)
{
    FBDBG_TRACE() ;
	s6d16a0x_ldi_disable();
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
int s6d16a0x_resume(struct platform_device *pdev, pm_message_t state)
{
    FBDBG_TRACE() ;
	s6d16a0x_ldi_init();
	s6d16a0x_ldi_enable();

	return 0;
}
#endif

////////////////////////////////////////////////////////////////////////////////
static struct spi_driver s6d16a0x_driver = {
	.driver = {
		.name	= "s6d16a0x",
		.owner	= THIS_MODULE,
	},
	.probe		= s6d16a0x_probe,
	.remove		= __exit_p(s6d16a0x_remove),
//#ifdef CONFIG_PM
//	.suspend		= s6d16a0x_suspend,
//	.resume		= s6d16a0x_resume,
//#else
	.suspend		= NULL,
	.resume		= NULL,
//#endif
};

////////////////////////////////////////////////////////////////////////////////
static int __init s6d16a0x_init(void)
{
    FBDBG_TRACE() ;
	return spi_register_driver(&s6d16a0x_driver);
}

////////////////////////////////////////////////////////////////////////////////
static void __exit s6d16a0x_exit(void)
{
    FBDBG_TRACE() ;
	spi_unregister_driver(&s6d16a0x_driver);
}

module_init(s6d16a0x_init);
module_exit(s6d16a0x_exit);


MODULE_AUTHOR("SAMSUNG");
MODULE_DESCRIPTION("S6D16A0X driver");
MODULE_LICENSE("GPL");
