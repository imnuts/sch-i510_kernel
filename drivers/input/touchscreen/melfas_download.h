//--------------------------------------------------------
//
//    MELFAS Firmware download base code for MCS6000
//    Version : v01
//    Date    : 2009.01.20
//
//--------------------------------------------------------

#ifndef __MELFAS_FIRMWARE_DOWNLOAD_H__
#define __MELFAS_FIRMWARE_DOWNLOAD_H__

//=====================================================================
//
//   MELFAS Firmware download pharameters
//
//=====================================================================

#if 0
//----------------------------------------------------
//   Return values of download function
//----------------------------------------------------
#define MCSDL_RET_SUCCESS                            0x00
#define MCSDL_RET_ENTER_DOWNLOAD_MODE_FAILED        0x01
#define MCSDL_RET_ERASE_FLASH_FAILED                0x02
#define MCSDL_RET_PREPARE_ERASE_FLASH_FAILED        0x0B
#define MCSDL_RET_ERASE_VERIFY_FAILED                0x03
#define MCSDL_RET_READ_FLASH_FAILED                    0x04
#define MCSDL_RET_READ_EEPROM_FAILED                0x05
#define MCSDL_RET_READ_INFORMAION_FAILED            0x06
#define MCSDL_RET_PROGRAM_FLASH_FAILED                0x07
#define MCSDL_RET_PROGRAM_EEPROM_FAILED                0x08
#define MCSDL_RET_PREPARE_PROGRAM_FAILED            0x09
//#define MCSDL_RET_PROGRAM_VERIFY_FAILED                0x0A

#define MCSDL_RET_WRONG_MODE_ERROR                    0xF0
#define MCSDL_RET_WRONG_SLAVE_SELECTION_ERROR        0xF1
#define MCSDL_RET_WRONG_PARAMETER                    0xF2
#define MCSDL_RET_COMMUNICATION_FAILED                0xF3
//#define MCSDL_RET_READING_HEXFILE_FAILED            0xF4
//#define MCSDL_RET_FILE_ACCESS_FAILED                0xF5
//#define MCSDL_RET_MELLOC_FAILED                        0xF6
//#define MCSDL_RET_WRONG_MODULE_REVISION                0xF7
#endif

//=====================================================================
//
//   MELFAS Firmware download pharameters
//
//=====================================================================

#define MELFAS_TRANSFER_LENGTH					((32/8)*32)		// Fixed value
#define MELFAS_FIRMWARE_MAX_SIZE				(32*1024) // 32 -> 35 eunsuk test for cief

//----------------------------------------------------
//   Return values of download function
//----------------------------------------------------
#define MCSDL_RET_SUCCESS						0x00
#define MCSDL_RET_ERASE_FLASH_VERIFY_FAILED		0x01
#define MCSDL_RET_PROGRAM_VERIFY_FAILED			0x02

#define MCSDL_RET_PROGRAM_SIZE_IS_WRONG			0x10
#define MCSDL_RET_VERIFY_SIZE_IS_WRONG			0x11
#define MCSDL_RET_WRONG_BINARY					0x12

#define MCSDL_RET_READING_HEXFILE_FAILED		0x21
#define MCSDL_RET_FILE_ACCESS_FAILED			0x22
#define MCSDL_RET_MELLOC_FAILED					0x23

#define MCSDL_RET_WRONG_MODULE_REVISION			0x30

#if 0
//----------------------------------------------------
// ISP commands
//----------------------------------------------------
#define MCSDL_ISP_CMD_ERASE                            0x02
#define MCSDL_ISP_CMD_ERASE_TIMING                    0x0F
#define MCSDL_ISP_CMD_PROGRAM_FLASH                    0x03
#define MCSDL_ISP_CMD_READ_FLASH                    0x04
#define MCSDL_ISP_CMD_PROGRAM_TIMING                0x0F
#define MCSDL_ISP_CMD_READ_INFORMATION                0x06
#define MCSDL_ISP_CMD_RESET                            0x07
#endif

//----------------------------------------------------
//   ISP Mode
//----------------------------------------------------
#define ISP_MODE_ERASE_FLASH					0x01
#define ISP_MODE_SERIAL_WRITE					0x02
#define ISP_MODE_SERIAL_READ					0x03

//----------------------------------------------------
// MCS5000's responses
//----------------------------------------------------
#define MCSDL_ISP_ACK_ERASE_DONE                    0x82
#define MCSDL_ISP_ACK_PREPARE_ERASE_DONE            0x8F
#define MCSDL_I2C_ACK_PREPARE_PROGRAM                0x8F
#define MCSDL_MDS_ACK_PROGRAM_FLASH                    0x83
#define MCSDL_MDS_ACK_READ_FLASH                    0x84
#define MCSDL_MDS_ACK_PROGRAM_INFORMATION            0x88
#define MCSDL_MDS_ACK_PROGRAM_LOCKED                0xFE
#define MCSDL_MDS_ACK_READ_LOCKED                    0xFE
#define MCSDL_MDS_ACK_FAIL                            0xFE

#define MCSDL_ISP_PROGRAM_TIMING_VALUE_0            0x00
#define MCSDL_ISP_PROGRAM_TIMING_VALUE_1            0x00
#define MCSDL_ISP_PROGRAM_TIMING_VALUE_2            0x78

#define MCSDL_ISP_ERASE_TIMING_VALUE_0                0x01
#define MCSDL_ISP_ERASE_TIMING_VALUE_1                0xD4
#define MCSDL_ISP_ERASE_TIMING_VALUE_2                0xC0


//----------------------------------------------------
//    I2C ISP
//----------------------------------------------------

#define MCSDL_I2C_SLAVE_ADDR_ORG                    0x7D                            // Original Address

#define MCSDL_I2C_SLAVE_ADDR_SHIFTED                (MCSDL_I2C_SLAVE_ADDR_ORG<<1)    // Adress after sifting.

#define MCSDL_I2C_SLAVE_READY_STATUS                0x55

#define USE_BASEBAND_I2C_FUNCTION                    1                                // Selection of i2c function ( This must be 1 )

#define MELFAS_USE_PROTOCOL_COMMAND_FOR_DOWNLOAD     0                                // If 'enable download command' is needed ( Pinmap dependent option ).


//----------------------------------------------------
//    MELFAS Version information address
//----------------------------------------------------
#define MCSDL_ADDR_MODULE_REVISION                    0x98
#define MCSDL_ADDR_FIRMWARE_VERSION                   0x9C

//#define MELFAS_TRANSFER_LENGTH                        64                                // Program & Read flash block size



//============================================================
//
//    Port setting. ( Melfas preset this value. )
//
//============================================================

// If want to set Enable : Set to 1

#define MCSDL_USE_CE_CONTROL						0
#define MCSDL_USE_INTR_CONTROL                      1
#define MCSDL_USE_VDD_CONTROL						1
#define MCSDL_USE_RESETB_CONTROL                    1
#define MCSDL_USING_HW_I2C							0
void mcsdl_vdd_on(void);
void mcsdl_vdd_off(void);

//#define GPIO_TOUCH_INT	89
//#define GPIO_I2C0_SCL   88 
//#define GPIO_I2C0_SDA   87 


/* Touch Screen Interface Specification Multi Touch (V0.5) */

/* REGISTERS */
#define MCSTS_STATUS_REG        0x00 //Status
#define MCSTS_MODE_CONTROL_REG  0x01 //Mode Control
#define MCSTS_RESOL_HIGH_REG    0x02 //Resolution(High Byte)
#define MCSTS_RESOL_X_LOW_REG   0x03 //Resolution(X Low Byte)
#define MCSTS_RESOL_Y_LOW_REG   0x04 //Resolution(Y Low Byte)
#define MCSTS_INPUT_INFO_REG    0x10 //Input Information
#define MCSTS_POINT_HIGH_REG    0x11 //Point(High Byte)
#define MCSTS_POINT_X_LOW_REG   0x12 //Point(X Low Byte)
#define MCSTS_POINT_Y_LOW_REG   0x13 //Point(Y Low Byte)
#define MCSTS_STRENGTH_REG      0x14 //Strength
#define MCSTS_MODULE_VER_REG    0x30 //H/W Module Revision
#define MCSTS_FIRMWARE_VER_REG  0x31 //F/W Version



//============================================================
//
//    Porting factors for Baseband
//
//============================================================

#include "melfas_download_porting.h"


//----------------------------------------------------
//    Functions
//----------------------------------------------------

// Current Quattro's TSP IC is not fully capable of generating dual touch points
// So instead, it gives one point and a distance from that point.
// Below feature enables "LIMITED" Multi Touch functionality.
// What it does is when TSP gives a point and a distance, it will generate
// a second point using the distance.
// For example if the received touch information are a touch point of (100,100) 
// and a distance of 50, a second touch point of (100,150) will be reported.
// This just works fine for the pinch-zoom in/out gestures because all it needs
// is a increase or decrease in distance of the two points.
//
// NOTE that it will not work for any other multi-touch gestures!!
//

#define USES_PINCH_DIST_MT      1


int mcsdl_download_binary_data(void);			// with binary type .c   file.
int mcsdl_download_binary_file(void);			// with binary type .bin file.

#if MELFAS_ENABLE_DELAY_TEST                    // For initial porting test.
void mcsdl_delay_test(INT32 nCount);
#endif


#endif        //#ifndef __MELFAS_FIRMWARE_DOWNLOAD_H__

