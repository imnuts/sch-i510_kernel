/*
 * This program is free software; you can redistribute it and/or modify      
 * it under the terms of the GNU General Public License version 2 as         
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/irq.h>
#include <linux/poll.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <mach/regs-gpio.h>
#include <plat/gpio-cfg.h>
#include <mach/hardware.h>

#include <linux/cdev.h>
#include "dpram_recovery.h"

#ifdef CONFIG_PROC_FS
#include <linux/proc_fs.h>
#endif	/* CONFIG_PROC_FS */

//#if defined(CONFIG_S5PV210_VICTORY)
//#include <mach/gpio-victory.h>
//#elif defined(CONFIG_S5PV210_ATLAS)
#include <mach/gpio-chief.h>
//#endif

#define _DEBUG_

#ifdef _DEBUG_
#define MSGCRIT "\x1b[1;31m"
#define MSGERR "\x1b[1;33m"
#define MSGWARN "\x1b[1;35m"
#define MSGINFO "\x1b[1;32m"
#define MSGDBG "\x1b[1;37m"

#define MSGEND "\x1b[0m \n"
#else
#define MSGCRIT 
#define MSGERR 
#define MSGWARN 
#define MSGINFO 
#define MSGDBG 
#define MSGEND 
#endif

#define DRIVER_NAME "/dev/dpram_recovery"
#define MAJOR_NUM 250

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#define MAKEWORD(a, b)      ((u16)(((u8)(a)) | ((u16)((u8)(b))) << 8))
#define MAKELONG(a, b)      ((LONG)(((u16)(a)) | ((u32)((u16)(b))) << 16))
#define LOWORD(l)           ((u16)(l))
#define HIWORD(l)           ((u16)(((u32)(l) >> 16) & 0xFFFF))
#define LOBYTE(w)           ((u8)(w))
#define HIBYTE(w)           ((u8)(((u16)(w) >> 8) & 0xFF))

static u16	SeqIdx;
static u16  g_TotFrame = 0;
static u16  g_CurFrame = 1;
static u16 error_code = 0x0;

struct dpram_dev {
    int memsize;
    int dpram_vbase;
    struct cdev cdev;
};

static struct dpram_dev *dpram;

#define CRC_TAB_SIZE	256             /* 2^CRC_TAB_BITS      */
#define CRC_16_L_SEED	0xFFFF

const u16 CRC_Table[CRC_TAB_SIZE] = {
    0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
    0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
    0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
    0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
    0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
    0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
    0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
    0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
    0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
    0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
    0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
    0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
    0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
    0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
    0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
    0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
    0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
    0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
    0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
    0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
    0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
    0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
    0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
    0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
    0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
    0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
    0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
    0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
    0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
    0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
    0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
    0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

static int dpram_recovery_ioremap(struct dpram_dev *dev);
static int dpram_recovery_modem_pwr_status(void);
static int dpram_recovery_modem_pwroff(void);
static int dpram_recovery_modem_reset(void);
static int dpram_recovery_modem_pwron(void);
static u32 dpram_recovery_ReadMagicCode(void);
static void dpram_recovery_WriteMagicCode(u32 dwCode);
static int dpram_recovery_dpram_Init(void);
static u16 dpram_recovery_ReadCommand(void);
static void dpram_recovery_WriteCommand(u16 nCmd);
static u32 dpram_recovery_check_command(u16 intr, u16 mask);
static int dpram_recovery_wait_response(u16 cmd_mask);
static int dpram_recovery_WaitReadyPhase(void);
static int dpram_recovery_write_modem_firmware(struct dpram_dev *dev, char __user *firmware, int size);
static u32 dpram_recovery_WriteImage(u8* const pBuf, u32 dwWriteLen);
static u16 dpram_recovery_CalcTotFrame(u32 nDividend, u16 nDivisor);
static void dpram_recovery_WriteDoneRequest(void);
static int dpram_recovery_SendDonePhase(void);
static int dpram_recovery_UpdateRequestPhase(void);
static u32 dpram_recovery_DownloadControl(u8 *pBufIn, u32 Len );
static u32 dpram_recovery_StatusNotiPhase(u32 *pPercent, u8* UpDoneNotiFlag);
static int dpram_recovery_check_status(struct dpram_dev *dev, int __user *pct);
static int dpram_recovery_modem_hw_init(void);


typedef struct Up_Noti {
   u16 Bop;
   u16 progress_status;
   u16 error_code;    
   u16 Eop;
} Status_UpNoti, *pStatus_UpNoti;    

static int 
__inline __writel_once(struct dpram_dev *dev, int addr, int data)
{
   *(int*)(dev->dpram_vbase+addr) = data;

   return 0;
}

static void 
__write_from_user(struct dpram_dev *dev, int addr, 
    char __user *data, size_t size)
{
   if(copy_from_user((void*)(dev->dpram_vbase+addr),data,size)<0) {
      printk(KERN_ERR "[%s:%d] Copy from user failed\n", __func__, __LINE__);
   }

   {
      int i;
      printk(KERN_ERR "\n %s dpram=0x%08x, Written Data=%d,  ", __func__, (int)(dev->dpram_vbase+addr), size);
   }
}

static void 
__read_to_user(struct dpram_dev *dev, int addr, char __user *data, size_t size)
{
   if(data == NULL) return;

   if(copy_to_user(data, (void*)(dev->dpram_vbase+addr), size) < 0) {
      printk(KERN_ERR "[%s] Copy to user failed\n", __func__);
   }

   {
      int i;
      printk(KERN_ERR "\n %s dpram=0x%08x, Read Data=%d,  ", __func__, (int)(dev->dpram_vbase+addr), size);
      for (i = 0; i < size; i++)	
      printk(KERN_ERR "%02x ", *((unsigned char *)data + i));
      printk(KERN_ERR "\n");
   }
}

static int 
dpram_recovery_ioremap(struct dpram_dev *dev)
{
   int i;

   printk(MSGDBG "%s" MSGEND, __func__);

   dev->dpram_vbase = (int)ioremap_nocache(DPRAM_BASE_ADDR, DPRAM_SIZE);
   
   if (dev->dpram_vbase == NULL) 
   {
      printk("failed ioremap\n");
      return -ENOENT;
   }

   printk(KERN_DEBUG "dpram vbase=0x%8x\n", dev->dpram_vbase);

   dev->memsize = DPRAM_SIZE;

   printk(MSGDBG "            dpram_vbase = 0x%08x" MSGEND, dev->dpram_vbase);
   printk(MSGDBG "                memsize = 0x%08x" MSGEND, dev->memsize);    

   for(i=0; i<DPRAM_SIZE; i=i+4) 
   {
      if( __writel_once(dev, i, 0xffffffff) )
      {
         printk(KERN_DEBUG "Exit during dpram initialization.\n");
         return 0;
      }
   }
   
   return 0;
}

static int
dpram_recovery_modem_pwr_status(void)
{
    printk(KERN_ERR "%s Modem Active: %d\n", __func__, gpio_get_value(GPIO_QSC_ACTIVE));
    
	return gpio_get_value(GPIO_QSC_ACTIVE);
}

static int 
dpram_recovery_modem_pwroff(void)
{
   printk(KERN_ERR "%s\n", __func__);

	/* phone power off */
   gpio_direction_output(GPIO_QSC_PHONE_ON, GPIO_LEVEL_LOW);
	gpio_direction_output(GPIO_QSC_RESET_N, GPIO_LEVEL_LOW);


   return 0;
}

static int 
dpram_recovery_modem_reset(void)
{
   printk(KERN_ERR "%s\n", __func__);

	/* Phone reset */
   gpio_direction_output(GPIO_QSC_PHONE_ON, GPIO_LEVEL_HIGH);
   msleep(50);
   
   gpio_direction_output(GPIO_QSC_RESET_N, GPIO_LEVEL_LOW);
   msleep(20);
   gpio_direction_output(GPIO_QSC_RESET_N, GPIO_LEVEL_HIGH);
   msleep(500);
   
   gpio_direction_output(GPIO_QSC_PHONE_ON, GPIO_LEVEL_LOW); 

   return 0;
}

static int 
dpram_recovery_modem_pwron(void)
{
   printk(KERN_ERR "%s\n", __func__);

   gpio_direction_output(GPIO_QSC_PHONE_ON, GPIO_LEVEL_LOW);
   gpio_direction_output(GPIO_QSC_RESET_N, GPIO_LEVEL_LOW);
   mdelay(500);

   gpio_direction_output(GPIO_QSC_RESET_N, GPIO_LEVEL_HIGH);
   gpio_direction_output(GPIO_QSC_PHONE_ON, GPIO_LEVEL_HIGH);

   msleep(100);

   printk(KERN_DEBUG "QSC_PHONE_ON: %s, QSC_RESET_N: %s, PHONE_ACTIVE: %s\n",
          gpio_get_value(GPIO_QSC_PHONE_ON) ? "HIGH" : "LOW",
          gpio_get_value(GPIO_QSC_RESET_N) ? "HIGH" : "LOW",
          gpio_get_value(GPIO_QSC_ACTIVE) ? "HIGH" : "LOW ");

   msleep(200);

   gpio_direction_output(GPIO_QSC_PHONE_ON, GPIO_LEVEL_LOW);

   printk(KERN_DEBUG "QSC_PHONE_ON: %s, QSC_RESET_N: %s, PHONE_ACTIVE: %s\n",
          gpio_get_value(GPIO_QSC_PHONE_ON) ? "HIGH" : "LOW",
          gpio_get_value(GPIO_QSC_RESET_N) ? "HIGH" : "LOW",
          gpio_get_value(GPIO_QSC_ACTIVE) ? "HIGH" : "LOW ");

   return 0;
}

static u32 
dpram_recovery_ReadMagicCode(void)
{
   u32 dwCode;
   u32* dwMagicCodeAddress;

   dwMagicCodeAddress = (u32*)(dpram->dpram_vbase);
   memcpy((void*)&dwCode, (void*)(dwMagicCodeAddress), 4);

   return dwCode;
}

static void 
dpram_recovery_WriteMagicCode(u32 dwCode)
{
   u32* dwMagicCodeAddress;
   dwMagicCodeAddress = (u32*)(dpram->dpram_vbase);

   SeqIdx = 1;
   memcpy((void*)dwMagicCodeAddress, (void*)&dwCode, 4);
}

static int 
dpram_recovery_dpram_Init(void)
{
   const u32 dwMagicCodeToWrite = MAGIC_FODN;
   u32 dwMagicCode = 0;  
   u16 resp;
   int iRetry = 0, nCnt = 0, i = 0, err;

   printk(KERN_DEBUG "+DPRAM_Init\n");

   dpram_recovery_modem_hw_init();

   /* Write the magic code */
   for (iRetry = 1; iRetry > 0 ; iRetry--) 
   {
      dwMagicCode = dpram_recovery_ReadMagicCode();
      
      if( dwMagicCode == dwMagicCodeToWrite )
      {
         printk(KERN_DEBUG "dwMagicCode == dwMagicCodeToWrite!!\n");
         break;
      }        
      dpram_recovery_WriteMagicCode(dwMagicCodeToWrite);  

      dwMagicCode = dpram_recovery_ReadMagicCode();
      printk(KERN_DEBUG "magic code: %x\n", dwMagicCode);
   }  

   dpram_recovery_modem_pwron(); 
   
   /* Check phone on status */
   while(!dpram_recovery_modem_pwr_status()) 
   {
      msleep(1000);
      nCnt++;
      if(nCnt >= 20)
      {
         printk(KERN_DEBUG "no phone active!! \n");
         return FALSE;
      }		
   }
   
   printk(KERN_DEBUG "Phone Active!!!\n");

   printk(KERN_DEBUG "-DPRAM_Init\n");

   return TRUE;
}    

static u16 
dpram_recovery_ReadCommand(void)
{
	u16 nCmd = 0;
	
	memcpy((void*)&nCmd, (void*)(dpram->dpram_vbase + DPRAM_PHONE2PDA_INTERRUPT_ADDRESS), 2);

	return nCmd;
}

static void 
dpram_recovery_clear_modem_command(void)
{
	u16 clear;	
	clear = 0x0000;

   printk(KERN_DEBUG "dpram_recovery_clear_modem_command\n");
	
	memcpy((void*)(dpram->dpram_vbase + DPRAM_PHONE2PDA_INTERRUPT_ADDRESS), (void*)&clear, 2);	
}

static void 
dpram_recovery_WriteCommand(u16 nCmd)
{
	printk(KERN_DEBUG "[DPRAM_WriteCommand] Start : 0x%04x\n", nCmd);

	dpram_recovery_clear_modem_command();

	memcpy((void*)(dpram->dpram_vbase + DPRAM_PDA2PHONE_INTERRUPT_ADDRESS), (void*)&nCmd, 2);
}


static u32 
dpram_recovery_check_command(u16 intr, u16 mask)
{
   u32 ret=0;

   if(intr == 0)// There's no Interrupt from Phone. we have to poll again.
   {
      return CMD_RETRY;
   }
   else   // Some command arrived.
   { 
      if ((intr & MASK_CMD_VALID) != MASK_CMD_VALID)
      {
         return CMD_RETRY;
      }
      else if((intr & MASK_PDA_CMD) == MASK_PDA_CMD)
      {
         return CMD_RETRY;
      }
      else if((intr & MASK_PHONE_CMD) == MASK_PHONE_CMD)
      {
         if( (intr & mask) == mask )
         {
            return CMD_TRUE;
         }
         else
         {
            return CMD_FALSE;
         }
      }
      else
      {  		
         return CMD_RETRY;
      }
   }
}

static int 
dpram_recovery_wait_response(u16 cmd_mask)
{
   u16 intr;
   int nRetry = 0;
   int ret=0;

   while( 1 )
   {
      if( nRetry > 100 ) 
         return FALSE;

      intr = dpram_recovery_ReadCommand();
      printk(KERN_DEBUG "intr = 0x%x\n", intr);

      ret = dpram_recovery_check_command(intr, cmd_mask);
      if(ret == CMD_TRUE)
      {
         printk(KERN_DEBUG "READY ok\n");
         return TRUE;
      }
      else if(ret == CMD_FALSE)
      {
         printk(KERN_DEBUG "READY failed.\n");
         return FALSE;
      }

      nRetry++;     
      msleep(1000);
   }

   return TRUE;
}

static int 
dpram_recovery_WaitReadyPhase(void)
{
	int retval = TRUE;

	/* Send Delta Image Receive Ready Request */
	dpram_recovery_WriteCommand(CMD_FOTA_IMG_RECEIVE_READY_REQ);
	/* Wait Delta Image Receive Ready Response */
	retval = dpram_recovery_wait_response(MASK_CMD_FOTA_IMG_RECEIVE_READY_RESP);

	return retval;
}

static int
dpram_recovery_write_modem_firmware(
    struct dpram_dev *dev, char __user *firmware, int size)
{
   int ret = FALSE;

   /* Start Wait Ready Phase */
   if (dpram_recovery_WaitReadyPhase() == FALSE)
   {
      ret = -2;    
   	goto Exit;
   }

   /* Downlaod start Req ~ Update Status Noti */
   ret = dpram_recovery_DownloadControl((u8 *)firmware, size);
   if (ret < 0)
   {
      printk(KERN_DEBUG "[DPRAM_Download]Failed in DownloadControl\n.");     
      goto Exit;
   }
   printk(KERN_DEBUG "[DPRAM_Download]FileSize : %d\n", size);
   printk(KERN_DEBUG "[DPRAM_Download]Bootloader Image Download Completed!\n");

   if (!dpram_recovery_UpdateRequestPhase())
   {
      printk(KERN_DEBUG "[DPRAM_DownloadControl] Wait FOTA Update Start Response Failed!!\n");
      ret = -7; // -7 means that FOTA Update Start Response failed.
      goto Exit;
   }

   ret = TRUE;

Exit:    
	return ret;
}

static u16 
dpram_recovery_MakeCRC(u16 length, u16* data_ptr_WORD)
{
   u16  data_crc = CRC_16_L_SEED;
   u16  const * table = CRC_Table;
   u8 * data_ptr = (u8 *) data_ptr_WORD;
   int i;

   for (i=0; i<length; i++) 
   { 
      data_crc = (((data_crc) >> 8) ^ table[((data_crc) ^ (u16)(data_ptr[i])) & 0x00ff]);
      //printk(KERN_DEBUG "%010d:0x%04x\r\n", i, data_crc ); 
   }
   //printk(KERN_DEBUG "[MakeCRC] length:%d pt:0x%08x i:%d v:0x%04x \r\n", length,  data_ptr, i, data_ptr[i] ); 
   return data_crc;
}

static u32 
dpram_recovery_WriteImage(u8* const pBuf, u32 dwWriteLen)
{
   u8*  pDest;
   u8* pDest_Data;
   u16 Len;
   u16 nCrc;

   //printk(KERN_DEBUG "Start %d 0x%04x(%d)\n", dwWriteLen, g_TotFrame, g_TotFrame);

   pDest = (u8*)(dpram->dpram_vbase + DPRAM_PDA2PHONE_FORMATTED_START_ADDRESS);
   Len   = (u16)min(dwWriteLen, FODN_DEFAULT_WRITE_LEN);

   //printk(KERN_DEBUG "Start : pDest(0x%08x),  dwWriteLen(%d)\n",pDest, Len);

   // Start Index
   *pDest++ = LOBYTE(START_INDEX);
   *pDest++ = HIBYTE(START_INDEX);

   //Total Frame number:
   *pDest++ = LOBYTE(g_TotFrame);
   *pDest++ = HIBYTE(g_TotFrame);

   //Current Frame number;
   *pDest++ = LOBYTE(g_CurFrame);
   *pDest++ = HIBYTE(g_CurFrame);
   g_CurFrame++;

   // Length - Does it include the length of START_INDEX??
   *pDest++ = LOBYTE(Len);
   *pDest++ = HIBYTE(Len);

   // Data
   pDest_Data = pDest;
   memcpy((void*)pDest, (void*)pBuf, Len);
   pDest += Len;

   // Fill null if data length is odd
   if (Len%2 != 0)
   {
      *pDest++ = 0xff;
      printk(KERN_DEBUG "odd  0x%08x  0x%02x\n", pDest-1, (u8)(*pDest-1));
   }

   //printk(KERN_DEBUG "len:%d default len:%d\n", Len, FODN_DEFAULT_WRITE_LEN);
   //printk(KERN_DEBUG "start data 0x%08x \n", pDest);

   if(Len < FODN_DEFAULT_WRITE_LEN )
   {
      memset((void*)pDest, 0x0 /*0xff*/, FODN_DEFAULT_WRITE_LEN - Len );
      pDest += (FODN_DEFAULT_WRITE_LEN - Len) ;
   }
   printk(KERN_DEBUG "CRC start 0x%08x\n", pDest);

   nCrc = dpram_recovery_MakeCRC(Len,(u16 *)pDest_Data);

   *pDest++ = LOBYTE(nCrc);
   *pDest++ = HIBYTE(nCrc);

   printk(KERN_DEBUG "CRC value 0x%04x \n", nCrc);

   *pDest++ = LOBYTE(END_INDEX);
   *pDest++ = HIBYTE(END_INDEX);

   // Write Command
   dpram_recovery_WriteCommand(CMD_FOTA_IMG_SEND_REQ);

   return Len;
}

static u16 
dpram_recovery_CalcTotFrame(u32 nDividend, u16 nDivisor)
{
   u16 nCompVal1 = 0;
   u16 nCompVal2 = 0;

   printk(KERN_DEBUG "[CalcTotFrame](%d)  %d %d\n", __LINE__, nDividend, nDivisor);
   nCompVal1 = (u16) (nDividend / nDivisor);
   nCompVal2 = (u16) (nDividend  - (nCompVal1 * nDivisor));
   if( nCompVal2 > 0 ) 
   {
      printk(KERN_DEBUG "[CalcTotFrame](%d) val2 : %d\n", __LINE__, nCompVal2);
      nCompVal1++;
   }
   printk(KERN_DEBUG "[CalcTotFrame](%d) result %d\n", __LINE__, nCompVal1);
   return nCompVal1;
}

static void 
dpram_recovery_WriteDoneRequest(void)
{
   printk(KERN_DEBUG "[DPRAM_WriteDoneRequest] Start\n");

   SeqIdx = 1;

   dpram_recovery_WriteCommand(CMD_FOTA_SEND_DONE_REQ);
   printk(KERN_DEBUG "[DPRAM_WriteDoneRequest] End\n");
}

static int 
dpram_recovery_SendDonePhase(void)
{
   int retval = TRUE;

   /* Send Write Done Request */
   dpram_recovery_WriteDoneRequest();
   
   /* Wait Write Done Response */	
   retval = dpram_recovery_wait_response(MASK_CMD_FOTA_SEND_DONE_RESP);
   if(retval == FALSE)
   {
      printk(KERN_DEBUG " Wait Write Done Response Failed!!\n");
      return retval;
   }

   printk(KERN_DEBUG "Wait 0.5 secs.. .. ..\n");
   msleep(500);

   return retval;
}

static int 
dpram_recovery_UpdateRequestPhase(void)
{
   int retval = TRUE;

   /* Send FOTA Update Start Request */
   dpram_recovery_WriteCommand(CMD_FOTA_UPDATE_START_REQ);
   /* Wait FOTA Update Start Response */
   //retval = dpram_recovery_wait_response(MASK_CMD_FOTA_UPDATE_START_RESP);

   return retval;
}

static u32 
dpram_recovery_DownloadControl(u8 *pBufIn, u32 Len )
{
   u32 dwWriteLen = 0, dwWrittenLen = 0, dwTotWrittenLen = 0;
   u32 dwRet = 0;
   u16 nTotalFrame = 0, nIntrValue = 0;
   int nwRetry = 0, nrRetry = 0, retval;

   nTotalFrame = dpram_recovery_CalcTotFrame(Len, DELTA_PACKET_SIZE);	
   g_TotFrame =  nTotalFrame;
   printk(KERN_DEBUG "[DPRAM_DownloadControl] total frame:%d,%d\n", g_TotFrame, nTotalFrame);

   while(dwTotWrittenLen < Len)
   {
      /*Write proper size of image to DPRAM*/
      printk(KERN_DEBUG "[DPRAM_DownloadControl]DPRAM_WriteImage %d/%d start\n", 
               g_CurFrame, g_TotFrame);
      dwWriteLen = min(Len - dwTotWrittenLen, DELTA_PACKET_SIZE);
      dwWrittenLen = dpram_recovery_WriteImage(pBufIn, dwWriteLen);
      printk(KERN_DEBUG "[DPRAM_DownloadControl]Written data:%d\n", dwWrittenLen);

      if(dwWrittenLen > 0)
      {
         dwTotWrittenLen += dwWrittenLen;
         pBufIn += dwWrittenLen;
      }
      else
      {
         printk(KERN_DEBUG "[DPRAM_DownloadControl]Write Image Len is wierd.\n");
         if(nwRetry < 3)
         {
            nwRetry++;
            printk(KERN_DEBUG "[DPRAM_DownloadControl]Retry to write. nRetry = %8x\n", nwRetry);
            continue;
         }
         else
         {
            printk(KERN_DEBUG "[DPRAM_DownloadControl]Fail to Write Image to DPRAM.\n"); 
            dwRet = -3;
            goto Exit;
         }
      }

      /* Wait for the IMAGE WRITE RESPONSE */
      while(nrRetry < 10)
      {
         retval = dpram_recovery_wait_response(MASK_CMD_FOTA_IMG_SEND_RESP);
         if(retval == FALSE)
         {
            printk(KERN_DEBUG "[DPRAM_DownloadControl] Wait Image Send Response Failed\n");
            dwRet = -4;
            goto Exit;
         }
         else
         {
            nIntrValue = dpram_recovery_ReadCommand();
            if((nIntrValue & MASK_CMD_RESULT_FAIL) == 0)
            {
               printk(KERN_DEBUG "[DPRAM_DownloadControl] MASK_CMD_IMAGE_SEND_RESPONSE OK issued.\n");
               printk(KERN_DEBUG "[DPRAM_DownloadControl] %d /%d ok\n", g_CurFrame -1, g_TotFrame);
               break;
            }
            else
            {
               printk(KERN_DEBUG "[DPRAM_DownloadControl] MASK_CMD_IMAGE_SEND_RESPONSE Failed issued.\n");
               msleep(100);
               nrRetry++;
               dpram_recovery_WriteCommand(CMD_FOTA_IMG_SEND_REQ);                       
               printk(KERN_DEBUG "[DPRAM_DownloadControl] CMD_IMG_SEND_REQ retry.(%d)\n", nrRetry); 
               
               if( nrRetry >= 10 )
               {
                  dwRet = -5;
                  goto Exit;
               }
            }
         }
      }
   }
   
   g_CurFrame = 1;

   if(!dpram_recovery_SendDonePhase())
   {
      printk(KERN_DEBUG "[DPRAM_DownloadControl] There's something unexpeted in SendDone Phase.\n");
      dwRet = -6; // -6 means that SendDone Phase failed.
      goto Exit;
   }

Exit:
    return dwRet;
}

static u32 
dpram_recovery_StatusNotiPhase(u32 *pPercent, u8* UpDoneNotiFlag)
{
   Status_UpNoti UpStatus;
   int nRetry =0;
   u16 intr = 0, ret = 0;
   u32 dwRet = TRUE;

   u8 progress_status = 0x0;
   u32 curr_offset = 0x0;
   u32 total_number = 0x0;

   UpStatus.progress_status = 0x00;
   UpStatus.error_code = 0x00;

   *UpDoneNotiFlag = FALSE;

   //dpram_recovery_WriteCommand(CMD_FOTA_UPDATE_STATUS_IND);

   /* Wait FOTA Update Status Indication */	
   dwRet = dpram_recovery_wait_response(MASK_CMD_FOTA_UPDATE_STATUS_IND);
   if (dwRet == FALSE)
   {
      printk(KERN_DEBUG "Wait FOTA Update Status Indication Failed!!\n");
      intr = dpram_recovery_ReadCommand();
      ret = dpram_recovery_check_command(intr, MASK_CMD_FOTA_UPDATE_END_IND);      
      if (ret == CMD_TRUE)
      {
         dwRet = TRUE;
         *UpDoneNotiFlag = TRUE;
         printk(KERN_DEBUG "But FOTA Update Done Indication Received!!\n");
         goto StatusRead;
      }
      else
         goto Exit;
   }

 StatusRead:
    memcpy((void*)&UpStatus, (void*)(dpram->dpram_vbase + DPRAM_PDA2PHONE_FORMATTED_START_ADDRESS), 8 );

    printk(KERN_DEBUG "0x%x  0x%x  0x%x  0x%x \n", UpStatus.Bop, UpStatus.progress_status, UpStatus.error_code, UpStatus.Eop);   
 
    if((UpStatus.Bop != START_INDEX) || (UpStatus.Eop != END_INDEX))
    {
        printk(KERN_DEBUG "Packet format is wrong.\n");
        dwRet = 0;
    }
    else
    {
        printk(KERN_DEBUG "Get the Status info.\n");
        *pPercent = (u32)UpStatus.progress_status;
        error_code = (u16)UpStatus.error_code;
    }

   if (error_code != 0)
   {
      printk(KERN_DEBUG "Update error is occured, error number:0x%x.\n", error_code);
      dwRet = FALSE;
      goto Exit;
   }

Exit:
   /* Clear modem command for next IOCTL_CHK_STAT */
   dpram_recovery_clear_modem_command();

   return dwRet;    
}

static int
dpram_recovery_check_status(struct dpram_dev *dev, int __user *pct)
{
   u32 bUpStatusPer = 0, PrevbUpStatusPer = 0, curr_percent = 0, prev_percent = 0; 
   u8 UpDoneNotiFlag = 0;
   u32 dwRet = 1;
   int retval = 0;

   /* Process for UPDATE STATUS Noti. phase. */    
   retval = dpram_recovery_StatusNotiPhase(&bUpStatusPer, &UpDoneNotiFlag);
   if(retval == FALSE)
   {
      printk(KERN_DEBUG "[DPRAM_DownloadControl] Wait Status Noti Phase Failed!!\n");
      dwRet = -8;
   }
   else
   {
      printk(KERN_DEBUG "Progress:%d\n", bUpStatusPer);

      if(bUpStatusPer > PrevbUpStatusPer)
         PrevbUpStatusPer = bUpStatusPer; 

      *pct = bUpStatusPer;

      if((bUpStatusPer == 100) && (UpDoneNotiFlag == TRUE))
      {
         printk(KERN_DEBUG "Done!!!\n");     
         dwRet = 0;
      }        
   }
       
   return dwRet;
}

static int
dpram_recovery_modem_hw_init(void)
{
   printk(KERN_ERR "%s\n", __func__);

	s3c_gpio_cfgpin(GPIO_QSC_PHONE_ON, S3C_GPIO_OUTPUT);	
	s3c_gpio_cfgpin(GPIO_QSC_RESET_N, S3C_GPIO_OUTPUT);
	s3c_gpio_cfgpin(GPIO_QSC_ACTIVE, S3C_GPIO_INPUT);
	s3c_gpio_cfgpin(GPIO_QSC_INT, S3C_GPIO_INPUT);

   return 0;
}

static ssize_t 
dpram_recovery_read(struct file *filp, char __user *buff, 
    size_t count, loff_t *fpos)
{
   return 0;
}

static ssize_t 
dpram_recovery_write(struct file *filp, const char __user *buff,
    size_t size, loff_t *fpos)
{
   return 0;
}

static int
dpram_recovery_ioctl(struct inode *inode, struct file *filp,
	   unsigned int cmd, unsigned long arg)
    
{
   struct dpram_dev *dev;
   int ret = 0;
	u32 magic_code = 0x0; 

   printk(MSGDBG "%s" MSGEND, __func__);
   printk(KERN_ERR "%s\n", __func__);

   dev = container_of(inode->i_cdev, struct dpram_dev, cdev);

   switch (cmd) 
   {
   case IOCTL_WRITE_MAGIC_CODE:
      dpram_recovery_dpram_Init();
      break;
   
   case IOCTL_ST_FW_UPDATE:
      printk(KERN_ERR "%s IOCTL_ST_FW_UPDATE\n", __func__);

      if(arg == NULL) 
      {
         printk(MSGWARN "Firmware should be written prior to this call" MSGEND);
      }
      else 
      {
         struct dpram_firmware fw;

         if(copy_from_user((void *)&fw, (void *)arg, sizeof(fw)) < 0) 
         {
            printk("[IOCTL_ST_FW_UPDATE]copy from user failed!");
            ret = -1;
            //return ret;
         }

         ret = dpram_recovery_write_modem_firmware(dev, fw.firmware, fw.size);

         if (ret < 0) 
         {
            printk("firmware write failed\n");
         }
      }
      break;

   case IOCTL_CHK_STAT:
      {
         struct stat_info *pst;

         printk(KERN_ERR "%s IOCTL_CHK_STAT\n", __func__);        

         pst = (struct stat_info*)arg;
         ret = dpram_recovery_check_status(dev, &(pst->pct));
      }
      break;

   case IOCTL_MOD_PWROFF:
      printk(KERN_ERR "%s IOCTL_MOD_PWROFF\n", __func__);        

      msleep(2000); /* Delay for display */

      /* Clear Magic Code */
      dpram_recovery_WriteMagicCode(magic_code);

      dpram_recovery_modem_pwroff();
      break;

   default:
      printk(KERN_ERR "Unknown command");
      break;
   }

   return ret;
}

static int 
dpram_recovery_open(struct inode *inode, struct file *filp)
{
   struct dpram_dev *dev;

   printk(KERN_ERR "%s\n", __func__);

   dev = container_of(inode->i_cdev, struct dpram_dev, cdev);
   filp->private_data = (void*)dev;

   return 0;
}

static int 
dpram_recovery_release(struct inode *inode, struct file *filp)
{
   printk(KERN_DEBUG "dpram recovery device released.\n");
   return 0;
}

static struct file_operations dpram_recovery_fops = 
{
   .owner = THIS_MODULE,
   .read = dpram_recovery_read,
   .write = dpram_recovery_write,
   .ioctl = dpram_recovery_ioctl,
   .open = dpram_recovery_open,
   .release = dpram_recovery_release,
};

static int 
__init dpram_recovery_init(void)
{
   int err = 0;
   int devno = MKDEV(MAJOR_NUM, 0);

   printk(KERN_ERR "[DPRAM_RECOVERY] %s() ...\n", __func__);

   dpram = kmalloc(sizeof(struct dpram_dev), GFP_KERNEL);

   if(dpram == NULL) 
   {
      printk(KERN_ERR "dpram recovery device allocation failed!!\n");
      err = -ENOMEM;
      goto out;
   }

   if(dpram_recovery_ioremap(dpram)<0)
   {
      printk(KERN_ERR "Dpram recovery ioremap failed!\n");
      goto out_err1;
   }

   if(register_chrdev_region(devno, 1, DRIVER_NAME) < 0) 
   {
      printk(KERN_DEBUG "chrdev region register failed\n");
      err = -1;
      goto out_err1;
   }

   cdev_init(&dpram->cdev, &dpram_recovery_fops);
   dpram->cdev.owner = THIS_MODULE;
   dpram->cdev.ops = &dpram_recovery_fops;

   err = cdev_add(&dpram->cdev, devno, 1);

   if(err < 0) 
   {
      printk(KERN_ERR "Dpram recovery device failed to register!\n");
      goto out_err1;
   }

   printk(KERN_ERR "%s %d\n", __func__, __LINE__);

out:
   return err;

out_err2:
   cdev_del(&dpram->cdev);

out_err1:
   kfree(dpram);
   return err;

}

static void __exit 
dpram_recovery_exit(void)
{
    cdev_del(&dpram->cdev);
    kfree(dpram);
	printk (KERN_DEBUG "Dpram recovery char device is unregistered\n");
}

module_init(dpram_recovery_init);
module_exit(dpram_recovery_exit);

MODULE_AUTHOR("Samsung Electronics Co., LTD");
MODULE_DESCRIPTION("Onedram Device Driver for recovery.");
MODULE_LICENSE("GPL");
