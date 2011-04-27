/*
 * MHD_SiI9234.c - Driver for Silicon Image MHD SiI9234 Transmitter driver
 *
 * Copyright 2010  Philju Lee (Daniel Lee)
 *
 * Based on preview driver from Silicon Image.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 */

static int MYDRV_MAJOR;

static int MHDDRV_MAJOR;

static bool tclkStable;
static bool mobileHdCableConnected;
static bool hdmiCableConnected;
static bool dsRxPoweredUp;
static byte tmdsPoweredUp;
static byte txPowerState;
static bool checkTclkStable;


/*=======================================================================================*/

void InitCBusRegs(void);
byte ReadIndexedRegister (byte PageNum, byte Offset);
bool delay_ms(int msec);
void MHD_HW_Reset(void);
void I2C_WriteByte(byte deviceID, byte offset, byte value);
byte I2C_ReadByte(byte deviceID, byte offset);
byte ReadByteTPI(byte Offset);
void WriteByteTPI(byte Offset, byte Data);
void ReadModifyWriteTPI(byte Offset, byte Mask, byte Data); 
void WriteIndexedRegister (byte PageNum, byte Offset, byte Data);
static void sii9234_initializeStateVariables (void);
bool siI9234_startTPI(void);
void sii9234_initial_registers_set(void);
void sii9234_enable_interrupts(void);
byte ReadByteCBUS(byte Offset);
void WriteByteCBUS(byte Offset, byte Data);
byte ReadIndexedRegister (byte PageNum, byte Offset);
void ReadModifyWriteIndexedRegister (byte PageNum, byte Offset, byte Mask, byte Data);
void TxPowerStateD3(void);
void DisableInterrupts(void);
void EnableTMDS(void);
static void TxPowerStateD0 (void); 
static void EnableInterrupts (void); 
void CheckTxFifoStable (void); 
void HotPlugService (void) ;
static void WakeUpFromD3 (void) ;
void ReadModifyWriteCBUS(byte Offset, byte Mask, byte Value) ;
static void OnMHDCableConnected (void) ;
void ForceUsbIdSwitchOpen (void) ;
static void ReleaseUsbIdSwitchOpen (void) ;
void DisableTMDS (void) ;
void OnDownstreamRxPoweredDown (void) ;
void OnHdmiCableDisconnected (void) ;
void OnDownstreamRxPoweredUp (void) ;
void OnHdmiCableConnected (void) ;
void sii9234_tpi_init(void);
void sii9234_polling(void);
void GoToD3 (void);
/*=======================================================================================*/


bool delay_ms(int msec)
{
  mdelay(msec);
  return 0;
}

void MHD_HW_Reset(void)
{
  printk("[SIMG]MHD_HW_Reset == Start == \n"); 
  MHL_HW_Reset();
  printk("[SIMG] MHD_HW_Reset == End == \n");   
}

void I2C_WriteByte(byte deviceID, byte offset, byte value)
{
	if(deviceID == 0x72)
		SII9234_i2c_write(SII9234_i2c_client,offset,value);
	else if(deviceID == 0x7A)
		SII9234_i2c_write(SII9234A_i2c_client,offset,value);
	else if(deviceID == 0x92)
		SII9234_i2c_write(SII9234B_i2c_client,offset,value);
	else if(deviceID == 0xC8)
		SII9234_i2c_write(SII9234C_i2c_client,offset,value);
	else
		printk("[MHL]I2C_WriteByte error %x\n",deviceID); 
}

byte I2C_ReadByte(byte deviceID, byte offset)
{
  byte number = 0;
  //printk("[MHL]I2C_ReadByte called ID%x Offset%x\n",deviceID,offset);
  	if(deviceID == 0x72)
		number = SII9234_i2c_read(SII9234_i2c_client,offset);
	else if(deviceID == 0x7A)
		number = SII9234_i2c_read(SII9234A_i2c_client,offset);
	else if(deviceID == 0x92)
		number = SII9234_i2c_read(SII9234B_i2c_client,offset);
	else if(deviceID == 0xC8)
		number = SII9234_i2c_read(SII9234C_i2c_client,offset);
	else
		printk("[MHL]I2C_ReadByte error %x\n",deviceID); 
	printk("[MHL]I2C_ReadByte ID:%x Offset:%x data:%x\n",deviceID,offset,number); 
    return (number);
}

byte ReadByteTPI (byte Offset) 
{
	return I2C_ReadByte(TPI_SLAVE_ADDR, Offset);
}

void WriteByteTPI (byte Offset, byte Data) 
{
	I2C_WriteByte(TPI_SLAVE_ADDR, Offset, Data);
}

void ReadModifyWriteTPI(byte Offset, byte Mask, byte Data) 
{

	byte Temp;

	Temp = ReadByteTPI(Offset);		// Read the current value of the register.
	Temp &= ~Mask;					// Clear the bits that are set in Mask.
	Temp |= (Data & Mask);			// OR in new value. Apply Mask to Value for safety.
	WriteByteTPI(Offset, Temp);		// Write new value back to register.
}


void WriteIndexedRegister (byte PageNum, byte Offset, byte Data) 
{
	WriteByteTPI(TPI_INDEXED_PAGE_REG, PageNum);		// Indexed page
	WriteByteTPI(TPI_INDEXED_OFFSET_REG, Offset);		// Indexed register
	WriteByteTPI(TPI_INDEXED_VALUE_REG, Data);			// Write value
}


static void sii9234_initializeStateVariables (void) 
{

	tclkStable = FALSE;
	checkTclkStable = TRUE;
	tmdsPoweredUp = FALSE;
	mobileHdCableConnected = FALSE;
	hdmiCableConnected = FALSE;
	dsRxPoweredUp = FALSE;
}

bool siI9234_startTPI(void) 
{

	byte devID = 0x00;
	word wID = 0x0000;

	WriteByteTPI(TPI_ENABLE, 0x00);				// Write "0" to 72:C7 to start HW TPI mode
	delay_ms(100);

	devID = ReadIndexedRegister(0x00, 0x03);
	wID = devID;
	wID <<= 8;
	devID = ReadIndexedRegister(0x00, 0x02);
	wID |= devID;

	devID = ReadByteTPI(TPI_DEVICE_ID);

	printk ("[SIMG] SiI %04X\n", (int) wID);

	if (devID == SiI_DEVICE_ID) {
		return TRUE;
		}

	printk ("[SIMG] Unsupported TX\n");
	return FALSE;
}

void InitCBusRegs(void) 
{
	I2C_WriteByte(0xC8, 0x1F, 0x02); 			// Heartbeat Max Fail Enable
	I2C_WriteByte(0xC8, 0x07, DDC_XLTN_TIMEOUT_MAX_VAL | 0x06); 			// Increase DDC translation layer timer
	I2C_WriteByte(0xC8, 0x40, 0x03); 			// CBUS Drive Strength
	I2C_WriteByte(0xC8, 0x42, 0x06); 			// CBUS DDC interface ignore segment pointer
	I2C_WriteByte(0xC8, 0x36, 0x0C);
	I2C_WriteByte(0xC8, 0x44, 0x02);
	I2C_WriteByte(0xC8, 0x09, 0x60);			// Enable PVC Xfer aborted / follower aborted
}

void sii9234_initial_registers_set(void)
{

	printk ("==[SIMG] sii9234_initial_registers_set Start ==\n");
#if 0
	// Power Up
	I2C_WriteByte(0x7A, 0x3D, 0x3F);			// Power up CVCC 1.2V core
	I2C_WriteByte(0x92, 0x11, 0x01);			// Enable TxPLL Clock
	I2C_WriteByte(0x92, 0x12, 0x15);			// Enable Tx Clock Path & Equalizer
	I2C_WriteByte(0x72, 0x08, 0x35);			// Power Up TMDS Tx Core

	// Analog PLL Control
	I2C_WriteByte(0x92, 0x17, 0x03);			// PLL Calrefsel
	I2C_WriteByte(0x92, 0x1A, 0x20);			// VCO Cal
	I2C_WriteByte(0x92, 0x22, 0x8A);			// Auto EQ
	I2C_WriteByte(0x92, 0x23, 0x6A);			// Auto EQ
	I2C_WriteByte(0x92, 0x24, 0xAA);			// Auto EQ
	I2C_WriteByte(0x92, 0x25, 0xCA);			// Auto EQ
	I2C_WriteByte(0x92, 0x26, 0xEA);			// Auto EQ
	I2C_WriteByte(0x92, 0x4C, 0xA0);			// Manual zone control
	I2C_WriteByte(0x92, 0x4D, 0x00);			// PLL Mode Value

	I2C_WriteByte(0x72, 0x80, 0x14);			// Enable Rx PLL Clock Value
	I2C_WriteByte(0x92, 0x45, 0x44);			// Rx PLL BW value from I2C
	I2C_WriteByte(0x92, 0x31, 0x0A);			// Rx PLL BW ~ 4MHz
	I2C_WriteByte(0x72, 0xA0, 0xD0);
	I2C_WriteByte(0x72, 0xA1, 0xFC);			// Disable internal Mobile HD driver

	//I2C_WriteByte(0x72, 0xA3, 0xFA);
 	I2C_WriteByte(0x72, 0xA3, 0xFF);  //amplitude max

	I2C_WriteByte(0x72, 0x2B, 0x01);			// Enable HDCP Compliance workaround

  I2C_WriteByte(0x72, 0x90, 0x26);	
  
  I2C_WriteByte(0x72, 0x91, 0xE5);		// Skip RGND detection

	I2C_WriteByte(0x72, 0x94, 0x66);			// 1.8V CBUS VTH & GND threshold

	//set bit 2 and 3, which is Initiator Timeout
	I2C_WriteByte(CBUS_SLAVE_ADDR, 0x31, I2C_ReadByte(CBUS_SLAVE_ADDR, 0x31) | 0x0c);
	I2C_WriteByte(0x72, 0xA5, 0x00);			// RGND Hysterisis.

	I2C_WriteByte(0x72, 0x95, 0x31);			// RGND & single discovery attempt (RGND blocking)

	I2C_WriteByte(0x72, 0x96, 0x22);			// use 1K and 2K setting
//	I2C_WriteByte(0x72, 0x97, 0x03);			// Auto Heartbeat failure enable

	ReadModifyWriteTPI(0x95, SI_BIT_6, SI_BIT_6);		// Force USB ID switch to open

	WriteByteTPI(0x92, 0x86);				//
	WriteByteTPI(0x93, 0xCC);				// Disable CBUS pull-up during RGND measurement		
	

	delay_ms(25);
	ReadModifyWriteTPI(0x95, SI_BIT_6, 0x00);		// Release USB ID switch

	I2C_WriteByte(0x72, 0x90, 0x27);			// Enable CBUS discovery

	InitCBusRegs();

	I2C_WriteByte(0x72, 0x05, ASR_VALUE); 		// Enable Auto soft reset on SCDT = 0

	I2C_WriteByte(0x72, 0x0D, 0x1C); 			// HDMI Transcode mode enable
#endif

#if 1 //old cable	
		// Power Up
		I2C_WriteByte(0x7A, 0x3D, 0x3F);			// Power up CVCC 1.2V core
		I2C_WriteByte(0x92, 0x11, 0x01);			// Enable TxPLL Clock
		I2C_WriteByte(0x92, 0x12, 0x15);			// Enable Tx Clock Path & Equalizer
		I2C_WriteByte(0x72, 0x08, 0x35);			// Power Up TMDS Tx Core	
	
	// Analog PLL Control
		I2C_WriteByte(0x92, 0x17, 0x03);			// PLL Calrefsel
		I2C_WriteByte(0x92, 0x1A, 0x20);			// VCO Cal
		I2C_WriteByte(0x92, 0x22, 0x8A);			// Auto EQ
		I2C_WriteByte(0x92, 0x23, 0x6A);			// Auto EQ
		I2C_WriteByte(0x92, 0x24, 0xAA);			// Auto EQ
		I2C_WriteByte(0x92, 0x25, 0xCA);			// Auto EQ
		I2C_WriteByte(0x92, 0x26, 0xEA);			// Auto EQ
		I2C_WriteByte(0x92, 0x4C, 0xA0);			// Manual zone control
		I2C_WriteByte(0x92, 0x4D, 0x00);			// PLL Mode Value
	
		I2C_WriteByte(0x72, 0x80, 0x14);			// Enable Rx PLL Clock Value	
		I2C_WriteByte(0x92, 0x45, 0x44);			// Rx PLL BW value from I2C
		I2C_WriteByte(0x92, 0x31, 0x0A);			// Rx PLL BW ~ 4MHz
		I2C_WriteByte(0x72, 0xA1, 0xFC);			// Disable internal Mobile HD driver	
		I2C_WriteByte(0x72, 0xA3, 0xFF);         //AMP
	  I2C_WriteByte(0x72, 0x2B, 0x01);			// Enable HDCP Compliance workaround  
	  I2C_WriteByte(0x72, 0x91, 0xE5);		// Skip RGND detection	
		I2C_WriteByte(0x72, 0xA5, 0x00);			// RGND Hysterisis.
	
	  
		I2C_WriteByte(0x72, 0x90, 0x27);			// Enable CBUS discovery
		//I2C_WriteByte(0x72, 0x05, ASR_VALUE);		// Enable Auto soft reset on SCDT = 0	
		I2C_WriteByte(0x72, 0x0D, 0x1C);			// HDMI Transcode mode enable
	
	  WriteByteTPI(TPI_ENABLE, 0x00);	
	
	  delay_ms(100); 
		WriteIndexedRegister(INDEXED_PAGE_0, 0xA0, 0x10);  
		delay_ms(100); 
	  ReadModifyWriteTPI(TPI_SYSTEM_CONTROL_DATA_REG, TMDS_OUTPUT_CONTROL_MASK, TMDS_OUTPUT_CONTROL_ACTIVE);  
#endif	
#if 0  //test pattern generate
	  WriteByteTPI(0xBC, 0x81); 
	
	

	  
		I2C_WriteByte(0x72, 0xBD, 0x01);		// Enable Auto soft reset on SCDT = 0	
		I2C_WriteByte(0x72, 0xBB, 0x1D);	  
#endif

#if 0 //new MHL cable
	  // Power Up
	  I2C_WriteByte(0x7A, 0x3D, 0x3F);			  // Power up CVCC 1.2V core
	//I2C_WriteByte(0x7A, 0x3D, 0x37);			  // Power up CVCC 1.2V core	  
	
	I2C_WriteByte(0x92, 0x11, 0x01);		  // Enable TxPLL Clock
	  I2C_WriteByte(0x92, 0x12, 0x15);			  // Enable Tx Clock Path & Equalizer
	
	  I2C_WriteByte(0x72, 0x08, 0x35);			  // Power Up TMDS Tx Core
	//I2C_WriteByte(0x72, 0x08, 0x37);			  // Power Up TMDS Tx Core	  
	
	  // Analog PLL Control
	  I2C_WriteByte(0x92, 0x17, 0x03);			  // PLL Calrefsel
	  I2C_WriteByte(0x92, 0x1A, 0x20);			  // VCO Cal
	  I2C_WriteByte(0x92, 0x22, 0x8A);			  // Auto EQ
	  I2C_WriteByte(0x92, 0x23, 0x6A);			  // Auto EQ
	  I2C_WriteByte(0x92, 0x24, 0xAA);			  // Auto EQ
	  I2C_WriteByte(0x92, 0x25, 0xCA);			  // Auto EQ
	  I2C_WriteByte(0x92, 0x26, 0xEA);			  // Auto EQ
	  I2C_WriteByte(0x92, 0x4C, 0xA0);			  // Manual zone control
	
	//I2C_WriteByte(0x92, 0x1C, 0x11); //daniel RX0_offset test 
	//I2C_WriteByte(0x92, 0x1D, 0x11); //daniel RX1_offset	  test		  
	//I2C_WriteByte(0x92, 0x1E, 0x11); //daniel RX2_offset	  test		  
	
	  I2C_WriteByte(0x92, 0x4D, 0x00);			  // PLL Mode Value
	  
	  I2C_WriteByte(0x72, 0x80, 0x14);			  // Enable Rx PLL Clock Value
	  //I2C_WriteByte(0x72, 0x80, 0x24);		  // Enable Rx PLL Clock Value	  
	//I2C_WriteByte(0x72, 0x80, 0x34);			  // Enable Rx PLL Clock Value	  
	
	  I2C_WriteByte(0x92, 0x45, 0x44);			  // Rx PLL BW value from I2C
	  I2C_WriteByte(0x92, 0x31, 0x0A);			  // Rx PLL BW ~ 4MHz
	  I2C_WriteByte(0x72, 0xA0, 0xD0);
	  I2C_WriteByte(0x72, 0xA1, 0xFC);			  // Disable internal Mobile HD driver
	
	  I2C_WriteByte(0x72, 0xA3, 0xFF);
	  I2C_WriteByte(0x72, 0x2B, 0x01);			  // Enable HDCP Compliance workaround
	
	  // CBUS & Discovery
	  //ReadModifyWriteTPI(0x90, BIT_3 | BIT_2, BIT_3);   // CBUS discovery cycle time for each drive and float = 150us
	
	  I2C_WriteByte(0x72, 0x91, 0xE5);		  // Skip RGND detection
	  I2C_WriteByte(0x72, 0x94, 0x66);			  // 1.8V CBUS VTH & GND threshold
	
	  //set bit 2 and 3, which is Initiator Timeout
	  //I2C_WriteByte(CBUS_SLAVE_ADDR, 0x31, I2C_ReadByte(CBUS_SLAVE_ADDR, 0x31) | 0x0c);
	
	  I2C_WriteByte(0x72, 0xA5, 0x00);			  // RGND Hysterisis.
	  I2C_WriteByte(0x72, 0x95, 0x31);			  // RGND & single discovery attempt (RGND blocking)
	  I2C_WriteByte(0x72, 0x96, 0x22);			  // use 1K and 2K setting
	
	  ReadModifyWriteTPI(0x95, SI_BIT_6, SI_BIT_6);	  // Force USB ID switch to open
	
	WriteByteTPI(0x92, 0x46);			  // Force MHD mode
	  WriteByteTPI(0x93, 0xDC); 			  // Disable CBUS pull-up during RGND measurement

	//old cable
//WriteByteTPI(0x92, 0x86);				// Force MHD mode
//WriteByteTPI(0x93, 0xCC);				// Disable CBUS pull-up during RGND measurement
	  
	  delay_ms(25);
	  ReadModifyWriteTPI(0x95, SI_BIT_6, 0x00);	  // Release USB ID switch
	
	  I2C_WriteByte(0x72, 0x90, 0x27);			  // Enable CBUS discovery
	
	  //InitCBusRegs();
	
	  I2C_WriteByte(0x72, 0x05, 0x04);		  // Enable Auto soft reset on SCDT = 0
	
	  I2C_WriteByte(0x72, 0x0D, 0x1C);			  // HDMI Transcode mode enable
	
	WriteByteTPI(TPI_ENABLE, 0x00);   
	
	delay_ms(100); 
	  WriteIndexedRegister(INDEXED_PAGE_0, 0xA0, 0x10);  
	  //WriteByteCBUS(0x07, DDC_XLTN_TIMEOUT_MAX_VAL | 0x0E);	  // Increase DDC translation layer timer (burst mode)
	  //WriteByteCBUS(0x47, 0x03);	
	  //WriteByteCBUS(0x21, 0x01); // Heartbeat Disable
	  
	  delay_ms(100); 
	ReadModifyWriteTPI(TPI_SYSTEM_CONTROL_DATA_REG, TMDS_OUTPUT_CONTROL_MASK, TMDS_OUTPUT_CONTROL_ACTIVE);	

#endif

#if 0  //0607 update
// Power Up
I2C_WriteByte(0x7A, 0x3D, 0x3F);			// Power up CVCC 1.2V core

I2C_WriteByte(0x92, 0x11, 0x01);			// Enable TxPLL Clock
I2C_WriteByte(0x92, 0x12, 0x15);			// Enable Tx Clock Path & Equalizer

I2C_WriteByte(0x72, 0x08, 0x35);			// Power Up TMDS Tx Core

I2C_WriteByte(0x92, 0x00, 0x00);			// SIMG: correcting HW default
I2C_WriteByte(0x92, 0x13, 0x60);			// SIMG: Set termination value
I2C_WriteByte(0x92, 0x14, 0xF0);			// SIMG: Change CKDT level
I2C_WriteByte(0x92, 0x4B, 0x06);			// SIMG: Correcting HW default

// Analog PLL Control
I2C_WriteByte(0x92, 0x17, 0x07);			// SIMG: PLL Calrefsel
I2C_WriteByte(0x92, 0x1A, 0x20);			// VCO Cal
I2C_WriteByte(0x92, 0x22, 0xE0);			// SIMG: Auto EQ
I2C_WriteByte(0x92, 0x23, 0xC0);			// SIMG: Auto EQ
I2C_WriteByte(0x92, 0x24, 0xA0);			// SIMG: Auto EQ
I2C_WriteByte(0x92, 0x25, 0x80);			// SIMG: Auto EQ
I2C_WriteByte(0x92, 0x26, 0x60);			// SIMG: Auto EQ
I2C_WriteByte(0x92, 0x27, 0x40);			// SIMG: Auto EQ
I2C_WriteByte(0x92, 0x28, 0x20);			// SIMG: Auto EQ
I2C_WriteByte(0x92, 0x29, 0x00);			// SIMG: Auto EQ

I2C_WriteByte(0x92, 0x4D, 0x02);			// SIMG: PLL Mode Value (order is important)
I2C_WriteByte(0x92, 0x4C, 0xA0);			// Manual zone control


I2C_WriteByte(0x72, 0x80, 0x14);			// Enable Rx PLL Clock Value

I2C_WriteByte(0x92, 0x31, 0x0B);			// SIMG: Rx PLL BW value from I2C BW ~ 4MHz
I2C_WriteByte(0x92, 0x45, 0x06);			// SIMG: DPLL Mode
I2C_WriteByte(0x72, 0xA0, 0x10);			// SIMG: Term mode
I2C_WriteByte(0x72, 0xA1, 0xFC);			// Disable internal Mobile HD driver

I2C_WriteByte(0x72, 0xA3, 0xEB);			// SIMG: Output Swing
I2C_WriteByte(0x72, 0xA6, 0x0C);			// SIMG: Swing Offset

I2C_WriteByte(0x72, 0x2B, 0x01);			// Enable HDCP Compliance workaround

// CBUS & Discovery
//ReadModifyWriteTPI(0x90, BIT_3 | BIT_2, BIT_3);	// CBUS discovery cycle time for each drive and float = 150us

I2C_WriteByte(0x72, 0x91, 0xE5);		// Skip RGND detection
I2C_WriteByte(0x72, 0x94, 0x66);			// 1.8V CBUS VTH & GND threshold

//set bit 2 and 3, which is Initiator Timeout
//I2C_WriteByte(CBUS_SLAVE_ADDR, 0x31, I2C_ReadByte(CBUS_SLAVE_ADDR, 0x31) | 0x0c);

I2C_WriteByte(0x72, 0xA5, 0x80);			// SIMG: RGND Hysterisis, 3x mode for Beast
I2C_WriteByte(0x72, 0x95, 0x31);			// RGND & single discovery attempt (RGND blocking)
I2C_WriteByte(0x72, 0x96, 0x22);			// use 1K and 2K setting

ReadModifyWriteTPI(0x95, SI_BIT_6, SI_BIT_6); 	// Force USB ID switch to open

WriteByteTPI(0x92, 0x46);				// Force MHD mode
WriteByteTPI(0x93, 0xDC);				// Disable CBUS pull-up during RGND measurement

//old cable
//WriteByteTPI(0x92, 0x86);				// Force MHD mode
//WriteByteTPI(0x93, 0xCC);				// Disable CBUS pull-up during RGND measurement


delay_ms(25);
ReadModifyWriteTPI(0x95, SI_BIT_6, 0x00);		// Release USB ID switch

I2C_WriteByte(0x72, 0x90, 0x27);			// Enable CBUS discovery

//InitCBusRegs();

I2C_WriteByte(0x72, 0x05, 0x04);		// Enable Auto soft reset on SCDT = 0

I2C_WriteByte(0x72, 0x0D, 0x1C);			// HDMI Transcode mode enable

WriteByteTPI(TPI_ENABLE, 0x00); 

delay_ms(100); 
WriteIndexedRegister(INDEXED_PAGE_0, 0xA0, 0x10);  
//WriteByteCBUS(0x07, DDC_XLTN_TIMEOUT_MAX_VAL | 0x0E); 	// Increase DDC translation layer timer (burst mode)
//WriteByteCBUS(0x47, 0x03);  
//WriteByteCBUS(0x21, 0x01); // Heartbeat Disable

delay_ms(100); 
ReadModifyWriteTPI(TPI_SYSTEM_CONTROL_DATA_REG, TMDS_OUTPUT_CONTROL_MASK, TMDS_OUTPUT_CONTROL_ACTIVE);	

	
#endif
	printk ("==[SIMG] sii9234_initial_registers_set END ==\n");	
}



void sii9234_enable_interrupts(void)
{
	ReadModifyWriteTPI(TPI_INTERRUPT_ENABLE_REG, HOT_PLUG_EVENT_MASK, HOT_PLUG_EVENT_MASK);
	WriteIndexedRegister(INDEXED_PAGE_0, 0x75, SI_BIT_5);	// Enable   
}

byte ReadByteCBUS (byte Offset) 
{
	return I2C_ReadByte(CBUS_SLAVE_ADDR, Offset);
}


void WriteByteCBUS(byte Offset, byte Data) 
{
	I2C_WriteByte(CBUS_SLAVE_ADDR, Offset, Data);
}

byte ReadIndexedRegister (byte PageNum, byte Offset) 
{
	WriteByteTPI(TPI_INDEXED_PAGE_REG, PageNum);		// Indexed page
	WriteByteTPI(TPI_INDEXED_OFFSET_REG, Offset);		// Indexed register
	return ReadByteTPI(TPI_INDEXED_VALUE_REG);			// Return read value
}

void ReadModifyWriteIndexedRegister (byte PageNum, byte Offset, byte Mask, byte Data) 
{

	byte Temp;

	Temp = ReadIndexedRegister (PageNum, Offset);	// Read the current value of the register.
	Temp &= ~Mask;									// Clear the bits that are set in Mask.
	Temp |= (Data & Mask);							// OR in new value. Apply Mask to Value for safety.
	WriteByteTPI(TPI_INDEXED_VALUE_REG, Temp);		// Write new value back to register.
}


void TxPowerStateD3 (void) 
{

	ReadModifyWriteIndexedRegister(INDEXED_PAGE_1, 0x3D, SI_BIT_0, 0x00);
	printk("[SIMG] TX Power State D3\n");
	txPowerState = TX_POWER_STATE_D3;
}

void DisableInterrupts (void) 
{

	ReadModifyWriteTPI(TPI_INTERRUPT_ENABLE_REG, HOT_PLUG_EVENT_MASK, 0x00);
}


void EnableTMDS (void) 
{

	printk("[SIMG] TMDS -> Enabled\n");
	ReadModifyWriteTPI(TPI_SYSTEM_CONTROL_DATA_REG, TMDS_OUTPUT_CONTROL_MASK, TMDS_OUTPUT_CONTROL_ACTIVE);
	tmdsPoweredUp = TRUE;
}

static void TxPowerStateD0 (void) 
{

	ReadModifyWriteTPI(TPI_DEVICE_POWER_STATE_CTRL_REG, TX_POWER_STATE_MASK, 0x00);
	TPI_DEBUG_PRINT(("[SIMG] TX Power State D0\n"));
	txPowerState = TX_POWER_STATE_D0;
}

static void EnableInterrupts (void) 
{

	ReadModifyWriteTPI(TPI_INTERRUPT_ENABLE_REG, HOT_PLUG_EVENT_MASK, HOT_PLUG_EVENT_MASK);
	WriteIndexedRegister(INDEXED_PAGE_0, 0x75, SI_BIT_5);	// Enable 
}

void CheckTxFifoStable (void) 
{

	byte bTemp;

	bTemp = ReadIndexedRegister(INDEXED_PAGE_0, 0x3E);			
	if ((bTemp & (SI_BIT_7 | SI_BIT_6)) != 0x00) 
  {
		TPI_DEBUG_PRINT(("[SIMG] FIFO Overrun / Underrun\n"));
		WriteIndexedRegister(INDEXED_PAGE_0, 0x05, SI_BIT_4 | ASR_VALUE);	// Assert MHD FIFO Reset
		delay_ms(1);
		WriteIndexedRegister(INDEXED_PAGE_0, 0x05, ASR_VALUE);			// Deassert MHD FIFO Reset
	}
}
void HotPlugService (void) 
{

	DisableInterrupts();
	EnableTMDS();
	TxPowerStateD0();
	EnableInterrupts();
	CheckTxFifoStable();
}


static void WakeUpFromD3 (void) 
{

	TPI_DEBUG_PRINT(("[SIMG] Waking up...\n"));
  sii9234_tpi_init();
}


void ReadModifyWriteCBUS(byte Offset, byte Mask, byte Value) 
{
  byte Temp;

  Temp = ReadByteCBUS(Offset);
  Temp &= ~Mask;
  Temp |= (Value & Mask);
  WriteByteCBUS(Offset, Temp);
}

void OnMHDCableConnected (void) 
{

	TPI_DEBUG_PRINT (("[SIMG] MHD Connected\n"));

	if (txPowerState == TX_POWER_STATE_D3) 
  {
	    siI9234_startTPI();
	    EnableInterrupts();
	    TxPowerStateD0();
	}

	mobileHdCableConnected = TRUE;

	WriteIndexedRegister(INDEXED_PAGE_0, 0xA0, 0x10);

	TPI_DEBUG_PRINT (("[SIMG] Setting DDC Burst Mode\n"));
	WriteByteCBUS(0x07, DDC_XLTN_TIMEOUT_MAX_VAL | 0x0E); 	// Increase DDC translation layer timer (burst mode)
	WriteByteCBUS(0x47, 0x03);  

 	WriteByteCBUS(0x21, 0x01); // Heartbeat Disable
 	
}


void ForceUsbIdSwitchOpen (void) 
{
	ReadModifyWriteIndexedRegister(INDEXED_PAGE_0, 0x90, SI_BIT_0, 0x00);				// Disable discovery
	ReadModifyWriteIndexedRegister(INDEXED_PAGE_0, 0x95, SI_BIT_6, SI_BIT_6);				// Force USB ID Switch
	WriteIndexedRegister(INDEXED_PAGE_0, 0x92, 0x46);							// Force MHD mode
	ReadModifyWriteIndexedRegister(INDEXED_PAGE_0, 0x79, SI_BIT_5 | SI_BIT_4, SI_BIT_4);		// Force HPD to 0 when not in MHD mode.
}


static void ReleaseUsbIdSwitchOpen (void) 
{
	delay_ms(25);
	ReadModifyWriteIndexedRegister(INDEXED_PAGE_0, 0x95, SI_BIT_6, 0x00);
	ReadModifyWriteIndexedRegister(INDEXED_PAGE_0, 0x90, SI_BIT_0, SI_BIT_0);				// Enable discovery
}

static void InitForceUsbIdSwitchOpen (void) 
{
	I2C_WriteByte(0x72, 0x90, 0x26);					// Disable CBUS discovery
	ReadModifyWriteTPI(0x95, SI_BIT_6, SI_BIT_6);				// Force USB ID switch to open
	ReadModifyWriteTPI(0x95, SI_BIT_6, SI_BIT_6);				// Force USB ID switch to open
  WriteByteTPI(0x92, 0x46);						// Force MHD mode
}


static void InitReleaseUsbIdSwitchOpen (void) 
{
	delay_ms(25);
	ReadModifyWriteTPI(0x95, SI_BIT_6, 0x00);				// Release USB ID switch
	ReadModifyWriteTPI(0x90, SI_BIT_0, SI_BIT_0);				// Enable CBUS discovery
}

void DisableTMDS (void) 
{

	TPI_DEBUG_PRINT(("[SIMG] TMDS -> Disabled\n"));
	ReadModifyWriteTPI(TPI_SYSTEM_CONTROL_DATA_REG, TMDS_OUTPUT_CONTROL_MASK, TMDS_OUTPUT_CONTROL_POWER_DOWN);
	tmdsPoweredUp = FALSE;
}

void OnDownstreamRxPoweredDown (void) 
{

	TPI_DEBUG_PRINT (("[SIMG] DSRX -> Powered Down\n"));

	dsRxPoweredUp = FALSE;
	DisableTMDS();
}

void OnHdmiCableDisconnected (void) 
{

	TPI_DEBUG_PRINT (("[SIMG] HDMI Disconnected\n"));

	hdmiCableConnected = FALSE;
	OnDownstreamRxPoweredDown();
}

void OnDownstreamRxPoweredUp (void) 
{

	TPI_DEBUG_PRINT (("[SIMG] DSRX -> Powered Up\n"));

	dsRxPoweredUp = TRUE;
	HotPlugService();
}

void OnHdmiCableConnected (void) 
{

	TPI_DEBUG_PRINT (("[SIMG] HDMI Connected\n"));

	ReadModifyWriteIndexedRegister(INDEXED_PAGE_0, 0x79, SI_BIT_4, 0x00);		// Un-force HPD

	hdmiCableConnected = TRUE;
	OnDownstreamRxPoweredUp();		// RX power not determinable? Force to on for now.
}

void sii9234_tpi_init(void)
{
  sii9234_initializeStateVariables();
  MHD_HW_Reset();
  sii9234_initial_registers_set();  
  //siI9234_startTPI();
  //sii9234_enable_interrupts();
}

void GoToD3 (void) 
{

	TxPowerStateD3();

	mobileHdCableConnected = FALSE;
	hdmiCableConnected = FALSE;
	dsRxPoweredUp = FALSE;
}

void sii9234_polling(void)
{

	byte InterruptStatusImage = NULL;
	byte cBusInt= NULL;
	byte intsHandled= NULL;
	byte bTemp= NULL, i= NULL, tclkStableCount= NULL, overUnderRunCount= NULL;
	byte bPost= NULL;//, bPost2;

	intsHandled = 0;
  
	if (txPowerState == TX_POWER_STATE_D0) 
  {

		bTemp = ReadIndexedRegister(INDEXED_PAGE_0, 0x09);

    {
      static byte temp =0;
      if(temp != bTemp)
      {
        temp = bTemp;
        TPI_DEBUG_PRINT(("[SIMG] (INDEXED_PAGE_0, 0x09):[%x] \n",(int)temp));
      }
    }   

		if ((bTemp & SI_BIT_2) == 0x00) 
    {

			TPI_DEBUG_PRINT(("[SIMG] RSEN Detected Low\n"));
      GoToD3 ();
			return;
		}

		cBusInt = ReadIndexedRegister(INDEXED_PAGE_0, 0x72);

    {
      static byte temp =0;
      if(temp != cBusInt)
      {
        temp = cBusInt;
        TPI_DEBUG_PRINT(("[SIMG] (INDEXED_PAGE_0, 0x72):[%x] \n",(int)temp));
      }
    }

    if (cBusInt & SI_BIT_1) 
    {

			WriteIndexedRegister(INDEXED_PAGE_0, 0x72, SI_BIT_1);

			if (checkTclkStable == TRUE) 
      {

				TPI_DEBUG_PRINT(("[SIMG] TClk Stable Change\n"));

				while (1) 
        {

					tclkStableCount = 0;
          
					for (i = 0; i < 5; i++) 
          {
						bTemp = ReadIndexedRegister(INDEXED_PAGE_0, 0x09);
						if (bTemp & SI_BIT_0) 
            {
							tclkStableCount++;
						}
					}

					// If tclk stable low or stable high, do nothing. Otherwise, disable, enable TX
					if (tclkStableCount != 0 && tclkStableCount != 5) 
          {
						TPI_DEBUG_PRINT(("[SIMG] TClk Unstable\n"));
						ReadModifyWriteIndexedRegister(INDEXED_PAGE_0, 0x80, SI_BIT_4, 0x00);
						delay_ms(10);
						ReadModifyWriteIndexedRegister(INDEXED_PAGE_0, 0x80, SI_BIT_4, SI_BIT_4);
					}

					tclkStableCount = 0;
          
					for (i = 0; i < 5; i++) 
          {
						bTemp = ReadIndexedRegister(INDEXED_PAGE_0, 0x09);
            
						if (bTemp & SI_BIT_0) 
            {
							tclkStableCount++;
						}
					}

					if (tclkStableCount == 0) 
          {
						TPI_DEBUG_PRINT(("[SIMG] TClk Stable Low\n"));
					}
					else 
          {
						TPI_DEBUG_PRINT(("[SIMG] TClk Stable High\n"));
						tclkStable = TRUE;

						do {

							TPI_DEBUG_PRINT(("[SIMG] Applying SWRST\n"));
							WriteIndexedRegister(INDEXED_PAGE_0, 0x05, ASR_VALUE | SI_BIT_0);	// Assert SWRST
							delay_ms(100);
							WriteIndexedRegister(INDEXED_PAGE_0, 0x05, ASR_VALUE);			// Deassert SWRST

							for (i = 0, overUnderRunCount = 0; i < 5; i++) 
              {
								bTemp = ReadIndexedRegister(INDEXED_PAGE_0, 0x3E);
								if ((bTemp & (SI_BIT_7 | SI_BIT_6)) != 0x00) 
                {
									overUnderRunCount++;
								}
							}

							if (overUnderRunCount != 0) 
              {
								TPI_DEBUG_PRINT(("[SIMG] FIFO Overrun/underrun\n"));
							}

						} while ((overUnderRunCount != 0));

						delay_ms(10);
            
						checkTclkStable = FALSE;
						break;
					}
				}
			}
		}

		InterruptStatusImage = ReadByteTPI(TPI_INTERRUPT_STATUS_REG);

    {
      static byte temp =0;
      if(temp != InterruptStatusImage)
      {
        temp = InterruptStatusImage;
        TPI_DEBUG_PRINT(("[SIMG] InterruptStatusImage:[%x] \n",(int)InterruptStatusImage));
      }
    }
    
		if (InterruptStatusImage == NON_MASKABLE_INT) 
    {               // Check if NMI has occurred
			TPI_DEBUG_PRINT (("[SIMG] TP -> NMI Detected\n"));
			sii9234_tpi_init();																// Reset and re-initialize
			HotPlugService();
			return;
		}


		if (mobileHdCableConnected == FALSE) 
    {
				TPI_DEBUG_PRINT(("[SIMG] Mobile HD Mode Established 1\n"));
				OnMHDCableConnected();
		}

		if (InterruptStatusImage & HOT_PLUG_EVENT_MASK) 
    {

			TPI_DEBUG_PRINT(("[SIMG] HOT_PLUG_EVENT_MASK \n"));
      
			cBusInt = ReadIndexedRegister(INDEXED_PAGE_0, 0x74);

			if (cBusInt & 0x04) 
      {                   // Mobile HD Mode Established
				TPI_DEBUG_PRINT(("[SIMG] Mobile HD Mode Established 2\n"));
				if (mobileHdCableConnected == FALSE)
        {
					OnMHDCableConnected();
				}
				intsHandled |= 0x04;
			}

			if (cBusInt & 0x08) 
      {                   // USB Mode Established
				TPI_DEBUG_PRINT(("[SIMG] USB Mode Established\n"));
				delay_ms(100);
				intsHandled |= 0x08;
			}

			if (cBusInt & 0x10) 
      {                   // CBus Lockout
				TPI_DEBUG_PRINT(("[SIMG] CBus Lockout\n"));
		
				WriteIndexedRegister(INDEXED_PAGE_0, 0x74, 0x10);					// Clear lockout interrupts
				ForceUsbIdSwitchOpen();
				ReleaseUsbIdSwitchOpen();
			}

			if (cBusInt & 0x40) 
      {                   // RGND Detection
				TPI_DEBUG_PRINT(("[SIMG] RGND Detection\n"));

				bTemp = ReadIndexedRegister(0x00, 0x99);
				TPI_DEBUG_PRINT(("[SIMG] [99] -> %02X\n", (int)bTemp));

				bTemp &= 0x03;
				if (bTemp == 0x00 || bTemp == 0x03)
        {
					ReadModifyWriteIndexedRegister(INDEXED_PAGE_0, 0x95, SI_BIT_5, 0x00);
				}
				else 
        {
					ReadModifyWriteIndexedRegister(INDEXED_PAGE_0, 0x95, SI_BIT_5, SI_BIT_5);
				}

				intsHandled |= 0x40;
			}

			if (intsHandled != 0x00) 
      {
				WriteIndexedRegister(INDEXED_PAGE_0, 0x74, intsHandled);			// Clear interrupts that were handled
			}

			WriteByteTPI (TPI_INTERRUPT_STATUS_REG, InterruptStatusImage & ~HOT_PLUG_EVENT_MASK);	// Clear this interrupt.
		}


		InterruptStatusImage = ReadByteCBUS(0x0D);

    TPI_DEBUG_PRINT(("[SIMG] (0x0D)InterruptStatusImage:[%x] \n",(int)InterruptStatusImage));
    
		if (((InterruptStatusImage & SI_BIT_6) >> 6) != hdmiCableConnected)  //original
    {

			if (hdmiCableConnected == TRUE) 
      {
      	TPI_DEBUG_PRINT(("[SIMG]Call OnHdmiCableDisconnected \n"));      
				OnHdmiCableDisconnected();
			}
			else 
      {
				if (mobileHdCableConnected == TRUE) 
        {           
        	TPI_DEBUG_PRINT(("[SIMG]Call OnHdmiCableConnected \n"));             
					OnHdmiCableConnected();
				}
			}
		}

		if (hdmiCableConnected == FALSE) 
    {
			return;
		}

		CheckTxFifoStable();
	}
	else if (txPowerState == TX_POWER_STATE_D3) 
  {

  	TPI_DEBUG_PRINT(("[SIMG] txPowerState == TX_POWER_STATE_D3)\n"));

#if 0
		cBusInt = ReadByteTPI(0x74);

		if (cBusInt & 0x04) 
    {                   // Mobile HD Mode Established

			TPI_DEBUG_PRINT(("[SIMG] Mobile HD Mode Established (D3)\n"));
			WriteByteTPI(0x74, 0x04);								// Clear interrupt
			if (mobileHdCableConnected == FALSE) 
      {
        OnMHDCableConnected();
			}

		}

		if (cBusInt & 0x08) 
    {                   // USB Mode Established
			TPI_DEBUG_PRINT(("[SIMG] USB Mode Established (D3)\n"));
			delay_ms(100);
			WriteByteTPI(0x74, 0x08);								// Clear interrupt
			return;
		}

		if (cBusInt & 0x10) 
    {                   // CBus Lockout
			TPI_DEBUG_PRINT(("[SIMG] CBus Lockout (D3)\n"));

			WriteByteTPI(0x74, 0x10);								// Clear lockout interrupts

			InitForceUsbIdSwitchOpen();
			InitReleaseUsbIdSwitchOpen();
		}

		if (cBusInt & 0x40) 
    {                   // RGND Detection

			TPI_DEBUG_PRINT(("[SIMG] RGND Detection (D3)\n"));

			bTemp = ReadByteTPI(0x99);
			TPI_DEBUG_PRINT(("[SIMG] [99] -> %02X\n", (int)bTemp));

			bTemp &= 0x03;
			if (bTemp == 0x00 || bTemp == 0x03)
      {
				WakeUpFromD3();
				WriteByteTPI(0x90, 0x25);					// Force Power State to ON
				ReadModifyWriteTPI(0x95, SI_BIT_5, 0x00);
			}
			else 
      {
				WakeUpFromD3();
				WriteByteTPI(0x90, 0x25);					// Force Power State to ON
				ReadModifyWriteTPI(0x95, SI_BIT_5, SI_BIT_5);
			}

			WriteByteTPI(0x74, 0x40);					// Clear this interrupt

		}
#endif 		
	}
}

