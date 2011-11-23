#ifndef __GPIO_ARIES_H_
#define __GPIO_ARIES_H_

#define GPIO_LEVEL_LOW			   0
#define GPIO_LEVEL_HIGH			   1
#define GPIO_LEVEL_NONE			   2

#define GPIO_INPUT			      0
#define GPIO_OUTPUT			      1


#define GPIO_DUMMY               GPIO_GPC14

#define GPIO_BT_RXD              S5PV210_GPA0(0)
#define GPIO_BT_RXD_AF           2

#define GPIO_BT_UART_RXD         S5PV210_GPA0(0)
#define GPIO_BT_UART_RXD_AF      2

#define GPIO_BT_TXD              S5PV210_GPA0(1)
#define GPIO_BT_TXD_AF           2

#define GPIO_BT_UART_TXD         S5PV210_GPA0(1)
#define GPIO_BT_UART_TXD_AF      2

#define GPIO_BT_CTS              S5PV210_GPA0(2)
#define GPIO_BT_CTS_AF           2

#define GPIO_BT_UART_CTS         S5PV210_GPA0(2)
#define GPIO_BT_UART_CTS_AF      2

#define GPIO_BT_RTS              S5PV210_GPA0(3)
#define GPIO_BT_RTS_AF           2

#define GPIO_BT_UART_RTS         S5PV210_GPA0(3)
#define GPIO_BT_UART_RTS_AF      2

#define GPIO_ACC_INT             S5PV210_GPA0(4)

// chief
#define GPIO_KEYSCAN6            S5PV210_GPA0(5)
#define GPIO_CAM_MEGA_EN         S5PV210_GPA0(6)
#define GPIO_TA_CURRENT_SEL_AP   S5PV210_GPA0(7)

#define GPIO_AP_RXD              S5PV210_GPA1(0)
#define GPIO_AP_RXD_AF           2

#define GPIO_AP_TXD              S5PV210_GPA1(1)
#define GPIO_AP_TXD_AF           2

#define GPIO_CP_RST             S5PV210_GPA1(2)		/* CP_DPRAM_MODE */
#define GPIO_QSC_RESET_N        S5PV210_GPA1(2)		/* CP_DPRAM_MODE */
#define GPIO_QSC_RESET_N_AF     1					/* CP_DPRAM_MODE */

#define GPIO_KEYSCAN7            S5PV210_GPA1(3)
#define GPIO_KEYSCAN7_AF         3

#define GPIO_LTE_SPI_CLK        S5PV210_GPB(0)      /* CMC_CLK_2.8V */
#define GPIO_LTE_SPI_CLK_AF     2
#define GPIO_LTE_SPI_CS         S5PV210_GPB(1)      /* CMC_CS_2.8V */
#define GPIO_LTE_SPI_CS_AF      2
#define GPIO_LTE_SPI_MISO       S5PV210_GPB(2)      /* CMC_DO_2.8V */
#define GPIO_LTE_SPI_MISO_AF    2
#define	GPIO_LTE_SPI_DO         S5PV210_GPB(2)
#define GPIO_LTE_SPI_DO_AF      2
#define GPIO_LTE_SPI_MOSI       S5PV210_GPB(3)      /* CMC_DI_2.8V */
#define GPIO_LTE_SPI_MOSI_AF    2
#define GPIO_LTE_SPI_DI         S5PV210_GPB(3)
#define GPIO_LTE_SPI_DI_AF      2

#define GPIO_BL_SCL_28V          S5PV210_GPB(4)
#define GPIO_BL_SCL_28V_AF       2
#define GPIO_BL_SDA_28V          S5PV210_GPB(5)
#define GPIO_BL_SDA_28V_AF       2

/* heatup - recheck
#define GPIO_GPB6                S5PV210_GPB(6)
#define	GPIO_TOUCH_RST          S5PV210_GPB(6)
#define	GPIO_TOUCH_ST_AF        1
*/
#define GPIO_BUCK_1_EN_A         S5PV210_GPB(6)

#define GPIO_GPB7                S5PV210_GPB(7)

#define GPIO_BUCK_2_EN           S5PV210_GPB(7)

/*
#define GPIO_REC_PCM_CLK         S5PV210_GPC0(0)
#define GPIO_REC_PCM_CLK_AF      3
*/
#define GPIO_GPC01               S5PV210_GPC0(1)

#define GPIO_DISPLAY_CLK         S5PV210_GPC0(1)
#define GPIO_DISPLAY_CLK_AF      S3C_GPIO_SFN(1)

#define GPIO_DISPLAY_SI          S5PV210_GPC0(3)
#define GPIO_DISPLAY_SI_AF       S3C_GPIO_SFN(1)

#define GPIO_MLCD_RST            S5PV210_GPC0(4)
#define GPIO_MLCD_RST_AF         S3C_GPIO_SFN(1)

/*
#define GPIO_REC_PCM_SYNC        S5PV210_GPC0(2)
#define GPIO_REC_PCM_SYNC_AF     3

#define GPIO_REC_PCM_IN          S5PV210_GPC0(3)
#define GPIO_REC_PCM_IN_AF       3

#define GPIO_REC_PCM_OUT         S5PV210_GPC0(4)
#define GPIO_REC_PCM_OUT_AF      3

#define BLOW_PCM_CLK             S5PV210_GPC1(0)
*/

#define GPIO_EARPATH_SEL         S5PV210_GPC1(0)

#define GPIO_GPC11               S5PV210_GPC1(1)

#define GPIO_GPC12               S5PV210_GPC1(2)

#define GPIO_GPC13               S5PV210_GPC1(3)

#define GPIO_GPC14               S5PV210_GPC1(4)

#define GPIO_GPD00               S5PV210_GPD0(0)

#define GPIO_BOOT_MODE           S5PV210_GPD0(0)

#define GPIO_VIBTONE_PWM         S5PV210_GPD0(1)

#define GPIO_MSENSE_nRST         S5PV210_GPD0(2)

// heatup - recheck
#define _3_GPIO_TOUCH_INT        S5PV210_GPD0(3)
#define _3_GPIO_TOUCH_INT_AF     S3C_GPIO_SFN(0xf)

#define GPIO_GPD03               S5PV210_GPD0(3)
#define GPIO_TOUCH_INT           S5PV210_GPD0(3)
#define GPIO_TOUCH_INT_AF        0xFF

#define GPIO_CAM_SDA_29V         S5PV210_GPD1(0)
#define GPIO_CAM_SDA_29V_AF      2

#define GPIO_CAM_SCL_29V         S5PV210_GPD1(1)
#define GPIO_CAM_SCL_29V_AF      2

/* heatup - recheck
#define GPIO_GRIP_SDA_28V        S5PV210_GPD1(2)
#define GPIO_GRIP_SDA_28V_AF     2

#define GPIO_GRIP_SCL_28V        S5PV210_GPD1(3)
#define GPIO_GRIP_SCL_28V_AF     2
*/

// heatup - recheck
#define AP_I2C_SDA               S5PV210_GPD1(2)
#define AP_I2C_SDA_28V           S5PV210_GPD1(2)
#define GPIO_AP_SDA_28V          S5PV210_GPD1(2)

#define AP_I2C_SCL               S5PV210_GPD1(3)
#define AP_I2C_SCL_28V           S5PV210_GPD1(3)
#define GPIO_AP_SCL_28V          S5PV210_GPD1(3)

#define _3_TOUCH_SDA_28V         S5PV210_GPD1(4)
#define GPIO_TSP_SDA_28V         S5PV210_GPD1(4)
#define GPIO_TSP_SDA_28V_AF      2

#define _3_TOUCH_SCL_28V         S5PV210_GPD1(5)
#define GPIO_TSP_SCL_28V         S5PV210_GPD1(5)
#define GPIO_TSP_SCL_28V_AF      2

#define GPIO_CAM_PCLK            S5PV210_GPE0(0)
#define GPIO_CAM_PCLK_AF         2

#define GPIO_CAM_VSYNC           S5PV210_GPE0(1)
#define GPIO_CAM_VSYNC_AF        2

#define GPIO_CAM_HSYNC           S5PV210_GPE0(2)
#define GPIO_CAM_HSYNC_AF        2

#define GPIO_CAM_D0              S5PV210_GPE0(3)
#define GPIO_CAM_D0_AF           2

#define GPIO_CAM_D1              S5PV210_GPE0(4)
#define GPIO_CAM_D1_AF           2

#define GPIO_CAM_D2              S5PV210_GPE0(5)
#define GPIO_CAM_D2_AF           2

#define GPIO_CAM_D3              S5PV210_GPE0(6)
#define GPIO_CAM_D3_AF           2

#define GPIO_CAM_D4              S5PV210_GPE0(7)
#define GPIO_CAM_D4_AF           2

#define GPIO_CAM_D5              S5PV210_GPE1(0)
#define GPIO_CAM_D5_AF           2

#define GPIO_CAM_D6              S5PV210_GPE1(1)
#define GPIO_CAM_D6_AF           2

#define GPIO_CAM_D7              S5PV210_GPE1(2)
#define GPIO_CAM_D7_AF           2

#define GPIO_CAM_MCLK            S5PV210_GPE1(3)
#define GPIO_CAM_MCLK_AF         2

#define GPIO_CAM_MEGA_nRST       S5PV210_GPE1(4) /* CAM_RST */

#define GPIO_DISPLAY_HSYNC       S5PV210_GPF0(0)
#define GPIO_DISPLAY_HSYNC_AF    2

#define GPIO_DISPLAY_VSYNC       S5PV210_GPF0(1)
#define GPIO_DISPLAY_VSYNC_AF    2

#define GPIO_DISPLAY_DE          S5PV210_GPF0(2)
#define GPIO_DISPLAY_DE_AF       2

#define GPIO_DISPLAY_PCLK        S5PV210_GPF0(3)
#define GPIO_DISPLAY_PCLK_AF     2

#define GPIO_LCD_D0              S5PV210_GPF0(4)
#define GPIO_LCD_D0_AF           2

#define GPIO_LCD_D1              S5PV210_GPF0(5)
#define GPIO_LCD_D1_AF           2

#define GPIO_LCD_D2              S5PV210_GPF0(6)
#define GPIO_LCD_D2_AF           2

#define GPIO_LCD_D3              S5PV210_GPF0(7)
#define GPIO_LCD_D3_AF           2

#define GPIO_LCD_D4              S5PV210_GPF1(0)
#define GPIO_LCD_D4_AF           2

#define GPIO_LCD_D5              S5PV210_GPF1(1)
#define GPIO_LCD_D5_AF           2

#define GPIO_LCD_D6              S5PV210_GPF1(2)
#define GPIO_LCD_D6_AF           2

#define GPIO_LCD_D7              S5PV210_GPF1(3)
#define GPIO_LCD_D7_AF           2

#define GPIO_LCD_D8              S5PV210_GPF1(4)
#define GPIO_LCD_D8_AF           2

#define GPIO_LCD_D9              S5PV210_GPF1(5)
#define GPIO_LCD_D9_AF           2

#define GPIO_LCD_D10             S5PV210_GPF1(6)
#define GPIO_LCD_D10_AF          2

#define GPIO_LCD_D11             S5PV210_GPF1(7)
#define GPIO_LCD_D11_AF          2

#define GPIO_LCD_D12             S5PV210_GPF2(0)
#define GPIO_LCD_D12_AF          2

#define GPIO_LCD_D13             S5PV210_GPF2(1)
#define GPIO_LCD_D13_AF          2

#define GPIO_LCD_D14             S5PV210_GPF2(2)
#define GPIO_LCD_D14_AF          2

#define GPIO_LCD_D15             S5PV210_GPF2(3)
#define GPIO_LCD_D15_AF          2

#define GPIO_LCD_D16             S5PV210_GPF2(4)
#define GPIO_LCD_D16_AF          2

#define GPIO_LCD_D17             S5PV210_GPF2(5)
#define GPIO_LCD_D17_AF          2

#define GPIO_LCD_D18             S5PV210_GPF2(6)
#define GPIO_LCD_D18_AF          2

#define GPIO_LCD_D19             S5PV210_GPF2(7)
#define GPIO_LCD_D19_AF          2

#define GPIO_LCD_D20             S5PV210_GPF3(0)
#define GPIO_LCD_D20_AF          2

#define GPIO_LCD_D21             S5PV210_GPF3(1)
#define GPIO_LCD_D21_AF          2

#define GPIO_LCD_D22             S5PV210_GPF3(2)
#define GPIO_LCD_D22_AF          2

#define GPIO_LCD_D23             S5PV210_GPF3(3)
#define GPIO_LCD_D23_AF          2

#define GPIO_CODEC_LDO_EN        S5PV210_GPF3(4)

#if 1    // heatup - recheck - add
#define GPIO_LTE_WAKE_INT        S5PV210_GPF3(5)
#else
#define GPIO_GPF35               S5PV210_GPF3(5)
#endif

#define GPIO_NAND_CLK            S5PV210_GPG0(0)
#define GPIO_NAND_CLK_AF         2

#define GPIO_NAND_CMD            S5PV210_GPG0(1)
#define GPIO_NAND_CMD_AF         2

#define GPIO_AP_PMIC_SCL         S5PV210_GPG0(2)
#define PMIC_I2C_SCL             S5PV210_GPG0(2)

#define GPIO_NAND_D0             S5PV210_GPG0(3)
#define GPIO_NAND_D0_AF          2

#define GPIO_NAND_D1             S5PV210_GPG0(4)
#define GPIO_NAND_D1_AF          2

#define GPIO_NAND_D2             S5PV210_GPG0(5)
#define GPIO_NAND_D2_AF          2

#define GPIO_NAND_D3             S5PV210_GPG0(6)
#define GPIO_NAND_D3_AF          2

#define GPIO_WLAN_SDIO_CLK       S5PV210_GPG1(0)
#define GPIO_WLAN_SDIO_CLK_AF    2

#define GPIO_WLAN_SDIO_CMD       S5PV210_GPG1(1)
#define GPIO_WLAN_SDIO_CMD_AF    2

#define GPIO_WLAN_RST            S5PV210_GPG1(2)
#define GPIO_WLAN_nRST           S5PV210_GPG1(2)
#define GPIO_WLAN_nRST_AF        1

#define GPIO_WLAN_SDIO_D0        S5PV210_GPG1(3)
#define GPIO_WLAN_SDIO_D0_AF     2

#define GPIO_WLAN_SDIO_D1        S5PV210_GPG1(4)
#define GPIO_WLAN_SDIO_D1_AF     2

#define GPIO_WLAN_SDIO_D2        S5PV210_GPG1(5)
#define GPIO_WLAN_SDIO_D2_AF     2

#define GPIO_WLAN_SDIO_D3        S5PV210_GPG1(6)
#define GPIO_WLAN_SDIO_D3_AF     2

#define GPIO_T_FLASH_CLK         S5PV210_GPG2(0)
#define GPIO_T_FLASH_CLK_AF      2

#define GPIO_T_FLASH_CMD         S5PV210_GPG2(1)
#define GPIO_T_FLASH_CMD_AF      2

#define GPIO_T_FLASH_D0          S5PV210_GPG2(3)
#define GPIO_T_FLASH_D0_AF       2

#define GPIO_T_FLASH_D1          S5PV210_GPG2(4)
#define GPIO_T_FLASH_D1_AF       2

#define GPIO_T_FLASH_D2          S5PV210_GPG2(5)
#define GPIO_T_FLASH_D2_AF       2

#define GPIO_T_FLASH_D3          S5PV210_GPG2(6)
#define GPIO_T_FLASH_D3_AF       2

#define GPIO_ALS_SDA_28V         S5PV210_GPG3(0)

#define GPIO_MASSMEMORY_EN       S5PV210_GPG3(1)

#define GPIO_ALS_SCL_28V         S5PV210_GPG3(2)

#define GPIO_BT_nRST             S5PV210_GPG3(3)
#define GPIO_BT_RST              GPIO_BT_nRST

#define GPIO_BT_WAKE             S5PV210_GPG3(4)

#define GPIO_WLAN_BT_EN          S5PV210_GPG3(5)

// heatup - chief -> GPIO_WLAN_BT_EN
//#define GPIO_WLAN_WAKE           S5PV210_GPG3(5)
//#define GPIO_WLAN_WAKE_AF        1

#define GPIO_AP_PMIC_SDA         S5PV210_GPG3(6)
#define PMIC_I2C_SDA             S5PV210_GPG3(6)


#define GPIO_AP_PS_HOLD          S5PV210_GPH0(0)
#define GPIO_AP_PS_HOLD_AF       1

#define GPIO_PS_VOUT             S5PV210_GPH0(1)
#define GPIO_PS_VOUT_AF          0xFF

#define GPIO_3P_SEND_END            S5PV210_GPH0(2)
#define GPIO_EAR_SEND_END_SHORT     GPIO_3P_SEND_END
#define GPIO_EAR_SEND_END_SHORT_AF  0xF

#define GPIO_WLAN_HOST_WAKE      S5PV210_GPH0(3)
#define GPIO_WLAN_HOST_WAKE_AF   0xF

#define GPIO_BT_HOST_WAKE        S5PV210_GPH0(4)
#define GPIO_BT_HOST_WAKE_AF     0xF


#define GPIO_DET_35              S5PV210_GPH0(6)
#define GPIO_DET_35_AF           0xFF

#define GPIO_AP_PMIC_IRQ         S5PV210_GPH0(7)
#define GPIO_AP_PMIC_IRQ_AF      0xFF


#define GPIO_C111_DPRAM_INT_N   S5PV210_GPH1(1)		/* CP_DPRAM_MODE */

#define GPIO_GAUGE_ALT          S5PV210_GPH1(2)
#define GPIO_nINT_ONEDRAM_AP    S5PV210_GPH1(3)
#define GPIO_nINT_ONEDRAM_AP_AF 0xF

#define GPIO_GPH14              S5PV210_GPH1(4)

#define GPIO_EAR_SEND_END       S5PV210_GPH1(5)
#define GPIO_EAR_SEND_END_OPEN  GPIO_EAR_SEND_END
#define GPIO_EAR_SEND_END_AF    0xFF

#if defined(CONFIG_S5PV210_GARNETT_DELTA)
#define GPIO_EAR_SEND_END_SHORT        S5PV210_GPH3(6)
#define GPIO_EAR_SEND_END_SHORT_AF     0xFF
#endif

#define GPIO_PHONE_ACTIVE		S5PV210_GPH1(7)		/* CP_DPRAM_MODE */
// #define GPIO_PHONE_ACTIVE_AF    2					/* CP_DPRAM_MODE */
// heatup - recheck - chief -> 2
#define GPIO_PHONE_ACTIVE_AF     0xFF //2

#define GPIO_QSC_ACTIVE         S5PV210_GPH1(7)		/* CP_DPRAM_MODE */
#define GPIO_QSC_ACTIVE_AF      0xFF				/* CP_DPRAM_MODE */
#define IRQ_QSC_ACTIVE          IRQ_EINT15			/* CP_DPRAM_MODE */


#define GPIO_KBC0                S5PV210_GPH2(0)
#define GPIO_KBC0_AF             3

#define GPIO_KBC1                S5PV210_GPH2(1)
#define GPIO_KBC1_AF             3

#define GPIO_nPOWER              S5PV210_GPH2(6)
#define GPIO_N_POWER             S5PV210_GPH2(6)
#define GPIO_N_POWER_AF          2

#define GPIO_JACK_nINT           S5PV210_GPH2(7)
#define GPIO_JACK_nINT_AF        0xF
#define GPIO_JACK_INT_N          S5PV210_GPH2(7)
#define GPIO_JACK_INT_N_AF       0xFF
   
#define GPIO_KBR0                S5PV210_GPH3(0)
#define GPIO_KBR0_AF             3

#define GPIO_VT_CAM_RST         S5PV210_GPH3(1)
#define GPIO_VT_CAM_RST_AF      3

#define GPIO_VT_CAM_EN          S5PV210_GPH3(2)
#define GPIO_VT_CAM_EN_AF       3

#define GPIO_KBR3                S5PV210_GPH3(3)
#define GPIO_KBR3_AF             3

#define S5PV210_GPH35            S5PV210_GPH3(5)

#define GPIO_T_FLASH_DETECT      S5PV210_GPH3(7)

#define GPIO_CODEC_I2S_CLK       S5PV210_GPI(0)
#define GPIO_CODEC_I2S_CLK_AF    2

#define GPIO_GPI1                S5PV210_GPI(1)

#define GPIO_CODEC_I2S_WS        S5PV210_GPI(2)
#define GPIO_CODEC_I2S_WS_AF     2

#define GPIO_CODEC_I3S_DI        S5PV210_GPI(3)
#define GPIO_CODEC_I3S_DI_AF     2

#define GPIO_CODEC_I3S_DO        S5PV210_GPI(4)
#define GPIO_CODEC_I3S_DO_AF     2

#define GPIO_GPI5                S5PV210_GPI(5)
#define GPIO_MSENSE_SCL_28V      S5PV210_GPI(5)

#define GPIO_GPI6                S5PV210_GPI(6)
#define GPIO_MSENSE_SDA_28V      S5PV210_GPI(6)

#define GPIO_POPUP_SW_EN         S5PV210_GPJ0(0)
#define GPIO_BUCK_1_EN_B         S5PV210_GPJ0(1)
#define GPIO_CAM_FLASH_SET       S5PV210_GPJ0(2)

#define GPIO_CAM_FLASH_EN        S5PV210_GPJ0(7)


#define GPIO_PHONE_ON			S5PV210_GPJ1(0)		/* CP_DPRAM_MODE */
#define GPIO_QSC_PHONE_ON       GPIO_PHONE_ON		/* CP_DPRAM_MODE */

#define GPIO_VIBTONE_EN1         S5PV210_GPJ1(1)

#define GPIO_MICBIAS_EN          S5PV210_GPJ1(2)
#if 0
/* FORTE & CHIEF : EARMIC_1.8V(LDO12:earmic bias) && POPUP_SW_EN(GPIO_)
   ATLAS         : EARMIC_BIAS_EN(regulator on)   && ..  */
#define GPIO_EAR_MICBIAS_EN      S5PV210_GPJ1(2)

#define GPIO_EAR_BIAS_EN         S5PV210_GPJ1(2)
#define GPIO_EAR_BIAS_EN_AF      1
#endif
#define GPIO_TOUCH_EN            S5PV210_GPJ1(3)
#define GPIO_TOUCH_EN_AF         1

#define _3_GPIO_TOUCH_EN         S5PV210_GPJ1(3)
#define _3_GPIO_TOUCH_EN_AF      1

//#define _3_GPIO_TOUCH_CE       S5PV210_GPJ2(6)
//#define _3_GPIO_TOUCH_ST_AF    1

#if 0
#define GPIO_USB_SDA_28V         S5PV210_GPD1(2)
#define GPIO_USB_SCL_28V         S5PV210_GPD1(3)
#endif

#define GPIO_WC_DETECT			GPIO_DUMMY

#define GPIO_CP_EBI2_AD0        S5PV210_GPJ2(0)		/* CP_DPRAM_MODE */
#define GPIO_CP_EBI2_AD1        S5PV210_GPJ2(1)		/* CP_DPRAM_MODE */
#define GPIO_CP_EBI2_AD2        S5PV210_GPJ2(2)		/* CP_DPRAM_MODE */
#define GPIO_CP_EBI2_AD3        S5PV210_GPJ2(3)		/* CP_DPRAM_MODE */
#define GPIO_CP_EBI2_AD4        S5PV210_GPJ2(4)		/* CP_DPRAM_MODE */
#define GPIO_CP_EBI2_AD5        S5PV210_GPJ2(5)		/* CP_DPRAM_MODE */
#define GPIO_CP_EBI2_AD6        S5PV210_GPJ2(6)		/* CP_DPRAM_MODE */
#define GPIO_CP_EBI2_AD7        S5PV210_GPJ2(7)		/* CP_DPRAM_MODE */
#define GPIO_CP_EBI2_AD8        S5PV210_GPJ3(0)		/* CP_DPRAM_MODE */
#define GPIO_CP_EBI2_AD9        S5PV210_GPJ3(1)		/* CP_DPRAM_MODE */
#define GPIO_CP_EBI2_AD10       S5PV210_GPJ3(2)		/* CP_DPRAM_MODE */
#define GPIO_CP_EBI2_AD11       S5PV210_GPJ3(3)		/* CP_DPRAM_MODE */
#define GPIO_CP_EBI2_AD12       S5PV210_GPJ3(4)		/* CP_DPRAM_MODE */
#define GPIO_CP_EBI2_AD13       S5PV210_GPJ3(5)		/* CP_DPRAM_MODE */
#define GPIO_CP_EBI2_AD14       S5PV210_GPJ3(6)		/* CP_DPRAM_MODE */
#define GPIO_CP_EBI2_AD15       S5PV210_GPJ3(7)		/* CP_DPRAM_MODE */

#define GPIO_CP_DPRAM_CS_N      S5PV210_GPJ4(0)		/* CP_DPRAM_MODE */
#define GPIO_CP_EBI2_WE_N       S5PV210_GPJ4(1)		/* CP_DPRAM_MODE */
#define GPIO_CP_EBI2_OE_N       S5PV210_GPJ4(2)		/* CP_DPRAM_MODE */
#define GPIO_DPRAM_INT_CP_N     S5PV210_GPJ4(3)		/* CP_DPRAM_MODE */
#define GPIO_CP_EBI2_ADV_N      S5PV210_GPJ4(4)		/* CP_DPRAM_MODE */

#define GPIO_MP010               S5PV210_MP01(0)

#define GPIO_MAIN_KEY_LED_EN     S5PV210_MP01(0)

#define GPIO_DISPLAY_CS          S5PV210_MP01(1)
#define GPIO_DISPLAY_CS_AF       S3C_GPIO_SFN(1)

// heatup - recheck - none
//#define GPIO_SUB_DISPLAY_CS    S5PV210_MP01(2)
#define GPIO_UART_SEL1           S5PV210_MP01(3)
#define GPIO_AP_NANDCS           S5PV210_MP01(4)
#define GPIO_AP_NANDCS_AF        5

#define GPIO_MP016               S5PV210_MP01(6)

#define GPIO_MP017               S5PV210_MP01(7)

#define GPIO_PS_ON               S5PV210_MP02(0)
//#define GPIO_MP020               S5PV210_MP02(0)

// heatup - recheck - chief - GPIO_USB_SEL
//#define GPIO_USB_SW              S5PV210_MP02(1)
//#define GPIO_MP021               S5PV210_MP02(1)
#define GPIO_USB_SEL             S5PV210_MP02(1)

#define GPIO_VCC_19V_PDA         S5PV210_MP02(2)

#define GPIO_MP023               S5PV210_MP02(3)

#define GPIO_MP030               S5PV210_MP03(0)

#define GPIO_MP031               S5PV210_MP03(1)

#define GPIO_MP032               S5PV210_MP03(2)

#define GPIO_PDA_ACTIVE         S5PV210_MP03(3)		/* CP_DPRAM_MODE */
#define GPIO_PDA_ACTIVE_AF      1					/* CP_DPRAM_MODE */

#define GPIO_VCC_18V_PDA         S5PV210_MP03(4)

#define GPIO_MP036               S5PV210_MP03(6)

#define GPIO_MAIN_KEY_LED_NEW    S5PV210_MP04(0)

#define GPIO_MP042               S5PV210_MP04(2)

#define GPIO_MP044               S5PV210_MP04(4)

// heatup - recheck - none
//#define GPIO_LVDS_RST          S5PV210_MP04(5)
//#define GPIO_MHL_RST           S5PV210_MP04(7)

#define FUEL_SCL_18V             S5PV210_MP05(0)

#define FUEL_SDA_18V             S5PV210_MP05(1)

#define GPIO_AP_SDA_18V          S5PV210_MP05(2)

#define GPIO_AP_SCL_18V          S5PV210_MP05(3)

#define GPIO_MP054               S5PV210_MP05(4)

#define GPIO_MP056               S5PV210_MP05(6)

#define GPIO_UART_SEL            S5PV210_MP05(7)

#define GPIO_GPS_UART_RXD        GPIO_DUMMY
#define GPIO_GPS_UART_RXD_AF     2

#define GPIO_GPS_RXD             GPIO_DUMMY
#define GPIO_GPS_RXD_AF          2

#define GPIO_GPS_UART_TXD        GPIO_DUMMY
#define GPIO_GPS_UART_TXD_AF     2

#define GPIO_GPS_TXD             GPIO_DUMMY
#define GPIO_GPS_TXD_AF          2

#define GPIO_GPS_UART_CTS        GPIO_DUMMY
#define GPIO_GPS_UART_CTS_AF     2

#define GPIO_GPS_CTS             GPIO_DUMMY
#define GPIO_GPS_CTS_AF          2

#define GPIO_GPS_UART_RTS        GPIO_DUMMY
#define GPIO_GPS_UART_RTS_AF     2

#define GPIO_GPS_RTS             GPIO_DUMMY
#define GPIO_GPS_RTS_AF          2

#define GPIO_FLM_RXD             GPIO_DUMMY
#define GPIO_FLM_RXD_AF          2

#define GPIO_AP_FLM_RXD          GPIO_DUMMY
#define GPIO_AP_FLM_RXD_AF       2

#define GPIO_FLM_TXD             GPIO_DUMMY
#define GPIO_FLM_TXD_AF          2

#define GPIO_AP_FLM_TXD          GPIO_DUMMY
#define GPIO_AP_FLM_TXD_AF       2




#define GPIO_VIBTONE_PWM1        GPIO_DUMMY
#define GPIO_GPS_nRST            GPIO_DUMMY
#define GPIO_GPS_PWR_EN          GPIO_DUMMY
#define GPIO_GPS_CLK_INT         GPIO_DUMMY
#define GPIO_BATT_ID             GPIO_DUMMY

// heatup - recheck - chief - none -->
#define GPIO_HWREV_MODE0         GPIO_DUMMY
#define GPIO_HWREV_MODE1         GPIO_DUMMY
#define GPIO_HWREV_MODE2         GPIO_DUMMY
#define GPIO_HWREV_MODE3         GPIO_DUMMY

#define GPIO_OLED_DET            GPIO_DUMMY
#define GPIO_CODEC_XTAL_EN       GPIO_DUMMY
#define GPIO_FM_INT              GPIO_DUMMY
#define GPIO_FM_RST              GPIO_DUMMY
// <---

#define GPIO_TV_EN               GPIO_DUMMY
#define GPIO_OLED_ID             GPIO_DUMMY
#define GPIO_DIC_ID              GPIO_DUMMY
#define GPIO_PCM_SEL             GPIO_DUMMY
#define GPIO_GPS_CLK_EN          GPIO_DUMMY

//#define GPIO_FM_SDA_28V        S5PV210_GPG3(0)
//#define GPIO_FM_SCL_28V        S5PV210_GPG3(2)
#define NFC_SDA_18V              S5PV210_GPIO_END
#define NFC_SCL_18V              S5PV210_GPIO_END
#define NFC_IRQ                  S5PV210_GPIO_END
#define NFC_EN                   S5PV210_GPIO_END
#define NFC_FIRM                 S5PV210_GPIO_END
#define GPIO_KBC2                S5PV210_GPIO_END
// to be checked
#define GPIO_DPRAM_INT_N		S5PV210_GPH1(1)	
#define GPIO_DPRAM_INT_N_AF		0xFF



//DUMMY
//#define GPIO_CAM_SDA_29V    GPIO_DUMMY  //S5PV210_GPD1(0)
//#define GPIO_CAM_SCL_29V    GPIO_DUMMY  //S5PV210_GPD1(1)
#define GPIO_CAM_3M_nRST    GPIO_DUMMY //__NC__
#define GPIO_CAM_3M_nSTBY   GPIO_DUMMY //__NC__
#define GPIO_WLAN_RST      GPIO_DUMMY  //S5PV210_GPG1(2)
#define GPIO_WLAN_EN       GPIO_DUMMY  //__NC__
#define GPIO_WLAN_EN_AF      GPIO_DUMMY  //__NC__

#define GPIO_FLM_RXD			GPIO_DUMMY
#define GPIO_FLM_RXD_AF			2

#define GPIO_TSP_LDO_ON      GPIO_DUMMY   //__NC__
#define GPIO_PHONE_ON_REV00   GPIO_DUMMY  //__NC__
#endif
/* end of __GPIO_ARIES_H_ */

