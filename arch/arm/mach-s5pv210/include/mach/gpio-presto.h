#ifndef __GPIO_ARIES_H_
#define __GPIO_ARIES_H_

//#include <mach/gpio.h>

#define BIT0                            0x00000001
#define BIT1                            0x00000002
#define BIT2                            0x00000004
#define BIT3                            0x00000008
#define BIT4                            0x00000010
#define BIT5                            0x00000020
#define BIT6                            0x00000040
#define BIT7                            0x00000080
#define BIT8                            0x00000100
#define BIT9                            0x00000200
#define BIT10                           0x00000400
#define BIT11                           0x00000800
#define BIT12                           0x00001000
#define BIT13                           0x00002000
#define BIT14                           0x00004000
#define BIT15                           0x00008000
#define BIT16                           0x00010000
#define BIT17                           0x00020000
#define BIT18                           0x00040000
#define BIT19                           0x00080000
#define BIT20                           0x00100000
#define BIT21                           0x00200000
#define BIT22                           0x00400000
#define BIT23                           0x00800000
#define BIT24                           0x01000000
#define BIT25                           0x02000000
#define BIT26                           0x04000000
#define BIT27                           0x08000000
#define BIT28                           0x10000000
#define BIT29                           0x20000000
#define BIT30                           0x40000000
#define BIT31                           0x80000000

#define GPIO_LEVEL_LOW      	0
#define GPIO_LEVEL_HIGH     	1
#define GPIO_LEVEL_NONE     	2
#define GPIO_INPUT				0
#define GPIO_OUTPUT				1
#define __NC__				S5PV210_GPA0(7)

//#define GPIO_BT_UART_RXD		S5PV210_GPA0(0)
//#define GPIO_BT_UART_RXD_AF		2

//#define GPIO_BT_UART_TXD		S5PV210_GPA0(1)
//#define GPIO_BT_UART_TXD_AF		2

//#define GPIO_BT_UART_CTS		S5PV210_GPA0(2)
//#define GPIO_BT_UART_CTS_AF		2

//#define GPIO_BT_UART_RTS		S5PV210_GPA0(3)
//#define GPIO_BT_UART_RTS_AF		2

#define GPIO_AP_RXD			S5PV210_GPA1(0)
#define GPIO_AP_RXD_AF			2

#define GPIO_AP_TXD			S5PV210_GPA1(1)
#define GPIO_AP_TXD_AF			2

#define	GPIO_LTE_SPI_CLK			S5PV210_GPB(0)
#define GPIO_LTE_SPI_CLK_AF		2

#define	GPIO_LTE_SPI_CS			S5PV210_GPB(1)
#define GPIO_LTE_SPI_CS_AF		2

#define GPIO_LTE_SPI_MISO		S5PV210_GPB(2)	// not used
#define GPIO_LTE_SPI_MISO_AF		2

#define	GPIO_LTE_SPI_MOSI			S5PV210_GPB(3)
#define	GPIO_LTE_SPI_MOSI_AF		2

#define	GPIO_WLAN_BT_EN			S5PV210_GPB(5)

#define GPIO_GPB6			__NC__//S5PV210_GPB(6)
#define GPIO_GPB7			__NC__//S5PV210_GPB(7)

#define GPIO_FM33_SCL			__NC__ //S5PV210_GPC0(0)

#define GPIO_FM33_SDA			__NC__ //S5PV210_GPC0(2)

// BT PCM??
#define GPIO_FM33_RST			__NC__ //S5PV210_GPC1(1)
#define GPIO_FM33_BP			__NC__ //S5PV210_GPC1(2)
#define GPIO_FM33_PWDN			__NC__ //S5PV210_GPC1(4)

//#define GPIO_BT_PCM_CLK			S5PV210_GPC1(0)
//#define GPIO_BT_PCM_CLK_AF		0x3
#define GPIO_GPC11			S5PV210_GPC1(1)
//#define GPIO_BT_PCM_SYNC		S5PV210_GPC1(2)
//#define GPIO_BT_PCM_SYNC_AF		0x3
//#define GPIO_BT_PCM_DIN			S5PV210_GPC1(3)
//#define GPIO_BT_PCM_DIN_AF		0x3
//#define GPIO_BT_PCM_DOUT		S5PV210_GPC1(4)
//#define GPIO_BT_PCM_DOUT_AF		0x3

#define GPIO_GPD00			S5PV210_GPD0(0)

//#define GPIO_GPD03				S5PV210_GPD0(3)

#define GPIO_GPE14			__NC__//S5PV210_GPE1(4)

#define GPIO_LTE_WAKE_INT		S5PV210_GPF3(5)

#define GPIO_WLAN_SDIO_CLK      S5PV210_GPG1(0)
#define GPIO_WLAN_SDIO_CLK_AF   2

#define GPIO_WLAN_SDIO_CMD      S5PV210_GPG1(1)
#define GPIO_WLAN_SDIO_CMD_AF   2

#define	GPIO_WLAN_RST		S5PV210_GPG1(2)
#define GPIO_WLAN_nRST          S5PV210_GPG1(2)
#define GPIO_WLAN_nRST_AF	1

#define GPIO_WLAN_SDIO_D0       S5PV210_GPG1(3)
#define GPIO_WLAN_SDIO_D0_AF    2

#define GPIO_WLAN_SDIO_D1       S5PV210_GPG1(4)
#define GPIO_WLAN_SDIO_D1_AF    2

#define GPIO_WLAN_SDIO_D2       S5PV210_GPG1(5)
#define GPIO_WLAN_SDIO_D2_AF    2

#define GPIO_WLAN_SDIO_D3       S5PV210_GPG1(6)
#define GPIO_WLAN_SDIO_D3_AF    2

#define GPIO_TA_CURRENT_SEL_AP	S5PV210_GPG3(3)

//#define GPIO_BT_WAKE			S5PV210_GPG3(4)

#define GPIO_WLAN_WAKE			S5PV210_GPG3(5)
#define GPIO_WLAN_WAKE_AF		1

//#define GPIO_BT_nRST		S5PV210_GPG3(6)
//#define	GPIO_BT_RST			S5PV210_GPG3(6)

#define	GPIO_AP_PS_HOLD			S5PV210_GPH0(0)
#define	GPIO_AP_PS_HOLD_AF		1 //?

#define GPIO_PS_VOUT			__NC__ //S5PV210_GPH0(2)
#define GPIO_PS_VOUT_AF			0xFF

#define GPIO_BUCK_1_EN_A		S5PV210_GPH0(3)

#define GPIO_BUCK_1_EN_B		S5PV210_GPH0(4)

#define GPIO_BUCK_2_EN			S5PV210_GPH0(5)

#define GPIO_AP_PMIC_IRQ		S5PV210_GPH0(7)
#define GPIO_AP_PMIC_IRQ_AF		0xFF

#define GPIO_DPRAM_INT_N		S5PV210_GPH1(0)
#define GPIO_DPRAM_INT_N_AF		0x1

#define GPIO_FUEL_INT			S5PV210_GPH1(1)

#define GPIO_CMC_RST			S5PV210_GPH1(2)

#define GPIO_nINT_ONEDRAM_AP		S5PV210_GPH1(3)
#define GPIO_nINT_ONEDRAM_AP_AF	0xF
#define GPIO_ONEDRAM_INT_N		S5PV210_GPH1(3)
#define GPIO_ONEDRAM_INT_N_AF		0xff

#define GPIO_220_PMIC_PWRON		S5PV210_GPH1(6)

#define GPIO_PHONE_ACTIVE		S5PV210_GPH1(7)
#define GPIO_PHONE_ACTIVE_AF		0x1

#define GPIO_KBC0			__NC__//S5PV210_GPH2(0)
#define GPIO_KBC0_AF			3

#define GPIO_KBC1			__NC__//S5PV210_GPH2(1)
#define GPIO_KBC1_AF			3

#define GPIO_BATT_ID			S5PV210_GPH2(3)

#define GPIO_WLAN_HOST_WAKE		S5PV210_GPH2(4)
#define GPIO_WLAN_HOST_WAKE_AF		0xF

//#define GPIO_BT_HOST_WAKE		S5PV210_GPH2(5)
//#define GPIO_BT_HOST_WAKE_AF		0xF

#define	GPIO_N_POWER		S5PV210_GPH2(6)
#define	GPIO_N_POWER_AF		2 //?

#define GPIO_JACK_nINT		S5PV210_GPH2(7)
#define GPIO_JACK_nINT_AF		0xF
#define GPIO_JACK_INT_N 		S5PV210_GPH2(7)
#define	GPIO_JACK_INT_N_AF		0xFF

#define GPIO_KBR0			__NC__//S5PV210_GPH3(0)
#define GPIO_KBR0_AF			3

#define GPIO_KBR1			__NC__//S5PV210_GPH3(1)
#define GPIO_KBR1_AF			3

#define GPIO_KBR2			__NC__//S5PV210_GPH3(2)
#define GPIO_KBR2_AF			3

#define GPIO_KBR3			__NC__//S5PV210_GPH3(3)
#define GPIO_KBR3_AF			3

#define GPIO_T_FLASH_DETECT		__NC__//S5PV210_GPH3(4)

#define GPIO_LTE_ACTIVE			S5PV210_GPH3(5)
#define GPIO_LTE_ACTIVE_AF		0x00


//VIA Reset(?)
#define GPIO_CP_RST			__NC__//S5PV210_GPH3(7)
#define GPIO_PHONE_RST_N		__NC__//S5PV210_GPH3(7)
#define GPIO_PHONE_RST_N_AF		0x1

#define GPIO_GPI1			S5PV210_GPI(1)

#define GPIO_GPI5			S5PV210_GPI(5)

#define GPIO_GPI6			S5PV210_GPI(6)

#define GPIO_LED3_R			S5PV210_GPJ0(0)

#define GPIO_LED3_G			S5PV210_GPJ0(1)

#define GPIO_HWREV_MODE0		S5PV210_GPJ0(2)

#define GPIO_HWREV_MODE1		S5PV210_GPJ0(3)

#define GPIO_HWREV_MODE2		S5PV210_GPJ0(4)

#define GPIO_LED3_B			S5PV210_GPJ0(5)

#define GPIO_LED4_R			S5PV210_GPJ0(6)

#define GPIO_HWREV_MODE3		S5PV210_GPJ0(7)

#define GPIO_PHONE_ON			S5PV210_GPJ1(0)
#define GPIO_PHONE_ON_AF		0x1

#define GPIO_LED1_R			S5PV210_GPJ1(1)

#define GPIO_LTE_PS_HOLD_OFF   	S5PV210_GPJ1(2)

#define GPIO_LED1_B			S5PV210_GPJ1(3)

#define GPIO_LED4_G			S5PV210_GPJ1(4)

#define GPIO_LED4_B			S5PV210_GPJ1(5)

#define GPIO_USB_SW_EN			__NC__ //S5PV210_GPJ2(0)

#define GPIO_UART_SEL1			S5PV210_GPJ2(1)

#define GPIO_VIA_PS_HOLD_OFF	S5PV210_GPJ2(2)

#define GPIO_BOOT_MODE			S5PV210_GPJ2(3)

#define GPIO_LED1_G			S5PV210_GPJ2(4)

#define GPIO_PDA_ACTIVE			S5PV210_GPJ2(5)
#define GPIO_PDA_ACTIVE_AF		0x1

#define GPIO_LED2_R		S5PV210_GPJ2(6)

#define GPIO_LED2_G		S5PV210_GPJ2(7)

#define GPIO_LED2_B		S5PV210_GPJ3(0)

#define GPIO_FUEL_SCL_28             S5PV210_GPJ3(1)

#define GPIO_FUEL_SDA_28        S5PV210_GPJ3(2)

#define GPIO_USB_SW             S5PV210_GPJ3(3)

#define GPIO_USB_SCL_28V		S5PV210_GPJ3(4)

#define GPIO_USB_SDA_28V		S5PV210_GPJ3(5)

#define GPIO_AP_PMIC_SDA		S5PV210_GPJ4(0)

#define GPIO_AP_PMIC_SCL		S5PV210_GPJ4(3)

#define GPIO_MP010			S5PV210_MP01(0)

#define GPIO_C110_DPRAM_CS_N		S5PV210_MP01(3)

#define GPIO_AP_NANDCS			S5PV210_MP01(4)
#define GPIO_AP_NANDCS_AF		5

//#define GPIO_MP016				S5PV210_MP01(6)
#define GPIO_C110_OE_N				S5PV210_MP01(6)

//#define GPIO_MP017				S5PV210_MP01(7)
#define GPIO_C110_WE_N				S5PV210_MP01(7)

//#define GPIO_MP020				S5PV210_MP02(0)
#define GPIO_C110_LB				S5PV210_MP02(0)

//#define GPIO_MP021				S5PV210_MP02(1)
#define GPIO_C110_UB				S5PV210_MP02(1)

#define GPIO_VCC_19V_PDA		S5PV210_MP02(2)

#define GPIO_MP023				S5PV210_MP02(3)

#define GPIO_MP030			__NC__//S5PV210_MP03(0)

#define GPIO_MP031			__NC__//S5PV210_MP03(1)

#define GPIO_MP032			__NC__//S5PV210_MP03(2)

#define GPIO_VCC_18V_PDA		S5PV210_MP03(4)

#define GPIO_CP_nRST			__NC__//S5PV210_MP03(5)

#define GPIO_MP036			__NC__//S5PV210_MP03(6)

#define GPIO_PCM_SEL			__NC__//S5PV210_MP03(7)

#define GPIO_USB_SEL			__NC__ //S5PV210_MP04(0)

#define GPIO_MP042				S5PV210_MP04(2)

#define GPIO_MP044				S5PV210_MP04(4)

#define GPIO_MP054				S5PV210_MP05(4)

#define GPIO_MP056				S5PV210_MP05(6)

#define GPIO_UART_SEL			S5PV210_MP05(7)
#if 0
/* uart 0~3 */
#define 	GPIO_BT_RXD 		S5PV210_GPA0(0)
#define 	GPIO_BT_RXD_AF 		2
#define 	GPIO_BT_TXD 		S5PV210_GPA0(1)
#define 	GPIO_BT_TXD_AF 		2
#define 	GPIO_BT_CTS 		S5PV210_GPA0(2)
#define 	GPIO_BT_CTS_AF 		2
#define 	GPIO_BT_RTS 		S5PV210_GPA0(3)
#define 	GPIO_BT_RTS_AF 		2

#define 	GPIO_GPS_RXD		S5PV210_GPA0(4)
#define 	GPIO_GPS_RXD_AF 	2
#define 	GPIO_GPS_TXD 		S5PV210_GPA0(5)
#define 	GPIO_GPS_TXD_AF 	2
#define 	GPIO_GPS_CTS		S5PV210_GPA0(6)
#define 	GPIO_GPS_CTS_AF 	2
#define 	GPIO_GPS_RTS		S5PV210_GPA0(7)
#define 	GPIO_GPS_RTS_AF 	2

#define 	GPIO_AP_RXD 		S5PV210_GPA1(0)
#define 	GPIO_AP_RXD_AF 		2
#define 	GPIO_AP_TXD 		S5PV210_GPA1(1)
#define 	GPIO_AP_TXD_AF 		2

#define 	GPIO_FLM_RXD 		S5PV210_GPA1(2)
#define 	GPIO_FLM_RXD_AF 	2
#define 	GPIO_FLM_TXD 		S5PV210_GPA1(3)
#define 	GPIO_FLM_TXD_AF 	2
#endif
#endif
