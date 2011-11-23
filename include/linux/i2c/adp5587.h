#ifndef KEY_EXPANDER_ADP5587_H
#define KEY_EXPANDER_ADP5587_H


#define ADP5587_I2C_WRITE_ADDR              0x68
#define ADP5587_I2C_READ_ADDR               0x69


/* ADP5587 Register */
// Device ID
#define ADP5587_REG_DEV_ID                  0x00

// Configuration Register 1
#define ADP5587_REG_CFG                     0x01

// Interrupt Status Register
#define ADP5587_REG_INT_STAT                0x02

// Keylock and event counter register
#define ADP5587_REG_KEY_LCK_EC_STAT         0x03

// Key Event Register A~H
#define ADP5587_REG_KEY_EVENTA              0x04
#define ADP5587_REG_KEY_EVENTB              0x05
#define ADP5587_REG_KEY_EVENTC              0x06
#define ADP5587_REG_KEY_EVENTD              0x07
#define ADP5587_REG_KEY_EVENTE              0x08
#define ADP5587_REG_KEY_EVENTF              0x09
#define ADP5587_REG_KEY_EVENTG              0x0A
#define ADP5587_REG_KEY_EVENTH              0x0B
#define ADP5587_REG_KEY_EVENTI              0x0C
#define ADP5587_REG_KEY_EVENTJ              0x0D

// Keypad Unlock 1 to Keypad Unlock 2 timer
#define ADP5587_REG_KP_LCK_TMR              0x0E

// Unlock Key 1
#define ADP5587_REG_UNLOCK1                 0x0F

// Unlock Key 2
#define ADP5587_REG_UNLOCK2                 0x10

// GPIO interrupt status
#define ADP5587_REG_GPIO_INT_STAT1          0x11
#define ADP5587_REG_GPIO_INT_STAT2          0x12
#define ADP5587_REG_GPIO_INT_STAT3          0x13

// GPIO data status, read twice to clear
#define ADP5587_REG_DAT_STAT1               0x14
#define ADP5587_REG_DAT_STAT2               0x15
#define ADP5587_REG_DAT_STAT3               0x16

// GPIO data out
#define ADP5587_REG_DAT_OUT1                0x17
#define ADP5587_REG_DAT_OUT2                0x18
#define ADP5587_REG_DAT_OUT3                0x19

// GPIO interrupt enable
#define ADP5587_REG_INT_EN1                 0x1A
#define ADP5587_REG_INT_EN2                 0x1B
#define ADP5587_REG_INT_EN3                 0x1C

// Keypad or GPIO selection
#define ADP5587_REG_KP_GPIO1                0x1D
#define ADP5587_REG_KP_GPIO2                0x1E
#define ADP5587_REG_KP_GPIO3                0x1F

// GPI Event Mode
#define ADP5587_REG_GPI_EM_REG1             0x20
#define ADP5587_REG_GPI_EM_REG2             0x21
#define ADP5587_REG_GPI_EM_REG3             0x22

// GPIO data direction
#define ADP5587_REG_GPIO_DIR1               0x23
#define ADP5587_REG_GPIO_DIR2               0x24
#define ADP5587_REG_GPIO_DIR3               0x25

// GPIO edge/level detect
#define ADP5587_REG_GPIO_INT_LVL1           0x26
#define ADP5587_REG_GPIO_INT_LVL2           0x27
#define ADP5587_REG_GPIO_INT_LVL3           0x28

// Debounce disable
#define ADP5587_REG_DEBOUNCE_DIS1           0x29
#define ADP5587_REG_DEBOUNCE_DIS2           0x2A
#define ADP5587_REG_DEBOUNCE_DIS3           0x2B

// GPIO pull disable
#define ADP5587_REG_GPIO_PULL1              0x2C
#define ADP5587_REG_GPIO_PULL2              0x2D
#define ADP5587_REG_GPIO_PULL3              0x2E

#define ADP5587_REG_MAX                     0xFF

// GPIO
#define KEY_INT                 40
#define KEY_RST                 72
#define KEY_SCL                 73
#define KEY_SDA                 74


struct adp5587_kpad_platform_data {
	/* code map for the keys */
  unsigned int rows;
  unsigned int cols;
  unsigned int en_keylock;
  unsigned int unlock_key1;
  unsigned int unlock_key2;
	unsigned int *keymap;
	unsigned int keymapsize;
  unsigned int repeat;
#if defined(CONFIG_MACH_AEGIS) || defined(CONFIG_MACH_VIPER) || defined(CONFIG_MACH_CHIEF)
	int lid_gpio;
	int lid_irq;
#endif
};


#endif /* KEY_EXPANDER_ADP5588_H */
