/* 
 * Copyright (C) 2008, 2009,  Samsung Electronics Co. Ltd. All Rights Reserved.
 *       Written by Linux Lab, MITs Development Team, Mobile Communication Division.
 */

/*****************************************************************************/
/*                                                                           */
/* NAME    : Samsung Secondary BootLoader									 */
/* FILE    : dpram_mmc.c													 */
/* PURPOSE : This file implements the Modem code update over DPRAM                        */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*   Permission is hereby granted to licensees of Samsung Electronics        */
/*   Co., Ltd. products to use or abstract this computer program for the     */
/*   sole purpose of implementing a product based on Samsung                 */
/*   Electronics Co., Ltd. products. No other rights to reproduce, use,      */
/*   or disseminate this computer program, whether in part or in whole,      */
/*   are granted.                                                            */
/*                                                                           */
/*   Samsung Electronics Co., Ltd. makes no representation or warranties     */
/*   with respect to the performance of this computer program, and           */
/*   specifically disclaims any responsibility for any damages,              */
/*   special or consequential, connected with the use of this program.       */
/*                                                                           */
/*****************************************************************************/
//------------------------------------------------------------------------------
//
//  File:  DPRAM.c
//	- ported to Stealth by v.putta on 05-10-2010
//------------------------------------------------------------------------------

#include <asm/arch.h>
#include <asm/command.h>
#include <asm/main.h>
#include <asm/fat.h>
#include <asm/error.h>

#include <asm/arch/dpram_crc.h>
#include <asm/arch/dpram_mmc.h>


//#include <asm/download.h>
#define MID	"[dpram] "

#undef d1
#define d0(arg,...)	printf(MID arg, ## __VA_ARGS__)
#define d1(arg,...)	printf(MID "%s(%d): "arg,__FUNCTION__,__LINE__, ## __VA_ARGS__)
//#define d2(arg,...)	printf(MID "%s(%d): "arg,__FUNCTION__,__LINE__, ## __VA_ARGS__)
#define d2(arg,...)	



#ifdef DPRAM_PHONE2PDA_INTERRUPT_ADDRESS
#undef DPRAM_PHONE2PDA_INTERRUPT_ADDRESS
#define DPRAM_PHONE2PDA_INTERRUPT_ADDRESS               (DPRAM_START_ADDRESS + 0x3FFC)
#endif

#ifdef DPRAM_PDA2PHONE_INTERRUPT_ADDRESS
#undef DPRAM_PDA2PHONE_INTERRUPT_ADDRESS
#define DPRAM_PDA2PHONE_INTERRUPT_ADDRESS               (DPRAM_START_ADDRESS + 0x3FFE)
#endif

//not sure if these definitions have to be some where else
#define GPIO_INT_DPRAM_R	S5P_GPH1_0
#define GPIO_VIA_RST_N	S5P_GPH3_7
#define GPIO_VIA_ACTIVE	S5P_GPH1_7

#define GPIO_INT_DPRAM_R_AF	0		// Input
#define GPIO_VIA_RST_N_AF	1		// Output
#define GPIO_VIA_ACTIVE_AF	0		// Input

#define DPRAM_BASE_ADDR	0x98000000
// end of not sure to place definitions

#define TEXT(s) s

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#define MAKEWORD(a, b)      ((u16)(((u8)(a)) | ((u16)((u8)(b))) << 8))
#define MAKELONG(a, b)      ((LONG)(((u16)(a)) | ((u32)((u16)(b))) << 16))
#define LOWORD(l)           ((u16)(l))
#define HIWORD(l)           ((u16)(((u32)(l) >> 16) & 0xFFFF))
#define LOBYTE(w)           ((u8)(w))
#define HIBYTE(w)           ((u8)(((u16)(w) >> 8) & 0xFF))


#define PHONE_BOOT_UPDATE	(1<<0)
#define PHONE_EFS_CLEAR		(1<<1)

#define COMMAND_FAIL					0xFF000000
#define COMMAND_OK						0x01000000
#define COMMAND_START					0x02000000
#define COMMAND_END						0x03000000
#define COMMAND_DOWNLOAD_COMPLETE		0x12340000

#define COMMAND_DATA(d)					((d & 0xff) << 16)

#define CMD_FAIL_PHONE_ACTIVE			COMMAND_DATA(1)
#define CMD_FAIL_PHONE_DOWNLOAD			COMMAND_DATA(2)


//---------------------------------------------------------------------------
// Function prototype
//---------------------------------------------------------------------------
extern void msleep(u32 msVal);
//extern void draw_progress_bar(u32 current);
//extern void clear_progress_bar(void);

void MemCopy(u32 pDest, u32 pSrc, u32 dwSize);
u16 MakeCRC(u16 length, u16* data_ptr_WORD);
u16 MakeCRC_DUMP(u16 length, u16* data_ptr_WORD);

#if 1
inline static u32	dpram_read(void* dest, void* src, u32 size);
inline static u32	dpram_write(void* dest, void* src, u32 size);

inline static void	dpram_write_command(u16 nCmd);
inline static u16	dpram_read_command(void);



void    dpram_init(void);
void 	phone_shutdown(void);
u32		dpram_enter_download_mode(void);

static u32	dpram_write_magic_code(u32 code);
static u32	dpram_write_image(const u8* pBuf, u32 dwWriteLen);
static void	dpram_write_done_request(int bBootUpdate);

int		dpram_wait_for_ready_noti(void);
int		dpram_write_start_phase(Req_Packet *pStartReq);
u32		dpram_send_done_phase(int bBootUpdate);
u32		dpram_status_noti_phase(u8 *pPercent);
int		dpram_update_done_noti_phase(void);
u32 	dpram_download_control(u8 *pBufIn, u32 Len, s32 flag);

u16 calc_total_frame(u32 nDividend, u16 nDivisor);
#endif
//---------------------------------------------------------------------------

static u8*		DPRAM_BASE_PTR = NULL;

u16  g_TotFrame = 0;
u16  g_CurFrame = 1;


static s32	s=0,e=0;

static int count = 0;

static u32 wait_for_dpram_int(u32 wait_ms, u32 count)
{
	u32 i=0,c=1;
	u32 dpram_int=1;
	u32 time_out = count? (count * wait_ms )/1000 : 60*5; // 5 min
	//d0("time out : %d\n", time_out);

	//dpram_int = gpio_get_level(GPIO_nDPRAM_INT);
    dpram_int = s5p_gpio_getpin(GPIO_INT_DPRAM_R);
//	d0("Dpram INT : %d\n", dpram_int);
//	d0("Dpram INTReg : 0x%08x\n", *(volatile u32 *)0xE0200C24);

	//while( c != count )
	while( c != count && dpram_int == 1 ) 
	{
		//dpram_int = gpio_get_level(GPIO_nDPRAM_INT);
        dpram_int = s5p_gpio_getpin(GPIO_INT_DPRAM_R);

#if 0
		printf("%d, ", dpram_int); // print
		d0("Dpram INTReg : 0x%08x\n", *(volatile u32 *)0xE0200C24);
#endif
		i= (wait_ms * c)/1000;
		c++;

		//d0("count = %d, i = %d\n", c, i);

		if( i > time_out ) 
		{
			d0("DPRAM INT time out!\n");            
			return 0;           
		}
		msleep(wait_ms);
	}

//	printf("Got interrupt \n"); // print

	return 1;
}


#define CMD_RETRY	0
#define CMD_TRUE	1
#define CMD_FALSE	-1

// return value
// -1 : false
// 0 : continue
// 1 : true
static u32 check_command(u16 intr, u32 mask)
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


// New Lok3 code only cares about 0 for successs and -1 for fail. COMMAND_OK and all not used.;
#define VIA_DWNLD_FAIL	-1
#define VIA_DWNLD_SUCCESS	0

extern int
dpram_download(const u8 * pBufIn, u32 dwSize, s32 boot_update, s32 efs_clear)
{
    int bRetVal = VIA_DWNLD_SUCCESS;			
    int retry = 0;
	int i=0;
    s32 flag = boot_update | efs_clear;

    d0("start, Boot Update : %s\n", (flag & PHONE_BOOT_UPDATE)?"True":"False");
    d0("start, EFS Clear   : %s\n", (flag & PHONE_EFS_CLEAR)?"True":"False");

    if (dwSize%2 != 0) dwSize+=1;

//	START(s);

    dpram_init();

    // Activate Phone    
    do
    {
    	if(!dpram_enter_download_mode())
    	{
    	    d0("Fail to Activate Phone\n");
    	    retry ++;
    	    msleep(1000);

			if(retry == 3)
			{
				printf("Fail to enter download mode.\n");
				return VIA_DWNLD_FAIL;		// CMD_FAIL_PHONE_ACTIVE; // fail
			}
    	}
    	else
    	{
    	    d0("Succeed to Activate Phone\n");
    	    retry = 0;
    	}    			

    } while( retry && (retry < 3));


    if(!dpram_wait_for_ready_noti())
    {
        d0("Failed in WaitReadyPhase\n.");
		bRetVal = VIA_DWNLD_FAIL;		// CMD_FAIL_PHONE_ACTIVE;
        msleep(2000);
        goto Exit;
    }

    //Downlaod start Req ~ Update Status Noti.
    if( dpram_download_control((u8 *)pBufIn, dwSize, flag) )
    {
        d1("Failed in DownloadControl\n.");
		bRetVal = VIA_DWNLD_FAIL;		// CMD_FAIL_PHONE_DOWNLOAD;
        goto Exit;
    }

    d1("File size : %d\n", dwSize);
    d1("Phone image download completed!\n");


	/* DPRAM Clear.. */
	for (i = DPRAM_START_ADDRESS; i <= DPRAM_END_OF_ADDRESS; i ++)
		*(DPRAM_BASE_PTR + i) = 0;



// Get UpdateDonwNotification
#if 0
    if(!dpram_update_done_noti_phase())
    {
        printf("Fail to get Update Done Noti\n");
        goto Exit;
    }
    else
    {
        printf("Succeed to get Update Done Noti\n");
        bRetVal = TRUE;
    }
#else
    //bRetVal = TRUE;
    bRetVal = VIA_DWNLD_SUCCESS;		// COMMAND_OK;
#endif

Exit:	
	phone_shutdown();
    return bRetVal;
}

void phone_reset(void)
{
	s5p_gpio_setpin(GPIO_VIA_RST_N, GPIO_LEVEL_LOW);
	msleep(100);
	s5p_gpio_setpin(GPIO_VIA_RST_N, GPIO_LEVEL_HIGH);	
}

u32 dpram_enter_download_mode(void)
{
    int wait_time=0;
	int i=0;
    u16 dpramIrq;

    s5p_gpio_setpin(GPIO_VIA_RST_N, GPIO_LEVEL_LOW);

    //Just read the dpram interrupt just in case we missed any interrupts.
    // TEMP CODE!!
    while (1 != ((*(volatile u32 *)0xE0200C24) & 0x01)){
	d0("DPRAM_Int pin Reg = 0x%08x \n", (*(volatile u32 *)0xE0200C24));
	dpramIrq = dpram_read_command();
	d0("Rx MBX = 0x%04x \n", dpramIrq);
	msleep(100);
    }
    
	
    for( i = 0; i < 3 ; i++) 
    {
        if (dpram_write_magic_code(MAGIC_DMDL))
		{
			d0("magic code is ok!\n");
			break;
		}
    }  
    //START(s);
//    phone_reset();
     msleep(500);
    s5p_gpio_setpin(GPIO_VIA_RST_N, GPIO_LEVEL_HIGH);
    //END(s,e);


    d0("wait for phone active pin to low\n");

	while (!s5p_gpio_getpin(GPIO_VIA_ACTIVE))//gpio_get_level(GPIO_VIA_ACTIVE))
	{
        if(wait_time >= 100)
        {
            printf("no phone active!! \n");
            //break; 
            return FALSE;
        }		
        msleep(100);
        wait_time++;
		if(!(wait_time % 10)) printf(".");
    }

    d0("Phone activated.\n");

	if(!wait_for_dpram_int(1000,20))
	{
		return 0; // failed.
	}

    d0("Entered Dpram download mode.\n");
//	END(s,e);
    return 1;
}


int dpram_wait_for_ready_noti(void)
{
    u16 intr;
    int nRetry = 0;
	int ret=0;

    while( 1 )
    {
        if( nRetry > 200 ) { return FALSE; }

		wait_for_dpram_int(10,20);

    	//Sleep(500);
        intr = dpram_read_command();
		printf("intr = 0x%x\n", intr);

		ret = check_command(intr, MASK_CMD_RECEIVE_READY_NOTIFICATION);
		if(ret == CMD_TRUE)
		{
			d1("READY ok\n");
			return TRUE;
		}
		else if(ret == CMD_FALSE)
		{
			d1("READY failed.\n");
			return FALSE;
		}
		
		nRetry++;
    }

    return TRUE;
}


u32 dpram_download_control(u8 *pBufIn, u32 Len, s32 flag)
{
    u32 dwWriteLen, dwWrittenLen, dwTotWrittenLen;
    u32 dwRet = 0;
    Req_Packet StartReq;
    u8 bUpStatusPer;
    u32  dwUpStatus;

    int nwRetry = 0;
    int nrRetry = 0;
    u16 intr = 0;
    u16 nCmd = 0;
    u16 nTotalFrame = 0;
	int ret=0;

	u32 curr_percent;
	u32 prev_percent;

	u32 curr_region=0;
	u32 prev_region=0;

    StartReq.DownMode = 0x0001;
    StartReq.StartAddress = 0x00000000;
    StartReq.dwLength = 0x00000000;
    
    dwWriteLen = 0;
    dwWrittenLen = 0;
    dwTotWrittenLen = 0;
	//---------------------------------------------------------
	// Something for analyzing how the phone.bin consists of.
	// To implement.
	//---------------------------------------------------------


//	START(s);
    //Download Start Req. 
    d1("write start.\n");
   	if(dpram_write_start_phase(&StartReq) == FALSE) return FALSE;
    d1("done.\n");
//	END(s,e);

	if(dpram_write_EFS_clear_phase((flag & PHONE_EFS_CLEAR)) == FALSE) return FALSE;

//	clear_progress_bar();
    //---------------------------------------------------------
    //Write image to DPRAM.
    //---------------------------------------------------------
    //Total frame no.    
    nTotalFrame = calc_total_frame(Len, DPDN_DEFAULT_WRITE_LEN);
    g_TotFrame =  nTotalFrame;
    d1("total frame:%d,%d\n", g_TotFrame, nTotalFrame);
    
    while(dwTotWrittenLen < Len)
    {
        dwWriteLen = min(Len - dwTotWrittenLen, DPDN_DEFAULT_WRITE_LEN);

        //Write proper size of image to DPRAM
        dwWrittenLen = dpram_write_image(pBufIn, dwWriteLen);

        d2("Written data : %d\n", dwWrittenLen);
        if(dwWrittenLen > 0)
        {
            dwTotWrittenLen += dwWrittenLen;
            pBufIn += dwWrittenLen;
            
        }
        else
        {
            d1("Write Image Len is wierd.\n");
            if(nwRetry<3)
            {
                d1("Retry to write. nRetry = %8x\n",nwRetry);
                continue;
            }
            else
            {
               d1("Fail to Write Image to DPRAM.\n"); 
               dwRet = -1;
               goto Exit;
            }
        }

        //Wait for the DOWNLOAD IMAGE SEND RESPONSE
        nrRetry = 0;
//        while(nrRetry < 10)
        while( 1 )
        {
			wait_for_dpram_int(10,0);

            intr = dpram_read_command();
            d2("intr = 0x%x\n", intr);

			ret = check_command(intr, MASK_CMD_IMAGE_SEND_RESPONSE);
			if(ret == CMD_TRUE )
			{
				if((intr & MASK_CMD_RESULT_SUCCESS) == MASK_CMD_RESULT_SUCCESS)
				{
					d2("==================> %d /%d ok\n", g_CurFrame -1, g_TotFrame);
					break;
				}
				else
				{
					d1("IMAGE_SEND_RESPONSE Failed.\n");
					msleep(100);
					nrRetry++;
					dpram_write_command(CMD_IMG_SEND_REQ);                       
					d1("CMD_IMG_SEND_REQ retry.(%d)\n", nrRetry); 

#if 0
					//if( nrRetry >= 10 )
					//{
					//    dwRet = -1;
					//    goto Exit;
					//}
#endif
				}
			}
			else if(ret == CMD_FALSE)
			{
				d1("CMD is not IMAGE_SEND_RESPONSE, %x\n", intr);
				dwRet = -1;
				goto Exit;
			}
			else	
			{
				nrRetry++;
				continue;
			}
        }
    	//Draw the progress bar.
		curr_percent = ((g_CurFrame-1)*100)/g_TotFrame;
		if(curr_percent != prev_percent)
		{
//			draw_progress_bar(curr_percent);
			printf(".");
			prev_percent = curr_percent;
		}
    }

	printf("\n");
//	END(s,e);

    g_CurFrame = 1;

//	clear_progress_bar();
    
    //Process for DOWNLOAD SEND DONE Phase.    
    if(!dpram_send_done_phase( (flag & PHONE_BOOT_UPDATE)))
    {
        d1("There's something unexpeted in SendDone Phase.\n");
        dwRet = -2; // -2 means that SendDone Phase failed.
        goto Exit;
    }

    //Process for UPDATE STATUS Noti. phase.    
    while( 1 )
    {
        dwUpStatus = dpram_status_noti_phase(&bUpStatusPer);
        if(dwUpStatus < 0x1 || dwUpStatus > 0x8 )
        {
            dwRet = -3;
            break;    
        }
        else
        {
            d2("region: %d, progress:%d\n", dwUpStatus, bUpStatusPer );

			curr_region = dwUpStatus;
			if(curr_region != prev_region)
			{
				printf("region : %d\n", dwUpStatus);
				prev_region = curr_region;
			}

//			draw_progress_bar(bUpStatusPer);

            if( dwUpStatus  == 0x8 && bUpStatusPer == 100 )
            {
                d1("Done!!!\n");      
                break;
            }        
        }
    }
//	END(s,e);

    //Acoording to Ststus value, I have to add something to handle f0.
    //To be implemented...
    
Exit:
    return dwRet;
	
}

int dpram_write_start_phase(Req_Packet *pStartReq)
{
    u16 intr = 0;
    int nRetry = 0;
	int ret = 0;
    Req_Packet pReq;


    pReq.DownMode = 0x0000; //pStartReq->DwMode;
    pReq.StartAddress = 0x00000000;//pStartReq->StartAddr;
    pReq.dwLength = 0x00000000;//pStartReq->Length;
    
    dpram_write_command(CMD_DL_START_REQ);
    dpram_write((void*) (DPRAM_BASE_PTR + DPRAM_PDA2PHONE_FORMATTED_START_ADDRESS),(void*)&pReq, sizeof(Req_Packet));

    while(nRetry < 3)
    {
		wait_for_dpram_int(1000,0);

        intr = dpram_read_command();
        d1("intr = 0x%x\n", intr);

		ret = check_command(intr, MASK_CMD_DOWNLOAD_START_RESPONSE);
		if(ret == CMD_TRUE)
		{
			d1("DOWNLOAD_START_RESPONSE issued.\n");
			return TRUE;
		}
		else if(ret == CMD_FALSE)
		{
			d1("DOWNLOAD_START_RESPONSE failed.\n");
			return FALSE;
		}

		nRetry++;
    }
    return FALSE;
}

int dpram_write_EFS_clear_phase(s32 bEFSClear)
{
    u16 intr = 0;
    int nRetry = 0;
	int ret = 0;
    Req_Packet pReq;

	if(!bEFSClear) return TRUE;

	d1("EFS Clear Phase\n");

#if 1
    pReq.DownMode = 0x0000; //pStartReq->DwMode;
    pReq.StartAddress = 0x00000000;//pStartReq->StartAddr;
    pReq.dwLength = 0x00000000;//pStartReq->Length;
#endif

    dpram_write_command(CMD_EFS_CLEAR_REQ);
    dpram_write((void*) (DPRAM_BASE_PTR + DPRAM_PDA2PHONE_FORMATTED_START_ADDRESS),(void*)&pReq, sizeof(Req_Packet));

    while(nRetry < 3)
    {
		wait_for_dpram_int(100,0);

        intr = dpram_read_command();
        d1("intr = 0x%x\n", intr);

		ret = check_command(intr, MASK_CMD_EFS_CLEAR_RESPONSE);
		if(ret == CMD_TRUE)
		{
			d1("EFS Clear issued.\n");
			return TRUE;
		}
		else if(ret == CMD_FALSE)
		{
			d1("EFS Clear failed.\n");
			return FALSE;
		}

		nRetry++;
    }
    return FALSE;
}



//---------------------------------------------------------------------------
//Function name: dpram_send_done_phase
//Function     : After Seding the total number of Phone image,
//               to notify the phone that there's no more image,
//               wait for the reponse from Phone.
//---------------------------------------------------------------------------
u32 dpram_send_done_phase(int bBootUpdate)
{
    u16 intr = 0;
    int nRetry = 0;    
	int ret = 0;

    // Send Done Request
    dpram_write_done_request((bBootUpdate & PHONE_BOOT_UPDATE));
    //    dpram_write_command(MASK_CMD_IMAGE_SEND_REQUEST);

    while(nRetry < 3)
    {
		wait_for_dpram_int(10,0);

        intr = dpram_read_command();
        d1("intr = 0x%x\n", intr);

		ret = check_command(intr, MASK_CMD_SEND_DONE_RESPONSE);
		if(ret == CMD_TRUE)
		{
			if((intr & MASK_CMD_RESULT_FAIL) == MASK_CMD_RESULT_FAIL)
			{
				d1("SEND DONE Response Failed.\n");
			}
			else
			{
				d1("SEND DONE Response succeeded.\n");
				return TRUE;
			}
		}
		else if (ret = CMD_FALSE)
		{
			d1("Invalid command!\n");

			if((intr & MASK_CMD_STATUS_UPDATE_NOTIFICATION) == MASK_CMD_STATUS_UPDATE_NOTIFICATION)
			{
				d1("but Update Notification comes..\n");
				return TRUE;
			}
			return FALSE;

		}

		nRetry++;
    }
    return FALSE;
}

//---------------------------------------------------------------------------
//Function name: dpram_status_noti_phase
//Function     : To be notified which image of Phone.bin is downloade
//               wait for the Status Update Notification.
//Return Value : 0 - fail
//               0x1 - PBL
//               0x2 - QCSBLHD
//               0x3 - MIBIB1
//               0x4 - MIBIB2
//               0x5 - QCSBL
//               0x6 - OEMSBL
//               0x7 - FOTA ENGINE
//               0x8-  AMSS
//---------------------------------------------------------------------------
u32   dpram_status_noti_phase(u8 *pPercent)
{
    Status_UpNoti UpStatus;
    int nRetry =0;
    u16 intr = 0;
    u32 dwRet = 0;
	int ret = 0;

    UpStatus.Bop = 0x00;
    UpStatus.Region = 0x00;
    UpStatus.Percent = 0x00;
    UpStatus.Eop = 0x00;

    while(nRetry < 3)
    {        
		wait_for_dpram_int(10,20);

        intr = dpram_read_command();
		d2("intr = 0x%x\n", intr);

		ret = check_command(intr, MASK_CMD_STATUS_UPDATE_NOTIFICATION);
		if(ret == CMD_TRUE)
		{
			if((intr & MASK_CMD_UPDATE_DONE_NOTIFICATION) == MASK_CMD_UPDATE_DONE_NOTIFICATION)
			{	d1("Update DONE Notification issued.\n"); }
			else
			{	d2("Update Notification issued.\n"); }

			nRetry = 3;

		}
		else if( ret == CMD_FALSE)
		{
			// Download Send Done Response
			d1("The Result of SEND DONE Response succeeded.\n");
			return dwRet;
		}
		else 
		{
			nRetry++;
		}
    }

    //Read data from Formatted section of DPRAM,and figure out which status has done.
    dpram_read((void*)&UpStatus, (void*)(DPRAM_BASE_PTR + DPRAM_PDA2PHONE_FORMATTED_START_ADDRESS), 8 );

    d2("0x%x  0x%x  0x%x  0x%x \n", UpStatus.Bop, UpStatus.Region, UpStatus.Percent, UpStatus.Eop);   
 
    if((UpStatus.Bop != START_INDEX) || (UpStatus.Eop != END_INDEX))
    {
        d1("Packet format is wrong.\n");
        dwRet = 0;
    }
    else
    {
        d2("Get the Status info.\n");
        dwRet = (u32)UpStatus.Region;
        *pPercent = (u8)UpStatus.Percent;
    }

    return dwRet;    
}

int dpram_update_done_noti_phase(void)
{
    int nRetry =0;
    u16 intr = 0;
	int ret = 0;

    //Wait for Update Done Notification.
    while(nRetry < 3)
    {
		wait_for_dpram_int(1000,0);

        intr = dpram_read_command();
        d1("intr = 0x%x\n", intr);

		ret = check_command(intr, MASK_CMD_UPDATE_DONE_NOTIFICATION);
		if(ret == CMD_TRUE)
		{
			d1("Update Done Notification issued.\n");
			return TRUE;
		}
		else if(ret == CMD_FALSE)
		{
			d1("Update Done Notification is wrong.\n");
			return FALSE;
		}
		nRetry++;
	}
    return FALSE;
}


//---------------------------------------------------------------------------
// Read data from DPRAM
//---------------------------------------------------------------------------
inline static u32 dpram_read(void* dest, void* src, u32 size)
{
	MemCopy((u32)dest, (u32)src, size);
    return size;
}


//---------------------------------------------------------------------------
// Write data to DPRAM
//---------------------------------------------------------------------------
inline static u32 dpram_write(void* dest, void* src, u32 size)
{
	MemCopy((u32)dest, (u32)src, size);
	return size;
}

	
//---------------------------------------------------------------------------
// Initialize DPRAM
//---------------------------------------------------------------------------
void dpram_init(void)
{
    u32 mSet;
    d0("Entered dpram_init \n");

	DPRAM_BASE_PTR = (u8*)DPRAM_BASE_ADDR;
    d0("DPRAM base addr: 0x%08x \n", DPRAM_BASE_ADDR);
	
    // Pass clock to SROM controller
    CLK_GATE_IP1_REG |= (0x01 << 26);

    // Set the bus width, Byte enables, 16 bit addressing etc
    SROM_BW_REG &= ~0x0000F000;
    SROM_BW_REG |= (0x0D << 12);

    // Set wait states
    SROM_BC3_REG = 0x00040000;

    //Config pins to Address and Data pins
    *(volatile u32 *)(GPJ0CON) = 0x22222222;	//Addr lines[7:0]

    mSet = *(volatile u32 *)(GPJ1CON);
    mSet &= ~0x000FFFFF;
    mSet |= 0x00022222;
    *(volatile u32 *)(GPJ1CON) = mSet;

    *(volatile u32 *)(MP06CON) = 0x22222222;	//Data lines[7:0]
    *(volatile u32 *)(MP07CON) = 0x22222222;	//Data lines[15:8]

    // Config ChipSel3
    mSet = *(volatile u32 *)(MP01CON);
    mSet &= ~0x0000F000;
    mSet |= 0x00002000;
    *(volatile u32 *)(MP01CON) = mSet;
	
    s5p_gpio_setcfg(GPIO_VIA_RST_N, GPIO_VIA_RST_N_AF);

    // Set pull down enable on VIA_RST
    mSet = *(volatile u32 *)(GPH3PUD);
    mSet &= ~0x0000C000;
    mSet |= 0x00004000;
    *(volatile u32 *)(GPH3PUD) = mSet;

   // Set 3x driver on VIA_RST
    mSet = *(volatile u32 *)(GPH3DRV);
    mSet &= ~0x0000C000;
    mSet |= 0x00004000;
    *(volatile u32 *)(GPH3DRV) = mSet;
 
    s5p_gpio_setpull(GPIO_VIA_RST_N, GPIO_PULL_DISABLE);
	
    s5p_gpio_setcfg(GPIO_VIA_ACTIVE, GPIO_VIA_ACTIVE_AF);
    s5p_gpio_setcfg(GPIO_INT_DPRAM_R, GPIO_INT_DPRAM_R_AF);

    *(volatile u32 *)DPRAM_BASE_PTR = 0x5555AAAA;
    mSet = *(volatile u32 *)DPRAM_BASE_PTR;
    d0("At base addr: 0x%08x \n", mSet);

    d0("DPRAM_PHONE2PDA_INTERRUPT_ADDRESS = 0x%08x \n", DPRAM_PHONE2PDA_INTERRUPT_ADDRESS);
    d0("DPRAM_PDA2PHONE_INTERRUPT_ADDRESS = 0x%08x \n", DPRAM_PDA2PHONE_INTERRUPT_ADDRESS);


}    

void phone_shutdown(void)
{
    s5p_gpio_setpin(GPIO_VIA_RST_N, GPIO_LEVEL_LOW);
}


//---------------------------------------------------------------------------
// Write Command
//---------------------------------------------------------------------------
inline void dpram_write_command(u16 nCmd)
{
	d2("Start : 0x%04x\n", nCmd);

	dpram_write((void*)(DPRAM_BASE_PTR + DPRAM_PDA2PHONE_INTERRUPT_ADDRESS), (void*)&nCmd, 2);
	
	//d1("End\n");
}

#if 0
u32 dpram_check_alarm_boot(void)
{

	u16 command=0, ret=0;
	u32 wait_time=0, nRetry=0, i=0;
	dpram_init();


    for( i = 0; i < 3 ; i++) 
    {
        if (dpram_write_magic_code(MAGIC_ALARMBOOT))
		{
			d0("magic code is ok!\n");
			break;
		}
    }  
#if 0
	gpio_set_level(GPIO_MSM_RST, GPIO_LEVEL_LOW);
	msleep(100);
	gpio_set_level(GPIO_MSM_RST, GPIO_LEVEL_HIGH);
#endif

#if 0
	while (!gpio_get_level(GPIO_PHONE_ACTIVE))
	{
        if(wait_time >= 100)
        {
            printf("no phone active!! \n");
            return FALSE;
        }		
        msleep(100);
        wait_time++;
		if(!(wait_time % 10)) printf(".");
    }

	wait_time=0;
#endif

	ret = wait_for_dpram_int(300,20);
	if(!ret){
		d1("Can't Receive Alarm Booting Command from Modem\n");
		return 1;
	}

	command = dpram_read_command();
	d1("command = 0x%x\n", command);

	if(command == CMD_ALARM_BOOT_OK)
	{
		d1("Receive Alarm Booting OK.\n");
		ret =1;
	}
	else if(command == CMD_ALARM_BOOT_FAIL)
	{
		d1("Receive Alarm Booting NO\n");
		ret =0;
	}
	else
	{
		d1("Receive Unknown Command : 0x%x\n", command );
		ret =0;
	}

	phone_shutdown();

	d1("Send Power Off Command\n");
	dpram_write_command(CMD_SEND_MODEM_POWEROFF);

	while (gpio_get_level(GPIO_ALARM_AP))
	{
		if(wait_time >= 50)
		{
			printf("phone activated!! \n");
			break; 
		}		
		msleep(100);
		wait_time++;
		if(!(wait_time % 10)) printf(".");
	}

	d1("Clear DPRAM\n");
	/* DPRAM Clear.. */
	for (i = DPRAM_START_ADDRESS; i <= DPRAM_END_OF_ADDRESS; i ++)
		*(DPRAM_BASE_PTR + i) = 0;

	return ret;
}
#endif

//---------------------------------------------------------------------------
// Read Command
//---------------------------------------------------------------------------
inline static u16 dpram_read_command(void)
{
	u16 cmd = 0;
	

	dpram_read((void*)&cmd, (void*)(DPRAM_BASE_PTR + DPRAM_PHONE2PDA_INTERRUPT_ADDRESS), 2);

	return cmd;
}


//---------------------------------------------------------------------------
// Write Magic Code
//---------------------------------------------------------------------------
static u32 dpram_write_magic_code(u32 code)
{
    u32 address = 0;
    u32 value = 0;

    address = (u32)(DPRAM_BASE_PTR + DPRAM_MAGIC_CODE_ADDRESS);
    //address = (u32)(DPRAM_BASE_PTR);

    d0("Code : 0x%08x, Magic code address : 0x%08x\n", code, address);

    dpram_write((void*)address, (void*)&code, 4);

    dpram_read( (void *)&value,(void *) address, 4 );    

    d0("DPRAM Read Magic = 0x%x\n", value);

	return (value == code);

}


//---------------------------------------------------------------------------
// Write 'Write Done' to DPRAM 
//---------------------------------------------------------------------------
static void	dpram_write_done_request(int bBootUpdate)
{
	d1("Start\n");
	if(bBootUpdate)
	{
		d1("Phone Boot update True\n");
		dpram_write_command(CMD_DL_SEND_DONE_REQ_BOOT_UPDATE);
	}
	else
	{
		d1("Phone Boot update False\n");
		dpram_write_command(CMD_DL_SEND_DONE_REQ);
	}
	d1("end\n");
}


//#define DMDL_DEBUG

//---------------------------------------------------------------------------
// Write Image data to DPRAM
//---------------------------------------------------------------------------
static u32 dpram_write_image(const u8* pBuf, u32 dwWriteLen)
{
    // u8*  pSrc;
    u8*  pDest;
    u8* pDest_Data;
    u16 Len;
    u16 nCrc;

#ifdef DMDL_DEBUG
    u8*  pDest_debug;
    u8*  pBuf_debug = pBuf;
    u32 i;
#endif

    //d1("Start %d 0x%04x(%d)\n", dwWriteLen, g_TotFrame, g_TotFrame);

    pDest = (u8*)(DPRAM_BASE_PTR + DPRAM_PDA2PHONE_FORMATTED_START_ADDRESS);
    Len   = (u16)min(dwWriteLen, DPDN_DEFAULT_WRITE_LEN);

    //d1("Start : pDest(0x%08x),  dwWriteLen(%d)\n",pDest, Len);

    // Start Index
//#if 1
//    *pDest++ = LOBYTE(START_INDEX);
 //   *pDest++ = HIBYTE(START_INDEX);
//#else
    *pDest++ = START_INDEX;
//#endif

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
        d1("odd  0x%08x  0x%02x\n", pDest-1, (u8)(*pDest-1));
    }

    //d1("len:%d default len:%d\n", Len, DPDN_DEFAULT_WRITE_LEN);
    //d1("start data 0x%08x \n", pDest);
	
    if(Len < DPDN_DEFAULT_WRITE_LEN )
    {
        d2("%d < %d, left : %d\n", Len, DPDN_DEFAULT_WRITE_LEN, 
				(DPRAM_BASE_PTR + DPRAM_PDA2PHONE_INTERRUPT_ADDRESS - pDest -WRITEIMG_TAIL_SIZE));

#if 0
        // Fill padding - 0xff
        memset((void*)pDest, 0x0 /*0xff*/, 
        	DPRAM_BASE_PTR + DPRAM_PDA2PHONE_INTERRUPT_ADDRESS - pDest -WRITEIMG_TAIL_SIZE );
        pDest = (u8*)(DPRAM_BASE_PTR + DPRAM_PDA2PHONE_INTERRUPT_ADDRESS - WRITEIMG_TAIL_SIZE);
#endif
        memset((void*)pDest, 0x0 /*0xff*/, DPDN_DEFAULT_WRITE_LEN - Len );
        pDest += (DPDN_DEFAULT_WRITE_LEN - Len) ;
    }
    d2("CRC start 0x%08x\n", pDest);

    nCrc = MakeCRC(Len,(u16 *)pDest_Data);

    *pDest++ = LOBYTE(nCrc);
    *pDest++ = HIBYTE(nCrc);

     d2("CRC value 0x%04x \n", nCrc);

     pDest = (u8*)(DPRAM_BASE_PTR + BSP_DPRAM_BASE_SIZE - DPRAM_INTERRUPT_SIZE*2 - DPRAM_INDEX_SIZE);
//#if 1
//    *pDest++ = LOBYTE(END_INDEX);
//   *pDest++ = HIBYTE(END_INDEX);
//#else
    *pDest++ = END_INDEX;
//#endif


#ifdef DMDL_DEBUG
    
    pDest_debug = (u8*)(DPRAM_BASE_PTR + DPRAM_PDA2PHONE_FORMATTED_START_ADDRESS);
    d1("debug start 0x%08x  0x%08x\n", pDest_debug, pDest);
    //while( pDest_debug < pDest )
    for( i=0; i <WRITEIMG_HEADER_SIZE ; i++)
    {
        printf(TEXT("%02x "), *pDest_debug++);        
    }

	printf("\n");
    d1("body start pdest %02x  pbuf %02x \n", pDest_debug,  pBuf_debug);        
    //MakeCRC_DUMP(Len,(u16 *)pDest_debug);
    //printf("\n dump2---------- \n");        
    //MakeCRC_DUMP(Len,(u16 *)pBuf_debug);
    for( i=0; pDest_debug < pDest -WRITEIMG_TAIL_SIZE; i++, pDest_debug++, pBuf_debug++)
    {
        if( *pDest_debug != *pBuf_debug )
        {
            printf("%09d %02x %02x \n", i, *pDest_debug, *pBuf_debug);                    
        }
    }     
    d1("end check idx:%09d\n", i);   
    for( i=0; i <WRITEIMG_TAIL_SIZE; i++)
    {
        printf(TEXT("%02x "), *pDest_debug++);        
    }
	printk("\n");
    d1("debug end 0x%08x  0x%08x\n", pDest_debug, pDest);

#endif

    // Write Command
    dpram_write_command(CMD_IMG_SEND_REQ);

    return Len;
}



u16 calc_total_frame(u32 nDividend, u16 nDivisor)
{
    u16 nCompVal1 = 0;
    u16 nCompVal2 = 0;

    d1("%d %d\n", nDividend, nDivisor);
    nCompVal1 = (u16) (nDividend / nDivisor);
    nCompVal2 = (u16) (nDividend  - (nCompVal1 * nDivisor));
    if( nCompVal2 > 0 ) 
    {
        d1("val2 : %d\n", nCompVal2);
         nCompVal1++;
    }
    d1("result %d\n", nCompVal1);
    return nCompVal1;
}

/*
//---------------------------------------------------------------------------
// Read upgrade progress info from DPRAM
//---------------------------------------------------------------------------
int DPRAM_ReadUpdateInfo(u32* dwCur, u32* dwTot, u8* StateCode)
{
	u8  ErrorCode;
	
    printf(TEXT("[DPRAM_ReadUpdateInfo] Start\n"));

	// dpram_read
	dpram_read((void*)StateCode		, (void*)(DPRAM_BASE_PTR + DPRAM_PDA2PHONE_FORMATTED_START_ADDRESS + 0), 1);
	dpram_read((void*)&ErrorCode	, (void*)(DPRAM_BASE_PTR + DPRAM_PDA2PHONE_FORMATTED_START_ADDRESS + 1), 1);
	dpram_read((void*)dwCur         , (void*)(DPRAM_BASE_PTR + DPRAM_PDA2PHONE_FORMATTED_START_ADDRESS + 2), 4);
	dpram_read((void*)dwTot         , (void*)(DPRAM_BASE_PTR + DPRAM_PDA2PHONE_FORMATTED_START_ADDRESS + 6), 4);

	printf(TEXT("[DPRAM_ReadUpdateInfo] End : Status(0x%02x), ErrorCode(0x%02x), dwCur(0x%08x), dwTot(0x%08x)\n"), *
StateCode, ErrorCode, *dwCur, *dwTot);

	return TRUE;
}

void PhoneReset()
{
	PhonePowerOff();
	PhonePowerOn();
}
*//*
u16 calc_total_frame(u16 nDividend,u16 nDivisor)
{
    float fCompVal1 = 0;
    u16 nCompVal2 = 0;

    fCompVal1 = ((long)nDividend /(long)nDivisor);

    nCompVal2 = nDividend / nDivisor;

    if((fCompVal1 - (long)nCompVal2) > 0)
    {
        return (nCompVal2+1);
    }
    else
    {
        return nCompVal2;
    }
}*/

u16 MakeCRC(u16 length, u16* data_ptr_WORD)
{
    u16  data_crc = CRC_16_L_SEED;
    u16  const * table = CRC_Table;
    u8 * data_ptr = (u8 *) data_ptr_WORD;
    int i;

    for (i=0; i<length; i++) { 
        data_crc = (((data_crc) >> 8) ^ table[((data_crc) ^ (u16)(data_ptr[i])) & 0x00ff]);
        //printf(TEXT("%010d:0x%04x\r\n"), i, data_crc ); 
    }
    //printf(TEXT("[MakeCRC] length:%d pt:0x%08x i:%d v:0x%04x \r\n"), length,  data_ptr, i, data_ptr[i] ); 
    return data_crc;

}

u16 MakeCRC_DUMP(u16 length,u16* data_ptr_WORD)
{
    u16  data_crc = CRC_16_L_SEED;
    u16  const * table = CRC_Table;
    u8 * data_ptr = (u8 *) data_ptr_WORD;
    int i;

    for (i=0; i<length; i++) { 
        data_crc = (((data_crc) >> 8) ^ table[((data_crc) ^ (u16)(data_ptr[i])) & 0x00ff]);        
        printf(TEXT("%010d:0x%04x\r\n"), i, data_crc ); 
    }
    printf(TEXT("[MakeCRC] length:%d pt:0x%08x i:%d v:0x%04x \r\n"), length,  data_ptr, i, data_ptr[i] ); 
    return data_crc;

}

// Memory Handling
void MemCopyByte2(u32 dwDest, u32 dwSrc)
{
	u16 *pwSrc;
	u16 *pwDest;

	if( 0 == dwDest % 2 && 0 == dwSrc % 2 )
	{
		pwDest = (u16*)dwDest;
		pwSrc = (u16*)dwSrc;
		*pwDest = (*pwDest & (u16)0xFF00) | (*pwSrc & (u16)0x00FF);
	}
	else if( 1 == dwDest % 2 && 1 == dwSrc % 2 )
	{
		pwDest = (u16*)(dwDest - 1);
		pwSrc = (u16*)(dwSrc - 1);
		*pwDest = (*pwDest & (u16)0x00FF) | (*pwSrc & (u16)0xFF00);
	}
	else if( 0 == dwDest % 2 && 1 == dwSrc % 2 )
	{
		pwDest = (u16*)dwDest;
		pwSrc = (u16*)(dwSrc - 1);
		*pwDest = (u16)((u16)(*pwDest & (u16)0xFF00) | (u16)(*pwSrc >> 8 & (u16)0x00FF));
	}
	else if( 1 == dwDest % 2 && 0 == dwSrc % 2 )
	{
		pwDest = (u16*)(dwDest - 1);
		pwSrc = (u16*)dwSrc;
		*pwDest = (u16)((u16)(*pwDest & (u16)0x00FF) | (u16)(*pwSrc << 8 & (u16)0xFF00));
	}
}


void MemCopyByte(u32 dwDest, u32 dwSrc)
{
	if( !(dwDest % 2) && !(dwSrc % 2) )
		*((u16*)dwDest) = (*((u16*)dwDest) & (u16)0xFF00) | (*((u16*)dwSrc) & (u16)0x00FF);
	else if( (dwDest % 2) && (dwSrc % 2) )
		*((u16*)(dwDest - 1)) = (*((u16*)(dwDest - 1)) & (u16)0x00FF) | (*((u16*)(dwSrc - 1)) & (u16)0xFF00);
	else if( !(dwDest % 2) && (dwSrc % 2) )
		*((u16*)dwDest) = (u16)((u16)(*((u16*)dwDest) & (u16)0xFF00) | (u16)(*((u16*)(dwSrc - 1)) >> 8 & (u16)0x00FF));
	else if( (dwDest % 2) && !(dwSrc % 2) )
		*((u16*)(dwDest - 1)) = (u16)((u16)(*((u16*)(dwDest - 1)) & (u16)0x00FF) | (u16)(*((u16*)dwSrc) << 8 & (u16)0xFF00));
}

#if 0
BOOL MemEqual(void* pDest, void* pSrc, u32 dwSize)
{
	u32 i;

	for( i = 0; i < dwSize; i++ )
	{
		if( *((u8*)pDest + i) != *((u8*)pSrc + i) )
			return FALSE;
	}

	return TRUE;		
}
#endif

void MemCopy(u32 pDest, u32 pSrc, u32 dwSize)
{
	//BJ: 5/31/2004
	if( dwSize==0 )
		return;
	
	if((pDest%2)^(pSrc%2)) // Hetero
	{			
		if(pDest%2)	// Dest is Odd
		{
			MemCopyByte2(pDest, pSrc);
			pDest++;
			pSrc++;
			dwSize--;
		}

		//BJ: 5/31/2004
		if( dwSize==0 )
			return;
		
		{
			// Now Dest is Even, Src is Odd
			u16	dwTmp;
			u32	dwHalfSize = (dwSize>>1);

			u32 i;

			for( i = 0; i<dwHalfSize; i++ )
			{	
				dwTmp = (u16)*((u8*)pSrc+(i<<1)) | (((u16)*((u8*)pSrc+(i<<1)+1)<<8)&(u16)0xFF00);
				// dwTmp = (((u16)*((u8*)pSrc+(i<<1))<<8)&(u16)0xFF00) | (u16)*((u8*)pSrc+(dwSize<<1)+1);
				*(((u16*)pDest) + i) = dwTmp;
			}

			if(dwSize%2) // Odd
				MemCopyByte2((pDest+dwSize-1), (pSrc+dwSize-1));
		}
	}
	else	// Homo
	{		
		if(pDest%2) // All Odd
		{
			MemCopyByte2(pDest, pSrc);
			pDest++;
			pSrc++;
			dwSize--;
		}
		// Now All Even

		//BJ: 5/31/2004
		if( dwSize==0 )
			return;
		
		{
			u32	dwHalfSize = (dwSize>>1);
			u32	i;

			for( i = 0; i<dwHalfSize; i++ )
				*(((u16*)pDest) + i) = *(((u16*)pSrc) + i);

			if(dwSize%2) // If Length is Odd
				MemCopyByte2((pDest+dwSize-1), (pSrc+dwSize-1));
		}
	}
}
#if 0
void lcd_puts(s32 fgcolor, s32 bgcolor, char *str)
{
	lcd_draw_font(10, 10 + (19 * count), fgcolor, bgcolor, strlen(str), str);

	printf("%s(%d, %d, %s)\n", __func__, fgcolor, bgcolor, str);

	if (count++ > 20) {
		count = 0;
	}
}
#endif


