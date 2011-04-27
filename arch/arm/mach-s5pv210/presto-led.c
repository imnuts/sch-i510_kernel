
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/init.h>

#include <mach/gpio.h>
#include <mach/hardware.h>
#include <plat/gpio-cfg.h>

#include <linux/sec_led.h>

#ifdef LED_DEBUG
/*
 * Presto LED Debug Functions
 */

unsigned int g_led_pins[][LED_COLOR_MAX]= {
	{GPIO_LED1_R, GPIO_LED1_G, GPIO_LED1_B},
	{GPIO_LED2_R, GPIO_LED2_G, GPIO_LED2_B},
	{GPIO_LED3_R, GPIO_LED3_G, GPIO_LED3_B},
	{GPIO_LED4_R, GPIO_LED4_G, GPIO_LED4_B}
	};

// gpio help functions
static void _gpio_out(unsigned int pin, bool level)
{
	s3c_gpio_cfgpin(pin, S3C_GPIO_OUTPUT);
	s3c_gpio_setpin(pin, level);
}
static bool _gpio_in(unsigned int pin)
{
	return gpio_get_value(pin);
}

//static void sec_init_leds(void)
//{
//	unsigned int i,j;
//	for(i=0;i<LED_MAX;i++)
//		for(j=0;j<LED_COLOR_MAX;j++)
//			s3c_gpio_cfgpin(g_led_pins[i][j], S3C_GPIO_OUTPUT);
//	return;
//}

//LED Control Main function
void sec_set_led_state(int lednum, int rgb)
{
	int i=0;
	if(lednum<LED1 || lednum>=LED_MAX || rgb<LED_OFF_ || rgb>LED_WHITE ){
		LOG("args boundary error\n");
		return ;
	}
	for(i=0;i<LED_COLOR_MAX;i++)  // i= LED_COLOR_BIT
		_gpio_out(g_led_pins[lednum][i], (rgb&(1<<i))?1:0);

}
//EXPORT_SYMBOL(sec_set_led_state);
int sec_get_led_state(int lednum)
{
	int i, rgb=0;
	if(lednum<LED1 || lednum>=LED_MAX){
		LOG("args boundary error\n");
		return -1;
	}
	for(i=0;i<LED_COLOR_MAX;i++) { // i= LED_COLOR_BIT
		rgb = _gpio_in(g_led_pins[lednum][i]);
		rgb=rgb<<1;
	}
	return rgb;
}
//EXPORT_SYMBOL(sec_get_led_state);
#endif // LED_DEBUG
