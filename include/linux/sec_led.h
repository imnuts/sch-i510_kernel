#ifndef __DEBUG_LED_H__
#define __DEBUG_LED_H__

#define LED_DEBUG

enum _LED_NUM {
	LED1,
	PWR_LED = LED1,
	LED2,
	LED3,
	LED4,
	LED_MAX,
};

enum _LED_COLOR_BIT {
	LED_RED_BIT,
	LED_GREEN_BIT,
	LED_BLUE_BIT,
	LED_COLOR_MAX,
};

#define LED_RED		(1<<LED_RED_BIT)
#define LED_GREEN 	(1<<LED_GREEN_BIT)
#define LED_BLUE 	(1<<LED_BLUE_BIT)

#define LED_YELLOW 	(LED_RED|LED_GREEN)
#define LED_MAGENTA 	(LED_RED|LED_BLUE)
#define LED_CYAN 	(LED_GREEN|LED_BLUE)

#define LED_WHITE 	(LED_RED|LED_GREEN|LED_BLUE)
#define LED_OFF_ 	0
#define LED_BLACK 	0

#define LOG(s, args...) printk(KERN_ERR "%s " s, __func__, ##args)

// Export function prototype
void sec_set_led_state(int lednum, int rgb);
int sec_get_led_state(int lednum);
#endif
