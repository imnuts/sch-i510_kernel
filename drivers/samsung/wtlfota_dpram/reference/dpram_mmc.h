/* 
 * Copyright (C) 2008, 2009,  Samsung Electronics Co. Ltd. All Rights Reserved.
 *       Written by Linux Lab, MITs Development Team, Mobile Communication Division.
 */

//------------------------------------------------------------------------------
//
//  File:  DPRAM.h
//
//	- ported to linux by geunyoung on 2008.04.23.
//------------------------------------------------------------------------------

#ifndef __DPRAM_MMC_H__
#define __DPRAM_MMC_H__

// type definitions. - by geunyoung.
//typedef int BOOL;
//typedef unsigned long DWORD;
//typedef unsigned short WORD;
//typedef unsigned short USHORT;
//typedef unsigned char BYTE;
//typedef unsigned char byte;

/*
    - Total Length : 16K
    
      0     1     2     3     4                               3FFC  3FFD  3FFE  3FFF
     忙式式成式式成式式成式式成式式式式式式式式式式式式式式式成式式成式式成式式成式式忖
     弛    弛    弛    弛    弛    ...... Buffer ......      弛    弛    弛    弛    弛
     戌式式扛式式扛式式扛式式扛式式式式式式式式式式式式式式式扛式式扛式式扛式式扛式式戎

    戌式式式式式式式式式戎								 戌式式式式式扛式式式式式戎
			     ∟												   ∟		   ∟
             Magic Code										   PDA2PHONE    PHONE2PDA
			     
*/

#define DPRAM_START_ADDRESS							0
//LDS 080121 MAGIC CODE address of Mirage is shifted by 0x1110. In other project, MAGIC CODE address should be DPRAM start address.
//LDS 080124 MAGIC CODE address of Mirage is not shifted in FOTA mode

#if 0
#define DPRAM_MAGIC_CODE_ADDRESS					(DPRAM_START_ADDRESS + 0x1110)
#else
#define DPRAM_MAGIC_CODE_ADDRESS					DPRAM_START_ADDRESS
#endif

//#define DPRAM_MAGIC_CODE_OFFSET_MIRAGE			0x1110
//#define DPRAM_MAGIC_CODE_ADDRESS					DPRAM_START_ADDRESS + DPRAM_MAGIC_CODE_OFFSET_MIRAGE
#define DPRAM_MAGIC_CODE_SIZE						0x4
#define DPRAM_PDA2PHONE_FORMATTED_START_ADDRESS		(DPRAM_START_ADDRESS + DPRAM_MAGIC_CODE_SIZE)

//#define DPRAM_BUFFER_SIZE							(DPRAM_PDA2PHONE_INTERRUPT_ADDRESS - DPRAM_PDA2PHONE_FORMATTED_START_ADDRESS)
#define DPRAM_BUFFER_SIZE							(DPRAM_PHONE2PDA_INTERRUPT_ADDRESS - DPRAM_PDA2PHONE_FORMATTED_START_ADDRESS)                       
// #define DELTA_PACKET_SIZE						(DPRAM_BUFFER_SIZE - 8)

//#define DPRAM_END_OF_ADDRESS						0x3FFF
//#define BSP_DPRAM_BASE_SIZE						(DPRAM_END_OF_ADDRESS + 1)
#define BSP_DPRAM_BASE_SIZE	                        0x4000	//16KB DPRAM in Mirage
#define DPRAM_END_OF_ADDRESS						BSP_DPRAM_BASE_SIZE -1

#define DPRAM_INTERRUPT_SIZE						0x2
//#define DPRAM_PDA2PHONE_INTERRUPT_ADDRESS			(DPRAM_START_ADDRESS + 0x3FFE)
#define DPRAM_PDA2PHONE_INTERRUPT_ADDRESS			(DPRAM_START_ADDRESS + BSP_DPRAM_BASE_SIZE -  DPRAM_INTERRUPT_SIZE*2)
//Modified by sun.
#define DPRAM_PHONE2PDA_INTERRUPT_ADDRESS			(DPRAM_START_ADDRESS + BSP_DPRAM_BASE_SIZE - DPRAM_INTERRUPT_SIZE)

//#define DPRAM_INDEX_SIZE							0x1
#define DPRAM_INDEX_SIZE							0x2

#define CACHED_TO_UNCACHED_OFFSET					0x20000000L
#define DPRAM_REGS_BASE_PHYSICAL					0x00000000
#define DPRAM_REGS_BASE_C_VIRTUAL					0x97C00000
#define DPRAM_REGS_BASE_U_VIRTUAL					(DPRAM_REGS_BASE_C_VIRTUAL + CACHED_TO_UNCACHED_OFFSET)
#define DPRAM_BASE_U_VIRTUAL						DPRAM_REGS_BASE_U_VIRTUAL	// Base Address (Uncached)

#if 0
#define GPIO_PHONE_POWER							81
//#define GPIO_PHONE_ACTIVE							107 // defined in nowplus.h
#define GPIO_PHONE_RESET							102
#else
#define GPIO_PHONE_POWER							GPIO_FONE_ON
#define GPIO_PHONE_RESET							GPIO_MSM_RST
#endif

#define MAGIC_FODN									0x4E444F46	// 0x464F444E	// PDA initiate phone code
#define MAGIC_DMDL									0x4445444C
#define MAGIC_ALARMBOOT								0x00410042			

/*
// Phone on
#define __PHONE_TURN_ON__(pReg)		\
{									\
    pReg->GPCR0 = GPIO_PHONE_POWER; \
    pReg->GPSR1 = GPIO_PHONE_RESET; \
    Sleep(400); \
    pReg->GPSR0 = GPIO_PHONE_POWER; \
    Sleep(250); \
    pReg->GPCR0 = GPIO_PHONE_POWER; \
}
*/
// Phone off
// 1. Set ON_SW low
// 2. Delay 1.5 sec
// 3. Set ON_SW high
/*
#define __PHONE_TURN_OFF__(pReg)    \
{ \
	pReg->gpsr2 = GPIO_PHONE_POWER; \
	Sleep(1500); \
	pReg->gpcr2 = GPIO_PHONE_POWER; \
}
*/
/*
// Phone reset
// 1. Set PCF5213_RST low
// 2. Delay 12 msec
// 3. Set PCF213_RST high
#define __PHONE_RESET__(pReg)		\
{									\
    pReg->GPCR1 = GPIO_PHONE_RESET; \
    Sleep(12);						\
    pReg->GPSR1 = GPIO_PHONE_RESET; \
}
*/
/*
#define IS_PHONE_ACTIVE(pReg) (pReg->gplr3 & GPIO_PHONE_ACTIVE)
*/

// interupt masks
#define MASK_CMD_VALID								0x8000
//#define MASK_RESULT								0x4000
#define MASK_PDA_CMD								0x1000
#define MASK_PHONE_CMD							    0x2000
//#define MASK_CMD									0x0F00
//#define MASK_RESP									0x00FF

#define MASK_CMD_RECEIVE_READY_NOTIFICATION			0x0100	
#define MASK_CMD_DOWNLOAD_START_REQUEST             0x0200
#define MASK_CMD_DOWNLOAD_START_RESPONSE            0x0300
#define MASK_CMD_IMAGE_SEND_REQUEST                 0x0400
#define MASK_CMD_IMAGE_SEND_RESPONSE                0x0500
#define MASK_CMD_SEND_DONE_REQUEST                  0x0600
#define MASK_CMD_SEND_DONE_RESPONSE                 0x0700
#define MASK_CMD_STATUS_UPDATE_NOTIFICATION         0x0800
#define MASK_CMD_UPDATE_DONE_NOTIFICATION           0x0900
#define MASK_CMD_EFS_CLEAR_RESPONSE					0x0B00
#define MASK_CMD_ALARM_BOOT_OK						0x0C00
#define MASK_CMD_ALARM_BOOT_FAIL					0x0D00

// Result mask
#define MASK_CMD_RESULT_FAIL                        0x0002
#define MASK_CMD_RESULT_SUCCESS                     0x0001

#define START_INDEX									0x007F
#define END_INDEX									0x007E

// FORMAT
#define CMD_RECEIVE_READY_NOTI                      0xA100
#define CMD_DL_START_REQ                            0x9200
#define CMD_DL_START_RESP                           0xA301
#define CMD_IMG_SEND_REQ                            0x9400
#define CMD_IMG_SEND_RESP                           0xA500
#define CMD_DL_SEND_DONE_REQ                        0x9600
#define CMD_DL_SEND_DONE_REQ_BOOT_UPDATE            0x9601
#define CMD_DL_SEND_DONE_RESP                       0xA700
#define CMD_UP_STATUS_NOTI                          0xA800
#define CMD_UP_DONE_NOTI                            0xA900
#define CMD_ALARM_BOOT_OK							0xAC00
#define CMD_ALARM_BOOT_FAIL							0xAD00
#define CMD_SEND_MODEM_POWEROFF                     0x9E00

#define CMD_EFS_CLEAR_REQ                           0x9A00

// spec擊 �挫�.
#define WRITEIMG_HEADER_SIZE					    8
#define WRITEIMG_TAIL_SIZE							4
#define WRITEIMG_BODY_SIZE							(DPRAM_BUFFER_SIZE - WRITEIMG_HEADER_SIZE - WRITEIMG_TAIL_SIZE )

#define DPDN_DEFAULT_WRITE_LEN						WRITEIMG_BODY_SIZE
//#define DPDN_DEFAULT_WRITE_LEN					(DPRAM_BUFFER_SIZE - 10)

/*
#define CMD_WRITE_REQ								0x9200
#define CMD_WRITE_DONE								0x9400
*/

typedef struct s_Packet {
	u8	bBOP;
	u16	TotFrameNo;
	u16	CurFrameNo;
	u8*	Data;
	u16  Crc;
	u8	bEOP;
} Packet, *pPacket;

typedef struct S_Req {
    u16  DownMode;
    u32   StartAddress;
    u32   dwLength;
} Req_Packet, *pReq_Packet;    

typedef struct Up_Noti{
    u16 Bop;
    u16 Region;
    u16   Percent;    
    u16 Eop;
} Status_UpNoti, *pStatus_UpNoti;    

/*
void  dpram_init(void);
void  dpram_write_magic_code(u32 code);
void  dpram_write_done_request(void);

//BOOL  DPRAM_PhonePowerOFF(void);
//void  DPRAM_PhoneReset(void);
u32 dpram_write_image(const u8* pBuf, u32 dwWriteLen);
BOOL  DPRAM_ReadUpdateInfo(u32* dwCur, u32* dwTot, u8* StateCode);

extern PVOID VirtualAllocCopy(unsigned size,char *str,PVOID pVirtualAddress);
void PhoneReset();
BOOL PhonePowerOn();
BOOL PhonePowerOff();

u32	DPRAM_Read(void* pDestAddr, void* pSrcAddr, u32 dwDestSize);
u32	DPRAM_Write(void* pDestAddr, void* pSrcAddr, u32 dwSrcSize);
u32   Phone_On();
u32   Phone_Off();
BOOL    DPRAM_WaitForReady();
u16  CalcTotFrame(u16 nDividend,u16 nDivisor);
//u32   DPRAM_DownloadStart((u8 *)pBufIn, u32 Len);*/
#if 0
u32	DPRAM_Read(void* pDestAddr, void* pSrcAddr, u32 dwDestSize);
u32	DPRAM_Write(void* pDestAddr, void* pSrcAddr, u32 dwSrcSize);
BOOL    DPRAM_Init(void);
u32   Phone_On();
u32   Phone_Off();
void    DPRAM_WriteMagicCode(u32 dwCode);
u32   DPRAM_WriteImage(u8* const pBuf, u32 dwWriteLen);
void  DPRAM_WriteDoneRequest(void);
void	DPRAM_WriteCommand(u16 nCmd);
u16	DPRAM_ReadCommand(void);

BOOL    DPRAM_WaitReadyPhase();
BOOL    DPRAM_WriteStartPhase(Req_Packet *pStartReq);
BOOL    DPRAM_SendDonePhase();
u32   DPRAM_StatusNotiPhase(u8 *pPercent);
BOOL    DPRAM_UpDoneNotiPhase();
u32   DPRAM_DownloadControl(u8 *pBufIn, u32 Len);
BOOL    DPRAM_Download(u8 *pBufIn, u32 dwSize);
//u16  CalcTotFrame(u16 nDividend, u16 nDivisor);
u16 CalcTotFrame(u32 nDividend, u16 nDivisor);
#endif
#endif	/* __DPRAM_MMC_H__ */

