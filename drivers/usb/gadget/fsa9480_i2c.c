#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <plat/pm.h>
#include <asm/irq.h>
#include <linux/delay.h>
#include <plat/gpio-cfg.h>
#include <mach/regs-gpio.h>
#include <mach/regs-clock.h>
#include <mach/param.h>
#include "fsa9480_i2c.h"
#include <linux/syscalls.h>
#include <linux/fcntl.h>
#include <asm/uaccess.h>
#include <linux/usb/composite.h>
#include <linux/usb/gadget.h>
#include <mach/max8998_function.h>
#include <linux/switch.h>
#include <linux/input.h>


#define _SUPPORT_SAMSUNG_AUTOINSTALLER_


extern void otg_phy_init(void);
extern void otg_phy_off(void);

extern struct device *switch_dev;
extern int askonstatus;
extern int inaskonstatus;
extern int BOOTUP;
extern unsigned int HWREV;

static struct line_status_t g_ls; 

static int g_tethering;
static int g_dock = DOCK_REMOVED;
static int g_default_ESN_status = 1;

int oldusbstatus=0;

int mtp_mode_on = 0;

u8 MicroTAstatus=0;

struct i2c_driver fsa9480_i2c_driver;
static struct i2c_client *fsa9480_i2c_client = NULL;

struct fsa9480_state {
	struct i2c_client *client;
};

static struct i2c_device_id fsa9480_id[] = {
	{"fsa9480", 0},
	{}
};

//static u8 fsa9480_device1 = 0, fsa9480_device2 = 0, fsa9480_adc = 0;
//int usb_path = 0;
int log_via_usb = LOG_USB_DISABLE;

int mtp_power_off = 0;

static wait_queue_head_t usb_detect_waitq;
static struct workqueue_struct *fsa9480_workqueue;
static struct work_struct fsa9480_work;
struct switch_dev indicator_dev;
struct switch_dev switch_dock_detection = {
		.name = "dock",
};
struct delayed_work micorusb_init_work;

#define DOCK_KEY_MASK	0x18
#define DOCK_KEY_SHIFT  3	
typedef enum
{
	DOCK_KEY_VOLUMEUP = 0,
	DOCK_KEY_VOLUMEDOWN,
	DOCK_KEY_MAX,
} dock_key_type;
static unsigned int dock_keys[DOCK_KEY_MAX] = 
{
	KEY_VOLUMEDOWN,
	KEY_VOLUMEUP,
};
static const char * dock_keys_string[DOCK_KEY_MAX] = 
{
	"KEY_VOLUMEDOWN",
	"KEY_VOLUMEUP",
};
static struct input_dev * dock_key_input_dev;

extern int currentusbstatus;
extern int usb_mtp_select(int disable);
extern int usb_switch_select(int enable);
extern int askon_switch_select(int enable);
extern unsigned int charging_mode_get(void);

int samsung_kies_mtp_mode_flag;
void FSA9480_Enable_CP_USB(u8 enable);
void FSA9480_OpenAllSwitch(void);

u8 FSA9480_Get_JIG_Status(void)
{
	struct line_status_t *ls = &g_ls;
	return ( (ls->jig == JIG_USB_ON )
		| (ls->jig == JIG_USB_OFF)
		| (ls->jig == JIG_UART_OFF)
	)?1:0;
}
EXPORT_SYMBOL(FSA9480_Get_JIG_Status);

u8 FSA9480_Get_USB_Status(void)
{
	struct line_status_t *ls = &g_ls;
	return (ls->line == LINE_USB)?1:0;

}

//GPIO help fuction
#define GPIO_HIGH 1
#define GPIO_LOW 0
static inline void _gpio_out(unsigned int pin, unsigned int level)
{
	s3c_gpio_cfgpin(pin, S3C_GPIO_OUTPUT);
	s3c_gpio_setpin(pin, level);
}

static int fsa9480_read(struct i2c_client *client, u8 reg, u8 *data)
{
	int ret;

	ret = i2c_smbus_read_byte_data(client, reg);
	if (ret < 0)
		return -EIO;

	*data = ret & 0xff;
	return 0;
}

static int fsa9480_write(struct i2c_client *client, u8 reg, u8 data)
{
	return i2c_smbus_write_byte_data(client, reg, data);
}

#ifdef _SUPPORT_SAMSUNG_AUTOINSTALLER_
bool IsKiesCurrentUsbStatus(void)
{
	if( currentusbstatus == USBSTATUS_SAMSUNG_KIES ) {
		return true;
	}

	return false;
}
EXPORT_SYMBOL(IsKiesCurrentUsbStatus);
#endif

void ap_usb_power_on(int set_vaue)
{
	byte reg_value=0;
	byte reg_address=0x0D;

	if(set_vaue){
		Get_MAX8998_PM_ADDR(reg_address, &reg_value, 1); // read 0x0D register
		reg_value = reg_value | (0x1 << 7);
		Set_MAX8998_PM_ADDR(reg_address,&reg_value,1);
		printk("[ap_usb_power_on]AP USB Power ON, askon: %d, mtp : %d\n",askonstatus,mtp_mode_on);
		if(mtp_mode_on == 1) {
			samsung_kies_mtp_mode_flag = 1;
			printk("[ap_usb_power_on] samsung_kies_mtp_mode_flag:%d, mtp:%d\n", samsung_kies_mtp_mode_flag, mtp_mode_on);
		}
		else {
			samsung_kies_mtp_mode_flag = 0;
			printk("[ap_usb_power_on]AP samsung_kies_mtp_mode_flag%d, mtp:%d\n",samsung_kies_mtp_mode_flag, mtp_mode_on);
		}
	}
	else{
		Get_MAX8998_PM_ADDR(reg_address, &reg_value, 1); // read 0x0D register
		reg_value = reg_value & ~(0x1 << 7);
		Set_MAX8998_PM_ADDR(reg_address,&reg_value,1);
		printk("[ap_usb_power_on]AP USB Power OFF, askon: %d, mtp : %d\n",askonstatus,mtp_mode_on);
	}
}
#if 0
static void ifcon_path_ctl_bb01(Usb_Uart_Sw_Mode_type mode)
{
	switch(mode){
		case AP_USB_MODE:
			_gpio_out(GPIO_USB_SW, GPIO_HIGH); //LTE USB SW
			_gpio_out(GPIO_USB_SW_EN, GPIO_HIGH); 
			FSA9480_Enable_CP_USB(0); // auto switch
				
			break;
		case AP_UART_MODE:
			_gpio_out(GPIO_USB_SW, GPIO_HIGH); //LTE USB SW
			_gpio_out(GPIO_USB_SW_EN, GPIO_HIGH);
			_gpio_out(GPIO_UART_SEL, GPIO_HIGH ); 	// UART_SEL MAX8998
			FSA9480_Enable_CP_USB(0);		// AUTO Switch
			break;
/*		case CP_USB_MODE:
			_gpio_out(GPIO_USB_SW, GPIO_HIGH); //LTE USB SW
			_gpio_out(GPIO_USB_SW_EN, GPIO_HIGH);
			FSA9480_Enable_CP_USB(1); //Manual set to CP port
			break;
*/
		case CP_UART_MODE:
			_gpio_out(GPIO_USB_SW, GPIO_HIGH); //LTE USB SW
			_gpio_out(GPIO_USB_SW_EN, GPIO_HIGH);
			_gpio_out(GPIO_UART_SEL, GPIO_LOW); 	// UART_SEL MAX8998
			FSA9480_Enable_CP_USB(0);
			break;
		case LTE_USB_MODE:
			_gpio_out(GPIO_USB_SW, GPIO_LOW); //LTE USB SW
			_gpio_out(GPIO_USB_SW_EN, GPIO_LOW); 
						// Don't care 
			break;
		case LTE_UART_MODE:
			_gpio_out(GPIO_USB_SW, GPIO_HIGH); //LTE USB SW
			_gpio_out(GPIO_USB_SW_EN, GPIO_HIGH);
			FSA9480_Enable_CP_USB(1);		// AUTO Switch
			break;
		default:
			break;
	}
}
#endif

// swhich line path
//

static void inline _path_ctl_usb(struct line_status_t *line_status)
{
	struct line_status_t *ls = line_status;
	LOGD("usb %x\n", ls->usb_path);
	switch(ls->usb_path){
		case PATH_CP:
			_gpio_out(GPIO_USB_SW, GPIO_HIGH); //LTE USB SW
			_gpio_out(GPIO_UART_SEL1, GPIO_HIGH); // CP SEL
			FSA9480_Enable_CP_USB(1); //Manual set to CP port
			break;
		case PATH_AP:
			_gpio_out(GPIO_USB_SW, GPIO_HIGH); //LTE USB SW
			FSA9480_Enable_CP_USB(0); // auto switch
			break;
		case PATH_LTE:
			_gpio_out(GPIO_USB_SW, GPIO_LOW); //LTE USB SW
			FSA9480_OpenAllSwitch();
			break;
		default:
			break;
	}
}

static void inline _path_ctl_uart(struct line_status_t *line_status)
{
	struct line_status_t *ls = line_status;
	LOGD("uart %x\n", ls->uart_path);
	switch(ls->uart_path){
		case PATH_CP:
			_gpio_out(GPIO_USB_SW, GPIO_HIGH); //LTE USB SW
			_gpio_out(GPIO_UART_SEL1, GPIO_LOW); // CP SEL
			FSA9480_Enable_CP_USB(1);		// Manual Switch
			break;
		case PATH_AP:
			_gpio_out(GPIO_USB_SW, GPIO_HIGH); //LTE USB SW
			_gpio_out(GPIO_UART_SEL, GPIO_HIGH ); 	// UART_SEL MAX8998
			FSA9480_Enable_CP_USB(0);		// AUTO Switch
			break;
		case PATH_LTE:
			_gpio_out(GPIO_USB_SW, GPIO_HIGH); //LTE USB SW
			_gpio_out(GPIO_UART_SEL, GPIO_LOW ); 	// UART_SEL MAX8998
			FSA9480_Enable_CP_USB(0);		// AUTO Switch
			break;
		default:
			break;
	}
}

static void inline _path_ctl_usb_hwrev04(struct line_status_t *line_status)
{
	struct line_status_t *ls = line_status;
	LOGD("usb %x\n", ls->usb_path);
	switch(ls->usb_path){

		case PATH_CP:
			_gpio_out(GPIO_USB_SW, GPIO_HIGH); //LTE USB SW
			_gpio_out(HWREV04_USB_SW_EN, GPIO_HIGH);
			FSA9480_Enable_CP_USB(1);
			break;
		case PATH_AP:
			_gpio_out(HWREV04_USB_SW_EN, GPIO_HIGH); //LTE USB SW
			_gpio_out(GPIO_USB_SW, GPIO_HIGH); //LTE USB SW
			FSA9480_Enable_CP_USB(0); // auto switch
			break;
		case PATH_LTE:
			_gpio_out(HWREV04_USB_SW_EN, GPIO_HIGH); 
			_gpio_out(GPIO_USB_SW, GPIO_LOW); //LTE USB SW
			break;
		default:
			break;
	}
}

static void inline _path_ctl_uart_hwrev04(struct line_status_t *line_status)
{
	struct line_status_t *ls = line_status;
	LOGD("uart %x\n", ls->uart_path);
	switch(ls->uart_path){
		case PATH_CP:
			_gpio_out(GPIO_USB_SW, GPIO_HIGH); //LTE USB SW
			_gpio_out(HWREV04_USB_SW_EN, GPIO_HIGH);
			_gpio_out(GPIO_UART_SEL, GPIO_LOW); 	// UART_SEL MAX8998
			FSA9480_Enable_CP_USB(0);
			break;
		case PATH_AP:
			_gpio_out(HWREV04_USB_SW_EN, GPIO_HIGH);
			_gpio_out(GPIO_USB_SW, GPIO_HIGH); //LTE USB SW
			_gpio_out(GPIO_UART_SEL, GPIO_HIGH ); 	// UART_SEL MAX8998
			FSA9480_Enable_CP_USB(0);		// AUTO Switch
			break;
		case PATH_LTE:
//			_gpio_out(GPIO_USB_SW, GPIO_HIGH); //LTE USB SW
//			_gpio_out(HWREV04_USB_SW_EN, GPIO_HIGH);
//			FSA9480_Enable_CP_USB(1);		// AUTO Switch
			break;
		default:
			break;
	}
}

static void line_path_ctl(struct line_status_t *line_status)
{
	struct line_status_t *ls = line_status;

	LOGD("HWREV : 0x%x\n", HWREV);
	switch(ls->line){
		case LINE_OFF:
			goto CleanUp;

		case LINE_USB:
			if(HWREV==04)
				_path_ctl_usb_hwrev04(ls);
			else
				_path_ctl_usb(ls);
			break;
		case LINE_UART:
			if(HWREV==04)
				_path_ctl_uart_hwrev04(ls);
			else
				_path_ctl_uart(ls);
			break;
		case LINE_DOCK:
			//TODO:
			break;
		default:
			break;
	}

CleanUp:
	return;
}

// below fuction use onedram drvier for LTE dump
#define SWITCH_PDA 1
#define SWITCH_MODEM 2
#define SWITCH_LTE 3

void usb_switch_mode(int sel) 
{
	struct line_status_t *ls = &g_ls;

	switch(sel){
		case SWITCH_PDA:
			ls->usb_path = PATH_AP;
			break;
		case SWITCH_MODEM:
			ls->usb_path = PATH_CP;
			break;
		case SWITCH_LTE:
			ls->usb_path = PATH_LTE;
			break;
	}
	LOGD("usb_path:%x\n",ls->usb_path);
	line_path_ctl(ls);
}
EXPORT_SYMBOL(usb_switch_mode);


static ssize_t factoryreset_value_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	struct line_status_t *ls= &g_ls;
	if(strncmp(buf, "FACTORYRESET", 12) == 0 || strncmp(buf, "factoryreset", 12) == 0){
		ls->factoryreset = 0xAE;
	}

	return size;
}
static DEVICE_ATTR(FactoryResetValue, S_IRUGO |S_IWUGO | S_IRUSR | S_IWUSR, NULL, factoryreset_value_store);


/* for sysfs control (/sys/class/sec/switch/usb_sel) */
static ssize_t usb_sel_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct line_status_t *ls = &g_ls;
	char *p = buf;
	char *chip_sel[] = {"MODEM", "PDA", "LTEMODEM"};

	p+=sprintf(p, "USB Switch : %s\n", chip_sel[ls->usb_path]);
//	LOGD("%s\n",buf);

	return p-buf;
}

static inline void _get_switch_sel(unsigned int *select)
{
	if (sec_get_param_value)
		sec_get_param_value(__SWITCH_SEL, select);
	LOGD("param switch sel 0x%x\n", *select);
}

static inline void _set_switch_sel(unsigned int select)
{
	if (sec_set_param_value)
		sec_set_param_value(__SWITCH_SEL, &select);
	LOGD("param switch sel 0x%x\n", select);
}
void param_load_path(struct line_status_t *ls)
{
	int i=0, bit_mask=(1<<USB_SAMSUNG_KIES_BIT);

	_get_switch_sel(&ls->switch_sel);

	ls->usb_path = (ls->switch_sel & USB_SEL_MASK)>>USB_SEL_BIT;
	ls->uart_path = (ls->switch_sel & UART_SEL_MASK)>>UART_SEL_BIT;
	ls->uart_debug_enable = (ls->switch_sel & UART_DEBUG_MASK)>>UART_DEBUG_BIT;

	LOGD("usb_path(%d),uart_path(%d),uart_debug_enable(%d)\n", 
			ls->usb_path, ls->uart_path, ls->uart_debug_enable);

	//USB settings
	for(i=USB_BIT_START;i<USB_FUNC_MAX+USB_BIT_START;i++){
		if(ls->switch_sel & (0x1<<i)){
			ls->usb_func = i;
			LOGD("SWITCH_SEL USB %x\n",ls->usb_func);
			break;
		}
	}
}

void param_save_path(struct line_status_t *ls)
{
	LOGD("uart(%x)usb(%x)usb(%x),uart_debug(%d)\n", 
			ls->uart_path, ls->usb_path, ls->usb_func, ls->uart_debug_enable);

	ls->switch_sel = ( (ls->uart_path << UART_SEL_BIT)
					   | (ls->usb_path << USB_SEL_BIT)
					   | B2M(ls->usb_func)
					   | (ls->uart_debug_enable << UART_DEBUG_BIT) );

	_set_switch_sel(ls->switch_sel);

	line_path_ctl(ls);
}

static ssize_t usb_sel_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct line_status_t *ls = &g_ls;

	LOGD("\n");
	LOGD("%s\n",buf);

	if(strncmp(buf, "PDA", 3) == 0 || strncmp(buf, "pda", 3) == 0)
	{
		ls->usb_path = PATH_AP;
	}
	if(strncmp(buf, "MODEM", 5) == 0 || strncmp(buf, "modem", 5) == 0)
	{
		ls->usb_path = PATH_CP;
	}
	if(strncmp(buf, "LTEMODEM", 8) == 0 || strncmp(buf, "ltemodem", 8) == 0)
	{
		ls->usb_path = PATH_LTE;
	}
	//path ctl
	//line_path_ctl(ls);

	//store param value
	param_save_path(ls);

	return size;
}
static DEVICE_ATTR(usb_sel, S_IRUGO |S_IWUGO | S_IRUSR | S_IWUSR, usb_sel_show, usb_sel_store);

/**********************************************************************
 *    Name         : usb_state_show()
 *    Description : for sysfs control (/sys/class/sec/switch/usb_state)
 *                        return usb state using fsa9480's device1 and device2 register
 *                        this function is used only when NPS want to check the usb cable's state.
 *    Parameter   :
 *                       
 *                       
 *    Return        : USB cable state's string
 *                        USB_STATE_CONFIGURED is returned if usb cable is connected
 ***********************************************************************/
static ssize_t usb_state_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct line_status_t *ls = &g_ls;
	LOGD("\n");
	sprintf(buf, "%s\n", (ls->fsa_status.cable_state & (CRB_JIG_USB<<8 | CRA_USB<<0 ))?"USB_STATE_CONFIGURED":"USB_STATE_NOTCONFIGURED");

	return sprintf(buf, "%s\n", buf);
} 


/**********************************************************************
 *    Name         : usb_state_store()
 *    Description : for sysfs control (/sys/class/sec/switch/usb_state)
 *                        noting to do.
 *    Parameter   :
 *                       
 *                       
 *    Return        : None
 *
 ***********************************************************************/
static ssize_t usb_state_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	LOGD("\n");
	return 0;
}

/*sysfs for usb cable's state.*/
static DEVICE_ATTR(usb_state, S_IRUGO |S_IWUGO | S_IRUSR | S_IWUSR, usb_state_show, usb_state_store);

static ssize_t uart_switch_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct line_status_t *ls = &g_ls;
	char *p = buf;
	char *chip_sel[] = {"MODEM", "PDA", "LTEMODEM"};
	//sprintf(buf, "%s[UART Switch] Current UART owner = PDA \n", buf);i

	p+=sprintf(p, "[UART Switch] Current UART owner = %s \n", chip_sel[ls->uart_path]);
	LOGD("%s\n",buf);

	return p-buf;
}

static ssize_t uart_switch_store(struct device *dev, struct device_attribute *attr,	const char *buf, size_t size)
{	
	struct line_status_t *ls = &g_ls;
	LOGD("%s\n",buf);

	if (strncmp(buf, "PDA", 3) == 0 || strncmp(buf, "pda", 3) == 0)	{		
		ls->uart_path = PATH_AP;
	}	

	if (strncmp(buf, "MODEM", 5) == 0 || strncmp(buf, "modem", 5) == 0) {		
		ls->uart_path = PATH_CP;
	}

	if (strncmp(buf, "LTEMODEM", 8) == 0 || strncmp(buf, "ltemodem", 8) == 0) {		
		ls->uart_path = PATH_LTE;
	}

	//store param value
	param_save_path(ls);

	return size;
}
static DEVICE_ATTR(uart_sel, S_IRUGO |S_IWUGO | S_IRUSR | S_IWUSR, uart_switch_show, uart_switch_store);


void FSA9480_ChangePathToAudio(u8 enable)
{
	struct i2c_client *client = fsa9480_i2c_client;
	u8 manualsw1;

	if(enable)
	{
		mdelay(10);
		fsa9480_write(client, REGISTER_MANUALSW1, 0x48);			

		mdelay(10);
		fsa9480_write(client, REGISTER_CONTROL, 0x1A);

		fsa9480_read(client, REGISTER_MANUALSW1, &manualsw1);
		printk("Fsa9480 ManualSW1 = 0x%x\n",manualsw1);
	}
	else
	{
		mdelay(10);
		fsa9480_write(client, REGISTER_CONTROL, 0x1E);	
	}
}
//EXPORT_SYMBOL(FSA9480_ChangePathToAudio);

void UsbMenuSelStore(int sel)
{	
	struct line_status_t *ls = &g_ls;
	unsigned int usb_func =0;

	switch(sel) {
		case USBSTATUS_UMS:
			usb_func = USB_UMS_BIT;
			break;
		case USBSTATUS_SAMSUNG_KIES:
			usb_func = USB_SAMSUNG_KIES_BIT;
			break;
		case USBSTATUS_MTPONLY:
			usb_func = USB_MTP_BIT;
			break;
		case USBSTATUS_ASKON:
			usb_func = USB_ASKON_BIT;
			break;
		case USBSTATUS_VTP:
			usb_func = USB_VTP_BIT;
			break;
		case USBSTATUS_ADB:
			usb_func = USB_ADB_BIT;
			break;
		default:
			goto CleanUp;
	}

	param_load_path(ls);
	ls->usb_func = usb_func;
	ls->switch_sel &= ~(USB_UMS_MASK|USB_MTP_MASK|USB_VTP_MASK|USB_ASKON_MASK|USB_SAMSUNG_KIES_MASK);
	ls->switch_sel |= B2M(ls->usb_func);

	LOGD("fsa %s SWITCH_SEL Store arg(%x) sel= 0x%x usb_func(%x)\n",__func__,sel, ls->switch_sel, ls->usb_func);

	param_save_path(ls);
CleanUp:
	return;
}
EXPORT_SYMBOL(UsbMenuSelStore);

static ssize_t UsbMenuSel_switch_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct line_status_t *ls = &g_ls;
	
    int usbstatus = B2S(ls->usb_func);

    if (ls->usb_path != PATH_AP) {
        return sprintf(buf, "[UsbMenuSel] MODEM\n");	
    }

#ifdef _SUPPORT_SAMSUNG_AUTOINSTALLER_
	usbstatus = currentusbstatus;
#else
    if (usbstatus == USBSTATUS_ASKON && inaskonstatus) {
        usbstatus = currentusbstatus;
    }
#endif	
	
	switch(usbstatus){
		case USBSTATUS_UMS:
			return sprintf(buf, "%s[UsbMenuSel] UMS\n", buf);	
#ifdef _SUPPORT_SAMSUNG_AUTOINSTALLER_
		case USBSTATUS_SAMSUNG_KIES:
			return sprintf(buf, "%s[UsbMenuSel] UMS_CDFS\n", buf);	
#else
		case USBSTATUS_SAMSUNG_KIES:
			return sprintf(buf, "%s[UsbMenuSel] ACM_MTP\n", buf);	
#endif			
		case USBSTATUS_MTPONLY:
			return sprintf(buf, "%s[UsbMenuSel] MTP\n", buf);	
		case USBSTATUS_ASKON:
			return sprintf(buf, "%s[UsbMenuSel] ASK\n", buf);	
		case USBSTATUS_VTP:
			return sprintf(buf, "%s[UsbMenuSel] VTP\n", buf);	
		case USBSTATUS_ADB:
			return sprintf(buf, "%s[UsbMenuSel] ACM_ADB_UMS\n", buf);
	}
}


static ssize_t UsbMenuSel_switch_store(struct device *dev, struct device_attribute *attr,	const char *buf, size_t size)
{		
	if (strncmp(buf, "KIES", 4) == 0)
	{
		UsbMenuSelStore(USBSTATUS_SAMSUNG_KIES);		
		usb_switch_select(USBSTATUS_SAMSUNG_KIES);
	}

	if (strncmp(buf, "MTP", 3) == 0)
	{
		UsbMenuSelStore(USBSTATUS_MTPONLY);					
		usb_switch_select(USBSTATUS_MTPONLY);
	}

	if (strncmp(buf, "UMS", 3) == 0)
	{
		UsbMenuSelStore(USBSTATUS_UMS);							
		usb_switch_select(USBSTATUS_UMS);
	}
#if !defined(CONFIG_ARIES_NTT) // disable tethering xmoondash
	if (strncmp(buf, "VTP", 3) == 0)
	{
		UsbMenuSelStore(USBSTATUS_VTP);							
		usb_switch_select(USBSTATUS_VTP);
	}
#endif
	if (strncmp(buf, "ASKON", 5) == 0)
	{		
		UsbMenuSelStore(USBSTATUS_ASKON);									
		usb_switch_select(USBSTATUS_ASKON);			
	}

	return size;
}

static DEVICE_ATTR(UsbMenuSel, S_IRUGO |S_IWUGO | S_IRUSR | S_IWUSR, UsbMenuSel_switch_show, UsbMenuSel_switch_store);


extern int inaskonstatus;
static ssize_t AskOnStatus_switch_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct line_status_t *ls = &g_ls;

	if(inaskonstatus || ls->usb_path != PATH_AP) 
		return sprintf(buf, "%s\n", "Blocking");
	else
		return sprintf(buf, "%s\n", "NonBlocking");
}


static ssize_t AskOnStatus_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{		
	return size;
}

static DEVICE_ATTR(AskOnStatus, S_IRUGO |S_IWUGO | S_IRUSR | S_IWUSR, AskOnStatus_switch_show, AskOnStatus_switch_store);


static ssize_t AskOnMenuSel_switch_show(struct device *dev, struct device_attribute *attr, char *buf)
{	
	return sprintf(buf, "%s[AskOnMenuSel] Port test ready!! \n", buf);	
}

static ssize_t AskOnMenuSel_switch_store(struct device *dev, struct device_attribute *attr,	const char *buf, size_t size)
{		
//	if (strncmp(buf, "KIES", 4) == 0)
//	{
//		askon_switch_select(USBSTATUS_SAMSUNG_KIES);
//	}

	if (strncmp(buf, "MTP", 3) == 0)
	{
		askon_switch_select(USBSTATUS_MTPONLY);
	}

	if (strncmp(buf, "UMS", 3) == 0)
	{
		askon_switch_select(USBSTATUS_UMS);
	}
#if !defined(CONFIG_ARIES_NTT) // disable tethering xmoondash
	if (strncmp(buf, "VTP", 3) == 0)
	{
		askon_switch_select(USBSTATUS_VTP);
	}
#endif
	return size;
}

static DEVICE_ATTR(AskOnMenuSel, S_IRUGO |S_IWUGO | S_IRUSR | S_IWUSR, AskOnMenuSel_switch_show, AskOnMenuSel_switch_store);


static ssize_t Mtp_switch_show(struct device *dev, struct device_attribute *attr, char *buf)
{	
	return sprintf(buf, "%s[Mtp] MtpDeviceOn \n", buf);	
}

static ssize_t Mtp_switch_store(struct device *dev, struct device_attribute *attr,	const char *buf, size_t size)
{
	struct line_status_t *ls = &g_ls;
	if (strncmp(buf, "Mtp", 3) == 0)
	{
		if(mtp_mode_on)
		{
			printk("[Mtp_switch_store]AP USB power on. \n");
#ifdef VODA
//			askon_switch_select(USBSTATUS_SAMSUNG_KIES);
#endif
			ap_usb_power_on(1);
		}
	}
	else if (strncmp(buf, "OFF", 3) == 0)
	{
		printk("[Mtp_switch_store]AP USB power off. \n");
		//usb_state = 0;
		ls->fsa_status.cable_state = 0;
		usb_mtp_select(1);
	}
	return size;
}

static DEVICE_ATTR(Mtp, S_IRUGO |S_IWUGO | S_IRUSR | S_IWUSR, Mtp_switch_show, Mtp_switch_store);


static int mtpinitstatus=0;
static ssize_t MtpInitStatusSel_switch_show(struct device *dev, struct device_attribute *attr, char *buf)
{	
	if(mtpinitstatus == 2)
		return sprintf(buf, "%s\n", "START");
	else
		return sprintf(buf, "%s\n", "STOP");
}

static ssize_t MtpInitStatusSel_switch_store(struct device *dev, struct device_attribute *attr,	const char *buf, size_t size)
{
	mtpinitstatus = mtpinitstatus + 1;

	return size;
}

static DEVICE_ATTR(MtpInitStatusSel, S_IRUGO |S_IWUGO | S_IRUSR | S_IWUSR, MtpInitStatusSel_switch_show, MtpInitStatusSel_switch_store);

void UsbIndicator(u8 state)
{
	LOGD("state(%d), indicator_dev->state(%d)\n", state, indicator_dev.state);

	switch_set_state(&indicator_dev, state); 
}

static ssize_t tethering_switch_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (g_tethering)
		return sprintf(buf, "1\n");
	else			
		return sprintf(buf, "0\n");
}

#define S2B(a) _s2b(a)
static int _s2b(unsigned int status)
{
	int i=0;

	for(i=0;i<USB_FUNC_MAX;i++)
		if(status == 0x1<<(i-1)){
			LOGD("to bit -%d\n", i+4);
			return (i+4);
		}
	LOGD("to bit, can't find it!!\n");
	return -1;
}

static ssize_t tethering_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int usbstatus;
	struct line_status_t *ls = &g_ls;
	usbstatus = FSA9480_Get_USB_Status();
	printk("usbstatus = 0x%x, currentusbstatus = 0x%x\n", usbstatus, currentusbstatus);

	if (strncmp(buf, "1", 1) == 0)
	{
		printk("tethering On\n");

		g_tethering = 1;
		//ls->usb_func_prev = ls->usb_func;
		usb_switch_select(USBSTATUS_VTP);
		UsbIndicator(0);
	}
	else if (strncmp(buf, "0", 1) == 0)
	{
		printk("tethering Off\n");

		g_tethering = 0;
		//usb_switch_select(ls->usb_func_prev);
		usb_switch_select(oldusbstatus);
		if(usbstatus)
			UsbIndicator(1);
	}
	ls->usb_func = S2B(currentusbstatus);

	return size;
}

static DEVICE_ATTR(tethering, S_IRUGO |S_IWUGO | S_IRUSR | S_IWUSR, tethering_switch_show, tethering_switch_store);


static ssize_t dock_switch_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (g_dock == DOCK_REMOVED)
		return sprintf(buf, "0\n");
	else if (g_dock == HOME_DOCK_INSERTED)
		return sprintf(buf, "1\n");
	else if (g_dock == CAR_DOCK_INSERTED)
		return sprintf(buf, "2\n");
    else
        return sprintf(buf, "0\n");
}

static ssize_t dock_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	if (strncmp(buf, "0", 1) == 0)
	{
		printk("remove dock\n");
		g_dock = DOCK_REMOVED;
	}
	else if (strncmp(buf, "1", 1) == 0)
	{
		printk("home dock inserted\n");
		g_dock = HOME_DOCK_INSERTED;
	}
	else if (strncmp(buf, "2", 1) == 0)
	{
		printk("car dock inserted\n");
		g_dock = CAR_DOCK_INSERTED;
	}

	return size;
}

static DEVICE_ATTR(dock, S_IRUGO |S_IWUGO | S_IRUSR | S_IWUSR, dock_switch_show, dock_switch_store);

static ssize_t DefaultESNStatus_switch_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    if (g_default_ESN_status) {    
        return sprintf(buf, "DefaultESNStatus : TRUE\n");
    }
    else{
        return sprintf(buf, "DefaultESNStatus : FALSE\n");        
    }
}

static ssize_t DefaultESNStatus_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{		
    if ((strncmp(buf, "TRUE", 4) == 0) ||(strncmp(buf, "true", 4) == 0)) {
        g_default_ESN_status = 1;
    }
    if ((strncmp(buf, "FALSE", 5) == 0) ||(strncmp(buf, "false", 5) == 0)) {
        g_default_ESN_status = 0;
    }

    return size;
}

static DEVICE_ATTR(DefaultESNStatus, S_IRUGO |S_IWUGO | S_IRUSR | S_IWUSR, DefaultESNStatus_switch_show, DefaultESNStatus_switch_store);


static int askinitstatus=0;
static ssize_t AskInitStatusSel_switch_show(struct device *dev, struct device_attribute *attr, char *buf)
{	
	if(askinitstatus == 2)
		return sprintf(buf, "%s\n", "START");
	else
		return sprintf(buf, "%s\n", "STOP");
}

static ssize_t AskInitStatusSel_switch_store(struct device *dev, struct device_attribute *attr,	const char *buf, size_t size)
{
	askinitstatus = askinitstatus + 1;

	return size;
}

static DEVICE_ATTR(AskInitStatusSel, S_IRUGO |S_IWUGO | S_IRUSR | S_IWUSR, AskInitStatusSel_switch_show, AskInitStatusSel_switch_store);

static ssize_t get_SwitchingInitValue(struct device *dev, struct device_attribute *attr,	char *buf)
{
	struct line_status_t *ls = &g_ls;
	char *p = buf;
	char chip_select[]={'C', 'A', 'L'};

	p+= snprintf(p, 12, "%cPUSB%cPUART\0", chip_select[ls->usb_path], chip_select[ls->uart_path]);

	return p-buf; 
}

static DEVICE_ATTR(SwitchingInitValue, S_IRUGO, get_SwitchingInitValue, NULL);

static ssize_t uart_debug_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct line_status_t *ls = &g_ls;
	char *p = buf;
	char *chip_sel[] = {"DISABLE", "ENABLE"};

	p+=sprintf(p, "%s\n", chip_sel[ls->uart_debug_enable]);
	LOGD("%s\n",buf);

	return p-buf;
}

static ssize_t uart_debug_store(struct device *dev, struct device_attribute *attr,	const char *buf, size_t size)
{	
	struct line_status_t *ls = &g_ls;
	LOGD("%s\n",buf);

	if (strncmp(buf, "DISABLE", 7) == 0 || strncmp(buf, "disable", 7) == 0) {		
		ls->uart_debug_enable = 0;
	}
	else if (strncmp(buf, "ENABLE", 6) == 0 || strncmp(buf, "enable", 6) == 0)	{		
		ls->uart_debug_enable = 1;
	}

	//store param value
	param_save_path(ls);

	return size;
}

static DEVICE_ATTR(UartDebug, S_IRUGO |S_IWUGO | S_IRUSR | S_IWUSR, uart_debug_show, uart_debug_store);

#ifdef _SUPPORT_SAMSUNG_AUTOINSTALLER_
static int kies_status = 0;
static ssize_t KiesStatus_switch_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	if(kies_status == 1)
		return sprintf(buf, "%s\n", "START");
	else if( kies_status == 2)
		return sprintf(buf, "%s\n", "STOP");
    else
        return sprintf(buf, "%s\n", "INIT");
}

static ssize_t KiesStatus_switch_store(struct device *dev, struct device_attribute *attr,	const char *buf, size_t size)
{
    printk("buf=%s\n", buf);

    if (strncmp(buf, "START", 5) == 0 )
    {
        kies_status = 1;
    }
	else if (strncmp(buf, "STOP", 4) == 0)
	{
	    kies_status = 2;
        UsbIndicator(2);
	}
    else if (strncmp(buf, "INIT", 4) == 0 )
    {
        kies_status = 0;
    }

    return size;
}

static DEVICE_ATTR(KiesStatus, S_IRUGO |S_IWUGO | S_IRUSR | S_IWUSR, KiesStatus_switch_show, KiesStatus_switch_store);
#endif /* _SUPPORT_SAMSUNG_AUTOINSTALLER_ */

//TODO:
// check the PMIC codes??
//
int  FSA9480_PMIC_CP_USB(void)
{
	int usb_sel = 0;
	unsigned int switch_sel=0;

	if (sec_get_param_value)
		sec_get_param_value(__SWITCH_SEL, &switch_sel);

	usb_sel = switch_sel & (int)(USB_SEL_MASK);

	return usb_sel;
}


int check_reg=0;
void FSA9480_Enable_CP_USB(u8 enable)
{
	struct i2c_client *client = fsa9480_i2c_client;
	byte reg_value=0;
	byte reg_address=0x0D;
	int data;

	if(enable)
	{
		printk("[FSA9480_Enable_CP_USB] Enable CP USB\n");
		mdelay(10);
		Get_MAX8998_PM_ADDR(reg_address, &reg_value, 1); // read 0x0D register
		check_reg = reg_value;
		reg_value = ((0x2<<5)|reg_value);
		check_reg = reg_value;
		Set_MAX8998_PM_ADDR(reg_address,&reg_value,1);
		check_reg = reg_value;

		mdelay(10);
		fsa9480_write(client, REGISTER_MANUALSW1, 0x90);  // D+/- switching by Audio_L/R in HW04

		mdelay(10);
		fsa9480_write(client, REGISTER_CONTROL, 0x1A);	

		mdelay(10);
		fsa9480_read(client, REGISTER_MANUALSW1, (int *)&data);  // D+/- switching by Audio_L/R in HW04

		mdelay(10);
		fsa9480_read(client, REGISTER_CONTROL, (int *)&data);	

	}
	else
	{
		printk("[FSA9480_Enable_AP_USB] Enable AP USB\n");
		Get_MAX8998_PM_ADDR(reg_address, &reg_value, 1); // read 0x0D register

		if(askonstatus||mtp_mode_on)
			ap_usb_power_on(0);
		else
			ap_usb_power_on(1);
		mdelay(10);
		fsa9480_write(client, REGISTER_CONTROL, 0x1E);
		mdelay(10);
		fsa9480_read(client, REGISTER_CONTROL, (int *)&data);
	}
}


void FSA9480_Enable_SPK(u8 enable)
{
	struct i2c_client *client = fsa9480_i2c_client;
	byte reg_value=0;
	byte reg_address=0x0D;

	if(enable)
	{
		LOGD("FSA9480_Enable_SPK --- enable\n");
		msleep(10);
		Get_MAX8998_PM_ADDR(reg_address, &reg_value, 1); // read 0x0D register
		check_reg = reg_value;
		reg_value = ((0x2<<5)|reg_value);
		check_reg = reg_value;
		Set_MAX8998_PM_ADDR(reg_address,&reg_value,1);
		check_reg = reg_value;

		msleep(10);
		fsa9480_write(client, REGISTER_MANUALSW1, 0x90);	// D+/- switching by V_Audio_L/R in HW03
		msleep(10);
		fsa9480_write(client, REGISTER_CONTROL, 0x1A);	//manual switching

	}
}

void FSA9480_OpenAllSwitch(void)
{
	struct i2c_client *client = fsa9480_i2c_client;
	
	LOGD("FSA9480_OpenAllSwitch\n");
	
	/* Switch Open, AUTO Switch */
	fsa9480_write(client, REGISTER_CONTROL, 0x0E);	
}

static void dock_keys_input(dock_key_type key, int press) 
{
	if( key >= DOCK_KEY_MAX )
		return;

	input_report_key(dock_key_input_dev, dock_keys[key], press);
	input_sync(dock_key_input_dev);

	LOGD("key pressed(%d) [%s] \n", press, dock_keys_string[key]);
}

extern void askon_gadget_disconnect(void);
extern int s3c_usb_cable(int connected);

extern void vps_status_change(int status);
extern void car_vps_status_change(int status);
byte chip_error=0;

#ifdef CONFIG_WIRELESS_CHARGING
extern int s3c_get_wireless_status(void);
extern void s3c_cable_changed(void);
#endif

void FSA9480_ProcessDevice(u8 dev1, u8 dev2, u8 attach)
{
	struct line_status_t *ls = &g_ls;

	LOGD("(dev1 : 0x%x, dev2 : 0x%x)\n", dev1, dev2);

	if(!attach && !chip_error && (mtp_mode_on == 1))
		chip_error = 0xAE;

	//if not connected
	if(attach & FSA9480_INT1_DETACH){
		LOGD("cable was disconneted\n");
		// disconection actions
		ls->line = LINE_OFF;
		ls->jig = JIG_NOT_DET;
		inaskonstatus = 0;
		chip_error = 0;
		UsbIndicator(0);
		askon_gadget_disconnect();
		
		if( g_dock == HOME_DOCK_INSERTED || g_dock == CAR_DOCK_INSERTED )
		{
			switch_set_state(&switch_dock_detection,(int)DOCK_REMOVED);

			g_dock = DOCK_REMOVED;
			LOGD("DOCK --- DETACH\n");
		}
		else
		{
			LOGD("FSA9480_DEV_TY1_USB --- DETACH\n");
		}

#ifdef CONFIG_WIRELESS_CHARGING
		s3c_cable_changed();
#endif

		goto CleanUp;
	}

    if( attach & (FSA9480_INT1_KP|FSA9480_INT1_LKP|FSA9480_INT1_LKR) )
    {
		u8 button2;
		bool is_press = 0;
		bool is_longkey = 0;
		struct i2c_client * client = fsa9480_i2c_client;

		/* read button register */ 
		fsa9480_read(client, REGISTER_BUTTON2, &button2);        

		if( attach & FSA9480_INT1_KP ) 
		{
			is_longkey = 0;

			LOGD("KEY PRESS\n");
		}
		else
		{
			is_longkey = 1;

			if( attach & FSA9480_INT1_LKP )
			{
				is_press = 1;

				LOGD("LONG KEY PRESS\n");
			}
			else
			{
				is_press = 0;

				LOGD("LONG KEY RELEASE\n");
			}
		}

    	if( g_dock == HOME_DOCK_INSERTED )
    	{
			u8 key_mask;
    		int index = 0;
			
			key_mask = (button2 & DOCK_KEY_MASK) >> DOCK_KEY_SHIFT;

			LOGD("key_mask 0x%x \n", key_mask);

    		while(key_mask) 
    		{
    			if( key_mask & 0x1 )
				{
					if( is_longkey )
					{
    					dock_keys_input(index, is_press);
					}
					else
					{
						dock_keys_input(index, 1);
						dock_keys_input(index, 0);
					}
				}

    			key_mask >>= 1;
    			index++;
    		}

			goto CleanUp;
    	}    
    }

	if(dev1)
	{
		switch(dev1)
		{
			case FSA9480_DEV_TY1_AUD_TY1:
			{	
				struct i2c_client *client = fsa9480_i2c_client;
				u8 interrupmask1_data;
				
				LOGD("Audio Type1 ");

				if (attach & FSA9480_INT1_ATTACH) {
					LOGD("FSA9480_enable_desk\n");
#ifndef CONFIG_PRESTO
					switch_set_state(&switch_dock_detection,(int)HOME_DOCK_INSERTED);
#endif
					g_dock = HOME_DOCK_INSERTED;
					ls->line = LINE_DOCK;
					ls->jig = JIG_NOT_DET;
				}
			}
				break;

			case FSA9480_DEV_TY1_AUD_TY2:
				LOGD("Audio Type2 ");
				break;

			case FSA9480_DEV_TY1_USB:
				LOGD("FSA9480_DEV_TY1_USB --- ATTACH\n");
				ls->line = LINE_USB;
				ls->jig = JIG_NOT_DET;
				log_via_usb = LOG_USB_ENABLE;
#ifdef CONFIG_WIRELESS_CHARGING
				s3c_cable_changed();
#endif
				if(!askonstatus)
					UsbIndicator(1);
				else
					inaskonstatus = 0;				
				break;

			case FSA9480_DEV_TY1_UART:
				LOGD("UART\ni");
				ls->line = LINE_UART;
				ls->jig = JIG_NOT_DET;
				break;

			case FSA9480_DEV_TY1_CAR_KIT:
				LOGD("Carkit\n");
				break;

			case FSA9480_DEV_TY1_USB_CHG:
				LOGD("USB\n");
				break;

			case FSA9480_DEV_TY1_DED_CHG:
				LOGD("Dedicated Charger ATTACH\n");
#ifdef CONFIG_WIRELESS_CHARGING
				s3c_cable_changed();
#endif
				break;

			case FSA9480_DEV_TY1_USB_OTG:
				LOGD("USB OTG\n");
				break;

			default:
				LOGD("Unknown device\n");
				break;
		}

	}

	if(dev2)
	{
		switch(dev2)
		{
			case FSA9480_DEV_TY2_JIG_USB_ON:
				LOGD("JIG USB ON attach or detach: %d",attach);
				ls->line = LINE_USB;
				ls->jig = JIG_USB_ON;
				if(!askonstatus)
					UsbIndicator(1);
				else
					inaskonstatus = 0;				
				break;

			case FSA9480_DEV_TY2_JIG_USB_OFF:
				LOGD("FSA9480_DEV_TY2_JIG_USB_OFF --- ATTACH\n");
				ls->line = LINE_USB;
				ls->jig = JIG_USB_OFF;
				if(!askonstatus)
					UsbIndicator(1);
				else
					inaskonstatus = 0;
				break;

			case FSA9480_DEV_TY2_JIG_UART_ON:
				LOGD("FSA9480_DEV_TY2_JIG_UART_ON\n");
				ls->line = LINE_UART;
				ls->jig = JIG_UART_ON;
				
				if(attach & FSA9480_INT1_ATTACH)
				{
					LOGD("FSA9480_DEV_TY2_JIG_UART_ON --- ATTACH\n");
					if( !g_default_ESN_status) {
#ifndef CONFIG_PRESTO
						switch_set_state(&switch_dock_detection, (int)CAR_DOCK_INSERTED);
#endif
						g_dock = CAR_DOCK_INSERTED;
					}
				}
				
				LOGD("JIG UART ON\n");
				break;

			case FSA9480_DEV_TY2_JIG_UART_OFF:
				LOGD("FSA9480_DEV_TY2_JIG_UART_OFF --- ATTACH\n");
				ls->line = LINE_UART;
				ls->jig = JIG_UART_OFF;
				break;

			case FSA9480_DEV_TY2_PDD:
				LOGD("PPD \n");
				break;

			case FSA9480_DEV_TY2_TTY:
				LOGD("TTY\n");
				break;

			case FSA9480_DEV_TY2_AV:
				LOGD("AudioVideo\n");
				
				if(attach & FSA9480_INT1_ATTACH)
				{
					LOGD("FSA9480_enable_car_dock \n");
					if( !g_default_ESN_status) {
#ifndef CONFIG_PRESTO
						switch_set_state(&switch_dock_detection, (int)CAR_DOCK_INSERTED);
#endif
						g_dock = CAR_DOCK_INSERTED;
						ls->line = LINE_DOCK;
						ls->jig = JIG_NOT_DET;
					}
				}
				break;

			default:
				LOGD("Unknown device\n");
				break;
		}
	}

	if((attach == FSA9480_INT1_ATTACH) && (chip_error == 0xAE) && (mtp_mode_on == 1)){
		ap_usb_power_on(1);
		chip_error = 0;
	}


	// Cable Path control
	line_path_ctl(ls);

CleanUp:
	return;

}

void fsa9480_interrupt_init(void);
void fsa9480_chip_init(void);

//void FSA9480_ReadIntRegister(struct work_struct *work)
// - FSA9480 irq handler functions
//
void FSA9480_ReadIntRegister(struct work_struct *work)
{
	struct i2c_client *client = fsa9480_i2c_client;
	u8 interrupt1 ,interrupt2 ,device1, device2, temp;
	struct line_status_t *ls=&g_ls;
	u8 interrupmask1_data;
    u8 control;

	LOGD("[FSA9480] %s\n", __func__);

	fsa9480_read(client, REGISTER_INTERRUPT1, &interrupt1);
	msleep(5);

	fsa9480_read(client, REGISTER_INTERRUPT2, &interrupt2);
	msleep(5);

	fsa9480_read(client, REGISTER_DEVICETYPE1, &device1);
	msleep(5);

	fsa9480_read(client, REGISTER_DEVICETYPE2, &device2);
    msleep(5);

    fsa9480_read(client, REGISTER_CONTROL, &control);

    if( control == 0x1F ) {
        printk("FSA9480 chip reset happened.\n");

        /*clear interrupt mask register*/
        fsa9480_write(client, REGISTER_CONTROL, control & ~INT_MASK);

        fsa9480_chip_init();

         printk("FSA9480 initialization finished.\n");
        
        goto __end__;
    }

#ifdef CONFIG_WIRELESS_CHARGING
	if (HWREV == 9) {
		if ((interrupt1 & FSA9480_INT1_ATTACH) && (device1 & FSA9480_DEV_TY1_USB)
			&& (gpio_get_value(GPIO_WC_DETECT) == 0))
		{
			printk(KERN_EMERG "%s: wireless charger is detected (wc_detect = %d)!!! skip ISR...\n",
				__func__, gpio_get_value(GPIO_WC_DETECT));
			goto __end__;
		}
	}
#endif

	ls->fsa_status.cable_state = (device2 <<8) | (device1<<0);

	if(interrupt1 & FSA9480_INT1_ATTACH)
	{
		if(device1 != FSA9480_DEV_TY1_DED_CHG){
			//LOGD("FSA9480_enable LDO8\n");
			s3c_usb_cable(1);
		}

		if(device1&FSA9480_DEV_TY1_CAR_KIT)
		{
			msleep(5);
			fsa9480_write(client, REGISTER_CARKITSTATUS, 0x02);

			msleep(5);
			fsa9480_read(client, REGISTER_CARKITINT1, &temp);
		}
	}

	msleep(5);

    interrupmask1_data = ~(ATTACH_INT_MASK|DETACH_INT_MASK|KEY_PRESS_INT_MASK \
                            |LONGKEY_PRESS_INT_MASK|LONGKEY_RELEASE_INT_MASK);
	fsa9480_write(client, REGISTER_INTERRUPTMASK1, interrupmask1_data);

#if defined(CONFIG_ARIES_NTT) // Modify NTTS1
	//syyoon 20100724	 fix for SC - Ad_10_2nd - 0006. When USB is removed, sometimes attatch value gets 0x00
	if((device1 == FSA9480_DEV_TY1_USB) && (!interrupt1)){
		printk("[FSA9480] dev1=usb, attach change is from 0 to 2\n");
		interrupt1 = FSA9480_INT1_DETACH;
	}
#endif
	FSA9480_ProcessDevice(device1, device2, interrupt1);

	if(interrupt1 & FSA9480_INT1_DETACH)
	{
		if(device1 != FSA9480_DEV_TY1_DED_CHG){
			//LOGD("FSA9480_disable LDO8\n");
			s3c_usb_cable(0);
		}
		ls->fsa_status.cable_state = 0;
	}

#ifdef CONFIG_WIRELESS_CHARGING
__end__:
#endif
	enable_irq(IRQ_FSA9480_INTB);
}

irqreturn_t fsa9480_interrupt(int irq, void *ptr)
{
	LOGD("%s\n", __func__);
	disable_irq_nosync(IRQ_FSA9480_INTB);

	queue_work(fsa9480_workqueue, &fsa9480_work);

	return IRQ_HANDLED; 
}

void fsa9480_interrupt_init(void)
{		
	s3c_gpio_cfgpin(GPIO_JACK_nINT, S3C_GPIO_SFN(GPIO_JACK_nINT_AF));
	s3c_gpio_setpull(GPIO_JACK_nINT, S3C_GPIO_PULL_NONE);
	set_irq_type(IRQ_FSA9480_INTB, IRQ_TYPE_EDGE_FALLING);

	if (request_irq(IRQ_FSA9480_INTB, fsa9480_interrupt, IRQF_DISABLED, "FSA9480 Detected", NULL)) 
		LOGD("[FSA9480]fail to register IRQ[%d] for FSA9480 USB Switch \n", IRQ_FSA9480_INTB);
}

void fsa9480_chip_init(void)
{
	struct i2c_client *client = fsa9480_i2c_client;
	u8 device1=0, device2=0;
    u8 interrupmask1_data = 0;
    u8 timingset_data = 0;

	fsa9480_write(client, HIDDEN_REGISTER_MANUAL_OVERRDES1, 0x01); //RESET

	mdelay(10);
	fsa9480_write(client, REGISTER_CONTROL, 0x1E);

	mdelay(10);

    interrupmask1_data = ~(ATTACH_INT_MASK|DETACH_INT_MASK|KEY_PRESS_INT_MASK \
                            |LONGKEY_PRESS_INT_MASK|LONGKEY_RELEASE_INT_MASK);
	fsa9480_write(client, REGISTER_INTERRUPTMASK1, interrupmask1_data);
	mdelay(10);    

    timingset_data = KEY_PRESS_TIME_200MS|ADC_DETECTION_TIME_300MS;
    fsa9480_write(client, REGISTER_TIMINGSET1, timingset_data);
	mdelay(10);    

	timingset_data = LONGKEY_PRESS_TIME_500MS;
	fsa9480_write(client, REGISTER_TIMINGSET2, timingset_data);
	mdelay(10);

	fsa9480_read(client, REGISTER_DEVICETYPE1, &device1);

	mdelay(10);

	fsa9480_read(client, REGISTER_DEVICETYPE2, &device2);
}

u8 FSA9480_Get_I2C_USB_Status(void)
{
	u8 device1, device2;

#ifdef CONFIG_WIRELESS_CHARGING
	if (HWREV == 9) {
		if (gpio_get_value(GPIO_WC_DETECT) == 0)
			return 0;
	}
#endif

	fsa9480_read(fsa9480_i2c_client, REGISTER_DEVICETYPE1, &device1);
	msleep(5);
	fsa9480_read(fsa9480_i2c_client, REGISTER_DEVICETYPE2, &device2);

	if((device1==FSA9480_DEV_TY1_USB)||(device2==FSA9480_DEV_TY2_JIG_USB_ON)||(device2==FSA9480_DEV_TY2_JIG_USB_OFF))
		return 1;
	else
		return 0;
}
EXPORT_SYMBOL(FSA9480_Get_I2C_USB_Status);

void FSA9480_ShowRegisterStatus(void) {
    u8 control, device1;

    fsa9480_read(fsa9480_i2c_client, REGISTER_CONTROL, &control);
	fsa9480_read(fsa9480_i2c_client, REGISTER_DEVICETYPE1, &device1);

	LOGD("control(0x%x), device1(0x%x)\n", control, device1);
}
EXPORT_SYMBOL(FSA9480_ShowRegisterStatus);

void connectivity_switching_init(struct work_struct *ignored)
{
	struct line_status_t *ls = &g_ls;
	int lpm_mode_check = charging_mode_get();

	if (sec_get_param_value){
		param_load_path(ls);
		cancel_delayed_work(&micorusb_init_work);
	}
	else{
		schedule_delayed_work(&micorusb_init_work, msecs_to_jiffies(200));		
		return;
	}

	if(BOOTUP){
		BOOTUP = 0; 
		otg_phy_init(); //USB Power on after boot up.
	}

	if(ls->factoryreset == 0xAE || ls->switch_sel == 0x1)	{
		// factory reset action
		//usb_switch_select(USBSTATUS_SAMSUNG_KIES);
		usb_switch_select(USBSTATUS_MTPONLY);
		//ls->usb_func = S2B(currentusbstatus); 
		ls->usb_func = S2B(USBSTATUS_MTPONLY); 
		mtp_mode_on = 1;
		ap_usb_power_on(0);
		UsbMenuSelStore(USBSTATUS_UMS);	
		ls->uart_path = PATH_CP;
		ls->uart_debug_enable = 0;
		//usb default path is AP after FACTORY RESET
		ls->usb_path = PATH_AP;
		param_save_path(ls);

		goto CleanUp;		
	}

	/*Turn off usb power when LPM mode*/
	if(lpm_mode_check)
		otg_phy_off();

	if(ls->usb_path == PATH_AP){
		switch(B2S(ls->usb_func)){
//			case USBSTATUS_SAMSUNG_KIES:
//				usb_switch_select(USBSTATUS_SAMSUNG_KIES);
				/*USB Power off till MTP Appl launching*/				
//				mtp_mode_on = 1;
//				ap_usb_power_on(0);
//				break;
			case USBSTATUS_MTPONLY:
				//usb_mtp_select(1);
				usb_switch_select(USBSTATUS_MTPONLY);
				/*USB Power off till MTP Appl launching*/				
				mtp_mode_on = 1;
				ap_usb_power_on(0);
				break;
			case USBSTATUS_UMS:
				while(usb_switch_select(USBSTATUS_UMS)==16){
					LOGD("eagain\n");
				}
				LOGD("here");
				break;
//			case USBSTATUS_VTP:
//				usb_switch_select(USBSTATUS_VTP);
//				break;
			case USBSTATUS_ASKON:
                usb_switch_select(USBSTATUS_ASKON);
				break;
		}
	}

	//if(ls->line == LINE_USB ){
	if(ls->line != LINE_USB ){
		s3c_usb_cable(1);
		mdelay(5);
		s3c_usb_cable(0);
	}

//Manafacture Request - default UART PATH to CP
	if( ls->uart_debug_enable == 0 ) {
		ls->uart_path = PATH_CP;
	}
	
	LOGD("switch_sel 0x%x line 0x%x usb 0x%x\n", ls->switch_sel, ls->line, ls->usb_func);
	param_save_path(ls);
CleanUp:
	return;
}

static ssize_t print_switch_name(struct switch_dev *sdev, char *buf)
{
	return sprintf(buf, "%s\n", DRIVER_NAME_UMS);
}

static ssize_t print_switch_state(struct switch_dev *sdev, char *buf)
{
	int usbstatus;
	struct line_status_t *ls = &g_ls;

	if( mtp_power_off == 1 )
	{
		printk("USB power off for MTP\n");
		mtp_power_off = 0;
		return sprintf(buf, "%s\n", "RemoveOffline");
	}
    
    if( ls->usb_path != PATH_AP ) {
        LOGD("usb path is not AP\n");
        return sprintf(buf, "%s\n", "RemoveOffline");
    }

	usbstatus = FSA9480_Get_USB_Status();

	//TODO : each platform require different noti
//#if 0 //froyo default
//	if(usbstatus){
//		return sprintf(buf, "%s\n", "online");
//	}
//	else{
//		return sprintf(buf, "%s\n", "offline");
//	}
//#elif 1 //P1
    LOGD("usbstatus %d, currentusbstatus %d\n", usbstatus, currentusbstatus);

	if(usbstatus){
		if(currentusbstatus == USBSTATUS_VTP)
			return sprintf(buf, "%s\n", "RemoveOffline");
		else if((currentusbstatus== USBSTATUS_UMS) 
			|| (currentusbstatus== USBSTATUS_ADB)
			|| (currentusbstatus== USBSTATUS_SAMSUNG_KIES))
			return sprintf(buf, "%s\n", "ums online");
		else
			return sprintf(buf, "%s\n", "InsertOffline");
	}
	else{
		if((currentusbstatus== USBSTATUS_UMS) 
			|| (currentusbstatus== USBSTATUS_ADB)
			|| (currentusbstatus== USBSTATUS_SAMSUNG_KIES))
			return sprintf(buf, "%s\n", "ums offline");
		else
			return sprintf(buf, "%s\n", "RemoveOffline");
	}
//#else //S1
//	if(usbstatus){
//		if((currentusbstatus== USBSTATUS_UMS) || (currentusbstatus== USBSTATUS_ADB))
//			return sprintf(buf, "%s\n", "InsertOnline");
//		else
//			return sprintf(buf, "%s\n", "InsertOffline");
//	}
//	else{
//		if((currentusbstatus== USBSTATUS_UMS) || (currentusbstatus== USBSTATUS_ADB))
//			return sprintf(buf, "%s\n", "RemoveOnline");
//		else
//			return sprintf(buf, "%s\n", "RemoveOffline");
//	}
//#endif
}


static int fsa9480_codec_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct fsa9480_state *state;
	struct device *dev = &client->dev;
	u8 pData;
	int i,err;

	LOGD("[FSA9480] %s\n", __func__);

	dock_key_input_dev = input_allocate_device();
	if( !dock_key_input_dev )
		return -ENOMEM;

	dock_key_input_dev->name = switch_dock_detection.name;
	set_bit(EV_SYN, dock_key_input_dev->evbit);
	set_bit(EV_KEY, dock_key_input_dev->evbit);
	
	for(i=0; i < DOCK_KEY_MAX; i++) 
	{
		set_bit(dock_keys[i], dock_key_input_dev->keybit);
	}
	err = input_register_device(dock_key_input_dev);
	if( err ) {
		input_free_device(dock_key_input_dev);
		return err;
	}
	
	s3c_gpio_cfgpin(GPIO_USB_SCL_28V, S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(GPIO_USB_SCL_28V, S3C_GPIO_PULL_NONE);

	s3c_gpio_cfgpin(GPIO_USB_SDA_28V, S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(GPIO_USB_SDA_28V, S3C_GPIO_PULL_NONE);

	s3c_gpio_cfgpin(GPIO_UART_SEL, S3C_GPIO_OUTPUT );
	s3c_gpio_setpull(GPIO_UART_SEL, S3C_GPIO_PULL_NONE);

	if (device_create_file(switch_dev, &dev_attr_uart_sel) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_uart_sel.attr.name);

	if (device_create_file(switch_dev, &dev_attr_usb_sel) < 0)
		LOGD("[FSA9480]Failed to create device file(%s)!\n", dev_attr_usb_sel.attr.name);

	if (device_create_file(switch_dev, &dev_attr_usb_state) < 0)
		LOGD("[FSA9480]Failed to create device file(%s)!\n", dev_attr_usb_state.attr.name);

#if 0
	if (device_create_file(switch_dev, &dev_attr_DMport) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_DMport.attr.name);

	if (device_create_file(switch_dev, &dev_attr_DMlog) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_DMlog.attr.name);
#endif

	if (device_create_file(switch_dev, &dev_attr_UsbMenuSel) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_UsbMenuSel.attr.name);

	if (device_create_file(switch_dev, &dev_attr_AskOnMenuSel) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_AskOnMenuSel.attr.name);

	if (device_create_file(switch_dev, &dev_attr_Mtp) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_Mtp.attr.name);

	if (device_create_file(switch_dev, &dev_attr_SwitchingInitValue) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_SwitchingInitValue.attr.name);		

	if (device_create_file(switch_dev, &dev_attr_FactoryResetValue) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_FactoryResetValue.attr.name);		

	if (device_create_file(switch_dev, &dev_attr_AskOnStatus) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_AskOnStatus.attr.name);			

	if (device_create_file(switch_dev, &dev_attr_MtpInitStatusSel) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_MtpInitStatusSel.attr.name);			

	if (device_create_file(switch_dev, &dev_attr_AskInitStatusSel) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_AskInitStatusSel.attr.name);	

	if (device_create_file(switch_dev, &dev_attr_tethering) < 0)		
		pr_err("Failed to create device file(%s)!\n", dev_attr_tethering.attr.name);

	if (device_create_file(switch_dev, &dev_attr_dock) < 0)		
		pr_err("Failed to create device file(%s)!\n", dev_attr_dock.attr.name);
	
	if (device_create_file(switch_dev, &dev_attr_DefaultESNStatus) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_DefaultESNStatus.attr.name);  

	if (device_create_file(switch_dev, &dev_attr_UartDebug) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_UartDebug.attr.name);  

#ifdef _SUPPORT_SAMSUNG_AUTOINSTALLER_
	if (device_create_file(switch_dev, &dev_attr_KiesStatus) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_KiesStatus.attr.name);
#endif	

	init_waitqueue_head(&usb_detect_waitq); 
	INIT_WORK(&fsa9480_work, FSA9480_ReadIntRegister);
	fsa9480_workqueue = create_singlethread_workqueue("fsa9480_workqueue");

	state = kzalloc(sizeof(struct fsa9480_state), GFP_KERNEL);
	if(!state) {
		dev_err(dev, "%s: failed to create fsa9480_state\n", __func__);
		return -ENOMEM;
	}

	indicator_dev.name = DRIVER_NAME_UMS;
	indicator_dev.print_name = print_switch_name;
	indicator_dev.print_state = print_switch_state;
	switch_dev_register(&indicator_dev);
	switch_dev_register(&switch_dock_detection);

	state->client = client;
	fsa9480_i2c_client = client;

	i2c_set_clientdata(client, state);
	if(!fsa9480_i2c_client)
	{
		dev_err(dev, "%s: failed to create fsa9480_i2c_client\n", __func__);
		return -ENODEV;
	}

	/*clear interrupt mask register*/
	fsa9480_read(fsa9480_i2c_client, REGISTER_CONTROL, &pData);
	fsa9480_write(fsa9480_i2c_client, REGISTER_CONTROL, pData & ~INT_MASK);

	fsa9480_interrupt_init();

	fsa9480_chip_init();

	INIT_DELAYED_WORK(&micorusb_init_work, connectivity_switching_init);
	schedule_delayed_work(&micorusb_init_work, msecs_to_jiffies(200));

	return 0;
}


static int __devexit fsa9480_remove(struct i2c_client *client)
{
	struct fsa9480_state *state = i2c_get_clientdata(client);
	kfree(state);
	return 0;
}


struct i2c_driver fsa9480_i2c_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "fsa9480",
	},
	.id_table	= fsa9480_id,
	.probe	= fsa9480_codec_probe,
	.remove	= __devexit_p(fsa9480_remove),
	.command = NULL,
};
