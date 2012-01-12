#ifndef __IPC_V40_H__
#define __IPC_V40_H__
/******************************************************************************
 *
 * Copyright (c) 2004 SAMSUNG Co. Ltd,
 * All rights reserved.
 *
 * File: IPC.h
 *
 * Release: 
 *
 * Description: Define IPC(Inter Process Command) message
 * 
 *
 * Revision History
 * JULY/13/2004   IYCho  Initial revision 
  *****************************************************************************/


/**********************************************************************************

                       I P C   M E S S A G E   F O R M A T

***********************************************************************************/

/**********************************************************************************

History - IPC4.0 

---------------------------------------------------------------------
Jeon Bong Goo
- Overview : TBD ( open issue )
- Power : no change
- Call  : changed
- SS    : no change
- Sound : no change

Choi Jong Ho
- Data  : changed
- GPRS  : no change

Son Kee Chul 
- SMS
- Service
- IMEI
- Factory ( new )

Lee Jeong In
- Security: no change
- PhoneBook : changed
- STK

Jo In Young
- Display - changed
- Network - changed

Kim Byung Sam
- Misc   : changed
- Config : changed
 
 Park JooHyung
 - Data : chaged
 - OMA-DM : Changed
***********************************************************************************/

/**********************************************************************************

                                                                Abbreviations

***********************************************************************************

ACK : Acknowledge
ASYNC : Asynchronous
AUTO : Automatic
AVAIL : Available

BAT : Battery
BT : Blue Tooth

CBMI : Cell Broadcast Message Identifier
CBS : Cell Broadcast Service
CLI : Calling Line Identification
CTRL : Control
CTS : Clear to send
CMD : Command
CFG : Configuration
CFRM : Confirm or Confirmation

DCD : Data Carrier Detect
DEL : Delete
DISP : Display
DSR : Data Set Ready
DTR : Data Terminal Ready

ECT : Explicit Call Transfer
EQUIP : Equipment
ERR : Error
EXEC : Execution

GEN : General
GND : Ground

INDI : Indication
INFO : Information
IMSI : International Mobile station Subscriber Identity
IPC : Inter process command

LANG : Language
LEN : Length

NET : Network
NOTI : Notification
NUM : Number

MANU : Manual
MAX : maximum
MEM : Memory
MIC : Microphone
MISC : Miscellaneous
MO : Mobile Originated
MPTY : Multiparty
MSG : Message
MT : Mobile Terminated

OPER : Operation
OPT : Option

PARAM : Parameter
PB : Phonebook
PERS : Personalization
PKT : Packet
PREF : Preferred
PW : Password
PWR : Power

REGIST : Register, Registration
REL : Release
RESP : Response
REQ : Request
REQU : Required
RI : Ring indicator
RTS : Request to send
RX : Receive Data

SEC : Security
SEL : Select, Selection
SEQ : Sequence
SIM : Subscriber Identity Module
SMS : Short Message Service
SN : Serial Number
SND : Sound
SPKR : Speaker
STR : String
SVC : Service
SYNC : Synchronous
SYS : System

TA : Terminal Adaptor, e.g. a GSM data card (equal to DCE; Data Circuit terminating Equipment)
TE : Terminal Equipment, e.g. a computer (equal to DTE; Data Terminal Equipment)
TX : Transmit Data

UNAVAIL : Unavailable

VBS : Voice Broadcast Service
VER : version
VIB : Vibrator
VGCS : Voice Group Call Service
VOL : Volume



*********************************************************************************/


/**********************************************************************************

                       H D L C   F O R M A T

***********************************************************************************/
  

/*  
-----------------------------------------------------------------------------------
  | Start_Flag(0x7F) | LENGTH(2) | CONTROL(1) | IPC-MESSAGE(x)  | Stop_Flag(0x7E) |
  ----------------------------------------------------------------------------------
*/


/**********************************************************************************

                       I P C   M E S S A G E   F O R M A T

***********************************************************************************/
/*  
-----------------------------------------------------------------------------------
  | LENGTH(2) | MSG_SEQ(1) | ACK_SEQ(1) | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
  ----------------------------------------------------------------------------------
  | IPC MESSAGE(x) |
  ----------------------------------------------------------------------------------
*/


/*********************************************************************************/


/*********************************************************************************

                                                            Main Command

*********************************************************************************/
typedef enum {
  IPC_PWR_CMD=0x01,   /* 0x01 : Power Control Commands */
  IPC_CALL_CMD,            /* 0x02 : Call Control Commands */
  IPC_CDMA_DATA_CMD,    /* 0x03 : CDMA DATA Control Command */
  IPC_SMS_CMD,             /* 0x04 : Short Message Service Commands */
  IPC_SEC_CMD,              /* 0x05 : Security - SIM control Commands */
  IPC_PB_CMD,               /* 0x06 : Phonebook Control Commands */
  IPC_DISP_CMD,            /* 0x07 : Display Control Commands */
  IPC_NET_CMD,             /* 0x08 : Network Commands */
  IPC_SND_CMD,             /* 0x09 : Sound Control Commands */
  IPC_MISC_CMD,           /* 0x0A : Miscellaneous Control Commands */
  IPC_SVC_CMD,             /* 0x0B : Service Mode Control Commands - Factory Test or Debug Screen Control */
  IPC_SS_CMD,               /* 0x0C : Supplementary Service Control Command */
  IPC_GPRS_CMD,           /* 0x0D : GPRS(AT Command to IPC) Commands */  // joonook 20041011 create
  IPC_SAT_CMD,             /* 0x0E : SIM Toolkit Commands */
  IPC_CFG_CMD,             /* 0x0F : Configuration Commands */
  IPC_IMEI_CMD,             /* 0x10 : IMEI Tool Commands */
  IPC_GPS_CMD,          /* 0x11 : GPS Control Commands */ // JG.JANG JAN.08.2007
  IPC_SAP_CMD,          /* 0x12 : SIM Access Profile Commands */
  IPC_FACTORY_CMD,      /* 0X13 : Factory Test Commands */
  IPC_OMADM_CMD,      /* 0X14 : OMA DM Commands */
  IPC_GEN_CMD=0x80,    /* 0x80 : General Response Command */
  IPC_CMD_MAX
} ipc_main_cmd_type;

/*********************************************************************************/


/*********************************************************************************

                                                         Command Type( PDA -> PHONE )

*********************************************************************************/
typedef enum {
  IPC_CMD_EXEC=0x01,         /* 0x01 : Execute - Request the action to phone */
  IPC_CMD_GET,                    /* 0x02 : Read the data from phone */
  IPC_CMD_SET,                    /* 0x03 : Write the data to phone, Response must be GENERAL_RESPONSE */
  IPC_CMD_CFRM,           /* 0x04 : Response of the INDICATION */
  IPC_CMD_EVENT,                /* 0x05 : Notify the status of PDA */
  IPC_PDA_MAX
} ipc_pda_cmd_e_type; 

/*********************************************************************************/


/*********************************************************************************

                                                         Command Type( PHONE -> PDA )

**********************************************************************************/
typedef enum {
  IPC_CMD_INDI=0x01,       /* 0x01 : Indication - Request the action or data to PDA */
  IPC_CMD_RESP,                /* 0x02 : Response - Response of the GET */
  IPC_CMD_NOTI,                /* 0x03 : Notification - Notify the status of phone */
  IPC_PHONE_MAX
} ipc_phone_cmd_e_type;

/*********************************************************************************/

/*********************************************************************************

                                                         Maximum Size

**********************************************************************************/

/*  IPC_PWR_CMD [0x01]  */

/*  IPC_CALL_CMD [0x02]  */
#define MAX_CALL_DIALED_DIGITS_NUM           82
#define MAX_CALL_LIST_NUM                     9

//peh_2009.01.02  temp_compile_error  error   ߰ ߿ ϼ ~~
//peh_start
#define MAXLENGTH_CALLED_DIGITS_SIZE					48//32		/* Maximum number of Calling Digits */         //Jeon Bong Goo add
#define MAXLENGTH_BURSTDTMF_DIGIT						32
#define MAXLENGTH_FLASHINFO_CHARS						32
#define MAXLENGTH_IOTA_DATA_SIZE						238		/* Mixmum IOTA  DATA SIZE */
#define MAXLENGTH_SPECIAL_NUM_DIGIT						32
#define MAXLENGTH_CALL_ALERT_INFO_NUMCHARS				32
#define MAXLENGTH_IOTA_PROCESSING_REVMSG				243
//peh Ʒ ipcdef.h  ִ 
#define IPC_MAX_CHARI_LEN				MAXLENGTH_CALLED_DIGITS_SIZE		/* Maximum CALL Origination Digits */
//peh_end

/*  IPC_DATA_CMD [0x03]  */

// NAI
#define MAXLENGTH_USER_NAI                             72
#define MAXLENGTH_MN_AAA_SHARED_SECRET   16
#define MAXLENGTH_MN_HA_SHARED_SECRET     16
#define MAX_NAI_LENTH                    130 // murali.yadav - mdified as per discussion with Modem team

/*  IPC_SMS_CMD [0x04]  */
/* -------------------------- CDMA SMS --------------------------- */
#define MAX_CDMA_SMS_DATA_SIZE              512 /* Maximum number of bytes SMSP Record size (Y + 28), y : 0 ~ 128 */
#define MAX_CDMA_SMS_ADDRESS_SIZE           32 /* MAX sms destination(or origination ) address /call back number */

/* -------------------------- GSM SMS --------------------------- */
#define MAX_GSM_SMS_TPDU_SIZE               244
#define MAX_GSM_SMS_MSG_NUM                 255 
#define MAX_GSM_SMS_SERVICE_CENTER_ADDR     12  /* Maximum number of bytes of service center address */
#define MAX_GSM_SMS_CBMI_LIST_SIZE          100 /* Maximum number of CBMI list size for CBS 30*2=60  */
#define MAX_GSM_SMS_PARAM_RECORD_SIZE       156 /* Maximum number of bytes SMSP Record size (Y + 28), y : 0 ~ 128 */
#define MAX_GSM_SMS_STATUS_FILE_SIZE        2 /* Last Used TP-MR + SMS "Memory Cap. Exceeded" Noti Flag */


/*  IPC_SEC_CMD [0x05]  */
#define MAX_SEC_PIN_LEN                 8
#define MAX_SEC_PUK_LEN                 8
#define MAX_SEC_PHONE_LOCK_PW_LEN       39 /* Maximum Phone Locking Password Length */
#define MAX_SEC_SIM_DATA_STRING         256 /* Maximum Length of the DATA or RESPONSE 
                                                                                         ** Restricted SIM Access, Generic SIM Access Message
                                                                                          */ // JBG 2005.05.20
#define MAX_SEC_NUM_LOCK_TYPE            8  /*Maximum number of Lock Type used in Lock Information Message */                                                                                        
#define MAX_SEC_IMS_AUTH_LEN             512 /*Maximum Length of IMS Authentication Message*///bjg 2006.11.03 for ims

/*  IPC_PB_CMD [0x06]  */
#define MAX_PB_NUM_LEN                  256
#define MAX_PB_TEXT_LEN                 256
#define MAX_PB_GROUP_NAME_CHAR          256
#define MAX_PB_GROUP_RECODE             10

/*  IPC_DISP_CMD [0x07]  */
#define MAX_DISP_O2_HOMEZONE_TAG_LEN        13 /* Maximum number of bytes Home Zone Tag name */
#define MAX_DISP_ERI_TEXT_INFO_LEN          32 //49
#define MAX_DISP_PHONE_FATAL_TEXT_LEN       64

/*  IPC_NET_CMD [0x08]  */
#define MAX_NET_PLMN_NAME_LEN               16
#define MAX_NET_PLMN_NAME_SHORT_LEN         8
#define MAX_NET_PLMN_NAME_LONG_LEN          16
#define MAX_NET_PLMN_NAME_NUMERIC_LEN       6                /* Max PLMN number (MCC(3)+MNC(3)), '#' is allowed in the last position if MNC is 2 digits */
#define MAX_NET_SUBS_NUM_LEN                40               /* Max Subscriber Number Length */
#define MAX_NET_SUBS_ALPHA_LEN              16               /* Max Subscriber Name Length */
#define MAX_NET_SUBS_SVC_NUM                6  /* The Max number of the Subscriber's services */
#define MAX_NET_PLMN_LIST_NUM               20             /* Max PLMN List Number */
#define MAX_NET_PREF_PLMN_NUM               150              /* Max Preferred Operator List Number */
#define MAX_NET_NW_SHORT_NAME_LEN           16              /* Max Network Short Name Length */
#define MAX_NET_NW_FULL_NAME_LEN            32              /* Max Network Full Name Length */

/*  IPC_SND_CMD [0x09]  */
#define MAX_SND_VOL_TYPE                20 /* Used in the Sound Speaker Volume Control Response Message */
#define MAX_SND_MIC_TYPE                3 /* Used in the Sound MIC Gain Control Response Message */

/*  IPC_MISC_CMD [0x0A]  */
#define MAX_MISC_SW_VERSION_LEN             32
#define MAX_MISC_HW_VERSION_LEN             32
#define MAX_MISC_RF_CAL_DATE_LEN            32
#define MAX_MISC_PRODUCT_CODE_LEN           32
#define MAX_MISC_MODEL_ID_LEN               17
#define MAX_MISC_PRL_ERI_VER_LEN            17
#define MAX_MISC_KEY_STRING_LEN             250
#define MAX_MISC_IMSI_LEN                   15
#define MAX_MISC_ME_SN_LEN                  32 /* Mobile Equipment Serial Number Length */
#define MAX_MISC_MCC_LEN                    3
#define MAX_MISC_MNC_LEN                    2
#define MAX_MISC_MIN_LEN                    10
#define MAX_MISC_MDN_LEN                    15
#define MAX_MISC_NAM_NAME_LEN               17
#define MAX_MISC_PHONE_DEBUG_LEN            450              /* Phone debug message max length*/

/*  IPC_SVC_CMD [0x0B]  */
#define MAX_SVC_LCD_WIDE                    31 // JY.LEE change 16 -> 31 EJ18
#define MAX_SVC_LINE_COUNT                  16

/*  IPC_SS_CMD [0x0C]  */
#define MAX_SS_BARR_PW_LEN                  4               /* Max Call Barring Password Length */
#define MAX_SS_CLASS_NUM                    8
#define MAX_SS_USSD_STRING_LEN              182    /* Max USSD(Unstructured SS) String Length */
#define MAX_SS_RELEASE_COMPLETE_DATA_LEN    260    /* Max Release Complete Data Length */
#define MAX_SS_INFO_NUMBER_LEN              32  /* Max Number Length of SS Info */
#define MAX_SS_INFO_NAME_LEN                82  /* Max Name Length of SS Info */

/*  IPC_GPRS_CMD [0x0D]  */
#define MAX_GPRS_APN_LEN                101 /* Qualcomm max size( 100 )+1 */
#define MAX_GPRS_PDP_ADDRESS_LEN        20
#define MAX_GPRS_NUM_PDP                3
#define MAX_GPRS_NUM_QOS                3
#define MAX_GPRS_NUM_ACT                3
#define MAX_GPRS_USERNAME_LEN           32 /* PDP Context Auth */
#define MAX_GPRS_PASSWORD_LEN           32 /* PDP Context Auth */
#define MAX_GPRS_DNS_LEN                16 /* PDP Context DNS  */


/*  IPC_SAT_CMD [0x0E]*/
#define MAX_SAT_PROFILE_LEN             20
#define MAX_SAT_ENVELOPE_CMD_LEN        256
#define MAX_SAT_ENVELOPE_RESP_LEN       256
#define MAX_SAT_PROACTIVE_CMD_LEN       256
#define MAX_SAT_PROACTIVE_RESP_LEN      256
#define MAX_SAT_EVENT_DOWN_RESP_LEN     256
#define MAX_SAT_PROVIDE_LOCAL_INFO_LEN  58   /*Max Length of Provide Local Info Data*/
#define MAX_SAT_EVENTLIST_LEN           17    /*Max Length of Event List */
#define MAX_SAT_CALL_CONTROL_ADDRESS    200   /*Max Length of call control address*/
#define MAX_SAT_IMG_LEN                 256

/*  IPC_CONFIG_CMD [0x0F]  */
#define MAX_CFG_PHONE_CFG_ITEM_DATA_LEN     128 /* Maximum Phone Configuration Item Data Length */
#define MAX_CFG_MAC_ADDR_SIZE               6 /* Maximum MAC Address Length */
#define MAX_CFG_AKEY_LEN                    26
#define MAX_CFG_MSL_LOCK_CODE_LEN           6
#define MAX_CFG_USER_LOCK_CODE_LEN          40
#define MAX_CFG_LMSC_DOMAIN_SIZE            100 /* CDMA only */


/*  IPC_IMEI_CMD [0x10]  */
#define MAX_IMEI_ITEM_DATA_LEN              450

/*  IPC_SAP_CMD [0x12]  */
#define MAX_SAP_CMD_APDU_LEN                256

/* IPC_FACTORY_CMD[0X13] */
#define MAX_WIFI_DATA_PDA	13
#define MAX_WIFI_DATA_PC	16
#define MAX_BT_MACADDRESS	6
#define MAX_PIN_CODE		4
#define MAX_LINE_SMS_ADDR	20
#define MAX_LINE_SMS_DATA	256
#define MAX_AXIS				4
#define MAX_MEM_TOTAL_SIZE	4
#define MAX_MEM_USED_SIZE	4
#define MAX_TOUCH_STATUS	4
/* IPC_OMADM_CMD [0X14] */
#define MAX_OMADM_DISP_NAME_SIZE 64
#define MAX_OMADM_ID_SIZE 64
#define MAX_OMADM_ALERT_TYPE_SIZE 128 
#define MAX_OMADM_ALERT_FORMAT_SIZE 32
#define MAX_OMADM_ALERT_SRC_SIZE 32
#define MAX_OMADM_ALERT_CORREL_SIZE 32
#define MAX_OMADM_ALERT_MARK_SIZE 32
#define MAX_OMADM_ALERT_DATA_SIZE 32
#define MAX_OMADM_DATA_SIZE 1024

#define MAX_WIFI_MACADDRESS	6//cyj_dc24


/* SMS Input / Read Data   Pineone hgwoo*/
#define MAX_READ_DATA	256   

/*********************************************************************************/


/*********************************************************************************

                                            Sub Command of IPC_PWR_CMD[0x01]

**********************************************************************************/
typedef enum{
    IPC_PWR_PHONE_PWR_UP=0x01,      /* 0x01 : Phone Power Up Message */
    IPC_PWR_PHONE_PWR_OFF,             /* 0x02 : Phone Power Off Message */
    IPC_PWR_PHONE_RESET,                  /* 0x03 : Phone Reset Message */
    IPC_PWR_BATT_STATUS,                  /* 0x04 : Battery Status Message */
    IPC_PWR_BATT_TYPE,                      /* 0x05 : Battery Type Message */  // JBG 2005.04.03
    IPC_PWR_BATT_COMP,                        /* 0x06 : Battery compensation value of PDA*/
    IPC_PWR_PHONE_STATE,		    /*0x07 : Phone State Message*/    
    IPC_PWR_MAX
} ipc_pwr_sub_cmd_type;

/*********************************************************************************/

/*=================================================================

   SUB_CMD : IPC_PWR_PHONE_PWR_UP      0x01               Phone Power Up Message
   
=================================================================*/
/*    IPC_CMD_EXEC                    0x01     */
/*    IPC_CMD_GET                      0x02     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_CFRM                    0x04     */
/*    IPC_CMD_EVENT                   0x05     */
/*    IPC_CMD_INDI                      0x01     */
/*    IPC_CMD_RESP                     0x02     */
//none

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_NOTI                     0x03     

 DESCRIPTION :
   Phone Power Up Notification
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-----------------------------------------------------------------------------------*/



/*==================================================================

   SUB_CMD : IPC_PWR_PHONE_PWR_OFF     0x02                Phone power Off Message
   
==================================================================*/

/*    IPC_CMD_GET                       0x02     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_CFRM                     0x04     */
/*    IPC_CMD_EVENT                   0x05     */
/*    IPC_CMD_INDI                      0x01     */
/*    IPC_CMD_RESP                     0x02     */
//none



/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_EXEC                     0x01     

 DESCRIPTION :
   Execute Phone power Off 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD)1) | CMD_TYPE(1) |
-----------------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_NOTI                      0x03 
 
 DESCRIPTION :
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |REASON(1) |
-----------------------------------------------------------------------------------*/


/* REASON field*/
typedef enum {
  IPC_PWROFF_RSN_BATT,         /* 0x00 : Power-Off Battery Level */
  IPC_PWROFF_RSN_MAX           /* 0x01 : Max */
} ipc_pwr_off_reason_e_type;


/*===================================================================*/


/*=================================================================

   SUB_CMD : IPC_PWR_PHONE_RESET    0x03               Phone Reset Message
   
=================================================================*/

/*    IPC_CMD_EXEC                     0x01     */
/*    IPC_CMD_GET                       0x02     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_RESP                     0x02     */
//none



/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_CFRM               0x04   

 DESCRIPTION :
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-----------------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_EVENT               0x05   

 DESCRIPTION :
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-----------------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_INDI                      0x01 

  DESCRIPTION :
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | RESET_MODE(1) | REASON(1) |
-----------------------------------------------------------------------------------*/

/* RESET_MODE field */
typedef enum {
  IPC_PWR_RESET_PHONE_ONLY_REQ =0x01,
  IPC_PWR_RESET_BOTH_REQ,
  IPC_PWR_RESET_MAX
} ipc_pwr_reset_mode_e_type;

/* REASON field*/
/* not defined yet */


/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_NOTI                      0x03 

  DESCRIPTION :
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | RESET_MODE(1) | REASON(1) |
-----------------------------------------------------------------------------------*/

/* RESET_MODE field */
/* see ipc_pwr_reset_mode_e_type */

/* REASON field*/
typedef enum{
	IPC_RESET_REASON_LOW_BATTERY,
	IPC_RESET_REASON_NO_SIM
}ipc_pwr_reset_reason_e_type;



/*=================================================================

   SUB_CMD : IPC_PWR_BATT_STATUS    0x04               Battery Status Message
   
=================================================================*/

/*    IPC_CMD_EXEC                     0x01     */
/*    IPC_CMD_GET                       0x02     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_CFRM                    0x04     */
/*    IPC_CMD_EVENT                   0x05     */
/*    IPC_CMD_INDI                     0x01     */
/*    IPC_CMD_RESP                     0x02     */
//none

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_NOTI                      0x03 

  DESCRIPTION :
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | BATTERY_STATUS(1) |
-----------------------------------------------------------------------------------*/

/* BATTERY_STATUS field */
typedef enum {
  IPC_PWR_BATT_STATUS_PWR_OFF = 0x01,    /* Power-Off Battery Level */
  IPC_PWR_BATT_STATUS_CRITICAL_LOW,      /*Critical low battery Level*/
  IPC_PWR_BATT_STATUS_LOW,                       /* Low Battery Level */
  IPC_PWR_BATT_STATUS_NORMAL,                 /* Normal Battery Level */
  IPC_PWR_BATT_STATUS_NORMAL_BAR3,
  IPC_PWR_BATT_STATUS_NORMAL_BAR4,
  IPC_PWR_BATT_STATUS_NORMAL_BAR5,
  IPC_PWR_BATT_STATUS_MAX
} ipc_pwr_battery_status_e_type;


// JBG 2005.04.03
/*=================================================================

   SUB_CMD : IPC_PWR_BATT_TYPE    0x05               Battery Type Message
   
=================================================================*/

/*    IPC_CMD_EXEC                     0x01     */
/*    IPC_CMD_GET                       0x02     */
/*    IPC_CMD_CFRM                    0x04     */
/*    IPC_CMD_EVENT                   0x05     */
/*    IPC_CMD_INDI                     0x01     */
/*    IPC_CMD_RESP                     0x02     */
/*    IPC_CMD_NOTI                     0x03     */
//none

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_SET                      0x03

  DESCRIPTION :
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | BATTERY_TYPE(1) |
-----------------------------------------------------------------------------------*/

/* BATTERY_STATUS field */
typedef enum {
  IPC_PWR_BATT_TYPE_1 = 0x01,    /* Battery Type 1 */
  IPC_PWR_BATT_TYPE_2,               /* Battery Type 2 */
  IPC_PWR_BATT_TYPE_3,               /* Battery Type 3 */
  IPC_PWR_BATT_TYPE_4,               /* Battery Type 4 */
  IPC_PWR_BATT_TYPE_5,               /* Battery Type 5 */
  IPC_PWR_BATT_TYPE_6,               /* Battery Type 6 */
  IPC_PWR_BATT_TYPE_7,               /* Battery Type 7 */
  IPC_PWR_BATT_TYPE_8,               /* Battery Type 8 */
  IPC_PWR_BATT_TYPE_9,               /* Battery Type 9 */
  IPC_PWR_BATT_TYPE_A,               /* Battery Type A */
  IPC_PWR_BATT_TYPE_B,               /* Battery Type B */
  IPC_PWR_BATT_TYPE_C,               /* Battery Type C */
  IPC_PWR_BATT_TYPE_D,               /* Battery Type D */
  IPC_PWR_BATT_TYPE_E,               /* Battery Type E */
  IPC_PWR_BATT_TYPE_F,               /* Battery Type F */
  IPC_PWR_BATT_TYPE_MAX
} ipc_pwr_battery_type_e_type;

// CKA 2006.02.14
/*=================================================================

   SUB_CMD : IPC_PWR_BATT_COMP    0x06               Battery compensation value of PDA
   
=================================================================*/

/*    IPC_CMD_GET                      0x02     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_CFRM                    0x04     */
/*    IPC_CMD_EVENT                   0x05     */
/*    IPC_CMD_INDI                     0x01     */
/*    IPC_CMD_RESP                     0x02     */
/*    IPC_CMD_NOTI                     0x03     */



/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_SET                      0x03

  DESCRIPTION :
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | COMP_VALUE(1) |
-----------------------------------------------------------------------------------*/


/*==================================================================

   SUB_CMD : IPC_PWR_PHONE_STATE     0x07               Phone state Message

==================================================================*/
   
/*    IPC_CMD_GET                       0x02     */
/*    IPC_CMD_SET                     0x03     */
/*    IPC_CMD_CFRM                     0x04     */
/*    IPC_CMD_EVENT                   0x05     */
/*    IPC_CMD_INDI                      0x01     */
/*    IPC_CMD_RESP                     0x02     */

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_SET                     0x03     

 DESCRIPTION :
   Change Phone  State
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |STATE(1) |
-----------------------------------------------------------------------------------*/
/* STATUS field */
typedef enum {
  IPC_PWR_STATE_OFFLINE = 0x01,  /* Offline Mode */ 
  IPC_PWR_STATE_FTM,  /* factory test mode */
  IPC_PWR_STATE_ONLINE,  /* Online Mode */
  IPC_PWR_STATE_LPM,  /* Low Power Mode */
  IPC_PWR_STATE_NORMAL, /* Normal Mode */
  IPC_PWR_STATE_DEEP_SLEEP, /* Deep Sleep Mode */
  IPC_PWR_PHONE_MAX
} ipc_pwr_phone_state_e_type;



/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_NOTI                      0x03 

 DESCRIPTION :
   success? or fail?
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | STATE(1) |
-----------------------------------------------------------------------------------*/
/* STATE(1) */


/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_GET                     0x02     

 DESCRIPTION :
   Get LPM Status 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_RESP                      0x02 

 DESCRIPTION :
   ONLINE_OFF(Normal mode) or ONLINE_ON(LPM mode)
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |STATE(1) |
-----------------------------------------------------------------------------------*/

/*=================================================================*/



/*********************************************************************************

                                            Sub Command of IPC_CALL_CMD [0x02]

*********************************************************************************/
typedef enum{
  IPC_CALL_OUTGOING=0x01,           /* 0x01 : Call Outgoing Message */
  IPC_CALL_INCOMING,                /* 0x02 : Call Incoming Message */
  IPC_CALL_RELEASE,                 /* 0x03 : Call Release Message */
  IPC_CALL_ANSWER,                  /* 0x04 : Call Answer Message */
  IPC_CALL_STATUS,                  /* 0x05 : Current Call Status Message */
  IPC_CALL_LIST,                    /* 0x06 : Current Call List Message */
  IPC_CALL_BURST_DTMF,              /* 0x07 : Burst DTMF Message */
  IPC_CALL_CONT_DTMF,               /* 0x08 : Continuous DTMF Message */
  IPC_CALL_WAITING,                 /* 0x09 : Call Waiting Message */
  IPC_CALL_LINE_ID,                 /* 0x0A : Call Line id Message */
  IPC_CALL_SIGNAL,                  /* 0x0B : CDMA Signaling Information*/
  IPC_CALL_VOICE_PRIVACY,           /* 0x0C : CDMA Voice Privacy */
  IPC_CALL_CALL_TIME_COUNT,         /* 0x0D : Call Time & count */
  IPC_CALL_OTA_PROGRESS,            /* 0x0E : CDMA OTA Progress Indication */
  IPC_CALL_DIAG_OUTGOING,           /* 0x0F : Diagnostic Outgoing*/
  IPC_CALL_E911_CB_MODE,            /* 0x10 : CDMA E911 Callback Mode */
  IPC_CALL_FLASH_INFO,              /* 0x11 : CDMA Send Flash */
  IPC_CALL_MAX
} ipc_call_sub_cmd_type;

/*********************************************************************************/

/*=================================================================

   SUB_CMD : IPC_CALL_OUTGOING     0x01                 Call Outgoing Message
   
=================================================================*/

/*    IPC_CMD_GET                       0x02     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_CFRM               0x04     */
/*    IPC_CMD_EVENT                   0x05     */
/*    IPC_CMD_INDI                      0x01     */
/*    IPC_CMD_RESP                     0x02     */
/*    IPC_CMD_NOTI                     0x03     */
//none



/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_EXEC                     0x01 

 DESCRIPTION :
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |CALL_TYPE(2) | CLIR_STATUS(1) |
-------------------------------------------------------------------------------------
| NUMBER_LEN(1) | NUMBER_TYPE(1) | NUMBER(x) | CUG_CALL(1) | CUG_INDEX(1) | CUG_INFO(1)
-----------------------------------------------------------------------------------
| TTY_STATUS(1) |
-----------------------------------------------------------------------------------*/

/* CALL_TYPE  field */
typedef enum {
  IPC_CALL_TYPE_NONE,                             /* None */
  IPC_CALL_TYPE_VOICE = 0x0100,              /* Default Voice Call Type */
  IPC_CALL_TYPE_SO_VOICE,                      /* Voice Call(Wildcard SO) */
  IPC_CALL_TYPE_SO_VOICE_EVRC,             /* Voice EVRC(Testcall Only) */
  IPC_CALL_TYPE_SO_VOICE_13K,               /* Voice 13K(Testcall Only) */
  IPC_CALL_TYPE_SO_VOICE_EVRC_B,            /* Voice EVRC-B(Testcall Only) */
                                                                 /* Reserved Voice S0(0x0104 ~ 0x010f) */
  IPC_CALL_TYPE_DATA = 0x0200,                /* Default Data Call Type */
  IPC_CALL_TYPE_SO_ASYNC_8K,                /* Asynchronous Data Service (IS-99) */
  IPC_CALL_TYPE_SO_ASYNC_13K,              /* Asynchronous Data Service (IS-99) over Rate set 2 */
  IPC_CALL_TYPE_SO_FAX_8K,                    /* Group 3 FAX Service (IS-99) */
  IPC_CALL_TYPE_SO_FAX_13K,                  /* Group 3 FAX Service (IS-99) over Rate set 2 */
  IPC_CALL_TYPE_SO_PACKET_8K,               /* Internet Standard PPP Packet Data Service (IS-657) */
  IPC_CALL_TYPE_SO_PACKET_13K,             /* Asynchronous Data Service (IS-99) over Rate set 2 */
  IPC_CALL_TYPE_SO_PACKET_MDR_8K,       /* Medium Data Rate over Rate set 1(IS-707A) */
  IPC_CALL_TYPE_SO_PACKET_MDR_13K,     /* Medium Data Rate over Rate set 2(IS-707A) */
  IPC_CALL_TYPE_SO_PACKET_IS2000,        /* CDMA2000 pkt service option */
  IPC_CALL_TYPE_SO_PACKET_HDR,            /* HDR pkt SO  */ 
                                                                /* Reserved Data Service SO(0x020B ~ 0x02ff) */
  IPC_CALL_TYPE_VIDEO = 0x0300,              /* Default Video Call Type */
  IPC_CALL_TYPE_SO_SMS = 0x0400,           /* Default Short Message Service */
                                                                /* Reserved SMS Service SO(0x0x0401 ~ 0x04ff) */
  IPC_CALL_TYPE_SO_LOOPBACK = 0x0500, /* Default Loopback Call */ 
  IPC_CALL_TYPE_SO_LOOPBACK_8K,         /* Loopback Call 8K */
  IPC_CALL_TYPE_SO_LOOPBACK_13K,       /* Loopback Call 13K */
  IPC_CALL_TYPE_SO_LOOPBACK_55,         /* Loopback Service Option 55 */
  IPC_CALL_TYPE_SO_SIMPLE_TDSO,          /* Service option for Simple TDSO(Test Data SO) */
  IPC_CALL_TYPE_SO_FULL_TDSO,              /* Service option for Full TDSO(Test Data SO) */
                                                                /* Reserved Loopback Call Service Option(0x0506 ~ 0x05ff) */
  IPC_CALL_TYPE_SO_MARKOV = 0x0600,     /* Default MarkOv Call*/
  IPC_CALL_TYPE_SO_MARKOV_8K,             /* MarkOv Call 8K  */
  IPC_CALL_TYPE_SO_MARKOV_13K,           /* MarkOv Call 13K */
  IPC_CALL_TYPE_SO_MARKOV_54,             /* MarkOv Service Option 54 */
 
  IPC_CALL_TYPE_EMERGENCY = 0x0700,      /* Emergency call*/
                                                                /* Reserved MarkOv Call Service Option(0x0701 ~ 0x06ff) */
  IPC_CALL_TYPE_MAX                   
} ipc_call_type_e_type; 

/* CLIR_STATUS  field */
/* override the CLIR supplementary service subscription default value for this call */
typedef enum {
  IPC_CALL_CLIR_STATUS_NOT_CHANGED,     /* 0x00 : previous CLI presentation */
  IPC_CALL_CLIR_STATUS_INVOCATION,        /* 0x01 : restrict CLI presentation */
  IPC_CALL_CLIR_STATUS_SUPPRESSION,      /* 0x02 : allow CLI presentation */
  IPC_CALL_CLIR_STATUS_MAX
} ipc_clir_status_e_type; 

/* NUMBER_TYPE  field */
/* NUMBER_TYPE = ( (NUM_TYPE << 4 ) | NUM_PLAN )
*/
typedef enum{
  IPC_NUM_TYPE_UNKNOWN,                 /* 0x00 : unknown */
  IPC_NUM_TYPE_INTERNATIONAL,       /* 0x01 : international number */
  IPC_NUM_TYPE_NATIONAL,                 /* 0x02 : national number */
  IPC_NUM_TYPE_NETWORK,                  /* 0x03 : network specific number */
  IPC_NUM_TYPE_DEDICATE,                 /* 0x04 : dedicated access, short code */
  IPC_NUM_TYPE_MAX
} ipc_num_type_e_type;

typedef enum {
  IPC_NUM_PLAN_UNKNOWN = 0x00,                /* 0x00 : unknown */
  IPC_NUM_PLAN_ISDN = 0x01,                        /* 0x01 : ISDN numbering plan */
  IPC_NUM_PLAN_DATA = 0x03,                       /* 0x03 : DATA numbering plan */
  IPC_NUM_PLAN_TELEX = 0x04,                      /* 0x04 : */
  IPC_NUM_PLAN_NATIONAL = 0x08,               /* 0x08 : */
  IPC_NUM_PLAN_PRIVATE = 0x09,                  /* 0x09 : */
  IPC_NUM_PLAN_RSVD_CTS = 0x0B,               /* 0x0B : */
  IPC_NUM_PLAN_RSVD_EXT = 0x0F,               /* 0x0F : */
  IPC_NUM_PLAN_MAX
} ipc_num_plan_e_type;

/* CUG_INFO */
typedef enum {
    IPC_CUG_INFO_NONE            =0x00,   	/* no information */
    IPC_CUG_INFO_SUPRESS_OA	    =0x01,		/* suppress OA(Outside Access) */
    IPC_CUG_INFO_SUPRESS_PREF	=0x02,		/* suppress preferential CUG */
    IPC_CUG_INFO_SUPRESS_0A_PREF =0x03,		/* suppress OA and preferential CUG */
    IPC_CUG_INFO_MAX
} ipc_cug_info_e_type;

/* TTY_STATUS : refere to the ipc_cfg_tty_e_type */


/*=================================================================*/


/*=================================================================

   SUB_CMD : IPC_CALL_INCOMING     0x02                Call Incoming Message
   
==================================================================*/
/*    IPC_CMD_EXEC                     0x01     */
/*    IPC_CMD_GET                       0x02     */
/*    IPC_CMD_SET                        0x03     */
/*    IPC_CMD_CFRM               0x04     */
/*    IPC_CMD_EVENT                   0x05     */
/*    IPC_CMD_INDI                      0x01     */
/*    IPC_CMD_RESP                     0x02     */
//none



/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_NOTI                      0x03

 DESCRIPTION :
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |CALL_TYPE(2) | CALL_ID(1) | LINE_ID(1)
-----------------------------------------------------------------------------------*/

/* CALL_TYPE field */
/* refer to the "ipc_call_type_e_type" */

/* CALL_ID  field */
/* Unique Identification Number to the each call
** the range is 0 - 7
*/




/*=================================================================

   SUB_CMD(1) : IPC_CALL_RELEASE     0x03                      Call Release Message
   
==================================================================*/

/*    IPC_CMD_GET                       0x02     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_CFRM               0x04     */
/*    IPC_CMD_EVENT                   0x05     */
/*    IPC_CMD_INDI                      0x01     */
/*    IPC_CMD_RESP                     0x02     */
/*    IPC_CMD_NOTI                     0x03     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_EXEC                     0x01 

 DESCRIPTION :
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-----------------------------------------------------------------------------------*/



/*=================================================================*/



/*==================================================================

   CMD_TYPE(1) : IPC_CALL_ANSWER     0x04                       Call Answer Message
   
==================================================================*/

/*    IPC_CMD_GET                      0x02     */
/*    IPC_CMD_SET                      0x03     */
/*    IPC_CMD_CFRM                    0x04     */
/*    IPC_CMD_EVENT                  0x05     */
/*    IPC_CMD_INDI                     0x01     */
/*    IPC_CMD_RESP                    0x02     */
/*    IPC_CMD_NOTI                    0x03     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_EXEC                     0x01  

 DESCRIPTION :
          Call Asnwer Execute
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-----------------------------------------------------------------------------------*/



/*=================================================================*/




/*=================================================================

   SUB_CMD(1) : IPC_CALL_STATUS     0x05             Call Status Message
   
==================================================================*/

/*    IPC_CMD_EXEC                     0x01     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_CFRM                     0x04     */
/*    IPC_CMD_EVENT                   0x05     */
/*    IPC_CMD_INDI                      0x01     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                       0x02  

 DESCRIPTION :
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-----------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                       0x02  

 DESCRIPTION :
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |CALL_TYPE(2) | CALL_ID(1) |
-------------------------------------------------------------------------------------
 | CALL_STATE(1) | REASON(1) |END_CAUSE(1) |
-------------------------------------------------------------------------------------
*/

/* CALL_TYPE field */
/* refer to ipc_call_type_e_type */

/* CALL_ID  field */
/* Call Identification number 0x00 - 0x06 */


/* CALL_STATE  field */
/* Call Processing Status ( or call state ) */
typedef enum{
  IPC_CALL_STATE_NONE,                  /* 0x00 : Idle State ( Non-call state ) */
  IPC_CALL_STATE_OUTGOING,          /* 0x01 : Originating Attempt state */
  IPC_CALL_STATE_INCOMING,           /* 0x02 : Incoming alert state */
  IPC_CALL_STATE_CONNECTED,        /* 0x03 : connected state */
  IPC_CALL_STATE_RELEASED,           /* 0x04 : Release State */
  IPC_CALL_STATE_CONNECTING,		/*0x05 : connecting state */
  IPC_CALL_STATE_MAX
} ipc_call_state_e_type;


/* REASON  field */
/* Call State Reason */
typedef enum{
  IPC_CALL_REASON_NORMAL,                          /* 0x00 : Normal Process ( no problem ) */
  IPC_CALL_REASON_REL_BY_USER,                 /* 0x01 : Call Released by User */
  IPC_CALL_REASON_REL_BY_NET,                   /* 0x02 : Call Released by Network */
  IPC_CALL_REASON_REL_NET_BUSY,               /* 0x03 : Call Released because of Network busy */
  IPC_CALL_REASON_REL_SIGNAL_LOSS,         /* 0x04 : Call Released because of Signal Loss ( e.g Fading ) */
  IPC_CALL_REASON_ORIG_FAIL_SIGNAL_LOSS,       /* 0x05 : Originating Failed because of Signal Loss */
  IPC_CALL_REASON_ORIG_REJECTED_NO_FUNDS,   /* 0x06 : Originating Rejected because of no Funds */
  IPC_CALL_REASON_ORIG_SILENT_REDIAL,             /* 0x07 : Originating is being Redialed ( or Silent Redial ) */

//new_add
  IPC_CALL_REASON_NO_SVC,                                   /* 0x08 : Entering the No Service area */
  IPC_CALL_REASON_OTHERS,                                    /* 0x09 : Released(or Originating failed ) by Others */
  IPC_CALL_REASON_BARRED,                                   /* 0x0A : Call is barred */
  IPC_CALL_REASON_FADING, 								  
  IPC_CALL_REASON_RELEASE_BY_REORDER,
  IPC_CALL_REASON_RELEASE_BY_INTERCEPT,
  IPC_CALL_REASON_SILENT_ZONE_RETRY,
  IPC_CALL_REASON_OTA_CALL_FAIL,
  IPC_CALL_REASON_PHONE_OFFLINE,
  IPC_CALL_REASON_PHONE_IS_CDMA_LOCKED,
  IPC_CALL_REASON_FLASH_IS_IN_PROGRESS_ERR,
  IPC_CALL_REASON_E911_MODE_ERR,
  IPC_CALL_REASON_SVC_TYPE_NOT_ALLOWED_IN_CALL  ,
  IPC_CALL_REASON_PHONE_IS_IN_USE,
  IPC_CALL_REASON_NOT_ALLOWED_CMD_IN_CURRENT_CALL_STATE,
  IPC_CALL_REASON_NO_AVAIL_BUFFER,
  IPC_CALL_REASON_OTHER_INTERNAL_ERR	  ,  
  IPC_CALL_REASON_MAX
} ipc_call_state_reason_e_type; 


/* End cause field */
/* Call state end cause */
typedef enum{
IPC_CALL_END_NO_CAUSE,
/*
 * These definitions are taken from GSM 04.08 Table 10.86
 */
IPC_CC_CAUSE_UNASSIGNED_NUMBER,     //070314_hyerin.CHO
IPC_CC_CAUSE_NO_ROUTE_TO_DEST,
IPC_CC_CAUSE_CHANNEL_UNACCEPTABLE,
IPC_CC_CAUSE_OPERATOR_DETERMINED_BARRING,
IPC_CC_CAUSE_NORMAL_CALL_CLEARING,
IPC_CC_CAUSE_USER_BUSY,
IPC_CC_CAUSE_NO_USER_RESPONDING,
IPC_CC_CAUSE_USER_ALERTING_NO_ANSWER,
IPC_CC_CAUSE_CALL_REJECTED,
IPC_CC_CAUSE_NUMBER_CHANGED,
IPC_CC_CAUSE_NON_SELECTED_USER_CLEARING,
IPC_CC_CAUSE_DESTINATION_OUT_OF_ORDER,
IPC_CC_CAUSE_INVALID_NUMBER_FORMAT,
IPC_CC_CAUSE_FACILITY_REJECTED,
IPC_CC_CAUSE_RESPONSE_TO_STATUS_ENQUIRY,
IPC_CC_CAUSE_NORMAL_UNSPECIFIED,
IPC_CC_CAUSE_NO_CIRCUIT_CHANNEL_AVAILABLE,
IPC_CC_CAUSE_NETWORK_OUT_OF_ORDER,
IPC_CC_CAUSE_TEMPORARY_FAILURE,
IPC_CC_CAUSE_SWITCHING_EQUIPMENT_CONGESTION,
IPC_CC_CAUSE_ACCESS_INFORMATION_DISCARDED,
IPC_CC_CAUSE_REQUESTED_CIRCUIT_CHANNEL_NOT_AVAILABLE,
IPC_CC_CAUSE_RESOURCES_UNAVAILABLE_UNSPECIFIED,
IPC_CC_CAUSE_QUALITY_OF_SERVICE_UNAVAILABLE,
IPC_CC_CAUSE_REQUESTED_FACILITY_NOT_SUBSCRIBED,
IPC_CC_CAUSE_INCOMING_CALL_BARRED_WITHIN_CUG,
IPC_CC_CAUSE_BEARER_CAPABILITY_NOT_AUTHORISED,
IPC_CC_CAUSE_BEARER_CAPABILITY_NOT_PRESENTLY_AVAILABLE,
IPC_CC_CAUSE_SERVICE_OR_OPTION_NOT_AVAILABLE,
IPC_CC_CAUSE_BEARER_SERVICE_NOT_IMPLEMENTED,
IPC_CC_CAUSE_ACM_GEQ_ACMMAX,
IPC_CC_CAUSE_REQUESTED_FACILITY_NOT_IMPLEMENTED,
IPC_CC_CAUSE_ONLY_RESTRICTED_DIGITAL_INFO_BC_AVAILABLE,
IPC_CC_CAUSE_SERVICE_OR_OPTION_NOT_IMPLEMENTED,
IPC_CC_CAUSE_INVALID_TRANSACTION_ID_VALUE,
IPC_CC_CAUSE_USER_NOT_MEMBER_OF_CUG,
IPC_CC_CAUSE_INCOMPATIBLE_DESTINATION,
IPC_CC_CAUSE_INVALID_TRANSIT_NETWORK_SELECTION,
IPC_CC_CAUSE_SEMANTICALLY_INCORRECT_MESSAGE,
IPC_CC_CAUSE_INVALID_MANDATORY_INFORMATION,
IPC_CC_CAUSE_MESSAGE_TYPE_NON_EXISTENT,
IPC_CC_CAUSE_MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROT_STATE,
IPC_CC_CAUSE_IE_NON_EXISTENT_OR_NOT_IMPLEMENTED,
IPC_CC_CAUSE_CONDITIONAL_IE_ERROR,
IPC_CC_CAUSE_MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE,
IPC_CC_CAUSE_RECOVERY_ON_TIMER_EXPIRY,
IPC_CC_CAUSE_PROTOCOL_ERROR_UNSPECIFIED,
IPC_CC_CAUSE_INTERWORKING_UNSPECIFIED,
IPC_CC_CAUSE_END = 128,

/* Reject causes*/
IPC_REJECT_CAUSE_IMSI_UNKNOWN_IN_HLR,
IPC_REJECT_CAUSE_ILLEGAL_MS,                                    
IPC_REJECT_CAUSE_IMSI_UNKNOWN_IN_VLR,                           
IPC_REJECT_CAUSE_IMEI_NOT_ACCEPTED,                             
IPC_REJECT_CAUSE_ILLEGAL_ME,                                    
IPC_REJECT_CAUSE_GPRS_SERVICES_NOT_ALLOWED,                     
IPC_REJECT_CAUSE_GPRS_SERVICES_AND_NON_GPRS_SERVICES_NOT_ALLOWED,
IPC_REJECT_CAUSE_MS_IDENTITY_CANNOT_BE_DERIVED_BY_THE_NETWORK,   
IPC_REJECT_CAUSE_IMPLICITLY_DETACHED,                           
IPC_REJECT_CAUSE_PLMN_NOT_ALLOWED,                              
IPC_REJECT_CAUSE_LA_NOT_ALLOWED,                                
IPC_REJECT_CAUSE_NATIONAL_ROAMING_NOT_ALLOWED,                  
IPC_REJECT_CAUSE_GPRS_SERVICES_NOT_ALLOWED_IN_THIS_PLMN,        
IPC_REJECT_CAUSE_NO_SUITABLE_CELLS_IN_LA,                       
IPC_REJECT_CAUSE_MSC_TEMPORARILY_NOT_REACHABLE,                 
IPC_REJECT_CAUSE_NETWORK_FAILURE ,                              
IPC_REJECT_CAUSE_MAC_FAILURE,                                   
IPC_REJECT_CAUSE_SYNCH_FAILURE,                                 
IPC_REJECT_CAUSE_CONGESTTION,                                   
IPC_REJECT_CAUSE_GSM_AUTH_UNACCEPTED,                           
IPC_REJECT_CAUSE_SERVICE_OPTION_NOT_SUPPORTED,                  
IPC_REJECT_CAUSE_REQ_SERV_OPT_NOT_SUBSCRIBED,                   
IPC_REJECT_CAUSE_SERVICE_OPT__OUT_OF_ORDER,                     
IPC_REJECT_CAUSE_CALL_CANNOT_BE_IDENTIFIED,                     
IPC_REJECT_CAUSE_NO_PDP_CONTEXT_ACTIVATED,                      
IPC_REJECT_CAUSE_RETRY_UPON_ENTRY_INTO_A_NEW_CELL_MIN_VALUE,    
IPC_REJECT_CAUSE_RETRY_UPON_ENTRY_INTO_A_NEW_CELL_MAX_VALUE,    
IPC_REJECT_CAUSE_SEMANTICALLY_INCORRECT_MSG,                    
IPC_REJECT_CAUSE_INVALID_MANDATORY_INFO,                        
IPC_REJECT_CAUSE_MESSAGE_TYPE_NON_EXISTANT,                     
IPC_REJECT_CAUSE_MESSAGE_TYPE_NOT_COMP_PRT_ST,                  
IPC_REJECT_CAUSE_IE_NON_EXISTANT,                               
IPC_REJECT_CAUSE_MSG_NOT_COMPATIBLE_PROTOCOL_STATE,             


/* Connection Management establishment rejection cause */
IPC_REJECT_CAUSE_REJ_UNSPECIFIED,                     

/* AS reject causes */
IPC_REJECT_CAUSE_AS_REJ_RR_REL_IND,                   
IPC_REJECT_CAUSE_AS_REJ_RR_RANDOM_ACCESS_FAILURE,     
IPC_REJECT_CAUSE_AS_REJ_RRC_REL_IND,                  
IPC_REJECT_CAUSE_AS_REJ_RRC_CLOSE_SESSION_IND,        
IPC_REJECT_CAUSE_AS_REJ_RRC_OPEN_SESSION_FAILURE,     
IPC_REJECT_CAUSE_AS_REJ_LOW_LEVEL_FAIL,               
IPC_REJECT_CAUSE_AS_REJ_LOW_LEVEL_FAIL_REDIAL_NOT_ALLOWED, 
IPC_REJECT_CAUSE_AS_REJ_LOW_LEVEL_IMMED_RETRY,        

/* MM reject causes */
IPC_REJECT_CAUSE_MM_REJ_INVALID_SIM,                  
IPC_REJECT_CAUSE_MM_REJ_NO_SERVICE,                   
IPC_REJECT_CAUSE_MM_REJ_TIMER_T3230_EXP,              
IPC_REJECT_CAUSE_MM_REJ_NO_CELL_AVAILABLE,            
IPC_REJECT_CAUSE_MM_REJ_WRONG_STATE,                  
IPC_REJECT_CAUSE_MM_REJ_ACCESS_CLASS_BLOCKED,         

/* Definitions for release ind causes between MM  and CNM*/
IPC_REJECT_CAUSE_ABORT_MSG_RECEIVED,                   
IPC_REJECT_CAUSE_OTHER_CAUSE,                         

/* CNM reject causes */
IPC_REJECT_CAUSE_CNM_REJ_TIMER_T303_EXP,              
IPC_REJECT_CAUSE_CNM_REJ_NO_RESOURCES,                
IPC_REJECT_CAUSE_CNM_MM_REL_PENDING,                  
IPC_REJECT_CAUSE_CNM_INVALID_USER_DATA, 
IPC_CALL_END_CAUSE_MAX = 255
}ipc_call_end_cause_e_type;

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                       0x03  

 DESCRIPTION :
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |CALL_TYPE(2) | CALL_ID(1) |
-------------------------------------------------------------------------------------
 | CALL_STATE(1) | REASON(1) |END_CAUSE(1) | MO_CALL_TIME(4) | MT_CALL_TIME(4)
-------------------------------------------------------------------------------------
*/

/* MO_CALL_TIME field */
/* Mobile Originate call time (second) */

/* MT_CALL_TIME  field */
/* Mobile Terminate call time (second) */


/*=================================================================

   SUB_CMD(1) : IPC_CALL_LIST     0x06                      Call List Message
   
==================================================================*/

/*    IPC_CMD_EXEC                     0x01     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_CFRM                     0x04     */
/*    IPC_CMD_EVENT                   0x05     */
/*    IPC_CMD_INDI                      0x01     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                       0x02     

 DESCRIPTION :
         Call List GET 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/




/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                       0x02     

 DESCRIPTION :
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |NUM_RECORD(1) | CALL_TYPE(2) |CALL_ID(1) | 
-------------------------------------------------------------------------------------
 | DIRECTION(1) | CALL_STATUS(1) | MPTY_STATUS(1) | NUMBER_LEN | 
-------------------------------------------------------------------------------------
 | NUMBER_TYPE(1) | NUMBER(X) |
-------------------------------------------------------------------------------------
*/


/* NUM_RECORD  field */
/* see MAX_GSM_CALL_LIST_NUM */

/* CALL_TYPE */
/* see ipc_call_type_e_type */

/* DIRECTION field */
typedef enum{
  IPC_CALL_DIR_MO = 0x01,     /* 0x01 : Mobile Originated Call ( MO Call ) */
  IPC_CALL_DIR_MT,     /* 0x02 : Mobile Terminated Call ( MT Call ) */
  IPC_CALL_DIR_MAX
} ipc_call_direction_e_type; 

/* CALL_STATUS field */
typedef enum{
  IPC_CALL_STATUS_NONE,              /* 0x00 : None Call status */
  IPC_CALL_STATUS_ACTIVE,           /* 0x01 : current call is active */
  IPC_CALL_STATUS_HELD,              /* 0x02 : current call is on hold */
  IPC_CALL_STATUS_DIALING,         /* 0x03 : current call is dialing(MO call) */
  IPC_CALL_STATUS_ALERT,             /* 0x04 : terminated party is ringing ( MO call ) */
  IPC_CALL_STATUS_INCOMING,      /* 0x05 : incoming call(MT call) */
  IPC_CALL_STATUS_WAITING,        /* 0x06 : Another incoming call ( call waiting ) is alerting to the user in conversation ( MT call ) */
  /* Others are reserved */
  IPC_CALL_STATUS_MAX
}  ipc_call_status_e_type; 


/* MPTY_STATUS field */
typedef enum{
  IPC_CALL_MPTY_SINGLE_CALL,    /* 0x00 : Single Call ( call is not multiparty call ) */
  IPC_CALL_MPTY_MPTY_CALL,       /* 0x01 : Multiparty Call ( call is one of the multiparty call ) */
  IPC_CALL_MPTY_MAX
} ipc_call_mpty_status_e_type; 


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                       0x03

 DESCRIPTION :
    Call List Notification 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |NUM_RECORD(1) | CALL_TYPE(1) |CALL_ID(1) | 
-------------------------------------------------------------------------------------
 | DIRECTION(1) | CALL_STATUS(1) | MPTY_STATUS(1) | NUMBER_LEN | 
-------------------------------------------------------------------------------------
 | NUMBER_TYPE(1) | NUMBER(X) |
-------------------------------------------------------------------------------------
*/



/*====================================================================*/



/*=================================================================

   SUB_CMD(1) : IPC_CALL_BURST_DTMF     0x07                  Burst DTMF Message
   
==================================================================*/

/*    IPC_CMD_GET                       0x02     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_CFRM                     0x04     */
/*    IPC_CMD_EVENT                   0x05     */
/*    IPC_CMD_INDI                      0x01     */
/*    IPC_CMD_RESP                     0x02     */
//none



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_EXEC                       0x01     

 DESCRIPTION :
     Send Burst DTMF EXECUTE
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |TONE_LENGTH(1) | DIGIT_NUM(1) | DTMF_DIGITS(X) |
-------------------------------------------------------------------------------------
*/

/* TONE_LENGTH  field */
/* Tone duration should be lesser than 350mil seconds. */
typedef enum{
  IPC_CALL_TONE_SHORT = 0x01,       /* 0x01 : Tone duration is SHORT */
  IPC_CALL_TONE_LONG,                    /* 0x02 : Tone duration is LONG */
  IPC_CALL_TONE_MAX
} ipc_call_dtmf_tone_length_e_type;

/* DIGIT_NUM  field */
/* The length of DTMF_DIGITS fields, maximum value is 32 (bytes). */

/* DTMF_DIGIT field */
/* The value of Burst DTMF. Maximum value is 32 bytes. */
/* see MAX_CALL_DIALED_DIGITS_NUM */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                       0x03     

 DESCRIPTION :
     Send Burst DTMF Notification
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/


/*=================================================================

   SUB_CMD(1) : IPC_CALL_CONT_DTMF     0x08                  Continuous DTMF Message
   
==================================================================*/

/*    IPC_CMD_EXEC                    0x01     */
/*    IPC_CMD_GET                      0x02     */
/*    IPC_CMD_CFRM                    0x04     */
/*    IPC_CMD_EVENT                   0x05     */
/*    IPC_CMD_INDI                      0x01     */
/*    IPC_CMD_RESP                     0x02     */
/*    IPC_CMD_NOTI                     0x03     */
//none



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                       0x03     

 DESCRIPTION :
     Send Continuous DTMF SET
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CONT_MODE(1) | DTMF_DIGITS(1) |
-------------------------------------------------------------------------------------
*/

/* CONT_MODE : continuous DTMF mode */
typedef enum {
  IPC_CONT_DTMF_START = 0x01,       /* 0x01 : Start Continuous DTMF */
  IPC_CONT_DTMF_STOP,                    /* 0x02 : Stop Continuous DTMF */
  IPC_CONT_DTMF_MAX
} ipc_call_cont_dtmf_mode_type;


/*=================================================================

   SUB_CMD(1) : IPC_CALL_WAITING     0x09                  Call Waiting Message
   
==================================================================*/

/*    IPC_CMD_EXEC                       0x01     */
/*    IPC_CMD_GET                       0x02     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_CFRM                     0x04     */
/*    IPC_CMD_EVENT                   0x05     */
/*    IPC_CMD_INDI                      0x01     */
/*    IPC_CMD_RESP                     0x02     */
//none



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                       0x03    

 DESCRIPTION :
     Call Waiting Notificaiton
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CALL_TYPE(2) | CALL_ID(1) | LINE_ID(1)
-------------------------------------------------------------------------------------
*/

/*=================================================================

   SUB_CMD(1) : IPC_CALL_LINE_ID    0x0A                  Call Line id Message
   
==================================================================*/

/*    IPC_CMD_EXEC                       0x01     */
/*    IPC_CMD_CFRM  	                 0x04     */
/*    IPC_CMD_EVENT	    	             0x05     */
/*    IPC_CMD_INDI          	         0x01     */
/*    IPC_CMD_NOTI                  	 0x03     */
//none

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET               0x02    

 DESCRIPTION :
     Call Line id GET
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET               0x03    

 DESCRIPTION :
     Call Line id SET
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | LINE_ID(1)
-------------------------------------------------------------------------------------
*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP               0x02    

 DESCRIPTION :
     Call Line id Response
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | LINE_ID(1)
-------------------------------------------------------------------------------------
*/





/*=================================================================

   SUB_CMD(1) : IPC_CALL_SIGNAL    0x0B                  CDMA Signal Information Message
   
==================================================================*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI               0x03

 DESCRIPTION :
     CDMA Signal Information Message.
     
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CALL_TYPE(2) | SIGNAL_TYPE(1) | 
-------------------------------------------------------------------------------------
 | ALERT_PITCH(1) | SIGNAL(1) |
-------------------------------------------------------------------------------------
*/

// CALL_TYPE


// SIGNAL_TYPE
typedef enum {
  IPC_CALL_SIGNAL_TONE = 0x00,
  IPC_CALL_SIGNAL_ISDN_ALERT,
  IPC_CALL_SIGNAL_IS54B_ALERT,
  IPC_CALL_SIGNAL_MAX
} ipc_call_signal_e_type;

// ALERT_PITCH
typedef enum {
  IPC_CALL_ALERT_PITCH_MIDIUM = 0x00,
  IPC_CALL_ALERT_PITCH_HIGH,
  IPC_CALL_ALERT_PITCH_LOW,
  IPC_CALL_ALERT_PITCH_MAX
} ipc_call_alert_pitch_e_type;

// SIGNAL : SIGNAL TYPE IPC_SIGNAL_TONE  
typedef enum {
  IPC_CALL_SIGNAL_TONE_DIAL = 0x00,
  IPC_CALL_SIGNAL_TONE_RINGBACK_TONE_ON,
  IPC_CALL_SIGNAL_TONE_INTERCEPT_TONE_ON,
  IPC_CALL_SIGNAL_TONE_ABBREV_TONE,
  IPC_CALL_SIGNAL_TONE_NETWORK_CONGESTION_TONE_ON,
  IPC_CALL_SIGNAL_TONE_ABBREV_NETWORK_CONGESTION,
  IPC_CALL_SIGNAL_TONE_BUSY_TONE_ON,
  IPC_CALL_SIGNAL_TONE_CFRM_TONE_ON,
  IPC_CALL_SIGNAL_TONE_ANSWER_TONE_ON,
  IPC_CALL_SIGNAL_TONE_CALL_WAITING_TONE_ON,
  IPC_CALL_SINGNAL_TONE_PIPE_TONE_ON,
  IPC_CALL_SIGNAL_TONE_OFF,
  IPC_CALL_SIGNAL_TONE_MAX
} ipc_call_signal_tone_e_type;

// SIGNAL : SIGNAL TYPE IPC_SIGNAL_ISDNALERT 
typedef enum {
  IPC_CALL_SIGNAL_ISDN_ALERT_NORMAL = 0x00,
  IPC_CALL_SIGNAL_ISDN_ALERT_INTER_GROUP,
  IPC_CALL_SIGNAL_ISDN_ALERT_SPECIAL_PRIORITY,
  IPC_CALL_SIGNAL_ISDN_ALERT_ISDN_RESERVED1,
  IPC_CALL_SIGNAL_ISDN_ALERT_PING_RING,
  IPC_CALL_SIGNAL_ISDN_ALERT_ISDN_RESERVED2,
  IPC_CALL_SIGNAL_ISDN_ALERT_ISDN_RESERVED3,
  IPC_CALL_SIGNAL_ISDN_ALERT_ISDN_RESERVED4,
  IPC_CALL_SIGNAL_ISDN_ALERT_MAX
} ipc_call_signal_isdn_alert_e_type;

// SIGNAL : SIGNAL TYPE IS-IPC_SIGNAL_IS54BALERT 
typedef enum {
  IPC_CALL_SIGNAL_IS54B_ALERT_NOTONE = 0x00,
  IPC_CALL_SIGNAL_IS54B_ALERT_LONG,
  IPC_CALL_SIGNAL_IS54B_ALERT_SHORT_SHORT,
  IPC_CALL_SIGNAL_IS54B_ALERT_SHORT_SHORT_LONG,
  IPC_CALL_SIGNAL_IS54B_ALERT_SHORT_LONG_SHORT,
  IPC_CALL_SIGNAL_IS54B_ALERT_SHORT_SHORT_SHORT_SHORT,
  IPC_CALL_SIGNAL_IS54B_ALERT_PBX_LONG,
  IPC_CALL_SIGNAL_IS54B_ALERT_PBX_SHORT_SHORT,
  IPC_CALL_SIGNAL_IS54B_ALERT_PBX_SHORT_SHORT_LONG,
  IPC_CALL_SIGNAL_IS54B_ALERT_PBX_SHORT_LONG_SHORT,
  IPC_CALL_SIGNAL_IS54B_ALERT_PBX_SHORT_SHORT_SHORT_SHORT,
  IPC_CALL_SIGNAL_IS54B_ALERT_MAX
} ipc_call_signal_is54b_alert_e_type;

/*=================================================================

   SUB_CMD(1) : IPC_CALL_VOICE_PRIVACY    0x0C           CDMA Voice Privacy Information Message
   
==================================================================*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET               0x02

 DESCRIPTION :
     GET CDMA Voice Privacy Information Message
     
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | REQ_TYPE(1) |
-------------------------------------------------------------------------------------
*/

// REQ_TYPE
typedef enum {
   IPC_CALL_VP_REQ_TYPE_MOBILE = 0x01,
   IPC_CALL_VP_REQ_TYPE_NETWORK,
   IPC_CALL_VP_REQ_TYPE_SERVING_STATUS,
   IPC_CALL_VP_REQ_TYPE_MAX
} ipc_call_vp_reg_e_type;



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET               0x03

 DESCRIPTION :
     SET CDMA Voice Privacy Information Message
     
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | REQ_TYPE(1) | VP_MODE |
-------------------------------------------------------------------------------------
*/

// REQ_TYPE : refer to GET TYPE

// VP_MODE 
typedef enum {
   IPC_CALL_VP_MODE_STANDARD = 0x00,
   IPC_CALL_VP_MODE_ENHANCED,
   IPC_CALL_VP_MODE_MAX
} ipc_call_vp_mode_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP               0x02

 DESCRIPTION :
     GET CDMA Voice Privacy Information Message
     
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | REQ_TYPE(1) | VP_MODE |
-------------------------------------------------------------------------------------
*/

// REQ_TYPE : refer to SET_TYPE

// VP_MODE : refer to SET_TYPE



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI               0x03

 DESCRIPTION :
     Notification CDMA Voice Privacy Information Message
     
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | REQ_TYPE(1) | VP_MODE |
-------------------------------------------------------------------------------------
*/

// REQ_TYPE : refer to SET_TYPE

// VP_MODE : refer to SET_TYPE



/*=================================================================

   SUB_CMD(1) : IPC_CALL_CALL_TIME_COUNT    0x0D           Call Time & Count Message
   
==================================================================*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET               0x02

 DESCRIPTION :
     GET Call Time Message
     
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CALL_TIME_COUNT_TYPE(2) |
-------------------------------------------------------------------------------------
*/

// CALL_TIME_COUNT_TYPE(2)
#define IPC_CALL_TIME_COUNT_TYPE_TOTAL_CALL_CNT  0x01  /* Total Call Count Mask */
#define IPC_CALL_TIME_COUNT_TYPE_TOTAL_CALL_TIME  0x02  /* Total Call Time Mask */
#define IPC_CALL_TIME_COUNT_TYPE_LAST_CALL_TIME  0x04  /* Last Call Time Mask */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP               0x02

 DESCRIPTION :
     Response Call Time & count Message
     
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CALL_TIME_COUNT_TYPE(2) | 
-------------------------------------------------------------------------------------
 | TOTAL_CALL_CNT(4) | TX_CALL_CNT(4) | RX_CALL_CNT(4) | 
-------------------------------------------------------------------------------------
 | TOTAL_CALL_TIME(4) | TX_CALL_TIME(4) | RX_CALL_TIME(4) | LAST_CALL_TIME(4)
-------------------------------------------------------------------------------------
*/

// TOTAL_CALL_CNT(4) : total call count
// TX_CALL_CNT(4) : total outgoing call count
// RX_CALL_CNT(4) : total incoming call count
// TOTAL_CALL_TIME(4) : total call time
// TX_CALL_TIME(4) : total outgoing call time
// RX_CALL_TIME(4) : total incoming call time
// LAST_CALL_TIME(4) : last call time


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI               0x03

 DESCRIPTION :
     Notification Call Time & count Message
     
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CALL_TIME_COUNT_TYPE(2) | 
-------------------------------------------------------------------------------------
 | TOTAL_CALL_CNT(4) | TX_CALL_CNT(4) | RX_CALL_CNT(4) | 
-------------------------------------------------------------------------------------
 | TOTAL_CALL_TIME(4) | TX_CALL_TIME(4) | RX_CALL_TIME(4) | LAST_CALL_TIME(4)
-------------------------------------------------------------------------------------
*/



/*=================================================================

   SUB_CMD(1) : IPC_CALL_OTA_PROGRESS    0x0E           CDMA OTA Progress Notification
   
==================================================================*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI               0x03


 DESCRIPTION :
      CDMA OTA Progress Notification
     
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | OTA_TYPE(1) | OTA_STATUS(1) |
-------------------------------------------------------------------------------------
*/

// OTA_TYPE(1)
#define IPC_CALL_OTATYPE_OTASP  0x01  /* OTASP : mobile initiated */
#define IPC_CALL_OTATYPE_OTAPA 0x02  /* OTAPA : network initiated */


// OTA_STATUS(1)
// In Case Of OTA_TYPE==OTASP : OTASP STATUS
#define IPC_CALL_OTASP_OK_SPL_UNLOCKED  0x01
#define IPC_CALL_OTASP_OK_AKEY_EXCESS  0x02
#define IPC_CALL_OTASP_OK_SSD_UPDATE  0x03
#define IPC_CALL_OTASP_OK_NAM_DWNLD  0x04
#define IPC_CALL_OTASP_OK_MDN_DWNLD  0x05
#define IPC_CALL_OTASP_OK_IMSI_DWNLD  0x06
#define IPC_CALL_OTASP_OK_PRL_DWNLD  0x07
#define IPC_CALL_OTASP_OK_COMMIT  0x08
#define IPC_CALL_OTASP_OK_PROGRAMMING  0x09
#define IPC_CALL_OTASP_SUCCESS  0x0A
#define IPC_CALL_OTASP_UNSUCCESS  0x0B
#define IPC_CALL_OTASP_OK_OTAPA_VERIFY  0x0C
#define IPC_CALL_OTASP_PROGRESS  0x0D
#define IPC_CALL_OTASP_FAILURES_EXCESS_SPC  0x0E
#define IPC_CALL_OTASP_LOCK_CODE_PW_SET  0x0F


//#define IPC_OTA_MSG_SKTOTA_NAM_SELECT				0x11
//#define IPC_OTA_MSG_SKTOTA_AUTH_CONFIG				0x12
//#define IPC_OTA_MSG_SKTOTA_ALWAYSON_CONFIG			0x13
//#define IPC_OTA_MSG_SKTOTA_PRLID_CONFIG				0x14
//#define IPC_OTA_MSG_SKTOTA_SCI_CONFIG				0x15
//#define IPC_OTA_MSG_SKTOTA_RUNAPP_CONFIG				0x16
//#define IPC_OTA_MSG_SKTOTA_SDMB_CAS_DATA_CONFIG		0x17


// In Case Of OTA_TYPE==OTAPA : OTAPA STATUS
#define IPC_CALL_OTAPA_STOP  0x00
#define IPC_CALL_OTAPA_START  0x01



/*=================================================================

   SUB_CMD(1) : IPC_CALL_DIAG_OUTGOING,          0x0F : Diagnostic Outgoing Message
   
==================================================================*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_IND               0x01

 DESCRIPTION :
     Indication Diagnostic Outgoing Message
     
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CALL_TYPE(2) | DIGIT_NUM(1) | CALLED_DIGITS(variable)
-------------------------------------------------------------------------------------
*/

// CALL_TYPE(2) : refert to  call outgoing message

// DIGIT_NUM(1) : length of the dialing number(CALLED_DIGITS)

// CALLED_DIGITS(variable) : dialing number



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_CFRM               0x04

 DESCRIPTION :
     Indication Diagnostic Outgoing Message
     
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CALL_TYPE(2) | DIGIT_NUM(1) | CALLED_DIGITS(variable)
-------------------------------------------------------------------------------------
*/





/*=================================================================

   SUB_CMD(1) : IPC_CALL_E911_CB_MODE,            0x10 : CDMA E911 Callback Mode Message
   
==================================================================*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET               0x03

 DESCRIPTION :
     Indication CDMA E911 Callback Mode Message
     
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | E911_CB_MODE(1) |
-------------------------------------------------------------------------------------
*/

// E911_CB_MODE(1)
#define IPC_CALL_E911_CB_MODE_EXIT  0x00  /* E911 Callback Mode Entry */
#define IPC_CALL_E911_CB_MODE_ENTRY  0x01  /* E911 Callback Mode Exit */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI               0x03

 DESCRIPTION :
     Notification CDMA E911 Callback Mode Message
     
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | E911_CB_MODE(1) |
-------------------------------------------------------------------------------------
*/

// E911_CB_MODE(1) : refer to  the SET message



/*=================================================================

   SUB_CMD(1) : IPC_CALL_FLASH_INFO     0x11                  Flash Information Message
   
==================================================================*/
/*    IPC_CMD_EXEC                       0x01    */
/*    IPC_CMD_GET                       0x02     */
/*    IPC_CMD_CFRM                     0x04     */
/*    IPC_CMD_EVENT                   0x05     */
/*    IPC_CMD_INDI                      0x01     */
/*    IPC_CMD_RESP                     0x02     */
//none



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET               0x03

 DESCRIPTION :
     SET Flash With Information Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | NUM_LEN(1) | NUM(x) |
-------------------------------------------------------------------------------------
*/
 /* NUM_LEN */

/* NUM(x) */

/*********************************************************************************

                                            Sub Command of IPC_CDMA_DATA_CMD[0x03]

*********************************************************************************/

typedef enum{
  IPC_CDMA_DATA_TE2_STATUS = 0x01,						/* 0x01 : TE2 Status Message  */
  IPC_CDMA_DATA_BYTE_COUNTER,					/* 0x02 : Bytes Counter Message */
  IPC_CDMA_DATA_INCOMING_CALL_TYPE,				/* 0x03 : Incoming Call Type Message */
  IPC_CDMA_DATA_TE2_DIALING_INFO,					/* 0x04 : TE2 Dialing Info Message */
  IPC_CDMA_DATA_TE2_DATA_RATE_INFO,				/* 0x05 : TE2 DataRate Info InfoMessage */
  IPC_CDMA_DATA_PACKET_DATA_CALL_CFG,			/* 0x06 : Packet Data Call Configuration Message */
  IPC_CDMA_DATA_DS_BAUD_RATE,					/* 0x07 : Data Service Baud Rate Message */
  IPC_CDMA_DATA_MOBILE_IP_NAI,					/* 0x08 : Mobile IP NAI Message */
  IPC_CDMA_DATA_CURRENT_NAI_INDEX,				/* 0x09 : Current NAI Index Message */
  IPC_CDMA_DATA_DORMANT_CONFIG,					/* 0x0A : Dormant Configuration Message */
  IPC_CDMA_DATA_MIP_NAI_CHANGED,				/* 0x0B : MIP NAI Changed Message */
  IPC_CDMA_DATA_SIGNEDIN_STATE,					/* 0x0C : Signed In State Message */
  IPC_CDMA_DATA_RESTORE_NAI,						/* 0x0D : Restore NAI Message */
  IPC_CDMA_DATA_MIP_CONNECT_STATUS,				/* 0x0E : MIP Connection Status Message */
  IPC_CDMA_DATA_DORMANT_MODE_STATUS,			/* 0x0F : Dormant mode Status Message */
  IPC_CDMA_DATA_R_SCH_CONFIG,					/* 0x10 : R-SCH Configuration Message */
  IPC_CDMA_DATA_HDR_SESSION_CLEAR,				/* 0x11 : HDR Session Clear Message */
  IPC_CDMA_DATA_SESSION_CLOSE_TIMER_EXPIRED,	/* 0x12 : Session close timer expired Message */
  IPC_CDMA_DATA_KEEPALIVETIMER_VALUE,			/* 0x13 : Current keepalivetimer value Message */
  IPC_CDMA_DATA_DDTMMODE_CONFIG,				/* 0x14 : Current ddtmmode perf Configuration Message */
  IPC_CDMA_DATA_ROAM_GUARD,						/* 0x15 : Roam guard Message */
  IPC_CDMA_DATA_MODEM_NAI,						/* 0x16 : Current Modem NAI Message */
  IPC_CDMA_DATA_KOREA_MODE,						/* 0x17 : Korea Mode Message */
  IPC_CDMA_DATA_DATA_SERVICE_TYPE,				/* 0x18 : CDMA DATA_SERVICE_TYPE    message */
  IPC_CDMA_DATA_FORCE_REV_A_MODE,				/* 0x19 : Force Rev-A Mode Message */
  IPC_CDMA_DATA_CUSTOM_CONFIG_MODE,			/* 0x1A : Custom Configuration Message */
  IPC_CDMA_DATA_NAI_SETTING_MODE,				/* 0x1B : NAI Setting Mode Message */
  IPC_CDMA_DATA_PIN_CTRL,				/* 0x1C : CDMA Data Pin Ctrl */
  IPC_CDMA_DATA_RAW_DATA_MODE,              /* 0x1D : CDMA RAW_DATA_MODE */ 
  IPC_CDMA_DATA_MAX  
} ipc_cdma_data_sub_cmd_type;

/*********************************************************************************/

/*================================================================================

   SUB_CMD(1) : IPC_CDMA_DATA_TE2_STATUS    0x01               TE2 Status message
   
==================================================================================*/
/*    IPC_CMD_GET			0x02     */
/*    IPC_CMD_RESP			0x02     */
/*    IPC_CMD_NOTI			0x03     */
//none

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | TE2_TYPE(1) | TE2_STATUS(1) 
-----------------------------------------------------------------------------
*/

// TE2 TYPE Field
#define IPC_CDMA_DATA_TE2_TYPE_UART			0x00
#define IPC_CDMA_DATA_TE2_TYPE_USB			0x01
#define IPC_CDMA_DATA_TE2_TYPE_BLUETOOTH		0x02

// TE2 STATUS Field
#define IPC_CDMA_DATA_TE2_STATUS_DETACHED			0x00
#define IPC_CDMA_DATA_TE2_STATUS_ATTACHED			0x01


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x03

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | TE2_TYPE(1) | TE2_STATUS(1) |
-----------------------------------------------------------------------------
*/



/*=================================================================================*/


/*=================================================================================

   SUB_CMD(1) : IPC_CDMA_DATA_BYTE_COUNTER    0x02             Bytes Counter message
   
===================================================================================*/
/*    IPC_CMD_EXEC			0x01     */
/*    IPC_CMD_GET			0x02     */
/*    IPC_CMD_RESP			0x02     */
//none

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_EXEC                        0x01

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  
-----------------------------------------------------------------------------
*/



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  
-----------------------------------------------------------------------------
*/




/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   
 FORMAT :
--------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CURRENT_RX_BYTE(4) |CURRENT_TX_BYTE(4) |
--------------------------------------------------------------------------------------
---------------------------------------
| TOTAL_RX_BYTE(8) |TOTAL_TX_BYTE(8) |
---------------------------------------
*/




/*=================================================================================*/


/*=================================================================================

   SUB_CMD(1) : IPC_CDMA_DATA_INCOMING_CALL_TYPE    0x03     Incoming Call Type message
   
===================================================================================*/
/*    IPC_CMD_GET			0x02     */
/*    IPC_CMD_SET			0x03     */
/*    IPC_CMD_RESP			0x02     */
/*    IPC_CMD_NOTI			0x03     */
//none



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  
-----------------------------------------------------------------------------
*/



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  CALL_TYPE(1) |
-----------------------------------------------------------------------------
*/

// CALL_TYPE Field
#define IPC_CDMA_DATA_CALL_TYPE_DEFAULT					0x00	// Voice =Default.
#define IPC_CDMA_DATA_CALL_TYPE_FAX_FOR_NEXT_CALL		0x01
#define IPC_CDMA_DATA_CALL_TYPE_FAX_FOR_ALL_CALLS		0x02
#define IPC_CDMA_DATA_CALL_TYPE_ASYNC_FOR_NEXT_CALL	0x03
#define IPC_CDMA_DATA_CALL_TYPE_ASYNC_FOR_ALL_CALLS	0x04
// otherwise reserved.



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  CALL_TYPE(1) |
-----------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                        0x03

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  CALL_TYPE(1) |
-----------------------------------------------------------------------------
*/



/*=================================================================================*/


/*=================================================================================

   SUB_CMD(1) : IPC_CDMA_DATA_TE2_DIALING_INFO    0x04          TE2 Dialing Info message
   
===================================================================================*/
/*    IPC_CMD_CFRM			0x04     */
/*    IPC_CMD_INDI			0x01     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_CFRM                        0x04

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  STATUS(1) |
-----------------------------------------------------------------------------
*/

// STATUS Field //Kansas. Bluechip .
#define IPC_CDMA_DATA_CALL_ALLOWED							0x00
#define IPC_CDMA_DATA_CALL_NOT_ALLOWED_NO_SERVICE			0x01
#define IPC_CDMA_DATA_CALL_NOT_ALLOWED_CALLING_VOICE		0x02
#define IPC_CDMA_DATA_CALL_NOT_ALLOWED_NOT_CDMA			0x03
#define IPC_CDMA_DATA_CALL_NOT_ALLOWED_LOCKED				0x04
#define IPC_CDMA_DATA_CALL_NOT_ALLOWED_CALL_GUARD			0x05


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_INDI                          0x01

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  DIAL_LENGTH(1) | DIAL_DIGIT(32) |
-----------------------------------------------------------------------------
*/




/*=================================================================================*/




/*=================================================================================

   SUB_CMD(1) : IPC_CDMA_DATA_TE2_DATA_RATE_INFO    0x05      TE2 DataRate Info message
   
===================================================================================*/
/*    IPC_CMD_NOTI			0x03     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                          0x03

 DESCRIPTION :
   
 FORMAT :
------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | TX(4) | RX(4) |RPT_PERIOD(1) | CHANNEL_NUM(1) |
------------------------------------------------------------------------------------------
---------------------------------------
| TX_BAR_LEVEL(1) |RX_BAR_LEVEL(8) |
---------------------------------------
*/




/*=================================================================================*/




/*=================================================================================

   SUB_CMD(1) : IPC_CDMA_DATA_PACKET_DATA_CALL_CFG    0x06     Packet Data Call Config message
   
===================================================================================*/
/*    IPC_CMD_GET			0x02     */
/*    IPC_CMD_SET			0x03     */
/*    IPC_CMD_RESP			0x02     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  
-----------------------------------------------------------------------------
*/



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | TYPE(1) | PARAM(1) |
-----------------------------------------------------------------------------
*/

/* TYPE Field */
#define IPC_CDMA_DATA_PACKET_DATA_TYPE_QNC						0x00
#define IPC_CDMA_DATA_PACKET_DATA_TYPE_MIP						0x01
#define IPC_CDMA_DATA_PACKET_DATA_TYPE_CALL_TYPE				0x02
#define IPC_CDMA_DATA_PACKET_DATA_TYPE_PKT_ORIGIN_STIRNG		0x03
#define IPC_CDMA_DATA_PACKET_DATA_TYPE_PAP_USER_ID				0x04
#define IPC_CDMA_DATA_PACKET_DATA_TYPE_PAP_USER_PASSWORD		0x05

/* PARAM TABLE Field */

// QNC
#define IPC_CDMA_DATA_QNC_DISABLE									0x00 // In Korea Carrier
#define IPC_CDMA_DATA_QNC_ENABLE									0x01 // In USA Carrier

//MOBILE IP
#define IPC_CDMA_DATA_MIP_SIMPLE_IP_ONLY							0x00
#define IPC_CDMA_DATA_MIP_IF_AVAIL								0x01
#define IPC_CDMA_DATA_MIP_ONLY									0x02 // Sprint PCS Default

// PACKET DATA CALL TYPE
#define IPC_CDMA_DATA_TYPE_ONLY_MDR_SVC				0x00
#define IPC_CDMA_DATA_TYPE_MDR_SVC					0x01
#define IPC_CDMA_DATA_TYPE_ONLY_LSPD					0x02
#define IPC_CDMA_DATA_TYPE_HSPD						0x03


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  TYPE(1) | PARAM(1) |
-----------------------------------------------------------------------------
*/


/*=================================================================================*/




/*=================================================================================

   SUB_CMD(1) : IPC_CDMA_DATA_DS_BAUD_RATE    0x07             Data Service Baud Rate message
   
===================================================================================*/
/*    IPC_CMD_GET			0x02     */
/*    IPC_CMD_SET			0x03     */
/*    IPC_CMD_RESP			0x02     */
//none



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  
-----------------------------------------------------------------------------
*/



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | BAUD_RATE(1) 
-----------------------------------------------------------------------------
*/

/*	BAUD_RATE Field */
#define IPC_CDMA_DATA_BAUDRATE_19200_BPS		0x00
#define IPC_CDMA_DATA_BAUDRATE_38400_BPS		0x01
#define IPC_CDMA_DATA_BAUDRATE_57600_BPS		0x02
#define IPC_CDMA_DATA_BAUDRATE_115200_BPS		0x03	// default value.
#define IPC_CDMA_DATA_BAUDRATE_230400_BPS		0x04


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | BAUD_RATE(1) 
-----------------------------------------------------------------------------
*/


/*=================================================================================*/




/*=================================================================================

   SUB_CMD(1) : IPC_CDMA_DATA_MOBILE_IP_NAI    0x08             Mobile IP NAI message
   
===================================================================================*/
/*    IPC_CMD_GET			0x02     */
/*    IPC_CMD_SET			0x03     */
/*    IPC_CMD_RESP			0x02     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  NAI_ID(1) | PARAM_MASK(2) |
-----------------------------------------------------------------------------
*/

//NAI_ID Field
#define IPC_CDMA_DATA_FACTORY_NAI	0x00
#define IPC_CDMA_DATA_CUSTOM_NAI		0x01

// PARAM_MASK Field
#define IPC_CDMA_DATA_NAI_MIP_MaskNone						0x0000
#define IPC_CDMA_DATA_NAI_MIP_MaskUserNAI					0x0001
#define IPC_CDMA_DATA_NAI_MIP_MaskSPI_MN_HA					0x0002
#define IPC_CDMA_DATA_NAI_MIP_MaskSPI_MN_AAA				0x0004
#define IPC_CDMA_DATA_NAI_MIP_MaskRevTunnel					0x0008
#define IPC_CDMA_DATA_NAI_MIP_MaskHomeAddr					0x0010
#define IPC_CDMA_DATA_NAI_MIP_MaskPrimaryHomeAgentIP		0x0020
#define IPC_CDMA_DATA_NAI_MIP_MaskSecondaryHomeAgentIP		0x0040
#define IPC_CDMA_DATA_NAI_MIP_MaskMN_AAA_SS					0x0080
#define IPC_CDMA_DATA_NAI_MIP_MaskMN_HA_SS					0x0100
#define IPC_CDMA_DATA_NAI_MIP_MaskALGO_SIP_SS                   0x0200
#define IPC_CDMA_DATA_NAI_MIP_MaskALGO_MIP_mn_aaa_SS            0x0400
#define IPC_CDMA_DATA_NAI_MIP_MaskALGO_MIP_mn_ha_SS             0x0800


#define IPC_CDMA_DATA_NAI_MIP_MaskAll				0x0FFF


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
   
 FORMAT :
----------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  NAI_ID(1) |PARAM_MASK(2) |
----------------------------------------------------------------------
------------------------------------
 | USER_NAI_LEN(1) | USER_NAI(Variable) |
------------------------------------
--------------------------------------------------------
 | SPI_MN-HA(4) | SPI_MN-AAA(4) | REV_TUNNEL_PREF(1) | HOME_ADDR(4) |
--------------------------------------------------------
-------------------------------------------------
 | PRI_HOME_AGENT_IP(4) | SEC_HOME_AGENT_IP(4) |
---------------------------------------------------
 --------------------------------------------------------------------------------
 | MN-AAA_SS_LEN(1) | MN-AAA_SS(Variable) | MN-HA_SS_LEN(1) | MN-HA_SSVariable) |
 --------------------------------------------------------------------------------
 -----------
| SIP_SS(1) |
 ----------------------------------------
  ALGO_MN-AAA_SS(1) |  ALGO_MN-HA_SS(1) |
 ----------------------------------------
*/





/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   
 FORMAT :
----------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  NAI_ID(1) |PARAM_MASK(2) |
----------------------------------------------------------------------
------------------------------------
 | USER_NAI_LEN(1) | USER_NAI(Variable) |
------------------------------------
--------------------------------------------------------
 | SPI_MN-HA(4) | SPI_MN-AAA(4) | REV_TUNNEL_PREF(1) | HOME_ADDR(4) |
--------------------------------------------------------
-------------------------------------------------
 | PRI_HOME_AGENT_IP(4) | SEC_HOME_AGENT_IP(4) |
---------------------------------------------------
 --------------------------------------------------------------------------------
 | MN-AAA_SS_LEN(1) | MN-AAA_SS(Variable) | MN-HA_SS_LEN(1) | MN-HA_SSVariable) |
 --------------------------------------------------------------------------------
-----------
|SIP_SS(1) | 
 ---------------------------------------
ALGO_MN-AAA_SS(1) |  ALGO_MN-HA_SS(1) |
 ---------------------------------------


*/


/*=================================================================================*/




/*=================================================================================

   SUB_CMD(1) : IPC_CDMA_DATA_CURRENT_NAI_INDEX    0x09             Current NAI Index message
   
===================================================================================*/
/*    IPC_CMD_GET			0x02     */
/*    IPC_CMD_RESP			0x02     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
-----------------------------------------------------------------------------
*/



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   
 FORMAT :
------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | ID(1) |
------------------------------------------------------------------------------------
*/

// ID Field 
//#define IPC_CDMA_DATA_FACTORY_NAI		0x00
//#define IPC_CDMA_DATA_CUSTOM_NAI		0x01




/*=================================================================================*/




/*=================================================================================

   SUB_CMD(1) : IPC_CDMA_DATA_DORMANT_CONFIG    0x0A            Dormant Config message
   
===================================================================================*/
/*    IPC_CMD_GET			0x02     */
/*    IPC_CMD_SET			0x03     */
/*    IPC_CMD_RESP			0x02     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
-----------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | DORMANT_STATUS(1) 
-----------------------------------------------------------------------------
*/
// DORMANT_STATUS  Field
#define IPC_CDMA_DATA_DORMANT_DISABLE	0x00
#define IPC_CDMA_DATA_DORMANT_ENABLE	0x01
#define IPC_CDMA_DATA_3G_DATACALL_ACTIVE	 0x02



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   
 FORMAT :
------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | DORMANT_STATUS(1)  |
------------------------------------------------------------------------------------
*/

/*=================================================================================*/




/*=================================================================================

   SUB_CMD(1) : IPC_CDMA_DATA_MIP_NAI_CHANGED    0x0B             MIP NAI Changed message
   
===================================================================================*/
/*    IPC_CMD_NOTI			0x03     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                          0x03

 DESCRIPTION :
   
 FORMAT :
------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | ID(1) | CHANGED_ITEM(1) |
------------------------------------------------------------------------------------------
*/

// CHANGED_ITEM
#define IPC_CDMA_DATA_MIP_CHANGED_ITEM_NAI_INFO		0x00
#define IPC_CDMA_DATA_MIP_CHANGED_ITEM_SHARED_SECRET	0x01


/*=================================================================================*/




/*=================================================================================

   SUB_CMD(1) : IPC_CDMA_DATA_SIGNEDIN_STATE    0x0C             Signed In State message
   
===================================================================================*/
/*    IPC_CMD_GET			0x02     */
/*    IPC_CMD_SET			0x03     */
/*    IPC_CMD_RESP			0x02     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
-----------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | STATE(1) |
-----------------------------------------------------------------------------
*/

//STATE Field
#define IPC_CDMA_DATA_PACKET_NAI_SIGNIN_OUT			0x00
#define IPC_CDMA_DATA_PACKET_NAI_SIGNIN_IN			0x01



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   
 FORMAT :
------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | STATE(1) |
------------------------------------------------------------------------------------
*/


/*=================================================================================*/




/*=================================================================================

   SUB_CMD(1) : IPC_CDMA_DATA_RESTORE_NAI    0x0D             Restore NAI message
   
===================================================================================*/
/*    IPC_CMD_SET			0x03     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
-----------------------------------------------------------------------------
*/


/*=================================================================================*/




/*=================================================================================

   SUB_CMD(1) : IPC_CDMA_DATA_MIP_CONNECT_STATUS    0x0E             MIP Connection Status message
   
===================================================================================*/
/*    IPC_CMD_NOTI			0x03     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                          0x03

 DESCRIPTION :
   
 FORMAT :
------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | RESULT(1) |
------------------------------------------------------------------------------------------
*/

// RESULT Field
#define IPC_CDMA_DATA_MIP_CONNECT_SUCCESS				0xFF
// The others are error.


/*=================================================================================*/





/*=================================================================================

   SUB_CMD(1) : IPC_CDMA_DATA_DORMANT_MODE_STATUS    0x0F             Bytes Counter message
   
===================================================================================*/
/*    IPC_CMD_GET			0x02     */
/*    IPC_CMD_SET			0x03     */
/*    IPC_CMD_RESP			0x02     */
/*    IPC_CMD_NOTI			0x03     */
//none



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
-----------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
-----------------------------------------------------------------------------
*/



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   
 FORMAT :
------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MODE(1) | DORMANT_STATUS(1) |
------------------------------------------------------------------------------------
*/

//DATA_METHOD_MODE

// DORMANT_STATUS
#define IPC_CDMA_DATA_DORMANT_DISABLE		0x00
#define IPC_CDMA_DATA_DORMANT_ENABLE		0x01
#define IPC_CDMA_DATA_3G_DATACALL_ACTIVE		0x02


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                          0x03

 DESCRIPTION :
   
 FORMAT :
------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MODE(1) |DORMANT_STATUS(1) |
------------------------------------------------------------------------------------------
*/



/*=================================================================================*/





/*=================================================================================

   SUB_CMD(1) : IPC_CDMA_DATA_R_SCH_CONFIG    0x10             R-SCH Config message
   
===================================================================================*/
/*    IPC_CMD_GET			0x02     */
/*    IPC_CMD_SET			0x03     */
/*    IPC_CMD_RESP			0x02     */
//none

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
-----------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | R-SCH_STATUS(1) |
-----------------------------------------------------------------------------
*/

// DATA_R_SCH_STATUS
#define IPC_CDMA_DATA_R_SCH_DISABLE	0x00
#define IPC_CDMA_DATA_R_SCH_ENABLE	0x01

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   
 FORMAT :
------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | R-SCH_STATUS(1) |
------------------------------------------------------------------------------------
*/


/*=================================================================================*/





/*=================================================================================

   SUB_CMD(1) : IPC_CDMA_DATA_HDR_SESSION_CLEAR    0x11          HDR Session Clear message
   
===================================================================================*/
/*    IPC_CMD_SET			0x03     */
//none

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
-----------------------------------------------------------------------------
*/


/*=================================================================================*/





/*=================================================================================

   SUB_CMD(1) : IPC_CDMA_DATA_SESSION_CLOSE_TIMER_EXPIRED    0x12     Session close timer expired message
   
===================================================================================*/
/*    IPC_CMD_NOTI			0x03     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                          0x03

 DESCRIPTION :
   
 FORMAT :
------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
------------------------------------------------------------------------------------------
*/


/*=================================================================================*/





/*=================================================================================

   SUB_CMD(1) : IPC_CDMA_DATA_KEEPALIVETIMER_VALUE    0x13     Current keepalivetimer value message
   
===================================================================================*/
/*    IPC_CMD_GET			0x02     */
/*    IPC_CMD_SET			0x03     */
/*    IPC_CMD_RESP			0x02     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
-----------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | TIMER_VALUE(2) |
-----------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   
 FORMAT :
------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | TIMER_VALUE(2) |
------------------------------------------------------------------------------------
*/


/*=================================================================================*/





/*=================================================================================

   SUB_CMD(1) : IPC_CDMA_DATA_DDTMMODE_CONFIG    0x14       Current ddtmmode perf Configuration message
   
===================================================================================*/
/*    IPC_CMD_GET			0x02     */
/*    IPC_CMD_SET			0x03     */
/*    IPC_CMD_RESP			0x02     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
-----------------------------------------------------------------------------
*/



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | DDTMMODE_STATUS(1) |
-----------------------------------------------------------------------------
*/

// DDTM_MODE Field
#define IPC_CDMA_DATA_DDTM_MODE_OFF	0x00
#define IPC_CDMA_DATA_DDTM_MODE_ON	0x01


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   
 FORMAT :
------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | DDTMMODE_STATUS(1) |
------------------------------------------------------------------------------------
*/


/*=================================================================================*/





/*=================================================================================

   SUB_CMD(1) : IPC_CDMA_DATA_ROAM_GUARD    0x15            Roam guard message
   
===================================================================================*/
/*    IPC_CMD_GET			0x02     */
/*    IPC_CMD_SET			0x03     */
/*    IPC_CMD_RESP			0x02     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
-----------------------------------------------------------------------------
*/



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | ROAM_GUARD(1) |
-----------------------------------------------------------------------------
*/

//ROAM_GUARD Field
// Sprint Only
#define IPC_CDMA_DATA_ROAM_GUARD_DEFAULT            0x00
#define IPC_CDMA_DATA_ROAM_GUARD_ALWAYS_ASK         0x01
#define IPC_CDMA_DATA_ROAM_GUARD_NEVER_ASK          0x02
// Other
#define IPC_CDMA_DATA_ROAM_GUARD_OFF	            0x03
#define IPC_CDMA_DATA_ROAM_GUARD_ON	            	0x04


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   
 FORMAT :
------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | ROAM_GUARD(1) |
------------------------------------------------------------------------------------
*/


/*=================================================================================*/





/*=================================================================================

   SUB_CMD(1) : IPC_CDMA_DATA_MODEM_NAI    0x16             Current Modem NAI message
   
===================================================================================*/
/*    IPC_CMD_GET			0x02     */
/*    IPC_CMD_SET			0x03     */
/*    IPC_CMD_RESP			0x02     */
//none



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
-----------------------------------------------------------------------------
*/



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MODEM_NAI(1) |
-----------------------------------------------------------------------------
*/

//MODEM_NAI Field
#define IPC_CDMA_DATA_MODEM_NAI_OFF	0x00
#define IPC_CDMA_DATA_MODEM_NAI_ON	0x01

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   
 FORMAT :
------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MODEM_NAI(1) |
------------------------------------------------------------------------------------
*/


/*=================================================================================*/





/*=================================================================================

   SUB_CMD(1) : IPC_CDMA_DATA_KOREA_MODE    0x17             Korea Mode message
   
===================================================================================*/
/*    IPC_CMD_GET			0x02     */
/*    IPC_CMD_SET			0x03     */
/*    IPC_CMD_RESP			0x02     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
-----------------------------------------------------------------------------
*/



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | KOREA_MODE(1) |
-----------------------------------------------------------------------------
*/

//KOREA_MODE Field
//#define IPC_CDMA_DATA_KOREA_MODE_OFF	0x00
//#define IPC_CDMA_DATA_KOREA_MODE_ON	0x01

#define IPC_CDMA_DATA_SKT_SIP					0x00
#define IPC_CDMA_DATA_SKT_MIP					0x01
#define IPC_CDMA_DATA_SPRINT_TESTBED			0x02
#define IPC_CDMA_DATA_SPRINT_TESTBED_OMADM	0x03
#define IPC_CDMA_DATA_SPIRENT					0x04
#define IPC_CDMA_DATA_STICLAB					0x05
#define IPC_CDMA_DATA_VERIZON					0x06
#define IPC_CDMA_DATA_SPRINT					0x07
#define IPC_CDMA_DATA_SPRINT_TESTBED_OFF       0x08
#define IPC_CDMA_DATA_MODE_OFF				0xFF


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   
 FORMAT :
------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | KOREA_MODE(1) |
------------------------------------------------------------------------------------
*/


/*=================================================================================*/





/*=================================================================================

   SUB_CMD(1) : IPC_CDMA_DATA_DATA_SERVICE_TYPE    0x18        CDMA DATA_SERVICE_TYPE    message
   
===================================================================================*/
/*    IPC_CMD_GET			0x02     */
/*    IPC_CMD_SET			0x03     */
/*    IPC_CMD_RESP			0x02     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
-----------------------------------------------------------------------------
*/



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | DATA_SERVICE_TYPE(1) |
-----------------------------------------------------------------------------
*/

//DATA_SERVICE_TYPE Field
typedef enum {
   IPC_CDMA_DATA_SERVICE_NORMAL = 0x01,     // Internal Data Call
   IPC_CDMA_DATA_SERVICE_DUN,
   IPC_CDMA_DATA_SERVICE_BT_DUN,
   IPC_CDMA_DATA_SERVICE_IS,                // Internet Sharing
   IPC_CDMA_DATA_SERVICE_MMS,
   IPC_CDMA_DATA_SERVICE_MAX
} ipc_cdma_data_service_type_e_type;
/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   
 FORMAT :
------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | DATA_SERVICE_TYPE(1) |
------------------------------------------------------------------------------------
*/

/*=================================================================================*/





/*=================================================================================

   SUB_CMD(1) : IPC_CDMA_DATA_FORCE_REV_A_MODE    0x19           Force Rev-A Mode message
   
===================================================================================*/
/*    IPC_CMD_GET			0x02     */
/*    IPC_CMD_SET			0x03     */
/*    IPC_CMD_RESP			0x02     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
-----------------------------------------------------------------------------
*/



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | REV0_CONFIG(1) |
-----------------------------------------------------------------------------
*/

//REV0_CONFIG Field
#define IPC_CDMA_DATA_REVA		0x00
#define IPC_CDMA_DATA_REV0		0x01



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   
 FORMAT :
------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | REV0_CONFIG(1) |
------------------------------------------------------------------------------------
*/



/*=================================================================================*/





/*=================================================================================

   SUB_CMD(1) : IPC_CDMA_DATA_CUSTOM_CONFIG_MODE    0x1A             Custom Config message
   
===================================================================================*/
/*    IPC_CMD_GET			0x02     */
/*    IPC_CMD_SET			0x03     */
/*    IPC_CMD_RESP			0x02     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
-----------------------------------------------------------------------------
*/



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CUSTOM_CONFIG(1) |
-----------------------------------------------------------------------------
*/

//CUSTOM_CONFIG Field
#define IPC_CDMA_DATA_CUSTOM_CONFIG_DEACTIVE		0x00
#define IPC_CDMA_DATA_CUSTOM_CONFIG_ACTIVE		0x01



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   
 FORMAT :
------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CUSTOM_CONFIG(1) |
------------------------------------------------------------------------------------
*/





/*=================================================================================*/



/*=================================================================================

   SUB_CMD(1) : IPC_CDMA_DATA_NAI_SETTING_MODE    0x1B             NAI Setting Mode message
   
===================================================================================*/
/*    IPC_CMD_GET			0x02     */
/*    IPC_CMD_SET			0x03     */
/*    IPC_CMD_RESP			0x02     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
-----------------------------------------------------------------------------
*/



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | NAI_SETTING_MODE(1) |
-----------------------------------------------------------------------------
*/

//NAI_SETTING_MODE Field
#define IPC_CDMA_DATA_COMMERCIAL_NETWORK		0x00
#define IPC_CDMA_DATA_DOMESTIC_NETWORK			0x01
// otherwise reserved.



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   
 FORMAT :
------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | NAI_SETTING_MODE(1) |
------------------------------------------------------------------------------------
*/


/*=================================================================================

   SUB_CMD(1) : IPC_CDMA_DATA_PIN_CTRL    0x1C             CDMA Data Pin Ctrl
   
===================================================================================*/
/*    IPC_CMD_EXEC			0x01     */
/*    IPC_CMD_NOTI          0x03     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_EXEC                        0x01

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SIGNAL(1) | STATUS(1) |
-----------------------------------------------------------------------------
*/

/* SIGNAL */
/* refer to "ipc_data_pin_signal_e_type" type */

/* STATUS */
/* refet to "ipc_data_pin_status_e_type" type */

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                        0x03

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SIGNAL(1) | STATUS(1) |
-----------------------------------------------------------------------------
*/

/* SIGNAL */
/* refer to "ipc_data_pin_signal_e_type" type */

/* STATUS */
/* refet to "ipc_data_pin_status_e_type" type */

/*=================================================================================*/


/*=================================================================================

   SUB_CMD(1) : IPC_CDMA_DATA_RAW_DATA_MODE    0x1D             CDMA RAW_DATA_MODE   message
   
===================================================================================*/
/*    IPC_CMD_GET			0x02     */
/*    IPC_CMD_SET			0x03     */
/*    IPC_CMD_RESP			0x02     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
-----------------------------------------------------------------------------
*/



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | RAW_DATA_MODE(1) |
-----------------------------------------------------------------------------
*/

//RAW_DATA_MODE Field
#define IPC_CDMA_DATA_DPRAM      0x01
#define IPC_CDMA_DATA_VSP      0x02


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   
 FORMAT :
------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | RAW_DATA_MODE(1) |
------------------------------------------------------------------------------------
*/



/*********************************************************************************

                                            Sub Command of IPC_SMS_CMD[0x04]

*********************************************************************************/
typedef enum{
  IPC_SMS_SEND_MSG=0x01,            /* 0x01 : Send the SMS-SUBMIT, SMS-COMMAND or 
                                              SMS-SUBMIT-REPORT message */
  IPC_SMS_INCOMING_MSG,             /* 0x02 : Incoming message from the network 
                                              SMS-DELIVER or SMS-STATUS REPORT*/
  IPC_SMS_READ_MSG,                 /* 0x03 : Read the message from the phone flash or SIM */
  IPC_SMS_SAVE_MSG,                 /* 0x04 : Stores a message to memory storage */
  IPC_SMS_DEL_MSG,                  /* 0x05 : Deletes message from preferred message storage */
  IPC_SMS_DELIVER_REPORT,           /* 0x06 : Send the SMS-DELIVER-REPORT message to 
                                                                                    the network for the SMS-DELIVER message. */
  IPC_SMS_DEVICE_READY,             /* 0x07 : Device is ready to send or receive message */
  IPC_SMS_SEL_MEM,                  /* 0x08 : Select Memory to read , save and delete the message */
  IPC_SMS_STORED_MSG_COUNT,         /* 0x09 : SMS Count Stored in Memory message*/
  IPC_SMS_SVC_CENTER_ADDR,          /* 0x0A : SMS Service Center Address message */
  IPC_SMS_SVC_OPTION,               /* 0x0B : SMS Service Option message */
  IPC_SMS_MEM_STATUS,               /* 0x0C : Memory Storage Status message*/
  IPC_SMS_CBS_MSG,                  /* 0x0D : Cell Broadcast SMS Message */  
  IPC_SMS_CBS_CFG,                  /* 0x0E : Cell Broadcast Configuration message */
  IPC_SMS_STORED_MSG_STATUS,        /* 0x0F : SMS Status Stored in Memory message */
  IPC_SMS_PARAM_COUNT,              /* 0x10 : SMS Parameter count message */
  IPC_SMS_PARAM,                    /* 0x11 : SMS parameter stored in EF(smsp) message */
  IPC_SMS_STATUS,                   /* 0x12 : SMS status stored in EF(smss) message */
  IPC_SMS_MAX
} ipc_sms_sub_cmd_type;

/*********************************************************************************/



/*=================================================================

   SUB_CMD(1) : IPC_SMS_SEND_MSG    0x01                  Send the SMS message
   
==================================================================*/

/*    IPC_CMD_GET                       0x02     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_CFRM                      0x04     */
/*    IPC_CMD_EVENT                     0x05     */
/*    IPC_CMD_INDI                      0x01     */
/*    IPC_CMD_RESP                      0x02     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_EXEC                        0x01

 DESCRIPTION :
   
 FORMAT :
---------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | NETWORK_TYPE(1) | DATA(x)
---------------------------------------------------------------------
*/

/* NETWORK_TYPE field */
/* Indicate current network type is CDMA or GSM */
typedef enum {
    IPC_SMS_NETWORK_TYPE_CDMA = 0x01,
    IPC_SMS_NETWORK_TYPE_GSM,
    IPC_SMS_NETWORK_TYPE_MAX
} ipc_sms_networktype_e_type;

/* DATA field */
/* depends on Network type */

/* DATA field for CDMA */
/* DATA = RFR_CNT + PARAMETER_RECORD */
/* RFR_CNT : 1byte, Remaining Frame Count */
/* When the Phone (or PDA) send message Phone (or PDA),
   we cannot send more than maximum buffer size in the DPRM.
   So RFR_CNT (Remaining Frame Count) means remaining frame count to send the other side.
   For example we have 3 messages to send the other side, the first message RFR_CNT is 2, second one is 1, last one is 0. */
/* PARAMETER RECORD : one or more parameter records can occurs
 -------------------------------------------------------
  PARAMETER_ID(1) | PARAMETER_LEN(1) | PARAMETER_DATA(x)
 -------------------------------------------------------
*/


/* DATA field for GSM */
/*------------------------------------------*/
/* MORETOSEND(1) | DATA_LEN(1) | GSM_DATA(x)    */
/*------------------------------------------*/
/* MORETOSEND field */
typedef enum {
  IPC_SMS_MORETOSEND_NONE,
  IPC_SMS_MORETOSEND_PERSIST_LINK,              /* 0x01 : Current message is concatenated message */
  IPC_SMS_MORETOSEND_NOT_PERSIST_LINK,          /* 0x02 : Current message is not concatenated message or last  concatenated message */
  IPC_SMS_MORETOSEND_MAX
} ipc_sms_moretosend_e_type;

/* DATA_LEN field */
/* Length of GSM_DATA field */

/* GSM_DATA = SCA + TPDU */
/* SCA : 2-12 byte See 3GPP TS 23.040 Address field */
/* TPDU : SMS-SUBMIT or SMS-COMMAND. See 3GPP TS 23.040 */
/* See MAX_GSM_SMS_TPDU_SIZE */




/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                        0x03

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | NETWORK_TYPE(1) | DATA(x) |
-----------------------------------------------------------------------
*/

/* NETWORK_TYPE field */
/* Indicate current network type is CDMA or GSM */
/* see ipc_sms_networktype_e_type */

/* DATA field */
/* depends on Network type */

/* DATA field for CDMA */
/* DATA = RFR_CNT + PARAMETER_RECORD */
/* RFR_CNT : 1byte, Remaining Frame Count */
/* When the Phone (or PDA) send message Phone (or PDA),
   we cannot send more than maximum buffer size in the DPRM.
   So RFR_CNT (Remaining Frame Count) means remaining frame count to send the other side.
   For example we have 3 messages to send the other side, the first message RFR_CNT is 2, second one is 1, last one is 0. */
/* PARAMETER RECORD : one or more parameter records can occurs
 -------------------------------------------------------
  PARAMETER_ID(1) | PARAMETER_LEN(1) | PARAMETER_DATA(x)
 -------------------------------------------------------
*/

/* DATA field for GSM */
/*------------------------------------------*/
/*RESULT(2) | MSG_REF(1) | DATA_LEN(1) | GSM_DATA(x) */
/*------------------------------------------*/

/* RESULT field */
typedef enum {
/* Short Message Service General-Error Code */
  IPC_SMS_RESULT_GEN_OK  = 0x0000,                                         /* 0x0000 : SMS OK(Success) */
  IPC_SMS_RESULT_GEN_ROUTING_RELEASED,                           /* 0x0001 : Routing Released */
  IPC_SMS_RESULT_GEN_ROUTING_NOT_AVAILABLE,                 /* 0x0002 : Routing Not Available */
  IPC_SMS_RESULT_GEN_ROUTING_NOT_ALLOWED,                    /* 0x0003 : Rout Not Allowed */
  IPC_SMS_RESULT_GEN_INVALID_PARAMETER,                           /* 0x0004 : Invalid Parameter */
  IPC_SMS_RESULT_GEN_DEVICE_FAILURE,                                  /* 0x0005 : Device Failure */
  IPC_SMS_RESULT_GEN_PP_RESERVED,                                       /* 0x0006 : Pp Reserved */
  IPC_SMS_RESULT_GEN_SERVICE_RESERVED,                             /* 0x0007 : Service Reserved */
  IPC_SMS_RESULT_GEN_INVALID_LOCATION,                              /* 0x0008 : Invalid Location */
  IPC_SMS_RESULT_GEN_NO_SIM,                                                  /* 0x0009 : No SIM */
  IPC_SMS_RESULT_GEN_SIM_NOT_READY,                                   /* 0x000A : SIM Not Ready */
  IPC_SMS_RESULT_GEN_NO_NETWORK_RESPONSE,                     /* 0x000B : No Network Response */
  IPC_SMS_RESULT_GEN_DESTINATION_FDN_RESTRICTED,         /* 0x000C : Destination Address FDN Restricted */
  IPC_SMS_RESULT_GEN_SCA_FDN_RESTRICTED,                         /* 0x000D : Service Centre Address FDN Restricted */
  IPC_SMS_RESULT_GEN_RESEND_ALREADY_DONE,                    /* 0x000E : Resend Already Done */
  IPC_SMS_RESULT_GEN_SCA_NOT_AVAILABLE,                            /* 0x000F : Service Centre Address Not Available */
  IPC_SMS_RESULT_GEN_SAT_MO_CONTROL_MODIFIED,              /* 0x0010 : SAT MO Control Modified */
  IPC_SMS_RESULT_GEN_SAT_NO_CONTROL_REJECTED,               /* 0x0011 : SAT MO Control Rejected */
/* Short Message Service RP-Error Code */
  IPC_SMS_RESULT_RP_UNASSIGNED_NUMBER = 0x8001,                        /* 0x8001 : Unassigned (unallocated) Number */
  IPC_SMS_RESULT_RP_OPERATOR_DETERMINED_BARRING = 0x8008,      /* 0x8008 : perator Determined Barring */
  IPC_SMS_RESULT_RP_CALL_BARRED = 0x800A,                                      /* 0x800A : Call Barred */
  IPC_SMS_RESULT_RP_TRANSFER_REJECTED = 0x8015,                          /* 0x8015 : Short Message Transfer Rejected */
  IPC_SMS_RESULT_RP_MEM_CAPACITY_EXCEEDED = 0x8016,                 /* 0x8016 : Memory Capacity Exceeded */
  IPC_SMS_RESULT_RP_DESTINATION_OUT_OF_SERVICE = 0x801B,        /* 0x801B : Destination Out Of Service */
  IPC_SMS_RESULT_RP_UNSPECIFED_NUMBER = 0x801C,                         /* 0x801C : Unspecified Subscriber */
  IPC_SMS_RESULT_RP_FACILITY_REJECTED = 0x801D,                            /* 0x801D : Facility Rejected */
  IPC_SMS_RESULT_RP_UNKNOWN_SUBSCRIBER = 0x801E,                      /* 0x801E : Unknown Subscriber */
  IPC_SMS_RESULT_RP_NETWORK_OUT_OF_ORDER = 0x8026,                  /* 0x8026 : Network Out Of Order */
  IPC_SMS_RESULT_RP_TEMPORARY_FAILURE = 0x8029,                           /* 0x8029 : Temporary Failure */
  IPC_SMS_RESULT_RP_CONGESTION = 0x802A,                                         /* 0x802A : Congestion */
  IPC_SMS_RESULT_RP_RESOURCES_UNAVAILABLE = 0x802F,                  /* 0x802F : Resources Unavailable, Unspecified */
  IPC_SMS_RESULT_RP_FACILITY_NOT_SUBSCRIBED = 0x8032,               /* 0x8032 : Requested Facility Not Subscribed */
  IPC_SMS_RESULT_RP_FACILITY_NOT_IMPLEMENTED = 0x8045,             /* 0x8045 : Requested Facility Not Implemented */
  IPC_SMS_RESULT_RP_INVALID_TRANSFER_REF_VALUE = 0x8051,         /* 0x8051 : Invalid Short Message Transfer Reference Value */
  IPC_SMS_RESULT_RP_INVALID_MESSAGE = 0x805F,                               /* 0x805F : Invalid Message, Unspecified */
  IPC_SMS_RESULT_RP_INVALID_MANDATORY_INFO = 0x8060,                /* 0x8060 : Invalid Mandatory Information */
  IPC_SMS_RESULT_RP_MSG_TYPE_NON_EXISTENT = 0x8061,                  /* 0x8061 : Message Type Non-Existent or Not Implemented */
  IPC_SMS_RESULT_RP_NOT_COMPATIBLE_PROTOCOL = 0x8062,              /* 0x8062 : Message Not Compatible with Short Message Protocol State */
  IPC_SMS_RESULT_RP_INFO_ELEMENT_NON_EXISTENT = 0x8063,           /* 0x8063 : Information Element Non-Existent or Not Implemented */
  IPC_SMS_RESULT_RP_PROTOCOL_ERROR = 0x806F,                                /* 0x806F : Protocol Error, Unspecified */
  IPC_SMS_RESULT_RP_INTERWORKING = 0x807F,                                    /* 0x807F : Interworking, Unspecified */
  IPC_SMS_RESULT_MAX
}ipc_sms_result_e_type;

/* MSG_REF field */
/* TP Message Reference  in TPDU. See 3GPP TS 23.040 */

/* DATA_LEN  field */
/* Length of GSM_DATA field */

/* GSM_DATA  field */
/* GSM_DATA = SCA+TPDU */
/* SCA : 2-12 byte See 3GPP TS 23.040 Address field */
/* TPDU : SMS-SUBMIT-REPORT. See 3GPP TS 23.040 */
/* See MAX_GSM_SMS_TPDU_SIZE */


/*====================================================================*/



/*=================================================================

   SUB_CMD(1) : IPC_SMS_INCOMING_MSG    0x02             Incoming SMS Message
   
==================================================================*/

/*    IPC_CMD_EXEC                      0x01     */
/*    IPC_CMD_GET                       0x02     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_CFRM                      0x04     */
/*    IPC_CMD_EVENT                     0x05     */
/*    IPC_CMD_INDI                      0x01     */
/*    IPC_CMD_RESP                      0x02     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                        0x03

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | NETWORK_TYPE(1) | DATA(x) |
-----------------------------------------------------------------------------------
*/

/* NETWORK_TYPE field */
/* Indicate current network type is CDMA or GSM */
/* see ipc_sms_networktype_e_type */

/* DATA field */
/* depends on Network type */

/* DATA field for CDMA */
/* DATA = RFR_CNT(1) +  SMS_MSG_CAT(1)+ PARAMETER_RECORD */
/* RFR_CNT : 1byte, Remaining Frame Count */
/* When the Phone (or PDA) send message Phone (or PDA),
   we cannot send more than maximum buffer size in the DPRM.
   So RFR_CNT (Remaining Frame Count) means remaining frame count to send the other side.
   For example we have 3 messages to send the other side, the first message RFR_CNT is 2, second one is 1, last one is 0. */
/* PARAMETER RECORD : one or more parameter records can occurs
 -------------------------------------------------------
  RFR_CNT(1) |  SMS_MSG_CAT(1) | PARAMETER_ID(1) | PARAMETER_LEN(1) | PARAMETER_DATA(x)
 -------------------------------------------------------
*/
/* RFR_CNT ( Remaining Frame Count ) */
	
/* SMS_MSG_CAT */
#define IPC_SMS_PTP		0x00	/* SMS Point to Point Message */
#define IPC_SMS_BC		0x01	/* SMS Broadcast Message */


/* DATA field for GSM */
/*------------------------------------------*/
/*  FORMAT(1) | SIM_INDEX(2) | DATA_LEN(1) | GSM_DATA(x) */
/*------------------------------------------*/

/* FORMAT field */
/* Data Format */
typedef enum {
  IPC_SMS_FORMAT_NONE,
  IPC_SMS_FORMAT_PP,                /* 0x01 : SMS Point-to-Point Message */
  IPC_SMS_FORMAT_SR,                /* 0x02 : SMS Status Report Message */  
  IPC_SMS_FORMAT_MAX
} ipc_sms_format_e_type;


/* SIM_INDEX field */
/* location value in SIM card */
/* 0xFFFF means that message does not saved in SIM card */

/* DATA_LEN  field */
/* Length of GSM_DATA field */

/* GSM_DATA  field */
/* GSM_DATA = SCA + TPDU */
/* SCA : 2-12 byte See 3GPP TS 23.040 Address field */
/* TPDU : SMS-DELIVER or SMS-STATUS-REPORT. See 3GPP TS 23.040 */
/* See MAX_GSM_SMS_TPDU_SIZE */


/*====================================================================*/



/*=================================================================

   SUB_CMD(1) : IPC_SMS_READ_MSG    0x03                  Read SMS Message
   
==================================================================*/

/*    IPC_CMD_EXEC                      0x01     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_CFRM                      0x04     */
/*    IPC_CMD_EVENT                     0x05     */
/*    IPC_CMD_INDI                      0x01     */
/*    IPC_CMD_NOTI                      0x03     */
//none



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
     Read SMS GET
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MEM_STORE(1) | INDEX(2) | 
-------------------------------------------------------------------------------------
*/

/* MEM_STORE field */
/* Current location used to store */
typedef enum {
IPC_SMS_MEMORY_STORE_NONE     = 0x00,   /* 0x00, Not Store */
IPC_SMS_MEMORY_STORE_RAM_GW,	        /* 0x01, Phone RAM */
IPC_SMS_MEMORY_STORE_SIM,               /* 0x02, GSM SIM Card for SLOT1 */
IPC_SMS_MEMORY_STORE_SIM2,	            /* 0x03, GSM SIM Card for SLOT2 */
IPC_SMS_MEMORY_STORE_NV_GW,             /* 0x04, NV + Phone RAM */
IPC_SMS_MEMORY_STORE_NV_SIM,            /* 0x05, NV + SIM */
IPC_SMS_MEMORY_STORE_RAM_CDMA = 0x11, 	/* 0x11, NOT SUPPORTED */
IPC_SMS_MEMORY_STORE_RUIM,              /* 0x12, CDMA RUIM Card for SLOT1 */
IPC_SMS_MEMORY_STORE_RUIM2,             /* 0x13, CDMA RUIM Card for SLOT2 */
IPC_SMS_MEMORY_STORE_NV_CDMA,           /* 0x14, NV */
IPC_SMS_MEMORY_STORE_MAX,
} ipc_sms_memory_e_type;

/* INDEX  field */
/* location value <index> from preferred message storage */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   Read SMS RESPONSE
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MEM_STORE(1) | INDEX(2) | DATA(x) |
-------------------------------------------------------------------------------------
*/

/* MEM_STORE field */
/* Current location used to store */
/* See ipc_sms_memory_e_type */

/* INDEX  field */
/* location value <index> from preferred message storage */

/* DATA field */
/* depends on Network type */

/* DATA field for CDMA */
/* FORMAT : 
------------------------------------------------------------
 RFR_CNT(1) | SMS_MSG_CAT(1) | TAG(1) | PARAMETER_RECORD(x)
------------------------------------------------------------*/
/* RFR_CNT : 1byte, Remaining Frame Count */
/* When the Phone (or PDA) send message Phone (or PDA),
   we cannot send more than maximum buffer size in the DPRM.
   So RFR_CNT (Remaining Frame Count) means remaining frame count to send the other side.
   For example we have 3 messages to send the other side, the first message RFR_CNT is 2, second one is 1, last one is 0. */
/* SMS_MSG_CAT field */
/* Indicating message category */
typedef enum {
    IPC_SMS_MSG_CAT_POINT2POINT = 0x00,
    IPC_SMS_MSG_CAT_BROADCAST,
    IPC_SMS_MSG_CAT_MAX
} ipc_sms_msg_cat_e_type;
/* TAG field */
/* Status of saved SMS */
/* See ipc_sms_status_e_type */
/* PARAMETER RECORD : one or more parameter records can occurs
 -------------------------------------------------------
  PARAMETER_ID(1) | PARAMETER_LEN(1) | PARAMETER_DATA(x)
 -------------------------------------------------------
*/


/* DATA field for GSM */
/*------------------------------------------*/
/* MSG_STATUS(1) | DATA_LEN(1) | GSM_DATA(x) */
/*------------------------------------------*/

/* MSG_STATUS  field */
/* the status of message in memory */
typedef enum{
  IPC_SMS_STATUS_UNKNOWN,       /* 0x00 : Required index is unknown or empty  Minsung Kim 2005.02.04 */ 
  IPC_SMS_STATUS_UNREAD,        /* 0x01 : Received unread message (i.e. new message) */
  IPC_SMS_STATUS_READ,          /* 0x02 : Received read message */
  IPC_SMS_STATUS_UNSENT,        /* 0x03 : unsent message (only applicable to SMs) */
  IPC_SMS_STATUS_SENT,          /* 0x04 : Stored sent message (only applicable to SMs) */
  IPC_SMS_STATUS_MAX
} ipc_sms_status_e_type; 

/* DATA_LEN  field */
/* Length of GSM_DATA field */

/* GSM_DATA  field */
/* GSM_DATA = SCA + TPDU */
/* SCA : 2-12 byte See 3GPP TS 23.040 Address field */
/* TPDU : SMS-SUBMIT or SMS-DELIVER, SMS-STATUS-REPORT. See 3GPP TS 23.040 */
/* See MAX_GSM_SMS_TPDU_SIZE */


/*====================================================================*/



/*=================================================================

   SUB_CMD(1) : IPC_SMS_SAVE_MSG    0x04                         Save SMS Message
   
=================================================================*/

/*    IPC_CMD_GET                       0x02     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_CFRM                      0x04     */
/*    IPC_CMD_EVENT                     0x05     */
/*    IPC_CMD_INDI                      0x01     */
/*    IPC_CMD_RESP                      0x02     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_EXEC                        0x01

 DESCRIPTION :
   Save SMS EXEC
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MEM_STORE(1) | INDEX(2) | DATA(x) |
-------------------------------------------------------------------------------------
*/

/* INDEX  field */
/* location value <index> in the specified memory */

/* MEM_STORE field */
/* Current location used to store */
/* See ipc_sms_memory_e_type */

/* DATA field */
/* depends on Network type */
/* Same as DATA of IPC_SMS_READ_MSG response type */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                        0x03

 DESCRIPTION :
   Save SMS NOTI
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MEM_STORE(1) | RESULT(2) |INDEX(2) | 
-------------------------------------------------------------------------------------
*/

/* MEM_STORE field */
/* Current location used to store */
/* See ipc_sms_memory_e_type */

/* RESULT  field */
/* See ipc_sms_result_e_type */

/* INDEX  field */
/* location value <index> from preferred message storage */


/*====================================================================*/



/*=================================================================

   SUB_CMD(1) : IPC_SMS_DEL_MSG    0x05                        Delete SMS Message
   
==================================================================*/

/*    IPC_CMD_GET                       0x02     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_CFRM                      0x04     */
/*    IPC_CMD_EVENT                     0x05     */
/*    IPC_CMD_INDI                      0x01     */
/*    IPC_CMD_RESP                      0x02     */
//none



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_EXEC                        0x01

 DESCRIPTION :
     Delete SMS EXEC
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MEM_STORE(1) | INDEX(2) | 
-------------------------------------------------------------------------------------
*/

/* MEM_STORE field */
/* Current location used to store */
/* See ipc_sms_memory_e_type */

/* INDEX  field */
/* location value <index> in the specified memory */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                        0x03

 DESCRIPTION :
   Deleete SMS NOTI
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MEM_STORE(1) | RESULT(2) |INDEX(2) | 
-------------------------------------------------------------------------------------
*/

/* MEM_STORE field */
/* Current location used to store */
/* See ipc_sms_memory_e_type */

/* RESULT  field */
/* See ipc_sms_result_e_type */

/* INDEX  field */
/* location value <index> from preferred message storage */


/*====================================================================*/



/*=================================================================

   IPC_SMS_DELIVER_REPORT    0x06                      Send the SMS-DELIVER-REPORT Message
   
==================================================================*/

/*    IPC_CMD_GET                       0x02     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_CFRM                      0x04     */
/*    IPC_CMD_EVENT                     0x05     */
/*    IPC_CMD_INDI                      0x01     */
/*    IPC_CMD_RESP                      0x02     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_EXEC                        0x01

 DESCRIPTION :
   Send SMS-DELIVER-REPORT Message EXEC
 FORMAT :
-----------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | NETWORK_TYPE(1) | DATA(x) |
-----------------------------------------------------------------------
*/

/* NETWORK_TYPE field */
/* Indicate current network type is CDMA or GSM */
/* see ipc_sms_networktype_e_type */

/* DATA field */
/* depends on Network type */

/* DATA field for CDMA */
/* Same as DATA of IPC_SMS_SEND_MSG excute type */

/* DATA field for GSM */
/*--------------------------------------------*/
/* | RESULT(2) | DATA_LEN(1) | GSM-DATA(x) |  */
/*--------------------------------------------*/

/* RESULT  field */
/*see ipc_sms_result_e_type */

/* DATA_LEN  field */
/* Length of DATA field */

/* GSM-DATA  field */
/* GSM-DATA = TPDU */
/* TPDU : SMS-DELIVER-REPORT. See 3GPP TS 23.040 */
/* See MAX_GSM_SMS_TPDU_SIZE */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                        0x03

 DESCRIPTION :
   Send SMS-DELIVER-REPORT Message NOTI
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | RESULT(2)
-------------------------------------------------------------------------------------
*/

/* RESULT  field */
/*see ipc_sms_result_e_type */


/*====================================================================*/




/*=================================================================

   SUB_CMD(1) : IPC_SMS_DEVICE_READY    0x07                     Device is ready to send or receive message
   
==================================================================*/

/*    IPC_CMD_EXEC                      0x01     */
/*    IPC_CMD_GET                       0x02     */
/*    IPC_CMD_CFRM                      0x04     */
/*    IPC_CMD_EVENT                     0x05     */
/*    IPC_CMD_INDI                      0x01     */
/*    IPC_CMD_RESP                      0x02     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
     Device(PDA) is ready to receive message from network
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1)
-------------------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                        0x03

 DESCRIPTION :
     Device(Phone) is ready to access message in SIM card
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MEM_STORE(1) |
-------------------------------------------------------------------------------------
*/


/*====================================================================*/



/*=================================================================

   SUB_CMD(1) : IPC_SMS_SEL_MEM    0x08                Select SMS Memory Message

==================================================================*/

/*    IPC_CMD_EXEC                      0x01     */
/*    IPC_CMD_CFRM                      0x04     */
/*    IPC_CMD_EVENT                     0x05     */
/*    IPC_CMD_INDI                      0x01     */
/*    IPC_CMD_NOTI                      0x03     */
//none



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
-------------------------------------------------------------------------------------
*/



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MEM_STORE(1) |
-------------------------------------------------------------------------------------
*/

/* MEM_STORE  field */
/* See ipc_sms_memory_e_type */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MEM_STORE(1) | 
-------------------------------------------------------------------------------------
*/


/*====================================================================*/



/*=================================================================

   SUB_CMD(1) : IPC_SMS_STORED_MSG_COUNT    0x09                 Stored SMS Count Message

=================================================================*/

/*    IPC_CMD_EXEC                      0x01     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_CFRM                      0x04     */
/*    IPC_CMD_EVENT                     0x05     */
/*    IPC_CMD_INDI                      0x01     */
/*    IPC_CMD_NOTI                      0x03     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MEM_STORE(1)
-------------------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MEM_STORE(1) | TOTAL(2) | USED(2) | INDEX(x) |
-------------------------------------------------------------------------------------
*/

/* TOTAL  field */
/* The total number of sms which can be in the memory */
/* see MAX_GSM_SMS_MSG_NUM */

/* USED  field */
/* The total used number of sms which can be in the memory */
/* see MAX_GSM_SMS_MSG_NUM */

/* INDEX field */
/* Index stored in the memory, the max bytes is MAX_GSM_SMS_MSG_NUM */


/*====================================================================*/



/*=================================================================

   SUB_CMD(1) : IPC_SMS_SVC_CENTER_ADDR    0x0A              SMS Service Center Address Message

=================================================================*/

/*    IPC_CMD_EXEC                      0x01     */
/*    IPC_CMD_CFRM                      0x04     */
/*    IPC_CMD_EVENT                     0x05     */
/*    IPC_CMD_INDI                      0x01     */
/*    IPC_CMD_NOTI                      0x03     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
   SMS Message Configuration GET
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
   SMS Message Configuration SET   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SCA(x) |
-------------------------------------------------------------------------------------
*/

/* SCA field */
/* Service Center Address */ 
/* See MAX_GSM_SMS_SERVICE_CENTER_ADDR */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                       0x02

 DESCRIPTION :
   SMS Message Configuration RESPONSE
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SCA(x) |
-------------------------------------------------------------------------------------
*/

/* SCA field */
/* Service Center Address */ 
/* See MAX_GSM_SMS_SERVICE_CENTER_ADDR */


/*====================================================================*/




/*=================================================================

   SUB_CMD(1) : IPC_SMS_SVC_OPTION    0x0B             SMS Service Option Message

=================================================================*/

/*    IPC_CMD_EXEC                      0x01    */
/*    IPC_CMD_GET                       0x02    */
/*    IPC_CMD_CFRM                      0x04     */
/*    IPC_CMD_EVENT                     0x05     */
/*    IPC_CMD_INDI                      0x01     */
/*    IPC_CMD_RESP                      0x02     */
/*    IPC_CMD_NOTI                      0x03     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
   SMS Message Configuration SET   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SVC(1) |
-------------------------------------------------------------------------------------
*/

/* SVC field */
/* defined in TS 27.007  10.1.20 */
typedef enum {
  IPC_SMS_SVC_NONE,
  IPC_SMS_SVC_PS_ONLY,            /* 0x01 : packet domain only */
  IPC_SMS_SVC_CS_ONLY,            /* 0x02 : circuit domain only */
  IPC_SMS_SVC_PS_PREFERED,     /* 0x03 : packet domain preferred */
  IPC_SMS_SVC_CS_PREFERED,     /* 0x04 : circuit domain preferred */
  IPC_SMS_SVC_MAX
} ipc_sms_svc_e_type;


/*====================================================================*/



/*=================================================================

   SUB_CMD(1) : IPC_SMS_MEM_STATUS    0x0C             Memory storage status Message 

=================================================================*/

/*    IPC_CMD_EXEC                      0x01    */
/*    IPC_CMD_GET                       0x02    */
/*    IPC_CMD_CFRM                      0x04    */
/*    IPC_CMD_EVENT                     0x05    */
/*    IPC_CMD_INDI                      0x01    */
/*    IPC_CMD_RESP                      0x02    */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
   PDA Memory is full or available   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |MEMORY_STATUS(1) |
-------------------------------------------------------------------------------------
*/

/* MEMORY_STATUS field */
typedef enum {
  IPC_SMS_MEM_STATUS_NONE,
  IPC_SMS_MEM_STATUS_AVAILABLE,     /* 0x01 : Memory is available*/
  IPC_SMS_MEM_STATUS_FULL,               /* 0x02 : Memory is full */
  IPC_SMS_MEM_STATUS_MAX
} ipc_sms_mem_status_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                        0x03

 DESCRIPTION :
     Phone Memory(SIM and NV) is full or available
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MEM_STORE(1) | MEMORY_STATUS(1) | 
-------------------------------------------------------------------------------------
*/

/* MEM_STORE field */
/* see ipc_sms_memory_e_type */

/* MEMORY_STATUS field */
/* see ipc_sms_mem_status_e_type */


/*====================================================================*/



/*=================================================================

   SUB_CMD(1) : IPC_SMS_CBS_MSG    0x0D             Cell broadcast Short Message Service Message
   
==================================================================*/

/*    IPC_CMD_EXEC                      0x01     */
/*    IPC_CMD_GET                       0x02     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_CFRM                      0x04     */
/*    IPC_CMD_EVENT                     0x05     */
/*    IPC_CMD_INDI                      0x01     */
/*    IPC_CMD_RESP                      0x02     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                        0x03

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CB_TYPE(1) | MESSAGE_LEN(1) | MESSAGE(x) |
-------------------------------------------------------------------------------------
*/

/* CB_TYPE field */
typedef enum {
  IPC_SMS_CB_TYPE_NONE,
  IPC_SMS_CB_TYPE_GSM,                  /* 0x01 : SMS Point-to-Point Message */
  IPC_SMS_CB_TYPE_UMTS,                /* 0x02 : SMS Status Report Message */  
  IPC_SMS_CB_TYPE_MAX
} ipc_sms_cb_type_e_type;

/* MESSAGE_LEN  field */
/* Length of MESSAGE field */

/* MESSAGE  field */
/* stored message index */


/*====================================================================*/



/*=================================================================

   SUB_CMD(1) : IPC_SMS_CBS_CFG    0x0E           Cell Broadcast SMS Configuration Message

=================================================================*/

/*    IPC_CMD_EXEC                      0x01     */
/*    IPC_CMD_CFRM                      0x04     */
/*    IPC_CMD_EVENT                     0x05     */
/*    IPC_CMD_INDI                      0x01     */
/*    IPC_CMD_NOTI                      0x03     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
   CBS Message Configuration GET
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
   CBS Message Configuration SET
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | ENABLE(1) |SELECTED ID(1) | LIST COUNT(1) | LIST (x) |
-------------------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                       0x02

 DESCRIPTION :
   CBS Message Configuration RESPONSE
 FORMAT :
--------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | ENABLE(1) |SELECTED ID(1) | MAX LIST COUNT(1) |
--------------------------------------------------------------------------------
-------------------------------------
| LIST COUNT(1) | LIST (x) |
-------------------------------------
*/

/* ENABLE field */
typedef enum {
  IPC_SMS_CBS_SVC_NONE,
  IPC_SMS_CBS_SVC_ENABLE,       /* 0x01 : Enable CBS service */
  IPC_SMS_CBS_SVC_DISABLE,      /* 0x02 : Disable CBS service */
  IPC_SMS_CBS_SVC_MAX
} ipc_sms_cbs_svc_e_type;

/* SELECTED ID field */
typedef enum {
  IPC_SMS_CBS_ID_NONE,
  IPC_SMS_CBS_ID_ALL_CBMI,      /* 0x01 : All CBMI is selected */
  IPC_SMS_CBS_ID_SOME_CBMI,   /* 0x02 : Some CBMI is selected */
  IPC_SMS_CBS_ID_MAX
} ipc_sms_cbs_id_e_type;


/*====================================================================*/



/*=================================================================

   SUB_CMD(1) : IPC_SMS_STORED_MSG_STATUS    0x0F           SMS Message Status Message

=================================================================*/

/*    IPC_CMD_EXEC                      0x01     */
/*    IPC_CMD_GET                       0x02     */
/*    IPC_CMD_CFRM                      0x04     */
/*    IPC_CMD_EVENT                     0x05     */
/*    IPC_CMD_INDI                      0x01     */
/*    IPC_CMD_RESP                      0x02     */
/*    IPC_CMD_NOTI                      0x03     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
   Stored Message Status SET
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MEM_STORE(1) | INDEX(2) | MSG_STATUS(1) |
-------------------------------------------------------------------------------------
*/

/* MEMORY field */
/* see ipc_sms_memory_e_type */

/* INDEX  field */
/* location value <index> from preferred message storage */

/* MSG_STATUS field */
/* ipc_sms_status_e_type */


/*====================================================================*/



/*=================================================================

   SUB_CMD(1) : IPC_SMS_PARAM_COUNT    0x10             SMS Parameter count Message

=================================================================*/

/*    IPC_CMD_EXEC                      0x01    */
/*    IPC_CMD_SET                       0x03    */
/*    IPC_CMD_CFRM                      0x04    */
/*    IPC_CMD_EVENT                     0x05    */
/*    IPC_CMD_INDI                      0x01    */
/*    IPC_CMD_NOTI                      0x03    */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
   SMS parameter record count GET   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MEM_STORE(1)
-------------------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   SMS parameter record count RESP   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MEM_STORE(1) | RECORD_COUNT(1) |
-------------------------------------------------------------------------------------
*/

/* RECORD_COUNT field */
/* count of record in EFsmsp. see 3GPP TS 31.102 */

/*====================================================================*/



/*=================================================================

   SUB_CMD(1) : IPC_SMS_PARAM    0x11             SMS parameter stored in EFsmsp message

=================================================================*/

/*    IPC_CMD_EXEC                      0x01    */
/*    IPC_CMD_CFRM                      0x04    */
/*    IPC_CMD_EVENT                     0x05    */
/*    IPC_CMD_INDI                      0x01    */
/*    IPC_CMD_NOTI                      0x03    */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
   SMS parameter GET   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MEM_STORE(1) | RECORD_INDEX(1) |
-------------------------------------------------------------------------------------
*/

/* RECORD_INDEX field */
/* Index of Records in EFsmsp */

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
   SMS parameter SET   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MEM_STORE(1) | RECORD_INDEX(1) | RECORD_LEN(1) | RECORD(x) |
-------------------------------------------------------------------------------------
*/

/* RECORD_LEN field */
/* Length of RECORD field */
/* See MAX_GSM_SMS_PARAM_RECORD_SIZE */

/* RECORD field */
/* EFsmsp Record as specified in 3GPP TS 31.102 : 4.2.27 */
/*
Alpha-Identifier (0 to Y)
Parameter Indicator (Y+1)
TP-Destination Address (Y+2 to Y+13)
TP-Service Centre Address (Y+14 to Y+25)
TP-Protocol Identifier (Y+26)
TP-Data Coding Scheme (Y+27)
TP-Validity Period (Y+28)
*/
/* The value of Y may be from zero to 128. See 3GPP TS 31.102 : 4.4.23 for more alpha identifier details.*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   SMS parameter RESP   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MEM_STORE(1) | RECORD_INDEX(1) | RECORD_LEN(1) | RECORD(x) |
-------------------------------------------------------------------------------------
*/


/*====================================================================*/


/*=================================================================

   SUB_CMD(1) : IPC_SMS_STATUS    0x12             SMS parameter stored in EFsmsp message

=================================================================*/

/*    IPC_CMD_EXEC                      0x01    */
/*    IPC_CMD_SET                       0x03    */
/*    IPC_CMD_CFRM                      0x04    */
/*    IPC_CMD_EVENT                     0x05    */
/*    IPC_CMD_INDI                      0x01    */
/*    IPC_CMD_NOTI                      0x03    */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
   SMS parameter GET   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MEM_STORE(1) | 
-------------------------------------------------------------------------------------
*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   SMS parameter RESP   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MEM_STORE(1) | MSG_REF(1) | MEM_EXCEEDED_FLAG(1) |
-------------------------------------------------------------------------------------
*/

/* MSG_REF field */
/* Initial message reference value */

/* MEM_EXCEEDED_FLAG field */
/* Flag which indicates whether memory is available */
/* 0x00 : FALSE - memory available      */
/* 0x01 : TRUE  - memory unavailable    */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
   SMS status SET   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MEM_STORE(1) | MSG_REF(1) | MEM_EXCEEDED_FLAG(1) |
-------------------------------------------------------------------------------------
*/

/* MSG_REF field */
/* Initial message reference value */

/* MEM_EXCEEDED_FLAG field */
/* Flag which indicates whether memory is available */
/* 0x00 : FALSE - memory available      */
/* 0x01 : TRUE  - memory unavailable    */



/*====================================================================*/
/*====================== START OF CDMA SMS PARAMETER ================*/

//=============================================================
//
//		RFR_CNT ( Remaining Frame Count ) :
//
//=============================================================


//-------------------------------------------------------------------------------*/

// <  >
// DPRAM SIZE  ѹ SMS       µ
// ̶  ϴ FRAME ؼ   FRAME  ϰ ִ
// FRAME  ǹѴ.
//
// <  >
// ϰ ϴ ü FRAME  3̰  1° FRAME ϰ  
// RFR_CNT 2 ȴ. ׸  2° FRAME   RFR_CNT 1̵Ǹ,
// 3° FRAME ϰ   RFR_CNT 0 ȴ.

//=============================================================
//
//		PARAMETER_ID
//
//=============================================================
#define IPC_SMSPARAMID_TELESERVICE_ID				0x01	/* Teleservice Identifier */
#define IPC_SMSPARAMID_SERVICE_CATEGORY			0x02	/* Broadcast Service Category */
#define IPC_SMSPARAMID_ADDRESS					0x03	/* Address */
#define IPC_SMSPARAMID_SUBADDRESS				0x04	/* Subaddress */
#define IPC_SMSPARAMID_BEARER_REPLY				0x05	/* Bearer Reply Option */
#define IPC_SMSPARAMID_CAUSE_CODES				0x06	/* Cause Codes */
#define IPC_SMSPARAMID_MESSAGE_ID				0x07	/* Message Identifier */
#define IPC_SMSPARAMID_USER_DATA					0x08	/* User Data */
#define IPC_SMSPARAMID_USER_RESPONSE_CODE		0x09	/* User Response Code */
#define IPC_SMSPARAMID_MC_TIME_STAMP			0x0A	/* Message Center Time Stamp */
#define IPC_SMSPARAMID_VALIDITY_PERIOD_ABS		0x0B	/* Validity Period - Absolute */
#define IPC_SMSPARAMID_VALIDITY_PERIOD_REL		0x0C	/* Validiry Period - Relative */
#define IPC_SMSPARAMID_DEFERRED_DELIVERY_ABS	0x0D	/* Deferred Delivery Time - Absolute */
#define IPC_SMSPARAMID_DEFERRED_DELIVERY_REL	0x0E	/* Deferred Delivery Time - Relative */
#define IPC_SMSPARAMID_PRIORITY					0x0F	/* Priority Indicator */
#define IPC_SMSPARAMID_PRIVACY					0x10	/* Privacy Indicator */
#define IPC_SMSPARAMID_REPLY_OPTION				0x11	/* Reply Option */
#define IPC_SMSPARAMID_NUMBER_OF_MESSAGE		0x12	/* Number of Messages : Voice Mail Count */
#define IPC_SMSPARAMID_ALERT_ON_DELIVERY			0x13	/* Alert on Message Delivery */
#define IPC_SMSPARAMID_LANGUAGE					0x14	/* Langauge Indicator */
#define IPC_SMSPARAMID_CALLBACK					0x15	/* Call Back Number */
#define IPC_SMSPARAMID_DISPLAY_MODE				0x16	/* Display Mode */
#define IPC_SMSPARAMID_MULTI_ENCODING_USER_DATA	0x17	/* Multiply Encoding User Data */
#define IPC_SMSPARAMID_MEMORY_INDEX				0x18	/* Memory address stored in Phone Memory */
#define IPC_SMSPARAMID_BEARER_DATA				0x19	/* Bearer data - raw data  */
#define IPC_SMSPARAMID_SCPT_DATA                               0x1A     /* Service Category Program Data */
#define IPC_SMSPARAMID_SCPT_RESURLT                        0x1B    /* Service Category Program Result */


/* PARAMETER_LENGTH
*/

//=============================================================
//
//		PARAMETER_DATA
//
//=============================================================

//------------------------------------------------------------------------------
// IPC_SMSPARAMID_TELESERVICE_ID			0x01	/* Teleservice Identifier */
//------------------------------------------------------------------------------
 /* Common TELESERVICE ID's */
#define IPC_TELESERVICEID_CPT95				0x1001	/* 4097 : Paging */
#define IPC_TELESERVICEID_CMT95				0x1002	/* 4098 : Text Message */
#define IPC_TELESERVICEID_VMN95				0x1003	/* 4099 : Voice Mail Notification */
#define IPC_TELESERVICEID_WAP					0x1004	/* WAP Message */

#if 0
#define IPC_TELESERVICEID_CMT91_CLI			0x03	/* 4096 : Analog Paging */
#define IPC_TELESERVICEID_CMT91_VOICEMAIL	0x04	/* 4096 : Analog Text Message */
#define IPC_TELESERVICEID_CMT91_SHARTMSG	0x05	/* 4096 : Analog Text Message */
#define IPC_TELESERVICEID_MSGWAITING		0x06	/* Message Waiting Record Indicator */

 /* Following TELESERVICE ID's are defined to be used
      only in KOREAN Wireless Service Carriers
 */
 /* Interactive Teleservice */
#define IPC_TELESERVICEID_PCS_INTERACTIVE			0x10	/* 49153 : PCS 3 ܸ ǥر԰ */
#define IPC_TELESERVICEID_STI_INTERACTIVE			0x11	/* 53248 : STI Interactive */
 /* Melody Download and Melody Mail */
#define IPC_TELESERVICEID_MELODY_DLOAD		0x12	/* Melody Download : KTF-LGT(32768),HST(49156),SKT(64000),STI(61456)*/
#define IPC_TELESERVICEID_MELODY_MAIL			0x13	/* 61457 : STI Melody Mail Service */
 /* STI Power Angel Service */
#define IPC_TELESERVICEID_STI_ANGEL_DISPLAY_CTRL		0x14	/* 61440:STI Service 61440 : ȭ   */
#define IPC_TELESERVICEID_STI_ANGEL_URGENT_DISPLAY	0x15	/* 61441:STI Service 61441 :  ȭ  */
 /* URL / ĳ ٿε/  / ī */
#define IPC_TELESERVICEID_STI_URL					0x16	/* 61472: STI : URL Data SMS ش. */
#define IPC_TELESERVICEID_KTF_URL					0x17	/* 45858: KTF : URL Addr ˷ش.(char/image download) */
#define IPC_TELESERVICEID_HST_URL					0x18	/* 49159: HST : URL Addr ˷ش.(char/image download) */

#define IPC_TELESERVICEID_HST_GAME_REPORT		0x19	/* 49158: HST :    ȸ */
#define IPC_TELESERVICEID_HST_MULTI_SMS			0x1A	/* 49160: HST : ĳʹٿε忡 Ѵ. */
															/* ĳʹٿε MOε . */
#define IPC_TELESERVICEID_LGT_CHARACTER_DLOAD	0x1B	/* 49154: LGT : ĳʹٿε忡 Ѵ. */
#define IPC_TELESERVICEID_LGT_CARD_ATA			0x1C	/* 49155: LGT : Card Data  */
#define IPC_TELESERVICEID_LGT_CARD_NOTIFY		0x1D	/* 49156: LGT : Card MT Notification */

 /* Touch Mail */
#define IPC_TELESERVICEID_TOUCH_MAIL_ACK		0x1E	/* 65400: SKT Touch Mail Acknowledgment Teleservice */
														/* used for an Acknowledgment
															before exchanging the Non-Text User Data */

#define IPC_TELESERVICEID_TOUCHMAIL_DATA		0x1F	/* 65401: SKT Touch Mail Data Teleservice */
#define IPC_TELESERVICEID_TOUCHMAIL_MAP		0x20	/* Reserved for the future : ൵  */
#define IPC_TELESERVICEID_TOUCHMAIL_CURPOS	0x21	/* Reserved for the future : ġ  */
#define IPC_TELESERVICEID_TOUCHMAIL_MSG		0x22	/* Reserved for the future */
  /* EMAIL and Chatting */
#define IPC_TELESERVICEID_EAMIL						0x23	/* Reserved for the future : Email Notify */
#define IPC_TELESERVICEID_CHAT_REQUEST		0x24	/* Reserved for the future : Chatting Request : ȭ ʴ */
#define IPC_TELESERVICEID_CHAT_REPLY			0x25	/* Reserved for the future : Chatting Reply : ȭ  */
  /* SKT Interactive Teleservice( Cyber Net Plus ) */
#define IPC_TELESERVICEID_CYBERNET_MENU_DLOAD	0x26	/* 65504: Menu Download */
#define IPC_TELESERVICEID_CYBERNET_LONG_DATA		0x27	/* 65516: Long Data Prompt and Response	*/
#define IPC_TELESERVICEID_CYBERNET_INIT_ACCESS		0x28	/* 65517: Initial Access */
#define IPC_TELESERVICEID_CYBERNET_MENU				0x29	/* 65518: Menu List Transfer / Selection */
#define IPC_TELESERVICEID_CYBERNET_CHAR				0x2A	/* 65519: Character Prompt / Response */
#define IPC_TELESERVICEID_CYBERNET_NUMERIC			0x2B	/* 65520: Numeric Prompt / Response */
#define IPC_TELESERVICEID_CYBERNET_CALL_BACK		0x2C	/* 65521: CallBack Prompt / Response */
#define IPC_TELESERVICEID_CYBERNET_TEXT				0x2D	/* 65522: Text Prompt  / Response */
#define IPC_TELESERVICEID_CYBERNET_INDICATION		0x2E	/* 65523: Indication Data */
#define IPC_TELESERVICEID_CYBERNET_SESSION_END	0x2F	/* 65524: Session Termination */
 /* GPS */
#define IPC_TELESERVICEID_GPS_SKT_INFORM			0x30	/* 65300: SKT ġ/  */
#define IPC_TELESERVICEID_GPS_STI_INFORM			0x31	/* Reserved for the future */
#define IPC_TELESERVICEID_GPS_KTF_INFORM			0x32	/* Reserved for the future */
#define IPC_TELESERVICEID_GPS_HST_INFORM			0x33	/* 49154: ܸ  Teleservice        */
#define IPC_TELESERVICEID_GPS_LGT_INFORM			0x34	/* Reserved for the future */
  /* Broadcast Teleservice */
#define IPC_TELESERVICEID_BC_CATEGORY				0x35	/* 49152: IS-637 Broadcast category */
#define IPC_TELESERVICEID_BC_PTP_CAT				0x36	/* 65500: IS-637 PTP Broadcast category(3 PCS carriers) */
#define IPC_TELESERVICEID_BC_CAT_REG				0x37	/* 65501: IS-637 Broadcast category reg(SKT) */
#define IPC_TELESERVICEID_BC_CAT_DEL				0x38	/* 65502: IS-637 Broadcast category del(SKT) */
#define IPC_TELESERVICEID_BC_CAT_ACT				0x39	/* 65503: Broadcast category activate(SKT) */
#define IPC_TELESERVICEID_BC_CAT_DEACT				0x3A	/* 65504: Broadcast category deactivate(SKT) */
#endif

//------------------------------------------------------------------------------
// IPC_SMSPARAMID_BCSERVICECATEGORY		0x02	/* Broadcast Service Category */
//------------------------------------------------------------------------------
// The Korean Wireless Service Carrier's do not use below Service Category's
// Below Service Category's depend on the Carriers.
#define IPC_CATEGORY_UNKNOWN				0x00
#define IPC_CATEGORY_EMERGENCY				0x01
#define IPC_CATEGORY_ADMIN					0x02
#define IPC_CATEGORY_MAINTENANCE			0x03
#define IPC_CATEGORY_GEN_NEWS_LOC			0x04
#define IPC_CATEGORY_GEN_NEWS_REG			0x05
#define IPC_CATEGORY_GEN_NEWS_NAT			0x06
#define IPC_CATEGORY_GEN_NEWS_INT			0x07
#define IPC_CATEGORY_FIN_NEWS_LOC			0x08
#define IPC_CATEGORY_FIN_NEWS_REG			0x09
#define IPC_CATEGORY_FIN_NEWS_NAT			0x0A
#define IPC_CATEGORY_FIN_NEWS_INT			0x0B
#define IPC_CATEGORY_SPT_NEWS_LOC			0x0C
#define IPC_CATEGORY_SPT_NEWS_REG			0x0D
#define IPC_CATEGORY_SPT_NEWS_NAT			0x0E
#define IPC_CATEGORY_SPT_NEWS_INT			0x0F
#define IPC_CATEGORY_ENT_NEWS_LOC			0x10
#define IPC_CATEGORY_ENT_NEWS_REG			0x11
#define IPC_CATEGORY_ENT_NEWS_NAT			0x12
#define IPC_CATEGORY_ENT_NEWS_INT			0x13
#define IPC_CATEGORY_LOC_WEATHER			0x14
#define IPC_CATEGORY_AREA_TRAFFIC			0x15
#define IPC_CATEGORY_AIRPORT_SCHED			0x16
#define IPC_CATEGORY_RESTAURANTS			0x17
#define IPC_CATEGORY_LODGINGS				0x18
#define IPC_CATEGORY_RETAILS				0x19
#define IPC_CATEGORY_ADS					0x1A
#define IPC_CATEGORY_STOCK_QUOTES			0x1B
#define IPC_CATEGORY_JOBS					0x1C
#define IPC_CATEGORY_MEDICAL				0x1D
#define IPC_CATEGORY_TECH_NEWS				0x1E
#define IPC_CATEGORY_MULTI					0x1F

//------------------------------------------------------------------------------
// IPC_SMSPARAMID_ADDRESS					0x03	/* Address */
//------------------------------------------------------------------------------

/* DIGIT MODE : CHARi[] filed 4bit dtmf code 8bit code ǹѴ.
*/
#define IPC_DIGITMODE_4BITDTMF			0x00
#define IPC_DIGITMODE_8BITCODE			0x01

/* NUMBER MODE
*/
#define IPC_NUMMODE_NONE_DATANETWORK	0x00	/* in ANSI TI.607 */
#define IPC_NUMMODE_DATANETWORK			0x01

/* NUMBER TYPE
*/
  /* The following are used when number mode is not data network address.
  */
#define IPC_NUMBER_TYPE_UNKNOWN			0x00
#define IPC_NUMBER_TYPE_INTERNATIONAL	0x01
#define IPC_NUMBER_TYPE_NATIONAL		0x02
#define IPC_NUMBER_TYPE_NETWORK			0x03
#define IPC_NUMBER_TYPE_SUBSCRIBER		0x04
#define IPC_NUMBER_TYPE_RESERVED_5		0x05
#define IPC_NUMBER_TYPE_ABREVIATED		0x06
#define IPC_NUMBER_TYPE_RESERVED_7		0x07
  /* The following are used only when number mode is data network address
       mode.
  */
#define IPC_NUMBER_TYPE_IP				0x01
#define IPC_NUMBER_TYPE_EMAILADDR		0x02

/* NUMBER PLAN
*/
#define IPC_NUMBER_PLAN_UNKNOWN			0x00
#define IPC_NUMBER_PLAN_TELEPHONY		0x01	/* CCITT E.164 and E.163,  including ISDN plan */
#define IPC_NUMBER_PLAN_RESERVED_2		0x02
#define IPC_NUMBER_PLAN_DATA			0x03	/* CCITT X.121 */
#define IPC_NUMBER_PLAN_TELEX			0x04	/* CCITT F.69 */
#define IPC_NUMBER_PLAN_RESERVED_5		0x05
#define IPC_NUMBER_PLAN_RESERVED_6		0x06
#define IPC_NUMBER_PLAN_RESERVED_7		0x07
#define IPC_NUMBER_PLAN_RESERVED_8		0x08
#define IPC_NUMBER_PLAN_PRIVATE			0x09
#define IPC_NUMBER_PLAN_RESERVED_10		0x0A
#define IPC_NUMBER_PLAN_RESERVED_11		0x0B
#define IPC_NUMBER_PLAN_RESERVED_12		0x0C
#define IPC_NUMBER_PLAN_RESERVED_13		0x0D
#define IPC_NUMBER_PLAN_RESERVED_14		0x0E
#define IPC_NUMBER_PLAN_RESERVED_15		0x0F

/* NUM_FIELDS
*/
/*		MAXLENGTH_SMS_ADDRESS			Maximum sms destination(or origination ) address /call back number */

/* CHARi
*/

//------------------------------------------------------------------------------
// IPC_SMSPARAMID_SUBADDRESS				0x04	/* Subaddress */
//------------------------------------------------------------------------------
/* SUB ADDRESS TYPE
*/
#define IPC_SUBADDR_NSAP				0x00	/* CCITT X.213 or ISO 8348 AD2 */
#define IPC_SUBADDR_USER_SPECIFIED		0x01	/* e.g. X.25 */

/* ODD  : ̰  ʿѰ ? phone  4bit ϸ Ǵ° ƴѰ ?
*/
  /* If the last CHARi field contains information only in the 4 most significant bits,
       the ODD field shall be set to '1'.  Otherwise, the ODD field shall be set to '0'.
*/
#define IPC_SUBADDR_EVEN				0x00
#define IPC_SUBADDR_ODD					0x01

/* NUM_FIELDS
*/

/* CHARi
*/

//------------------------------------------------------------------------------
// IPC_SMSPARAMID_BEARER_REPLY				0x05	/* Bearer Reply Option */
//------------------------------------------------------------------------------
/* REPLY_SEQ
*/
  // Range : 0 - 63
  //	 refer to MAXLENGTH_SMS_REPLY_SEQUENCE_NUMBER

//------------------------------------------------------------------------------
// IPC_SMSPARAMID_CAUSE_CODES				0x06	/* Cause Codes */
//------------------------------------------------------------------------------
/* REPLY_SEQ
*/
  // Range : 0 - 63
  // Refer to MAXLENGTH_SMS_REPLY_SEQUENCE_NUMBER

/* ERROR_CLASS
*/
#define IPC_ERRORCLASS_NONE					0x00	/* No Error */
#define IPC_ERRORCLASS_TEMP					0x02	/* Tempory Error */
#define IPC_ERRORCLASS_PERMANENT			0x03	/* Permanent Error */
#define IPC_ERRORCLASS_INTERNAL				0x04	/* Phone Internal Error */

/* CAUSE_CODE : defined in IS-41D
*/

  /* A. Network Problems:
  */
#define IPC_CAUSECODE_ADDRESS_VACANT							0
#define IPC_CAUSECODE_ADDRESS_TRANSLATION_FAILURE				1
#define IPC_CAUSECODE_NETWORK_RESOURCE_SHORTAGE					2
#define IPC_CAUSECODE_NETWORK_FAILURE							3
#define IPC_CAUSECODE_INVALID_TELESERVICE_ID					4
#define IPC_CAUSECODE_OTHER_NETWORK_PROBLEM						5
#define IPC_CAUSECODE_OTHER_NETWORK_PROBLEM_MORE_FIRST			6
/* all values within this range are treated as IPC_CAUSECODE_OTHER_NETWORK_PROBLEM_S	*/
#define IPC_CAUSECODE_OTHER_NETWORK_PROBLEM_MORE_LAST			31

/* B. Terminal Problems:
*/
#define IPC_CAUSECODE_NO_PAGE_RESPONSE							32
#define IPC_CAUSECODE_DESTINATION_BUSY							33
#define IPC_CAUSECODE_NO_ACK									34
#define IPC_CAUSECODE_DESTINATION_RESOURCE_SHORTAGE				35
#define IPC_CAUSECODE_SMS_DELIVERY_POSTPONED					36
#define IPC_CAUSECODE_DESTINATION_OUT_OF_SERVICE				37
#define IPC_CAUSECODE_DESTINATION_NO_LONGER_AT_THIS_ADDRESS		38
#define IPC_CAUSECODE_OTHER_TERMINAL_PROBLEM					39
#define IPC_CAUSECODE_OTHER_TERMINAL_PROBLEM_MORE_FIRST			40
/* all values within this range are treated as IPC_CAUSECODE_OTHER_TERMINAL_PROBLEM_S */
#define IPC_CAUSECODE_OTHER_TERMINAL_PROBLEM_MORE_LAST			47
#define IPC_CAUSECODE_SMS_DELIVERY_POSTPONED_MORE_FIRST			48
#define IPC_CAUSECODE_SMS_DELIVERY_POSTPONED_MORE_LAST			63

/* C. Radio Interface Problems:
*/
#define IPC_CAUSECODE_RADIO_IF_RESOURCE_SHORTAGE				64
#define IPC_CAUSECODE_RADIO_IF_INCOMPATIBLE						65
#define IPC_CAUSECODE_OTHER_RADIO_IF_PROBLEM					66
#define IPC_CAUSECODE_OTHER_RADIO_IF_PROBLEM_MORE_FIRST			67
/* all values within this range are treated as IPC_CAUSECODE_OTHER_RADIO_IF_PROBLEM */
#define IPC_CAUSECODE_OTHER_RADIO_IF_PROBLEM_MORE_LAST			95

/* D. General Problems:
*/
#define IPC_CAUSECODE_UNEXPECTED_PARM_SIZE						96
#define IPC_CAUSECODE_SMS_ORIGINATION_DENIED					97
#define IPC_CAUSECODE_SMS_TERMINATION_DENIED					98
#define IPC_CAUSECODE_SUPPL_SERVICE_NOT_SUPPORTED				99
#define IPC_CAUSECODE_SMS_NOT_SUPPORTED							100
#define IPC_CAUSECODE_RESERVED_101								101
#define IPC_CAUSECODE_MISSING_EXPECTED_PARM						102
#define IPC_CAUSECODE_MISSING_MANDATORY_PARM					103
#define IPC_CAUSECODE_UNRECOGNIZED_PARM_VALUE					104
#define IPC_CAUSECODE_UNEXPECTED_PARM_VALUE						105
#define IPC_CAUSECODE_USER_DATA_SIZE_ERROR						106
#define IPC_CAUSECODE_OTHER_GENERAL_PROBLEMS					107
#define IPC_CAUSECODE_OTHER_GENERAL_PROBLEMS_MORE_FIRST			108
/* all values within this range are treated as IPC_CAUSECODE_OTHER_GENERAL_PROBLEMS */
#define IPC_CAUSECODE_OTHER_GENERAL_PROBLEMS_MORE_LAST			255

/* following Codes are not defined in IS-41.
    if phone can not send the sms message internally,
     then error_class = 4 and cause_code is as follows :
*/

// status 

//ڻ 20030602
/*cause code for  success*/
#define IPC_SMS_STATUS_SEND_OK             									0

/*cause code for error*/
#define IPC_SMS_STATUS_WAITING_FOR_TL_ACK										1
#define IPC_SMS_STATUS_OUT_OF_RESOURCES  										2
#define IPC_SMS_STATUS_ACCESS_TOO_LARGE 										3
#define IPC_SMS_STATUS_DTC_TOO_LARGE     										4
#define IPC_SMS_STATUS_DTC_CONNECTED												5
#define IPC_SMS_STATUS_NETWORK_NOT_READY 										6
#define IPC_SMS_STATUS_NO_SVC																7
#define IPC_SMS_STATUS_PHONE_NOT_READY    									8
#define IPC_SMS_STATUS_NOT_ALLOWED_IN_AMPS									9
#define IPC_SMS_STATUS_CANNOT_SEND_BROADCAST								10
#define IPC_SMS_STATUS_INVALID_TRANSACTION_ID								11
//ڻ 20030602

//#define IPC_SMS_STATUS_L2ACK_RCVD					0x00
//#define IPC_SMS_STATUS_FAIL						0x01
//#define IPC_SMS_STATUS_FAIL_NO_SVC				0x02
//#define IPC_SMS_STATUS_FAIL_PHONE_BUSY			0x03
//#define IPC_SMS_STATUS_FAIL_OTHER					0x04


//------------------------------------------------------------------------------
// IPC_SMSPARAMID_MESSAGE_ID				0x07	/* Message Identifier */
//------------------------------------------------------------------------------
/* MESSAGE_TYPE
*/
#define IPC_MESSAGETYPE_DELIVER				0x01
#define IPC_MESSAGETYPE_SUBMIT				0x02
#define IPC_MESSAGETYPE_CANCEL				0x03
#define IPC_MESSAGETYPE_DELIVERY_ACK		0x04
#define IPC_MESSAGETYPE_USER_ACK			0x05

/* MESSAGE_ID
*/
// 2 bytes :    Range : 0 - 65535

//------------------------------------------------------------------------------
// IPC_SMSPARAMID_USER_DATA					0x08	/* User Data */
//------------------------------------------------------------------------------
/* MESSAGE_ENCODING
*/
#define IPC_MSGENCODING_UNSPECIFIED						0x00	/* OCTET */
#define IPC_MSGENCODING_IS91_EXTENDED_PROTOCOL			0x01	/* */
#define IPC_MSGENCODING_7BIT_ASCII						0x02	/* */
#define IPC_MSGENCODING_IA5								0x03	/* */
#define IPC_MSGENCODING_UNICODE							0x04	/* */
#define IPC_MSGENCODING_SHIFT_JIS						0x05	/* */
#define IPC_MSGENCODING_KSC5601							0x06	/* Korean */
#define IPC_MSGENCODING_HEBREW							0x07	/* ISO_8859_8 */
#define IPC_MSGENCODING_LATIN							0x08	/* ISO_8859_1 */
#define IPC_MSGENCODING_KSC5601_3PCS						0x10	/* Korean Standard */

/* NUM_FIELDS
*/
//	refer to
//           MAXLENGTH_SMS_MT_USER_DATA				255		// 229 Bytes in Qucalcomm Source
//		MAXLENGTH_SMS_MO_USER_DATA				255		// 229 Bytes in Qucalcomm Source


  /* SMS ߽Ž    ǵ user data ִ size */
//   refer to
//		MAXLENGTH_SMS_MO_USER_DATA_VERIZON				160
//		MAXLENGTH_SMS_MO_USER_DATA_SKT					80
//		MAXLENGTH_SMS_MO_USER_DATA_STI					100
//		MAXLENGTH_SMS_MO_USER_DATA_KTF					100
//		MAXLENGTH_SMS_MO_USER_DATA_HST					120
//		MAXLENGTH_SMS_MO_USER_DATA_LGT					120

/* CHARi
*/

//------------------------------------------------------------------------------
// IPC_SMSPARAMID_USER_RESPONSE_CODE		0x09	/* User Response Code */
//------------------------------------------------------------------------------
/* USER_RESPONSE_CODE
*/
// This field value depends on the Teleservices or the carrier
// This field had used in the Interactive Teleservices in Korea
// currently the wireless service carriers adopt the browser( WAP,etc )
// instead of SMS protocol in order to serve the interactive teleservice.

//------------------------------------------------------------------------------
// IPC_SMSPARAMID_MC_TIME_STAMP				0x0A	/* Message Center Time Stamp */
//------------------------------------------------------------------------------

/* YEAR
*/
// 4bit BCD Code,  example : if the year is 2002, the YEAR field contains 0x02.

/* MONTH
*/
// 1 ~ 12 ( in decimal )

/* DAY
*/
// 1 ~ 31( in decimal )

/* HOUR
*/
// 0 ~ 23 ( in decimal )

/* MINUTE
*/
// 0 ~ 59 ( in decimal )

/* SECOND
*/
// 0 ~ 59 ( in decimal )

//------------------------------------------------------------------------------
// IPC_SMSPARAMID_VALIDITY_PERIOD_ABS		0x0B	/* Validity Period - Absolute */
//------------------------------------------------------------------------------
// refer to IPC_SMSPARAMID_MC_TIME_STAMP

//------------------------------------------------------------------------------
// IPC_SMSPARAMID_VALIDITY_PERIOD_REL		0x0C	/* Validiry Period - Relative */
//------------------------------------------------------------------------------
/* VALIDITY
*/

//------------------------------------------------------------------------------
// IPC_SMSPARAMID_DEFERRED_DELIVERY_ABS		0x0D	/* Deferred Delivery Time - Absolute */
//------------------------------------------------------------------------------
// refer to IPC_SMSPARAMID_MC_TIME_STAMP

//------------------------------------------------------------------------------
// IPC_SMSPARAMID_DEFERRED_DELIVERY_REL		0x0E	/* Deferred Delivery Time - Relative */
//------------------------------------------------------------------------------
/* DELIVERY_TIME
*/

//------------------------------------------------------------------------------
// IPC_SMSPARAMID_PRIORITY					0x0F	/* Priority Indicator */
//------------------------------------------------------------------------------
#define IPC_PRIORITY_NORMAL					0x00
#define IPC_PRIORITY_INTERACTIVE			0x01
#define IPC_PRIORITY_URGENT					0x02
#define IPC_PRIORITY_EMERGENCY				0x03

//------------------------------------------------------------------------------
// IPC_SMSPARAMID_PRIVACY					0x10	/* Privacy Indicator */
//------------------------------------------------------------------------------
#define IPC_PRIVACY_NOT_RESTRICTED			0x00
#define IPC_PRIVACY_RESTRICTED				0x01
#define IPC_PRIVACY_CONFIDENTIAL			0x02
#define IPC_PRIVACY_SECRET					0x03

//------------------------------------------------------------------------------
// IPC_SMSPARAMID_REPLY_OPTION				0x11	/* Reply Option */
//------------------------------------------------------------------------------

// USER_ACK_REQ : user ack requested
#define IPC_SMS_USER_ACK_NOT_REQUESTED		0x00
#define IPC_SMS_USER_ACK_REQUESTED			0x01

// DAK_REQ : delivery ack requested
#define IPC_SMS_DELIVERY_ACK_NOT_REQUESTED	0x00
#define IPC_SMS_DELIVERY_ACK_REQUESTED		0x01

//------------------------------------------------------------------------------
// IPC_SMSPARAMID_NUMBER_OF_MESSAGE			0x12	/* Number of Messages : Voice Mail Count */
//------------------------------------------------------------------------------
/* MESSAGE_COUNT : voice mail count ( 0 - 99 )
*/
// refer to		MAXLENGTH_SMS_VOICE_MAIL_COUNT		99

//------------------------------------------------------------------------------
// IPC_SMSPARAMID_ALERT_ON_DELIVERY			0x13	/* Alert on Message Delivery */
//------------------------------------------------------------------------------
// IS-637B Feature : not yet adopted
//		IPC_ALERT_PRIORITY_DEFAULT			0x00	/* mobile default */
//		IPC_ALERT_PRIORITY_LOW				0x01	/* low priority alert: This alert is defined by the mobile station */
//		IPC_ALERT_PRIORITY_MED				0x02	/* medium priority alert : This alert is defined by the mobile station */
//		IPC_ALERT_PRIORITY_HIGH				0x03	/* high priority alert : This alert is defined by the mobile station */

//------------------------------------------------------------------------------
// IPC_SMSPARAMID_LANGUAGE					0x14	/* Langauge Indicator */
//------------------------------------------------------------------------------
#define IPC_LANGUAGE_UNKNOWN				0x00
#define IPC_LANGUAGE_ENGLISH				0x01
#define IPC_LANGUAGE_FRENCH					0x02
#define IPC_LANGUAGE_SPANISH				0x03
#define IPC_LANGUAGE_JAPANESE				0x04
#define IPC_LANGUAGE_KOREAN					0x05
#define IPC_LANGUAGE_CHINESE				0x06
#define IPC_LANGUAGE_HEBREW					0x07
#define IPC_LANGUAGE_KOREAN1				0x40	/* Used in korean 3 PCS's and STI */
#define IPC_LANGUAGE_KOREAN_SKT				0xFE	/* Used in only SKT  */

//------------------------------------------------------------------------------
// IPC_SMSPARAMID_CALLBACK					0x15	/* Call Back Number */
//------------------------------------------------------------------------------
/* DIGIT_MODE : CHARi[] filed 4bit dtmf code 8bit code ǹѴ.
*/
// refer to :
//		IPC_DIGITMODE_4BITDTMF			0x00
//		IPC_DIGITMODE_8BITCODE			0x01

/* NUMBER_TYPE
*/
// refer to :
//		IPC_NUMBER_TYPE_UNKNOWN			0x00
//		IPC_NUMBER_TYPE_INTERNATIONAL		0x01
//		IPC_NUMBER_TYPE_NATIONAL			0x02
//		IPC_NUMBER_TYPE_NETWORK			0x03
//		IPC_NUMBER_TYPE_SUBSCRIBER		0x04
//		IPC_NUMBER_TYPE_RESERVED_5		0x05
//		IPC_NUMBER_TYPE_ABREVIATED		0x06
//		IPC_NUMBER_TYPE_RESERVED_7		0x07

/* NUMBER_PLAN
*/
// refer to :
//		IPC_NUMBER_PLAN_UNKNOWN			0x00
//		IPC_NUMBER_PLAN_TELEPHONY			0x01	/* CCITT E.164 and E.163,  including ISDN plan */
//		IPC_NUMBER_PLAN_RESERVED_2		0x02
//		IPC_NUMBER_PLAN_DATA				0x03	/* CCITT X.121 */
//		IPC_NUMBER_PLAN_TELEX				0x04	/* CCITT F.69 */
//		IPC_NUMBER_PLAN_RESERVED_5		0x05
//		IPC_NUMBER_PLAN_RESERVED_6		0x06
//		IPC_NUMBER_PLAN_RESERVED_7		0x07
//		IPC_NUMBER_PLAN_RESERVED_8		0x08
//		IPC_NUMBER_PLAN_PRIVATE			0x09
//		IPC_NUMBER_PLAN_RESERVED_10		0x0A
//		IPC_NUMBER_PLAN_RESERVED_11		0x0B
//		IPC_NUMBER_PLAN_RESERVED_12		0x0C
//		IPC_NUMBER_PLAN_RESERVED_13		0x0D
//		IPC_NUMBER_PLAN_RESERVED_14		0x0E
//		IPC_NUMBER_PLAN_RESERVED_15		0x0F

/* NUM_FIELDS
*/
// refer to :
//		MAXLENGTH_SMS_ADDRESS			32

/* CHARi
*/

//------------------------------------------------------------------------------
// IPC_SMSPARAMID_DISPLAY_MODE				0x16	/* Display Mode */
//------------------------------------------------------------------------------
// IS-637B Feature : not yet adopted
#define	IPC_DISPLSY_MODE_IMMEDIATE			0x00	/* The mobile station is to display the */
//													/* received message as soon as possible.*/
#define	IPC_DISPLSY_MODE_MOBILEDEFAULT		0x01	/* The mobile station is to display the received */
//												/* message based on a pre-defined mode in the mobile station.*/
#define	IPC_DISPLSY_MODE_USERINVOKE			0x02	/* The mobile station is to display the received */
//												/*message based on the mode selected by the user.*/
#define	IPC_DISPLSY_MODE_RESERVED			0x03	/*Reserved*/

//------------------------------------------------------------------------------
// IPC_SMSPARAMID_MULTI_ENCODING_USER_DATA	0x17	/* Multiply Encoding User Data */
//------------------------------------------------------------------------------
// refer to :
//          IPC_SMSPARAMID_USER_DATA
//		MAXLENGTH_SMS_MULTI_ENCODING			10		/* ӽ÷, ߿  define */


 /*====================== END OF CDMA SMS PARAMETER ==========================*/
//*********************************************************************************


/*********************************************************************************

                                            Sub Command of IPC_SEC_CMD[0x05]

**********************************************************************************/
typedef enum{
  IPC_SEC_PIN_STATUS=0x01,            /* 0x01 : PIN Status Message */
  IPC_SEC_PHONE_LOCK,                    /* 0x02 : Phone Lock Message */
  IPC_SEC_CHANGE_LOCKING_PW,     /* 0x03 : Change Locking Password Message */
  IPC_SEC_SIM_LANG,                        /* 0x04 : SIM Language Message */
  IPC_SEC_RSIM_ACCESS,                  /* 0x05 : +CRSM, Restricted SIM Access Message */
  IPC_SEC_GSIM_ACCESS,                  /* 0x06 : +CSIM, General SIM Access Message */             
  IPC_SEC_SIM_ICC_TYPE,                 /* 0x07 : SIM ICC Type Message*/
  IPC_SEC_LOCK_INFOMATION,              /* 0x08 : Lock Information Message*/
  IPC_SEC_IMS_AUTH,                     /* 0x09 : IMS Authentication Message*/
  IPC_SEC_RUIM_CONFIG,               /* 0x0A : RUIM Configuration Notification Message*/  // jilee_cd23
  IPC_SEC_MAX
} ipc_sec_sub_cmd_type; 

/*********************************************************************************/


/*=================================================================

   SUB_CMD(1) : IPC_SEC_PIN_STATUS      0x01                  Security PIN Status Message
   
=================================================================*/


/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_CFRM            0x04     */
/*    IPC_CMD_EVENT           0x05     */
/*    IPC_CMD_INDI              0x01     */
/* none */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
   Ask for the phone locking status information. : +CPIN?
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
   Enter the PIN message. +CPIN=xxx,xxx
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | LOCK_TYPE(1) | PIN_LEN(1) | PUK_LEN(1) | 
-------------------------------------------------------------------------------------
 | PIN (x) | PUK (x) | 
-------------------------------------------------------------------------------------
*/

/* LOCK_TYPE  field */
/* The Lock Type of the ME */
typedef enum{
  IPC_SEC_LOCK_TYPE_READY,   /* 0x00 : ME is not locked */
  IPC_SEC_LOCK_TYPE_PS,          /* 0x01 : PH-SIM, Lock Phone to SIM/UICC card 
                                                       ( MT asks password when other than 
                                                         current SIM/UICC card inserted; 
                                                         MT may remember certain amount of 
                                                         previously used cards thus not 
                                                         requiring password when they are inserted ) */ 
  IPC_SEC_LOCK_TYPE_PF,      /* 0x02 : PH-FSIM, Lock Phone to the very First 
                                                         inserted SIM/UICC card ( MT asks password
                                                 when other than the first SIM/UICC card is inserted ) */
  IPC_SEC_LOCK_TYPE_SC,      /* 0x03 : Lock SIM/UICC card ( SIM asks password in ME
                                                     power-up and when this command is issued ) */
  IPC_SEC_LOCK_TYPE_FD,      /* 0x04 : SIM card or active application in the UICC
                                                      (GSM or USIM) fixed dialing memory feature */
  IPC_SEC_LOCK_TYPE_PN,      /* 0x05 : Network Personalization */
  IPC_SEC_LOCK_TYPE_PU,      /* 0x06 : Network subset Personalization */
  IPC_SEC_LOCK_TYPE_PP,      /* 0x07 : Service Provider Personalization */
  IPC_SEC_LOCK_TYPE_PC,      /* 0x08 : Corporate Personalization */
  IPC_SEC_LOCK_TYPE_SC2,     /* 0x09 : Lock PIN2 ( ... ) */
  IPC_SEC_LOCL_TYPE_PUK2,	  /* 0x0A : Lock PUK2 (... ) */
  IPC_SEC_LOCK_TYPE_ACL,	 /*0x0B: ACL */
  
  /* 0x0a - 0x7F are Reserved */
  
  /* SIM Status Information which are used only
  ** in IPC_SEC_PIN_STATUS Noti 
  */
  IPC_SEC_LOCK_TYPE_NO_SIM=0x80,  /* 0x80 : SIM is not inserted */
  IPC_SEC_LOCK_TYPE_UNAVAIL,      /* 0x81 : SIM is inserted but can not communicate
                                          with SIM ( SIM interface error ) */
  IPC_SEC_SIM_INIT_COMPLETED,   /* 0x82 : SIM Initialize Completed */  // JBG 2005.05.18
  IPC_SEC_PB_INIT_COMPLETED,   /* 0x83 : Phonebook Initialize Completed*/
  
  IPC_SEC_LOCK_TYPE_MAX
}ipc_sec_lock_e_type;

/* PIN_LEN  field */
/* PIN[] length */

/* PUK_LEN  field */
/* PUK[] length */

/* PIN[]  field */
/* Personal Identification Number
 ** see MAX_SEC_PIN_LEN
*/

/* PUK[]  field */
/* PIN Unblocking Key
 ** see MAX_GSM_PUK_LEN
*/



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
    PIN Status Response
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | LOCK_TYPE(1) | LOCK_KEY(1) |
-------------------------------------------------------------------------------------
*/


/* LOCK_TYPE */
/* see ipc_sec_lock_e_type */

/* LOCK_KEY */
typedef enum{
  IPC_SEC_LOCK_KEY_UNLOCKED,  /* 0x00 : Not necessary */
  IPC_SEC_LOCK_KEY_PIN,             /* 0x01 : PIN required as a password */
  IPC_SEC_LOCK_KEY_PUK,            /* 0x02 : PUK required as a password */
  IPC_SEC_LOCK_KEY_PIN2,           /* 0x03 : PIN2 required as a password */
  IPC_SEC_LOCK_KEY_PUK2,           /* 0x04 : PUK2 required as a password */
  IPC_SEC_LOCK_KEY_PERM_BLOCKED,  /* 0x05 : PIN Permanent Blocked */   // JBG 2005.05.18
  IPC_SEC_LOCK_KEY_PIN2_DISABLE,    /* 0x06 : PIN2 Lock Disabled*/
  IPC_SEC_LOCK_KEY_MAX
} ipc_sec_lock_key_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                        0x03

 DESCRIPTION :
     PIN Status Notification
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | LOCK_TYPE(1) | LOCK_KEY(1) |
-------------------------------------------------------------------------------------
*/

/* LOCK_TYPE  field */
/* see ipc_sec_lock_e_type */

/* LOCK_KEY  field */
/* see ipc_sec_lock_key_e_type */


/*=================================================================*/



/*=================================================================

   IPC_SEC_PHONE_LOCK      0x02                      Security Phone Lock Message
   
=================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_CFRM         0x04     */
/*    IPC_CMD_EVENT             0x05     */
/*    IPC_CMD_INDI               0x01     */
/*    IPC_CMD_NOTI              0x03     */
/* none */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
  +CLCK="LOCK_TYPE",2
  Get  Phone Lock Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | LOCK_TYPE(1) |
-------------------------------------------------------------------------------------
*/

/* LOCK_TYPE  field */
/* see ipc_sec_lock_e_type */




/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
  Set  Phone Lock Message
   Activate or deactivate the phone lock in the LOCK_TYPE.
   : +CLCK="LOCK_TYPE",LOCK_MODE,"PASSWORD" 
  
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | LOCK_TYPE(1) | LOCK_MODE(1) | PWD_LEN(1) |
-------------------------------------------------------------------------------------
 | PWD (x)  |
-------------------------------------------------------------------------------------
*/

/* LOCK_TYPE  field */
/* see  ipc_sec_lock_e_type  */

/* LOCK_MODE  field */
/* The activation or deactivation of personalization */
typedef enum{
  IPC_SEC_LOCK_MODE_UNLOCK, /* 0x00 : Disable / Depersonalization */
  IPC_SEC_LOCK_MODE_LOCK,     /* 0x01 : Enable/ Personalization */
  IPC_SEC_LOCK_MODE_MAX
} ipc_sec_lock_mode_e_type;

/* PWD_LEN  field */
/* The length of PW */

/* PWD  field */
 /* see MAX_SEC_PHONE_LOCK_PW_LEN */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
  Phone Lock Response Message 
  
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | LOCK_TYPE(1) | LOCK_MODE(1) |
-------------------------------------------------------------------------------------
*/

/* LOCK_TYPE */
/* see ipc_sec_lock_e_type */

/* LOCK_MODE */
/* see ipc_sec_lock_mode_e_type */



/*=================================================================*/



/*=================================================================

   SUB_CMD(1) : IPC_SEC_CHANGE_LOCKING_PW      0x03   Security Change Locking Password Message
   
=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_CFRM       0x04     */
/*    IPC_CMD_EVENT           0x05     */
/*    IPC_CMD_INDI              0x01     */
/*    IPC_CMD_RESP             0x02     */
/*    IPC_CMD_NOTI             0x03     */
/* none */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
   Set Change Locking Password
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | LOCK_TYPE(1) |
-------------------------------------------------------------------------------------
 | PWD_LEN(1) | NEW_PWD_LEN(1) | PWD (x) | NEW_PWD (x) |
-------------------------------------------------------------------------------------
*/

/* LOCK_TYPE */
/* Password ϰ ϴLock  */
/* see ipc_sec_lock_e_type */

/* PWD_LEN */
/* PWD[ ] ( password ) length */

/* NEW_PWD_LEN */
/* NEW_PWD[ ] ( new password ) length */

/* PWD[] */
 /* see MAX_SEC_PHONE_LOCK_PW_LEN */

/* NEW_PWD[] */
/* see MAX_SEC_PHONE_LOCK_PW_LEN */



/*=================================================================*/



/*=================================================================

   SUB_CMD(1) : IPC_SEC_SIM_LANG    0x04               Security SIM Language Message
   
=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_SET                  0x03     */
/*    IPC_CMD_CFRM          0x04     */
/*    IPC_CMD_EVENT              0x05     */
/*    IPC_CMD_INDI                 0x01     */
/*    IPC_CMD_NOTI		0x03     */
/* none */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
      SIM Language GET  
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
  SIM Language Response
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | LANGUAGE(1) |
-------------------------------------------------------------------------------------
*/

/* LANGUAGE  field */
/* the language of the SIM */
typedef enum{
  IPC_SEC_LANG_SWEDISH=0x01,  /* 0x01 : Language for Sweden */
  IPC_SEC_LANG_FINNISH,       /* 0x02 : Language for Finland */
  IPC_SEC_LANG_DANISH,        /* 0x03 : Language for Finland */
  IPC_SEC_LANG_NORWEGIAN,     /* 0x04 : Language for Norway */
  IPC_SEC_LANG_GERMAN,        /* 0x05 : Language for Germany */
  IPC_SEC_LANG_FRENCH,        /* 0x06 : Language for France */
  IPC_SEC_LANG_SPANISH,       /* 0x07 : Language for Spain */
  IPC_SEC_LANG_ITALIAN,       /* 0x08 : Language for Italy */
  IPC_SEC_LANG_ENGLISH,       /* 0x09 : Language for USA or England */
  IPC_SEC_LANG_MAX
} ipc_sec_lang_e_type; 


// JBG 2005.05.20 begin
/*=================================================================*/



/*=================================================================

   SUB_CMD(1) : IPC_SEC_RSIM_ACCESS   0x05    Security Restricted SIM Access Message
   
=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_SET                0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT           0x05     */
/*    IPC_CMD_INDI              0x01     */
/*    IPC_CMD_NOTI             0x03     */
/* none */




/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
   Get Security Restricted SIM Access Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | ACCESS_CMD(1) | FILE_ID(2) |
-------------------------------------------------------------------------------------
 | P1(1) | P2(1) | P3 (1) | DATA (x) |
-------------------------------------------------------------------------------------
*/

/* ACCESS_CMD  Field : see  3GPP TS11.11, TS27.07 */
typedef enum {   
    IPC_SEC_SIM_ACC_READ_BINARY=0xB0,      /*Reads a string of bytes from the current transparent EF.*/
    IPC_SEC_SIM_ACC_READ_RECORD=0xB2,      /*Reads one complete record in the current linear fixed or cyclic EF.*/
    IPC_SEC_SIM_ACC_GET_RESPONSE=0xC0,    /*The response data depends on the preceding command.*/
    IPC_SEC_SIM_ACC_UPDATE_BINARY=0xD6,  /*Updates the current transparent EF with a string of bytes.*/
    IPC_SEC_SIM_ACC_UPDATE_RECORD=0xDC,  /*Updates one complete record in the current linear fixed or cyclic EF.*/
    IPC_SEC_SIM_ACC_STATUS=0xF2,  /*Information concerning the current directory.*/
    IPC_SEC_SIM_ACC_MAX
} ipc_sec_sim_access_e_type;

/* FILE_ID field */

/* P1 Field : see  3GPP TS11.11, TS27.07 */

/* P2 Field : see  3GPP TS11.11, TS27.07 */

/* P3 Field : see  3GPP TS11.11, TS27.07 */

/* DATA field : see  3GPP TS11.11, TS27.07 */
/* Maximum Length : see  MAX_SEC_SIM_DATA_STRING*/



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   Security Restricted SIM Access Response Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SW1(1) | SW2(1) |
-------------------------------------------------------------------------------------
| RESPONSE_LEN(1) | RESPONSE(x) |
-------------------------------------------------------------------------------------
*/

/* SW1 Field : Status Word1, 3GPP TS11.11, TS27.07 */

/* SW2 Field : Status Word2, 3GPP TS11.11, TS27.07 */

/* RESPONSE Field : : 3GPP TS11.11, TS27.07 */
/* Maximum Length : see  MAX_SEC_SIM_DATA_STRING*/


/*=================================================================

   SUB_CMD(1) : IPC_SEC_GSIM_ACCESS   0x06    Security General SIM Access Message
   
=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_SET               0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT           0x05     */
/*    IPC_CMD_INDI              0x01     */
/*    IPC_CMD_NOTI             0x03     */
/* none */




/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
   Get Security General SIM Access Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | ACCESS_CMD(1) |
-------------------------------------------------------------------------------------
 | DATA_LEN(1) | DATA (x) |
-------------------------------------------------------------------------------------
*/

/* ACCESS_CMD field */
/* see  ipc_sec_sim_access_e_type  */

/* DATA_LEN Field : see 3GPP TS11.11, TS27.07 */

/* DATA field : see 3GPP TS11.11, TS27.07 */
/* Maximum Length : see  MAX_SEC_SIM_DATA_STRING*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   Security General SIM Access Response Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | RESPONSE_LEN(1) | RESPONSE(x) |
-------------------------------------------------------------------------------------
*/

/* RESPONSE_LEN Field : see 3GPP TS11.11, TS27.07 */

/* RESPONSE field : see 3GPP TS11.11, TS27.07 */
/* Maximum Length : see  MAX_SEC_SIM_DATA_STRING*/



/*=================================================================*/
// JBG 2005.05.20 end



/*=================================================================

   SUB_CMD(1) : IPC_SEC_SIM_ICC_TYPE   0x07    SIM ICC Type Message
   
=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_SET               0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT           0x05     */
/*    IPC_CMD_INDI              0x01     */
/* none */





/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
   Get Current SIM ICC Type Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) |CMD_TYPE(1) |
-------------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   Get Current SIM ICC Type Response Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |ICC_TYPE (1)|
-------------------------------------------------------------------------------------
*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                        0x03

 DESCRIPTION :
     Get Current SIM ICC Type  Notification
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | ICC_TYPE (1) | 
-------------------------------------------------------------------------------------
*/


/* ICC TYPE field */
typedef enum{
  IPC_SEC_SIM_UNKNOWN,		  /* 0x00 : UNKNOWN */
  IPC_SEC_SIM_2GGSM,          /* 0x01 : 2G GSM */
  IPC_SEC_SIM_3G,		      /* 0x02 : 3G*/
  IPC_SEC_SIM_RUIM,           /* 0x03 : RUIM */  // jilee_cd23 
  IPC_SEC_SIM_ICC_MAX
} ipc_sec_sim_icc_e_type; 


/*=================================================================

   SUB_CMD(1) : IPC_SEC_LOCK_INFOMATION   0x08    Lock Information Message
   
=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_SET               0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT           0x05     */
/*    IPC_CMD_INDI              0x01     */
/*    IPC_CMD_NOTI             0x03     */
/* none */





/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
   Get Lock Information Message
   Lock Type, Lock Key, Number of Retry..
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) |CMD_TYPE(1) |NUM_LOCK_TYPE (1)| LOCK_TYPE(X)|
-------------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   Get Lock Information Response Message
   Lock Type, Lock Key, Number of Retry..
 FORMAT :
-----------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |NUM_LOCK_TYPE(1) | LOCK_INFO_ELEMENT(X)|
-----------------------------------------------------------------------------------

LOCK_INFO_ELEMENT contains as below

------------------------------------------
|LOCK_TYPE (1)|LOCK_KEY (1)|NUM_RETRY (1)|
------------------------------------------
*/


/*=================================================================

   SUB_CMD(1) : IPC_SEC_IMS_AUTH   0x09    IMS Authentication Message
   
=================================================================*/

/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_RESP             0x02     */
/* none */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
   Get IMS Authentication Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) |CMD_TYPE(1) |AUTH_CMD[] (X)|
-------------------------------------------------------------------------------------*/
/*
AUTH_CMD[] (X) contains as below
-------------------------------------------------------------------------------------
 | RAND_LEN (1) | RAND(RAND_LEN) | AUTN_LEN(1) | AUTN(AUTN_LEN) |
-------------------------------------------------------------------------------------
Max Authentication context 512
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   Get IMS Authentication Response Message
 FORMAT :
-----------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) |CMD_TYPE(1) |AUTH_RESULT (1) | AUTH_RES[] (X) |
-----------------------------------------------------------------------------------*/
/*
AUTH_RES[] (X) contains as below
-------------------------------------------------------------------------------------
 | RES_LEN (1) | RES(RES_LEN) | AUTS_LEN(1) | AUTS(AUTS_LEN) | CK_LEN (1) | CK_KEY( CK_LEN)|
-------------------------------------------------------------------------------------
 | IK_LEN (1) | IK_KEY(IK_LEN) |
-------------------------------------------------------------------------------------
Max Authentication context 512
*/

typedef enum{
   IPC_SEC_IMS_AUTH_NO_ERR,           /* 0x00 : IMS Authentication No Error*/
   IPC_SEC_IMS_AUTH_CANNOT_PERFORM,   /* 0x01 : IMS Authentication Cannot Perform */
   IPC_SEC_IMS_AUTH_SKIP_RESPONSE,    /* 0x02 : IMS Authentication Skip Response */
   IPC_SEC_IMS_AUTH_MAK_CODE_FAILURE,	/* 0x03 : IMS Authentication MAK_CODE_FAILURE */
   IPC_SEC_IMS_AUTH_SQN_FAIL,              	/* 0x04 : IMS Authentication SQN Failure*/
   IPC_SEC_IMS_AUTH_MAX
} ipc_sec_ims_auth_result_e_type;


/*=================================================================*/


/*********************************************************************************

                                            Sub Command of IPC_PB_CMD[0x06]

**********************************************************************************/
typedef enum{
  IPC_PB_ACCESS=0x01,     /* 0x01 : PhoneBook Access Message */
  IPC_PB_STORAGE,            /* 0x02 : PhoneBook Storage Message */
  IPC_PB_STORAGE_LIST,  /* 0x03 : PhoneBook Storage List Message */
  IPC_PB_ENTRY_INFO,      /* 0x04 : PhoneBook Entry Information Message */
  IPC_PB_3GPB_CAPA,      /* 0x05 : 3G Phonebook Capability Message */
  IPC_PB_MAX
} ipc_pb_sub_cmd_type;

/*********************************************************************************/


/*================================================================/

   SUB_CMD(1) : IPC_PB_ACCESS      0x01               PhoneBook Access Message

=================================================================*/

/*    IPC_CMD_SET                      0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT           0x05     */
/*    IPC_CMD_INDI              0x01     */
/* none */




/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_EXEC                        0x01

 DESCRIPTION :
  
 FORMAT :
-------------------------------------------------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SELECTED_SLOT(1) | MODE(1) | 
-------------------------------------------------------------------------------------
| PB_STORAGE_TYPE(1) | INDEX(2) | SUB_PARAMETERS(x) |
-------------------------------------------------------------------------------------

if PB_STORAGE_TYPE == PB_EN or PB_FD or PB_LD or PB_ON or PB_SDN, SUB_PARAMETERS are as follows
-------------------------------------------------------------------------------------
 | NUMBER_LEN(1) | NUMBER_TYPE(1) | NUMBER (y) | TEXT_LEN(1) | TEXT_ENCTYPE(1) | TEXT (z) |
-------------------------------------------------------------------------------------
* y <= 256
* z <= 256

if PB_STORAGE_TYPE ==  PB_ME or PB_3GSIM, SUB_PARAMETERS are as follows
-------------------------------------------------------------------------------------
| PB_FIELD_TYPE(1) |PB_FIELD_LENGTH(2) | PB_FIELD_DATA_TYPE(1) | PB_FIELD_DATA(X) |
-------------------------------------------------------------------------------------
|IPC_PB_FIELD_END (1) |
-------------------------------------------------------------------------------------
* above data structuresexecpt IPC_PB_FIELD_END are repeated 
* X <= 256

*PB_FIELD_DATA_TYPE field
*ipc_pb_field_e_type


*IPC_PB_FIELD_END
*0xFF

if PB_STORAGE_TYPE  == PB_DC or PB_MC or PB_RC or PB_ICI or PB_OCI, SUB_PARAMETERS are as follows
--------------------------------------------------------------------------------------
 | NUMBER_LEN(1) | NUMBER_TYPE(1) | NUMBER(X) | TEXT_LEN(1) | TEXT_ENCTYPE(1) | TEXT(X) |
--------------------------------------------------------------------------------------
 |DATE_TIME(7) |CALL_DURATION(3) | CALL_STATUS(1) | LINK_ENTRY(3) |
--------------------------------------------------------------------------------------
*X <= 256

if PB_STORAGE_TYPE == PB_AAS or PB_GAS , SUB_PARAMETERS are as follows
--------------------------------------------------------------------------------------
 | LEN_ALPHA_STRING(1) | DATA_TYPE(1) | ALPHA_STRING(x)
--------------------------------------------------------------------------------------
*X <= 256
*/

#define IPC_PB_FIELD_END              	0xff
#define ADDRESS_FIELDS_COUNT		13
#define IPC_PB_UPBOOK_ENTRY_MAX	 800

typedef enum{
  IPC_PB_SLOT_1 = 0x01,       /* 0x01 : Slot1 */
  IPC_PB_SLOT_2
} ipc_pb_slot_id_e_type;

typedef enum{
  IPC_TEXT_ENC_ASCII = 0x01,       /* 0x01 : ASCII   */
  IPC_TEXT_ENC_GSM7BIT,            /* 0x02 : GSM7BIT */
  IPC_TEXT_ENC_UCS2,               /* 0x03 : UCS2 */
  IPC_TEXT_ENC_HEX                 /* 0x04 : HEX */
} ipc_text_enc_e_type;


/* MODE  field */
typedef enum{
  IPC_PB_MODE_ADD=0x01,       /* 0x01 */
  IPC_PB_MODE_DEL,                 /* 0x02 */
  IPC_PB_MODE_EDIT,               /* 0x03 */
  IPC_PB_MODE_DELETE_ALL,          	/* 0x04 */
  IPC_PB_MODE_MAX
} ipc_pb_acc_mode_e_type;

/*PB_FIEDL TYPE field*/
typedef enum{
	IPC_PB_FIELD_3GPP_NAME = 0x01,		/*0x01 : Name of current storage*/
	IPC_PB_FIELD_3GPP_NUMBER,			/*0x02 : Number of current storage*/
	IPC_PB_FIELD_3GPP_ANR,				/*0x03 : First additional number*/
	IPC_PB_FIELD_3GPP_EMAIL,				/*0x04 : First email address*/
	IPC_PB_FIELD_3GPP_SNE,				/*0x05 : Second name entry*/
	IPC_PB_FIELD_3GPP_GRP,				/*0x06 : Grouping file*/
	IPC_PB_FIELD_3GPP_PBC,				/*0x07 : Phonebook control*/
	IPC_PB_FIELD_3GPP_ANRA,				/*0x08 : Second additional number*/
	IPC_PB_FIELD_3GPP_ANRB,				/*0x09 : Third additional number*/
	IPC_PB_FIELD_3GPP_ANRC,				/*0x0A : Fourth additional number*/
	IPC_PB_FIELD_3GPP_EMAILA,			/*0x0B : Second email address*/
	IPC_PB_FIELD_3GPP_EMAILB,			/*0x0C : Third email address*/
	IPC_PB_FIELD_3GPP_EMAILC,			/*0x0D : Fourth email address*/
	IPC_PB_FIELD_ME_FIRSTNAME = 0x80,		/*0x80 : used in ME Phonebook*/
	IPC_PB_FIELD_ME_MIDDLENAME,			/*0x81 : used in ME Phonebook*/
	IPC_PB_FIELD_ME_LASTNAME,			/*0x82 : used in ME Phonebook*/
	IPC_PB_FIELD_ME_NICKNAME,			/*0x83 : used in ME Phonebook*/
	IPC_PB_FIELD_ME_NAME,				/*0x84 : used in ME Phonebook*/
	IPC_PB_FIELD_ME_ORG,					/*0x85 : used in ME Phonebook*/
	IPC_PB_FIELD_ME_JOBTITLE,				/*0x86 : used in ME Phonebook*/
	IPC_PB_FIELD_ME_ADDRESS,				/*0x87 : used in ME Phonebook*/
	IPC_PB_FIELD_ME_CITY,					/*0x88 : used in ME Phonebook*/
	IPC_PB_FIELD_ME_STATE,				/*0x89 : used in ME Phonebook*/
	IPC_PB_FIELD_ME_COUNTRY,			/*0x8A : used in ME Phonebook*/
	IPC_PB_FIELD_ME_ZIPCODE,				/*0x8B : used in ME Phonebook*/
	IPC_PB_FIELD_ME_URL,					/*0x8C : used in ME Phonebook*/
	IPC_PB_FIELD_ME_DEPARTMENT,			/*0x8D : used in ME Phonebook*/
	IPC_PB_FIELD_ME_EMAIL,				/*0x8E : used in ME Phonebook*/
	IPC_PB_FIELD_ME_GENDER,				/*0x8F : used in ME Phonebook*/
	IPC_PB_FIELD_ME_PHONE_WORK,		/*0x90 : used in ME Phonebook*/
	IPC_PB_FIELD_ME_PHONE_HOME,			/*0x91 : used in ME Phonebook*/
	IPC_PB_FIELD_ME_PHONE_FAX,			/*0x92 : used in ME Phonebook*/
	IPC_PB_FIELD_ME_PHONE_VOICE,		/*0x93 : used in ME Phonebook*/
	IPC_PB_FIELD_ME_PHONE_PREF,			/*0x94 : used in ME Phonebook*/
	IPC_PB_FIELD_ME_PHONE_CELL,			/*0x95 : used in ME Phonebook*/
	IPC_PB_FIELD_ME_PHONE_PAGER,		/*0x96 : used in ME Phonebook*/
	IPC_PB_FIELD_ME_PHONE_GENERIC,		/*0x97 : used in ME Phonebook*/
	IPC_PB_FIELD_ME_PHONE_OTHER,		/*0x98 : used in ME Phonebook*/
	IPC_PB_FIELD_ME_BDAY,				/*0x99 : used in ME Phonebook*/
	IPC_PB_FIELD_ME_SORTSTRING,			/*0x9A : used in ME Phonebook*/
	IPC_PB_FIELD_ME_NOTES,				/*0x9B : used in ME Phonebook*/
	IPC_PB_FIELD_ME_SIP_URL,				/*0x9C : used in ME Phonebook*/
	IPC_PB_FIELD_ME_DIAL_STRING,			/*0x9D : used in ME Phonebook*/
	IPC_PB_FIELD_MAX		
}ipc_pb_field_e_type;

/* NUMBER_TYPE field */
typedef enum{
  IPC_PB_NUMBER_UNKNOWN,  			/* 0x00 : unknown */
  IPC_PB_NUMBER_INTERNATIONAL,  	/* 0x01 : International Number */
  IPC_PB_NUMBER_NATIONAL,  			/* 0x02 : national Number */
  IPC_PB_NUMBER_NETWORK,  			/* 0x03 : network specific Number */
  IPC_PB_NUMBER_DEDICATE,  			/* 0x04 : dedicated access, short code */
  IPC_PB_NUMBER_MAX
} ipc_pb_number_type_e_type;

/* HIDDEN_ENTRY field */
typedef enum{
  IPC_PB_HIDDEN_ENTRY_NO_HIDDEN=0x01,  		/* 0x00 : No Hidden */
  IPC_PB_HIDDEN_ENTRY_HIDDEN,  				/* 0x01 : Hidden */
  IPC_PB_HIDDEN_ENTRY_MAX
} ipc_pb_hidden_entry_e_type;


/* INDEX  field */
/* Location which the numbers are stored in the storage. */
/* If MODE = 0x01 ( ADD ), then INDEX field value is ignored
** so that NUMBER is stored in the empty memory location. */

/* NUMBER_LEN  field */
/* The length of NUMBER[] */

/* NUMBER_TYPE  field */
/* The type of the NUMBER */

/* NUMBER[]  field */
/* Maximum length : 256 bytes */
/* see MAX_PB_NUM_LEN */

/* TEXT_LEN  field */
/* The length of TEXT[] */

/* TEXT_ENCTYPE  field */
/* The type of the TEXT */

/* TEXT[]  field */
/* Maximum length : 256 bytes */
/* see MAX_PB_TEXT_LEN */

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
  
 FORMAT :
-------------------------------------------------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SELECTED_SLOT(1) | PB_STORAGE_TYPE(1) |
-------------------------------------------------------------------------------------
| START_INDEX(2) | END_INDEX(2) |
-------------------------------------------------------------------------------------
*/

/*SELECTED_SLOT(1) */
// refer to SET message

/*PB_STORAGE_TYPE*/
/*ipc_pb_storage_type_e_type*/

/* START_INDEX field */
/* start index of phonebook to be read */

/* END_INDEX  field */
/* end index of phonebook to be read */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :

 FORMAT :
-------------------------------------------------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  SELECTED_SLOT(1) | MODE(1) | PB_STORAGE_TYPE(1) |INDEX(2) |NEXT_INDEX(2) |
-------------------------------------------------------------------------------------
|SUB_PARAMETERS(x) |
-------------------------------------------------------------------------------------
*/



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                        0x03

 DESCRIPTION :
  
 FORMAT :
 --------------------------------------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  SELECTED_SLOT(1) | MODE(1) |PB_STORAGE_TYPE(1) |INDEX(2) | 
---------------------------------------------------------------------------
*/

/* INDEX field */
/* location of phonebook entry added */


/*=================================================================*/

/*=================================================================

   SUB_CMD(1) : IPC_PB_STORAGE      0x02                PhoneBook Storage Message

=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT           0x05     */
/*    IPC_CMD_INDI              0x01     */
/*    IPC_CMD_NOTI             0x03     */
/* none */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
  Read the phonebook storage information : 
           total memory size and current used memory size. +CPBS=?
  
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  SELECTED_SLOT(1) | PB_STORAGE_TYPE(1) |
-------------------------------------------------------------------------------------
*/



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
  Set phonebook storage. +CPBS=
  
 FORMAT :
-------------------------------------------------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  SELECTED_SLOT(1) | PB_STORAGE_TYPE(1) |
-------------------------------------------------------------------------------------
*/

/* PB_STORAGE_TYPE  field */
typedef enum{
  IPC_PB_ST_DC=0x01,  	/* 0x01 : ME dialed calls list */
  IPC_PB_ST_EN,  		/* 0x02 : SIM ( or ME ) emergency number */
  IPC_PB_ST_FD,  		/* 0x03 : SIM fixed-dialing phonebook EF[FDN] */
  IPC_PB_ST_LD,  		/* 0x04 : SIM last-dialing phonebook */
  IPC_PB_ST_MC,  		/* 0x05 : ME missed calls list */
  IPC_PB_ST_ME,  		/* 0x06 : ME phonebook */
  IPC_PB_ST_MT,  		/* 0x07 : Combined ME and SIM phonebook */
  IPC_PB_ST_ON,  		/* 0x08 : SIM ( or ME ) own numbers ( MSISDNs ) list EF[MSISDN], */
  IPC_PB_ST_RC,  		/* 0x09 : ME received calls list */
  IPC_PB_ST_SIM,  		/* 0x0A : SIM phonebook EF[ADN], DF[PHONEBOOK] */
  IPC_PB_ST_SDN,        /* 0x0B : Service Dialing Number */
  IPC_PB_ST_3GSIM,		/*0x0C : 3G SIM Phonebook EF[ADN]*/
  IPC_PB_ST_ICI,			/*0x0D : Incoming Call Information*/
  IPC_PB_ST_OCI,		/*0x0E : Outgoing Call Information*/
  IPC_PB_ST_AAS,		/*0x0F : Additional Number Alpha String*/
  IPC_PB_ST_GAS,		/*0x10 : Grouping Information String*/
  IPC_PB_ST_MAX
} ipc_pb_storage_type_e_type;




/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
  Set phonebook storage. +CPBS=
  
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  SELECTED_SLOT(1) | PB_STORAGE_TYPE(1) | TOTAL_COUNT(2) |
-------------------------------------------------------------------------------------
 | USED_COUNT(2) | 
-------------------------------------------------------------------------------------
*/

/* PB_STORAGE_TYPE  field */
/* see ipc_pb_storage_type_e_type */ 

/* TOTAL_COUNT  field */
/* The number of the total phonebook memory according to phonebook storage. */
 
/* USED_COUNT  field */
/* The number of the used phonebook memory. */



/*=================================================================

   SUB_CMD(1) : IPC_PB_STORAGE_LIST      0x03                PhoneBook Storage List Message

=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_SET               0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT           0x05     */
/*    IPC_CMD_INDI              0x01     */
/*    IPC_CMD_NOTI             0x03     */
/* none */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
  Read the phonebook storage List information : 
  
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SELECTED_SLOT(1) | 
-------------------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
  Read the phonebook storage List information : RESPONSE
  
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  SELECTED_SLOT(1) | PB_LIST(4) |
-------------------------------------------------------------------------------------
*/

/* PB_LIST field : Phone Book Lists supported by the phone */
/* The value is in a bit mask */



/*=================================================================

   SUB_CMD(1) : IPC_PB_ENTRY_INFO      0x04              PhoneBook Entry Information Message

=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_SET               0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT           0x05     */
/*    IPC_CMD_INDI              0x01     */
/*    IPC_CMD_NOTI             0x03     */
/* none */




/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
  Read the phonebook Entry information : 
  
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SELECTED_SLOT(1) | PB_STORAGE_TYPE(1) |
-------------------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
  Read the phonebook Entry information : RESPONSE
  
 FORMAT :
-------------------------------------------------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SELECTED_SLOT(1) | PB_STORAGE_TYPE(1) | PB_INDEX_MIN(2) | PB_INDEX_MAX(2) |
-------------------------------------------------------------------------------------
| PB_NUM_MAX(2) | PB_TEXT_MAX(2) |
-------------------------------------------------------------------------------------
*/

/* PB_INDEX_MIN field : Minimum index of phonebook entries */

/* PB_INDEX_MAX field : Maximum index of phonebook entries */

/* PB_NUM_MAX field : Maximum number of phonebook entry number fields */

/* PB_TEXT_MAX field : Maximum number of phonebook entry text fields */



/*=================================================================

   SUB_CMD(1) : IPC_PB_3GPB_CAPA     0x05       3G Phonebook Capability Message

=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_SET                      0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT           0x05     */
/*    IPC_CMD_INDI              0x01     */
/* none */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
  Get 3G Phone book Capability
  
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  SELECTED_SLOT(1) | 
-------------------------------------------------------------------------------------
*/



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
  3G Phone book Capability Response 
  
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SELECTED_SLOT(1) | PB_FIELD_NUMBER(1) | PB_FIELD(X) |
-------------------------------------------------------------------------------------

PB_FIELD contains as below
-------------------------------------------------------------------------------------
 | PB_FIELD_TYPE(1) | MAX_INDEX(2) | MAX_ENTRY_LEN(2) |USED_RECORD(2) |
 ------------------------------------------------------------------------------------
*/

/*PB_FIELD_TYPE : ipc_pb_field_e_type*/
/*MAX_INDEX : Maximum index of phonebook field entries*/
/*MAX_ENTRY_LEN : Maximum number of phonebook filed entry field*/
/*USED_RECORD : Record in use of phoebook field */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                        0x03

 DESCRIPTION :
  3G Phone book Capability Notification
  
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SELECTED_SLOT(1) | PB_FIELD_NUMBER(1) | PB_FIELD(X) |
-------------------------------------------------------------------------------------

PB_FIELD contains as below
-------------------------------------------------------------------------------------
 | PB_FIELD_TYPE(1) | MAX_INDEX(2) | MAX_ENTRY_LEN(2) |USED_RECORD(2) |
 ------------------------------------------------------------------------------------
*/

/*PB_FIELD_TYPE : ipc_pb_field_e_type*/
/*MAX_INDEX : Maximum index of phonebook field entries*/
/*MAX_ENTRY_LEN : Maximum number of phonebook filed entry field*/
/*USED_RECORD : Record in use of phoebook field */


/*=================================================================*/


/*********************************************************************************

                                            Sub Command of IPC_DISP_CMD[0x07]

**********************************************************************************/
typedef enum{
  IPC_DISP_ICON_INFO = 0x01,                 /* 0x01 : Display Icon Information Message */
  IPC_DISP_HOMEZONE_INFO = 0x02,        /* 0x02 : Display HomeZone Information Message */
  IPC_DISP_PHONE_FATAL_INFO = 0x03,  /* 0x03 : Display HomeZone Information Message */
  IPC_DISP_EXT_ROAM_INFO = 0x04,        /* 0x04 : Display CDMA Extended Roam Information message */
  IPC_DISP_USER_INDICATION = 0x05,     /* 0x05 : Display CDMA User Indication Message */
  IPC_DISP_MAX
} ipc_sub_disp_cmd_type;

/*********************************************************************************/


/*=================================================================

   SUB_CMD(1) : IPC_DISP_ICON_INFO      0x01                  Display Icon Information Message

=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_SET               0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT           0x05     */
/*    IPC_CMD_INDI             0x01     */
/* none */




/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :

 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | ICON_TYPE(1) |
-------------------------------------------------------------------------------------
*/

/* ICON_TYPE  field */
/* Sets the request value */
typedef enum{
  IPC_DISP_ICON_TYPE_RSSI           =0x01,    /* 0000 0001 : signal strength */
  IPC_DISP_ICON_TYPE_BAT            =0x02,    /* 0000 0010 : battery level   */ 
  IPC_DISP_ICON_TYPE_RSSI2        =0x03,
  IPC_DISP_ICON_TYPE_ALL=0xFF,     /* 1111 1111 : ALL Values Request */
} ipc_disp_icon_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :

 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | ICON_TYPE(1) |
-------------------------------------------------------------------------------------
 | RSSI(1) |  RSSI2(1) | BATTERY(1) | 
-------------------------------------------------------------------------------------
*/

/* ICON_TYPE  field */
/* see ipc_disp_icon_e_type */

/* RSSI  field */
/* Radio signal strength */
typedef enum{
  IPC_DISP_RSSI_0,     /* 0x00 */
  IPC_DISP_RSSI_1,     /* 0x01 */
  IPC_DISP_RSSI_2,     /* 0x02 */
  IPC_DISP_RSSI_3,     /* 0x03 */
  IPC_DISP_RSSI_4,     /* 0x04 */
  IPC_DISP_RSSI_5,     /* 0x05 */
  IPC_DISP_RSSI_6,     /* 0x06 */
  IPC_DISP_RSSI_MAX
} ipc_disp_icon_rssi_e_type;

/* RSSI2  field */
/* see ipc_disp_icon_rssi_e_type */

/* BATTERY  field */
/* Determines a battery Icon. */
typedef enum{
  IPC_DISP_BAT_LEVEL_0,    /* 0x00 : blank */
  IPC_DISP_BAT_LEVEL_1,    /* 0x01 */
  IPC_DISP_BAT_LEVEL_2,    /* 0x02 */
  IPC_DISP_BAT_LEVEL_3,    /* 0x03 : normal full */
  IPC_DISP_BAT_LEVEL_4,    /* 0x04 : full for smart phone*/
  IPC_DISP_BAT_LEVEL_5,    /* 0x05 : full for battery bar 5 //2008 new battery policy*/
  IPC_DISP_BAT_MAX
} ipc_disp_icon_batt_e_type;


/*    IPC_CMD_NOTI             0x03     */
/* same as IPC_CMD_RESP */


/*=================================================================

   SUB_CMD(1) : IPC_DISP_HOMEZONE_INFO      0x02                  Display Zone Information Message

=================================================================*/

/*    IPC_CMD_EXEC              0x01    */
/*    IPC_CMD_GET               0x02    */
/*    IPC_CMD_SET               0x03    */
/*    IPC_CMD_CFRM                    0x04     */
/*    IPC_CMD_EVENT                   0x05     */
/*    IPC_CMD_INDI              0x01    */
/*    IPC_CMD_RESP              0x02    */
/* none */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                        0x03

 DESCRIPTION : O2 HomeZone or CityZone Service Notification

 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | ZONE_INDI(1) | ZONE_TYPE(1) | ZONE_TAG_PRESENT(1) 
-------------------------------------------------------------------------------------
 | ZONE_TAG_NAME(x) |
-------------------------------------------------------------------------------------
*/

/* ZONE_INDI  field */
typedef enum {
  IPC_DISP_O2_ZONEINDI_NONE,
  IPC_DISP_O2_ZONEINDI_IND,
  IPC_DISP_O2_ZONEINDI_CLEAR,
  IPC_DISP_O2_ZONEINDI_MAX
} ipc_disp_o2_zone_indi_e_type;

/* ZONE_TYPE  field */
typedef enum {
  IPC_DISP_O2_ZONETYPE_NONE,
  IPC_DISP_O2_ZONETYPE_HOMEZONE,
  IPC_DISP_O2_ZONETYPE_CITYZONE,
  IPC_DISP_O2_ZONETYPE_MAX
} ipc_disp_o2_zone_type_e_type;

/* ZONE_TAG_PRESENT  field */
typedef enum {
  IPC_DISP_O2_ZONETAG_NONE,
  IPC_DISP_O2_ZONETAG_PRESENT,       
  IPC_DISP_O2_ZONETAG_ABSENT, 
  IPC_DISP_O2_ZONETAG_MAX
} ipc_disp_o2_zone_tag_present_e_type;




/*=================================================================

   SUB_CMD(1) : IPC_DISP_PHONE_FATAL_INFO = 0x03,      Phone Fatal Error Information Message

=================================================================*/

/*    IPC_CMD_EXEC              0x01    */
/*    IPC_CMD_GET               0x02    */
/*    IPC_CMD_SET               0x03    */
/*    IPC_CMD_CFRM                    0x04     */
/*    IPC_CMD_EVENT                   0x05     */
/*    IPC_CMD_INDI              0x01    */
/*    IPC_CMD_RESP              0x02    */
/* none */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                        0x03

 DESCRIPTION : Phone fatal error Notification

 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MESSAGE_LEN(1) | MESSAGE(x) 
-------------------------------------------------------------------------------------
*/





/*=================================================================

   SUB_CMD(1) : IPC_DISP_EXT_ROAM_INFO = 0x04,      CDMA Extended Roam Information message

=================================================================*/

/*    IPC_CMD_EXEC              0x01    */
/*    IPC_CMD_GET               0x02    */
/*    IPC_CMD_SET               0x03    */
/*    IPC_CMD_CFRM              0x04     */
/*    IPC_CMD_EVENT             0x05     */
/*    IPC_CMD_INDI              0x01    */
/*    IPC_CMD_RESP              0x02    */
/* none */

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                        0x03

 DESCRIPTION : CDMA Extended Roam Information Notification

 FORMAT :
---------------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CONTROL(1) | DATA_ROAM_GUARD(1) | TEXT_LEN(1) | TEXT(x) 
---------------------------------------------------------------------------------------------------
*/

/* CONTROL  field */
typedef enum {
  IPC_DISP_ERI_CONTROL_ROAM_TEXT_OFF = 0x01,         /* 0x01 : ROAM TEXT OFF */
  IPC_DISP_ERI_CONTROL_ROAM_TEXT_ON,                 /* 0x02 : ROAM TEXT ON */
  IPC_DISP_ERI_CONTROL_ROAM_TEXT_MAX
} ipc_disp_ext_roam_control_e_type;

/* DATA_ROAM_GUARD field */
typedef enum {
  IPC_DISP_ERI_DATA_ROAM_GUARD_OFF = 0x01,         /* 0x01 : ROAM DEFAULT */
  IPC_DISP_ERI_DATA_ROAM_GUARD_ON,                 /* 0x02 : ROAM TEXT */
  IPC_DISP_ERI_DATA_ROAM_GUARD_MAX
} ipc_disp_ext_roam_guard_e_type;

/* TEXT_LEN field */
/* refer to MAX_DISP_ERI_TEXT_INFO_LEN */





/*=================================================================

   SUB_CMD(1) : IPC_DISP_USER_INDICATION = 0x05,      CDMA User Indication message

=================================================================*/

/*    IPC_CMD_EXEC              0x01    */
/*    IPC_CMD_GET               0x02    */
/*    IPC_CMD_SET               0x03    */
/*    IPC_CMD_CFRM              0x04     */
/*    IPC_CMD_EVENT             0x05     */
/*    IPC_CMD_INDI              0x01    */
/*    IPC_CMD_RESP              0x02    */
/* none */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                        0x03

 DESCRIPTION : CDMA User Indication Notification

 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | USER_IND_TYPE(1) 
-------------------------------------------------------------------------------------
*/

/* USER_IND_TYPE field */
typedef enum {
  IPC_DISP_USER_INDI_NONE,
  IPC_DISP_USER_INDI_NORMAL = 0x01,                         /* 0x01 : Normal Mode */
  IPC_DISP_USER_INDI_INITIAL_PROGRAM_NEEDED,                /* 0x02 : Initial Programming needed */
  IPC_DISP_USER_INDI_MAINTENANCE_REQUIRED,                  /* 0x03 : Maintenance Required */
  IPC_DISP_USER_INDI_AUTHENTICATION_REQUIRED,               /* 0x04 : Authentication Required */
  IPC_DISP_USER_INDI_MAX
} ipc_disp_user_indication_e_type;







/*=================================================================*/





/*********************************************************************************

                                            Sub Command of IPC_NET_CMD [0x08]

**********************************************************************************/
typedef enum{
  IPC_NET_PREF_PLMN=0x01,           /* 0x01 : Network Preferred PLMN Message */
  IPC_NET_PLMN_SEL,                 /* 0x02 : Network PLMN Selection Message */
  IPC_NET_SERVING_NETWORK,          /* 0x03 : Network Current PLMN Message */
  IPC_NET_PLMN_LIST,                /* 0x04 : Netowrk PLMN List Message */
  IPC_NET_REGIST,                   /* 0x05 : Network Registration Message */
  IPC_NET_SUBSCRIBER_NUM,           /* 0x06 : ME Subscriber Number Message */
  IPC_NET_BAND_SEL,                 /* 0x07 : Network Band Selection Message */
  IPC_NET_SERVICE_DOMAIN_CONFIG,    /* 0x08 : Network Service domain configuration Message */
  IPC_NET_POWERON_ATTACH,           /* 0x09 : Network Power On Attach Message */  
  IPC_NET_MODE_SEL,                 /* 0x0A : CDMA-GSM Mode Select Message */
  IPC_NET_ACQ_ORDER,                /* 0x0B : Network Acquistion Order Message */
  IPC_NET_IDENTITY,                 /* 0x0C : Network Identity Message */
  IPC_NET_PREFERRED_NETWORK_INFO,   /* 0x0D : CDMA Preferred Network Message */
  IPC_NET_HYBRID_MODE,              /* 0x0E : CDMA Hybrid Mode Message */
  IPC_NET_MAX
} ipc_sub_net_cmd_type;

/*********************************************************************************/




/*=================================================================

   SUB_CMD(1) : IPC_NET_PREF_PLMN        0x01                Network Preferred PLMN Message
   
=================================================================*/

/*    IPC_CMD_EXEC               0x01     */
/*    IPC_CMD_CFRM               0x04     */
/*    IPC_CMD_EVENT              0x05     */
/*    IPC_CMD_INDI                0x01     */
/*    IPC_CMD_NOTI               0x03     */
/* none */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
   Get Preferred PLMN List
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
   Set Preferred Operator List
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MODE(1) | INDEX(1) | PLMN(6) | ACT(1)
-------------------------------------------------------------------------------------
*/

/* MODE field */
typedef enum{
  IPC_NET_MODE_ADD=0x01,   /* 0x01 : Add the Preferred Operator */
  IPC_NET_MODE_EDIT,           /* 0x02 : Edit the Preferred Operator */
  IPC_NET_MODE_DEL,             /* 0x03 : Delete the Preferred Operator */
  IPC_NET_MODE_MAX
} ipc_net_oper_mode_e_type;

/* INDEX  field */
/* Operator index in the EFplmnsel of the SIM */


/* PLMN  field */
/* Operator Name or Code. Terminated by null */
/* this depends on the FORMAT field */
/* see MAX_NET_PLMN_NAME_NUMERIC_LEN */


/* ACT field */
/* Access technology selected */
typedef enum{
  IPC_NET_ACT_GSM = 0x01,           /* 0x01 : GSM */
  IPC_NET_ACT_GPRS,                 /* 0x02 : GPRS */
  IPC_NET_ACT_EGPRS,                /* 0x03 : EGPRS */
  IPC_NET_ACT_UMTS,                 /* 0x04 : UMTS */
  IPC_NET_ACT_IS95A = 0x11,         /* 0x11 : IS95A */
  IPC_NET_ACT_IS95B,                /* 0x12 : IS95B */
  IPC_NET_ACT_1X,                   /* 0x13 : 1X(IS2000) */
  IPC_NET_ACT_EVDO_REV_0,           /* 0x14 : EV-DO rev0 */
  IPC_NET_ACT_1X_EVDO_REV_0_HYBRID, /* 0x15 : 1X + EV-DO rev0 */
  IPC_NET_ACT_EVDO_REV_A,           /* 0x16 : EV-DO revA */
  IPC_NET_ACT_1X_EVDO_REV_A_HYBRID, /* 0x17 : 1X + EV-DO revA */
  IPC_NET_ACT_EVDV,                 /* 0x18 : EV-DV */
  IPC_NET_ACT_NOTSPECIFIED = 0xFF   /* 0xFF : Not Specified*/
} ipc_net_act_e_type;



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
   Preferred Operator List Response
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | NUM_RECORD(1) | INDEX(1) | PLMN(6) | ACT(1)
-------------------------------------------------------------------------------------
*/


/* NUM_RECORDS field */
/* all following fields are repeated NUM_RECORDS times */
/* see MAX_GSM_PREF_PLMN_LIST_NUM */

/* INDEX  field */
/* Operator index in the EFplmnsel of the SIM */

/* PLMN field */
/* Numeric PLMN Code. MCC(3)+MNC(3) */

/* ACT  field */
/* see ipc_net_act_e_type */

/*=================================================================*/


/*=================================================================

   SUB_CMD(1) : IPC_NET_PLMN_SEL          0x02        Network PLMN Selection Message

=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI              0x01     */
/*    IPC_CMD_NOTI             0x03     */
/* none */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
   Get the PLMN Selection Mode   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
-------------------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
   Set the PLMN Selection Mode   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MODE_SELECT(1) | PLMN(6) | ACT(1)
-------------------------------------------------------------------------------------
*/

/* MODE_SELECT field : Network Selection Mode */
typedef enum{
  IPC_NET_SEL_NONE,         /* 0x00 */
  IPC_NET_SEL_GLOBAL,       /* 0x01 : Automatic */
  IPC_NET_SEL_GSM_AUTO,     /* 0x02 : GSM Automatic selection*/
  IPC_NET_SEL_GSM_MANU,     /* 0x03 : GSM Manual selection*/
  IPC_NET_SEL_CDMA,         /* 0x04 : CDMA selection*/
  IPC_NET_SEL_MAX
} ipc_net_sel_e_type;


/* PLMN field */
/* this field is not valid if MODE_SELECT = automatic */

/* ACT  field */
/* see ipc_net_act_e_type */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
   the PLMN Selection Mode Response
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MODE_SELECT(1) |
-------------------------------------------------------------------------------------
*/

/* MODE_SELECT field */
/* see the ipc_net_sel_e_type */

/*=================================================================

   SUB_CMD(1) : IPC_NET_SERVING_NETWORK          0x03        Network Serving network Message

=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI              0x01     */
/* none */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
    Get Current Network,
    Get the current mode, the currently selected network and
     the current Access Technology 
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
    Current Operator Reponse
   
 FORMAT :
--------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MODE_SELECT | REG_STATUS(1) | ACT(1) | PLMN(6) | DATA(x) |
--------------------------------------------------------------------------------------------
*/

/* MODE_SELECT  field */
/* see ipc_net_sel_e_type 
*/

/* REG_STATUS field */
/* see ipc_net_regist_status_e_type */

/* ACT  field */
/* see ipc_net_act_e_type */

/* DATA field for GSM */
/*
-------------------------------------------------------------------------------------
 | LAC(2)|
-------------------------------------------------------------------------------------
*/

/* PLMN  field : MCC(3)+MNC(3)*/

/*LAC field*/
/*Location Area Code (Word Type)*/


/* DATA field for CDMA */
/*
-------------------------------------------------------------------------------------
 | CARRIER(1) | SID(2) | NID(2) | BS_ID(2) | BS_LAT(2) | BS_LONG(2) | REG_ZONE(2) | PILOT_PN(2) | 
-------------------------------------------------------------------------------------
 | BAND_CLASS(1) | CHANNEL(2) |
------------------------------------------------------------------------------------- 
*/

/* CARRIER field */
typedef enum {
  IPC_NET_SERVING_CARRIER_TEST,                             /* 0x00 : TestBed */
  IPC_NET_SERVING_CARRIER_SKT = 0x01,                       /* 0x01 : KOREA SKT */
  IPC_NET_SERVING_CARRIER_KTF,                              /* 0x02 : KOREA KTF */
  IPC_NET_SERVING_CARRIER_LGT,                              /* 0x03 : KOREA LGT */
  IPC_NET_SERVING_CARRIER_VERIZON = 0x11,                   /* 0x11 : US VERIZON */
  IPC_NET_SERVING_CARRIER_SPRINT,                           /* 0x12 : US SPRINT */
  IPC_NET_SERVING_CARRIER_ALLTEL,                           /* 0x13 : US ALLTEL */
  IPC_NET_SERVING_CARRIER_METRO_PCS,                        /* 0x14 : US METRO_PCS */
  IPC_NET_SERVING_CARRIER_US_CELLULAR,                      /* 0x15 : US CELLULAR */
  IPC_NET_SERVING_CARRIER_CRIKET,                           /* 0x16 : US CRIKET */
  IPC_NET_SERVING_CARRIER_TELUS = 0x21,                     /* 0x21 : CANADA TELUS */
  IPC_NET_SERVING_CARRIER_BMC,                              /* 0x22 : CANADA BMC */
  IPC_NET_SERVING_CARRIER_BWA,                              /* 0x23 : CANADA BWA */
  IPC_NET_SERVING_CARRIER_CTC = 0x31,                       /* 0x31 : CHINA CTC */
  IPC_NET_SERVING_CARRIER_MAX
} ipc_net_serving_carrier_e_type;


/* SID field */
/* 5 digit System ID */

/* NID(2) */
/* 5 digit Network ID */

/* BS_ID(2) */

/* BS_LAT(2) */

/* BS_LONG(2) */

/* REG_ZONE(2) */

/* PILOT_PN(2) */

/* BAND_CLASS */
typedef enum {
IPC_NET_BAND_CLASS_NONE,
IPC_NET_BAND_CLASS_0_CELL800M,            /* Cellular 800  */
IPC_NET_BAND_CLASS_1_PCS1900M,            /* PCS 1900      */
IPC_NET_BAND_CLASS_2_TACS,                /* TACS          */
IPC_NET_BAND_CLASS_3_JTACS,               /* JTACS         */
IPC_NET_BAND_CLASS_4_KPCS,                /* Korean PCS    */
IPC_NET_BAND_CLASS_5_450M,                /* 450 MHz       */
IPC_NET_BAND_CLASS_6_2G,                  /* 2 GHz         */
IPC_NET_BAND_CLASS_7_700M,                /* 700 MHz       */
IPC_NET_BAND_CLASS_8_1800M,               /* 1800 MHz      */
IPC_NET_BAND_CLASS_9_900M,                /* 1900 MHz      */
IPC_NET_BAND_CLASS_10_SECONDARY_800M,     /* Secondary 800 */
IPC_NET_BAND_CLASS_11_400M_PAMR,          /* 400 MHz PAMR  */
IPC_NET_BAND_CLASS_12_800M_PAMR,          /* 800 MHz PAMR  */
IPC_NET_BAND_CLASS_13_2500M_IMT2000_EXT,  /* 2.5 GHz IMT-2000 Extensions */
IPC_NET_BAND_CLASS_14_USPCS_1900M,        /* US PCS 1.9 GHz */
IPC_NET_BAND_CLASS_15_AWS,                /* AWS */
IPC_NET_BAND_CLASS_MAX
}ipc_net_serving_band_class_e_type;

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
    Current Operator Notification
   
 FORMAT :
--------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MODE_SELECT | REG_STATUS(1) | ACT(1) | PLMN(6) | DATA(x) |
--------------------------------------------------------------------------------------------
*/


/*=================================================================*/




/*=================================================================

   SUB_CMD(1) : IPC_NET_PLMN_LIST           0x04          Network PLMN List Message
   
=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_SET               0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT           0x05     */
/*    IPC_CMD_INDI             0x01     */
/* none */




/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
    Get PLMN List 
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
-------------------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
    PLMN List Response
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | NUM_RECORD(1) | 
-------------------------------------------------------------------------------------
 | PLMN_STATUS(1) | PLMN(6) | ACT(1) | LAC(2) |
-------------------------------------------------------------------------------------
*/

/* PLMN_STATUS  field */
typedef enum{
  IPC_NET_PLMN_STATUS_UNKNOWN = 0x01,        /* 0x01 : Unknown or unavailable */
  IPC_NET_PLMN_STATUS_AVAIL,               /* 0x02 : Available */
  IPC_NET_PLMN_STATUS_CURRENT,          /* 0x03 : Current */
  IPC_NET_PLMN_STATUS_FORBIDDEN,      /* 0x04 : Forbidden */
  IPC_NET_PLMN_STATUS_MAX
} ipc_net_plmn_status_e_type;

/* NUM_RECORD field */
/* all following fields are repeated NUM_RECORD times */

/* PLMN  field */
/* Numeric PLMN - MCC(3byte) + MNC(3byte) */
/* see MAX_NET_PLMN_NAME_NUMERIC_LEN */

/* ACT  field */
/* see ipc_net_act_e_type */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
    PLMN List  Notification
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | NUM_RECORD(1) | 
-------------------------------------------------------------------------------------
 | PLMN_STATUS(1) | PLMN(6) | ACT(1) | LAC(2) |
-------------------------------------------------------------------------------------
*/


/*=================================================================*/





/*=================================================================

   SUB_CMD(1) : IPC_NET_REGIST                0x05             Network Registration Message
   
=================================================================*/

/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_CFRM          0x04     */
/*    IPC_CMD_EVENT              0x05     */
/*    IPC_CMD_INDI                 0x01     */
/* none */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_EXEC                     0x01

 DESCRIPTION :
    Execute Network Registration
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | ACT(1) | REGIST_MODE(1)
-------------------------------------------------------------------------------------
*/

/* ACT  field */
/* see ipc_net_act_e_type */

/* REGIST_MODE field : registration mode */
/* MODE  field */
typedef enum {
  IPC_NET_DEREGISTER = 0x01,			/* 0x01 : Deregister to the Network */
  IPC_NET_REGISTER         	/* 0x02 : Register to the Network */
} ipc_net_regist_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
    Get Network Registration
    Read the registration status of the mobile equipment from the phone. : +CREG
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | ACT(1) | SERVICE_DOMAIN(1) |
-------------------------------------------------------------------------------------
*/

/* ACT  field */
/* see ipc_net_act_e_type */

/* SERVICE_DOMAIN filed */
/* see ipc_net_svc_domain_e_type */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
     Network Registration response
    Registration status information to PDA : +CREG
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | ACT(1) | SERVICE_DOMAIN(1) | REGIST_STATUS(1) | 
-------------------------------------------------------------------------------------
 | EDGE_SUPPORT(1) | LAC(2) | CELL_ID(4) | REJ_CAUSE(1)
-------------------------------------------------------------------------------------
*/

/* ACT field */
/* see ipc_net_act_e_type */

/* SERVICE_DOMAIN filed */
/* see ipc_net_svc_domain_e_type */

/* REGIST_STATUS  field */
/* Registration status */
typedef enum{
  IPC_NET_REG_STAT_NOT_REG=0x01,   /* 0x01 : not registered, ME is not currently searching a new
                                                                                       PLMN to register to */
  IPC_NET_REG_STAT_REG_HOME,          /* 0x02 : Registered, home network */
  IPC_NET_REG_STAT_SEARCHING,         /* 0x03 : not registered, but ME is currently searching a new
                                                                                        SPLMN to register to */
  IPC_NET_REG_STAT_REG_DENIED,       /* 0x04 : registration denied */
  IPC_NET_REG_STAT_UNKNOWN,           /* 0x05 : unknown */
  IPC_NET_REG_STAT_REG_ROAM,          /* 0x06 : registered, roaming */
  IPC_NET_REG_STAT_MAX
} ipc_net_regist_status_e_type;

/* EDGE_SUPPORT filed */
typedef enum{
  IPC_NET_EDGE_NOTSUPPORT=0x00,
  IPC_NET_EDGE_SUPPORT=0x01
}ipc_net_edge_support_e_type;

/* LAC  field */
/* Location Area Code in hexadecimal format */

/* CELL_ID */
/* Cell ID in hexadecimal format */

/* REJ_CODE */
/* REJ_CODE in hexadecimal format */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
     Network Registration Notification
    Registration status information to PDA : +CREG
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | ACT(1) | SERVICE_DOMAIN(1) | REGIST_STATUS(1) | 
-------------------------------------------------------------------------------------
 | EDGE_SUPPORT(1) | LAC(2) | CELL_ID(4) | REJ_CAUSE(1)
-------------------------------------------------------------------------------------
*/



/*=================================================================*/




/*=================================================================

   SUB_CMD(1) : IPC_NET_SUBSCRIBER_NUM      0x06   Network ME Subscriber Number Message
   
=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT           0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_NOTI             0x03     */
/* none */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
   Get Subscriber Number 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
   Set Subscriber Number 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | NUM_SUB(1) | NUMBER_LEN(1) | NUMBER_TYPE(1) |
-------------------------------------------------------------------------------------
 | NUMBER(X) | ALPHA_LEN(1) | ALPHA(X) |
-------------------------------------------------------------------------------------
*/

/* NUM_SERVICE */
/* the number of the service which the user is subscribed to */
/* NUM_SERVICE occurrences of the following fields */

/* NUMBER TYPE */
/* enum ipc_num_type_e_type defined in IPC_CALL_OUTGOING sub command */

/* NUMBER */
/* ASCII Value, maximum length is 16.  Subscriber Number ( MSISDN ) */
/* see MAX_NET_SUBS_NUM_LEN */

/* ALPHA_LEN */
/* The length of ALPHA field */

/* ALPHA */
/* optional alphanumeric string associated with number maximum length is 32 */
/* see MAX_NET_SUBS_ALPHA_LEN */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
   Subscriber Number Response
 FORMAT :
-------------------------------------------------------------------------------------
 |  MAIN_CMD(1)  |   SUB_CMD(1)   | CMD_TYPE(1) | 
-------------------------------------------------------------------------------------
       1                 1               1        
-------------------------------------------------------------------------------------
  | NUMBER_LEN(1) | NUMBER_TYPE(1) | NUMBER(X) | ALPHA_LEN(1) | ALPHA(X) |
-------------------------------------------------------------------------------------
               1                     1                 X               1                X
-------------------------------------------------------------------------------------
*/


/*=================================================================

   SUB_CMD(1) : IPC_NET_BAND_SEL      0x07   Network Band Selection Message
   
=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT           0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_NOTI             0x03     */
/* none */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
   Get Band Selection
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
   Set Band Selection
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | BAND_MODE(1) | BAND(1) |
-------------------------------------------------------------------------------------
*/

/* BAND_MODE field */
typedef enum {
  IPC_NET_BAND_MODE_PREF =0x01,     /* 0x01 : Band Preferred Mode */
  IPC_NET_BAND_MODE_ONLY,               /* 0x02 : Band Only Mode */
  IPC_NET_BAND_MODE_MAX
} ipc_net_band_mode_e_type;

/* BAND field */
typedef enum {
  IPC_NET_BAND_ANY = 0x01,              /* 0x01 : ANY */
  IPC_NET_BAND_GSM850 = 0x02,        /* 0x02 : GSM850 */
  IPC_NET_BAND_GSM900 = 0x03,        /* 0x03 : GSM900 */
  IPC_NET_BAND_GSM1800 = 0x04,      /* 0x04 : GSM1800 */
  IPC_NET_BAND_GSM1900 = 0x05,      /* 0x05 : GSM1900 */
  IPC_NET_BAND_GSM850_1900 = 0x06,    /* 0x06 : GSM850_1900 */
  IPC_NET_BAND_GSM900_1800 = 0x07,    /* 0x07 : GSM900_1800 */
  IPC_NET_BAND_GSM_ALL = 0x08,    /* 0x08 : GSM_ALL(850, 900, 1800, 1900) */
  IPC_NET_BAND_UMTS = 0x09,                /* 0x09 : UMTS */
  IPC_NET_BAND_MAX
} ipc_net_band_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
   Network Band Selection Response
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | BAND_MODE(1) | BAND(1) |
-------------------------------------------------------------------------------------
*/



/*=================================================================*/



/*=================================================================

   SUB_CMD(1) : IPC_NET_SERVICE_DOMAIN_CONFIG      0x08   Network Service Domain configuration Message
   
=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT           0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_NOTI             0x03     */
/* none */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
   Get Band Selection
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
   Set Band Selection
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SERVICE_DOMAIN(1) |
-------------------------------------------------------------------------------------
*/

/* SERVICE_DOMAIN field */
typedef enum {
  IPC_NET_SERVICE_DOMAIN_COMBINED =0x01,     /* 0x01 : Combined(CS + PS) Mode */
  IPC_NET_SERVICE_DOMAIN_CIRCUIT,            /* 0x02 : Circuit Only Mode */
  IPC_NET_SERVICE_DOMAIN_PACKET,           	 /* 0x03 : Packet service Only Mode */  
  IPC_NET_SERVICE_DOMAIN_MODE_MAX
} ipc_net_svc_domain_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
   Network Service Domain configuration Response
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SERVICE_DOMAIN(1) |
-------------------------------------------------------------------------------------
*/



/*=================================================================

   SUB_CMD(1) : IPC_NET_POWERON_ATTACH      0x09   Network Power on attach Message
   
=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_NOTI             0x03     */
/* none */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
   Get Band Selection
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
   Set Power on attach Selection
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | POWERONATTACH(1) |
-------------------------------------------------------------------------------------
*/

/* POWERONATTACH field */
typedef enum {
  IPC_NET_POWERONATTACH_DISABLE =0x00,     /* 0x00 : Power On attach disable*/
  IPC_NET_POWERONATTACH_ENABLE,            /* 0x01 : Power On attach enable */
  IPC_NET_POWERONATTACH_MODE_MAX
} ipc_net_poweronattach_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
   Network Power on attach Response
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | POWERONATTACH(1) |
-------------------------------------------------------------------------------------
*/


/*=================================================================

   SUB_CMD(1) : IPC_NET_MODE_SEL      0x0a   Network Mode Select Message
   
=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI              0x01     */
/*    IPC_CMD_NOTI             0x03     */
/* none */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
   Get Mode Selection
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
   Set Mode Selection
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  MODE_SELECT(1) |
-------------------------------------------------------------------------------------
*/

/* MODE_SELECT field */
typedef enum {
  IPC_NET_MODE_SEL_AUTO = 0x01,   /* 0x01 : GSM + WCDMA Mode */
  IPC_NET_MODE_SEL_GSM,            /* 0x02 : GSM Mode */
  IPC_NET_MODE_SEL_WCDMA,       /* 0x03 : WCDMA Mode */
  IPC_NET_MODE_SEL_MAX
} ipc_net_mode_select_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
   Mode Selection Response
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MODE_SELECT(1) |
-------------------------------------------------------------------------------------
*/


/*=================================================================

   SUB_CMD(1) : IPC_NET_ACQ_ORDER      0x0b   Network Acquisition Order Message
   
=================================================================*/
/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT           0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_NOTI             0x03     */
/* none */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
   Get Acquisition Order
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
   Set Acquisition Order
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  ACQ_ORDER(1) |
-------------------------------------------------------------------------------------
*/

/* ACQ_ORDER field */
typedef enum {
  IPC_NET_ACQ_ORDER_AUTO = 0x01,   			/* 0x01 : Auto */
  IPC_NET_ACQ_ORDER_GSM_FIRST,			/* 0x02 : GSM First  */
  IPC_NET_ACQ_ORDER_WCDMA_FIRST,			/* 0x03 : WCDMA First */
  IPC_NET_ACQ_ORDER_NO_CHANGE,			/* 0x04 : No Change  */
  IPC_NET_ACQ_ORDER_MAX
} ipc_net_acq_order_e_type;



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
   Network Acquisition Order Response
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | ACQ_ORDER(1) |
-------------------------------------------------------------------------------------
*/

/*=================================================================

   SUB_CMD(1) : IPC_NET_IDENTITY     0x0c   Network Idenetity Message
   
=================================================================*/
/*    IPC_CMD_EXEC                    0x01 */
/*    IPC_CMD_GET                     0x02 */
/*    IPC_CMD_SET                     0x03 */
/*    IPC_CMD_CFRM                    0x04 */
/*    IPC_CMD_EVENT                   0x05 */
/*    IPC_CMD_INDI                    0x01 */
/*    IPC_CMD_RESP                    0x02 */

/* none */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) :     IPC_CMD_NOTI             0x03     

 DESCRIPTION :
   Network Identity Notification
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MM_INFO_TYPE(1) | ACTIVE_MASK(1) | 
-------------------------------------------------------------------------------------
 | SHORT_NAME_CI(1) | SHORT_NAME_LEN(1) | SHORT_NAME_DCS(1) | SHORT_NAME(16) |
-------------------------------------------------------------------------------------
| FULL_NAME_CI(1)   | FULL_NAME_LEN(1)  | FULL_NAME_DCS(1)  | FULL_NAME(32)  |
-------------------------------------------------------------------------------------
*/

/* MM_INFO_TYPE field */
typedef enum {
  IPC_NET_IDENTITY_MM_INFO_CS = 0x00,                   /* 0x00 : Disable */
  IPC_NET_IDENTITY_MM_INFO_PS = 0x01,                   /* 0x01 : Enable */
  IPC_NET_IDENTITY_MM_INFO_MAX
} ipc_net_identity_mm_info_e_type;

/* ACTIVE_MASK field */
typedef enum {
  IPC_NET_IDENTITY_ACTIVE_MASK_SHORT = 0x01,   			/* 0x01 : Short */
  IPC_NET_IDENTITY_ACTIVE_MASK_FULL,   			        /* 0x02 : Full */
  IPC_NET_IDENTITY_ACTIVE_MASK_ALL = 0xff                                      /* 0xff : All */
} ipc_net_identity_active_mask_e_type;

/* CI field - common for SHORT_NAME_CI and FULL_NAME_CI */
typedef enum {
  IPC_NET_IDENTITY_CI_DISABLE = 0x00, 	                             /* 0x00 : Disable */
  IPC_NET_IDENTITY_CI_ENABLE = 0x01, 	                             /* 0x01 : Enable */
  IPC_NET_IDENTITY_CI_MAX
} ipc_net_identity_ci_e_type;

/* NAME_LEN field */
// Length of name (byte)

/* NAME_DCS field */
// Data coding scheme of NAME field
// Refer to the ipc_text_enc_e_type




/*=================================================================

   SUB_CMD(1) : IPC_NET_PREFERRED_NETWORK_INFO     0x0D : CDMA Preferred Network Message
   
=================================================================*/
/*    IPC_CMD_EXEC                    0x01 */
/*    IPC_CMD_CFRM                    0x04 */
/*    IPC_CMD_EVENT                   0x05 */
/*    IPC_CMD_INDI                    0x01 */
/*    IPC_CMD_NOTI                    0x03 */
/* none */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) :     IPC_CMD_GET              0x02    

 DESCRIPTION :
   Preferred Network get
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) :     IPC_CMD_SET                     0x03     

 DESCRIPTION :
   Preferred Network set
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | PREF_NET_TYPE(1) |
-------------------------------------------------------------------------------------
*/

/* PREF_NET_TYPE field */
typedef enum {
IPC_NET_PREF_NET_TYPE_AUTOMATIC = 0x01, /* 0x01 : AUTOMATIC */
IPC_NET_PREF_NET_TYPE_AUTOMATIC_A,      /* 0x02 : AUTOMATIC A */
IPC_NET_PREF_NET_TYPE_AUTOMATIC_B,      /* 0x03 : AUTOMATIC B */
IPC_NET_PREF_NET_TYPE_HOME_ONLY,        /* 0x04 : HOME ONLY */
IPC_NET_PREF_NET_TYPE_MAX
}ipc_net_pref_net_type_e_type;


/* PRL_PREF_ONLY field */
typedef enum {
IPC_NET_PRL_PREF_ONLY_OFF = 0x00,  /* 0x00 : PRL Preferred Only Off */
IPC_NET_PRL_PREF_ONLY_ON,               /* 0x02 : PRL Preferred Only On */
IPC_NET_PRL_PREF_ONLY_MAX
}ipc_net_prl_pref_only_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) :     IPC_CMD_RESP                    0x02    

 DESCRIPTION :
   Preferred Network response
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CARRIER(1) | PREF_NET_TYPE(1) | PRL_PREF_ONLY(1) |
-------------------------------------------------------------------------------------
*/

/* PREF_NET_TYPE field */
/* ipc_net_pref_net_type_e_type */

/* PRL_PREF_ONLY field */
/* ipc_net_prl_pref_only_e_type */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) :     IPC_CMD_NOTI                    0x03 

 DESCRIPTION :
   Preferred Network response
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CARRIER(1) | PREF_NET_TYPE(1) | PRL_PREF_ONLY(1) |
-------------------------------------------------------------------------------------
*/

/* PREF_NET_TYPE field */
/* ipc_net_pref_net_type_e_type */

/* PRL_PREF_ONLY field */
/* ipc_net_prl_pref_only_e_type */




/*=================================================================

   SUB_CMD(1) : IPC_NET_HYBRID_MODE     0x0E : CDMA Hybrid Mode Message
   
=================================================================*/
/*    IPC_CMD_EXEC                    0x01 */
/*    IPC_CMD_CFRM                    0x04 */
/*    IPC_CMD_EVENT                   0x05 */
/*    IPC_CMD_INDI                    0x01 */
/*    IPC_CMD_NOTI                    0x03 */
/* none */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) :     IPC_CMD_GET              0x02    

 DESCRIPTION :
   CDMA Hybrid Mode Get
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/





/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) :     IPC_CMD_SET                     0x03     

 DESCRIPTION :
   CDMA Hybrid Mode set
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | HYBRID_MODE(1) | 
-------------------------------------------------------------------------------------
*/

/* HYBRID_MODE field */
typedef enum {
IPC_NET_HYBRID_MODE_HYBRID = 0x01,  /* 0x01 : HYBRID(1X + EVDO) */
IPC_NET_HYBRID_MODE_1X_ONLY,        /* 0x02 : 1X ONLY */
IPC_NET_HYBRID_MODE_EVDO,           /* 0x03 : EVDO ONLY */
IPC_NET_HYBRID_MODE_MAX
}ipc_net_hybrid_mode_e_type;



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) :     IPC_CMD_RESP                    0x02    

 DESCRIPTION :
   CDMA Hybrid Mode response
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | HYBRID_MODE(1) | 
-------------------------------------------------------------------------------------
*/




/*=================================================================*/



/*********************************************************************************

                                            Sub Command of IPC_SND_CMD [0x09]

**********************************************************************************/
typedef enum{
  IPC_SND_KEY_TONE=0x01,          		/* 0x01 : Sound Keytone Message */
  IPC_SND_NOTI_TONE,                 	/* 0x02 : Sound Notification Tone Message */
  IPC_SND_LED_CTRL,                     /* 0x03 : Sound LED Control Message */
  IPC_SND_VIB_CTRL,                     /* 0x04 : Sound Vibrator Control Message */
  IPC_SND_SPKR_VOLUME_CTRL,     		/* 0x05 : Sound Speaker Volume Control Message */
  IPC_SND_MIC_GAIN_CTRL,           		/* 0x06 : Sound MIC Gain Control Message */
  IPC_SND_MIC_MUTE_CTRL,           		/* 0x07 : Sound MIC Mute Control Message */
  IPC_SND_SPKR_PHONE_CTRL,       		/* 0x08 : Sound SpeakerPhone Control Message */
  IPC_SND_HFK_AUDIO_STARTSTOP,       	/* 0x09 : HFK start/stop Message */
  IPC_SND_VOICECALL_RECORD_REPORT,      /* 0x0A : Voice recording report Message */  
  IPC_SND_AUDIO_PATH_CTRL,       		/* 0x0B : Audio Path Control Message */
  IPC_SND_AUDIO_SOURCE_CTRL,       		/* 0x0C : Audio source Control Message */
  IPC_SND_USER_SND_CONFIG,       		/* 0x0D : User Sound Configuration Message */
  IPC_SND_GAIN_CTRL,	        		/* 0x0E : Set Sound Gain Control Message */   //yjlee 2006.01.02
  IPC_SND_QUIET_MODE_CTRL,					/* 0x0F	: Set Quiet Mode Control Message */		//hnryu	2006.07.20.
  IPC_SND_DYVE_MODE_CTRL,			/* 0x10 : Set DyVE Mode Control Message */
  IPC_SND_MAX
} ipc_snd_sub_cmd_type;

/*********************************************************************************/


/*=================================================================

   SUB_CMD(1) : IPC_SND_KEY_TONE      0x01                       Sound Keytone Message

=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_CFRM      0x04     */
/*    IPC_CMD_EVENT          0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_RESP             0x02     */
/*    IPC_CMD_NOTI             0x03     */
// none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
   Set Sound Keytone
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MODE(1) | VOLUME(1) | KEY_CODE(1) |
-------------------------------------------------------------------------------------
*/

/* MODE  field */
typedef enum {
  IPC_SND_MODE_STOP,     /* 0x00 : Stop the Sound */
  IPC_SND_MODE_START,   /* 0x01 : Start the Sound */
  IPC_SND_MODE_MAX
} ipc_snd_mode_e_type;

/* VOLUME  field */
/* key tone, bell and voice volume level */
typedef enum {
  IPC_SND_VOL_MUTE,     /* 0x00 */
  IPC_SND_VOL_LEVEL_1,  /* 0x01 */
  IPC_SND_VOL_LEVEL_2,  /* 0x02 */
  IPC_SND_VOL_LEVEL_3,  /* 0x03 */
  IPC_SND_VOL_LEVEL_4,  /* 0x04 */
  IPC_SND_VOL_LEVEL_5,  /* 0x05 */
  IPC_SND_VOL_LEVEL_6,  /* 0x06 */
  IPC_SND_VOL_LEVEL_7,  /* 0x07 */
  IPC_SND_VOL_LEVEL_8,  /* 0x08 */
  IPC_SND_VOL_MAX
} ipc_snd_vol_level_e_type;


/* KEY_CODE  field */
/* key value which is used in PDA keypad */
typedef enum{
    IPC_KEYCODE_SPEAKERPHONE			=0x0A,
    IPC_KEYCODE_OK						=0x0B,
    IPC_KEYCODE_START					=0x0C,
    IPC_KEYCODE_RETURN					=0x0D,
    IPC_KEYCODE_BACKSPACE				=0x0F,
    IPC_KEYCODE_POWER				    =0x1B,
	IPC_KEYCODE_SHARP					=0x23,
	IPC_KEYCODE_STAR					=0x2A,
	IPC_KEYCODE_0						=0x30,
	IPC_KEYCODE_1						=0x31,
	IPC_KEYCODE_2						=0x32,
	IPC_KEYCODE_3						=0x33,
	IPC_KEYCODE_4						=0x34,
	IPC_KEYCODE_5						=0x35,
	IPC_KEYCODE_6						=0x36,
	IPC_KEYCODE_7						=0x37,
	IPC_KEYCODE_8						=0x38,
	IPC_KEYCODE_9						=0x39,

	IPC_KEYCODE_A                       =0x41,
	IPC_KEYCODE_B                       =0x42,
	IPC_KEYCODE_C                       =0x43,
	IPC_KEYCODE_D                       =0x44,
	IPC_KEYCODE_E                       =0x45,
	IPC_KEYCODE_F                       =0x46,

	IPC_KEYCODE_SEND					=0x50,
	IPC_KEYCODE_END						=0x51,
	IPC_KEYCODE_CLEAR					=0x52,
	IPC_KEYCODE_SAVE					=0x53,

	// Side Volume Key
	IPC_KEYCODE_VOLUP					=0x54,
	IPC_KEYCODE_VOLDOWN					=0x55,

	// Soft Key
	IPC_KEYCODE_SOFT_LEFT				=0x5B,
	IPC_KEYCODE_SOFT_RIGHT				=0x5C,

	// Navigation Key
	IPC_KEYCODE_UP						=0x63,
	IPC_KEYCODE_DOWN					=0x64,
	IPC_KEYCODE_LEFT					=0x65,
	IPC_KEYCODE_RIGHT					=0x66,

	// Mits Key
	IPC_KEYCODE_MENU					=0x70,
	IPC_KEYCODE_HOME					=0x71,
	IPC_KEYCODE_FUNC_1					=0x72,
	IPC_KEYCODE_FUNC_2					=0x73,
	IPC_KEYCODE_FUNC_3					=0x74,
	IPC_KEYCODE_FUNC_4					=0x75,
	IPC_KEYCODE_FUNC_5					=0x76,
	// IPC_KEYCODE_FUNC_RESERVED : 0x77 ~ 0x7F

	IPC_KEYCODE_VMEMO					=0x8B,
	IPC_KEYCODE_WAP						=0x8C,
	IPC_KEYCODE_CAMERA					=0x8D,
	IPC_KEYCODE_LOCK                    =0xB0
}ipc_snd_key_code_e_type;

/*=================================================================*/

/*=================================================================

   SUB_CMD(1) : IPC_SND_NOTI_TONE      0x02                      Sound Notification tone Message
   
=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_CFRM      0x04     */
/*    IPC_CMD_EVENT          0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_RESP             0x02     */
/*    IPC_CMD_NOTI             0x03     */
/* none */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
   Set Sound Nofitication Tone
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MODE(1) | VOLUME(1) | COUNT(1) | 
-------------------------------------------------------------------------------------
 | TONE_TYPE(1) |
-------------------------------------------------------------------------------------
*/

/* MODE  field */
/* see ipc_snd_mode_e_type */

/* VOLUME  field */
/* bell volume level */
/* see ipc_snd_vol_level_e_type */

/* COUNT  field */
/* repeat count */
/*
   0x00: Repeat until "Stop Sound Request" is received.
   0x01 ~ 0xFF: Repeat Count
*/

/* TONE_TYPE  field */
/* Designates the tone type to be used. */
/* Not defined yet */



/*=================================================================*/

/*=================================================================

   SUB_CMD(1) : IPC_SND_LED_CONTROL      0x03                      Sound LED Control Message
   
=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_CFRM      		0x04     */
/*    IPC_CMD_EVENT          	0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_RESP             0x02     */
/*    IPC_CMD_NOTI             0x03     */
/* none */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
   Set Sound LED Control 
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MODE(1) |
-------------------------------------------------------------------------------------
 | LED_TYPE(1) | COUNT(1) | ON_TIME(1) | OFF_TIME(1) |
-------------------------------------------------------------------------------------
*/

/* MODE  field */
/* see ipc_snd_mode_e_type */

/* LED_TYPE  field  */
typedef enum{
  IPC_SND_LED_TYPE_PRE_DEFINED,    /* 0x00 : Pre-defined LED Type */
  IPC_SND_LED_TYPE_20MS,           /* 0x01 : 20 ms type */
  IPC_SND_LED_TYPE_250MS,          /* 0x02 : 250 ms type */
  IPC_SND_LED_TYPE_MAX
} ipc_snd_led_e_type;

/* COUNT  field */
/* Repeat Count or Pre-defined LED Index */
/* When the LED_TYPE is 0x00, */
typedef enum{
  IPC_SND_LED_INDEX_PWR_ON,               /* 0x00 : Used at the time of turn the power on.
                                                                        At this time, 1sec ON / 250msec Off. */
  IPC_SND_LED_INDEX_ALARM,                  /* 0x01 : Alarm Ring, which repeats 250ms ON/OFF for 3 minutes. */
  IPC_SND_LED_INDEX_SCHEDULE,            /* 0x02 : Schedule Ring, which repeats 250ms ON/OFF for 40 seconds. */
  IPC_SND_LED_INDEX_CONNECT_TONE,   /* 0x03 : LED informing that the phone is connected,
                                                                             which repeats 250ms ON/OFF for three times. */
  IPC_SND_LED_INDEX_VOICE_RECORDING,       /* 0x04 : Used for recording the voice dialing/memo,
                                                                 which repeats 60ms ON/20ms OFF until the Stop Sound Request is received. */
  IPC_SND_LED_INDEX_VOICE_PLAYING,         /* 0x05 : Used for playing back the voice dialing/memo,
                                                                           which repeats 20ms ON/OFF until the Stop Sound Request is received. */
  IPC_SND_LED_INDEX_SMS,         /* 0x06 : Used for the SMS reception, which repeats 250ms ON/OFF eight times. */
  IPC_SND_LED_INDEX_INCOMING_CALL,   /* 0x07 : Used to alert that there is an incoming call,
                                                                                       which repeats 250ms ON/OFF unlimitedly. */
  IPC_SND_LED_INDEX_MAX
} ipc_snd_led_index_e_type;

/* When the LED_TYPE is 0x01 or 0x02 */
/* 0x00: Repeat until "Stop Sound Request" is received.
** 0x01 ~ 0xFF: Repeat Count
*/

/* ON_TIME  field */
/* ON Time when the LED is blinking (when the LED_TYPE is 0x00, this field is ignored.)
** Designate the ON time by the unit of 20ms or of 250ms according to the LED_TYPE.
** (Designate the ON time either as 20 ~5,100ms or as 250ms ~ 63,750ms.)
*/

/* OFF_TIME  field */
/* OFF Time when the LED is blinking (when the LED_TYPE is 0x00, this field is ignored.)
** Designate the OFF time by the unit of 20ms or of 250ms according to the LED_TYPE.
** (Designate the OFF time either as 20 ~5,100ms or as 250ms ~ 63,750ms.)
*/



/*=================================================================*/

/*=================================================================

   SUB_CMD(1) : IPC_SND_VIBRATOR_CONTROL      0x04           Sound Vibrator Control Message
   
=================================================================*/

/*    IPC_CMD_EXEC            0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_CFRM      0x04     */
/*    IPC_CMD_EVENT          0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_RESP             0x02     */
/*    IPC_CMD_NOTI             0x03     */
/* none */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
   Set Sound Vibrator Control 
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MODE(1) |
-------------------------------------------------------------------------------------
 | VIB_TYPE(1) | COUNT(1) | ON_TIME(1) | OFF_TIME(1) |
-------------------------------------------------------------------------------------
*/
/* MODE  field */
/* see ipc_snd_mode_e_type */

/* VIB_TYPE  field */
typedef enum{
  IPC_SND_VIB_TYPE_PRE_DEFINED,     /* 0x00 : Pre-define Vibrate Type */
  IPC_SND_VIB_TYPE_100MS,                /* 0x01 : 100 ms Type */
  IPC_SND_VIB_TYPE_MAX
} ipc_snd_vib_e_type;

/* COUNT  field */
/* Repeat Count or Pre-defined Vibrate Type */
/* When the VIB_TYPE is 0x00, */
typedef enum{
  IPC_SND_VIB_INDEX_INCOMING_CALL,  /* 0x00 : Repeat 1-sec ON/3-sec OFF
                                            until the Stop Sound Request is received.
                                            When incoming call is received it works to vibrates. */
  IPC_SND_VIB_INDEX_ONE_VIB,             /* 0x01 : 1-sec ON/OFF Only 1 times 
                                            (when vibration is selected in the Bell/Vibration menu) */
  IPC_SND_VIB_INDEX_MAX
} ipc_snd_vib_index_e_type;

/* When the VIB_TYPE is 0x01, */
/* 0x00: Repeat until  Stop Sound Request " is received.
** 0x01 ~ 0xFF: Repeat Count 
*/

/* ON_TIME  field */
/* Vibrate ON Time (100ms unit) */

/* OFF_TIME  field */
/* Vibrate OFF Time (100ms unit) */



/*=================================================================*/

/*=================================================================

   SUB_CMD(1) : IPC_SND_SPKR_VOLUME_CTRL      0x05       Sound Speaker Volume Control Message
   
=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_CFRM       0x04     */
/*    IPC_CMD_EVENT           0x05     */
/*    IPC_CMD_INDI              0x01     */
/*    IPC_CMD_NOTI             0x03     */
/* none */




/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
   Get Sound Speaker Volume Control 
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | VOLUME_TYPE(1) |
-------------------------------------------------------------------------------------
*/

/* VOLUME_TYPE  field */
/* ipc_snd_vol_device_e_type  4bit,
** ipc_snd_vol_snd_e_type  4bit  Ѵ.
*/
typedef enum{
  IPC_SND_VOL_DEV_RECEIVER 	= 0x00,         /* Receiver */
  IPC_SND_VOL_DEV_SPEAKER	= 0x10,         /* Speaker  */
  IPC_SND_VOL_DEV_HFK		= 0x20,         /* Handsfree */
  IPC_SND_VOL_DEV_HEADSET	= 0x30,         /* Headset( Earphone ) */
  IPC_SND_VOL_DEV_BT        = 0x40,	        /* Bluetooth */	
  IPC_SND_VOL_DEV_EC        = 0xA0,         /*Echo Canceller mode*/
  IPC_SND_VOL_DEV_MAX  	    
} ipc_snd_vol_device_e_type;

typedef enum {
  IPC_SND_VOL_SND_VOICE = 0x01,      /* Voice */
  IPC_SND_VOL_SND_KEYTONE,              /* Key Tone */
  IPC_SND_VOL_SND_BELL,                     /* Bell */
  IPC_SND_VOL_SND_MSG,                      /* Message( SMS ) */
  IPC_SND_VOL_SND_ALARM,                  /* Alarm */
  IPC_SND_VOL_SND_MISC,                    /* Miscelloneous PDA Sound */
  IPC_SND_VOL_SND_MAX
} ipc_snd_vol_snd_e_type;

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
   Set Sound Speaker Volume Control 
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | VOLUME_TYPE(1) | VOLUME(1) |
-------------------------------------------------------------------------------------
*/

/* VOLUME_TYPE  field */
/* see ipc_snd_vol_device_e_type,
** see ipc_snd_vol_snd_e_type
*/


/* VOLUME  field */
/* New volume level */
/* see ipc_snd_vol_level_e_type */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
   Sound Speaker Volume Control Response
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | NUM_RECORD(1) | VOLUME_TYPE(1) | VOLUME(1) |
-------------------------------------------------------------------------------------
*/

/* NUM_RECORD */
/* The number of following records */
/* see MAX_SND_VOL_TYPE */

/* VOLUME_TYPE */
/* see ipc_snd_vol_device_e_type,
** see ipc_snd_vol_snd_e_type
*/

/* VOLUME */
/* New volume level */
/* see ipc_snd_vol_level_e_type */



/*=================================================================*/

/*=================================================================

   SUB_CMD(1) : IPC_SND_MIC_GAIN_CTRL      0x06                    Sound MIC Gain Control Message

=================================================================*/
/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_CFRM        0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_NOTI             0x03     */
/* none */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
   Get Sound MIC Gain Control
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MIC_TYPE(1) |
-------------------------------------------------------------------------------------
*/

/* MIC_TYPE  field */
typedef enum{
  IPC_SND_MIC_TYPE_HANDSET,        /* 0x00 : Handset */
  IPC_SND_MIC_TYPE_HEADSET,        /* 0x01 : Headset ( Earphone ) */
  IPC_SND_MIC_TYPE_HANDSFREE,   /* 0x02 : Handsfree ( Carkit ) */
  IPC_SND_MIC_TYPE_ALL,                /* 0x03 : All Device Type */
  IPC_SND_MIC_TYPE_MAX
} ipc_snd_mic_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
   Set Sound MIC Gain Control
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MIC_TYPE(1) | MIC_GAIN(1) |
-------------------------------------------------------------------------------------
*/

/* MIC_TYPE */
/* see ipc_snd_mic_e_type */

/* MIC_GAIN */
/* Not defined yet */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
   Sound MIC Gain Control Response
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | NUM_RECORD(1) | MIC_TYPE(1) | MIC_GAIN(1) |
-------------------------------------------------------------------------------------
*/

/* NUM_RECORD */
/* The number of following records */
/* see MAX_SND_MIC_TYPE */

/* MIC_TYPE */
/* see ipc_snd_mic_e_type */

/* MIC_GAIN */
/* Not defined yet */



/*=================================================================*/

/*=================================================================

   SUB_CMD(1) : IPC_SND_MIC_MUTE_CTRL      0x07                  Sound MIC Mute Control Message

=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_CFRM       0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI              0x01     */
/*    IPC_CMD_NOTI             0x03     */
// none



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
   Get Sound MIC Mute Control
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
   Set Sound MIC Mute Control
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MUTE_STATUS(1) |
-------------------------------------------------------------------------------------
*/

/* MUTE_STATUS  field */
/* Mute control */
typedef enum{
  IPC_SND_MUTE_STATUS_UNMUTE,   /* 0x00 : Unmute */
  IPC_SND_MUTE_STATUS_MUTE,     /* 0x01 : Mute */
  IPC_SND_MUTE_STATUS_MAX
} ipc_snd_mute_status_e_type;



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
   Sound MIC Mute Control Response
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MUTE_STATUS(1) |
-------------------------------------------------------------------------------------
*/

/* MUTE_STATUS */
/* see ipc_snd_mute_status_e_type */

/*=================================================================*/



/*=================================================================

   SUB_CMD(1) : IPC_SND_SPKR_PHONE_CTRL      0x08        Sound Speaker-Phone Control Message
   
=================================================================*/
/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_RESP             0x02     */
/*    IPC_CMD_NOTI             0x03     */
/* none */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
   Set Sound Speaker-Phone Control 
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SPKR_STATUS(1) |
-------------------------------------------------------------------------------------
*/

/* SPKR_STATUS  field */
typedef enum{
  IPC_SND_SPKR_PHONE_OFF,      /* 0x00 : Speaker Phone Off */
  IPC_SND_SPKR_PHONE_ON,       /* 0x01 : Speaker Phone On */
  IPC_SND_SPKR_STATUS_MAX
} ipc_snd_spkr_phone_status_e_type;

/*=================================================================*/




/*=================================================================

   SUB_CMD(1) : IPC_SND_HFK_AUDIO_STARTSTOP      0x09       HFK start/stop Message 
   
=================================================================*/
/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_RESP             0x02     */
/*    IPC_CMD_NOTI             0x03     */
/* none */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
   Set HFK start/stop Message  
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | HFK_AUDIO_ACTION(1) | HFK_AUDIO_TYPE(1) |
-------------------------------------------------------------------------------------
*/

/* HFK_AUDIO_ACTION  field */
typedef enum{
  IPC_SND_HFK_AUDIO_ACTION_STOP,      /* 0x00 : Speaker Phone Off */
  IPC_SND_HFK_AUDIO_ACTION_START,      /* 0x01 : Speaker Phone On */
  IPC_SND_HFK_AUDIO_ACTION_MAX
} ipc_snd_hfk_audio_action_e_type;

/* HFK_AUDIO_TYPE  field */
typedef enum{
  IPC_SND_HFK_AUDIO_PDA_SOUND = 0x01,    /* 0x01 : HFK aution pda sound */
  IPC_SND_HFK_AUDIO_VOICE_RECORDING,     /* 0x02 : HFK aution pda recording */
  IPC_SND_HFK_AUDIO_TYPE_MAX
} ipc_snd_hfk_audio_type_e_type;

/*=================================================================*/



/*=================================================================

   SUB_CMD(1) : IPC_SND_VOICECALL_RECORD_REPORT      0x0A       Voice recording report Message
   
=================================================================*/
/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_RESP             0x02     */
/*    IPC_CMD_NOTI             0x03     */
/* none */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
   Set Sound Voice recording report 
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | RECORD(1) |
-------------------------------------------------------------------------------------
*/

/* SPKR_STATUS  field */
typedef enum{
  IPC_SND_VOICE_RECORDING_STOP,      /* 0x00 : Voice recording start */
  IPC_SND_VOICE_RECORDING_START,      /* 0x01 : Voice recording stop */
  IPC_SND_VOICE_RECORDING_MAX
} ipc_snd_voice_record_report_e_type;



/*=================================================================

   SUB_CMD(1) : IPC_SND_AUDIO_PATH_CTRL      0x0B       Audio Path Control Message
   
=================================================================*/
/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_RESP             0x02     */
/* none */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
   Set Sound audio path control message
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | AUDIO_PATH(1) |
-------------------------------------------------------------------------------------
*/
/* AUDIO_PATH field */
typedef enum{
  IPC_SND_AUDIO_PATH_HANDSET =0x01,   /* 0x01 : Audio Path is Handset */
  IPC_SND_AUDIO_PATH_HEADSET,  	      /* 0x02 : Audio Path is Earphone */
  IPC_SND_AUDIO_PATH_HFK,  	          /* 0x03 : Audio Path is HFK */
  IPC_SND_AUDIO_PATH_BLUETOOTH,       /* 0x04 : Audio Path is Bluetooth */
  IPC_SND_AUDIO_PATH_STEREO_BLUETOOTH,           /* 0x05 : Audio Path is Stereo Bluetooth */
  IPC_SND_AUDIO_PATH_SPEAKER_PHONE,			/* 0x06  : Audio path is speaker */
  IPC_SND_AUDIO_PATH_3P5PI,                   /*0x07 : 3.5pi headset*/
  IPC_SND_AUDIO_PATH_MAX,  	
} ipc_snd_audio_path_e_type;



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                    0x03

 DESCRIPTION :
   Notification Sound audio path control message

 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | AUDIO_PATH(1) |
-------------------------------------------------------------------------------------
*/
/* AUDIO_PATH field */
/* see  ipc_snd_audio_path_e_type */

/*=================================================================*/



/*=================================================================

   SUB_CMD(1) : IPC_SND_AUDIO_SOURCE_CTRL      0x0C       Audio Source Control Message
   
=================================================================*/
/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_SET              0x03     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_RESP             0x02     */
/* none */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                    0x03

 DESCRIPTION :
   Notification Sound audio source control message

 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | AUDIO_SOURCE(1) |
-------------------------------------------------------------------------------------
*/
/* AUDIO_SOURCE field */
typedef enum{
  IPC_SND_AUDIO_SOURCE_PHONE =0x01,   /* 0x01 : Audio source is Phone */
  IPC_SND_AUDIO_SOURCE_PDA,  	      /* 0x02 : Audio source is PDA */
  IPC_SND_AUDIO_SOURCE_MAX,  	
} ipc_snd_audio_source_e_type;

/*=================================================================*/



// JBG 2005.07.19 bgein
/*=================================================================

   SUB_CMD(1) : IPC_SND_USER_SND_CONFIG      0x0D       User Sound Configuration Message 
   
=================================================================*/
/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_NOTI             0x03     */
/* none */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                    0x02

 DESCRIPTION :
   Get Sound audio source control message

 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | USER_SOUND_MASK(2) |
-------------------------------------------------------------------------------------
*/

/* USER_SOUND_MASK field */
/* Process this field as bit mask */
typedef enum{
  IPC_SND_USER_1MIN = 0x0001,           /* 1 Minute Alert Sound in conversation */
  IPC_SND_USER_SVC_CHANGE = 0x0002,  /* Service Change Alert Sound */
  IPC_SND_USER_CONNECT = 0x0004,    /* Connect Tone */
  IPC_SND_USER_DISCONNECT = 0x0008,  /* Disconnect Tone */
  IPC_SND_USER_CALL_FADE = 0x00010,   /* Call Fade Tone */
  IPC_SND_USER_ROAM = 0x0020,              /* Roam Alert Tone */
  IPC_SND_USER_VOICE_PRIVACY = 0x0040,              /* voice privacy tone*/
  IPC_SND_USER_MAX = 0x8000
} ipc_snd_user_snd_mask_e_type;




/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                    0x03

 DESCRIPTION :
   Set Sound audio source control message

 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | USER_SOUND_MASK(2) | USER_SOUND_VALUE(2)
-------------------------------------------------------------------------------------
*/

/* USER_SOUND_MASK field */
/* see ipc_snd_user_snd_mask_e_type */


/* USER_SOUND_MASK field */
/* USER_SOUND_VALUE field */
/* Process this field as bit mask according to USER_SOUND_MASK */
typedef enum{
  IPC_SND_USER_SND_OFF,   /* 0x00 : User Config Sound Off */
  IPC_SND_USER_SND_ON,     /* 0x01 : User Config Sound On */
  IPC_SND_USER_SND_NO_CHANGE,     /* 0x02 : NO Change*/
  IPC_SND_USER_SND_MAX, 
} ipc_snd_user_snd_value_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                    0x02

 DESCRIPTION :
   Sound audio source control Response message

 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | USER_SOUND_MASK(2) | USER_SOUND_VALUE(2)
-------------------------------------------------------------------------------------
*/

/* USER_SOUND_MASK field */
/* see ipc_snd_user_snd_mask_e_type */


/* USER_SOUND_MASK field */
/* ipc_snd_user_snd_value_e_type */

/*=================================================================*/
#define IPC_SND_ONE_MINUTE_OFF			0x00
#define IPC_SND_ONE_MINUTE_ON			0x01
#define IPC_SND_NO_CHANGE_STATE			0x02
// JBG 2005.07.19 end

//yjlee 2006.01.02 start
/*=================================================================

   SUB_CMD(1) : IPC_SND_GAIN_CTRL      0x0E                    Sound Gain Control Message

=================================================================*/

/*    IPC_CMD_CFRM              0x04     */
/*    IPC_CMD_EVENT             0x05     */
/*    IPC_CMD_INDI               0x01     */

/* none */

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_EXEC                     0x01

 DESCRIPTION :
   Execution Phone Sound Parameter
   
 FORMAT :
 CONFIG_DEV_TYPE 0x00 ~ 0x40 type.
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CONFIG_DEV_TYPE(1) | RX_FLAG(1) | 
-------------------------------------------------------------------------------------
 | CONFIG_PAR_TYPE(1) | CONFIG_VALUE(4)
-------------------------------------------------------------------------------------


 CONFIG_DEV_TYPE 0xA0(EC) type.
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CONFIG_DEV_TYPE(1) |EC_MODE(1) | 
-------------------------------------------------------------------------------------
 | CONFIG_PAR_TYPE(1) | CONFIG_VALUE(4)
-------------------------------------------------------------------------------------
*/
/* CONFIG_DEV_TYPE  field */
/* see ipc_snd_vol_device_e_type */

typedef enum {				
  IPC_SND_CONFIG_TX_AUDIO_NV_REBUILD = 0x01,				
  IPC_SND_CONFIG_TX_MIC_AMP_GAIN,				
  IPC_SND_CONFIG_TX_GAIN,                    				
  IPC_SND_CONFIG_TX_ST_GAIN,                       				
  IPC_SND_CONFIG_TX_VOLUME,				
  IPC_SND_CONFIG_TX_DTMF_GAIN,				
  IPC_SND_CONFIG_TX_AGC_STATIC_GAIN,   				
  IPC_SND_CONFIG_TX_AGC_EXP_THRES,      				
  IPC_SND_CONFIG_TX_AGC_EXP_SLOPE,      				
  IPC_SND_CONFIG_TX_AGC_COMP_THRES,				
  IPC_SND_CONFIG_TX_AGC_COMP_SLOPE,				
  IPC_SND_CONFIG_TX_AGC_ON_OFF,				
  IPC_SND_CONFIG_TX_FILTER_0,
  IPC_SND_CONFIG_TX_FILTER_1,
  IPC_SND_CONFIG_TX_FILTER_2,
  IPC_SND_CONFIG_TX_FILTER_3,
  IPC_SND_CONFIG_TX_FILTER_4,
  IPC_SND_CONFIG_TX_FILTER_5,
  IPC_SND_CONFIG_TX_FILTER_6,
  IPC_SND_CONFIG_TX_MAX				
} ipc_snd_gain_config_tx_parameter_type;				

typedef enum {				
  IPC_SND_CONFIG_RX_AUDIO_NV_REBUILD = 0x01,				
  IPC_SND_CONFIG_RX_GAIN,                    				
  IPC_SND_CONFIG_RX_VOLUME,         				
  IPC_SND_CONFIG_RX_DTMF_GAIN,				
  IPC_SND_CONFIG_RX_AGC_STATIC_GAIN,   				
  IPC_SND_CONFIG_RX_AGC_EXP_THRES,      				
  IPC_SND_CONFIG_RX_AGC_EXP_SLOPE,      				
  IPC_SND_CONFIG_RX_AGC_COMP_THRES,				
  IPC_SND_CONFIG_RX_AGC_COMP_SLOPE,				
  IPC_SND_CONFIG_RX_AGC_ON_OFF,				
  IPC_SND_CONFIG_RX_FILTER_0,
  IPC_SND_CONFIG_RX_FILTER_1,
  IPC_SND_CONFIG_RX_FILTER_2,
  IPC_SND_CONFIG_RX_FILTER_3,
  IPC_SND_CONFIG_RX_FILTER_4,
  IPC_SND_CONFIG_RX_FILTER_5,
  IPC_SND_CONFIG_RX_FILTER_6,
  IPC_SND_CONFIG_RX_MAX				
} ipc_snd_gain_config_rx_parameter_type;				

typedef enum {				
  IPC_SND_CONFIG_FAREND_HO_THRES = 0x01,				
  IPC_SND_CONFIG_DOUBLETALK_HO_THRES,
  IPC_SND_CONFIG_STARTUP_MUTE_HO_TERS,
  IPC_SND_CONFIG_STARTUP_MUTE_MODE,
  IPC_SND_CONFIG_STARTUP_ERLE_THRES, //0x05
  IPC_SND_CONFIG_EC_MAX				
} ipc_snd_gain_config_ec_parameter_type;				

/* RX_FLAG  field */
/* RX_FLAG = 0 :TX parameter */
/* RX_FLAG = 1 :RX parameter */

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
   Get Phone Sound Parameter
   
 FORMAT :
 CONFIG_DEV_TYPE 0x00 ~ 0x40 type.
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CONFIG_DEV_TYPE(1) | RX_FLAG(1) | 
-------------------------------------------------------------------------------------
 | CONFIG_PAR_TYPE(1) | CONFIG_VALUE(4)
-------------------------------------------------------------------------------------


 CONFIG_DEV_TYPE 0xA0(EC) type.
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CONFIG_DEV_TYPE(1) |EC_MODE(1) | 
-------------------------------------------------------------------------------------
 | CONFIG_PAR_TYPE(1) | CONFIG_VALUE(4)
-------------------------------------------------------------------------------------
*/

/* CONFIG_DEV_TYPE field */
/* see ipc_snd_vol_device_e_type */


/* CONFIG_PAR_TYPE field */
/* ipc_snd_gain_config_tx_parameter_type */
/* ipc_snd_gain_config_rx_parameter_type */
/* ipc_snd_gain_config_ec_parameter_type*/

/* RX_FLAG  field */
/* RX_FLAG = 0 :TX parameter */
/* RX_FLAG = 1 :RX parameter */

/* EC_MODE field   */
/* 0x01 : ESEC       */
/* 0x02 : HEADSET */
/* 0x03 : AEC         */
/* 0x04 : SPEAKER */
/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
   Set Phone Sound Parameter
   
 FORMAT :
 CONFIG_DEV_TYPE 0x00 ~ 0x40 type.
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CONFIG_DEV_TYPE(1) | RX_FLAG(1) | 
-------------------------------------------------------------------------------------
 | CONFIG_PAR_TYPE(1) | CONFIG_VALUE(4)
-------------------------------------------------------------------------------------


 CONFIG_DEV_TYPE 0xA0(EC) type.
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CONFIG_DEV_TYPE(1) |EC_MODE(1) | 
-------------------------------------------------------------------------------------
 | CONFIG_PAR_TYPE(1) | CONFIG_VALUE(4)
-------------------------------------------------------------------------------------
*/

/* CONFIG_DEV_TYPE field */
/* see ipc_snd_vol_device_e_type */


/* CONFIG_PAR_TYPE field */
/* ipc_snd_gain_config_tx_parameter_type */
/* ipc_snd_gain_config_rx_parameter_type */
/* ipc_snd_gain_config_ec_parameter_type*/

/* RX_FLAG  field */
/* RX_FLAG = 0 :TX parameter */
/* RX_FLAG = 1 :RX parameter */

/* EC_MODE field   */
/* 0x01 : ESEC       */
/* 0x02 : HEADSET */
/* 0x03 : AEC         */
/* 0x04 : SPEAKER */

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
     Phone Sound Parameter Response
 FORMAT :
-------------------------------------------------------------------------------------
  | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  CONFIG_VALUE(4) | CONFIG_STATUS(1) 
-------------------------------------------------------------------------------------
*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
     Notification Phone Sound Parameter
 FORMAT :
-------------------------------------------------------------------------------------
  | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  CONFIG_VALUE(4) | CONFIG_STATUS(1) 
-------------------------------------------------------------------------------------
*/

/* CONFIG_STATUS  field */
/* 0 : Fail */
/* 1 : Sucess */

//yjlee 2006.01.02 end

/*=================================================================

   SUB_CMD(1) : IPC_SND_QUIET_MODE_CTRL      0x0F        Sound Quiet Mode Control Message
   
=================================================================*/
/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_RESP             0x02     */
/*    IPC_CMD_NOTI             0x03     */
/* none */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
   Set Sound Quiet Mode Control 
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | QUIET_MODE(1) |
-------------------------------------------------------------------------------------
*/

/* SPKR_STATUS  field */
typedef enum{
  IPC_SND_QUIET_MODE_OFF,      /* 0x00 : Quiet mode off */
  IPC_SND_QUIET_MODE_ON,       /* 0x01 : Quiet mode On */
  IPC_SND_QUIET_MODE_MAX
} ipc_snd_quiet_mode_e_type;

/*=================================================================

   SUB_CMD(1) : IPC_SND_DYVE_MODE_CTRL      0x10       DyVE Mode Control Message
   
=================================================================*/
/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_NOTI             0x03     */
/* none */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
   Get DyVE Mode 
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | DYVE_MODE(1) |
-------------------------------------------------------------------------------------
*/

/* DyVe Mode field */
typedef enum{
  IPC_SND_DYVE_MODE_OFF,      /* 0x00 : DyVE mode off */
  IPC_SND_DYVE_MODE_ON,       /* 0x01 : DyVE mode On */
  IPC_SND_DYVE_MODE_MAX
} ipc_snd_dyve_mode_e_type;

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
   Set DyVE Mode  
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | DYVE_MODE(1) |
-------------------------------------------------------------------------------------
*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
   Response DyVE Mode
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | DYVE_MODE(1) |
-------------------------------------------------------------------------------------
*/

/*=================================================================*/
//hnryu 2006.07.20. end

/*********************************************************************************

                                            Sub Command of IPC_MISC_CMD[0x0A]

**********************************************************************************/
typedef enum{
  IPC_MISC_ME_VERSION=0x01, 		/* 0x01 : Mobile Equipment Version Message */
  IPC_MISC_ME_IMSI,                 /* 0x02 : Mobile Equipment IMSI Message */
  IPC_MISC_ME_SN,                   /* 0x03 : Mobile Equipment Serial Number Message*/
  IPC_MISC_KEY_EVENT_PROCESS,         /* 0x04 : Key event process Message */
  IPC_MISC_TIME_INFO,               /* 0x05 : Current Time Zone Message   */
  IPC_MISC_NAM_INFO,            /* 0x06 : CDMA NAM Information Message */
  IPC_MISC_VCALL_CHANNEL_ID,        /* 0x07 : Video call channel id*/
  IPC_MISC_PHONE_DEBUG,             /* 0x08 : Phone debug Message*/
  IPC_MISC_FUS,                             /*0x09:  FUS start  Message For LINUX */  
  IPC_MISC_MODEM_INT_MODE = 0x0B,  /*0x0B: Modem Interface Mode*/
  IPC_MISC_MODEM_RMPC_ATI = 0x0C, /*0x0C: Modem RMPC ATI toggle*/
  IPC_MISC_MAX
} ipc_misc_sub_cmd_type;

/*********************************************************************************/

/*=================================================================

   SUB_CMD(1) : IPC_MISC_ME_VERSION      0x01            Mobile Equipment Version Message
   
=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_SET               0x03     */
/*    IPC_CMD_CFRM       0x04     */
/*    IPC_CMD_EVENT           0x05     */
/*    IPC_CMD_INDI              0x01     */
/*    IPC_CMD_NOTI             0x03     */
/* none */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
     Get ME Version
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | VER_MASK(1)
-------------------------------------------------------------------------------------
*/
//Version Masking
#define IPC_MISC_MASK_VER_SW                           0x01
#define IPC_MISC_MASK_VER_HW                           0x02
#define IPC_MISC_MASK_VER_RF_CAL                    0x04
#define IPC_MISC_MASK_VER_PRODUCT_CODE       0x08
#define IPC_MISC_MASK_VER_MODEL_ID                0x10 //CDMA Only
#define IPC_MISC_MASK_VER_PRL                          0x20 //CDMA Only
#define IPC_MISC_MASK_VER_ERI                          0x40 //CDMA Only
#define IPC_MISC_MASK_VER_ALL                          0xFF


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
     ME Version Response
 FORMAT :
-------------------------------------------------------------------------------------
  | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | VER_MASK(1) | SW_VERSION(32) | HW_VERSION(32) | CAL_DATE(32) | 
-------------------------------------------------------------------------------------
 PRODUCT_CODE(32) | MODEL_ID(17) | PRL_NAM_NUM(1) | PRL_VER(17 * PRL_NAM_NUM) | ERI_NAM_NUM(1) | ERI_VER(17 * ERI_NAM_NUM)
-------------------------------------------------------------------------------------
*/

/* SW_VERSION  field */
/* Maximum 32 character, terminated by null */
/* see MAX_MISC_SW_VERSION_LEN */

/* HW_VERSION  field */
/* Maximum 32 character, terminated by null */
/* see MAX_MISC_HW_VERSION_LEN */

/* CAL_DATE  field */
/* Maximum 32 character, terminated by null */
/* see MAX_MISC_RF_CAL_DATE_LEN */

/* PRODUCT_CODE  field */
/* Maximum 32 character, terminated by null */
/* see MAX_MISC_PRODUCT_CODE_LEN */

/*=================================================================*/

/*=================================================================

   SUB_CMD(1) : IPC_MISC_ME_IMSI      0x02                          Mobile Equipment IMSI Message

=================================================================*/
/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_SET              0x03     */
/*    IPC_CMD_CFRM      0x04     */
/*    IPC_CMD_EVENT          0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_NOTI             0x03     */
/* none */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
     Get ME IMSI 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
     Get ME IMSI 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | IMSI_LEN(1) | IMSI(x) |
-------------------------------------------------------------------------------------
*/

/* IMSI_LEN */
/* IMSI length */

/* IMSI */
/* Maximum 15 bytes */
/* see MAX_MISC_IMSI_LEN */


/*=================================================================*/

/*=================================================================

   SUB_CMD(1) : IPC_MISC_ME_SN      0x03                        Mobile Equipment Serial Number Message
   
=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_CFRM       0x04     */
/*    IPC_CMD_EVENT           0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_NOTI             0x03     */
/* none */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
     Get ME Serial Number
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SN_INDEX(1)
-------------------------------------------------------------------------------------
*/
//SN_INDEX
#define IPC_MISC_ME_IMEI        0x01
#define IPC_MISC_ME_ESN         0x02
#define IPC_MISC_ME_MEID       0x03

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
     ME Serial Number Write
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SN_INDEX(1) | SERIAL_NUMBER_LEN(1) | SERIAL_NUMBER(X) |
-------------------------------------------------------------------------------------
*/

/* SERIAL_NUMBER_LEN  field */
/* SERIAL NUMBER length */

/* SERIAL_NUMBER  field */
//#define IPC_MISC_ME_IMEI        0x01
/* Maximum length :string type 
   The serial Number of mobile equipment */
//#define IPC_MISC_ME_ESN         0x02
    //4bytes(dword)
//#define IPC_MISC_ME_MEID       0x03
    //8bytes(hi(dword) + lo(dword))


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
     ME Serial Number Response
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SN_INDEX(1) | SERIAL_NUMBER_LEN(1) | SERIAL_NUMBER(X) |
-------------------------------------------------------------------------------------
*/

/* SERIAL_NUMBER_LEN  field */
/* SERIAL NUMBER length */

/* SERIAL_NUMBER  field */
/* Maximum length :string type 
   The serial Number of mobile equipment */
/* see MAX_MISC_ME_SN_LEN */

/*================================================================================*/



/*=================================================================================

   SUB_CMD(1) : IPC_MISC_KEY_EVENT_PROCESS      0x04 	Key event process Message
   
=================================================================================*/

/*    IPC_CMD_EXEC             		0x01     */
/*    IPC_CMD_GET               	0x02     */
/*    IPC_CMD_CFRM 			      	0x04     */
/*    IPC_CMD_EVENT         	  	0x05     */
/*    IPC_CMD_INDI             		0x01     */
/*    IPC_CMD_RESP             		0x02     */
/* none */

typedef enum{
  IPC_MISC_KEY_MODE_DIAG=0x00,      /* 0x00 : for Diagnostic Monitor (DIAG or DM) */
  IPC_MISC_KEY_MODE_ATCKPD,         /* 0x01 : for AT Command (AT+CKPD) */      
  IPC_MISC_KEY_MODE_FACTORY,        /* 0x02 : for samsung factory test */  
  IPC_MISC_KEY_MODE_UTS,            /* 0x03 : for Verizon's UTS Test */  
  IPC_MISC_KEY_MODE_SLATE,          /* 0x04 : for Sprint's SLATE Test */  
  IPC_MISC_KEY_MODE_MAX     
} ipc_misc_key_mode_e_type;

typedef enum{
  IPC_MISC_KEY_STATUS_RELEASE=0x00,      /* 0x00 : key code release */
  IPC_MISC_KEY_STATUS_HOLD,  	    	  /* 0x01 : key code hold */  	
  IPC_MISC_KEY_STATUS_MAX
} ipc_misc_key_status_e_type;

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_EVENT                     0x05

 DESCRIPTION :
     Key event process Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | KEY_MODE(1) | KEY_CODE(1) |  KEY_HOLD(1) |
-------------------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
     Key event process Message
 FORMAT :
 
 // KEY_MODE is not IPC_MISC_KEY_MODE_ATCKPD
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | KEY_MODE(1) | KEY_CODE(1) |  KEY_HOLD(1) |
-------------------------------------------------------------------------------------

// KEY_MODE is IPC_MISC_KEY_MODE_ATCKPD
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | KEY_MODE(1) | 
-------------------------------------------------------------------------------------
 | KEY_PRESS_TIME(4) | KEY_PAUSE_TIME(4) | KEY_STR_LEN(1) | KEY_STRING(Variable) |
-------------------------------------------------------------------------------------

*/
//Max length of KEY_STRING : 250 bytes

/*=================================================================*/


/*========================================================================================

   SUB_CMD(1) : IPC_MISC_TIME_INFO      0x05  Mobile Equipment Time Info Message

=========================================================================================*/
/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_EVENT            0x05     */
/* none */

/* TIME_INFO_TYPE field */
typedef enum {
  IPC_MISC_TIME_INFO_INVALID=0x00,      /* 0x00 : Invalid Time Information */
  IPC_MISC_TIME_INFO_NETWORK_CDMA=0x01,      /* 0x01 : Time Information from CDMA network */
  IPC_MISC_TIME_INFO_NETWORK_GSM=0x02,      /* 0x02 : Time Information from GSM network */
  IPC_MISC_TIME_INFO_RTCLINE=0x03,      /* 0x03 : RTC information from the Line */
  IPC_MISC_TIME_INFO_RTCBACKUP=0x04,    /* 0x04 : RTC backup Information from phone */
  IPC_MISC_TIME_INFO_NO_NITZ = 0x05,   /* 0x05 : no time information*/
  IPC_MISC_TIME_INFO_MAX
} ipc_misc_time_info_type_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
     Get TIME ZONE 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
     Set TIME ZONE 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | TIME_INFO_TYPE(1) | DAYLIGHT_VALID (1) | 
-------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------
 | YEAR(1) | MONTH(1) | DAY(1) | HOUR(1) | MIMUTE (1) | SECOND(1) | TIME_ZONE(1) | DAYLIGHT_ADJUST(1) | DAY_OF_WEEK(1)
----------------------------------------------------------------------------------------------------------------------
*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_INDI                     0x01

 DESCRIPTION :
     Request TIME ZONE to PDA
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_CFRM                     0x04

 DESCRIPTION :
     Response of Indication 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | TIME_INFO_TYPE(1) | DAYLIGHT_VALID (1) | 
-------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------
 | YEAR(1) | MONTH(1) | DAY(1) | HOUR(1) | MIMUTE (1) | SECOND(1) | TIME_ZONE(1) | DAYLIGHT_ADJUST(1) | DAY_OF_WEEK(1)
-----------------------------------------------------------------------------------------------------

TIME_INFO_TYPE : see  ipc_misc_time_info_type_type

*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
     Respons TIME ZONE
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | TIME_INFO_TYPE(1) | DAYLIGHT_VALID (1) | 
-------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------
 | YEAR(1) | MONTH(1) | DAY(1) | HOUR(1) | MIMUTE (1) | SECOND(1) | TIME_ZONE(1) | DAYLIGHT_ADJUST(1) | DAY_OF_WEEK(1)
-----------------------------------------------------------------------------------------------------

TIME_INFO_TYPE : see  ipc_misc_time_info_type_type

*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
     Notification TIME ZONE
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | TIME_INFO_TYPE(1) | DAYLIGHT_VALID (1) | 
-------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------
 | YEAR(1) | MONTH(1) | DAY(1) | HOUR(1) | MIMUTE (1) | SECOND(1) | TIME_ZONE(1) | DAYLIGHT_ADJUST(1) | DAY_OF_WEEK(1)
-----------------------------------------------------------------------------------------------------

TIME_INFO_TYPE : see  ipc_misc_time_info_type_type

*/



/*========================================================================================

   SUB_CMD(1) : IPC_MISC_NAM_INFO            0x06 : CDMA NAM Information

=========================================================================================*/
/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_SET              0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_NOTI             0x03     */
/* none */

/* NAM Information MASK field*/
#define	NAM_MASK_TOTAL			0x0001
#define	NAM_MASK_CURRENT_INDEX  	0x0002
#define	NAM_MASK_REQUEST_INDEX	        0x0004
#define	NAM_MASK_AUTONAM		0x0008
#define	NAM_MASK_MCC			0x0010
#define	NAM_MASK_MNC			0x0020
#define	NAM_MASK_MIN			0x0040
#define	NAM_MASK_MDN			0x0080
#define	NAM_MASK_NAM_NAME		0x0100
#define	NAM_MASK_NAM_CHANGE		0x0200
#define	NAM_MASK_NAM_NUM		0x0400
#define	NAM_MASK_ALL			0x8000

/* NAMIndex Field */
typedef enum {
  IPC_MISC_NAM_REQUEST_INDEX_1 = 0,
  IPC_MISC_NAM_REQUEST_INDEX_2,     
  IPC_MISC_NAM_REQUEST_INDEX_3,     
  IPC_MISC_NAM_REQUEST_INDEX_4,
  IPC_MISC_NAM_REQUEST_INDEX_5
  } ipc_misc_nam_info_index_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
     Get CDMA feature 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | NAM_INFO_MASK(2) |  NAM_REQUEST_INDEX(1) 
-------------------------------------------------------------------------------------
*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_SET                     0x03    

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | NAM_INFO_MASK(2) |  NAM_CURRENT_INDEX(1) 
 | NAM_REQUEST_INDEX(1) | NAM_AUTO(1) | NAM_MCC(3) |  NAM_MNC(2) | NAM_MIN(10) | NAM_MDN(15) | NAM_NAME(17)
-------------------------------------------------------------------------------------
*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
     Respons CDMA feature 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | NAM_INFO_MASK(2) |  NAM_TOTAL(1) | NAM_CURRENT_INDEX(1) 
 | NAM_REQUEST_INDEX(1) | NAM_AUTO(1) | NAM_MCC(3) |  NAM_MNC(2) | NAM_MIN(10) | NAM_MDN(15) | NAM_NAME(17)
-------------------------------------------------------------------------------------
*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC__CMD_NOTI                     0x03     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | NAM_INFO_MASK(2) |  NAM_TOTAL(1) | NAM_CURRENT_INDEX(1) 
 | NAM_REQUEST_INDEX(1) | NAM_AUTO(1) | NAM_MCC(3) |  NAM_MNC(2) | NAM_MIN(10) | NAM_MDN(15) | NAM_NAME(17)
-----------------------------------------------------------------------------------*/



/*========================================================================================

   SUB_CMD(1) : IPC_MISC_VCALL_CHANNEL_ID        0x07 : Video call channel id

=========================================================================================*/
/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_RESP             0x02     */
/*    IPC_CMD_NOTI             0x03     */
/* none */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
     Video call channel id 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CHANNEL_ID |
-------------------------------------------------------------------------------------
*/

/*========================================================================================

   SUB_CMD(1) : IPC_MISC_PHONE_DEBUG        0x08 : Phone debug message

=========================================================================================*/
/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_SET              0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_RESP             0x02     */
/* none */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
     Phone debug message 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MESSAGE_MODE(1) | MESSAGE_LEN(2) | MESSAGE(X)
-------------------------------------------------------------------------------------
*/
/* Message mode */
typedef enum {
  IPC_MISC_PHONE_DEBUG_MODE_LOG=0x01,     /* 0x01 : Phone debug log mode */
  IPC_MISC_PHONE_DEBUG_MODE_FATALALERT,   /* 0x02 : Phone fatal alert mode */
  IPC_MISC_PHONE_DEBUG_MODE_MAX
} ipc_misc_phone_debug_mode_type;

/*The max. length of the MESSAGE(X) is MAX_MISC_PHONE_DEBUG_LEN*/


/*========================================================================================

   SUB_CMD(1) : IPC_MISC_FUS                 0x09      FUS start  Message For LINUX

========================================================================================*/
/* 		IPC_CMD_EXEC                        0x01  */
/* 		IPC_CMD_GET                         0x02  */
/* 		IPC_CMD_SET                         0x03  */
/* 		IPC_CMD_CFRM                        0x04  */
/* 		IPC_CMD_EVENT                       0x05  */
/* 		IPC_CMD_INDI                        0x01  */
/* 		IPC_CMD_RES                         0x02  */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_INDI                        0x01

 DESCRIPTION :
   
 FORMAT :

-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | 
-------------------------------------------------------------------------------------
*/


/*=================================================================*/



/*********************************************************************************

                                            Sub Command of IPC_SVC_CMD[0x0B]

**********************************************************************************/
typedef enum{
  IPC_SVC_ENTER=0x01,      			/* 0x01 : Enter Service Mode Message */
  IPC_SVC_END,                   			/* 0x02 : End Service Mode Message */
  IPC_SVC_PRO_KEYCODE,   			/* 0x03 : Process Keycode Message  */
  IPC_SVC_SCREEN_CFG,     			/* 0x04 : Screen Configuration Message */
  IPC_SVC_DISPLAY_SCREEN,  		/* 0x05 : Display Screen Message */
  IPC_SVC_CHANGE_SVC_MODE,	        /* 0x06 : Change Service Mode */
  IPC_SVC_DEVICE_TEST,	        /* 0x07 : Device Test Message */	
  IPC_SVC_MAX
} ipc_svc_sub_cmd_type;

/*********************************************************************************/

/*=================================================================

   SUB_CMD(1) : IPC_SVC_ENTER      0x01                Enter Service Mode Message

=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_CFRM              0x04     */
/*    IPC_CMD_EVENT             0x05     */
/*    IPC_CMD_INDI                0x01     */
/*    IPC_CMD_RESP               0x02     */
/* none */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
     Set ME Serial Number
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SVC_MODE(1) | TEST_MODE_SUB_TYPE(2) |
-------------------------------------------------------------------------------------

SVC_MODE IPC_SVC_MODE_TEST_MANUAL ƴ  TEST_MODE_SUB_TYPE(2)= 0X0000   
    
*/

/* SVC_MODE  field */
/* Service mode   */
typedef enum {
  IPC_SVC_MODE_TEST_MANUAL=0x01,     /* 0x01 : Manual test mode */
  IPC_SVC_MODE_TEST_AUTO,                  /* 0x02 : Auto test mode */
  IPC_SVC_MODE_NAM,                              /* 0x03 : NAM edit mode */
  IPC_SVC_MODE_MONITOR,                      /* 0x04 : Monitor screen mode */
  IPC_SVC_MODE_PHONE_TEST,                /* 0x05 : Phone test mode ( just for the debugging ) */
  IPC_SVC_MODE_OPERATOR_SPECIFIC_TEST,    /*0x06: Specific test mode required by operator*/
  IPC_SVC_MODE_MAX
} ipc_svc_mode_type;



/* Service mode 	IPC_SVC_MODE_TEST_MANUAL 츸 ش */ 
typedef enum { 
	TST_TESTMODE_ENTER=0X1000,						/* 0x1000 : Testmode enter */
	TST_SW_VERSION_ENTER,							/* 0x1001 : SW_version enter */
	TST_FTA_SW_VERSION_ENTER,						/* 0x1002 : FTA SW version enter */
	TST_FTA_HW_VERSION_ENTER,						/* 0x1003 : FTA HW version enter */
	TST_ALL_VERSION_ENTER,							/* 0x1004 : All version enter  */
	TST_BATTERY_INFO_ENTER,							/* 0x1005 : Battery Information enter  */
	TST_CIPHERING_PROTECTION_ENTER,			                        /* 0x1006 : Ciphering protection enter */
	TST_INTEGRITY_PROTECTION_ENTER, 			                /* 0x1007 : Integrity protection enter */
	TST_IMEI_READ_ENTER,							/* 0x1008 : IMEI enter */
	TST_BLUETOOTH_TEST_ENTER,						/* 0x1009 : Bluetooth test enter */
	TST_VIBRATOR_TEST_ENTER,						/* 0x100A : Vibrator test enter */
	TST_MELODY_TEST_ENTER,							/* 0x100B : Melody test enter */
	TST_MP3_TEST_ENTER,							/* 0x100C : MP3 test enter */
	TST_FACTORY_RESET_ENTER,						/* 0x100D : Factory test enter */
	TST_FACTORY_PRECONFIG_ENTER,					        /* 0x100E : Factory preconfig enter */
	TST_TFS4_EXPLORE_ENTER,							/* 0x100F : TFS4 explore enter */
	TST_RTC_TIME_DISPLAY_ENTER,						/* 0x1010 : RTC time display enter */
	TST_RSC_FILE_VERSION_ENTER,						/* 0x1011 : RSC file version enter */
	TST_USB_DRIVER_ENTER,							/* 0x1012 : USB driver enter */
	TST_USB_UART_DIAG_CONTROL_ENTER,			                /* 0x1013 : USB UART diag control enter*/
	TST_RRC_VERSION_ENTER,							/* 0x1014 : RRC Version enter */ 
	TST_GPSONE_SS_TEST_ENTER,                                   /* 0x1015 : GPSone testmode enter */ 
	TST_BAND_SEL_ENTER,							  /* 0x1016 : Band selection mode enter */
	TST_GCF_TESTMODE_ENTER,                         /* 0x1017 : GCF Test mode enter */
	TST_GSM_FACTORY_AUDIO_LB_ENTER,         /* 0x1018 : GSM Factory Audio Loopback enter */
	TST_FACTORY_VF_TEST_ENTER,                                         /* 0x1019 : VF ADC Test mode enter */
	TST_TOTAL_CALL_TIME_INFO_ENTER,                        /* 0x101A : Display Total Call time info enter */ /* kys 2008.01.30 Total Call time check NV item */
// FEATURE_SAMSUNG_SELLOUT_INFO_OVER_SMS
	TST_SELLOUT_SMS_ENABLE_ENTER,                                         /* 0x101B : Sell_Out_SMS_Enable_enter */
	TST_SELLOUT_SMS_DISABLE_ENTER,                                         /* 0x101C : Sell_Out_SMS_Disable_enter */
	TST_SELLOUT_SMS_TEST_MODE_ON,                                         /* 0x101D : Sell_Out_SMS_Test_Mode_ON */
	TST_SELLOUT_SMS_PRODUCT_MODE_ON,                                  /* 0x101E : Sell_Out_SMS_Product_Mode_ON */
      TST_GET_SELLOUT_SMS_INFO_ENTER,                                      /* 0x101F : GET_SELLOUT_SMS_INFO_ENTER */      
	TST_SUB_MODE_MAX

} ipc_test_mode_sub_type;


/* Service mode 	IPC_SVC_MODE_NAM  츸 ش */ 
typedef enum { 
	IPC_SVC_NAM_EDIT=0x0001,                           /* 0x0001 : NAM Full Edit Mode */
	IPC_SVC_NAM_BASIC_EDIT,                            /* 0x0002 : NAM Basic Edit Mode */
	IPC_SVC_NAM_ADVANCED_EDIT,                     /* 0x0003 : NAM Advanced Edit Mode */
	IPC_SVC_NAM_MAX
} ipc_svc_nam_sub_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
     ME Serial Number Notification
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SVC_MODE(1) |
-------------------------------------------------------------------------------------
*/

/* SVC_MODE  field */
/* Service mode   */
/* see ipc_svc_mode_type */



/*=================================================================*/

/*=================================================================

   SUB_CMD(1) : IPC_SVC_END      0x02                      End Service Mode Message

=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_CFRM          0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_RESP             0x02     */
/* none */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
    Set End Service Mode
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SVC_MODE(1) |
-------------------------------------------------------------------------------------
*/

/* SVC_MODE  field */
/* Service mode   */
/* see ipc_svc_mode_type */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
    End Service Mode Notification
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SVC_MODE(1) |RESET_REQUIRED(1) |
-------------------------------------------------------------------------------------
*/

/* SVC_MODE  field */
/* Service mode   */
/* see ipc_svc_mode_type */ 

/* RESET_REQUIRED */
/* indicate that whether phone reset or not after end of svc mode */
typedef enum{
  IPC_SVC_END_NO_ACTION=0x00,	/* 0x00 : RESET_NO_ACTION */
  IPC_SVC_END_PHONE_ONLY_RESET,      /* 0x01 : RESET_PHONE_ONLY_REQUIRED */
  IPC_SVC_END_BOTH_RESET, 		           /* 0x02 : RESET_BOTH_REQUIRED */
  IPC_SVC_END_MAX
} ipc_svc_end_e_type;

/*=================================================================*/

/*=================================================================

   SUB_CMD(1) : IPC_SVC_PRO_KEYCODE      0x03                  Process Key Code Message
   
=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_CFRM      0x04     */
/*    IPC_CMD_EVENT          0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_RESP             0x02     */
/* none */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
    Set Process Key Code
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | KEY_CODE(1) |
-------------------------------------------------------------------------------------
*/

/* KEY_CODE  field */
/* Pressed Key Code */
/* Refer to the  "SOUND related ipc_snd_key_code_e_type" */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
    Screen Configuration Notification
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | KEY_CODE(1) | KEY_HOLD(1) |
-------------------------------------------------------------------------------------
*/

/* KEY_CODE  field */
/* Pressed Key Code */
/* Refer to the  "SOUND related ipc_snd_key_code_e_type" */

/* KEY_HOLD */
/* indicate that the key is whether release or hold */
typedef enum{
  IPC_SVC_KEYRLEASE,      /* 0x00 : Key code is release */
  IPC_SVC_KEYHOLD, 		  /* 0x01 : Key code is hold */
  IPC_SVC_KEY_MAX
} ipc_svc_keyhold_e_type;


/*=================================================================*/

/*=================================================================

   SUB_CMD(1) : IPC_SVC_SCREEN_CFG      0x04             Screen Configuration Message
   
=================================================================*/

/*    IPC_CMD_EXEC            0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_SET              0x03     */
/*    IPC_CMD_CFRM      0x04     */
/*    IPC_CMD_EVENT          0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_RESP            0x02     */
/* none */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
    Screen Configuration Notification
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | NUM_LINE(1) | KEYPAD(1) |
-------------------------------------------------------------------------------------
*/

/* NUM_LINE */
/* The number of the line on the screen. */

/* KEYPAD */
/* Decide if the keypad is necessary. */
typedef enum{
  IPC_SVC_KEYPAD_NO,      /* 0x00 : Keypad is not necessary */
  IPC_SVC_KEYPAD_YES,     /* 0x01 : Keypad is necessary */
  IPC_SVC_KEYPAD_MAX
} ipc_svc_keypad_e_type;


/*=================================================================*/

/*=================================================================

   SUB_CMD(1) : IPC_SVC_DISPLAY_SCREEN      0x05                     Display Screen Message

=================================================================*/
/*    IPC_CMD_EXEC            0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_SET              0x03     */
/*    IPC_CMD_CFRM      0x04     */
/*    IPC_CMD_EVENT          0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_RESP             0x02     */
/* none */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
    Display Screen Notification
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | NUM_LINE(1) | LINE(1) | REVERSE(1) |
-------------------------------------------------------------------------------------
 | STRING(32) | 
-------------------------------------------------------------------------------------
*/

/* NUM_LINE  field */
/* The number of lines */

/* LINE  field */
/* Line ȣ(0 ~ 10) */

/* STRING  field */
/* LCD ǥϰ ϴ ڿ.  ڿ NULL  ڿ̴.
    ִ ڿ ̴ 31 byte ( 31, ѱ 15) ̴. */

/* REVERSE  field */
/*  */
typedef enum {
  IPC_SVC_REVERSE_OFF,    /* 0x00 : Ϲ  */
  IPC_SVC_REVERSE_ON,     /* 0x01 :   */
  IPC_SVC_REVERSE_MAX
} ipc_svc_reverse_e_type;

/*=================================================================

   SUB_CMD(1) :   IPC_SVC_CHANGE_SVC_MODE   0x06   	Change service mode 
   
=================================================================*/

/*    IPC_CMD_EXEC            0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_SET              0x03     */
/*    IPC_CMD_CFRM      0x04     */
/*    IPC_CMD_EVENT          0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_RESP             0x02     */
/* none */

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
    Change Service mode Notification
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SVC_MODE(1) |
-------------------------------------------------------------------------------------
*/

/* SVC_MODE  field */
/* Service mode   */
/* see ipc_svc_mode_type */

/*=================================================================

   SUB_CMD(1) : IPC_SVC_DEVICE_TEST      0x07                Device Test Message

=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_SET              0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_RESP             0x02     */
/* none */

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
    Device Test Notification
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | DEVICE_ID(1) |STATUS(1) |
-------------------------------------------------------------------------------------
*/

/* DEVICE_ID  field */
/* test device id Ŵ */
typedef enum{
  IPC_SVC_DEVICE_TEST_VIBRATION=0x01,	/* 0x01 : VIBRATION TEST */
  IPC_SVC_DEVICE_TEST_BACKLIGHT,		/* 0x02 : BACKLIGHT TEST */
  IPC_SVC_DEVICE_TEST_FRONTLCD,		/* 0x03 : FRONT LCD TEST */
  IPC_SVC_DEVICE_TEST_KEYLED,		/* 0x04 : KEY LED TEST */
  IPC_SVC_DEVICE_TEST_ALERTLED,  		/* 0x05 : ALERT LED TEST */
  IPC_SVC_DEVICE_TEST_MAX
} ipc_svc_device_id_e_type;

/* STATUS field */
/* TEST DEVICE ON / OFF ¸ Ŵ */
typedef enum{
  IPC_SVC_DEVICE_TEST_OFF=0x00,	/* 0x00 : DEVICE TEST OFF */
  IPC_SVC_DEVICE_TEST_ON,     	/* 0x01 : DEVICE TEST ON */
  IPC_SVC_DEVICE_TEST_AUTO,	/*0x02 : AUTO TEST MODE */
  IPC_SVC_DEVICE_TEST_STATUS_MAX
} ipc_svc_status_e_type;

/*=================================================================*/






/*********************************************************************************

                                            Sub Command of IPC_SS_CMD[0x0C]

**********************************************************************************/
typedef enum{
  IPC_SS_WAITING=0x01,          /* 0x01 : Call Waiting Message */
  IPC_SS_CLI,                             /* 0x02 : Calling Line Identification Configuration Message */
  IPC_SS_BARRING,                    /* 0x03 : Call barring Configuration Message */
  IPC_SS_BARRING_PW,             /* 0x04 : Call barring password change Message */
  IPC_SS_FORWARDING,             /* 0x05 : Call forwarding Configuration Message*/
  IPC_SS_INFO,                          /* 0x06 : SS Information Message */
  IPC_SS_MANAGE_CALL,           /* 0x07 : Manage Call Message */
  IPC_SS_USSD,                          /* 0x08 : USSD Message */
  IPC_SS_AOC,                       /* 0x09 : AOC Message */
  IPC_SS_RELEASE_COMPLETE,                       /* 0x0A : Release Complete Message */
  IPC_SS_MAX
} ipc_ss_sub_cmd_type;

/*********************************************************************************/


/*=================================================================

   SUB_CMD(1) : IPC_SS_WAITING    0x01                    Call Waiting Message

==================================================================*/

/*    IPC_CMD_EXEC                     0x01     */
/*    IPC_CMD_CFRM                    0x04     */
/*    IPC_CMD_EVENT                   0x05     */
/*    IPC_CMD_INDI                      0x01     */
//none



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
    Get Call Waiting
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CLASS(1) |
-------------------------------------------------------------------------------------
*/

/* CLASS  field */
typedef enum {
  IPC_SS_CLASS_NONE=0x00,                               /* 0x00 */

  /* TELESERVICE */
  IPC_SS_CLASS_ALL_TELE=0x10,                        /* 0x10 : All Teleservices */
  IPC_SS_CLASS_VOICE=0x11,                              /* 0x11 : All Voice ( telephony ) */
  IPC_SS_CLASS_ALL_DATA_TELE=0x12,             /* 0x12 : All Data Teleservices */
  IPC_SS_CLASS_FAX=0x13,                                /* 0x13 : All Fax Service */
  IPC_SS_CLASS_SMS=0x16,                                /* 0x16 : SMS service	 */
  IPC_SS_CLASS_VGCS=0x17,                              /* 0x17 : Voice Group Call Service */
  IPC_SS_CLASS_VBS=0x18,                                /* 0x18 : Voice Broadcast */
  IPC_SS_CLASS_ALL_TELE_EXPT_SMS=0x19,    /* 0x19 : All teleservice except SMS */

  /* BEARER SERVICE */
  IPC_SS_CLASS_ALL_BEARER=0x20,                  /* 0X20 : all bearer services */
  IPC_SS_CLASS_ALL_ASYNC=0x21,                    /* 0x21 : All Async services */
  IPC_SS_CLASS_ALL_SYNC=0x22,                      /* 0x22 : All sync services*/
  IPC_SS_CLASS_ALL_CS_SYNC=0x24,                /* 0x24 : All Circuit switched sync */
  IPC_SS_CLASS_ALL_CS_ASYNC=0x25,              /* 0x25 : All Circuit switched async */
  IPC_SS_CLASS_ALL_DEDI_PS=0x26,                /* 0x26 : All Dedicated packet Access */
  IPC_SS_CLASS_ALL_DEDI_PAD=0x27,              /* 0x27 : All Dedicated PAD Access */
  IPC_SS_CLASS_ALL_DATA_CDA=0x28,		/*0x28 : All Data CDA*/  

  /* PLMN SPECIFIC TELESERVICE */
  IPC_SS_CLASS_PLMN_TELE_ALL = 0x50,         /*0x50 : PLMN specific teleservices*/
  IPC_SS_CLASS_PLMN_TELE_1 = 0x51,              /*0x51 :PLMN specific teleservice 1*/
  IPC_SS_CLASS_PLMN_TELE_2 = 0x52,             /*0x52 : PLMN specific teleservice 2*/
  IPC_SS_CLASS_PLMN_TELE_3 = 0x53,             /*0x53 : PLMN specific teleservice 3*/
  IPC_SS_CLASS_PLMN_TELE_4 = 0x54,             /*0x54 : PLMN specific teleservice 4*/
  IPC_SS_CLASS_PLMN_TELE_5 = 0x55,             /*0x55 : PLMN specific teleservice 5*/
  IPC_SS_CLASS_PLMN_TELE_6 = 0x56,             /*0x56 : PLMN specific teleservice 6*/
  IPC_SS_CLASS_PLMN_TELE_7 = 0x57,             /*0x57 : PLMN specific teleservice 7*/
  IPC_SS_CLASS_PLMN_TELE_8 = 0x58,             /*0x58 : PLMN specific teleservice 8*/
  IPC_SS_CLASS_PLMN_TELE_9 = 0x59,             /*0x59 : PLMN specific teleservice 9*/
  IPC_SS_CLASS_PLMN_TELE_A = 0x60,           /*0x60 :PLMN specific teleservice 10*/
  IPC_SS_CLASS_PLMN_TELE_B = 0x61,           /*0x61 :PLMN specific teleservice 11*/
  IPC_SS_CLASS_PLMN_TELE_C = 0x62,             /*0x62 : PLMN specific teleservice 12*/
  IPC_SS_CLASS_PLMN_TELE_D = 0x63,             /*0x63 : PLMN specific teleservice 13*/
  IPC_SS_CLASS_PLMN_TELE_E = 0x64,             /*0x64 : PLMN specific teleservice 14*/
  IPC_SS_CLASS_PLMN_TELE_F = 0x65,             /*0x65 : PLMN specific teleservice 15*/

  /* PLMN SPECIFIC BEARER SERVICE */
  IPC_SS_CLASS_PLMN_BEAR_ALL = 0x70,         /*0x70 : All PLMN specific bearer services*/
  IPC_SS_CLASS_PLMN_BEAR_1 = 0x71,              /*0x71 :PLMN specific bearer service 1*/
  IPC_SS_CLASS_PLMN_BEAR_2 = 0x72,             /*0x72 : PLMN specific bearer service  2*/
  IPC_SS_CLASS_PLMN_BEAR_3 = 0x73,             /*0x73 : PLMN specific bearer service  3*/
  IPC_SS_CLASS_PLMN_BEAR_4 = 0x74,             /*0x74 : PLMN specific bearer service  4*/
  IPC_SS_CLASS_PLMN_BEAR_5 = 0x75,             /*0x75 : PLMN specific bearer service  5*/
  IPC_SS_CLASS_PLMN_BEAR_6 = 0x76,             /*0x76 : PLMN specific bearer service  6*/
  IPC_SS_CLASS_PLMN_BEAR_7 = 0x77,             /*0x77 : PLMN specific bearer service  7*/
  IPC_SS_CLASS_PLMN_BEAR_8 = 0x78,             /*0x78 : PLMN specific bearer service  8*/
  IPC_SS_CLASS_PLMN_BEAR_9 = 0x79,             /*0x79 : PLMN specific bearer service  9*/
  IPC_SS_CLASS_PLMN_BEAR_A = 0x80,            /*0x80 : PLMN specific bearer service  10*/
  IPC_SS_CLASS_PLMN_BEAR_B = 0x81,             /*0x81 : PLMN specific bearer service  11*/
  IPC_SS_CLASS_PLMN_BEAR_C = 0x82,            /*0x82 : PLMN specific bearer service  12*/
  IPC_SS_CLASS_PLMN_BEAR_D = 0x83,            /*0x83 : PLMN specific bearer service  13*/
  IPC_SS_CLASS_PLMN_BEAR_E = 0x84,             /*0x84 : PLMN specific bearer service  14*/
  IPC_SS_CLASS_PLMN_BEAR_F = 0x85,             /*0x85 : PLMN specific bearer service  15*/
  
  /* CPHS - AUXILIARY SERVICE */
  IPC_SS_CLASS_AUX_VOICE = 0x89,			/* 0x89 : All Auxiliary Voice ( Auxiliary telephony ) */
  
  IPC_SS_CLASS_ALL_GPRS_BEARER=0x99,       /* 0x99 : All GPRS bearer services */
  IPC_SS_CLASS_ALL_TELE_BEARER=0xFF,        /* 0xFF : all tele and bearer services */
} ipc_ss_class_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
    Set Call Waiting
 FORMAT :
-------------------------------------------------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CLASS(1) | SS_MODE(1) |
-------------------------------------------------------------------------------------
*/

/* CLASS  field */
/* see ipc_ss_class_e_type */


/* STATUS  field */
typedef enum {
  IPC_SS_MODE_REG=0x01,         /* 0x01 : Registration */
  IPC_SS_MODE_DEREG,              /* 0x02 : De-registration( erase ) */
  IPC_SS_MODE_ACTIVATE,        /* 0x03 : Activation */
  IPC_SS_MODE_DEACTIVATE,    /* 0x04 : De-activation */
  IPC_SS_MODE_MAX
} ipc_ss_mode_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
    Call Waiting Response
 FORMAT :
-------------------------------------------------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | NUM_CLASS(1) | CLASS(1) | SS_STATUS(1) |
-------------------------------------------------------------------------------------
*/

/* NUM_CLASS */
/* NUM_CLASS occurences of the following fields */
/* see the MAX_SS_CLASS_NUM */

/* CLASS  field */
/* see ipc_ss_class_e_type */

/* STATUS  field */
typedef enum {
  IPC_SS_STATUS_NOT_ACTIVE = 0x01,
  IPC_SS_STATUS_ACTIVE,
  IPC_SS_STATUS_MAX
} ipc_ss_status_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
    Call Waiting Notification
 FORMAT :
-------------------------------------------------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | NUM_CLASS(1) | CLASS(1) | SS_STATUS(1) |
-------------------------------------------------------------------------------------
*/



/*====================================================================*/


/*=================================================================

   IPC_SS_CLI    0x02                    Call Line Identification Configuration Message
   
==================================================================*/
/*    IPC_CMD_EXEC                     0x01     */
/*    IPC_CMD_SET                     0x03     */
/*    IPC_CMD_CFRM               0x04     */
/*    IPC_CMD_EVENT                   0x05     */
/*    IPC_CMD_INDI                      0x01     */
/*    IPC_CMD_NOTI                     0x03     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
    Get  Call Line Identification Configuration
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | LINE_ID_TYPE(1) |
-------------------------------------------------------------------------------------
*/

/* LINE_ID_TYPE  field */
typedef enum{
  IPC_SS_LINE_ID_NONE,
  IPC_SS_LINE_ID_CLIP,       /* 0x01 : Calling Line Identification Presentation */
  IPC_SS_LINE_ID_CLIR,       /* 0x02 : Calling Line Identification Restriction */
  IPC_SS_LINE_ID_COLP,      /* 0x03 : Connected Line Identification Presentation */
  IPC_SS_LINE_ID_COLR,      /* 0x04 : Connected Line Identification Restriction */
  IPC_SS_LINE_ID_CDIP,       /* 0x05 : Called Line Identification Presentation */
  IPC_SS_LINE_ID_CNAP,      /* 0x06 : Calling Name Presentation */
  IPC_SS_LINE_ID_MAX
} ipc_ss_line_id_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
    Set  Call Line Identification Configuration
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | LINE_ID_TYPE(1) | SS_MODE(1) |
-------------------------------------------------------------------------------------
*/

/* LINE_ID_TYPE  field */
/* see ipc_ss_line_id_e_type */

/* SS_MODE  field */
/* see ipc_ss_mode_e_type */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
    Call Line Identification Configuration Response
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | LINE_ID_TYPE(1) | CLI_STATUS(1) |
-------------------------------------------------------------------------------------
*/


/* SERVICE_STATUS  field */
/* Line Identification Supplementary Service Status */
typedef enum{
  IPC_SS_CLI_STAT_NOT_PROVISIONED=0x01,     /* 0x01 : Not Provisioned and Deactivated */
  IPC_SS_CLI_STAT_PROVISIONED,              /* 0x02 : Provisioned but Deactivated */
  IPC_SS_CLI_STAT_ACTIVATED,                /* 0x03 : Provisioned and Activated */  
  IPC_SS_CLI_STAT_UNKNOWN,                  /* 0x04 : Unknown */
  IPC_SS_CLI_STAT_TEMP_RESTRICTED,          /* 0x05 : Temporary Presentation Restricted( only applicable in CLIR ) */
  IPC_SS_CLI_STAT_TEMP_ALLOWED,             /* 0x06 : Temporary Presentation Allowed( only applicable in CLIR ) */
  IPC_SS_CLI_STAT_MAX
} ipc_ss_cli_status_e_type;



/*====================================================================*/




/*=================================================================

   SUB_CMD(1) : IPC_SS_BARRING    0x03                      Call Barring Configuration Message

==================================================================*/

/*    IPC_CMD_EXEC                     0x01     */
/*    IPC_CMD_CFRM               0x04     */
/*    IPC_CMD_EVENT                   0x05     */
/*    IPC_CMD_INDI                      0x01     */
//none




/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
    Get Call Barring Configuration
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | BARR_TYPE(1) | CLASS(1) |
-------------------------------------------------------------------------------------
*/

/* BARR_TYPE field */
typedef enum {
  IPC_SS_BARR_TYPE_NONE,
  IPC_SS_BARR_TYPE_BAOC,                /* 0x01 : Barring All Outgoing Calls */
  IPC_SS_BARR_TYPE_BOIC,                /* 0x02 : Barring Outgoing International Calls */
  IPC_SS_BARR_TYPE_BOIC_NOT_HC, /* 0x03 : Barring Outgoing International Calls
                                                                              except to Home Country */
  IPC_SS_BARR_TYPE_BAIC,                /* 0x04 : Barring All Incoming Calls */
  IPC_SS_BARR_TYPE_BIC_ROAM,       /* 0x05 : Barring Incoming Calls when roam, 
                                                                            outside of the Home Country */
  IPC_SS_BARR_TYPE_AB,                   /* 0x06 : All Barring Services */
  IPC_SS_BARR_TYPE_AOB,                 /* 0x07 : All Outgoing Barring Services */
  IPC_SS_BARR_TYPE_AIB,                  /* 0x08 : All Incoming Barring Services */
  IPC_SS_BARR_TYPE_BIC_NOT_SIM, /* 0x09 : Barring Incoming Calls which is
                                                                       not stored in the SIM memory */
  IPC_SS_BARR_TYPE_MAX
} ipc_ss_barr_e_type;

/* CLASS  field */
/* see  ipc_ss_class_e_type */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
    Set Call Barring Configuration
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | BARR_TYPE(1) | SS_MODE(1) | 
-------------------------------------------------------------------------------------
 | CLASS(1) |  PWD(X) |
-------------------------------------------------------------------------------------
*/

/* BARR_TYPE field */
/* see "ipc_ss_barr_e_type "*/

/* SS_MODE  field */
/* see ipc_ss_mode_e_type */

/* CLASS field */
/* see "ipc_ss_class_e_type"*/

/* PWD  field */
/* Call barring password in ASCII. Maximum length is 4 characters. The range is 0000 9999. */
/* The range is 0000 9999. */
/* see MAX_SS_BARR_PW_LEN */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
    Call Barring Configuration Response
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | BARR_TYPE(1) | NUM_CLASS(1) |
-------------------------------------------------------------------------------------
  | CLASS(1) | SS_STATUS(1) |
-------------------------------------------------------------------------------------
*/

/* BARR_TYPE field */
/* see "ipc_ss_barr_e_type " */

/* NUM_CLASS  field */
/* all next fields are repeated NUM_CLASS times */
/* see MAX_SS_CLASS_NUM */

/* CLASS */
/* see  ipc_ss_class_e_type */

/* SS_STATUS */
/* see ipc_ss_status_e_type */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
    Call Barring Configuration Notification
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | BARR_TYPE(1) | NUM_CLASS(1) |
-------------------------------------------------------------------------------------
  | CLASS(1) | SS_STATUS(1) |
-------------------------------------------------------------------------------------
*/



/*====================================================================*/




/*=================================================================

   IPC_SS_BARRING_PW    0x04                     Call Barring Password Message
   
==================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_GET                       0x02     */
/*    IPC_CMD_EVENT                   0x05     */
/*    IPC_CMD_RESP                     0x02     */
/*    IPC_CMD_NOTI                     0x03     */
//none



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
    Set Call Barring Password
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | PWD(4) | NEW_PWD(4) | NEW_PWD_AGAIN(4)
-------------------------------------------------------------------------------------
*/

/* PWD */
/* old password : 0000 ~ 9999 */

/* NEW_PWD */
/* new password : 0000 ~ 9999 */

/* NEW_PWD_AGAIN */
/* new password again : 0000 ~ 9999 */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_CFRM                     0x04

 DESCRIPTION :
    Confirm Call Barring Password
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | PW_GUIDANCE(1) |  PWD(4) |
-------------------------------------------------------------------------------------
*/

/* PW_GUIDANCE */
typedef enum {
  IPC_SS_PW_GUID_REQ = 0x01,               /* 0x01 : Password is required */
  IPC_SS_PW_GUID_NEW,              /* 0x02 : New Password is required */
  IPC_SS_PW_GUID_NEW_AGAIN, /* 0x03 : New Password is required again */
  IPC_SS_PW_GUID_MAX               /* 0x04 : max */
} ipc_ss_pw_guidance_e_type;


/* PW  */
/* The Maximum Length is 4 characters */
/* the range is 0000 - 9999 */
/* see MAX_SS_BARR_PW_LEN */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_INDI                     0x01

 DESCRIPTION :
    Call Barring Password Indication
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | PW_GUIDANCE(1) |
-------------------------------------------------------------------------------------
*/

/* PW_GUIDANCE */
/* see ipc_ss_pw_guidance_e_type */



/*====================================================================*/




/*=================================================================

   SUB_CMD(1) : IPC_SS_FORWARDING    0x05            Call Forwarding Configuration Message
   
==================================================================*/

/*    IPC_CMD_EXEC                     0x01     */
/*    IPC_CMD_CFRM               0x04     */
/*    IPC_CMD_EVENT                   0x05     */
/*    IPC_CMD_INDI                      0x01     */
//none




/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
    Get Call Forwarding Configuration
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CF_TYPE(1) | CLASS(1) |
-------------------------------------------------------------------------------------
*/


/* CF_TYPE  field */
typedef enum{
  IPC_SS_CF_TYPE_CFU = 0x01,               /* 0x01 : Call Forwarding Unconditional */
  IPC_SS_CF_TYPE_CFB,               /* 0x02 : Call Forwarding Mobile Busy */
  IPC_SS_CF_TYPE_CFNRy,          /* 0x03 : Call Forwarding No Reply */
  IPC_SS_CF_TYPE_CFNRc,          /* 0x04 : Call Forwarding Not Reachable */
  IPC_SS_CF_TYPE_CF_ALL,        /* 0x05 : All Call Forwarding */
  IPC_SS_CF_TYPE_CFC,              /* 0x06 : All Conditional Call Forwarding */
  IPC_SS_CF_TYPE_MAX               /* 0x07 : Max */
} ipc_ss_cf_e_type;

/* CLASS  field */
/* refer to the ipc_ss_class_e_type */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
    Set Call Forwarding Configuration
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SS_MODE(1) | CF_TYPE(1) | CLASS(1) |
-------------------------------------------------------------------------------------
 | NO_REPLY_WAIT_TIME(1) | NUMBER_LEN(1) | NUMBER_TYPE(1) | NUMBER(X) |
-------------------------------------------------------------------------------------
*/

/* SS_MODE  field */
/* see ipc_ss_mode_e_type */

/* NO_REPLY_WAIT_TIME  field */
/* In the case of "no reply", this gives the time in seconds to wait before call is forwarded.
The range is 1 to 30.  default is 20 */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
    Call Forwarding Configuration Response
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CF_TYPE(1) | NUM_CLASS(1) | CLASS(1) |
-------------------------------------------------------------------------------------
 | SS_STATUS(1) | NO_REPLY_WAIT_TIME(1) | NUMBER_LEN(1) | NUMBER_TYPE(1) | NUMBER(X) |
-------------------------------------------------------------------------------------
*/



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
    Call Forwarding Configuration Notification
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CF_TYPE(1) | NUM_CLASS(1) | CLASS(1) |
-------------------------------------------------------------------------------------
 | SS_STATUS(1) | NO_REPLY_WAIT_TIME(1) | NUMBER_LEN(1) | NUMBER_TYPE(1) | NUMBER(X) |
-------------------------------------------------------------------------------------
*/



/*====================================================================*/




/*=================================================================

   SUB_CMD(1) : IPC_SS_INFO    0x06                  Supplementary Service Information Message
   
==================================================================*/

/*    IPC_CMD_EXEC                     0x01     */
/*    IPC_CMD_GET                       0x02     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_CFRM               0x04     */
/*    IPC_CMD_EVENT                   0x05     */
/*    IPC_CMD_INDI                      0x01     */
/*    IPC_CMD_RESP                     0x02     */
//none



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
    Supplementary Service Information Notification
 FORMAT :
------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SS_INFO(1) | PI(1) | NO_CLI_CAUSE(1)|
------------------------------------------------------------------------------------------
 | CUG_INDEX(1) | NUMBER_LEN(1) | NUMBER_TYPE(1) | NUMBER(X) | NAME_LEN(1) | DCS(1) | NAME(X) |
------------------------------------------------------------------------------------------
*/
/* DCS */
/* refer to ipc_cbs_dcs_e_type */

/* CBS Data Coding Scheme */
/* Refer to Refer to 3GPP TS 23.038 "CBS Data Coding Scheme" */ 

typedef enum  {
  IPC_CBS_DCS_7_BIT         = 0x00,     /* 7 bit Data encoding scheme used for ussd */
  IPC_CBS_DCS_8_BIT         = 0x04,     /* 8 bit Data encoding scheme used for ussd */  
  IPC_CBS_DCS_UCS2          = 0x08,     /* UCS2 Data encoding scheme used for ussd */  
  IPC_CBS_DCS_UNSPECIFIED   = 0x0F,     /* Data encoding scheme unspecified */
} ipc_cbs_dcs_e_type;

/* SS_INFO  field */
typedef enum{
  IPC_SS_INFO_NONE=0x00,

  /* SS Information received During a mobile Originated call setup */
  IPC_SS_INFO_COL =0x01,                         /* 0x01 : Connected Party Number */
  IPC_SS_INFO_MO_WAITING,                      /* 0x02 : this call is waiting */
  IPC_SS_INFO_CUG_CALL_MO,                    /* 0x03 : this is cug call */
  IPC_SS_INFO_FORWARDED_MO,                /* 0x04 : outgoing call is forwarded */
  IPC_SS_INFO_INCOMING_BARRED,            /* 0x05 : incoming call is barred */
  IPC_SS_INFO_OUTGOING_BARRED,           /* 0x06 : outgoing call is barred */
  IPC_SS_INFO_DEFLECTED_MO,                 /* 0x07 : outgoing call is deflected */
  IPC_SS_INFO_CLIR_SUPPRESSION_REJ,   /* 0x08 : CLIR suppression rejected */
  IPC_SS_INFO_CFU_ACTIVE,                      /* 0x09 : Unconditional CF is active */
  IPC_SS_INFO_CFC_ACTIVE,                      /* 0x0a : some of the conditional CF are active */
  IPC_SS_INFO_ALL_OUTGOING_BARRED,              /* 0x10 :Barring All outgoing call  => BAOC */
  IPC_SS_INFO_OUTGOING_INTERNATIONAL_BARRED,      /* 0x11 :Barring Outgoing International Calls => BOIC */
  IPC_SS_INFO_OUTGOING_INTERNATIONAL_EXHC_BARRED,   /* 0x12 :Baarring Outgoing International Calls Except  those directed to the home PLMN country => BoicExHC */
  IPC_SS_INFO_ALL_INCOMING_BARRED,          /* 0x13 :Barring All incoming call => BAIC*/
  IPC_SS_INFO_INCOMING_ROAM_BARRED,         /* 0x14 :Barring Incoming Call While Roam => BicRoam */


   /* other values are reserved ( 0x0B - 0x30 )*/  
  
  /* SS Information received During a mobile Terminated call setup */
  IPC_SS_INFO_CLI=0x31,                           /* 0x31 : Calling Party Number */
  IPC_SS_INFO_CNA,                                     /* 0x32 : Calling Party name */
  IPC_SS_INFO_FORWARDED_MT,                 /* 0x33 : this call is forwarded */
  IPC_SS_INFO_CUG_CALL_MT,                     /* 0x34 : this is cug call */
  IPC_SS_INFO_DEFLECTED_MT,                   /* 0X35 : this is cug call */
  IPC_SS_INFO_ECT_MT,                               /* 0x36 : call has been connected with the other remote party */
   /* other values are reserved ( 0x37-0x60 ) */  

  /* SS Information received in a Connected State */
  IPC_SS_INFO_CALL_ON_HOLD = 0x61,                  /* 0x61 : Call has been put on hold */
  IPC_SS_INFO_CALL_RETRIEVED,               /* 0x62 : Call has been retrieved */
  IPC_SS_INFO_MPTY_ENTERED,                  /* 0x63 : Multiparty Call Entered */
  IPC_SS_INFO_HELD_CALL_RELEASED,       /* 0x64 : call on hold has been released */
  IPC_SS_INFO_ECT_REMOTE_ALERT,          /* 0x65 : Call is being connected(alerting) with the remote party */
                                                                              /* in alerting state in explicit call transfer operation */
  IPC_SS_INFO_ECT_REMOTE_ACTIVE,        /* 0x66 : Call is been connected(active) with the remote party */
                                                                              /* in alerting state in explicit call transfer operation */
  IPC_SS_INFO_FORWARD_CHECK_SS ,        /* 0x67 : forward check SS message received (can be 
                                                                                              received in Idle or during a call ) */
  IPC_SS_INFO_CDMA_DISPLAY = 0x71,        /* 0x71 CDMA DISPLAY REC (except CNA) */    
   /* other values are reserved ( 0x72-0xFF )*/  
  
  IPC_SS_REASON_MAX
} ipc_ss_info_e_type;


/* PI field : Presentation Indicator */
typedef enum{
  IPC_SS_PI_ALLOWED,                  /* 0x00 : Allowed */
  IPC_SS_PI_RESTRICTED,             /* 0x01 : Restricted */
  IPC_SS_PI_NOT_AVAILABLE,       /* 0x02 : Number not available */
  IPC_SS_PI_RESERVED,       /* 0x03 : Number not available */
  IPC_SS_PI_MAX
} ipc_ss_pi_e_type;

/* NO_CLI_CAUSE field : Cause of No CLI*/
typedef enum{
  IPC_SS_NO_CLI_CAUSE_UNAVAILABLE,                      /* 0x00 : Unavailable */
  IPC_SS_NO_CLI_CAUSE_REJECT_BY_USER,                   /* 0x01 : Reject by user */
  IPC_SS_NO_CLI_CAUSE_INTERACTION_WITH_OTHER_SERVICES,  /* 0x02 : Interaction with other service */
  IPC_SS_NO_CLI_CAUSE_COIN_LINE_PAYPHONE,               /* 0x03 : Coin line/payphone */
  IPC_SS_NO_CLI_CAUSE_MAX
}ipc_ss_no_cli_cause_e_type;

/* CUG_INDEX field*/
/* the index of the Closed User Group Service */

/*====================================================================*/


/*=================================================================

   SUB_CMD(1) : IPC_SS_MANAGE_CALL    0x07              SS Manage Call Message
   
==================================================================*/

/*    IPC_CMD_EXEC                     0x01     */
/*    IPC_CMD_GET                       0x02     */
/*    IPC_CMD_CFRM                    0x04     */
/*    IPC_CMD_EVENT                   0x05     */
/*    IPC_CMD_INDI                      0x01     */
/*    IPC_CMD_RESP                     0x02     */
/*    IPC_CMD_NOTI                     0x03     */
//none



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
    Set Supplementary Service Manage Call
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CM_MODE(1) | CALL_ID(1) | 
-------------------------------------------------------------------------------------
 | NUMBER_LEN(1) | NUMBER(X) | 
-------------------------------------------------------------------------------------
*/

/* CM_MODE field : Call Manage Mode */
typedef enum {
  IPC_SS_CM_0_SEND=0x01,             /* 0x01 : Releases all held calls or sets User Determined 
                                                                                 User Busy (UDUB) for a waiting call*/
  IPC_SS_CM_1_SEND,                      /* 0x02 : Releases all active calls (if any exist) and
                                                                                      accepts the other (held or waiting) call.*/
  IPC_SS_CM_1X_SEND,                    /* 0x03 : Releases a specific active call X.
                                                                                      (also <INDEX> present)*/
  IPC_SS_CM_2_SEND,                      /* 0x04 : Places all active calls (if any exist) on hold 
                                                                               and accepts the other (held or waiting) call.*/
  IPC_SS_CM_2X_SEND,                    /* 0x05 : Places all active calls on hold except call X
                                                                             with which communication shall be supported.
                                                                              (also <INDEX> present)*/
  IPC_SS_CM_3_SEND,                      /* 0x06 : Adds a held call to the conversation.*/
  IPC_SS_CM_4_SEND,                      /* 0x07 : Connects the two calls and disconnects
                                                                                the subscriber from both calls (ECT).*/
  IPC_SS_CM_4DN_SEND,                 /* 0x08 : Redirect an incoming or a waiting call 
                                                                            to the specified followed by SEND directory
                                                                              number.(also <NUMBER> present)*/
  IPC_SS_CM_5_SEND,                      /* 0x09 : Activates the Completion of Calls to
                                                                                  Busy Subscriber Request.*/
  IPC_SS_CM_6_SEND,                      /* 0x0a : Release a held conference call.
                                                                               (This is a special-case to enable us to pass
                                                                               FTA case GSM 51.010 31.4.4.1.2.4) */
  IPC_SS_CM_MAX
} ipc_ss_cm_mode_e_type;


/*====================================================================*/


/*=================================================================

   SUB_CMD(1) : IPC_SS_USSD   0x08              SS USSD Message
   
==================================================================*/

/*    IPC_CMD_GET                       0x02     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_EVENT                   0x05     */
/*    IPC_CMD_RESP                     0x02     */
//none



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_EXEC                     0x01

 DESCRIPTION :
    Execute USSD Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | USSD_TYPE(1) | DCS(1) | STRING_LEN(1) |  STRING(x)
-------------------------------------------------------------------------------------
*/

/* USSD_TYPE */
typedef enum {
    IPC_SS_USSD_TYPE_USER_INITIATED=0x01,   /* User Initiated USSD Message */
    IPC_SS_USSD_TYPE_USER_RES,                       /* User Response to Network Initiated Message */
    IPC_SS_USSD_TYPE_USER_RELEASE,               /* SS Termination by user */
    IPC_SS_USSD_TYPE_MAX,
} ipc_ss_ussd_type_e_type;


/* DCS */
/* Refer to Refer to 3GPP TS 23.038 "CBS Data Coding Scheme" */ 

/* STRING_LEN  field : STRING field length */

/* STRING field : USSD string */
/* see MAX_SS_USSD_STRING_LEN */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_CFRM                     0x04

 DESCRIPTION :
    Confirm USSD Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | DCS(1) | STRING_LEN(1) |  STRING(x)
-------------------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_INDI                     0x01

 DESCRIPTION :
    USSD Message Indication
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | STATUS(1) | DCS(1) | STRING_LEN(1) |  STRING(x)
-------------------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
    USSD Message Notification
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | STATUS(1) | DCS(1) | STRING_LEN(1) |  STRING(x)
-------------------------------------------------------------------------------------
*/

/* STATUS field */
typedef enum {
	IPC_SS_USSD_NO_ACTION_REQUIRE = 0x01,  /* 0x01 : no further user action required 
	                                                                                            (network initiated USSD Notify, or no further 
	                                                                                            information needed after mobile initiated operation) */
	IPC_SS_USSD_ACTION_REQUIRE,                     /* 0x02 : further user action required 
	                                                                                            (network initiated USSD Request, or further 
	                                                                                            information needed after mobile initiated operation) */
	IPC_SS_USSD_TERMINATED_BY_NET,              /* 0x03 : USSD terminated by network */
	IPC_SS_USSD_OTHER_CLIENT,                         /* 0x04 : other local client has responded */
	IPC_SS_USSD_NOT_SUPPORT,                          /* 0x05 : operation not supported */
	IPC_SS_USSD_TIME_OUT,                                 /* 0x06 : network time out */
	IPC_SS_USSD_MAX
} ipc_ss_ussd_status_e_type;


/*=================================================================

   SUB_CMD(1) : IPC_SS_AOC   0x09              SS AOC Message
   
==================================================================*/

/*    IPC_CMD_EXEC                     0x01     */
/*    IPC_CMD_CFRM                     0x04     */
/*    IPC_CMD_EVENT                    0x05     */
/*    IPC_CMD_INDI                     0x01     */

//none

/* AOC_TYPE */
typedef enum {
    IPC_SS_AOC_TYPE_RESET		=0x00,   		/* AoC Reset Message */
    IPC_SS_AOC_TYPE_ACM			=0x01,		/* Accumulated call meter Message */
    IPC_SS_AOC_TYPE_CCM			=0x02,		/* Current call meter Message */
    IPC_SS_AOC_TYPE_MAXACM     	=0x04,		/* Max Accumulated call meter Message */
    IPC_SS_AOC_TYPE_PUC			=0x08,		/* Price per unit and currency Message */
    IPC_SS_AOC_TYPE_MAX 		=0x10
} ipc_ss_aoc_type_e_type;

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
    Get AOC Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | AOC_TYPE(1) |
-------------------------------------------------------------------------------------
*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
    Set AOC Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | AOC_TYPE(1) | 
-------------------------------------------------------------------------------------
 | ACM(4) | CCM(4) | MAXACM(4) | PPU(4) | CHARTYPE(1) | CURRENCY(3) | 
-------------------------------------------------------------------------------------
*/
/* AOC_TYPE has belows type */
/*
IPC_SS_AOC_TYPE_RESET
IPC_SS_AOC_TYPE_MAXACM 
IPC_SS_AOC_TYPE_PUC
*/
/*CHARTYPE*/
/*see ipc_text_enc_e_type*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
    Response Aoc Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | AOC_TYPE(1) | 
-------------------------------------------------------------------------------------
 | ACM(4) | CCM(4) | MAXACM(4) | PPU(4) | CHARTYPE(1) | CURRENCY(3) | 
-------------------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
    Notification Aoc Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | AOC_TYPE(1) | 
-------------------------------------------------------------------------------------
 | ACM(4) | CCM(4) | MAXACM(4) | PPU(4) | CHARTYPE(1) | CURRENCY(3) | 
-------------------------------------------------------------------------------------
*/
/* AOC_TYPE has belows type */
/*
IPC_SS_AOC_TYPE_CCM 
IPC_SS_AOC_TYPE_MAXACM :
*/


/*=================================================================

   SUB_CMD(1) : IPC_SS_RELEASE_COMPLETE   0x0A              SS Release Complete Message
   
==================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_RESP             0x02     */
//none

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
    Notification Release Complete Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | DATA_LEN(1) | DATA(X)
-------------------------------------------------------------------------------------
*/
 

#if 0// jbg delete
/*=================================================================

   SUB_CMD(1) : IPC_SS_ERR    0x0C                   Supplementary Service Error Message
   
==================================================================*/

/*    IPC_CMD_EXEC                     0x01     */
/*    IPC_CMD_GET                       0x02     */
/*    IPC_CMD_SET                        0x03     */
/*    IPC_CMD_CFRM               0x04     */
/*    IPC_CMD_EVENT                   0x05     */
/*    IPC_CMD_INDI                      0x01     */
/*    IPC_CMD_RESP                     0x02     */
//none


/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
    Supplementary Service Status Notification
 FORMAT :
---------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | ERROR |
---------------------------------------------------------------------
         1                 1                    1                1                   
-------------------------------------------------------------------*/

/* ERROR  field */
typedef enum{
IPC_SS_ERR_NONE,
/**/
IPC_SS_ERR_UNKNOWN_SUBSCRIBER,                  /* 0x01 : unknownSubscriber  "Invalid User Number" */
IPC_SS_ERR_DEFLECTION_SUBSCRIBER,               /* 0x02 : DeflectionToServedSubscriber "Deflected to own number " */
IPC_SS_ERR_INVALID_DEFLECT_NUM,            /* 0x03 : InvalidDeflectedToNumber "Invalid deflected to number " */
IPC_SS_ERR_BEARER_NOT_PROVISION,                /* 0x04 : BearerServiceNotProvisioned "Request Rejected" */
IPC_SS_ERR_TELESERVICE_NOT_PROVISION,       /* 0x05 : TeleServiceNotProvisioned */
IPC_SS_ERR_CALL_BARRED,                                   /* 0x06 : CallBarred */
IPC_SS_ERR_ILLEGAL_SS_OPER,                        /* 0x07 : illegalSS_Operation */
IPC_SS_ERR_ERR_STATUS,                                 /* 0x08 : ss_ErrorStatus */
IPC_SS_ERR_SUBSCRIPTION_VIOLATION,              /* 0x09 : ss_SubscriptionViolation */
IPC_SS_ERR_INCOMPATIBILITY,                             /* 0x0A : ss_Incompatibility */
IPC_SS_ERR_FACILITY_NOT_SUPPORT,                  /* 0x0B : FacilityNotSupported */
IPC_SS_ERR_PW_ATTEMPS_VIOLATION,                /* 0x0C : numberOfPW_AttemptsViolation - password ִ  õ Ƚ  */
IPC_SS_ERR_MPTY_PARTICIPANTS_EXCEED,         /* 0x0D : maxNumberOfMPTY_ParticipantsExceeded */
IPC_SS_ERR_SPECIAL_SVC_CODE,                  /* 0x0E : SpecialServiceCode */
IPC_SS_ERR_NOT_AVAIL,                                /* 0x0F : ss_NotAvailable "Service not available" */
IPC_SS_ERR_RESOURCE_NOT_AVAIL,             /* 0x10 : ResourcesNotAvailable */
IPC_SS_ERR_SYS_FAILURE,                              /* 0x11 : "SystemFailure "Please try again" */
IPC_SS_ERR_DATA_MISSING,                                  /* 0x12 : DataMissing */
IPC_SS_ERR_UNEXPECTED_DATA,/* UnexpectedDataValue 0x13 */
IPC_SS_ERR_PW_REGIST_FAIL,                    /* 0x14 : PasswordRegistrationFailure -ο password Ͻ error ߻  ( ex:password mismatch ) */
IPC_SS_ERR_NEGATIVE_PW_CHECK,                         /* 0x15 : NegativePasswordCheck - password Է½ ߸ Է  */
IPC_SS_ERR_UNRECOGNIZED_COMPONENT,             /* 0x16 : UNRECOGNIZED_COMPONENT "Please try again" */
IPC_SS_ERR_BADLY_STRUCTURED_COMPONENT,     /* 0x17 : BADLY_STRUCTURED_COMPONENT */

IPC_SS_ERR_MISTYPED_COMPONENT,                      /* 0x18 : MISTYPED_COMPONENT */
IPC_SS_ERR_UNRECOGNISED_INVOKE_ID,              /* 0x19 : UNRECOGNISED_INVOKE_ID "Request Rejected" */
IPC_SS_ERR_RETURN_RESULT_UNEXPECTED,          /* 0x1A : RETURN_RESULT_UNEXPECTED */
IPC_SS_ERR_RETURN_ERR_UNEXPECTED,            /* 0x1B : RETURN_ERROR_UNEXPECTED */
IPC_SS_ERR_UNRECOGNISED_ERR,                      /* 0x1C : UNRECOGNISED_ERROR */
IPC_SS_ERR_RE_MISTYPED_PARAM,                /* 0x1D : RE_MISTYPED_PARAMETER */

IPC_SS_ERR_DUPLICATE_INVOKE_ID,                      /* 0x1E : DUPLICATE_INVOKE_ID "Request Rejected" */
IPC_SS_ERR_UNRECOGNISED_OPER,              /* 0x1F : UNRECOGNISED_OPERATION */
IPC_SS_ERR_INITIATING_REL,                         /* 0x20 : INITIATING_RELEASE */
IPC_SS_ERR_UNRECOGNISED_LINKED_ID,               /* 0x21 : UNRECOGNISED_LINKED_ID */
IPC_SS_ERR_LINKED_RESP_UNEXPECTED,              /* 0x22 : LINKED_RESPONSE_UNEXPECTED */
IPC_SS_ERR_UNEXPECTED_LINKED_OPER,     /* 0x23 : UNEXPECTED_LINKED_OPERATION */
IPC_SS_ERR_MISTYPED_PARAM,                       /* 0x24 : MISTYPED_PARAMETER "Please try again" */
IPC_SS_ERR_RESOURCE_LIMITATION,                      /* 0x25 : RESOURCE_LIMITATION */
IPC_SS_ERR_UNKNOWN_ERR,                               /* 0x26 : Unknown Errors */
IPC_SS_ERR_MAX
} ipc_ss_error_e_type;
#endif//if 0

/*====================================================================*/





/*********************************************************************************

                                            Sub Command of IPC_GPRS_CMD[0x0D]

**********************************************************************************/
typedef enum{
  IPC_GPRS_DEFINE_PDP_CONTEXT=0x01, 	/* 0x01 : Define PDP Context */
  IPC_GPRS_QOS,  						/* 0x02 : Quality of Service Profile */
  IPC_GPRS_PS,  						/* 0x03 : PS attach or detach */
  IPC_GPRS_PDP_CONTEXT,   				/* 0x04 : PDP context activate or deactivate */                                                                     
  IPC_GPRS_ENTER_DATA,  				/* 0x05 : Enter data */
  IPC_GPRS_SHOW_PDP_ADDR,   			/* 0x06 : Show PDP address*/
  IPC_GPRS_MS_CLASS,  					/* 0x07 : GPRS mobile station class*/
  IPC_GPRS_3G_QUAL_SRVC_PROFILE,		/* 0x08 : 3G Quality of service profile */
  IPC_GPRS_IP_CONFIGURATION,			/* 0x09 : Multiple PDP context IP configuration */
  IPC_GPRS_DEFINE_SEC_PDP_CONTEXT,	/*0X0A : AT+CGDSCONT 	05.10.07 ky.doo EDIT */
  IPC_GPRS_TFT,						/*0X0B : AT+CGTFT 	05.10.07 ky.doo EDIT */
  IPC_GPRS_HSDPA_STATUS,			/* 0x0C : HSDPA status */
  IPC_GPRS_CURRENT_SESSION_DATA_COUNTER, /* 0x0D : Current data session tx/rx total bytes */
  IPC_GPRS_DATA_DORMANT,				/* 0x0E : Force to set Dormant */  
  IPC_GPRS_PIN_CTRL ,					/* 0x0F : Dial up Networking Pin Control Message */  
  IPC_GPRS_CALL_STATUS,				/* 0x10 : DS TE2 Data Call result(Multiple PDP) */  
  IPC_GPRS_MAX
} ipc_sub_gprs_cmd_type; 

/*********************************************************************************/

/*=================================================================

   SUB_CMD(1) : IPC_GPRS_DEFINE_PDP_CONTEXT      0x01                        Define PDP Context

=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_NOTI             0x03     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
    Define PDP Context Get
 FORMAT :
------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
------------------------------------------
*/

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
    Define PDP Context Set
 FORMAT :

---------------------------------------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MODE(1) | CID(1) | PDP_TYPE(1) |
---------------------------------------------------------------------------
---------------------------------------------------------------------
| APN(102) | PDP_ADDRESS(20) | D_COMP(1) | H_COMP(1) | AUTHTYPE (1) |
---------------------------------------------------------------------
*/

/* Define PDP Context Set Mode (MODE)*/
typedef enum{
IPC_GPRS_DEFINE_PDP_MODE_ADD=0x01,  	/* 0x01 : Add PDP information */
IPC_GPRS_DEFINE_PDP_MODE_DELETE,  		/* 0x02 : Delete PDP information */
IPC_GPRS_DEFINE_PDP_MODE_MAX
} ipc_gprs_define_pdp_mode_e_type;

/* Packet Data Protocol Type (P_TYPE) */
typedef enum{
  IPC_GPRS_DEFINE_PDP_CONTEXT_P_TYPE_UNKNOWN=0x00,  	/* 0x00 : Unknown */
  IPC_GPRS_DEFINE_PDP_CONTEXT_P_TYPE_X25,  			/* 0x01 : ITU-T/CCITT X.25 Layer 4 */
  IPC_GPRS_DEFINE_PDP_CONTEXT_P_TYPE_IP,  			/* 0x02 : Internet Protocol (IETF STD 5) */
  IPC_GPRS_DEFINE_PDP_CONTEXT_P_TYPE_IHOST,   		/* 0x03 : Internet Hosted Octet Stream Protocol */
  IPC_GPRS_DEFINE_PDP_CONTEXT_P_TYPE_PPP,  			/* 0x04 : Point-to-Point Protocol */
  IPC_GPRS_DEFINE_PDP_CONTEXT_P_TYPE_IPV6,   		/* 0x05 : Internet Protocol, version 6(IETF RFC 2460)-Maybe not use */
  IPC_GPRS_DEFINE_PDP_CONTEXT_P_TYPE_MAX
} ipc_gprs_define_pdp_context_p_e_type;

/* PDP Data Compression Type (D_COMP) */
typedef enum{
  IPC_GPRS_DEFINE_PDP_CONTEXT_D_COMP_OFF=0x00,  	/* 0x00 : Compression off (Default) */
  IPC_GPRS_DEFINE_PDP_CONTEXT_D_COMP_ON,  			/* 0x01 : Compression on */
  IPC_GPRS_DEFINE_PDP_CONTEXT_D_COMP_V42BIS, 		/* 0x02 : V42BIS (maybe not use) */
  IPC_GPRS_DEFINE_PDP_CONTEXT_D_COMP_V44,   		/* 0x03 : V44 (maybe not use) */
  IPC_GPRS_DEFINE_PDP_CONTEXT_D_COMP_MAX
} ipc_gprs_define_pdp_context_d_comp_e_type;

/* PDP Header Compression Type (H_COMP) */
typedef enum{
  IPC_GPRS_DEFINE_PDP_CONTEXT_H_COMP_OFF=0x00,  	/* 0x00 : Compression off (Default) */
  IPC_GPRS_DEFINE_PDP_CONTEXT_H_COMP_ON,  			/* 0x01 : Compression on */
  IPC_GPRS_DEFINE_PDP_CONTEXT_H_COMP_RFC1144, 		/* 0x02 : RFC1144 (maybe not use) */
  IPC_GPRS_DEFINE_PDP_CONTEXT_H_COMP_RFC2507,   	/* 0x03 : RFC2507 (maybe not use) */
  IPC_GPRS_DEFINE_PDP_CONTEXT_H_COMP_RFC3095,		/* 0x04 : RFC3095 (maybe not use) */
  IPC_GPRS_DEFINE_PDP_CONTEXT_H_COMP_MAX
} ipc_gprs_define_pdp_context_h_comp_e_type;

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
    Define PDP Context Set
 FORMAT :
-------------------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | NUM_PDP(1) |
-------------------------------------------------------
--------------------------------------------------------------------------------
(| CID(1) | PDP_TYPE(1) | APN(102) | PDP_ADDRESS(20) | D_COMP(1) | H_COMP(1) |)
--------------------------------------------------------------------------------
*/


/*=================================================================

   SUB_CMD(1) : IPC_GPRS_QOS 	0x02 	Quality of Service

=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_NOTI             0x03     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
    Quality of Service Profile Get
 FORMAT :
--------------------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | QOS_TYPE(1) |
--------------------------------------------------------
*/
/* Quality of Service Profile Type (TYPE) */
typedef enum{
  IPC_GPRS_QOS_TYPE_REQ=0x01,   	/* 0x01 : Quality of Service Profile Type Request (+CGQREQ) */
  IPC_GPRS_QOS_TYPE_MIN,  			/* 0x02 : Quality of Service Profile Type Minimum Acceptable (+CGQMIN) */
  IPC_GPRS_QOS_TYPE_MAX
} ipc_gprs_qos_e_type;


/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
    Quality of Service Profile Set
 FORMAT :
---------------------------------------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MODE(1) | QOS_TYPE(1) | CID(1) |
---------------------------------------------------------------------------
-------------------------------------------------------------------------------------
| QOS_PRECEDENCE(1) | QOS_DELAY(1) | QOS_RELIABILITY(1) | QOS_PEAK(1) | QOS_MEAN(1) |
-------------------------------------------------------------------------------------
*/

/* Quality of Service Profile Set Mode (MODE)*/
typedef enum{
IPC_GPRS_QOS_MODE_ADD=0x01,  		/* 0x01 : Add Quality of Service Profile */
IPC_GPRS_QOS_MODE_DELETE,   		/* 0x02 : Delete Quality of Service Profile */
IPC_GPRS_QOS_MODE_MAX
} ipc_gprs_qos_mode_e_type;

/* Quality of Service Precedence (PREC) */
typedef enum{
  IPC_GPRS_QOS_PREC_SUBSCRIBED=0x00,  	/* 0x00 : Subscribed by the Network / default if value is omitted */
  IPC_GPRS_QOS_PREC_HIGH,  				/* 0x01 : Service commitments shall be maintained ahead of precedence classes 2 and 3 */
  IPC_GPRS_QOS_PREC_NORMAL,  			/* 0x02 : Service commitments shall be maintained ahead of precedence class 3 */
  IPC_GPRS_QOS_PREC_LOW,   				/* 0x03 : Service commitments shall be maintained ahead of precedence classes 1 and 2 */
  IPC_GPRS_QOS_PREC_MAX
} ipc_gprs_qos_prec_e_type;

/* Quality of Service Delay (DELAY) */
typedef enum{
  IPC_GPRS_QOS_DELAY_SUBSCRIBED=0x00,  	/* 0x00 : Subscribed by the Network / default if value is omitted */
  IPC_GPRS_QOS_DELAY_PRED1,  			/* 0x01 : Predictive 1 */
  IPC_GPRS_QOS_DELAY_PRED2,  			/* 0x02 : Predictive 2 */
  IPC_GPRS_QOS_DELAY_PRED3,   			/* 0x03 : Predictive 3 */
  IPC_GPRS_QOS_DELAY_PRED4, 			/* 0x04 : Predictive 4 */
  IPC_GPRS_QOS_DELAY_MAX
} ipc_gprs_qos_delay_e_type;

/* Quality of Service Reliability (RELI) */
typedef enum{
  IPC_GPRS_QOS_RELI_SUBSCRIBED=0x00,  	/* 0x00 : Subscribed by the Network / default if value is omitted */
  IPC_GPRS_QOS_RELI_CLASS1,  			/* 0x01 : Reliability class 1 */
  IPC_GPRS_QOS_RELI_CLASS2,  			/* 0x02 : Reliability class 2 */
  IPC_GPRS_QOS_RELI_CLASS3,   			/* 0x03 : Reliability class 3 */
  IPC_GPRS_QOS_RELI_CLASS4, 			/* 0x04 : Reliability class 4 */
  IPC_GPRS_QOS_RELI_CLASS5, 			/* 0x05 : Reliability class 5 */
  IPC_GPRS_QOS_RELI_MAX
} ipc_gprs_qos_reli_e_type;

/* Quality of Service Peak (PEAK) */
typedef enum{
  IPC_GPRS_QOS_PEAK_SUBSCRIBED=0x00,  	/* 0x00 : Subscribed by the Network / default if value is omitted */
  IPC_GPRS_QOS_PEAK_8K,  				/* 0x01 : 8,000 bit per second */
  IPC_GPRS_QOS_PEAK_16K,  				/* 0x02 : 16,000 bit per second */
  IPC_GPRS_QOS_PEAK_32K,   				/* 0x03 : 32,000 bit per second */
  IPC_GPRS_QOS_PEAK_64K, 				/* 0x04 : 64,000 bit per second */
  IPC_GPRS_QOS_PEAK_128K, 				/* 0x05 : 128,000 bit per second */
  IPC_GPRS_QOS_PEAK_256K, 				/* 0x06 : 256,000 bit per second */
  IPC_GPRS_QOS_PEAK_512K, 				/* 0x07 : 512,000 bit per second */
  IPC_GPRS_QOS_PEAK_1024K, 				/* 0x08 : 1,024,000 bit per second */
  IPC_GPRS_QOS_PEAK_2048K, 				/* 0x09 : 2,048,000 bit per second */  
  IPC_GPRS_QOS_PEAK_MAX
} ipc_gprs_qos_peak_e_type;


/* Quality of Service Mean (MEAN) */
typedef enum{
  IPC_GPRS_QOS_MEAN_SUBSCRIBED=0x00,  	/* 0x00 : Subscribed by the Network / default if value is omitted */
  IPC_GPRS_QOS_MEAN_1,  				/* 0x01 : 0.22 bits per second */
  IPC_GPRS_QOS_MEAN_2,  				/* 0x02 : 0.44 bit per second */
  IPC_GPRS_QOS_MEAN_3,   				/* 0x03 : 1.11 bit per second */
  IPC_GPRS_QOS_MEAN_4, 					/* 0x04 : 2.2 bit per second */
  IPC_GPRS_QOS_MEAN_5, 					/* 0x05 : 4.4 bit per second */
  IPC_GPRS_QOS_MEAN_6, 					/* 0x06 : 11.1 bit per second */
  IPC_GPRS_QOS_MEAN_7, 					/* 0x07 : 22 bit per second */
  IPC_GPRS_QOS_MEAN_8, 					/* 0x08 : 44 bit per second */
  IPC_GPRS_QOS_MEAN_9, 					/* 0x09 : 111 bit per second */  
  IPC_GPRS_QOS_MEAN_10,					/* 0x10 : 220 bit per second */ 
  IPC_GPRS_QOS_MEAN_11,					/* 0x11 : 440 bit per second */
  IPC_GPRS_QOS_MEAN_12,					/* 0x12 : 1,110 bit per second */
  IPC_GPRS_QOS_MEAN_13,					/* 0x13 : 2,200 bit per second */
  IPC_GPRS_QOS_MEAN_14,					/* 0x14 : 4,400 bit per second */
  IPC_GPRS_QOS_MEAN_15,					/* 0x15 : 11,100 bit per second */
  IPC_GPRS_QOS_MEAN_16,					/* 0x16 : 22,000 bit per second */
  IPC_GPRS_QOS_MEAN_17,					/* 0x17 : 44,000 bit per second */
  IPC_GPRS_QOS_MEAN_18,					/* 0x18 : 111,000 bit per second */
  IPC_GPRS_QOS_MEAN_31=0x31,					/* 0x31 : Best effort */ 
  IPC_GPRS_QOS_MEAN_MAX
} ipc_gprs_qos_mean_e_type;


/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
    Quality of Service Profile Response
 FORMAT :
-------------------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | NUM_QOS(1) |
-------------------------------------------------------
-------------------------------------------------------------------------------------------------
(| CID(1) | QOS_PRECEDENCE(1) | QOS_DELAY(1) | QOS_RELIABILITY(1) | QOS_PEAK(1) | QOS_MEAN(1) |)
-------------------------------------------------------------------------------------------------
* 0 <= NUM_QOS(1) <= 4
*/



/*=================================================================

   SUB_CMD(1) : IPC_GPRS_PS 	0x03 	Packet Switch attach or detach

=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
    Packet Switch State Get
 FORMAT :
------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
------------------------------------------
*/


/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
    Packet Switch State Set
 FORMAT :
-----------------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | STATE(1) |
-----------------------------------------------------
*/

/* Packet Switch State Type (STATE) */
typedef enum{
  IPC_GPRS_PS_STATE_DETACHED=0x00,   	/* 0x00 : Detached */
  IPC_GPRS_PS_STATE_ATTACHED,  			/* 0x01 : Attached */
  IPC_GPRS_PS_STATE_MAX
} ipc_gprs_ps_state_e_type;


/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
    Packet Switch State Response
 FORMAT :
-------------------------------------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | STATE(1) | TRANSFER_STATUS(1)|
-------------------------------------------------------------------------
*/


/* STATE field */
/* see ipc_gprs_ps_state_e_type 
*/


/* Packet Data Transfer Status Type */
typedef enum{
  IPC_GPRS_PS_SUSPEND =0x00,   	/* 0x00 : Packet Data Transfer Status is Suspend */
  IPC_GPRS_PS_ACTIVE,  			/* 0x01 : Packet Data Transfer Status is Active(Resume) */
  IPC_GPRS_PS_TRANSFER_STATUS_MAX
} ipc_gprs_ps_transfer_e_type;


/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
    Packet Switch State Notification
 FORMAT :
-------------------------------------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | STATE(1) | TRANSFER_STATUS(1)|
-------------------------------------------------------------------------
*/


/* STATE field */
/* see ipc_gprs_ps_state_e_type 
*/


/* STATE field */
/* see ipc_gprs_ps_transfer_e_type 
*/

/*=================================================================

   SUB_CMD(1) : IPC_GPRS_PDP_CONTEXT  0x04 	PDP context activate or deactivate

=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_NOTI             0x03     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
    PDP context State Get
 FORMAT :
------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
------------------------------------------*/


/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
    PDP context State Set
 FORMAT :
------------------------------------------------------------------------------------------------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | STATE(1) | CID(1) |BIT_MASK(4)|USER_NAME(32)|PASSWORD(32)|DNS1(16)|DNS2(16)|AUTH_TYPE(1)|
------------------------------------------------------------------------------------------------------------------------------------
*/

/* PDP context State Type (STATE) */
typedef enum {
  IPC_GPRS_PDP_CONTEXT_STATE_DETACHED=0x00,   	/* 0x00 : Detached */
  IPC_GPRS_PDP_CONTEXT_STATE_ATTACHED,  		/* 0x01 : Attached */
  IPC_GPRS_PDP_CONTEXT_STATE_CANCEL,            /* 0x02 : Cancel   */
  IPC_GPRS_PDP_CONTEXT_STATE_MAX
} ipc_gprs_pdp_context_state_e_type;

typedef enum {
  BIT_MASK_USERNAME =	0x00000001,
  BIT_MASK_PASSWORD =	0x00000002,
  BIT_MASK_DNS1	    =    0x00000004,
  BIT_MASK_DNS2	    =    0x00000008,
  BIT_MASK_AUTH_TYPE = 0x00000010,
  BIT_MASK_PCSCF = 0x00000020
  
}ipc_gprs_pdp_context_bit_mask_e_type;

typedef enum {
  IPC_GPRS_DEFINE_PDP_CONTEXT_AUTHTYPE_NONE=0x00,  	/* 0x00 : NONE */
  IPC_GPRS_DEFINE_PDP_CONTEXT_AUTHTYPE_PAP=0x01,  	/* 0x01 : PAP */
  IPC_GPRS_DEFINE_PDP_CONTEXT_AUTHTYPE_CHAP=0x02,  	/* 0x02 : CHAP */
}ipc_gprs_pdp_context_auth_e_type;


/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
    PDP context State Response
 FORMAT :
-----------------------------------------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | NUM_ACT(1) (| CID(1) | STATE(1) |)
-----------------------------------------------------------------------------
* 0 <= NUM_ACT(1) <= 4

*/



/*=================================================================

   SUB_CMD(1) : IPC_GPRS_ENTER_DATA  0x05 	ENTER Data

=================================================================*/

/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_SET              0x03     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_RESP             0x02     */
/*    IPC_CMD_NOTI             0x03     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_EXEC                     0x01

 DESCRIPTION :
    Enter Data (Packet Data call Start)
 FORMAT :
---------------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CID(1) |
---------------------------------------------------
*/



/*=================================================================

   SUB_CMD(1) : IPC_GPRS_SHOW_PDP_ADDR  0x06 	Show PDP address

=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_SET              0x03     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_NOTI             0x03     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
    PDP address Get
 FORMAT :
---------------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CID(1) |
---------------------------------------------------
*/


/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
    PDP address Get
 FORMAT :
----------------------------------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CID(1) | PDP_ADDRESS(20) |
----------------------------------------------------------------------
*/


/*=================================================================

   SUB_CMD(1) : IPC_GPRS_MS_CLASS  0x07 	GPRS mobile station class

=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_NOTI             0x03     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
    GPRS mobile station class get
 FORMAT :
------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
------------------------------------------
*/


/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
    GPRS mobile station class Set
 FORMAT :
---------------------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MS_CLASS(1) | 
---------------------------------------------------------
*/

/* GPRS mobile station class (MS_CLASS) */
typedef enum {
  IPC_GPRS_MS_CLASS_GSMANDGPRS=0x00,  				/* 0x00 : "A" : Simultaneous voice and GPRS data */
  IPC_GPRS_MS_CLASS_GSMORGPRS,  				/* 0x01 : "B" : Simultaneous voice and GPRS traffic channel, one or other data */
  IPC_GPRS_MS_CLASS_GSMORGPRS_EXCLUSIVE,  		/* 0x02 : "C" : Either all voice or all GPRS, both traffic channels unmonitored */
  IPC_GPRS_MS_CLASS_GPRSONLY,  					/* 0x03 : "CC" : Only GPRS */
  IPC_GPRS_MS_CLASS_GSMONLY,  					/* 0x04 : "CG" : Only circuit switched voice and data */
  IPC_GPRS_MS_CLASS_MAX
} ipc_gprs_ms_class_e_type;


/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
    GPRS mobile station class Response
 FORMAT :
---------------------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MS_CLASS(1) | 
---------------------------------------------------------
*/


/*===========================================================================================

   SUB_CMD(1) : IPC_GPRS_3G_QUAL_SRVC_PROFILE     0x08     3G Quality of Service Profile

===========================================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_NOTI             0x03     */
//none



/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
    3G Quality of Service Profile get
 FORMAT :
-----------------------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 3G_QOS_TYPE(1) |
-----------------------------------------------------------
*/

/* 3G_QOS_TYPE */
typedef enum {
  IPC_GPRS_3G_QOS_TYPE_REQUESTED=0x01,   		/* 0x00 : Requested(+CGEQREQ) */
  IPC_GPRS_3G_QOS_TYPE_MINIMUM_ACCEPTABLE,  	/* 0x01 : Minimum Acceptable(+CGEQMIN) */
  IPC_GPRS_3G_QOS_TYPE_NEGOTIATED,  			/* 0x02 : Negotiated(+CGEQNEG) */
  IPC_GPRS_3G_QOS_TYPE_MAX
} ipc_gprs_3g_qos_type_e_type;



/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
    3G Quality of Service Profile Set
 FORMAT :
-------------------------------------------------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 3G_QOS_MODE(1) | 3G_QOS_TYPE(1) | CID(1) |
-------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------
| 3G_QOS_TRAFFIC(1) | 3G_QOS_MAX_BITRATE_UL(1) | 3G_QOS_MAX_BITRATE_DL(1) | 3G_QOS_GUARANTEED_BITRATE_UL(1) |
-------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------
| 3G_QOS_GUARANTEED_BITRATE_DL(1) | 3G_QOS_DELIVERY_ORDER(1) | 3G_QOS_MAX_SDU_SIZE(2) | 3G_QOS_SDU_ERROR(1) |
-------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------
| 3G_QOS_RESIDUAL_BIT_ERROR(1) | 3G_QOS_DELIVERY_ERROREOUS_SDU(1) | 3G_QOS_TRANSFER_DELAY(1) | 
----------------------------------------------------------------------------------------------
---------------------------------------
| 3G_QOS_TRAFFIC_HANDLING_PRIORITY(1) |
---------------------------------------
*/

/* 3G_QOS_MODE */
typedef enum{
  IPC_GPRS_3G_QOS_MODE_ADD=0x01,  		/* 0x01 : Add Quality of Service Profile */
  IPC_GPRS_3G_QOS_MODE_DELETE,   		/* 0x02 : Delete Quality of Service Profile */
  IPC_GPRS_3G_QOS_MODE_MAX
} ipc_gprs_3g_qos_mode_e_type;

/* 3G_QOS_TRAFFIC */
typedef enum {
  IPC_GPRS_3G_QOS_TRAFFIC_CONVERSATIONAL=0x00, 	/* 0x00 : Conversational */
  IPC_GPRS_3G_QOS_TRAFFIC_STREAMING,  			/* 0x01 : Streaming */
  IPC_GPRS_3G_QOS_TRAFFIC_INTERACTIVE,  		/* 0x02 : Interactive */
  IPC_GPRS_3G_QOS_TRAFFIC_BACKGROUND,  			/* 0x03 : Background */
  IPC_GPRS_3G_QOS_TRAFFIC_SUBSCRIVED_VALUE,  	/* 0x04 : Subscrived_value */
  IPC_GPRS_3G_QOS_TRAFFIC_MAX
} ipc_gprs_3g_qos_traffic_e_type;

/* 3G_QOS_MAX_BITRATE_UL / 3G_QOS_MAX_BITRATE_DL / 3G_QOS_GUARANTEED_BITRATE_UL / 3G_QOS_GUARANTEED_BITRATE_DL */
typedef enum {
  IPC_GPRS_3G_QOS_BITRATE_SUBSCRIBED=0x00, 	/* 0x00 : Subscribed by the Network / default if value is omitted */
  IPC_GPRS_3G_QOS_BITRATE_8K,  				/* 0x01 : 8,000 bits per second (bps) */
  IPC_GPRS_3G_QOS_BITRATE_16K,  			/* 0x02 : 16,000 bps */
  IPC_GPRS_3G_QOS_BITRATE_32K,  			/* 0x03 : 32,000 bps */
  IPC_GPRS_3G_QOS_BITRATE_64K,  			/* 0x04 : 64,000 bps */
  IPC_GPRS_3G_QOS_BITRATE_128K,  			/* 0x05 : 128,000 bps */
  IPC_GPRS_3G_QOS_BITRATE_256K,  			/* 0x06 : 256,000 bps */
  IPC_GPRS_3G_QOS_BITRATE_512K,  			/* 0x07 : 512,000 bps */
  IPC_GPRS_3G_QOS_BITRATE_1024K,  			/* 0x08 : 1,024,000 bps */
  IPC_GPRS_3G_QOS_BITRATE_2048K,  			/* 0x09 : 2,048,000 bps */
  IPC_GPRS_3G_QOS_BITRATE_MAX
} ipc_gprs_3g_qos_bitrate_e_type;

/* 3G_QOS_SDU_ERROR */
typedef enum {
  IPC_GPRS_3G_QOS_SDU_ERROR=0x00, 	/* 0x00 : Subscribed by the Network / default if value is omitted */
  IPC_GPRS_3G_QOS_SDU_ERROR_1E1,  	/* 0x01 : 10^(-1) */
  IPC_GPRS_3G_QOS_SDU_ERROR_1E2,  	/* 0x02 : 10^(-2) */
  IPC_GPRS_3G_QOS_SDU_ERROR_7E3,  	/* 0x03 : 7*(10^(-3)) */
  IPC_GPRS_3G_QOS_SDU_ERROR_1E3,  	/* 0x04 : 10^(-3) */
  IPC_GPRS_3G_QOS_SDU_ERROR_1E4,  	/* 0x05 : 10^(-4) */
  IPC_GPRS_3G_QOS_SDU_ERROR_1E5,  	/* 0x06 : 10^(-5) */
  IPC_GPRS_3G_QOS_SDU_ERROR_1E6,  	/* 0x07 : 10^(-6) */
  IPC_GPRS_3G_QOS_SDU_ERROR_MAX
} ipc_gprs_3g_qos_sdu_error_e_type;

/* 3G_QOS_RESIDUAL_BIT_ERROR */
typedef enum {
  IPC_GPRS_3G_QOS_RESIDUAL_BIT_ERROR=0x00, 	/* 0x00 : Subscribed by the Network / default if value is omitted */
  IPC_GPRS_3G_QOS_RESIDUAL_BIT_ERROR_5E2,  	/* 0x01 : 5*(10^(-2)) */
  IPC_GPRS_3G_QOS_RESIDUAL_BIT_ERROR_1E2,  	/* 0x02 : 10^(-2) */
  IPC_GPRS_3G_QOS_RESIDUAL_BIT_ERROR_5E3,  	/* 0x03 : 5*(10^(-3)) */
  IPC_GPRS_3G_QOS_RESIDUAL_BIT_ERROR_4E3,  	/* 0x04 : 4*(10^(-3)) */
  IPC_GPRS_3G_QOS_RESIDUAL_BIT_ERROR_1E4,  	/* 0x05 : 10^(-4) */
  IPC_GPRS_3G_QOS_RESIDUAL_BIT_ERROR_1E5,  	/* 0x06 : 10^(-5) */
  IPC_GPRS_3G_QOS_RESIDUAL_BIT_ERROR_1E6,  	/* 0x07 : 10^(-6) */
  IPC_GPRS_3G_QOS_RESIDUAL_BIT_ERROR_6E8,  	/* 0x08 : 6*(10^(-8)) */
  IPC_GPRS_3G_QOS_RESIDUAL_BIT_ERROR_MAX
} ipc_gprs_3g_qos_residual_bit_error_e_type;

/* 3G_QOS_TRAFFIC_HANDLING_PRIORITY */
typedef enum {
  IPC_GPRS_3G_QOS_TRAFFIC_HANDLING_PRIORITY_0=0x00, 	/* 0x00 : Subscribed by the Network / default if value is omitted */
  IPC_GPRS_3G_QOS_TRAFFIC_HANDLING_PRIORITY_1,  	/* 0x01 : 5*(10^(-2)) */
  IPC_GPRS_3G_QOS_TRAFFIC_HANDLING_PRIORITY_2,  	/* 0x02 : 10^(-2) */
  IPC_GPRS_3G_QOS_TRAFFIC_HANDLING_PRIORITY_3,  	/* 0x03 : 5*(10^(-3)) */
  IPC_GPRS_3G_QOS_TRAFFIC_HANDLING_PRIORITY_MAX
} ipc_gprs_3g_qos_traffic_handling_priority_e_type;



/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
    3G Quality of Service Profile Response
 FORMAT :
-------------------------------------------------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 3G_QOS_TYPE(1) | NUM_3G_QOS(1) (| CID(1) |
-------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------
| 3G_QOS_TRAFFIC(1) | 3G_QOS_MAX_BITRATE_UL(1) | 3G_QOS_MAX_BITRATE_DL(1) | 3G_QOS_GUARANTEED_BITRATE_UL(1) |
-------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------
| 3G_QOS_GUARANTEED_BITRATE_DL(1) | 3G_QOS_DELIVERY_ORDER(1) | 3G_QOS_MAX_SDU_SIZE(2) | 3G_QOS_SDU_ERROR(1) |
-------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------
| 3G_QOS_RESIDUAL_BIT_ERROR(1) | 3G_QOS_DELIVERY_ERROREOUS_SDU(1) | 3G_QOS_TRANSFER_DELAY(1) | 
----------------------------------------------------------------------------------------------
---------------------------------------
| 3G_QOS_TRAFFIC_HANDLING_PRIORITY(1) |
---------------------------------------
* 0 <= NUM_3G_QOS(1) <= 4
*/



/*=================================================================

   SUB_CMD(1) : IPC_GPRS_IP_CONFIGURATION  0x09 	Multiple PDP context IP configuration

=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_SET              0x03     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_RESP             0x02     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
    Notify IP configuration changed
 FORMAT :
-----------------------------------------------------------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CID(1) | FIELD_FLAG(2) | IP_ADDRESS(4) | PRIMARY_DNS(4) |
-----------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------
| SECONDARY_DNS(4) | DEFAULT_GATEWAY(4) | SUBNET_MASK(4) | | P-CSCF Address(4)
-------------------------------------------------------------------------------
*/

/* IP Configuration (FIELD_FLAG) */
typedef enum {
  IPC_GPRS_IP_CONFIGURATION_FIELD_FLAG_IP_ADDR=0x0001,				/* 0x0001 : IP Address */
  IPC_GPRS_IP_CONFIGURATION_FIELD_FLAG_PRIMARY_DNS=0x0002,		/* 0x0002 : Primary DNS */
  IPC_GPRS_IP_CONFIGURATION_FIELD_FLAG_SECONDARY_DNS=0x0004,	/* 0x0004 : Secondary DNS */
  IPC_GPRS_IP_CONFIGURATION_FIELD_FLAG_DEFAULT_GATEWAY=0x0008,	/* 0x0004 : Default Gateway */
  IPC_GPRS_IP_CONFIGURATION_FIELD_FLAG_SUBNET_MASK=0x0010,		/* 0x0010 : Subnet Mask */
  IPC_GPRS_IP_CONFIGURATION_FIELD_FLAG_PCSCF=0x0020,                /* 0x0020 : P-CSCF Address */
  IPC_GPRS_IP_CONFIGURATION_FIELD_FLAG_MAX=0xFFFF
} ipc_gprs_ip_configuration_field_flag_e_type;

/*=================================================================

   SUB_CMD(1) : IPC_GPRS_DEFINE_SEC_PDP_CONTEXT      0x0A                        Define secondary PDP Context

=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_RESP             0x02     */
/*    IPC_CMD_NOTI             0x03     */
// none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
    Define secondary PDP Context Set
 FORMAT :
----------------------------------------------------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  CID(1) | Primary_CID(1) | D_COMP(1) | H_COMP(1) |
----------------------------------------------------------------------------------------

---------------------------------------------------------------------
*/



/*=================================================================

   SUB_CMD(1) : IPC_GPRS_TFT      0x0B Traffic Flow Templete

=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_RESP             0x02     */
/*    IPC_CMD_NOTI             0x03     */
// none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
    Set Traffic Flow Templete
 FORMAT :
----------------------------------------------------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  CID(1) | PF_id(1) | EV_prec_index(1) | 
----------------------------------------------------------------------------------------
|src_addr(4) | subnet_mask(4) | prot_num(1) |  ...... later to be added
---------------------------------------------------------------------*/


/*=================================================================

   SUB_CMD(1) : IPC_GPRS_HSDPA_STATUS      0x0C HSDPA STATUS

=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_SET              0x03     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_RESP             0x02     */
// none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
    Notification HSDPA status
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | HSDPA_state(1) | 
-------------------------------------------------------------------------------------
*/
//  HSDPA_status field
//  Active : TRUE, Inactive : FALSE

/*==================================================================================

   SUB_CMD(1) : IPC_GPRS_CURRENT_SESSION_DATA_COUNTER      0x0D : Current data session tx/rx total bytes

===================================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_SET              0x03     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
//none


/*-----------------------------------------------------------------------------------
 
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
    Get current data session tx/rx total bytes
    This IPC is vaild during the call.
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | Context-ID(1) | 
-------------------------------------------------------------------------------------
*/

/*-----------------------------------------------------------------------------------
 
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
    Response current data session tx/rx total bytes
    This IPC is vaild during the call.
 FORMAT :
-----------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | Context-ID(1) | Total_rxbytes(4) | Total_txbytes(4)
-----------------------------------------------------------------------------------------------
*/

/*-----------------------------------------------------------------------------------
 
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
    Notify current data session tx/rx total bytes
    This IPC is used at end of the call
 FORMAT :
-----------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | Context-ID(1) | Total_rxbytes(4) | Total_txbytes(4)
-----------------------------------------------------------------------------------------------
*/

/*=================================================================

   SUB_CMD(1) : IPC_GPRS_DATA_DORMANT      0x0E : Data dormant Message

=================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_RESP              0x02     */
/*    IPC_CMD_NOTI               0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_INDI                0x01     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION : Data Dormant Message SET
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/


/*==================================================================================================

   SUB_CMD(1) : IPC_GPRS_PIN_CTRL    0x0F         DUN(Dial up netwotking) Pin Control Message
   
====================================================================================================*/

/*    IPC_CMD_EXEC                     0x01     */
/*    IPC_CMD_GET                       0x02     */
/*    IPC_CMD_CFRM                     0x04     */
/*    IPC_CMD_EVENT                   0x05     */
/*    IPC_CMD_INDI                      0x01     */
/*    IPC_CMD_RESP                     0x02     */
//none



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03 

 DESCRIPTION :
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SIGNAL(1) | STATUS(1) |
-------------------------------------------------------------------------------------
*/



/* Signal Line */
typedef enum {
  IPC_DATA_SIGNAL_NONE,
  IPC_DATA_SIGNAL_DCD,                /* 0x01 */
  IPC_DATA_SIGNAL_RX,                   /* 0x02 */
  IPC_DATA_SIGNAL_TX,                   /* 0x03 */
  IPC_DATA_SIGNAL_DTR,                /* 0x04 */
  IPC_DATA_SIGNAL_GND,                /* 0x05 */
  IPC_DATA_SIGNAL_DSR,                /* 0x06 */
  IPC_DATA_SIGNAL_RTS,                /* 0x07 */
  IPC_DATA_SIGNAL_CTS,                /* 0x08 */
  IPC_DATA_SIGNAL_RI,                  /* 0x09 */
  IPC_DATA_SIGNAL_MAX                /* 0x0A */
} ipc_data_pin_signal_e_type;

/* STATUS  field */
/* Status of Signal Line */
typedef enum {
  IPC_DATA_PIN_STATUS_OFF,    /* 0x00 */
  IPC_DATA_PIN_STATUS_ON      /* 0x01 */
} ipc_data_pin_status_e_type; 


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                        0x03 

 DESCRIPTION :
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SIGNAL(1) | STATUS(1) |
-------------------------------------------------------------------------------------
*/

/* SIGNAL */
/* refer to "ipc_data_pin_signal_e_type" type */

/* STATUS */
/* refet to "ipc_data_pin_status_e_type" type */


/*========================================================================================

   SUB_CMD(1) : IPC_GPRS_CALL_STATUS    0x10      GPRS Call State Result(MultiplePDP)

========================================================================================*/
/* 		IPC_CMD_EXEC                        0x01  */
/* 		IPC_CMD_GET                         0x02  */
/* 		IPC_CMD_SET                         0x03  */
/* 		IPC_CMD_CFRM                        0x04  */
/* 		IPC_CMD_EVENT                       0x05  */
/* 		IPC_CMD_INDI                        0x01  */
/* 		IPC_CMD_RES                         0x02  */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                        0x03

 DESCRIPTION :
   
 FORMAT :

-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CID(1) | CALLSTATE(1) | REASON(1) |
-------------------------------------------------------------------------------------
 | EXTERNAL_PS_CALL(1) |
------------------------

*/

/* CALLSTATE(1) : data call state result */
typedef enum {
  IPC_DATA_TE2_DATA_CALL_RESULT_OK=0x00,   		/* 0x00 : OK (maybe not use) */
  IPC_DATA_TE2_DATA_CALL_RESULT_CONNECT,  		/* 0x01 : CONNECT */
  IPC_DATA_TE2_DATA_CALL_RESULT_RING,  			/* 0x02 : RING (maybe not use) */
  IPC_DATA_TE2_DATA_CALL_RESULT_NO_CARRIER,  	/* 0x03 : NO CARRIER */
  IPC_DATA_TE2_DATA_CALL_RESULT_ERROR,  		/* 0x04 : ERROR */
  IPC_DATA_TE2_DATA_CALL_RESULT_NO_DIALTONE,  	/* 0x05 : NO DIALTONE (maybe not use) */
  IPC_DATA_TE2_DATA_CALL_RESULT_BUSY,  			/* 0x06 : BUSY (maybe not use)*/
  IPC_DATA_TE2_DATA_CALL_RESULT_NO_ANSWER,   	/* 0x07 : NO ANSWER */
  IPC_DATA_TE2_DATA_CALL_RESULT_FDN,		/* 0x08 : FDN */   
  IPC_DATA_TE2_DATA_CALL_RESULT_ACL,		/* 0x09 : ACL */ 
  IPC_DATA_TE2_DATA_CALL_RESULT_MAX
} ipc_data_te2_data_call_result_e_type;


// REASON(1) ky.doo suwon data call cause
typedef enum {
  IPC_DATA_CALL_REASON_NORMAL=0x00,
  IPC_DATA_CALL_REASON_REL_BY_USER,
  IPC_DATA_CALL_REASON_REGULAR_DEACTIVATION,
  IPC_DATA_CALL_REASON_LLC_SNDCP_FAILURE,
  IPC_DATA_CALL_REASON_INSUFFICIENT_RESOURCES,
  IPC_DATA_CALL_REASON_MISSING_OR_UNKNOWN_APN,          
  IPC_DATA_CALL_REASON_UNKNOWN_PDP_ADDRESS_OR_TYPE,
  IPC_DATA_CALL_REASON_USER_AUTHENTICATION_FAILED,
  IPC_DATA_CALL_REASON_ACTIVATION_REJECTED_BY_GGSN,
  IPC_DATA_CALL_REASON_ACTIVATION_REJECTED_UNSPECIFIED,
  IPC_DATA_CALL_REASON_SERVICE_OPTION_NOT_SUPPORTED, 				
  IPC_DATA_CALL_REASON_SERVICE_NOT_SUBSCRIBED, 
  IPC_DATA_CALL_REASON_SERVICE_OUT_OR_ORDER,
  IPC_DATA_CALL_REASON_NSAPI_ALREADY_USED,
  IPC_DATA_CALL_REASON_QOS_NOT_ACCEPTED,
  IPC_DATA_CALL_REASON_NETWORK_FAILURE,
  IPC_DATA_CALL_REASON_REACTIVATION_REQUIRED,
  IPC_DATA_CALL_REASON_FEATURE_NOT_SUPPORTED,
  IPC_DATA_CALL_REASON_TFT_OR_FILTER_ERR,
  IPC_DATA_CALL_REASON_UNKNOWN_PDP_CONTEXT,
  IPC_DATA_CALL_REASON_INVALID_MSG,
  IPC_DATA_CALL_REASON_PROTOCOL_ERR,
  IPC_DATA_CALL_REASON_MOBILE_FAILURE_ERR,
  IPC_DATA_CALL_REASON_TIMEOUT_ERR,
  IPC_DATA_CALL_REASON_UNKNOWN_ERR
}ipc_data_call_reason_e_type;

// EXTERNAL_PS_CALL: TRUE: External PS Call, FALSE : Internal PS Call 


/*********************************************************************************

                                            Sub Command of IPC_SAT_CMD[0x0E]

**********************************************************************************/
typedef enum{
  IPC_SAT_PROFILE_DOWNLOAD=0x01,          /* 0x01 : Profile Dwonload */
  IPC_SAT_ENVELOPE_CMD,                   /* 0x02 : Envelope Command */
  IPC_SAT_PROACTIVE_CMD,                  /* 0x03 : Proactive Command */
  IPC_SAT_TERMINATE_USAT_SESSION,         /* 0x04 : Terminate USAT Session */                                                                     
  IPC_SAT_EVENT_DOWNLOAD,  				  /* 0x05 : Event Download */
  IPC_SAT_PROVIDE_LOCAL_INFO,             /* 0x06 : Provide Local Information*/
  IPC_SAT_POLLING,			              /* 0x07 : Poll Interval or Polling Off*/
  IPC_SAT_REFRESH,			              /* 0x08 : Refresh*/
  IPC_SAT_SETUP_EVENT_LIST,			      /* 0x09 : Set Up Event List*/
  IPC_SAT_CALL_CONTROL_RESULT,            /* 0x0A : Call Control Result*/
  IPC_SAT_IMAGE_CLUT,                     /* 0x0B : Icon, Image, CLUT Message*/
  IPC_SAT_SETUP_CALL_PROCESSING,          /* 0x0C : Setup Call Processing Message*/
  IPC_SAT_SIM_INITIATE_MESSAGE,           /* 0x0D : SIM initiate message*/  
  IPC_SAT_MAX
} ipc_sub_sat_cmd_type;

/*********************************************************************************/


/*=================================================================

   SUB_CMD(1) : IPC_SAT_PROFILE_DOWNLOAD      0x01   Profile Dwonload Message
   
=================================================================*/


/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_NOTI             0x03     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
  Get Profile Download Message
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
   Set Profile Download Message
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | NOTI_RESULTCODE(1) | SAT_PROFILE(20) |
-------------------------------------------------------------------------------------
 */

/* NOTI_RESULTCODE  */
typedef enum{
   IPC_SAT_PROFILE_DISABLE_NOTI,
   IPC_SAT_PROFILE_ENABLE_NOTI
}ipc_sat_profile_resultcode_e_type;

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   Profile Download Response Message
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | NOTI_RESULTCODE(1) | SAT_PROFILE(20) |
-------------------------------------------------------------------------------------
 */

/*=================================================================

   SUB_CMD(1) : IPC_SAT_ENVELOPE_CMD      0x02   Envelope Command Message
   
=================================================================*/


/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_RESP             0x02     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_EXEC                        0x01

 DESCRIPTION :
  Execute Envelope Command Message
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |LEN_ENVELOPE_CMD(1)|ENVELOPE_CMD[]|
-------------------------------------------------------------------------------------
*/
/* ENVELOPE_CMD[]  field */
/* Maximum length : 256 bytes */

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
   Notification Envelope Command Message ==> Response of Envelope Command 
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |SW1(1) |SW2(1) |LEN_ENVELOPE_CMD(1)|ENVELOPE_RESP[]|
-------------------------------------------------------------------------------------
*/
/* ENVELOPE_RESP[]  field */
/* Maximum length : 256 bytes */


/*=================================================================

   SUB_CMD(1) : IPC_SAT_PROACTIVE_CMD      0x03   Proactive Command Message
   
=================================================================*/


/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_NOTI             0x03     */

//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
  Get Proactive Command Message
  Terminal Response( Response of Proactive Command) Message
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |LEN_PROACTIVE_RESP(1)|PROACTIVE_RESP[]|
-------------------------------------------------------------------------------------
*/
/* PROACTIVE_RESP[]  field */
/* Maximum length : 256 bytes */

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_INDI                     0x01

 DESCRIPTION :
   Indication Proactive Command Message 
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |LEN_PROACTIVE_RESP(2)|PROACTIVE_CMD[]|
-------------------------------------------------------------------------------------
*/
/* PROACTIVE_CMD[]  field */
/* Maximum length : 256 bytes */

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   Response Proactive Command Message
   Response( Response of Terminal Response) Message
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SW1(1) | SW2(1) |
-------------------------------------------------------------------------------------
 */


/*=================================================================

   SUB_CMD(1) : IPC_SAT_TERMINATE_USAT_SESSION 0x04   Terminate USAT Session Message
   
=================================================================*/


/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_RESP             0x02     */
/*    IPC_CMD_NOTI             0x03     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
  To terminate a USIM application toolkit command or session
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |USAT_TERM_CAUSES(1)|
-------------------------------------------------------------------------------------
*/
/* NOTI_RESULTCODE	*/
typedef enum{
   IPC_SAT_TERM_USER_STOP_REDIAL,
   IPC_SAT_TERM_END_REDIAL_REACHED,
   IPC_SAT_TERM_USER_END_SESSION
}ipc_sat_usat_term_resultcode_e_type;


/*=================================================================

   SUB_CMD(1) : IPC_SAT_EVENT_DOWNLOAD  0x05   Event Download Message
   
=================================================================*/


/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_RESP             0x02     */
/*    IPC_CMD_NOTI             0x03     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
  Event Download Message
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |LEN_EVENT_RESP(1)|EVENT_RESP[]|
-------------------------------------------------------------------------------------
*/
/* EVENT_RESP[]  field */
/* Maximum length : 256 bytes */

/*=================================================================

   SUB_CMD(1) : IPC_SAT_PROVIDE_LOCAL_INFO  0x06    Provide Local Information
   
=================================================================*/


/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_NOTI             0x03     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
  Get Provide local information message
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |INFO_TYPE(1)|
-------------------------------------------------------------------------------------
*/

typedef enum{
   IPC_SAT_PROVIDE_LOCI,           /*0x00:Location Information according to current NAA*/
   IPC_SAT_PROVIDE_IMEI,           /*0x01:IMEI*/
   IPC_SAT_PROVIDE_NMR,            /*0x02:Network Measurement Results*/
   IPC_SAT_PROVIDE_DATE_TIME,      /*0x03:Date,time and time zone*/
   IPC_SAT_PROVIDE_LANG_SET,       /*0x04:Language setting*/
   IPC_SAT_PROVIDE_TIMING_ADV,     /*0x05:Timing advance*/
   IPC_SAT_PROVIDE_ACC_TECH        /*0x06:Access Technology*/
}ipc_sat_provide_local_info_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   Get Provide local information message
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | RESULT (1) |  INFO_TYPE(1) | INFO_DATA(X) |
-------------------------------------------------------------------------------------
 INFO_DATA[]  field 
 Maximum length : 58 bytes 


 if INFO_TYPE = IPC_SAT_PROVIDE_LOCI (0x00),  INFO_DATA are as follows
 -------------------------------------------------------------------------------------
 | MCC_MNC(3) | LOC_AREA_CODE(2) | CELL_ID(4) | CELL_ID_LEN(1) |
 -------------------------------------------------------------------------------------
 
 if INFO_TYPE = IPC_SAT_PROVIDE_IMEI(0x01), INFO_DATA are as follows
 -------------------------------------------------------------------------------------
 | IMEI(8) |
--------------------------------------------------------------------------------------

 if INFO_TYPE = IPC_SAT_PROVIDE_NMR (0x02),  INFO_DATA are as follows
 -------------------------------------------------------------------------------------
 | NMR(16) | BCC_LIST_LEN(1) | BCC_LIST(41) | 
 -------------------------------------------------------------------------------------

 if INFO_TYPE = IPC_SAT_PROVIDE_DATE_TIME (0x03),  INFO_DATA are as follows
 -------------------------------------------------------------------------------------
 | YEAR(1) | MONTH(1) | DAY(1) | HOUR(1) | MINUTE(1) | SECOND(1) | ZONE(1) |
 -------------------------------------------------------------------------------------

 if INFO_TYPE = IPC_SAT_PROVIDE_LANG_SET (0x04),  INFO_DATA are as follows
 -------------------------------------------------------------------------------------
 | LANG_CODE(2) | 
 -------------------------------------------------------------------------------------

 if INFO_TYPE = IPC_SAT_PROVIDE_TIMING(0x05),  INFO_DATA are as follows
 -------------------------------------------------------------------------------------
 | ME_STATUS(1) | TIMING_ADVANCE(1) |
 -------------------------------------------------------------------------------------

 if INFO_TYPE = IPC_SAT_PROVIDE_ACC_TECH (0x06),  INFO_DATA are as follows
 -------------------------------------------------------------------------------------
  | ACCESS_TECH(1) |
 -------------------------------------------------------------------------------------
 */



/*=================================================================

   SUB_CMD(1) : IPC_SAT_POLLING  0x07   Poll Interval or Polling Off
   
=================================================================*/


/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_NOTI             0x03     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
  Get Poll Interval or Polling Off
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |POLL_REQ_TYPE(1)| PRESENT(1) |UNITS(1)| VALUE(1)|
-------------------------------------------------------------------------------------

  POLL_REQ_TYPE field
     Poll Interval :0x00, Polling Off: 0x01  
  PRESENT field 
     TRUE: present, FALSE :no present
  UNITS field
     MINUTE:0x00, Seconds: 0x01, Tenths of seconds: 0x02
  VALUE field
     Max value is 255*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   Response  Poll Interval or Polling Off
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |POLL_CONF_TYPE(1)|POLLING_RESULT(1)|
-------------------------------------------------------------------------------------
 PRESENT(1) |UNITS(1)| VALUE(1)|
-------------------------------------------------------------------------------------
  POLLING_RESULT field
     Success: TRUE, FAIL: FALSE
*/



/*=================================================================

   SUB_CMD(1) : IPC_SAT_REFRESH  0x08   Refresh
   
=================================================================*/


/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */

//none

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
  Get Refresh message
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |REFRESH_MODE(1)| NUM_OF_FILE(1) | 
-------------------------------------------------------------------------------------
 |FILE_PATH_LEN(1)| FILE_PATH(256)| AID_PRESENT(1)| AID (16)
-------------------------------------------------------------------------------------
*/
typedef enum{
   IPC_SAT_REFRESH_INIT_FFCN,           /*0x00:Initialization and Full File Change Notification*/
   IPC_SAT_REFRESH_FCN,                 /*0x01:File Change Notification*/
   IPC_SAT_REFRESH_INIT_FCN,            /*0x02:Initialization and File Change Notification*/
   IPC_SAT_REFRESH_INIT,                /*0x03:Initialization*/
   IPC_SAT_REFRESH_UICC_RESET,          /*0x04:UICC Reset*/
   IPC_SAT_REFRESH_3G_APP_RESET,        /*0x05:3G Application Reset*/
   IPC_SAT_REFRESH_3G_SESSION_RESET     /*0x06:3G Session Reset*/
}ipc_sat_refresh_mode_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   Response Refresh message
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |STATUS(1)|
-------------------------------------------------------------------------------------
*/
typedef enum{
   IPC_SAT_REFRESH_OK,                        /*0x00:Refresh OK*/
   IPC_SAT_REFRESH_BUSY_ON_CALL,              /*0x01:Refresh Busy On Call*/
   IPC_SAT_REFRESH_AID_NOT_ACTIVE,            /*0x02:Refresh Aid Not Active*/
   IPC_SAT_REFRESH_NOT_OK,                    /*0x03:Refresh Not Ok*/
   IPC_SAT_REFRESH_PATH_DECODE_ERR            /*0x04:Refresh Path Decode Error*/
}ipc_sat_refresh_status_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
    Notification Refresh Finished.
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | ACTION_TYPE(1)
-------------------------------------------------------------------------------------
*/
typedef enum{
   IPC_SAT_REFRESH_ACTION_START = 0x01,                     /*0x01:Refresh Start*/
   IPC_SAT_REFRESH_ACTION_END,                       /*0x02:Refresh End*/
   IPC_SAT_REFRESH_ACTION_FAIL		/*0x03:Refresh Fail*/
}ipc_sat_refresh_action_e_type;


/*=================================================================

   SUB_CMD(1) : IPC_SAT_SETUP_EVENT_LIST  0x09   Set Up Event List
   
=================================================================*/


/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_NOTI             0x03     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
  Get Set Up Event List message
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |EVT_LIST_LEN(1)| EVT_LIST(17) |
-------------------------------------------------------------------------------------
*/
typedef enum{
   IPC_SAT_EVT_MT_CALL,                   /*0x00:MT Call*/
   IPC_SAT_EVT_CALL_CONNECT,              /*0x01:Call Connected*/
   IPC_SAT_EVT_CALL_DISCONNECT,           /*0x02:Call Disconnected*/
   IPC_SAT_EVT_LOCATION_STATUS,           /*0x03:Location Status*/
   IPC_SAT_EVT_USER_ACTIVITY,             /*0x04:User Activity*/
   IPC_SAT_EVT_IDLE_SCREEN_AVAIL,         /*0x05:Idle Screen Available*/
   IPC_SAT_EVT_CARD_READER_STATUS,        /*0x06:Card Reader Status*/
   IPC_SAT_EVT_LANG_SELECTION,            /*0x07:Language Selection*/
   IPC_SAT_EVT_BROWSER_TERM,              /*0x08:Browser Termination*/
   IPC_SAT_EVT_DATA_AVAIL,                /*0x09:Data Available*/
   IPC_SAT_EVT_CHANNEL_STATUS,            /*0x0A:Channel status*/
   IPC_SAT_EVT_ACCESS_TECH_CHANGE,        /*0x0B:Access Technology Change*/
   IPC_SAT_EVT_DISP_PARAM_CHANGE,         /* 0x0C:Display Parameters Changed*/
   IPC_SAT_EVT_LOCAL_CONNECTION,          /*0x0D:Local Connection*/
   IPC_SAT_EVT_NET_SEARCH_CHANGE,         /*0x0E:Network Search Mode Change*/
   IPC_SAT_EVT_BROWSING_STATUS,           /*0x0F:Browsing Status*/
   IPC_SAT_EVT_FRAMES_INFO_CHANGE         /*0x10: Frames Information Change*/
}ipc_sat_event_list_e_type;



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   Response Set Up Event List message
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |STATUS(1)|
-------------------------------------------------------------------------------------
    STATUS field
     Success: TRUE, FAIL: FALSE
*/


/*=================================================================

   SUB_CMD(1) : IPC_SAT_CALL_CONTROL_RESULT  0x0A   Call Control Result Message
   
=================================================================*/


/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_RESP             0x02     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                        0x03

 DESCRIPTION :
  Notification Call Control Result Message
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |CALL_TYPE(1)|CONTROL_RESULT(1)|

 ALPHA_ID_PRESENT(1) |ALPHA_ID_LEN(1)|ALPHA_ID(64) |CALL_ID(1)|OLD_CALL_TYPE(1)|

 SUB_PARAMETERS (X)|
 -------------------------------------------------------------------------------------

if CALL_TYPE = Voice Call or Supplementary Service(SS), SUB_PARAMETERS are as follows
-------------------------------------------------------------------------------------
 | MODADD_TON(1) | MODADD_NPI(1) | MODADD_LEN (1) | MODADD(X) | SUBADD_LEN(1) | SUBADD (X) |

 | BC_REPEAT_IND(1) | CCP1_LEN(1) | CCP1 (X) | CCP2_LEN(1) | CCP2(X) 
-------------------------------------------------------------------------------------

if CALL_TYPE = Unstructured Supplementary Service Data(USSD), SUB_PARAMETERS are as follows
-------------------------------------------------------------------------------------
 | MODUSSD_LEN(1) | MODUSSD_DCS(1) | MODUSSD(X) | SUBADD_LEN(1) | SUBADD (X) |

 | BC_REPEAT_IND(1) | CCP1_LEN(1) | CCP1 (X) | CCP2_LEN(1) | CCP2(X) 
-------------------------------------------------------------------------------------

if CALL_TYPE = SMS MO, SUB_PARAMETERS are as follows
-------------------------------------------------------------------------------------
 | RP_ADDR_TON(1) | RP_ADDR_NPI(1) | RP_ADDR_LEN(1) | RP_ADDR(X) |

 | TP_ADDR_TON(1) | TP_ADDR_NPI(1) | TP_ADDR_LEN(1) | TP_ADDR(X) |
-------------------------------------------------------------------------------------

if CALL_TYPE = PDP Context Activation(3G), SUB_PARAMETERS are as follows
-------------------------------------------------------------------------------------
 | PDP_ACT_LEN(1) |PDP_ACT(X) | SUBADD_LEN(1) | SUBADD (X) |

 | BC_REPEAT_IND(1) | CCP1_LEN(1) | CCP1 (X) | CCP2_LEN(1) | CCP2(X) 
-------------------------------------------------------------------------------------
*/
//Max ADDRESS length 200

typedef enum{
   IPC_SAT_CALL_CONTROL_VOICE_CALL,              /*0x00:Voice Call*/
   IPC_SAT_CALL_CONTROL_SMS_MO,                 /* 0x01 : SMS MO*/
   IPC_SAT_CALL_CONTROL_SS,                     /* 0x02 : SS*/
   IPC_SAT_CALL_CONTROL_USSD,                    /* 0x03 : USSD*/
   IPC_SAT_CALL_CONTROL_PDP_ACT                  /* 0x04 : PDP Activate*/
} ipc_sat_call_control_call_e_type;

typedef enum{
   IPC_SAT_CALL_CONTROL_NO,                       /*0x00:No Call Control*/
   IPC_SAT_CALL_CONTROL_ALLOW_WITH_NOMODIF,       /*0x01:Call Control Result is Allowed with No Modificationl*/
   IPC_SAT_CALL_CONTROL_NOT_ALLOWED,              /*0x02:Call Control Result is Not Allowed*/
   IPC_SAT_CALL_CONTROL_ALLOW_BUT_MODIF           /*0x03:Call Control Result is Allowed But Modification*/
} ipc_sat_call_control_result_e_type;

/*=================================================================

   SUB_CMD(1) : IPC_SAT_IMAGE_CLUT  0x0B   Icon, Image, CLUT Message
   
=================================================================*/


/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_NOTI                     0x03     */

//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
  Get Icon, Image, CLUT Message
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |IMAGE_TYPE(1)|
    FILE_PATH(2) |DATA_LENGTH(2)| DATA_OFFSET(2) |
-------------------------------------------------------------------------------------
*/
typedef enum{
   IPC_SAT_IMAGE_TYPE_ICON,                   	/*0x00:Icon*/
   IPC_SAT_IMAGE_TYPE_IMAGE,              		/*0x01:Image*/
   IPC_SAT_IMAGE_TYPE_CLUT           			/*0x02:CLUT*/
}ipc_sat_image_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   Response Icon, Image, CLUT Message
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |IMAGE_TYPE(1)|
    IMAGE_STATUS(1) |IMAGE_LENGTH(1)| IMAGE_DATA(X) |
-------------------------------------------------------------------------------------
*/

typedef enum{
   IPC_SAT_IMAGE_OK,                   			/*0x00:OK*/
   IPC_SAT_IMAGE_INVALID_FILE,      			/*0x01:Invalid File*/
   IPC_SAT_IMAGE_NOT_OK,         				/*0x02:Not OK*/
   IPC_SAT_IMAGE_DATA_NOT_AVAILABLE	/*0x02:Not Avilable*/
}ipc_sat_image_result_e_type;


/*=================================================================

   SUB_CMD(1) : IPC_SAT_SETUP_CALL_PROCESSING 0x0C   Setup Call Processing Message
   
=================================================================*/


/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_RESP             0x02     */
/*    IPC_CMD_NOTI             0x03     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                        0x03

 DESCRIPTION :
  Notify setup call processing status to BP
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |SETUPCALL_STATUS(1)|
-------------------------------------------------------------------------------------
*/
/* NOTI_RESULTCODE	*/
typedef enum{
   IPC_SAT_PROCESSING_SETUPCALL,
   IPC_SAT_SENT_TR_FOR_SETUPCALL
}ipc_sat_setupcall_status_e_type;



/*=================================================================

   SUB_CMD(1) : IPC_SAT_SIM_INITIATE_MESSAEG    0x0C   SIM initiate Message
   
=================================================================*/


/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_RESP             0x02     */
/*    IPC_CMD_NOTI             0x03     */
//none


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_EVENT                  0x05

 DESCRIPTION :
  Event SIM initiate Message.
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/









/*********************************************************************************

              Sub Command of IPC_CFG_CMD[0x0F]

**********************************************************************************/
typedef enum{
  IPC_CFG_DEFAULT_CONFIG=0x01,      /* 0x01 : Default setting Message */
  IPC_CFG_EXTERNAL_DEVICE,              /* 0x02 : External device Message */
  IPC_CFG_MAC_ADDRESS,   			/* 0x03 : MAC address read/write Message  */
  IPC_CFG_CONFIGURATION_ITEM, 	/* 0x04 : Configuration item read/write Message */
  IPC_CFG_TTY,			                     /* 0x05 : TTY enable/disable Message */
  IPC_CFG_HSDPA_TMP_SETTING,		/* 0x06 : HSDPA tmp setting */
  IPC_CFG_HSDPA_PERM_SETTING,       /* 0x07 : HSDPA permanent setting */
  IPC_CFG_SIO_MODE,                           /* 0x08 : Setting SIO mode of modem message */
  IPC_CFG_AKEY_VERIFY,                      /* 0x09 : A-Key Get/Verify Message */
  IPC_CFG_MSL_INFO,                            /* 0x0A : MSL Information Message */
  IPC_CFG_USER_LOCK_CODE,               /* 0x0B : User Lock Code Message */ 
  IPC_CFG_USB_PATH ,                          /* 0x0C : USB Path Message */ 
  IPC_CFG_CURRENT_SVC_CARRIER,      /* 0x0D : Current Carrier Message */
  IPC_CFG_RADIO_CONFIG,                   /* 0x0E : Radio Configuration Message */ 
  IPC_CFG_VOCODER_OPTION,               /* 0x0F : Vocoder Option Message */ 
  IPC_CFG_TEST_SYS,                            /* 0x10 : Test System value Message */ 
  IPC_CFG_RECONDITIONED_DATE,       /* 0x11 : Reconditioned Status/Date Message */ 
  IPC_CFG_PROTOCOL_REVISION,          /* 0x12 : MS Protocol Revision Message */ 
  IPC_CFG_SLOT_MODE,                         /* 0x13 : Slotted Mode Message */ 
  IPC_CFG_ACTIVATION_DATE,             /* 0x14 : Activation Date Message */ 
  IPC_CFG_CURRENT_UATI,                   /* 0x15 : Current UATI Message */ 
  IPC_CFG_QUICK_PAGING,                   /* 0x16 : Quick Paging Message */ 
  IPC_CFG_LMSC_INFO,                         /* 0x17 : LMSC Information Message */
  IPC_CFG_TAS_INFO,                           /* 0x18 : TAS Information Message */
  IPC_CFG_AUTH_INFO ,                        /* 0x19 : AUTH INFORMATION Message */ 
  IPC_CFG_HIDDEN_MENU_ACCESS,     /* 0x1A : Hidden menu access Message */ 
  IPC_CFG_UTS_SMS_SEND,           /* 0x1B : Sending SMS using UTS dll */
  IPC_CFG_UTS_SMS_COUNT,          /* 0x1C : Getting count of messages using UTS dll */
  IPC_CFG_UTS_SMS_MSG,            /* 0x1D : Getting message content using UTS dll */
  IPC_CFG_SCM_INFO,                 /* 0x1E : SCM Message */
  IPC_CFG_SCI_INFO,                 /* 0x1F : Slot cycle index Message */
  IPC_CFG_ACCOLC_INFO,              /* 0x20 : Accolc Mess */
  IPC_CFG_MOBTERM_INFO,             /* 0x21 : mobile term Message */
  IPC_CFG_1X_EVDO_DIVERSITY_CONFIG,     /*   0x22 : 1x EVDO Diversity Test Configuration */
  IPC_CFG_DEVICE_CONFIGURATION,         /*0x23 : Device Configuration Message*/
  IPC_CFG_USER_LOCK_CODE_STATUS,    /*0x24 : PDA user lock code status Message*/
  IPC_CFG_UTS_SMS_GET_UNREAD_MSG_STATUS, /*0x25 : UTS Get if any Unread SMS messages*/
  IPC_CFG_MAX                                     /* MAX :  */
} ipc_cfg_sub_cmd_type;

/*********************************************************************************/

/*=================================================================

   SUB_CMD(1) : IPC_CFG_DEFAULT_CONFIG      0x01 Default setting Message

=================================================================*/

/*    IPC_CMD_GET               0x02     */
/*    IPC_CMD_SET              0x03     */
/*    IPC_CMD_EVENT             0x05     */
/*    IPC_CMD_RESP             0x02     */
/* none */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_EXEC                     0x01

 DESCRIPTION :
   Execute Default configuration setting
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | DEFAULT_MODE(1) |
-------------------------------------------------------------------------------------
*/

/* DEFAULT_MODE field */
typedef enum {
      IPC_CFG_DEFAULT_FULL      = 0x01,         /* 0x01 : Full reset */
      IPC_CFG_DEFAULT_FACTORY,                   /* 0x02 : Factory reset */
      IPC_CFG_DEFAULT_SERVICE,                  /* 0x03 : Service reset */
      IPC_CFG_DEFAULT_CUSTOM,                   /* 0x04 : Custom reset */  
      IPC_CFG_DEFAULT_SELECTIVE,                   /* 0x05 : Selective reset */
      IPC_CFG_DEFAULT_MODE_MAX
} ipc_cfg_default_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_CFRM                     0x04

 DESCRIPTION :
   confirm Default configuration setting
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | DEFAULT_MODE(1) |
-------------------------------------------------------------------------------------
*/
/* DEFAULT_MODE */
/* see ipc_cfg_default_e_type */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_INDI                     0x01

 DESCRIPTION :
   Indication Default configuration setting
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | DEFAULT_MODE(1) |
-------------------------------------------------------------------------------------
*/

/* DEFAULT_MODE */
/* see ipc_cfg_default_e_type */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
   Notifiy that the Default configuration setting is done.
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | DEFAULT_MODE(1) |
-------------------------------------------------------------------------------------
*/

/* DEFAULT_MODE */
/* see ipc_cfg_default_e_type */



/*=================================================================*/





/*=================================================================

   SUB_CMD(1) : IPC_CFG_EXTERNAL_DEVICE          0x02 : External device Message

=================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_SET               0x03     */
/*	  IPC_CMD_CFRM              0x04	 */
/*	  IPC_CMD_INDI              0x01	 */
/* none */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
   Get External device status
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | DEVICE_ID(1) |
-------------------------------------------------------------------------------------
*/

/* DEVICE_ID field */
typedef enum {
	IPC_CFG_DEVICE_EARPHONE 	= 0x01,	    /* 0x01 : Earphone or Headset */
	IPC_CFG_DEVICE_HFK,           			/* 0x02 : Hands Free Kit 	*/
	IPC_CFG_DEVICE_CLC,             		/* 0x03 : Cigar Lay Charger */  
	IPC_CFG_DEVICE_TC,           			/* 0x04 : Travel Charger 	*/
	IPC_CFG_DEVICE_DTC,           			/* 0x05 : DeskTopCharger or Holder 	*/	  
	IPC_CFG_DEVICE_KEYBOARD,           		/* 0x06 : External keyboard */	  	  
	IPC_CFG_DEVICE_BT,           	      /* 0x07 : Bluetooth */	  	  	  
	IPC_CFG_DEVICE_BT_HEADSET,  /* 0x08 : Bluetooth Headset */	  	  	  
	IPC_CFG_DEVICE_TTY,                     /* 0x09 : TTY */	
	IPC_CFG_DEVICE_MAX
} ipc_cfg_device_id_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_EVENT                     0x05

 DESCRIPTION :
	Event External device status 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | DEVICE_ID(1) |DEVICE_STATUS(1) |
-------------------------------------------------------------------------------------
*/

/* DEVICE_ID field */
/* see ipc_cfg_device_id_e_type */

/* DEVICE_STATUS field */
/* if DEVICE_ID = (CLC,TC,DTC,KEYBOARD,BLUETOOTH, BT_HEADSET, TTY)  */
typedef enum {
	IPC_CFG_DEVICE_DISCONNECTED 	= 0x00,	     /* 0x00 : Device disconnected */
	IPC_CFG_DEVICE_CONNECTED,           		 /* 0x01 : Device connected */
} ipc_cfg_device_status_e_type;

/* if DEVICE_ID = EARPHONE */
typedef enum {
	IPC_CFG_EARHPHONE_DISCONNECTED	= IPC_CFG_DEVICE_DISCONNECTED,
	IPC_CFG_EARHPHONE_CONNECTED,
	IPC_CFG_EARHPHONE_REMOTEKEY_OFF,	  		 /* 0x02 : Earphone remotekey off */
	IPC_CFG_EARHPHONE_REMOTEKEY_ON,           	 /* 0x03 : Earphone remotekey on  */
	IPC_CFG_EARPHONE_STATUS_MAX
} ipc_cfg_earphone_status_e_type;

/* if DEVICE_ID = HFK */
typedef enum {
	IPC_CFG_HFK_DISCONNECTED	 = IPC_CFG_DEVICE_DISCONNECTED,
	IPC_CFG_HFK_CONNECTED,
	IPC_CFG_HFK_IGNITION_OFF,	     			/* 0x02 : HFK ignition off */
	IPC_CFG_HFK_STATUS_MAX
} ipc_cfg_hfk_status_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
   Notification External device status
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | DEVICE_ID(1) |DEVICE_STATUS(1) |
-------------------------------------------------------------------------------------
*/

/* DEVICE_ID field */
/* see ipc_cfg_device_id_e_type */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
   Notification External device status
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  DEVICE_ID(1) |DEVICE_STATUS(1) |
-------------------------------------------------------------------------------------
*/

/* DEVICE_ID field */
/* see ipc_cfg_device_id_e_type */



/*=================================================================*/




/*=================================================================

   SUB_CMD(1) : IPC_CFG_MAC_ADDRESS          0x03 : MAC Address Message

=================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*	  IPC_CMD_CFRM              0x04	 */
/*    IPC_CMD_EVENT             0x05     */
/*	  IPC_CMD_INDI              0x01	 */
/*	  IPC_CMD_NOTI              0x03	 */
/* none */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
   Get MAC address
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | INDEX(1) |
-------------------------------------------------------------------------------------
*/
/* INDEX field */
typedef enum {
	  IPC_CFG_WIFI_MAC_ADDRESS 		= 0x01,	    	/* 0x01 : WiFi MAC address */
	  IPC_CFG_BLUETOOTH_BD_ADDRESS,           		/* 0x02 : Bluetooth bd address 	*/
	  IPC_CFG_MAC_ADDRESS_INDEX_MAX
} ipc_cfg_mac_address_index_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
	Set MAC address
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | INDEX(1) | MAC_ADDR_LEN(1) | MAC_ADDR(...)
-------------------------------------------------------------------------------------
*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_CFRM                     0x04

 DESCRIPTION :
	Set MAC address
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | INDEX(1) | MAC_ADDR_LEN(1) | MAC_ADDR(...)
-------------------------------------------------------------------------------------
*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_INDI                     0x01

 DESCRIPTION :
	Reponse MAC address
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  INDEX(1)
-------------------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
	Reponse MAC address
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  INDEX(1) | MAC_ADDR_LEN(1) | MAC_ADDR(...)
-------------------------------------------------------------------------------------
*/
/*=================================================================*/




/*=================================================================

   SUB_CMD(1) : IPC_CFG_CONFIGURATION_ITEM         0x04 : Configuration item read/write Message

=================================================================*/
/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET               0x02     */
/*    IPC_CMD_SET               0x03     */
/*    IPC_CMD_EVENT             0x05     */
/*	  IPC_CMD_RESP              0x02	 */
/*	  IPC_CMD_NOTI              0x03	 */
/* none */

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_CFRM                     0x04

 DESCRIPTION :
   Confirm configuration item
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MODE(1) | ITEM_ID(1) |RESULT(1) | DATA(...)
-------------------------------------------------------------------------------------
*/
typedef enum{
  IPC_CFG_CONFIG_ITEM_GET=0x01,		   	/* 0x01 : Get */
  IPC_CFG_CONFIG_ITEM_SET,  			/* 0x02 : Set */
  IPC_CFG_CONFIG_ITEM_MODE_MAX
} ipc_cfg_config_item_mode_e_type;

typedef enum{
  IPC_CFG_CONFIG_ITEM_FAIL		=0x00,		/* 0x00 : Fail */
  IPC_CFG_CONFIG_ITEM_SUCCESS,  			/* 0x01 : Success */
  IPC_CFG_CONFIG_ITEM_RESULT_MAX
} ipc_cfg_config_item_result_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_INDI                     0x01

 DESCRIPTION :
   Indication configuration item
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MODE(1) | ITEM_ID(1) | DATA(...)
-------------------------------------------------------------------------------------
*/
/* DATA  field */
/* Maximum 128 character, terminated by null */
/* see MAX_GSM_PSTITEM_LEN */



/*=================================================================

   SUB_CMD(1) :  IPC_CFG_TTY           0x05 : TTY status Message

=================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*	  IPC_CMD_CFRM              0x04	 */
/*    IPC_CMD_EVENT             0x05     */
/*	  IPC_CMD_INDI              0x01	 */
/*	  IPC_CMD_NOTI              0x03	 */
/* none */



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
   Get TTY status
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
   Set TTY status
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | STATUS(1)
-------------------------------------------------------------------------------------
*/

/* STATUS field */
typedef enum {
  IPC_CFG_TTY_DISABLE             = 0x00,         /* 0x00 : disable TTY status */
  IPC_CFG_TTY_ENABLE_TALK     = 0x01,         /* 0x01 : enable TTY status(TALK) */
  IPC_CFG_TTY_ENABLE_HEAR     = 0x02,         /* 0x02 : enable TTY status(HEAR) */
  IPC_CFG_TTY_ENABLE_FULL     = 0x03,         /* 0x03 : enable TTY status(FULL) */
  IPC_CFG_TTY_MAX
} ipc_cfg_tty_e_type;

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
	Reponse TTY status
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  STATUS(1)
-------------------------------------------------------------------------------------
*/
/* STATUS field */
/* See ipc_cfg_tty_e_type */

/*=================================================================*/



/*=================================================================

   SUB_CMD(1) :  IPC_CFG_HSDPA_TMP_SETTING           0x06 : Configure HSDPA temporary setting

=================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET              0x02     */
/*	  IPC_CMD_CFRM              0x04	 */
/*    IPC_CMD_EVENT             0x05     */
/*	  IPC_CMD_INDI              0x01	 */
/*    IPC_CMD_RESP             0x02     */
/* none */




/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
   Set HSDPA temporary configuration
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |Number of (STATUS+MCC) | STATUS(1) | MCC(3) |
-------------------------------------------------------------------------------------
*/

/* STATUS field */
typedef enum {
  IPC_CFG_HSDPA_TMP_DISABLE      = 0x00,         /* 0x00 : disable HSDPA temporary setting */
  IPC_CFG_HSDPA_TMP_ENABLE,                      /* 0x02 : enable HSDPA temporary setting */
  IPC_CFG_HSDPA_TMP_MAX
} ipc_cfg_hsdpa_tmp_setting_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
	Notification HSDPA configuration status change
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | STATUS(1) | MCC(3) |
-------------------------------------------------------------------------------------
*/
/* STATUS field */


/*=================================================================*/




/*=================================================================

   SUB_CMD(1) :  IPC_CFG_HSDPA_PERM_SETTING           0x07 : Configure HSDPA permanent setting

=================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*	  IPC_CMD_CFRM              0x04	 */
/*    IPC_CMD_EVENT             0x05     */
/*	  IPC_CMD_INDI              0x01	 */
/*    IPC_CMD_NOTI              0x03     */
/* none */


/*-----------------------------------------------------------------------------------
CMD_TYPE(1) :  IPC_CMD_GET                     0x02 
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------

*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
   Set HSDPA enable/disable configuration
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | STATUS(1) |
-------------------------------------------------------------------------------------
*/

/* STATUS field */
typedef enum {
  IPC_CFG_HSDPA_SETTING_DISABLE      = 0x00,         /* 0x00 : disable HSDPA option */
  IPC_CFG_HSDPA_SETTING_ENABLE,                      /* 0x01 : enable HSDPA option */
  IPC_CFG_HSDPA_SETTING_MAX
} ipc_cfg_hsdpa_setting_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
	Notification HSDPA configuration status change
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | STATUS(1) |
-------------------------------------------------------------------------------------
*/
/* STATUS field */
/*
See ipc_cfg_hsdpa_setting_e_type
*/



/*=================================================================================================================

   SUB_CMD(1) : IPC_CFG_SIO_MODE     0x08                  Setting SIO mode of modem message
   
==================================================================================================================*/

/* INTERFACE_MODE */
typedef enum{
  IPC_CFG_SIO_U1_DIAG_USB2_GPS = 0x00,
  IPC_CFG_SIO_U1_HFK_USB2_GPS = 0x01,
  IPC_CFG_SIO_U1_HFK_USB2_DIAG = 0x02,
  IPC_CFG_SIO_INTERFACE_MODE_MAX
}ipc_cfg_sio_interface_mode_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_GET                     0x02     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) 
-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_SET                     0x03     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | INTERFACE_MODE(1)
-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_RESP                     0x02     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | INTERFACE_MODE(1)
-----------------------------------------------------------------------------------*/


/*=================================================================================

   SUB_CMD : IPC_CFG_AKEY_VERIFY		0x09		A-Key Get/Verify Message
   
==================================================================================*/
typedef enum{
	IPC_CFG_AKEY_CMD_INDEX_GET 			= 0x00,
	IPC_CFG_AKEY_CMD_INDEX_VERIFY 	       = 0x01,
	IPC_CFG_AKEY_CMD_INDEX_MAX,
}ipc_cfg_akey_cmd_index_e_type;

typedef enum{
	IPC_CFG_AKEY_VERIFY_FAIL 			= 0x00,
	IPC_CFG_AKEY_VERIFY_OK,
	IPC_CFG_AKEY_VERIFY_MAX,
}ipc_cfg_akey_verify_e_type;

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_GET                     0x02     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CMD_INDEX(1) | A_KEY(26)
-----------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_RESP                     0x02     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CMD_INDEX(1) | A_KEY(26) | RESULT(1)
-----------------------------------------------------------------------------------*/


/*=================================================================================

   SUB_CMD : IPC_CFG_MSL_INFO		0x0A		MSL Information Message
   
==================================================================================*/

typedef enum{
	IPC_CDMA_INFO_OTKSL_FLAG_OFF	= 0x00,
	IPC_CDMA_INFO_OTKSL_FLAG_ON,
	IPC_CDMA_INFO_OTKSL_FLAG_MAX
}ipc_cdma_info_otksl_flag_e_type;

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_GET                     0x02     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) 
-----------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_RESP                     0x02     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MSL(6) | OTKSL(6) | OTKSL_FLAG(1)
-----------------------------------------------------------------------------------*/


/*==================================================================================================

   SUB_CMD : IPC_CFG_USER_LOCK_CODE      0x0B         User Lock Code Message
   
===================================================================================================*/

/* User Lock Code field*/
typedef enum {
  IPC_CFG_USER_LOCK_CODE_DISABLE		= 0x00,	/* 0x00 : Lock Code Disable */
  IPC_CFG_USER_LOCK_CODE_NUMBER_TYPE		= 0x01,	/* 0x01 : Lock Code Number Type */
  IPC_CFG_USER_LOCK_CODE_CHARS_TYPE		= 0x02,	/* 0x02 : Lock Code Chars Type */
  IPC_CFG_USER_LOCK_CODE_NOT_USE_ENABLE	= 0x03,	/* 0x03 : Lock Code Enable but not use */
  IPC_CFG_USER_LOCK_CODE_MAX				/* MAX */
} ipc_cfg_setting_user_lock_code_e_type;

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_GET                     0x02     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1)
 -----------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_SET                     0x03     

 DESCRIPTION :
   
 FORMAT :
-----------------------------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | LOCK_CODE_TYPE(1) | ACTIVE_PERIOD_TIME(2) | DIGIT_NUM(1) | DIGITS(40)
----------------------------------------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_RESP                     0x02     

 DESCRIPTION :
   
 FORMAT :
------------------------------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | LOCK_CODE_TYPE(1) | ACTIVE_PERIOD_TIME(2) | DIGIT_NUM(1) | DIGITS(40)
-----------------------------------------------------------------------------------------------------------------*/



/*==================================================================================================

   SUB_CMD : IPC_CFG_USB_PATH      0x0C         USB Path Message
   
===================================================================================================*/

/* USB Path field*/
typedef enum {
  IPC_CFG_USB_PATH_TO_PDA		= 0x00,	/* 0x00 : PDA uses USB */
  IPC_CFG_USB_PATH_TO_PHONE		= 0x01,	/* 0x01 : Phone uses USB */
  IPC_CFG_USB_PATH_MAX				/* MAX */
} ipc_cfg_setting_usb_path_e_type;

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_GET                     0x02     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1)
 -----------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_SET                     0x03     

 DESCRIPTION :
   
 FORMAT :
--------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | USB_PATH(1)
-------------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_RESP                     0x02     

 DESCRIPTION :
   
 FORMAT :
---------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | USB_PATH(1)
--------------------------------------------------------------------------------------*/


/*=================================================================================

   SUB_CMD : IPC_CFG_CURRENT_SVC_CARRIER		0x0D		Current Carrier Message
   
==================================================================================*/
typedef enum{
	IPC_CFG_CARRIER_TEST			= 0x00,
	IPC_CFG_CARRIER_SKT,
	IPC_CFG_CARRIER_KTF,
	IPC_CFG_CARRIER_LGT,
	IPC_CFG_CARRIER_VERIZON,
	IPC_CFG_CARRIER_SPRINT,
	IPC_CFG_CARRIER_ALLTEL,
	IPC_CFG_CARRIER_TELUS,
	IPC_CFG_CARRIER_BMC,
	IPC_CFG_CARRIER_ALIANT,
	IPC_CFG_CARRIER_SASKTEL,
	IPC_CFG_CARRIER_MTS,
	IPC_CFG_CARRIER_MAX
}ipc_cfg_carrier_e_type;

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_GET                     0x02     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) 
-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_SET                     0x03     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CARRIER(1)
-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_RESP                     0x02     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CARRIER(1)
-----------------------------------------------------------------------------------*/


/*=================================================================================================================

   SUB_CMD(1) : IPC_CFG_RADIO_CONFIG     0x0E            Radio Configuration Message
   
==================================================================================================================*/

typedef enum{
 IPC_CFG_RC_MODE_DIS_TEST_RC=0x00,
 IPC_CFG_RC_MODE_FRC1_RRC1,
 IPC_CFG_RC_MODE_FRC2_RRC2,
 IPC_CFG_RC_MODE_FRC3_RRC3,
 IPC_CFG_RC_MODE_FRC4_RRC3,
 IPC_CFG_RC_MODE_FRC5_RRC4,
 IPC_CFG_RC_MODE_F_MAX
}ipc_cfg_rc_mode_e_type;

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_GET                     0x02     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) 
-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_SET                     0x03     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | RC_MODE(1)
-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_RESP                     0x02     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | RC_MODE(1)
-----------------------------------------------------------------------------------*/



/*=================================================================================================================

   SUB_CMD(1) : IPC_CFG_VOCODER_OPTION     0x0F           Vocoder Option Message
   
==================================================================================================================*/
typedef enum{
  IPC_CFG_VOC_OPT_DEFAULT,
  IPC_CFG_VOC_OPT_EVRC,
  IPC_CFG_VOC_OPT_13K,
  IPC_CFG_VOC_OPT_8K,
  IPC_CFG_VOC_OPT_MAX
}ipc_cfg_vocoder_option_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_GET                     0x02     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) 
-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_SET                     0x03     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | VOCODER_OPTION_HOME(1) | VOCODER_OPTION_ROAM(1)
-----------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_RESP                     0x02     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | VOCODER_OPTION_HOME(1) | VOCODER_OPTION_ROAM(1)
-----------------------------------------------------------------------------------------*/



/*=================================================================================================================

   SUB_CMD(1) : IPC_CFG_TEST_SYS     0x10            Test System value Message
   
==================================================================================================================*/

/* TEST_SYS_VALUE */

//0~65534 

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_GET                     0x02     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) 
-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_RESP                     0x02     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | TEST_SYS_VALUE(2)
-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_SET                     0x03     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | TEST_SYS_VALUE(2) 
-----------------------------------------------------------------------------------*/



/*=================================================================================

   SUB_CMD : IPC_CFG_RECONDITIONED_DATE		0x11		Reconditioned Status/Date Message
   
==================================================================================*/
typedef enum{
	IPC_CFG_RECON_STATUS_NO 		= 0x00,
	IPC_CFG_RECON_STATUS_YES,
	IPC_CFG_RECON_STATUS_MAX
}ipc_cfg_recon_status_e_type;

//- YEAR : 0 ~ XXXX   - MONTH  : 1 ~ 12  - DAY : 1 ~ 31
//- HOUR : 0~23     - MINUTE : 0~59    - SECOND : 0~59

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_GET                     0x02     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) 
-----------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_RESP                     0x02     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | STATUS(1) || YEAR(2) | MONTH(1) | DAY(1) | HOUR(1) | MIN(1) | SEC(1) 
-----------------------------------------------------------------------------------*/


/*==================================================================================================

   SUB_CMD : IPC_CFG_PROTOCOL_REVISION      0x12         MS Protocol Revision Message
   
===================================================================================================*/

/* MS Protocol Revision field*/
typedef enum {
  IPC_CFG_PROTOCOL_REVISION_NONE	= 0x00,	/* 0x00 : None (No Service or AMPS */
  IPC_CFG_PROTOCOL_REVISION_JSTD008,		/* 0x01 : TSB74(J-STD-T008  */
  IPC_CFG_PROTOCOL_REVISION_IS95A_2,		/* 0x02 : IS95A_2 */
  IPC_CFG_PROTOCOL_REVISION_IS95A_3,		/* 0x03 : IS95A_3 */
  IPC_CFG_PROTOCOL_REVISION_IS95B_4,		/* 0x04 : IS95B_4 */
  IPC_CFG_PROTOCOL_REVISION_IS95B_5,		/* 0x05 : IS95B_5 */
  IPC_CFG_PROTOCOL_REVISION_IS2000_0,	/* 0x06 : IS2000 Rel 0 */
  IPC_CFG_PROTOCOL_REVISION_IS2000_A,	/* 0x06 : IS2000 Rel A */
  IPC_CFG_PROTOCOL_REVISION_MAX			/* Max */
} ipc_cfg_ms_protocol_revision_e_type;

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_GET                     0x02     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1)
 -----------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_SET                     0x03     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | PROTOCOL_REVISION(1)
-----------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_RESP                     0x02     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | PROTOCOL_REVISION(1)
-----------------------------------------------------------------------------------*/


/*==================================================================================================

   SUB_CMD : IPC_CFG_SLOT_MODE      0x13         Slotted Mode Message
   
===================================================================================================*/

/* Slotted Mode field*/
typedef enum {
  IPC_CFG_SLOT_MODE_OFF	       = 0x00,	/* 0x00 : Slot Mode Off*/
  IPC_CFG_SLOT_MODE_ON		= 0x01	/* 0x01 : Slot Mode On */
} ipc_cfg_slot_mode_e_type;

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_GET                     0x02     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1)
 -----------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_SET                     0x03     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SLOT_MODE(1)
-----------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_RESP                     0x02     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SLOT_MODE(1)
-----------------------------------------------------------------------------------*/



/*=================================================================================

   SUB_CMD : IPC_CFG_ACTIVATION_DATE		0x14		Activation Date Message
   
==================================================================================*/
//- YEAR : 0 ~ XXXX   - MONTH  : 1 ~ 12  - DAY : 1 ~ 31
//- HOUR : 0~23     - MINUTE : 0~59    - SECOND : 0~59

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_GET                     0x02     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) 
-----------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_RESP                     0x02     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | YEAR(2) | MONTH(1) | DAY(1) | HOUR(1) | MIN(1) | SEC(1) 
-----------------------------------------------------------------------------------*/



/*=================================================================================

   SUB_CMD : IPC_CFG_CURRENT_UATI		0x15		Current UATI Message
   
==================================================================================*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_GET                     0x02     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) 
-----------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_RESP                     0x02     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | UATI(4)
-----------------------------------------------------------------------------------*/



/*==================================================================================================

   SUB_CMD : IPC_CFG_QUICK_PAGING      0x16         Quick Paging Message
   
===================================================================================================*/

/* Quick Paging field*/
typedef enum {
  IPC_CFG_QUICK_PAGING_OFF  	= 0x00,	/* 0x00 : Quick Paging Off */
  IPC_CFG_QUICK_PAGING_ON   	= 0x01	/* 0x01 : Quick Paging On */
} ipc_cfg_quick_paging_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_GET                     0x02     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1)
 -----------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_SET                     0x03     

 DESCRIPTION :
   
 FORMAT :
--------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | QUICK_PAGING(1)
-------------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_RESP                     0x02     

 DESCRIPTION :
   
 FORMAT :
---------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | QUICK_PAGING(1)
--------------------------------------------------------------------------------------*/


/*==================================================================================================

   SUB_CMD : IPC_CFG_LMSC_INFO      0x17         LMSC Information Message
   
===================================================================================================*/

/* Total LMSC Information field*/
#define  IPC_CFG_LMSC_INFO_IP_ADDRESS            0x01	
#define  IPC_CFG_LMSC_INFO_PORT_NUMBER         0x02	
#define  IPC_CFG_LMSC_INFO_DORMAIN_NAME      0x04	
#define  IPC_CFG_LMSC_INFO_ALL                          0xFF	



/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_GET                     0x02     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1)
 -----------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_SET                     0x03     

 DESCRIPTION :
   
 FORMAT :
--------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | KIND_INFO(1) |IP_ADDRESS(4) | PORT_NUMBER(4) 
 | DOMAIN_LENGTH(1) | DOMAIN(variable)
-------------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_RESP                     0x02     

 DESCRIPTION :
   
 FORMAT :
---------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | KIND_INFO(1) |IP_ADDRESS(4) | PORT_NUMBER(4) 
 | DOMAIN_LENGTH(1) | DOMAIN(variable)
--------------------------------------------------------------------------------------*/


/*==================================================================================================

   SUB_CMD : IPC_CFG_TAS_INFO      0x18         TAS Information Message
   
===================================================================================================*/

/* TAS EVDO field*/
typedef enum {
  IPC_CFG_TAS_EVDO_OFF  	= 0x00,	/* 0x00 : TAS EVDO Off */
  IPC_CFG_TAS_EVDO_ON   	= 0x01	/* 0x01 : TAS EVDO On */
} ipc_cfg_tas_info_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_GET                     0x02     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1)
 -----------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_SET                     0x03     

 DESCRIPTION :
   
 FORMAT :
--------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | TAS_EVDO(1)
-------------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_RESP                     0x02     

 DESCRIPTION :
   
 FORMAT :
---------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | TAS_EVDO(1)
--------------------------------------------------------------------------------------*/


/*==================================================================================================

   SUB_CMD : IPC_CFG_AUTH_INFO      0x19         AUTH INFORMATION Message
   
===================================================================================================*/

/* Auth Information field*/
typedef enum {
  IPC_CFG_AUTH_OFF  = 0x00,	/* 0x00 : Auth Information Off */
  IPC_CFG_AUTH_ON   = 0x01		/* 0x01 : Auth Information On */
} ipc_cfg_auth_info_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_GET                     0x02     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1)
 -----------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_SET                     0x03     

 DESCRIPTION :
   
 FORMAT :
--------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | AUTH_ENABLE(1)
-------------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_RESP                     0x02     

 DESCRIPTION :
   
 FORMAT :
---------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | AUTH_ENABLE(1)
--------------------------------------------------------------------------------------*/




/*=================================================================================

   SUB_CMD : IPC_CFG_HIDDEN_MENU_ACCESS      0x1A          Hidden menu access Message
   
==================================================================================*/

/* ACCESS_TYPE field*/
typedef enum {
  IPC_CFG_HIDDEN_MENU_DISABLE,	       /* 0x00 :   */
  IPC_CFG_HIDDEN_MENU_ENABLE,	       /* 0x01 :   */
  IPC_CFG_HIDDEN_MENU_MAX                    /* Max */
} ipc_cfg_hidden_menu_access_e_type;

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_GET                     0x02     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1)
-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_SET                     0x03     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | ACCESS_TYPE(1)
-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_RES                     0x02     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | ACCESS_TYPE(1)
-----------------------------------------------------------------------------------*/


/*=================================================================================
  
   SUB_CMD : IPC_CFG_UTS_SMS_SEND      0x1B          Sending SMS using UTS dll
     
==================================================================================*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_NOTI                     0x03     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | PARAM (200)
-----------------------------------------------------------------------------------*/



/*=================================================================================
  
   SUB_CMD : IPC_CFG_UTS_SMS_COUNT      0x1C          Getting count of messages using UTS dll
     
==================================================================================*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_CFRM                     0x04     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | COUNT(1)
-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_INDI                     0x01     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1)
-----------------------------------------------------------------------------------*/



/*=================================================================================
  
   SUB_CMD : IPC_CFG_UTS_SMS_MSG      0x1D          Getting message content using UTS dll
     
==================================================================================*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_CFRM                     0x04     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | PARAM(300)
-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_INDI                     0x01     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | INDEX(1)
-----------------------------------------------------------------------------------*/



/*=================================================================================

   SUB_CMD : IPC_CFG_SCM_INFO      0x1E          SCM Message

==================================================================================*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_GET                     0x02     

 DESCRIPTION :

 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1)
-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_SET                     0x03     

 DESCRIPTION :

 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SCM_VALUE(1)
-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_RES                     0x02     

 DESCRIPTION :

 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SCM_VALUE(1)
-----------------------------------------------------------------------------------*/

/*=================================================================================

   SUB_CMD : IPC_CFG_SCI_INFO      0x1F : Slot cycle index Message

==================================================================================*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_GET                     0x02     

 DESCRIPTION :

 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1)
-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_SET                     0x03     

 DESCRIPTION :

 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SCI_VALUE(1)
-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_RES                     0x02     

 DESCRIPTION :

 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SCI_VALUE(1)
-----------------------------------------------------------------------------------*/

/*=================================================================================

   SUB_CMD : IPC_CFG_ACCOLC_INFO      0x20 : Accolc Message 

==================================================================================*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_GET                     0x02     

 DESCRIPTION :

 FORMAT :
------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1)
-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_SET                     0x03     

 DESCRIPTION :

 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | ACCOLC_VALUE(1)
-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_RES                     0x02     

 DESCRIPTION :

 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | ACCOLC_VALUE(1)
-----------------------------------------------------------------------------------*/

/*=================================================================================

  SUB_CMD : IPC_CFG_MOBTERM_INFO      0x21 : mobile term Message

==================================================================================*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_GET                     0x02     

 DESCRIPTION :

 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) 
-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_SET                     0x03     

 DESCRIPTION :

 FORMAT :
------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MOB_HOME(1) | MOB_SID(1) | MOB_NID(1)
-----------------------------------------------------------------------------------*/


/* MOB_XXX field*/
typedef enum {
  IPC_CFG_MOBTERM_DISABLE,          /* 0x00 :   */
  IPC_CFG_MOBTERM_ENABLE,         /* 0x01 :   */
  IPC_CFG_MOBTERM_NOT_CHANGED       /* 0x02 */
} ipc_cfg_mobterm_info_e_type;

 
/*-----------------------------------------------------------------------------------

 CMD_TYPE : IPC_CMD_RES                     0x02     

 DESCRIPTION :

 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MOB_HOME(1) | MOB_SID(1) | MOB_NID(1)
-----------------------------------------------------------------------------------*/

/*=================================================================================

   SUB_CMD : IPC_CFG_1X_EVDO_DIVERSITY_CONFIG      0x22 : 1x EVDO Diversity Test Configuration
   
==================================================================================*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_GET                     0x02     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) 
-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_SET                     0x03     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 1X_MRD(1) | EVDO_MRD(1) | SHDR(1)| DIVERSITY_TEST(1)
-----------------------------------------------------------------------------------*/
/* 1X_MRD field,  EVDO_MRD field, SHDR field, DIVERSITY_TEST field*/

typedef enum {
  IPC_CFG_RF_VALUE_OFF=0x00,      /* 0x00 :   */
  IPC_CFG_RF_VALUE_ON,	       /* 0x01 :   */
  IPC_CFG_RF_VALUE_NOT_CHANGED       /* 0x02 */
} ipc_cfg_1x_evdo_diversity_config_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_RES                     0x02     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 1X_MRD(1) | EVDO_MRD(1) | SHDR(1)| DIVERSITY_TEST(1)
-----------------------------------------------------------------------------------*/
/* 1X_MRD field,  EVDO_MRD field, SHDR field, DIVERSITY_TEST field
ipc_cfg_1x_evdo_diversity_config_e_type
*/


/*=================================================================================

   SUB_CMD : IPC_CFG_DEVICE_CONFIGURATION      0x23 : Device Configuration Message

==================================================================================*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_GET                     0x02   

 DESCRIPTION :

 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) 
-----------------------------------------------------------------------------------*/

 

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_SET                     0x03     

 DESCRIPTION :

 FORMAT :
---------------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CHANGE_FLAG(1) | IMSI_CLASS(1) | IMSI_M_11_12(1) | IMSI_S2(2)| IMSI_S1[0](2)
---------------------------------------------------------------------------------------------------
 IMSI_S1[1](2) | IMSI_S1[2](2) | LOCAL_CONTROL(1) |  IMSI_ADDR_NUM(1) | SID_NID_NUM(1) | SID_NID_PAIR(x)
---------------------------------------------------------------------------------------------------*/
 

/* CHANGE FLAG(1) */
typedef enum {
  IMSI_CLASS            = 0x01,
  IMSI_M_11_12        = 0x02,
  IMSI_S2                  = 0x04,
  IMSI_S1                  = 0x08,
  LOCAL_CONTROL      = 0x10,
  IMSI_ADDR_NUM         = 0x20,
  SID_NID_PAIR          = 0x40
}ipc_cfg_devide_config_flag_e_type;

/* SID_NID_PAIR(x)  ; x =(SID_NID_NUM(1)) ; MAX X = 20 */
/*  SID (2) | NID (2) | ...*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_RES                     0x02

 DESCRIPTION :

 FORMAT :
---------------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | IMSI_CLASS(1) | IMSI_M_11_12(1) | IMSI_S2(2)| IMSI_S1[0](2)
---------------------------------------------------------------------------------------------------
 IMSI_S1[1](2) | IMSI_S1[2](2) | LOCAL_CONTROL(1) |  IMSI_ADDR_NUM(1) | MAX_SID_NID(1) | SID_NID_NUM(1) | SID_NID_PAIR(x)
---------------------------------------------------------------------------------------------------*/

/* SID_NID_PAIR(x)  ; x =(SID_NID_NUM(1)) ; MAX X = 20 */
/*  SID (2) | NID (2) | ...*/



/*=================================================================================

   SUB_CMD : IPC_CFG_USER_LOCK_CODE_STATUS      0x24 : PDA user lock code status Message
   
==================================================================================*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE : IPC_CMD_SET                     0x03     

 DESCRIPTION :
   
 FORMAT :
-------------------------------------------------------------- 
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | LOCK_CODE_STATUS(1) 
---------------------------------------------------------------*/

/* CHANGE FLAG(1) */
typedef enum {
  USER_LOCL_CODE_STATUS_OFF      = 0x00,
  USER_LOCL_CODE_STATUS_ON        = 0x00
}ipc_cfg_user_lock_code_status_e_type;


/*********************************************************************************

              Sub Command of IPC_IMEI_CMD[0x10]

**********************************************************************************/
typedef enum{
  IPC_IMEI_START=0x01,				/* 0x01 : IMEI Tool Start Message */
  IPC_IMEI_CHECK_DEVICE_INFO,		/* 0x02 : IMEI Tool Check Device Infomation Message */
  IPC_IMEI_PRE_CONFIG,				/* 0x03 : IMEI Tool Pre Config Message */
  IPC_IMEI_WRITE_ITEM,				/* 0x04 : IMEI Tool Item Writee Message */
  IPC_IMEI_REBOOT,					/* 0x05 : IMEI Tool Reboot message */
  IPC_IMEI_VERIFY_FACTORY_RESET,	/* 0x06 : IMEI Tool Clear Reset Result Message  */
  IPC_IMEI_COMPARE_ITEM,			/* 0x07 : IMEI Tool Compare Item Message */
  IPC_IMEI_MASS_STORAGE_INFO,       /* 0x08 : IMEI Tool Mass Storage Information Message */
  IPC_IMEI_MAX
} ipc_imei_sub_cmd_type;




/*=================================================================

   SUB_CMD(1) : IPC_IMEI_START      0x01 : IMEI Tool Start Message

=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_RESP             0x02     */
/*    IPC_CMD_NOTI             0x03     */
/* none */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_CFRM                     0x04

 DESCRIPTION :
   Confirm IMEI Tool Start Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | RESULT(1) |
-------------------------------------------------------------------------------------
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_INDI                     0x01

 DESCRIPTION :
   Indication IMEI Tool Start Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | START_TYPE(1)|
-------------------------------------------------------------------------------------
*/
/* START_TYPE field */
typedef enum {
	IPC_IMEI_START_MODE_WRITE 	= 0x01,				/* 0x01 : Start of Write mode*/
	IPC_IMEI_START_MODE_COMPARE,				/* 0x02 : Start of Compare mode*/
	IPC_IMEI_START_MODE_STORAGE_COPY,          			 /* 0x03 : Start of Storage copy mode*/
	IPC_IMEI_START_MODE_MAX
} ipc_imei_start_mode_e_type;

/* RESULT field */
typedef enum {
	IPC_IMEI_RESULT_CODE_FAIL 	= 0x00,				/* 0x00 : Operation is failed*/
	IPC_IMEI_RESULT_CODE_SUCCESS,					/* 0x01 : Operation is successed*/
	IPC_IMEI_RESULT_CODE_MAX
} ipc_imei_result_code_e_type;



/*=================================================================

   SUB_CMD(1) : IPC_IMEI_CHECK_DEVICE_INFO  0x02 : IMEI Tool Check Device Info Message

=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_SET              0x03     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_RESP             0x02     */
/*    IPC_CMD_NOTI             0x03     */
/* none */

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_CFRM                     0x04

 DESCRIPTION :
   Confirm IMEI Tool Check Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SW_VER(32) | HW_VER(8)| RTC(7)|
-------------------------------------------------------------------------------------
 RFCAL_DATE(16) | BT_ID (6)|WIFY_MAC_ADDR (6) | IMEI (14)
-------------------------------------------------------------------------------------
*/

/* RTC Field */
/* RTC   , none ä  */

/* IMEI Field */
/* IMEI ȣ  , none ä */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_INDI                     0x01

 DESCRIPTION :
   Indication IMEI Tool Check Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
-------------------------------------------------------------------------------------
*/



/*=================================================================

   SUB_CMD(1) : IPC_IMEI_PRE_CONFIG      0x03 : IMEI Tool Pre Configuration Message

=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_NOTI             0x03     */
/* none */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
   Get IMEI Tool Pre Config Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
   Set IMEI Tool Pre Config Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | COUNTRY (2) |OPERATOR (2)|
-------------------------------------------------------------------------------------
*/
/* COUNTRY Field */
/* See ipc_imei_pre_config_country_e_type */

/* OPERATOR Field */
/* Not defined yet */

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_CFRM                     0x04

 DESCRIPTION :
   confirm IMEI Tool Pre Config Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | RESULT (1) | COUNTRY (2) |OPERATOR (2)|
-------------------------------------------------------------------------------------
*/

/* RESULT Field */
/* See ipc_imei_result_code_e_type */

/* COUNTRY Field */
/* See ipc_imei_pre_config_country_e_type */

/* OPERATOR Field */
/* Not defined yet */

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_INDI                    0x01

 DESCRIPTION :
   indication IMEI Tool Pre Config Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                0x02

 DESCRIPTION :
   Set IMEI Tool Pre Config Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | COUNTRY (2) | OPERATOR (2) | ITEM_DATA_LEN(4) |
-------------------------------------------------------------------------------------
*/

/* COUNTRY Field */
/* See ipc_imei_pre_config_country_e_type */

/* OPERATOR Field */
/* Not defined yet */

/* ITEM_DATA_LEN Field */

/* COUNTRY field */
typedef enum{
   IPC_IMEI_COUNTRY_DEFAULT = 0x00,
   IPC_IMEI_COUNTRY_AUSTRALIA,
   IPC_IMEI_COUNTRY_AUSTRIA,
   IPC_IMEI_COUNTRY_BELGIUM,
   IPC_IMEI_COUNTRY_CHINA,
   
   IPC_IMEI_COUNTRY_CROATIA,  /*5*/
   IPC_IMEI_COUNTRY_CZECHO,
   IPC_IMEI_COUNTRY_DENMARK,
   IPC_IMEI_COUNTRY_EGYPT,
   IPC_IMEI_COUNTRY_FINLAND,
   
   IPC_IMEI_COUNTRY_FRANCE, /* 10 */
   IPC_IMEI_COUNTRY_GERMANY,
   IPC_IMEI_COUNTRY_GREECE,
   IPC_IMEI_COUNTRY_HUNGARY,
   IPC_IMEI_COUNTRY_IRELANDS,
   
   IPC_IMEI_COUNTRY_ISRAEL, /* 15 */
   IPC_IMEI_COUNTRY_ITALY,
   IPC_IMEI_COUNTRY_JAPAN,
   IPC_IMEI_COUNTRY_KOREA,
   IPC_IMEI_COUNTRY_MALTA,
   
   IPC_IMEI_COUNTRY_NETHERLANDS, /* 20 */
   IPC_IMEI_COUNTRY_NEWZEALAND,
   IPC_IMEI_COUNTRY_NORWAY,
   IPC_IMEI_COUNTRY_POLAND,
   IPC_IMEI_COUNTRY_PORTUGAL,
   
   IPC_IMEI_COUNTRY_SLOVENIA,  /* 25*/
   IPC_IMEI_COUNTRY_SOUTHAFRICA,
   IPC_IMEI_COUNTRY_SPAIN,
   IPC_IMEI_COUNTRY_SWEDEN,
   IPC_IMEI_COUNTRY_SWISS,
   
   IPC_IMEI_COUNTRY_TURKEY,  /* 30 */
   IPC_IMEI_COUNTRY_UK,
   IPC_IMEI_COUNTRY_UK_PREPAID,
   IPC_IMEI_COUNTRY_US,
   IPC_IMEI_TEST_0,
   
   IPC_IMEI_TEST_1,  /* 35 */
   IPC_IMEI_TEST_2,
   IPC_IMEI_TEST_3,
   IPC_IMEI_TEST_4,
   IPC_IMEI_TEST_5,
   
   IPC_IMEI_TEST_6,  /* 40 */
   IPC_IMEI_TEST_7,
   IPC_IMEI_TEST_8,
   IPC_IMEI_TEST_9,
   IPC_IMEI_COUNTRY_RUSSIA,
   
   IPC_IMEI_COUNTRY_UKRAINE,  /* 45 */
   IPC_IMEI_COUNTRY_ROMANIA,	
   IPC_IMEI_COUNTRY_LATVIA,
   IPC_IMEI_COUNTRY_ESTONIA,
   IPC_IMEI_COUNTRY_LITHUANIA,
   
   IPC_IMEI_COUNTRY_SERBIA,  /* 50 */
   IPC_IMEI_COUNTRY_ALBANIA,
   IPC_IMEI_COUNTRY_KAZAKHSTAN,
   IPC_IMEI_COUNTRY_SLOVAKIA,	
   IPC_IMEI_COUNTRY_MACEDONIA,
   
   IPC_IMEI_COUNTRY_BULGARIA,  /* 55 */
   IPC_IMEI_COUNTRY_BALTIC,
   IPC_IMEI_COUNTRY_ARAB,
   IPC_IMEI_COUNTRY_NORDIC, 	//(,ũ,ɶ,)
   IPC_IMEI_COUNTRY_ICELAND,
   
   IPC_IMEI_COUNTRY_INDONESIA,  /* 60 */
   IPC_IMEI_COUNTRY_MALAYSIA,
   IPC_IMEI_COUNTRY_PHILIPPINE,
   IPC_IMEI_COUNTRY_SINGAPORE,
   IPC_IMEI_COUNTRY_THAILAND,
   
   IPC_IMEI_COUNTRY_VIETNAM,  /* 65 */
   IPC_IMEI_COUNTRY_INDIA,
   IPC_IMEI_COUNTRY_ALGERIA,
   IPC_IMEI_COUNTRY_TUNISIA,
   IPC_IMEI_COUNTRY_MOROCCO,

   IPC_IMEI_COUNTRY_QATAR,    /* 70 */
   IPC_IMEI_COUNTRY_PAKISTAN,
   IPC_IMEI_COUNTRY_KUWAIT,
   IPC_IMEI_COUNTRY_BAHRAIN,
   IPC_IMEI_COUNTRY_LEBANON,
   
   IPC_IMEI_COUNTRY_OMAN,    /* 75 */
   IPC_IMEI_COUNTRY_JORDAN,
   IPC_IMEI_COUNTRY_SYRIA,
   IPC_IMEI_COUNTRY_KSA,
   IPC_IMEI_COUNTRY_HONGKONG,

   IPC_IMEI_COUNTRY_TAIWAN, /* 80 */
   IPC_IMEI_COUNTRY_CANADA,
   IPC_IMEI_COUNTRY_MEXICO,
   IPC_IMEI_COUNTRY_ARGENTINA,
   IPC_IMEI_COUNTRY_LIBYA,

   IPC_IMEI_COUNTRY_BRAZIL, /* 85 */
   IPC_IMEI_COUNTRY_VENEZUELA,
   IPC_IMEI_COUNTRY_CHILE,
   IPC_IMEI_COUNTRY_PERU,
   IPC_IMEI_COUNTRY_COLOMBIA,   

   IPC_IMEI_COUNTRY_GUATEMALA, /* 90 */
   IPC_IMEI_COUNTRY_ECUADOR,
   IPC_IMEI_COUNTRY_PUERTORICO,
   IPC_IMEI_COUNTRY_KENYA,
   IPC_IMEI_COUNTRY_NIGERIA,
   
   IPC_IMEI_COUNTRY_URUGUAY, /* 95 */
   IPC_IMEI_COUNTRY_PARAGUAY,
   IPC_IMEI_COUNTRY_BENELUX,
   IPC_IMEI_COUNTRY_EUROPE,
   IPC_IMEI_COUNTRY_SEASIA,

   IPC_IMEI_COUNTRY_MEASIA, /* 100 */
   IPC_IMEI_COUNTRY_EASTERN_EUROPE,   
   IPC_IMEI_COUNTRY_IRAN,
   IPC_IMEI_COUNTRY_CYPRUS,
   IPC_IMEI_COUNTRY_DIGICEL,
   IPC_IMEI_COUNTRY_BOSNIA,
   
   IPC_IMEI_COUNTRY_MAX
}ipc_imei_pre_config_country_e_type;


/*=================================================================

   SUB_CMD(1) : IPC_IMEI_WRITE_ITEM      0x04 : IMEI Tool Write Item Message

=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_CFRM       	   0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_NOTI             0x03     */
/* none */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                        0x02

 DESCRIPTION :
  Get IMEI Tool Writee Item Message
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | FRAME_INDEX (1)  |
-------------------------------------------------------------------------------------
*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION :
   Set IMEI Tool Write Item Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | REMAIN_FRAME (1) | ITEM_COUNT(1) |
-------------------------------------------------------------------------------------
 | ITEM_ID (2) |  ITEM_LEN (2) | ITEM_VALUE (x) |
-------------------------------------------------------------------------------------
*/
/*if ITEM_VALUE have Element Type 
-----------------------------------------------------------------------------
 |ELEMENT_COUNT (1) | ELEMENT_ID (2) | ELEMENT_LEN (2) | ELEMENT_VALUE (x)|
----------------------------------------------------------------------------
MAX_IMEI_ITEM_DATA_LEN  450
*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                        0x02

 DESCRIPTION :
   Response IMEI Tool Write Item Message
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | REMAIN_FRAME (1) | ITEM_COUNT(1) |
-------------------------------------------------------------------------------------
 | ITEM_ID (2) |  ITEM_LEN (2) | ITEM_VALUE (x) |
-------------------------------------------------------------------------------------
*/
/*if ITEM_VALUE have Element Type 
-----------------------------------------------------------------------------
 |ELEMENT_COUNT (1) | ELEMENT_ID (2) | ELEMENT_LEN (2) | ELEMENT_VALUE (x)|
----------------------------------------------------------------------------
MAX_IMEI_ITEM_DATA_LEN	450
*/

/*
    
------------------------------------------------------------------------------
  
  ITEM_ID: GENERAL_LOCK(0x81)
  
    Element 1 : Lock Setting , Type:integer, Len:1
    Element 2 : Master key, Type:string, Len:Max8
------------------------------------------------------------------------------
  ITEM_ID: NP_LOCK(0x82)~SIM_USIM_LOCK(0x86)

    Element 1 : Attempt count, Type:integer, Len:1
    Element 2 : Max Attempt count, Type:integer, Len:1
    Element 3 : Personalization Code, Type:string, Len:Max1000
    Element 4 : Unlock key, Type:string, Len:Max8
------------------------------------------------------------------------------
*/


/* ITEM_ID field */
typedef enum{
    IPC_IMEI_ITEM_BT_ID	= 0x0001, 		/* 0x01 :Bluetooth ID  Type:String Length:12 */
    IPC_IMEI_ITEM_GREETING_MSG,			/* 0x02 : Greeting Message Type:integer Len: Max 255 */
    IPC_IMEI_ITEM_LANG,					/* 0x03 : Language Type: integer Len:2*/
    IPC_IMEI_ITEM_T9CODE, 				/* 0x04 : T9 Code Type:integer Len:1*/
    IPC_IMEI_ITEM_DICTIONARY, 			/* 0x05 : Dictionary Type:integer Len:1*/
    IPC_IMEI_ITEM_VOICE_MAILBOX_NUM,	/* 0x06 : Voiec Mailbox Num Type:string Len:Max255*/
    IPC_IMEI_ITEM_RING_TONE,			/* 0x07 : Ring Tone Type:string Len:Max255*/
    IPC_IMEI_ITEM_MSG_TONE,				/* 0x08 : Message Tone Type:string Len:Max255*/
    IPC_IMEI_ITEM_VIBRATOR,				/* 0x09 : Vibrator Type:integer Len:1*/
    IPC_IMEI_ITEM_CONNECT_TONE,			/* 0x0A : Connect Tone Type:integer Len:1*/
    IPC_IMEI_ITEM_RINGTONE_VOL,			/* 0x0B : Ringtone Volume Type:interger Len:1*/
    IPC_IMEI_ITEM_KEYTONE_VOL,			/* 0x0C : Keypad Volume Type:integer Len:1*/
    IPC_IMEI_ITEM_AUTOREDIAL,			/* 0x0D : Auto Redial Type:integer Len:1*/
    IPC_IMEI_ITEM_ANYKEY_ANSWER,		/* 0x0E : Anykey Answer Type:integer Len:1*/
    IPC_IMEI_ITEM_BACKGROUND,			/* 0x0F : Background image path Type:string Len:Max255*/
    IPC_IMEI_ITEM_DATE_FORMAT,			/* 0x10 : Data Format Type:integer Len:1*/
    IPC_IMEI_ITEM_TIME_FORMAT,			/* 0x11 : Time Format Type:integer Len:1*/
    IPC_IMEI_ITEM_BOOKMARK,				/* 0x12 : Bookmark consist of element type*/
    IPC_IMEI_ITEM_MMS_CLIENT,			/* 0x13 : MMS Client*/
    IPC_IMEI_ITEM_TIME_ZONE,		    /* 0x14 : TIME Zone Setting */
    IPC_IMEI_ITEM_REGISTRY,			    /* 0x15 : REGISTRY SETTING  */
    IPC_IMEI_ITEM_KEYPAD_LAYOUT,        /* 0x16 : Keypad Layout */
    IPC_IMEI_ITEM_T9_DEFAULT_LANGUAGE,  /* 0x17 : T9 Default language */
    IPC_IMEI_ITEM_MTU_SIZE,             /* 0x18 : GPRS MTU Size */
    IPC_IMEI_ITEM_EMERGENCY_CALL_LIST,  /* 0x19 : Emergency call list */
    IPC_IMEI_ITEM_MAX_CONTEXT_NUM,      /* 0x1A : Maximum Context Number */
	/*Reserved 0x1B~0x2F*/

    
    IPC_IMEI_ITEM_OPERATOR  = 0x0030, 	/* 0x30 :Operator name */    
    IPC_IMEI_ITEM_APN_CONFIG,         	/* 0x31 :APN Configuration*/
    IPC_IMEI_ITEM_PROXY_CONFIG,         /* 0x32 :PROXY Configuration*/
    IPC_IMEI_ITEM_EMAIL_CONFIG,         /* 0x33 :EMAIL Configuration*/
    IPC_IMEI_ITEM_MMS_CONFIG,           /* 0x34 :MMS Configuration*/
    IPC_IMEI_ITEM_SPEED_DIAL,           /* 0x35 :Speed Dial */
    IPC_IMEI_ITEM_METANETWORKS,			/* 0x36 : CM_NETWORK Setting  */
    IPC_IMEI_ITEM_NETWORK_MAPPINGS,		/* 0x37 : CM_MAPPING Setting  */
    IPC_IMEI_ITEM_PREFERRED_CONNECTION,		/* 0x38 : Preferred connection Setting  */

    IPC_IMEI_ITEM_DRM = 0x0044,             /* 0x44 : DRM Setting  */
    
    IPC_IMEI_ITEM_AUDIO_GAIN_FOR_BSP = 0x0050, /* 0x50 : Audio gain for BSP Setting  */
    IPC_IMEI_ITEM_AUDIO_GAIN_FOR_MM,            /* 0x51 : Audio gain for MM Setting  */
    IPC_IMEI_ITEM_MOVINAND,                     /* 0x52 : Mass storage format/copy Setting  */
    
	/*Reserved 0x39~0x7F*/

    IPC_IMEI_ITEM_IMEI	=0x0080,		/* 0x80 : IMEI Type:string Len:15*/
    IPC_IMEI_ITEM_GENERAL_LOCK,			/* 0x81 : General Lock Info consist of element type*/
    IPC_IMEI_ITEM_NP_LOCK,				/* 0x82 : Network Personalization Info consist of element type*/
    IPC_IMEI_ITEM_NSP_LOCK,				/* 0x83 : Network Subset Personalization consist of element type*/
    IPC_IMEI_ITEM_SP_LOCK,				/* 0x84 : Service Provide Personalization consist of element type*/
    IPC_IMEI_ITEM_CP_LOCK,				/* 0x85 : Corporate Personalization consist of element type*/
    IPC_IMEI_ITEM_SIM_USIM_LOCK,		/* 0x86 : SIM/USIM Personalization consist of element type*/
    IPC_IMEI_ITEM_NETWORK_MODE,			/* 0x87 : Network Operation Mode Type:integer Len:1*/
    IPC_IMEI_ITEM_SECURITY_CODE,		/* 0x88 : Phone Lock Password Type:string Len:Max8*/
    IPC_IMEI_ITEM_GPRS_MODE,			/* 0x89 : GPRS Mode Type:integer Len:1*/
    /*Reserved 0x8A~0xFFFF*/
    IPC_IMEI_ITEM_MAX = 0xFFFF						
}ipc_imei_item_id_e_type;

/* Element Type*/
typedef enum {
    IPC_IMEI_ELEMENT_INTEGER 	= 0x01,		/* 0x01 : Element is integer value*/
    IPC_IMEI_ELEMENT_STRING					/* 0x02 : Element is string value*/
} ipc_imei_element_type_e_type;

/*IMEI Setting ON/OFF */
typedef enum {
    IPC_IMEI_SETTING_OPER_OFF,				/* 0x00 : Operation OFF*/
    IPC_IMEI_SETTING_OPER_ON				/* 0x01 : Operation ON*/
} ipc_imei_setting_on_off_e_type;

/* DATA FORMAT  */
typedef enum {
    IPC_IMEI_DATA_FORMAT_AMERICAN 	= 0x01,		/* 0x01 : American : mm/dd/yyyy*/
    IPC_IMEI_DATA_FORMAT_EUROPEAN,				/* 0x02 : European : dd/mm/yyyy*/
    IPC_IMEI_DATA_FORMAT_JAPANESE				/* 0x03 : Japanese : yyyy/mm/dd*/
} ipc_imei_data_format_e_type;

/* TIME FORMAT*/
typedef enum {
    IPC_IMEI_TIME_FORMAT_12HOUR 	= 0x01,		/* 0x01 : 12 Hour*/
    IPC_IMEI_TIME_FORMAT_24HOUR					/* 0x02 : 24 Hour*/
} ipc_imei_time_format_e_type;

/* GPRS ATTACH MODE*/
typedef enum {
    IPC_IMEI_GPRS_UNATTACH,						/* 0x00 : Unattach*/
    IPC_IMEI_GPRS_ATTACH						/* 0x01 : Attach*/
} ipc_imei_gprs_attach_e_type;

/* Personalization SETTING*/
typedef enum {
    IPC_IMEI_NO_LOCK,						/* 0x00 : NO Personalization*/
    IPC_IMEI_NP_LOCK,						/* 0x01 : Network Personalization*/
    IPC_IMEI_NSP_LOCK,						/* 0x02 : Network Subset Personalization*/
    IPC_IMEI_SP_LOCK,						/* 0x03 : Service Provider Personalization*/
    IPC_IMEI_CP_LOCK,						/* 0x04 : Corporate Personalization*/
    IPC_IMEI_SIM_LOCK						/* 0x05 : SIM/USIM Personalization*/
} ipc_imei_lock_setting_e_type;

/* NETWORK  MODE*/
typedef enum {
    IPC_IMEI_NET_AUTOMATIC  = 0x01,				/* 0x01 : Automatic*/
    IPC_IMEI_NET_UMTS_ONLY,						/* 0x02 : 3G only*/
    IPC_IMEI_NET_GSM_ONLY,						/* 0x03 : GSM900/1800/1900*/
    IPC_IMEI_NET_GSM900_ONLY,					/* 0x04 : GSM900*/
    IPC_IMEI_NET_GSM1800_ONLY,					/* 0x05 : GSM1800*/
    IPC_IMEI_NET_GSM1900_ONLY					/* 0x06 : GSM1900*/
} ipc_imei_network_mode_e_type;


/* IPC_IMEI_ITEM_VOICE_MAIL Element */
typedef enum {
    IPC_IMEI_ELEMENT_INDEX           = 0x01,        
    IPC_IMEI_ELEMENT_OPERATOR = 0x02,
    IPC_IMEI_ELEMENT_VOICE_MAILBOX_NUM = 0x03
} ipc_imei_element_voice_mailbox_num_e_type;

/* IPC_IMEI_ITEM_RING_TONE Element */
typedef enum {
    /*IPC_IMEI_ELEMENT_INDEX           = 0x01,        */
    /*IPC_IMEI_ELEMENT_OPERATOR = 0x02,    */
    IPC_IMEI_ELEMENT_RING_TONE = 0x03
} ipc_imei_element_ring_tone_e_type;

/* IPC_IMEI_ITEM_MSG_TONE Element */
typedef enum {
    /*IPC_IMEI_ELEMENT_INDEX           = 0x01,        */
    /*IPC_IMEI_ELEMENT_OPERATOR = 0x02,    */
    IPC_IMEI_ELEMENT_MSG_TONE = 0x03
} ipc_imei_element_msg_tone_e_type;

typedef enum {
    /* IPC_IMEI_ELEMENT_INDEX           = 0x01,     */
    /* IPC_IMEI_ELEMENT_OPERATOR        = 0x02,     */
    IPC_IMEI_ELEMENT_VIBRATOR = 0x03
} ipc_imei_element_vobrator_e_type;

/* IPC_IMEI_ITEM_RINGTONE_VOL Element */
typedef enum {
    /*IPC_IMEI_ELEMENT_INDEX           = 0x01,        */
    /*IPC_IMEI_ELEMENT_OPERATOR = 0x02,    */
    IPC_IMEI_ELEMENT_RINGTONE_VOL = 0x03
} ipc_imei_element_ringtone_vol_e_type;

/* IPC_IMEI_ITEM_KEYTONE_VOL Element */
typedef enum {
    /*IPC_IMEI_ELEMENT_INDEX           = 0x01,        */
    /*IPC_IMEI_ELEMENT_OPERATOR = 0x02,    */
    IPC_IMEI_ELEMENT_KEYTONE_VOL = 0x03
} ipc_imei_element_keytone_vol_e_type;

/* IPC_IMEI_ITEM_ANYKEY_ANSWER Element */
typedef enum {
    /*IPC_IMEI_ELEMENT_INDEX           = 0x01,        */
    /*IPC_IMEI_ELEMENT_OPERATOR = 0x02,    */
    IPC_IMEI_ELEMENT_ANYKEY_ANSWER = 0x03
} ipc_imei_element_anykey_answer_e_type;

/* IPC_IMEI_ITEM_BACKGROUND Element */
typedef enum {
    /*IPC_IMEI_ELEMENT_INDEX           = 0x01,        */
    /*IPC_IMEI_ELEMENT_OPERATOR = 0x02,    */
    IPC_IMEI_ELEMENT_BGImage = 0x03
} ipc_imei_element_background_e_type;

/* IPC_IMEI_ITEM_BOOKMARK Element */
typedef enum {
    /* IPC_IMEI_ELEMENT_INDEX           = 0x01,     */
    /* IPC_IMEI_ELEMENT_OPERATOR        = 0x02,     */
    IPC_IMEI_ELEMENT_BOOKMARK_TYPE    = 0x03,            
    IPC_IMEI_ELEMENT_BOOKMARK_FOLDER,                  
    IPC_IMEI_ELEMENT_BOOKMARK_NAME,
    IPC_IMEI_ELEMENT_BOOKMARK_URL,
} ipc_imei_element_bookmark_e_type;

/* IPC_IMEI_ITEM_MMS_CLIENT Element */
typedef enum {
    /* IPC_IMEI_ELEMENT_INDEX           = 0x01,     */
    /* IPC_IMEI_ELEMENT_OPERATOR        = 0x02,     */
    IPC_IMEI_ELEMENT_MMS_CLIENT_SAVE_SENT_MESSAGE  = 0x03,
    IPC_IMEI_ELEMENT_MMS_CLIENT_CREATION_MODE,                /* 0X04 */
    IPC_IMEI_ELEMENT_MMS_CLIENT_DELIVERY_TIME,                /* 0X05 */
    IPC_IMEI_ELEMENT_MMS_CLIENT_EXPIRATION_TIME,                /* 0X06 */
    IPC_IMEI_ELEMENT_MMS_CLIENT_HIDE_ADDRESS,                /* 0X07 */
    IPC_IMEI_ELEMENT_MMS_CLIENT_REQUEST_READ_REPORT,         /* 0X08 */   
    IPC_IMEI_ELEMENT_MMS_CLIENT_REQUEST_DELIVERY_REPORT,     /* 0X09 */
    IPC_IMEI_ELEMENT_MMS_CLIENT_PRIORITY,                    /* 0X0A */
    IPC_IMEI_ELEMENT_MMS_CLIENT_OUTGOING_MESSAGE_SIZE,       /* 0X0B */
    IPC_IMEI_ELEMENT_MMS_CLIENT_REJECT_UNKNOWN_SENDER,       /* 0X0C */
    IPC_IMEI_ELEMENT_MMS_CLIENT_ROAMING_DOWNLOADMODE,        /* 0X0D */
    IPC_IMEI_ELEMENT_MMS_CLIENT_HOME_DOWNLOADMODE,           /* 0X0E */
    IPC_IMEI_ELEMENT_MMS_CLIENT_REPORT_ALLOWED,              /* 0X0F */
    IPC_IMEI_ELEMENT_MMS_CLIENT_INFORMATION_ALLOWED,         /* 0X10 */
    IPC_IMEI_ELEMENT_MMS_CLIENT_ADVERT_ALLOWED,              /* 0X11 */
    IPC_IMEI_ELEMENT_MMS_CLIENT_UAPROFILE,                   /* 0X12 */
    IPC_IMEI_ELEMENT_MMS_CLIENT_USERAGENT,                   /* 0X13 */
    IPC_IMEI_ELEMENT_MMS_CLIENT_DEFAULT_CONNECTION,          /* 0X14 */    
} ipc_imei_element_mms_client_e_type;
    

/* IPC_IMEI_ITEM_REGISTRY Element */
typedef enum {
    /*IPC_IMEI_ELEMENT_INDEX            = 0x01,*/
    /* IPC_IMEI_ELEMENT_OPERATOR        = 0x02,     */
    IPC_IMEI_ELEMENT_REGISTRY_TYPE        = 0x03,
    IPC_IMEI_ELEMENT_REGISTRY_SUBKEY,
    IPC_IMEI_ELEMENT_REGISTRY_VALUENAME,
    IPC_IMEI_ELEMENT_REGISTRY_DATA,
} ipc_imei_element_registry_e_type;

/* IPC_IMEI_ITEM_OPERATOR Element */
typedef enum {
    /*IPC_IMEI_ELEMENT_INDEX 	  = 0x01,*/		
    IPC_IMEI_ELEMENT_OPERATOR_NAME= 0x02,
    IPC_IMEI_ELEMENT_OPERATOR_CODE,
} ipc_imei_element_operator_e_type;

/* connection Element 
    IPC_IMEI_ITEM_APN_CONFIG
    IPC_IMEI_ITEM_WAP_CONFIG
    IPC_IMEI_ITEM_MMS_CONFIG
*/

/* IPC_IMEI_ITEM_CONNECTION_NETWORK Element */
typedef enum {	
   		
    IPC_IMEI_ELEMENT_CONNECTION_INTERNET= 0x00,
    IPC_IMEI_ELEMENT_CONNECTION_WAP,            /*0X01*/
    IPC_IMEI_ELEMENT_CONNECTION_MMS,            /*0X02*/
    IPC_IMEI_ELEMENT_CONNECTION_EMAIL,          /*0X03*/
    IPC_IMEI_ELEMENT_CONNECTION_PPP,            /*0X04*/
} ipc_imei_element_connection_network_e_type;

/* IPC_IMEI_ITEM_MAX_CONTEXT_NUM Element */
typedef enum {
    /*IPC_IMEI_ELEMENT_INDEX       = 0x01,*/        
    /*IPC_IMEI_ELEMENT_OPERATOR= 0x02,*/        /*0x02*/
    IPC_IMEI_ELEMENT_MAX_CONTEXT_2G_CONTEXT=0x03,         /*0X03*/
    IPC_IMEI_ELEMENT_MAX_CONTEXT_3G_CONTEXT,           /*0X04*/
} ipc_imei_element_max_context_num_e_type;

/* IPC_IMEI_EMERGENCY_CALL_LIST Element */
typedef enum {
    IPC_IMEI_ELEMENT_EMERGENCY_WITH_SIM= 0x01,
    IPC_IMEI_ELEMENT_EMERGENCY_WITHOUT_SIM,     /*0x02*/
} ipc_imei_element_emergency_call_list_e_type;

/* IPC_IMEI_SPEED_DIAL Element */
typedef enum {
    IPC_IMEI_ELEMENT_SPEED_DIAL_INDEX= 0x01,    /*0x01*/
    IPC_IMEI_ELEMENT_SPEED_DIAL_NAME,           /*0x02*/
    IPC_IMEI_ELEMENT_SPEED_DIAL_KEY,            /*0x03*/
    IPC_IMEI_ELEMENT_SPEED_DIAL_NUMBER,          /*0x04*/
} ipc_imei_element_speed_dial_e_type;

typedef enum {
    /*IPC_IMEI_ELEMENT_INDEX       = 0x01,*/        
    /*IPC_IMEI_ELEMENT_OPERATOR= 0x02,*/        /*0x02*/
    IPC_IMEI_ELEMENT_NETWORK_TYPE=0x03,         /*0X03*/
    IPC_IMEI_ELEMENT_APN_PROFILENAME,           /*0X04*/
    IPC_IMEI_ELEMENT_APN_NETWORK_ID,            /*0X05*/
    IPC_IMEI_ELEMENT_APN,                       /*0X06*/
    IPC_IMEI_ELEMENT_APN_AUTH_TYPE,             /*0X07*/
    IPC_IMEI_ELEMENT_APN_ENABLED,               /*0X08*/
    IPC_IMEI_ELEMENT_APN_USER_ID,               /*0X09*/    
    IPC_IMEI_ELEMENT_APN_PASSWORD,              /*0X0A*/
    IPC_IMEI_ELEMENT_DNS1,                        /*0X0B*/
    IPC_IMEI_ELEMENT_DNS2,                      /*0X0C*/    
} ipc_imei_element_apn_e_type;

typedef enum {
    /*IPC_IMEI_ELEMENT_INDEX 	  = 0x01,*/		
    /*IPC_IMEI_ELEMENT_OPERATOR= 0x02,   */     /*0x02*/
    IPC_IMEI_ELEMENT_PROXY_PROFILE = 0x03,      /*0X03*/
    IPC_IMEI_ELEMENT_PROXY_SOURCE_ID,           /*0x04*/
    IPC_IMEI_ELEMENT_PROXY_DESTINATION_ID,      /*0X05*/
    IPC_IMEI_ELEMENT_PROXY_TYPE,                /*0X06*/
    IPC_IMEI_ELEMENT_PROXY_ADDRESS,             /*0X07*/
    IPC_IMEI_ELEMENT_PROXY_PORT,                /*0X08*/
    IPC_IMEI_ELEMENT_PROXY_AUTH_TYPE,           /*0X09*/
    IPC_IMEI_ELEMENT_PROXY_USER_ID,             /*0X0A*/
    IPC_IMEI_ELEMENT_PROXY_PASSWORD,            /*0X0B*/
    IPC_IMEI_ELEMENT_MMS_SERVER_URL,            /*0X0C*/
    IPC_IMEI_ELEMENT_HOMEURL,                   /*0X0D*/
} ipc_imei_element_proxy_e_type;

/* IPC_IMEI_ITEM_EMAIL Element */
typedef enum {
    /*IPC_IMEI_ELEMENT_INDEX 	  = 0x01,*/		
    /*IPC_IMEI_ELEMENT_OPERATOR= 0x02,   */
    IPC_IMEI_ELEMENT_EMAIL_ID=0x03,             /*0x03*/
    IPC_IMEI_ELEMENT_EMAIL_PROFILENAME,         /*0X04*/
    IPC_IMEI_ELEMENT_EMAIL_NETWORK_ID,          /*0X05*/
    IPC_IMEI_ELEMENT_EMAIL_USER_NAME,           /*0X06*/
    IPC_IMEI_ELEMENT_EMAIL_SERVICETYPE,         /*0X07*/
    IPC_IMEI_ELEMENT_EMAIL_INSERVER,            /*0X08*/
    IPC_IMEI_ELEMENT_EMAIL_OUTSERVER,           /*0X09*/
    IPC_IMEI_ELEMENT_EMAIL_REPLYADDRESS,        /*0X0A*/
    IPC_IMEI_ELEMENT_EMAIL_USER_ID,	            /*0X0B*/
    IPC_IMEI_ELEMENT_EMAIL_PASSWORD,            /*0X0C*/
    IPC_IMEI_ELEMENT_EMAIL_DOMAIN,              /*0X0D*/
    IPC_IMEI_ELEMENT_EMAIL_OUTSERVER_AUTH,      /*0X0E*/
    IPC_IMEI_ELEMENT_EMAIL_SSL,                 /*0X0F*/
    IPC_IMEI_ELEMENT_EMAIL_DOWN_DAY,            /*0X10*/
    IPC_IMEI_ELEMENT_EMAIL_DOWN_SIZE,           /*0X11*/
    IPC_IMEI_ELEMENT_EMAIL_DOWN_FREQ,           /*0X12*/	
    
} ipc_imei_element_email_e_type;

/* IPC_IMEI_ITEM_MMS_CONFIG Element */
typedef enum {
    /*IPC_IMEI_ELEMENT_INDEX 	  = 0x01,*/		
	/*IPC_IMEI_ELEMENT_OPERATOR   = 0x02,   */
	IPC_IMEI_ELEMENT_MMS_CONFIG_PROFILENAME=0x03,           /* 0X03 */
	IPC_IMEI_ELEMENT_MMS_CONFIG_GPRS_PROFILENAME,           /* 0X04 */
	IPC_IMEI_ELEMENT_MMS_CONFIG_CSD_PROFILENAME,            /* 0X05 */
	IPC_IMEI_ELEMENT_MMS_CONFIG_MMSC_URL,                   /* 0X06 */
	IPC_IMEI_ELEMENT_MMS_CONFIG_PROXY_USE,                  /* 0X07 */
	IPC_IMEI_ELEMENT_MMS_CONFIG_PROXY_ADDRESS,              /* 0X08 */
	IPC_IMEI_ELEMENT_MMS_CONFIG_PROXY_PORT,                 /* 0X09 */
	IPC_IMEI_ELEMENT_MMS_CONFIG_BEARER,                     /* 0X0A */
	IPC_IMEI_ELEMENT_MMS_CONFIG_DNS1,                       /* 0X0B */
	IPC_IMEI_ELEMENT_MMS_CONFIG_DNS2,                       /* 0X0C */
	IPC_IMEI_ELEMENT_MMS_CONFIG_APN,                        /* 0X0D */
	IPC_IMEI_ELEMENT_MMS_CONFIG_GPRS_LOGIN_ID,              /* 0X0E */
	IPC_IMEI_ELEMENT_MMS_CONFIG_GPRS_LOGIN_PASSWORD,        /* 0X0F */
	IPC_IMEI_ELEMENT_MMS_CONFIG_DIAL_NUMBER,                /* 0X10 */
	IPC_IMEI_ELEMENT_MMS_CONFIG_LOGIN_ID,               /* 0X11 */
	IPC_IMEI_ELEMENT_MMS_CONFIG_LOGIN_PASSWORD,         /* 0X12 */
	IPC_IMEI_ELEMENT_MMS_CONFIG_DATA_CALLTYPE,              /* 0X13 */
	IPC_IMEI_ELEMENT_MMS_CONFIG_IS_DATA_CALLTYPE,           /* 0X14 */
	
} ipc_imei_element_mms_config_e_type;

/* IPC_IMEI_ITEM_METANETWORKS Element */
typedef enum {
    /*IPC_IMEI_ELEMENT_INDEX 	  = 0x01,*/		
    /*IPC_IMEI_ELEMENT_OPERATOR   = 0x02,   */
    IPC_IMEI_ELEMENT_METANETWORKS_NAME=0x03,           /* 0X03 */
    IPC_IMEI_ELEMENT_METANETWORKS_DESTID,           /* 0X04 */
} ipc_imei_element_metanetworks_e_type;

/* IPC_IMEI_ITEM_NETWORK_MAPPINGS Element */
typedef enum {
    /*IPC_IMEI_ELEMENT_INDEX 	  = 0x01,*/		
    /*IPC_IMEI_ELEMENT_OPERATOR   = 0x02,   */
    IPC_IMEI_ELEMENT_NETWORK_MAPPINGS_TYPE=0x03,           /* 0X03 */
    IPC_IMEI_ELEMENT_NETWORK_MAPPINGS_PATTERN,              /* 0X04 */
    IPC_IMEI_ELEMENT_NETWORK_MAPPINGS_NETWORK,             /* 0X05 */    
} ipc_imei_element_network_mappings_e_type;

/* IPC_IMEI_ITEM_PREFERRED_CONNECTION Element */
typedef enum {
    /*IPC_IMEI_ELEMENT_INDEX 	  = 0x01,*/		
    /*IPC_IMEI_ELEMENT_OPERATOR   = 0x02,   */
    IPC_IMEI_ELEMENT_PREFERRED_CONNECTION_NETWORK_GUID=0x03,                /* 0X03 */
    IPC_IMEI_ELEMENT_PREFERRED_CONNECTION_NETWORK_CONNECTION_NAME,  /* 0X04 */
} ipc_imei_element_preferred_connection_e_type;



/*=================================================================

   SUB_CMD(1) : IPC_IMEI_REBOOT      0x05 : IMEI Tool Reboot message

=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_SET              0x03     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_RESP             0x02     */
/*    IPC_CMD_NOTI             0x03     */
/* none */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_CFRM                     0x04

 DESCRIPTION :
   Confirm IMEI Tool Reboot message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | RESULT(1) |
-------------------------------------------------------------------------------------
*/
/* RESULT Field */
/* See ipc_imei_result_code_e_type */

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_INDI                     0x01

 DESCRIPTION :
   Indication IMEI Tool Reboot message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | RESET_TYPE (1) |
-------------------------------------------------------------------------------------
*/
/* RESET_TYPE field */
typedef enum {
	IPC_IMEI_REBOOT_RESET_ONLY	= 0x01, 			/* 0x01 : Reset Only*/
	IPC_IMEI_REBOOT_FACTORY_RESET,					/* 0x02 : Factory Reset*/
    IPC_IMEI_REBOOT_POWER_OFF,   					/* 0x03 : Power Off*/
	IPC_IMEI_REBOOT_MAX
} ipc_imei_reboot_mode_e_type;


/*=================================================================

   SUB_CMD(1) : IPC_IMEI_VERIFY_FACTORY_RESET 0x03 : IMEI Tool Verify Factory Reset Message 

=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_RESP             0x02     */
/*    IPC_CMD_NOTI             0x03     */
/* none */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_CFRM                     0x04

 DESCRIPTION :
   Confirm IMEI Tool Verify Factory Reset Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | RESULT(1) | COUNTRY(2) | OPERATOR(2)
-------------------------------------------------------------------------------------
*/
/* RESULT Field */
/* See ipc_imei_result_code_e_type */

/* OPERATOR Field */
/* Reserved */

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_INDI                     0x01

 DESCRIPTION :
   Indication IMEI Tool Verify Factory Reset Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
-------------------------------------------------------------------------------------
*/


/*=================================================================

   SUB_CMD(1) : IPC_IMEI_COMPARE_ITEM      0x07 : IMEI Tool Compare Message

=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_SET                       0x03     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_RESP             0x02     */
/*    IPC_CMD_NOTI             0x03     */
/* none */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_CFRM                     0x04

 DESCRIPTION :
   Confirm IMEI Tool Compare Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | RESULT (1) | REMAIN_FRAME (1) | ITEM_COUNT(1) |
-------------------------------------------------------------------------------------
 | ITEM_ID (2) |  ITEM_LEN (2) | ITEM_VALUE (x) |
-------------------------------------------------------------------------------------
*/
/*if ITEM_VALUE have Element Type 
-----------------------------------------------------------------------------
 |ELEMENT_COUNT (1) | ELEMENT_ID (2) | ELEMENT_LEN (2) | ELEMENT_VALUE (x)|
----------------------------------------------------------------------------
MAX_IMEI_ITEM_DATA_LEN	450
*/
/* RESULT Field */
/* See ipc_imei_result_code_e_type */


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_INDI                     0x01

 DESCRIPTION :
   Indication IMEI Tool Compare Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | REMAIN_FRAME (1) | ITEM_COUNT(1) |
-------------------------------------------------------------------------------------
 | ITEM_ID (2) |  ITEM_LEN (2) | ITEM_VALUE (x) |
-------------------------------------------------------------------------------------
*/
/*if ITEM_VALUE have Element Type 
-----------------------------------------------------------------------------
 |ELEMENT_COUNT (1) | ELEMENT_ID (2) | ELEMENT_LEN (2) | ELEMENT_VALUE (x)|
----------------------------------------------------------------------------
MAX_IMEI_ITEM_DATA_LEN	450
*/


/*=================================================================

   SUB_CMD(1) : IPC_IMEI_MASS_STORAGE_INFO  0x08 : IMEI Tool Mass Storage Information Message

=================================================================*/

/*   IPC_CMD_CFRM              0x04    */
/*	 IPC_CMD_INDI              0x01    */
/* none */

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_CFRM                     0x04

 DESCRIPTION :
   Confirm IMEI Tool Mass Storage Information Message
 FORMAT :
--------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | TOTAL_MASS_STORAGE(4) | USED_MASS_STORAGE(4) |
--------------------------------------------------------------------------------------------
*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_INDI                     0x01

 DESCRIPTION :
   Indication IMEI Tool Mass Storage Information Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
-------------------------------------------------------------------------------------
*/


/*=================================================================*/
//from here, ipc definition for GPS. JG.JANG JAN.08.2007

/*********************************************************************************

              Sub Command of IPC_GPS_CMD[0x11]

**********************************************************************************/
typedef enum{
/* General GPS Commands (0x01 ~ 0x30) */
   IPC_GPS_OPEN=0x01,                               /* 0x01 : GPS Open Message */
   IPC_GPS_CLOSE,					                        /* 0x02 : GPS Stop Message */
   IPC_GPS_START,					                        /* 0x03 : GPS Start Message */  
   IPC_GPS_DEVICE_STATE,                          /* 0x04 : GPS Device State Message */
   IPC_GPS_OPTION,				                        /* 0x05 : GPS Option Message */  
   IPC_GPS_TTFF,						                        /* 0x06 : GPS Time to first fix Message */ 
   IPC_GPS_LOCK_MODE,                             /* 0x07 : GPS Lock State Message */
   IPC_GPS_SECURITY_UPDATE,                    /* 0x08 : GPS Security Update Message */
   IPC_GPS_SSD,                                          /* 0x09 : GPS Shared Secret Data Message */
   IPC_GPS_SECURITY_UPDATE_RATE,           /* 0x0A : GPS Security Update Rate Message */
   IPC_GPS_FIX_REQ,                                   /* 0x0B : GPS Fix Request Message */
   IPC_GPS_POSITION_RESULT,                     /* 0x0C : GPS Position Result Message */
   IPC_GPS_EXT_POSITION_RESULT,              /* 0x0D : GPS Extended Position Information Message */
   IPC_GPS_EXT_STATUS_INFO,                    /* 0x0E : GPS Extended Status Information Message */
   IPC_GPS_PD_CMD_CB,                             /* 0x0F : GPS PD Command Callback Message */
   IPC_GPS_DLOAD_STATUS,			                 /* 0x10 : GPS Data Download Status Message */
   IPC_GPS_END_SESSION,                           /* 0x11 : GPS End Session Message */
   IPC_GPS_FAILURE_INFO,                          /* 0x12 : GPS Failure Information Message */
   
/* XTRA GPS Commands (0x31 ~ 0x60) */
  IPC_GPS_XTRA_SET_TIME_INFO=0x31,                /* 0x31 : XTRA Set Time Info Message */ 
  IPC_GPS_XTRA_SET_DATA,                        /* 0x32 : XTRA Set Data Message */ 
  IPC_GPS_XTRA_CLIENT_INIT_DOWNLOAD,  /* 0x33 : XTRA Client Init Download Message */ 
  IPC_GPS_XTRA_QUERY_DATA_VALIDITY,    /* 0x34 : XTRA Query Data Validity Message */ 
  IPC_GPS_XTRA_SET_AUTO_DOWNLOAD,     /* 0x35 : XTRA Set Auto Download Message */ 
  IPC_GPS_XTRA_SET_XTRA_ENABLE,           /* 0x36 : XTRA Set XTRA Enable Message */ 
  IPC_GPS_XTRA_DOWNLOAD,                      /* 0x37 : XTRA Download Message */ 
  IPC_GPS_XTRA_VALIDITY_STATUS,            /* 0x38 : XTRA Validity Status Message */ 
  IPC_GPS_XTRA_TIME_EVENT,                    /* 0x39 : XTRA Time Event Message */ 
  IPC_GPS_XTRA_DATA_INJECTION_STATUS, /* 0x3A : XTRA Data Injection Status Message */  
  IPC_GPS_XTRA_USE_SNTP,                        /* 0x3B : XTRA SNTP Message */  

/* AGPS GPS Commands (0x61 ~ 0x90) */
  IPC_GPS_AGPS_PDP_CONNECTION=0x61,           /* 0x61 : AGPS PDP Connection Request Message */
  IPC_GPS_AGPS_DNS_QUERY,                    /* 0x62 : AGPS DNS Query Request Message */
  IPC_GPS_AGPS_SSL,                                /* 0x63 : AGPS SSL Message */
  IPC_GPS_AGPS_MODE,                             /* 0x64 : AGPS MODE Set/get Message */
  IPC_GPS_VERIFICATION,                          /* 0x65 : GPS Verification Message */  
  IPC_GPS_DISPLAY_SUPLFLOW,			          /* 0x66 : GPS Display SUPL Session Flow Message */ 
  
  IPC_GPS_MAX
} ipc_gps_sub_cmd_type;
/*********************************************************************************/

/*=================================================================

   SUB_CMD(1) : IPC_GPS_OPEN      0x01 GPS Open Message

=================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_RESP              0x02     */
/*    IPC_CMD_NOTI               0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_INDI                0x01     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION : GPS Open Message SET
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/


/*=================================================================

   SUB_CMD(1) : IPC_GPS_CLOSE      0x02 GPS Close Message

=================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_RESP              0x02     */
/*    IPC_CMD_NOTI               0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_INDI                0x01     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION : GPS Close Message SET
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/


/*=================================================================

   SUB_CMD(1) : IPC_GPS_START      0x03 GPS Start Message

=================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_RESP              0x02     */
/*    IPC_CMD_NOTI               0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_INDI                0x01     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION : GPS Start Message SET
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/

/*=================================================================

   SUB_CMD(1) : IPC_GPS_DEVICE_STATE      0x04 GPS Device State Message

=================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_SET                0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_INDI                0x01     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION : GPS Device State Message GET
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION : GPS Device State Message RESP
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | STATE(1) |
-------------------------------------------------------------------------------------
*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                       0x03

 DESCRIPTION : GPS Device State Message NOTI
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | STATE(1) |
-------------------------------------------------------------------------------------
*/

/* STATE field */
typedef enum
{
    IPC_GPS_STATE_NO_CLIENT_ID = 0,
	IPC_GPS_STATE_ID_VALID_NO_SESSION,
	IPC_GPS_STATE_SESSION_REQUESTED,
	IPC_GPS_STATE_SESSION_STARTED,
	IPC_GPS_STATE_SESSION_COMPLETED,
	IPC_GPS_STATE_SESSION_END_REQUESTED,
	IPC_GPS_STATE_SESSION_ENDED,
	IPC_GPS_STATE_ERROR_UPDATING_POSITION,
    IPC_GPS_STATE_ERROR_NO_BUFFER,
    IPC_GPS_STATE_DLOAD_OCCURRED,
    IPC_GPS_STATE_MAX_STATE  
} ipc_gps_device_state_e_type;


/*=================================================================

   SUB_CMD(1) : IPC_GPS_OPTION      0x05 GPS Option Message

=================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_RESP              0x02     */
/*    IPC_CMD_NOTI               0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_INDI                0x01     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION : GPS Option Message SET
 
 FORMAT :
---------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SESSION_TYPE(1) | OPERATING_TYPE(1) |
 | COLD_START(1) | HSLP_URL{128) | APN(101) | USER_NAME(32) | PASSWORD(32) | AUTH_TYPE(1) |
 | QOS_PERFORMANCE(1) | QOS_ACCURACY_HORIZONTAL(1) | QOS_ACCURACY_VERTICAL(1) | 
 | TIME_BETWEEN_FIXES(1) | EPH_TIMEOUT_INIT(4) | MAX_LOCATION_AGE(2) | QOP_DELAY (1) 
----------------------------------------------------------------------------------------------
*/
//to here, ipc definition for GPS. JG.JANG JAN.03.2007

typedef enum
{
  IPC_GPS_OPTION_SESSION_TYPE,                      /* 0X00 */
  IPC_GPS_OPTION_OPERATING_TYPE,                    /* 0X01 */
  IPC_GPS_OPTION_COLD_START,                        /* 0x02 */                    
  IPC_GPS_OPTION_HSLP_URL,                          /* 0x03 */
  IPC_GPS_OPTION_APN,                               /* 0x04 */
  IPC_GPS_OPTION_USER_NAME,	                        /* 0x05 */												/* 0x05 */
  IPC_GPS_OPTION_PASSWORD,	                        /* 0x06 */												/* 0x06 */
  IPC_GPS_OPTION_AUTH_TYPE,	                        /* 0x07 */												/* 0x07 */
  IPC_GPS_OPTION_QOS_PERFORMANCE,                   /* 0x08 */
  IPC_GPS_OPTION_QOS_ACCURACY_HORIZONTAL,           /* 0x09 */
  IPC_GPS_OPTION_QOS_ACCURACY_VERTICAL,             /* 0x0A */
  IPC_GPS_OPTION_TIME_BETWEEN_FIX,                  /* 0x0B */
  IPC_GPS_OPTION_EPH_TIMEOUT_INT,                   /* 0x0C */
  IPC_GPS_OPTION_MAX_LOCATION_AGE,                  /* 0x0D */
  IPC_GPS_OPTION_QOP_DELAY,                         /* 0x0E */
} ipc_gps_option_tag_e_type;

/* Session Type field */
typedef enum
{
  IPC_GPS_SESSION_SINGLE_FIX,                   /* 0x00 */
  IPC_GPS_SESSION_NAVIGATION_MODE,              /* 0x01 */
} ipc_gps_session_type_e_type;

/* Operating Type field */
typedef enum
{
  IPC_GPS_OPERATION_TYPE_STANDALONE_ONLY,       /* 0X00 */
  IPC_GPS_OPERATION_TYPE_MS_BASED,              /* 0x01 */
  IPC_GPS_OPERATION_TYPE_SPEED_OPTIMAL,         /* 0x02 */
  IPC_GPS_OPERATION_TYPE_ACCURACY_OPTIMAL,      /* 0x03 */
  IPC_GPS_OPERATION_TYPE_DATA_OPTIMAL,          /* 0x04 */
  IPC_GPS_OPERATION_TYPE_NETWORK_ONLY,          /* 0x05 */
  IPC_GPS_GPS_OPERATION_TYPE_BEST_POSITION,         /* 0x06 */
  IPC_GPS_GPS_OPERATION_TYPE_MAX,                  /* 0x07 */
} ipc_gps_operating_type_e_type;

/* Cold Start field */
typedef enum
{
  IPC_GPS_COLDSTART,                            /* 0X00 */
  IPC_GPS_WARMSTART,                            /* 0X01 */
} ipc_gps_cold_start_e_type;

/* SERVER_ADDS field */
// IP address

/* SERVER_PORT field */
// server port

/* CONTEXT ID field */
// context ID

/* QOS_PERFORMANCE field */
// typically 45

/* QOS_ACCURACY_THRESHOLD field */
// typically 50

/* TIME_BETWEEN_FIXES field */
// valid if NUM_FIXES > 1

/* EPH_TIMEOUT_INIT field */
// default 0


/*=================================================================

   SUB_CMD(1) : IPC_GPS_TTFF      0x06 GPS Time To First Fix

=================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_RESP              0x02     */
/*    IPC_CMD_NOTI               0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_INDI                0x01     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION : GPS Time To First Fix NOTIFICATION
 
 FORMAT :
------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | TTFF(1) | 
------------------------------------------------------
*/



/*=================================================================

   SUB_CMD(1) : IPC_GPS_LOCK_MODE      0x07 GPS Lock Mode Message

=================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_SET                0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_INDI                0x01     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION : GPS Lock Mode Message GET
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION : GPS Lock Mode Message RESP
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | GPS_LOCK(1) |
-------------------------------------------------------------------------------------
*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                       0x03

 DESCRIPTION : GPS Lock Mode Message SET
   
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | GPS_LOCK(1) |
-------------------------------------------------------------------------------------
*/

/* GPS_LOCK field */
typedef enum
{
    IPC_GPS_E911_ONLY = 0,
    IPC_GPS_LOCATION_ON,
} ipc_gps_gps_lock_e_type;


/*====================================================================================

   SUB_CMD(1) : IPC_GPS_SECURITY_UPDATE      0x08 GPS Security Update Message
 
======================================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_RESP              0x02     */
/*    IPC_CMD_NOTI               0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_INDI                0x01     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03
 
 DESCRIPTION : GPS Security Update NOTIFICATION
 
 FORMAT :
---------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  HASH_ALG(4) | SEC_DATA_ID(1) |SEC_DATA_LEN(1)
 | SEC_DATA(20)
----------------------------------------------------------------------------------------------
*/ 

/* HASH_ALG */
/* Authentication Hash algorithm (GPS_HASH_ constant)*/
typedef enum {
    IPC_GPS_PDSM_PD_HASH_ALG_NONE =-1,        /* FOR INTERNAL USE OF PDSM ONLY! */
    IPC_GPS_PDSM_PD_HASH_ALG_SHA1,            /* SHA-1 hash diget algorithm for authentication */
    IPC_GPS_PDSM_PD_HASH_ALG_MAX,             /* FOR INTERNAL USE OF PDSM ONLY! */
    IPC_GPS_PDSM_PD_HASH_ALG_SIZE=0x10000000  /* To force enum to 32 bit for windows NT */
} ipc_gps_pdsm_pd_hash_alg_e_type;


/*=================================================================

   SUB_CMD(1) : IPC_GPS_SSD      0x09  GPS Shared Secret Data Message 

=================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_RESP              0x02     */
/*    IPC_CMD_NOTI               0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_INDI                0x01     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION : GPS Shared Secret Data SET
 
 FORMAT :
---------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SSD_LEN(4) |SSD_DATA(1) |
----------------------------------------------------------------------------------------------
*/


/*=================================================================

   SUB_CMD(1) : IPC_GPS_SECURITY_UPDATE_RATE      0x0A  GPS Security Update Rate Message 

=================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_RESP              0x02     */
/*    IPC_CMD_NOTI               0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_INDI                0x01     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION : GPS Security Update Rate SET
 
 FORMAT :
---------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SEC_UPDATE_RATE(1) |
----------------------------------------------------------------------------------------------
*/


/*=================================================================

   SUB_CMD(1) : IPC_GPS_FIX_REQ      0x0B GPS Fix Request Message

=================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_RESP              0x02     */
/*    IPC_CMD_NOTI               0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_INDI                0x01     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION : GPS Fix Request Message SET
 
 FORMAT :
---------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | QOS_ACCURACY(4) |QOS_SESS_TIMEOUT(1) |
 | SESSION(1) | OPERATION_MODE(1) | NUM_FIXS(4) | TIME_BTW_FIXES(4) |
 | SERVER_OPTION(1) | SERVER_ADRS(4) | SERVER_PORT(4) | 
 | CLASS_ID(4) | SEC_DATA_ID(1)| SEC_DATA_LEN(1) | SEC_DATA(20)|  
----------------------------------------------------------------------------------------------
*/
/* QOS_ACCURACY field */
// typically 50

/* QOS_SESS_TIMEOUT field */
// typically 16, standalone 255


/* SESSION */
typedef enum {
   IPC_GPS_PDSM_PD_SESS_TYPE_LATEST       = 0x00,  /* Get latest PD info */
   IPC_GPS_PDSM_PD_SESS_TYPE_NEW,                 /* Get new position */
   IPC_GPS_PDSM_PD_SESS_TYPE_TRACK_IND,           /* Tracking mode independent fixes */
   IPC_GPS_PDSM_PD_SESS_TYPE_DATA_DLOAD,          /* Data download option */
} ipc_gps_session_e_type;

/* OPERATION_MODE */
typedef enum {
    IPC_GPS_PDSM_SESSION_OPERATION_STANDALONE_ONLY      = 0x01,
    IPC_GPS_PDSM_SESSION_OPERATION_MSBASED,
    IPC_GPS_PDSM_SESSION_OPERATION_MSASSISTED,
    IPC_GPS_PDSM_SESSION_OPERATION_OPTIMAL_SPEED,
    IPC_GPS_PDSM_SESSION_OPERATION_OPTIMAL_ACCURACY,
    IPC_GPS_PDSM_SESSION_OPERATION_OPTIMAL_DATA,
    IPC_GPS_PDSM_SESSION_OPERATION_REF_POSITION,
    IPC_GPS_PDSM_SESSION_OPERATION_MAX
} ipc_gps_operation_mode_e_type;

/* NUM_FIXS */
// number of fix

/* TIME_BTW_FIXES field */
// if num_fixs=1, it must be 0.

/* SERVER_OPTION */
typedef enum {
    IPC_GPS_PDSM_SERVER_OPTION_USE_DEFAULT      = 0x00,
    IPC_GPS_PDSM_SERVER_OPTION_USE_LOCAL,
    IPC_GPS_PDSM_SERVER_OPTION_MAX
} ipc_gps_server_option_e_type;

/* SERVER_ADDS field */
// IP address

/* SERVER_PORT field */
// server port

/* CLASS_ID field */
// class ID

/* SEC_DATA_ID */
// security data id

/* SEC_DATA_LEN */
// security data length

/* SEC_DATA */
// security data


/*====================================================================================

   SUB_CMD(1) : IPC_GPS_POSITION_RESULT      0x0C GPS Position Result
 
======================================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_RESP              0x02     */
/*    IPC_CMD_NOTI               0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_INDI                0x01     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION : GPS Position Result NOTIFICATION
 
 FORMAT :
---------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | PD_EVENT(4) |CLIENT_ID(4) |
 | END_STATUS(4) | LAT(4) | LON(4) | TIME_STAMP(4) | LOC_UNC_ANG(1) |LOC_UNC_A(1) |LOC_UNC_P(1) |
 | OPT_FIELD_MASK(1) | ALTITUDE(2) | HEADING(2) | VELOCITY_HOR(2) | FIX_TYPE(1) | VELOCITY_VER(1) |
 | LOC_UNC_V(1) | TIME_STAMP_MSEC(2) | POSITIONING_SOURCE(4) | POSITIONING_MODE(1) 
 | RAND_NUM_INFO(28) | ENCRYPT_DATA(24) |
----------------------------------------------------------------------------------------------
*/ 
/* PD_EVENT*/
// Postion Determination Events

/*CLIENT_ID*/
// client id

 /* END_STATUS*/

typedef enum {
    IPC_GPS_PDSM_PD_END_SESS_NONE = -1,     // For internal use of PDSM 
    IPC_GPS_PDSM_PD_END_OFFLINE,            // Phone Offline 
    IPC_GPS_PDSM_PD_END_NO_SRV,             // No servcie 
    IPC_GPS_PDSM_PD_END_NO_CON,             // No connection with PDE 
    IPC_GPS_PDSM_PD_END_NO_DATA,            // No data available 
    IPC_GPS_PDSM_PD_END_SESS_BUSY,          // Session Manager Busy 
    IPC_GPS_PDSM_PD_END_CDMA_LOCK,          // Phone is CDMA locked 
    IPC_GPS_PDSM_PD_END_GPS_LOCK,           // Phone is GPS locked 
    IPC_GPS_PDSM_PD_END_CON_FAIL,           // Connection failure with PDE 
    IPC_GPS_PDSM_PD_END_ERR_STATE,          // PDSM Ended session because of Error condition 
    IPC_GPS_PDSM_PD_END_CLIENT_END,         // User ended the session 
    IPC_GPS_PDSM_PD_END_UI_END,             // End key pressed from UI 
    IPC_GPS_PDSM_PD_END_NT_END,             // Network Session was ended 
    IPC_GPS_PDSM_PD_END_TIMEOUT,            // Timeout (viz., for GPS Search) 
    IPC_GPS_PDSM_PD_END_PRIVACY_LEVEL,      // Conflicting reques for session and level of privacy 
    IPC_GPS_PDSM_PD_END_NET_ACCESS_ERR,     // Could not connect to the Network 
    IPC_GPS_PDSM_PD_END_FIX_ERROR,             // Error in Fix 
    IPC_GPS_PDSM_PD_END_PDE_REJECT,         // Reject from PDE 
    IPC_GPS_PDSM_PD_END_TC_EXIT,            // Ending session due to TC exit. 
    IPC_GPS_PDSM_PD_END_E911,               // Ending session due to E911 call 
    IPC_GPS_PDSM_PD_END_SERVER_ERROR,      // Added protocol specific error type 
    IPC_GPS_PDSM_PD_END_STALE_BS_INFO,      // Ending because BS info is stale 
    IPC_GPS_PDSM_PD_END_VX_AUTH_FAIL,       // VX lcs agent auth fail 
    IPC_GPS_PDSM_PD_END_UNKNWN_SYS_ERROR,   // Unknown System Error 
    IPC_GPS_PDSM_PD_END_UNSUPPORTED_SERVICE, // Unsupported Service 
    IPC_GPS_PDSM_PD_END_SUBSRIPTION_VIOLATION,  // Subscription Violation 
    IPC_GPS_PDSM_PD_END_FIX_METHOD_FAILURE,     // The desired fix method failed 
    IPC_GPS_PDSM_PD_END_ANTENNA_SWITCH,   // Antenna switch 
    IPC_GPS_PDSM_PD_END_NO_FIX_NO_TX_CONFIRM, // No fix reported due to no tx confirmation rcvd 
    IPC_GPS_PDSM_PD_END_NORMAL_ENDING,            // Network indicated a Normal ending of the session 
    IPC_GPS_PDSM_PD_END_NONSPECIFIED_ERROR,  // No error specified by the network 
    IPC_GPS_PDSM_PD_END_RESOURCE_SHORTAGE,    // No resources left on the network 
    IPC_GPS_PDSM_PD_END_POS_SERVER_NOT_AVAILABLE, // Position server not available 
    IPC_GPS_PDSM_PD_END_UNSUPPORTED_VERSION, // Network reported an unsupported version of protocol
    
    IPC_GPS_PDSM_PD_END_SS_MOLR_ERRORS_SYSTEM_FAILURE, // mapped to corresponding SS-molr-error error code            
    IPC_GPS_PDSM_PD_END_SS_MOLR_ERRORS_UNEXPECTED_DATA_VALUE,
    IPC_GPS_PDSM_PD_END_SS_MOLR_ERRORS_DATA_MISSING,
    IPC_GPS_PDSM_PD_END_SS_MOLR_ERRORS_FACILITY_NOT_SUPPORTED,
    IPC_GPS_PDSM_PD_END_SS_MOLR_ERRORS_SS_SUBSCRIPTION_VIOLATION,
    IPC_GPS_PDSM_PD_END_SS_MOLR_ERRORS_POSITION_METHOD_FAILURE,
    IPC_GPS_PDSM_PD_END_SS_MOLR_ERRORS_UNDEFINED,

    IPC_GPS_PDSM_PD_END_CP_CF_DISRUPT_SMLC_TO, // control plane's smlc timeout, may or may not end pd 
    IPC_GPS_PDSM_PD_END_MT_GUARD_TIMER_EXPIRY, // control plane's MT guard time expires 
    IPC_GPS_PDSM_PD_END_WAIT_ADDITION_ASSIST_EXPIRY, // end waiting for addtional assistance 

    // KDDI specific error codes 
    //.........
    IPC_GPS_PDSM_PD_END_MAX,                // Maximum for PDSM internal use 
    IPC_GPS_PDSM_PD_END_E_SIZE = 0x10000000 // To fix enum Size as int 
} ipc_gps_pdsm_pd_end_e_type;

  
 /* LAT */
  /* LON */
 /* TIME_STAMP*/
 /* LOC_UNC_ANG*/
 /*LOC_UNC_A */
 /*LOC_UNC_P */
 /* OPT_FIELD_MASK*/
 /* ALTITUDE */
 /* HEADING */
 /* VELOCITY_HOR*/
 /* FIX_TYPE */
 /* VELOCITY_VER */
 /* LOC_UNC_V */
 /* TIME_STAMP_MSEC*/
 
 /* POSITIONING_SOURCE*/
 #define IPC_GPS_PDSM_PD_POSITION_SOURCE_GPS 0x0001
#define IPC_GPS_PDSM_PD_POSITION_SOURCE_CELLID 0x0002
#define IPC_GPS_PDSM_PD_POSITION_SOURCE_AFLT 0x0004
#define IPC_GPS_PDSM_PD_LOCATION_SOURCE_HYBRID  0x0008
#define IPC_GPS_PDSM_PD_LOCATION_SOURCE_CELL_ERROR  0x0010
#define IPC_GPS_PDSM_PD_POSITION_SOURCE_DEFAULT     0x0020
#define IPC_GPS_PDSM_PD_POSITION_SOURCE_UNKNOWN 0x0040


 /* POSITIONING_MODE */
 typedef enum{
  IPC_GPS_PDSM_PD_POSITION_MODE_INVALID=-1,
  IPC_GPS_PDSM_PD_POSITION_MODE_MSBASED,
  IPC_GPS_PDSM_PD_POSITION_MODE_MSASSISTED,
  IPC_GPS_PDSM_PD_POSITION_MODE_STANDALONE,
  IPC_GPS_PDSM_PD_POSITION_MODE_UNKNOWN,
  IPC_GPS_PDSM_PD_POSITION_MODE_MAX
} ipc_gps_pdsm_pd_position_mode_e_type;


 /* RAND_NUM_INFO*/
//Random number update

 /* ENCRYPT_DATA */
// Encryption Data



/*====================================================================================

   SUB_CMD(1) : IPC_GPS_EXT_POSITION_RESULT      0x0D GPS extended Position Result
 
======================================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_RESP              0x02     */
/*    IPC_CMD_NOTI               0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_INDI                0x01     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION : GPS Position Result NOTIFICATION
 
 FORMAT :
---------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | PD_EVENT(4) |LAT(8)|LON(8)|ALTITUDE(4)|
 |SPEED_KNOT(4)|SPEED_KMH(4)|HEADING_TRUE(2)|VELOCITY_INCLUDE(1)|FIX_TYPE(1)|
 |HOUR(1)|MINUTE(1)|SEC(2)|DATE(4)|H_DOP(4)|P_DOP(4)|V_DOP(4)|SV_IN_USE(4)|
 |MAGNETIC_VARIATION(8)|SELECTION_TYPE(1)|FIX_QUALITY(1)|NUM_SV_IN_VIEW(1)|SV(48)|
----------------------------------------------------------------------------------------------
*/ 
/* PD_EVENT*/
// Postion Determination Events

/*LAT*/
/*LON*/
/*ALTITUDE*/
/*SPEED_KNOT*/
/*SPEED_KMH*/
/*HEADING_TRUE*/
/*VELOCITY_INCLUDE*/

/*FIX_TYPE*/
typedef enum {
   IPC_GPS_PDSM_PD_FIX_TYPE_NONE = -1,      // FOR INTERNAL USE OF PDSM ONLY! 
   IPC_GPS_PDSM_PD_FIX_TYPE_UNKNOWN = 0,    // Type of fix is unknown.  
   IPC_GPS_PDSM_PD_FIX_TYPE_2D,             // Fix is two-dimensional and only provides latitude and longitude. This occurs when only three GPS satellites are used.  
   IPC_GPS_PDSM_PD_FIX_TYPE_3D,             // Fix is three-dimensional and provides latitude, longitude, and elevation. This occurs when four or more GPS satellites are used. 
   IPC_GPS_PDSM_PD_FIX_TYPE_MAX,            // FOR INTERNAL USE OF PDSM ONLY! 
   IPC_GPS_PDSM_PD_FIX_TYPE_E_SIZE=0x100000 // To force enum to int  
} ipc_gps_pdsm_pd_fix_type_e_type;

 /*HOUR*/
 /*MINUTE*/
 /*SEC*/
 /*DATE*/
 /*H_DOP*/
 /*P_DOP*/
 /*V_DOP*/
 /*SV_IN_USE*/
 /*MAGNETIC_VARIATION*/

 /*SELECTION_TYPE*/
typedef enum {
   IPC_GPS_PDSM_PD_FIX_SELECTION_NONE = -1,      // FOR INTERNAL USE OF PDSM ONLY! 
   IPC_GPS_PDSM_PD_FIX_SELECTION_UNKNOWN = 0,    // Selection type is unknown.  
   IPC_GPS_PDSM_PD_FIX_SELECTION_AUTO,           // Selection mode is automatic.
   IPC_GPS_PDSM_PD_FIX_SELECTION_MANUAL,         // Selection mode is manual.   
   IPC_GPS_PDSM_PD_FIX_SELECTION_MAX,            // FOR INTERNAL USE OF PDSM ONLY! 
   IPC_GPS_PDSM_PD_FIX_SELECTION_E_SIZE=0x100000 // To force enum to int 
} ipc_gps_pdsm_pd_fix_selection_e_type;

 /*FIX_QUALITY*/
typedef enum {
   IPC_GPS_PDSM_PD_FIX_QUALITY_NONE = -1,      // FOR INTERNAL USE OF PDSM ONLY! 
   IPC_GPS_PDSM_PD_FIX_QUALITY_UNKNOWN = 0,    // Quality of fix is unknown. 
   IPC_GPS_PDSM_PD_FIX_QUALITY_GPS,            // Fix uses information from GPS satellites only. 
   IPC_GPS_PDSM_PD_FIX_QUALITY_DGPS,           // Fix uses information from GPS satellites and also a differential GPS (DGPS) station. 
   IPC_GPS_PDSM_PD_FIX_QUALITY_MAX,            // FOR INTERNAL USE OF PDSM ONLY! 
   IPC_GPS_PDSM_PD_FIX_QUALITY_E_SIZE=0x100000 // To force enum to int 
} ipc_gps_pdsm_pd_fix_quality_e_type;


 /*NUM_SV_IN_VIEW*/

 /*SV*/
/*
(SV_NUM(1)+ELEV(1)+AZIMUTH_SNR(2))*12
*/


/*====================================================================================

   SUB_CMD(1) : IPC_GPS_EXT_STATUS_INFO      0x0E GPS extended Position Result
 
======================================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_RESP              0x02     */
/*    IPC_CMD_NOTI               0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_INDI                0x01     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION : GPS Position Result NOTIFICATION
 
 FORMAT : 
---------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CLIENT_ID(4) | EXT_STATUS_TYPE(1)|
 |NUM_SVS(1)|EPH_SVMASK(4)|ALM_SVMASK(4)|EXT_MEAS_REPORT_TYPE(48)|
----------------------------------------------------------------------------------------------
*/ 

/*CLIENT_ID*/

/* EXT_STATUS_TYPE*/
// PDSM_EXT_STATUS_MEASUREMENT=2

/*NUM_SVS*/
//num svs in this measurement

/*EPH_SVMASK*/
//sv mask for the ephemeris

/*ALM_SVMASK*/
//sv mask for almanac 

/*EXT_MEAS_REPORT_TYPE*/
/*
(SV_ID(1)+C_N0(2))*16
*/


/*====================================================================================

   SUB_CMD(1) : IPC_GPS_PD_CMD_CB      0x0F GPS PD Command Callback
 
======================================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_RESP              0x02     */
/*    IPC_CMD_NOTI               0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_INDI                0x01     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION : GPS PD Command Callback NOTIFICATION
 
 FORMAT :
---------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  PD_CMD(4) | PD_CMD_ERR(4) |
----------------------------------------------------------------------------------------------
*/ 

/*PD_CMD*/
typedef enum {
    IPC_GPS_PDSM_PD_CMD_NONE =-1,           // FOR INTERNAL USE OF PDSM ONLY! 
    IPC_GPS_PDSM_PD_CMD_GET_POS,            // Get postion Command 
    IPC_GPS_PDSM_PD_CMD_END_SESSION,        // End PD session 
    IPC_GPS_PDSM_PD_CMD_INJECT_TIME,        // Inject External time 
    IPC_GPS_PDSM_PD_CMD_INJECT_POS,         // Inject external position 
    IPC_GPS_PDSM_PD_CMD_MAX,                // FOR INTERNAL USE OF PDSM ONLY!
    IPC_GPS_PDSM_CMD_E_SIZE=0x10000000  // To force enum to 32 bit for windows NT
} ipc_gps_pdsm_pd_cmd_e_type;


/*PD_CMD_ERR*/
typedef enum {
    IPC_GPS_PDSM_PD_CMD_ERR_NONE=-1,      // FOR INTERNAL USE OF PDSM ONLY! 
    IPC_GPS_PDSM_PD_CMD_ERR_NOERR,        // No errors found 
    IPC_GPS_PDSM_PD_CMD_ERR_CLIENT_ID_P,        // invalid client ID 
    IPC_GPS_PDSM_PD_CMD_ERR_SRV_TYPE_P,        // Bad service parameter (Not used anymore)
    IPC_GPS_PDSM_PD_CMD_ERR_SESS_TYPE_P,        // Bad session type parameter 
    IPC_GPS_PDSM_PD_CMD_ERR_PRIVACY_P, // not used anymore 
    IPC_GPS_PDSM_PD_CMD_ERR_DLOAD_P,   //0x05 // not used anymore 
    IPC_GPS_PDSM_PD_CMD_ERR_NET_ACCESS_P, // not used anymore 
    IPC_GPS_PDSM_PD_CMD_ERR_OPERATION_P,
    IPC_GPS_PDSM_PD_CMD_ERR_NUM_FIXES_P,
    IPC_GPS_PDSM_PD_CMD_ERR_LSINFO_P,       // Wrong server Information parameters 
    IPC_GPS_PDSM_PD_CMD_ERR_TIMEOUT_P,   //0x0A     // Error in timeout parameter 
    IPC_GPS_PDSM_PD_CMD_ERR_QOS_P,           // Error in QOS accuracy thershold param 
    IPC_GPS_PDSM_PD_CMD_ERR_NO_SESS_S,         // No session Active, while trying to end Session 
    IPC_GPS_PDSM_PD_CMD_ERR_SESS_ACT_S,         // Session active for this client  not used anymore 
    IPC_GPS_PDSM_PD_CMD_ERR_BUSY_S,         // Session Busy status 
    IPC_GPS_PDSM_PD_CMD_ERR_OFFLINE_S,        // Phone is offline 
    IPC_GPS_PDSM_PD_CMD_ERR_CDMA_LOCK_S,  //0x10      // Phone is CDMA locked 
    IPC_GPS_PDSM_PD_CMD_ERR_GPS_LOCK_S,        // GPS is locked 
    IPC_GPS_PDSM_PD_CMD_ERR_STATE_S,        // The command is invalid in this state. (Ex When is phone is in E911CB) 
    IPC_GPS_PDSM_PD_CMD_ERR_NO_CONNECT_S,       // Connection Failure with PDE. 
    IPC_GPS_PDSM_PD_CMD_ERR_NO_BUF_L,     // No available PDSM command buffers to queue the command 
    IPC_GPS_PDSM_PD_CMD_ERR_SEARCH_COM_L,     //0x15  // Communication problems with Search - e.g. SRCH buffer shortage 
    IPC_GPS_PDSM_PD_CMD_ERR_CANT_RPT_NOW_S,     // PD Results cannot be reported at this time, retry later or wait. not used anymore 
    IPC_GPS_PDSM_PD_CMD_ERR_MODE_NOT_SUPPORTED_S, // not used anymore 
    IPC_GPS_PDSM_PD_CMD_ERR_PERIODIC_NI_IN_PROGRESS_S,
    IPC_GPS_PDSM_PD_CMD_ERR_AUTH_FAIL_S,      // Client Authentication Failure 
    IPC_GPS_PDSM_PD_CMD_ERR_OTHER,    //0x1A   // A problem other than the above was found 
    IPC_GPS_PDSM_PD_CMD_ERR_MAX,
    IPC_GPS_PDSM_PD_CMD_ERR_E_SIZE = 0x10000000
} ipc_gps_pdsm_pd_cmd_err_e_type;


/*====================================================================================

   SUB_CMD(1) : IPC_GPS_DLOAD_STATUS      0x10 GPS Data Download Status Message
 
======================================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_RESP              0x02     */
/*    IPC_CMD_NOTI               0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_INDI                0x01     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION : GPS Data Download Status NOTIFICATION
 
 FORMAT :
---------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  DLOAD_STATUS(4) |
----------------------------------------------------------------------------------------------
*/ 

/*DLOAD_STATUS*/
#define IPC_GPS_PDSM_PD_EVENT_DLOAD_BEGIN 0x10000000UL  // Dload begin event 
#define IPC_GPS_PDSM_PD_EVENT_DLOAD       0x20000000UL      // Dload occurred event 
#define IPC_GPS_PDSM_PD_EVENT_DLOAD_DONE  0x40000000UL  // Dload done event 
#define IPC_GPS_PDSM_PD_DLOAD_EVENT_END         0x80000000UL  // Indicating End of dload session. 


/*=================================================================

   SUB_CMD(1) : IPC_GPS_END_SESSION      0x11 GPS End Session Message

=================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_RESP              0x02     */
/*    IPC_CMD_NOTI               0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_INDI                0x01     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION : GPS End Session SET
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/


/*====================================================================================

   SUB_CMD(1) : IPC_GPS_FAILURE_INFO      0x12 GPS Failure Information Message
 
======================================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_RESP              0x02     */
/*    IPC_CMD_NOTI               0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_INDI                0x01     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION : GPS Data Failure Information NOTIFICATION
 
 FORMAT :
---------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  FAIL_INFO(1) |
----------------------------------------------------------------------------------------------
*/ 
/* INFO */
typedef enum {
    IPC_GPS_ERR_INFO_UPDATE_FAILURE     = 0x01,
    IPC_GPS_ERR_INFO_MAX
} ipc_gps_err_info_e_type;



/*====================================================================================

   SUB_CMD(1) : IPC_GPS_XTRA_SET_TIME_INFO      0x31 XTRA set time info Message

======================================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI                0x01     */
/*    IPC_CMD_NOTI               0x03     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION : XTRA Set Time Info Message SET
 
 FORMAT :
------------------------------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | TIME_MSEC(8) | TIME_UNC_MSEC(4) | REF_TO_UTC_TIME(1) | FORCE_FLAG(1) |
------------------------------------------------------------------------------------------------------------------
*/


/*=============================================================================

   SUB_CMD(1) : IPC_GPS_XTRA_SET_DATA      0x32 IPC_GPS_XTRA_SET_DATA

===============================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI                0x01     */
/*    IPC_CMD_NOTI               0x03     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION : XTRA Set Data Message SET
 
 FORMAT :
----------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | DC_STATUS(1) |
----------------------------------------------------------
*/


/*========================================================================================

   SUB_CMD(1) : IPC_GPS_XTRA_CLIENT_INIT_DOWNLOAD      0x33 XTRA_CLIENT_INIT_DOWNLOAD

==========================================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI                0x01     */
/*    IPC_CMD_NOTI               0x03     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION : XTRA Client Iint Download Message SET
 
 FORMAT :
-------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------
*/


/*=======================================================================================

   SUB_CMD(1) : IPC_GPS_XTRA_QUERY_DATA_VALIDITY      0x34 XTRA_QUERY_DATA_VALIDITY

=========================================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI                0x01     */
/*    IPC_CMD_NOTI               0x03     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION : XTRA Query Data Validity Message GET
 
 FORMAT :
---------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | UTC_TIME(8) |
---------------------------------------------------------
*/

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x03

 DESCRIPTION : XTRA Query Data Validity Message RESP
 
 FORMAT :
-------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | RESULT(1) |
-------------------------------------------------------
*/


/*===================================================================================

   SUB_CMD(1) : IPC_GPS_XTRA_SET_AUTO_DOWNLOAD      0x35 XTRA_SET_AUTO_DOWNLOAD

=====================================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI                0x01     */
/*    IPC_CMD_NOTI               0x03     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION : XTRA Set Auto Download Message SET
 
 FORMAT :
-------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | ENABLE_AUTODOWNLOAD(1) | DOWNLOAD_INTERVAL(2) |
-------------------------------------------------------------------------------------------
*/


/*==============================================================================

   SUB_CMD(1) : IPC_GPS_XTRA_SET_XTRA_ENABLE      0x36 XTRA_SET_XTRA_ENABLE

================================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI                0x01     */
/*    IPC_CMD_NOTI               0x03     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION : XTRA Set XTRA Enable Message SET
 
 FORMAT :
-------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | ENABLE(1) |
-------------------------------------------------------
*/


/*=================================================================

   SUB_CMD(1) : IPC_GPS_XTRA_DOWNLOAD      0x37 XTRA_DOWNLOAD

=================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_SET                0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI                0x01     */
/*    IPC_CMD_RESP              0x02     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION : XTRA Download Message NOTI 
 
 FORMAT :
-----------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | PRIMARY_SERVER_LEN(1) | PRIMARY_SERVER(128) |
-----------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------
 | SECONDARY_SERVER_LEN(1) | SECONDARY_SERVER(128) | TERTIARY_SERVER_LEN(1) | TERTIARY_SERVER(128) |
----------------------------------------------------------------------------------------------------
---------------------------------------------
 | MAX_FILE_PART_SIZE(4) | MAX_FILE_SIZE(4) |
---------------------------------------------
*/


/*===============================================================================

   SUB_CMD(1) : IPC_GPS_XTRA_VALIDITY_STATUS      0x38 XTRA_VALIDITY_STATUS

=================================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_SET                0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI                0x01     */
/*    IPC_CMD_RESP              0x02     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION : XTRA Validity Status Message NOTI 
 
 FORMAT :
----------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | REMAINING_HOURS(2) |
----------------------------------------------------------------
*/


/*====================================================================

   SUB_CMD(1) : IPC_GPS_XTRA_TIME_EVENT      0x39 XTRA_TIME_EVENT 

======================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_SET                0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI                0x01     */
/*    IPC_CMD_RESP              0x02     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION : XTRA Time Event Message NOTI 
 
 FORMAT :
-----------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | PRIMARY_SERVER_LEN(1) | PRIMARY_SERVER(128) |
-----------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------
 | SECONDARY_SERVER_LEN(1) | SECONDARY_SERVER(128) | TERTIARY_SERVER_LEN(1) | TERTIARY_SERVER(128) |
----------------------------------------------------------------------------------------------------
------------------------------------
 | ONEWAY_DELAY_FAILOVER_THRESH(4) |
------------------------------------
*/


/*===========================================================================================

   SUB_CMD(1) : IPC_GPS_XTRA_DATA_INJECTION_STATUS      0x3A XTRA_DATA_INJECTION_STATUS

=============================================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_SET                0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI                0x01     */
/*    IPC_CMD_RESP              0x02     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION : XTRA Data Injection Status Message NOTI 
 
 FORMAT :
-------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | STATUS(1) |
-------------------------------------------------------
*/

/*===========================================================================================

   SUB_CMD(1) : IPC_GPS_XTRA_USE_SNTP      0x3B XTRA_USE_SNTP

=============================================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_SET                0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI                0x01     */
/*    IPC_CMD_RESP              0x02     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION : XTRA SNTP Message GET
 
 FORMAT :
-------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------
*/

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION : XTRA SNTP Message RESP
 
 FORMAT :
---------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | USE_SNTP(1) |
---------------------------------------------------------
*/
/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION : XTRA SNTP Message SET
 
 FORMAT :
---------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | USE_SNTP(1) |
---------------------------------------------------------
*/

/*===========================================================================================

   SUB_CMD(1) : IPC_GPS_AGPS_PDP_CONNECTION      0x61 AGPS_PDP_CONNECTION
 
=============================================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_SET                0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI                0x01     */
/*    IPC_CMD_RESP              0x02     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_INDI                     0x01

 DESCRIPTION : AGPS PDP Connection Request Message INDI
 
 FORMAT :
--------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | PDP_COMMAND(1) | PDP_TYPE(1) |
 | SESSION_HANDLE(4) | IFACE_NAME(1) |
--------------------------------------------------------------------------
*/
/* PDP_COMMAND */
typedef enum
{
  IPC_GPS_PDP_OPEN =0x01,
  IPC_GPS_PDP_CLOSE =0x02
}ipc_gps_pdp_cmd_e_type;


/* PDP_TYPE */
typedef enum
{
  IPC_GPS_PDSM_ATL_PDP_IP=0x0,                     /* PDP type IP                 */
  IPC_GPS_PDSM_ATL_PDP_PPP,                        /* PDP type PPP                */
  IPC_GPS_PDSM_ATL_PDP_IPV6,
  IPC_GPS_PDSM_ATL_PDP_MAX=0xff          /* force max to 0xff so that enum is defined as a byte   */
} ipc_gps_pdsm_atl_pdp_type_e_type;

/* SESSION_HANDLE */


/* IFACE_NAME */
typedef enum {
  IPC_GPS_PDSM_ATL_IFACE_NONE = -1, 
  IPC_GPS_PDSM_ATL_IFACE_CDMA_SN = 0,
  IPC_GPS_PDSM_ATL_IFACE_CDMA_AN,
  IPC_GPS_PDSM_ATL_IFACE_UMTS,
  IPC_GPS_PDSM_ATL_IFACE_SIO,
  IPC_GPS_PDSM_ATL_IFACE_LO,
  IPC_GPS_PDSM_ATL_IFACE_WWAN,
  IPC_GPS_PDSM_ATL_IFACE_ANY_DEFAULT,
  IPC_GPS_PDSM_ATL_IFACE_ANY,
  IPC_GPS_PDSM_ATL_IFACE_RM,
  IPC_GPS_PDSM_ATL_IFACE_MAX
} ipc_gps_pdsm_atl_iface_name_e_type;



/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_CFRM                     0x04

 DESCRIPTION : AGPS PDP Connection Request Message CFRM
   
 FORMAT :
------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | RESULT(4) | APN(101) |
------------------------------------------------------------------
*/

typedef enum
{
  IPC_GPS_ATL_EVENT_CLEAR = 0x0000,
  IPC_GPS_ATL_EVENT_OPEN_SUCCESS = 0x0001,
  IPC_GPS_ATL_EVENT_OPEN_FAIL = 0x0002,
  IPC_GPS_ATL_EVENT_CLOSE_SUCCESS = 0x0004,
  IPC_GPS_ATL_EVENT_CLOSE_FAIL = 0x0008,
  IPC_GPS_ATL_EVENT_CONNECT_SUCCESS = 0x0010, 
  IPC_GPS_ATL_EVENT_CONNECT_FAIL = 0x0020,
  IPC_GPS_ATL_EVENT_DISCONNECT_SUCCESS = 0x0040, 
  IPC_GPS_ATL_EVENT_DISCONNECT_FAIL = 0x0080,
  IPC_GPS_ATL_EVENT_SEND = 0x0100,
  IPC_GPS_ATL_EVENT_RECV = 0x0200,
  IPC_GPS_ATL_EVENT_FORCE_DORMANCY_SUCCESS = 0x0400,
  IPC_GPS_ATL_EVENT_FORCE_DORMANCY_FAIL = 0x0800,
  IPC_GPS_ATL_EVENT_UNFORCE_DORMANCY_SUCCESS = 0x1000,
  IPC_GPS_ATL_EVENT_UNFORCE_DORMANCY_FAIL = 0x2000,
  IPC_GPS_ATL_EVENT_NETWORK_FAILURE = 0x4000,
  IPC_GPS_ATL_EVENT_SOCKET_FAILURE = 0x8000,
  IPC_GPS_ATL_EVENT_L2_PROXY_OPEN_ACK = 0x10000,
  IPC_GPS_ATL_EVENT_L2_PROXY_CLOSE_ACK = 0x20000,
  IPC_GPS_ATL_EVENT_DNS_LOOKUP_SUCCESS = 0x40000,
  IPC_GPS_ATL_EVENT_DNS_LOOKUP_FAIL = 0x80000,
} ipc_gps_pdp_result_e_type;

/*===========================================================================================

   SUB_CMD(1) : IPC_GPS_AGPS_DNS_QUERY      0x62 AGPS_DNS_QUERY

=============================================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_SET                0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI                0x01     */
/*    IPC_CMD_RESP              0x02     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_INDI                     0x01

 DESCRIPTION : AGPS DNS Query Request Message INDI
 
 FORMAT :
-------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------
*/

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_CFRM                     0x04

 DESCRIPTION : AGPS PDP Connection Request Message CFRM

 FORMAT :
------------------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | RESULT(4) | ADDRESS_TYPE(1) | IP_ADDRESS(4) | IP_PORT(2) |
------------------------------------------------------------------------------------------------------
*/

/*===========================================================================================

   SUB_CMD(1) : IPC_GPS_AGPS_SSL      0x63 AGPS SSL Message

=============================================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_SET                0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI                0x01     */
/*    IPC_CMD_RESP              0x02     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION : AGPS SSL Message GET
 
 FORMAT :
-------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------
*/

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION : AGPS SSL Message RESP

 FORMAT :
-----------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | RESULT(1) | 
-----------------------------------------------------
*/

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION : AGPS SSL Message SET
 
 FORMAT :
--------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SSL_VALUE(1) | 
--------------------------------------------------------
*/ 

typedef enum
{
  IPC_GPS_MODE_USER_PLANE = 0x00,
  IPC_GPS_MODE_CONTROL_PLANE = 0x01  ,
} ipc_gps_mode_e_type;
/*===========================================================================================

   SUB_CMD(1) : IPC_GPS_AGPS_MODE      0x64 AGPS MODE SET

=============================================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_SET                0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI                0x01     */
/*    IPC_CMD_RESP              0x02     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION : AGPS MODE GET
 
 FORMAT :
-------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------
*/

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION : AGPS MODE RESP

 FORMAT :
-----------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MODE(1) | 
-----------------------------------------------------
*/

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION : AGPS MODE SET

 FORMAT :
--------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MODE(1) | 
--------------------------------------------------------
*/ 

/*=================================================================

   SUB_CMD(1) : IPC_GPS_VERIFICATION        0x65 Verification Message

=================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_RESP              0x02     */
/*    IPC_CMD_SET                0x03     */
/*    IPC_CMD_NOTI               0x03     */
//none

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_CFRM                     0x04

 DESCRIPTION :
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | RESULT(1) | 
-------------------------------------------------------------------------------------
*/

/* RESULT field */
typedef enum
{
  IPC_GPS_VERIFICATION_RESULT_YES,                  /* 0X00 */
  IPC_GPS_VERIFICATION_RESULT_NO,                   /* 0X01 */
  IPC_GPS_VERIFICATION_RESULT_NORESPONSE,           /* 0X02 */
} ipc_gps_verification_result_tag_e_type;
/* Reserved for confirmation message */

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_INDI                       0x01

 DESCRIPTION : Verification Message INDI
   
 FORMAT :
---------------------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | NUMBER_OF_RECORD(1) | RECORD(n) [TYPE(1) | LENGTH(N) | VALUE(N)] |
---------------------------------------------------------------------------------------------------------
*/

typedef enum
{
  IPC_GPS_VERIFICATION_NOTIFY_TYPE = 0x01,          /* 0X01 */
  IPC_GPS_VERIFICATION_ENCODING_TYPE,               /* 0X02 */
  IPC_GPS_VERIFICATION_REQUESTOR_TYPE,              /* 0x03 */                    
  IPC_GPS_VERIFICATION_REQUESTOR_NAME,              /* 0x04 */
  IPC_GPS_VERIFICATION_CLIENT_TYPE,                 /* 0x05 */
  IPC_GPS_VERIFICATION_CLIENT_NAME,                 /* 0x06 */
} ipc_gps_verification_tag_e_type;

/* Notify type field */
typedef enum
{
  IPC_GPS_USER_NOTIFY_ONLY = 0x01,
  IPC_GPS_USER_NOTIFY_VERIFY_ALLOW_NO_RESP = 0x02,
  IPC_GPS_USER_NOTIFY_VERIFY_NOT_ALLOW_NO_RESP = 0x03,
} ipc_gps_notify_verify_e_type;

/* Ecoding type field */
typedef enum
{
  IPC_GPS_ECODIG_TYPE_UCS2 = 0x00,
  IPC_GPS_ECODIG_TYPE_GSM_DEFAULT = 0x01,
  IPC_GPS_ECODIG_TYPE_UTF8 = 0x02,
} ipc_gps_encoding_type_e_type;

/* Requestor type field */
typedef enum
{
  IPC_GPS_REQUESTOR_TYPE_LOGICAL_NAME = 0x00,
  IPC_GPS_REQUESTOR_TYPE_EMAIL = 0x01,
  IPC_GPS_REQUESTOR_TYPE_MSISDN = 0x02,
  IPC_GPS_REQUESTOR_TYPE_URL = 0x03,
  IPC_GPS_REQUESTOR_TYPE_SIP_URL = 0x04,
  IPC_GPS_REQUESTOR_TYPE_MIN = 0x05,
  IPC_GPS_REQUESTOR_TYPE_MDN = 0x06,
} ipc_gps_requestor_type_e_type;

/* Requestor type field */
typedef enum
{
  IPC_GPS_CLIENT_TYPE_LOGICAL_NAME = 0x00,
  IPC_GPS_CLIENT_TYPE_EMAIL = 0x01,
  IPC_GPS_CLIENT_TYPE_MSISDN = 0x02,
  IPC_GPS_CLIENT_TYPE_URL = 0x03,
  IPC_GPS_CLIENT_TYPE_SIP_URL = 0x04,
  IPC_GPS_CLIENT_TYPE_MIN = 0x05,
  IPC_GPS_CLIENT_TYPE_MDN = 0x06,
} ipc_gps_client_type_e_type;


/*====================================================================================

   SUB_CMD(1) : IPC_GPS_DISPLAY_SUPLFLOW      0x66 GPS Display SUPL Session Flow
 
======================================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET                0x02     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_RESP              0x02     */
/*    IPC_CMD_NOTI               0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_INDI                0x01     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION : GPS Display SUPL Session Flow NOTIFICATION
 
 FORMAT :
-----------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MESSAGE(1) | STATUS_CODE(1) |
-----------------------------------------------------------------------
*/ 

/* Message field */
typedef enum
{
  IPC_GPS_DISPLAY_SUPLFLOW_SUPL_INIT = 0x01,                   /* 0x01 */
  IPC_GPS_DISPLAY_SUPLFLOW_SUPL_START = 0x02,                  /* 0x02 */
  IPC_GPS_DISPLAY_SUPLFLOW_SUPL_RESPONSE = 0x03,               /* 0x03 */
  IPC_GPS_DISPLAY_SUPLFLOW_SUPL_POS_INIT = 0x04,               /* 0x04 */
  IPC_GPS_DISPLAY_SUPLFLOW_SUPL_POS = 0x05,                    /* 0x05 */
  IPC_GPS_DISPLAY_SUPLFLOW_SUPL_END = 0x06,                    /* 0x06 */
  IPC_GPS_DISPLAY_SUPLFLOW_SUPL_AUTH_REQ = 0x07,               /* 0x07 */
  IPC_GPS_DISPLAY_SUPLFLOW_SUPL_AUTH_RES = 0x08,               /* 0x08 */
  IPC_GPS_DISPLAY_SUPLFLOW_SUPL_TLS_FAILURE = 0x09,               /* 0x09 */
  IPC_GPS_DISPLAY_SUPLFLOW_SUPL_FALLBACK_SGPS = 0x0A,               /* 0x0A */
} ipc_gps_display_suplflow_e_type;

/* Status code field */
typedef enum
{
  IPC_GPS_DISPLAY_SUPLFLOW_STATUS_UNSPECIFIED = 0x01,                   /* 0x01 */
  IPC_GPS_DISPLAY_SUPLFLOW_STATUS_SYSTEM_FAILURE = 0x02,                  /* 0x02 */
  IPC_GPS_DISPLAY_SUPLFLOW_STATUS_PROTOCOL_ERROR = 0x03,               /* 0x03 */
  IPC_GPS_DISPLAY_SUPLFLOW_STATUS_DATA_MISSING = 0x04,               /* 0x04 */
  IPC_GPS_DISPLAY_SUPLFLOW_STATUS_UNEXPECTED_DATA_VALUE = 0x05,                    /* 0x05 */
  IPC_GPS_DISPLAY_SUPLFLOW_STATUS_POS_METHOD_FAILURE = 0x06,                    /* 0x06 */
  IPC_GPS_DISPLAY_SUPLFLOW_STATUS_POS_METHOD_MISMATCH = 0x07,               /* 0x07 */
  IPC_GPS_DISPLAY_SUPLFLOW_STATUS_POS_PROTOCOL_MISMATCH = 0x08,               /* 0x08 */
  IPC_GPS_DISPLAY_SUPLFLOW_STATUS_TARGET_SET_NOT_REACHABLE = 0x09,               /* 0x09 */
  IPC_GPS_DISPLAY_SUPLFLOW_STATUS_VERSION_NOT_SUPPORTED = 0x0A,               /* 0x0A */
  IPC_GPS_DISPLAY_SUPLFLOW_STATUS_RESOURCE_SHORTAGE = 0x0B,               /* 0x0B */
  IPC_GPS_DISPLAY_SUPLFLOW_STATUS_INVALID_SESSION_ID = 0x0C,               /* 0x0C */
  IPC_GPS_DISPLAY_SUPLFLOW_STATUS_NON_PROXY_MODE_NOT_SUPPORTED = 0x0D,               /* 0x0D */
  IPC_GPS_DISPLAY_SUPLFLOW_STATUS_PROXY_MODE_NOT_SUPPORTED = 0x0E,               /* 0x0E */
  IPC_GPS_DISPLAY_SUPLFLOW_STATUS_POSITIONING_NOT_PERMITTED = 0x0F,               /* 0x0F */
  IPC_GPS_DISPLAY_SUPLFLOW_STATUS_AUTH_NET_FAILURE = 0x10,               /* 0x10 */
  IPC_GPS_DISPLAY_SUPLFLOW_STATUS_AUTH_SULINIT_FAILURE = 0x11,               /* 0x11 */
  IPC_GPS_DISPLAY_SUPLFLOW_STATUS_CONSENT_DENINED_BY_USER = 0x64,               /* 0x64 */
  IPC_GPS_DISPLAY_SUPLFLOW_STATUS_CONSENT_GRANTED_BY_USER = 0x65,               /* 0x65 */
  IPC_GPS_DISPLAY_SUPLFLOW_STATUS_NONE_ERROR = 0xFF,               /* 0xFF */
} ipc_gps_status_code_e_type;



/*********************************************************************************

              Sub Command of IPC_SAP_CMD[0x12]

**********************************************************************************/
typedef enum{
  IPC_SAP_CONNECT=0x01,                 /* 0x01 : SAP Connect Message */
  IPC_SAP_STATUS,                       /* 0x02 : SAP Status Message */
  IPC_SAP_TRANSFER_ATR,                 /* 0x03 : SAP Transfer ATR Message */  
  IPC_SAP_TRANSFER_APDU,                /* 0x04 : SAP Transfer APDU Message */
  IPC_SAP_TRANSPORT_PROTOCOL,           /* 0x05 : SAP Transport Protocol Message */  
  IPC_SAP_SIM_POWER,                    /* 0x06 : SAP SIM Power Message */
  IPC_SAP_TRANSFER_CARD_READER_STATUS,  /* 0x07 : SAP Transfer Card Reader Status Message */
  IPC_SAP_MAX
} ipc_sap_sub_cmd_type;
/*********************************************************************************/


/*=================================================================

   SUB_CMD(1) : IPC_SAP_CONNECT      0x01 SAP Connect Message

=================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_SET               0x03     */
/*    IPC_CMD_CFRM              0x04     */
/*    IPC_CMD_EVENT             0x05     */
/*    IPC_CMD_INDI              0x01     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION : SAP Connect Message GET
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MSG_ID(1) | MAX_MSG_SIZE(2)
-------------------------------------------------------------------------------------
*/

/* MSG_ID field */
/* See ipc_sap_msg_id_e_type */
/* only IPC_SAP_MSG_ID_CONNECT_REQ, IPC_SAP_MSG_ID_DISCONNECT_REQ can be used */

typedef enum
{
  IPC_SAP_MSG_ID_CONNECT_REQ=0x00,
  IPC_SAP_MSG_ID_CONNECT_RESP, /* 1 */
  IPC_SAP_MSG_ID_DISCONNECT_REQ,
  IPC_SAP_MSG_ID_DISCONNECT_RESP,
  IPC_SAP_MSG_ID_DISCONNECT_IND,
  IPC_SAP_MSG_ID_TRANSFER_APDU_REQ,
  IPC_SAP_MSG_ID_TRANSFER_APDU_RESP,
  IPC_SAP_MSG_ID_TRANSFER_ATR_REQ,
  IPC_SAP_MSG_ID_TRANSFER_ATR_RESP,
  IPC_SAP_MSG_ID_POWER_SIM_OFF_REQ, /* 9 */
  IPC_SAP_MSG_ID_POWER_SIM_OFF_RESP, /* a */
  IPC_SAP_MSG_ID_POWER_SIM_ON_REQ, /* b */
  IPC_SAP_MSG_ID_POWER_SIM_ON_RESP, /* c */
  IPC_SAP_MSG_ID_RESET_SIM_REQ, /* d */
  IPC_SAP_MSG_ID_RESET_SIM_RESP,
  IPC_SAP_MSG_ID_TRANSFER_CARD_READER_STATUS_REQ,
  IPC_SAP_MSG_ID_TRANSFER_CARD_READER_STATUS_RESP,
  IPC_SAP_MSG_ID_STATUS_IND,
  IPC_SAP_MSG_ID_ERROR_RESP,
  IPC_SAP_MSG_ID_SET_TRANSPORT_PROTOCOL_REQ,
  IPC_SAP_MSG_ID_SET_TRANSPORT_PROTOCOL_RESP,
  IPC_SAP_MSG_ID_MAX
} ipc_sap_msg_id_e_type;

/* MAX_MSG_SIZE field */
/* Max message size*/


/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION : SAP Connect Message RESP
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MSG_ID(1) | CONNECTION_STATUS(1) | MAX_MSG_SIZE(2)
-------------------------------------------------------------------------------------
*/

/* MSG_ID field */
/* See ipc_sap_msg_id_e_type */
/* only IPC_SAP_MSG_ID_CONNECT_RESP, IPC_SAP_MSG_ID_DISCONNECT_RESP can be used */

/* CONNECTION_STATUS field */
typedef enum
{
  IPC_SAP_CONNECTION_STATUS_OK=0x00,
  IPC_SAP_CONNECTION_STATUS_UNABLE_TO_ESTABLISH,
  IPC_SAP_CONNECTION_STATUS_NOT_SUPPORT_MAX_SIZE,
  IPC_SAP_CONNECTION_STATUS_TOO_SMALL_MAX_SIZE,
  IPC_SAP_CONNECTION_STATUS_MAX
} ipc_sap_connection_status_e_type;

/* MAX_MSG_SIZE field */
/* Max message size*/


/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION : SAP Status Message NOTI
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | DISCONNECT_TYPE(1)
-------------------------------------------------------------------------------------
*/

/* DISCONNECT_TYPE field */
typedef enum
{
  IPC_SAP_DISCONNECT_TYPE_GRACEFUL=0x00,
  IPC_SAP_DISCONNECT_TYPE_IMMEDIATE,
  IPC_SAP_DISCONNECT_TYPE_MAX
} ipc_sap_disconnect_type_e_type;



/*=================================================================

   SUB_CMD(1) : IPC_SAP_STATUS      0x02 SAP Status Message

=================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_SET               0x03     */
/*    IPC_CMD_CFRM              0x04     */
/*    IPC_CMD_EVENT             0x05     */
/*    IPC_CMD_INDI              0x01     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION : SAP Status Message GET
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1)
-------------------------------------------------------------------------------------
*/


/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION : SAP Status Message RESP
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | SAP_STATUS(1)
-------------------------------------------------------------------------------------
*/

/* SAP_STATUS field */
typedef enum
{
  IPC_SAP_STATUS_UNKNOWN=0x00,
  IPC_SAP_STATUS_NO_SIM,
  IPC_SAP_STATUS_NOT_READY,
  IPC_SAP_STATUS_READY,
  IPC_SAP_STATUS_CONNECTED,
  IPC_SAP_STATUS_MAX
} ipc_sap_status_e_type;


/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION : SAP Status Message NOTI
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CARD_STATUS(1)
-------------------------------------------------------------------------------------
*/

/* CARD_STATUS field */
typedef enum
{
  IPC_SAP_CARD_STATUS_UNKNOWN=0x00,
  IPC_SAP_CARD_STATUS_RESET,
  IPC_SAP_CARD_STATUS_NOT_ACCESSIBLE,
  IPC_SAP_CARD_STATUS_REMOVED,
  IPC_SAP_CARD_STATUS_INSERTED,
  IPC_SAP_CARD_STATUS_RECOVERED,
  IPC_SAP_CARD_STATUS_MAX
} ipc_sap_card_status_e_type;



/*=================================================================

   SUB_CMD(1) : IPC_SAP_TRANSFER_ATR      0x03 SAP Transfer ATR Message

=================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_SET               0x03     */
/*    IPC_CMD_CFRM              0x04     */
/*    IPC_CMD_EVENT             0x05     */
/*    IPC_CMD_INDI              0x01     */
/*    IPC_CMD_NOTI              0x03     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION : SAP Transfer ATR Message GET
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1)
-------------------------------------------------------------------------------------
*/


/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION : SAP Transfer ATR Message RESP
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | RESULT_CODE(1) | ATR_LEN(2) | ATR(x)
-------------------------------------------------------------------------------------
*/

/* RESULT_CODE field */
typedef enum
{
  IPC_SAP_RESULT_CODE_OK=0x00,
  IPC_SAP_RESULT_CODE_NO_REASON,
  IPC_SAP_RESULT_CODE_CARD_NOT_ACCESSIBLE,
  IPC_SAP_RESULT_CODE_CARD_ALREADY_POWER_OFF,
  IPC_SAP_RESULT_CODE_CARD_REMOVED,
  IPC_SAP_RESULT_CODE_CARD_ALREADY_POWER_ON,
  IPC_SAP_RESULT_CODE_DATA_NOT_AVAILABLE,
  IPC_SAP_RESULT_CODE_NOT_SUPPORT,
  IPC_SAP_RESULT_CODE_MAX
} ipc_sap_result_code_e_type;

/* ATR_LEN field */
/* ATR Length */

/* ATR field */



/*=================================================================

   SUB_CMD(1) : IPC_SAP_TRANSFER_APDU      0x04 SAP Transfer APDU Message

=================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_SET               0x03     */
/*    IPC_CMD_CFRM              0x04     */
/*    IPC_CMD_EVENT             0x05     */
/*    IPC_CMD_INDI              0x01     */
/*    IPC_CMD_NOTI              0x03     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION : SAP Transfer APDU Message GET
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | APDU_LEN(2) | APDU(x)
-------------------------------------------------------------------------------------
*/

/* APDU_LEN field */
/* APDU Length */

/* APDU field */


/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION : SAP Transfer APDU Message RESP
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | RESULT_CODE(1) | APDU_LEN(1) | APDU(x)
-------------------------------------------------------------------------------------
*/

/* RESULT_CODE field */
/* See ipc_sap_result_code_e_type */

/* APDU_LEN field */
/* APDU Length */

/* APDU field */


/*=================================================================

   SUB_CMD(1) : IPC_SAP_TRANSPORT_PROTOCOL      0x05 SAP Transport Protocol Message

=================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_SET               0x03     */
/*    IPC_CMD_CFRM              0x04     */
/*    IPC_CMD_EVENT             0x05     */
/*    IPC_CMD_INDI              0x01     */
/*    IPC_CMD_NOTI              0x03     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION : SAP Transport Protocol Message GET
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | TRANSFER_PROTOCOL(1)
-------------------------------------------------------------------------------------
*/

/* TRANSFER_PROTOCOL field */
typedef enum
{
  IPC_SAP_TRANSFER_PROTOCOL_T0=0x00,
  IPC_SAP_TRANSFER_PROTOCOL_T1,
  IPC_SAP_TRANSFER_PROTOCOL_MAX
} ipc_sap_transfer_protocol_e_type;


/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION : SAP Transport Protocol Message RESP
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | RESULT_CODE(1)
-------------------------------------------------------------------------------------
*/

/* RESULT_CODE field */
/* See ipc_sap_result_code_e_type */



/*=================================================================

   SUB_CMD(1) : IPC_SAP_SIM_POWER      0x06 SAP SIM Power Message

=================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_SET               0x03     */
/*    IPC_CMD_CFRM              0x04     */
/*    IPC_CMD_EVENT             0x05     */
/*    IPC_CMD_INDI              0x01     */
/*    IPC_CMD_NOTI              0x03     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION : SAP SIM Power Message GET
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MSG_ID(1)
-------------------------------------------------------------------------------------
*/

/* MSG_ID field */
/* See ipc_sap_msg_id_e_type */
/* only IPC_SAP_MSG_ID_POWER_SIM_OFF_REQ, IPC_SAP_MSG_ID_POWER_SIM_ON_REQ, 
   IPC_SAP_MSG_ID_RESET_SIM_REQ can be used */


/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION : SAP SIM Power Message RESP
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MSG_ID(1) | RESULT_CODE(1)
-------------------------------------------------------------------------------------
*/

/* MSG_ID field */
/* See ipc_sap_msg_id_e_type */
/* only IPC_SAP_MSG_ID_POWER_SIM_OFF_RESP, IPC_SAP_MSG_ID_POWER_SIM_ON_RESP, 
   IPC_SAP_MSG_ID_RESET_SIM_RESP can be used */

/* RESULT_CODE field */
/* See ipc_sap_result_code_e_type */



/*=================================================================

   SUB_CMD(1) : IPC_SAP_TRANSFER_CARD_READER_STATUS      0x07 SAP Transfer Card Reader Status Message

=================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_SET               0x03     */
/*    IPC_CMD_CFRM              0x04     */
/*    IPC_CMD_EVENT             0x05     */
/*    IPC_CMD_INDI              0x01     */
/*    IPC_CMD_NOTI              0x03     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION : SAP Transfer Card Reader Status Message GET
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1)
-------------------------------------------------------------------------------------
*/



/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION : SAP Transfer Card Reader Status Message RESP
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | RESULT_CODE(1) | CARD_READER_STATUS(1)
-------------------------------------------------------------------------------------
*/

/* RESULT_CODE field */
/* See ipc_sap_result_code_e_type */

/* CARD_READER_STATUS field */
/* Refer to GSM11.14, Section12.33 */



/*=================================================================*/


/*********************************************************************************

                                            Sub Command of IPC_FACTORY_CMD[0x13]

**********************************************************************************/
typedef enum{
	IPC_FACTORY_DEVICE_TEST = 0x01,        /* 0x01 : FACTORY DEVICE TEST */
	IPC_FACTORY_OMISSION_AVOIDANCE_TEST,        /* 0x02 : FACTORYOMISSION AVOIDANCE TEST */
	IPC_FACTORY_DFT_TEST,       		 /* 0x03 : FACTORY DFT TEST */
	IPC_FACTORY_MISCELLANEOUS_TEST,        /* 0x04 : FACTORY MISCELLANEOUS TEST */		
	IPC_FACTORY_RETURN_AT_PORT_NOTIFICATION,			/* 0x05 : RETURN AT PORT */
	IPC_FACTORY_SLEEP_MODE_CMD,			/* 0x06 : ENTER SLEEP MODE */
	IPC_FACTORY_SET_TEST_NV_CMD,			/* 0x07 : SET NV ITEM */
	IPC_FACTORY_CAL_INFO_NOTIFICATION,    /* 0x08 : calibration information band2, band4 */
	IPC_FACTORY_MAX
} ipc_factory_sub_cmd_type; 

/*********************************************************************************/


/*=================================================================

  SUB_CMD(1) :    IPC_FACTORY_AUTO_TEST_CDMA         0x01     Auto Test for CDMA Message 

=================================================================*/
/*    IPC_CMD_EXEC                  0x01     */
/*    IPC_CMD_GET                   0x02     */
/*    IPC_CMD_SET                   0x03     */
/*    IPC_CMD_EVENT                 0x05     */
/*    IPC_CMD_RESP                  0x02     */
/*    IPC_CMD_NOTI                  0x03     */
//none



/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION : Auto Test for CDMA Message SET 
 
 FORMAT :
-----------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MODE(1) | ACTION(1) |
-----------------------------------------------------------------------------------------
*/

/* MODE field */
typedef enum {
    IPC_FACTORY_AUTO_CDMA_MODE_AUDIO_LOOPBACK_RECEIVER = 0x01,
    IPC_FACTORY_AUTO_CDMA_MODE_AUDIO_LOOPBACK_LOUDSPEAKER,
    IPC_FACTORY_AUTO_CDMA_MODE_AUDIO_LOOPBACK_EARPHONE,
    IPC_FACTORY_AUTO_CDMA_MODE_SET_MAX
} ipc_factory_auto_cdma_mode_set_type;

/* ACTION field */
typedef enum {
    IPC_FACTORY_AUTO_CDMA_ACTION_STOP = 0x00,
    IPC_FACTORY_AUTO_CDMA_ACTION_START,
    IPC_FACTORY_AUTO_CDMA_ACTION_SET_MAX
} ipc_factory_auto_cdma_action_set_type;


/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_CFRM                     0x04

 DESCRIPTION : Auto Test for CDMA Message CFRM
 
 FORMAT :
-----------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | RESULT(1) |
-----------------------------------------------------------------------------------------
*/
/* RESULT field */
/* For IPC_FACTORY_AUTO_CDMA_RESULT_TEST, 
When the front six digits of MIN value are 000000 indeed operation when MIN value is 000000wxyz */
typedef enum {
    IPC_FACTORY_AUTO_CDMA_RESULT_FAIL      = 0x00, /* 0x00 : Auto Test Fail */
    IPC_FACTORY_AUTO_CDMA_RESULT_SUCCESS,          /* 0x01 : Auto Test Success */
    IPC_FACTORY_AUTO_CDMA_RESULT_CLEAN_BOOT,       /* 0x02 : Send Second ACK about completion check after Factory Reset */
    IPC_FACTORY_AUTO_CDMA_RESULT_TEST,             /* 0x03 : Send ACK signal for normality boot */
    IPC_FACTORY_AUTO_CDMA_RESULT_MAX
} ipc_factory_auto_cdma_result_e_type;

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_INDI                     0x01

 DESCRIPTION : Auto Test for CDMA Message INDI 
 
 FORMAT :
-----------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MODE(1) | ACTION(1) |
-----------------------------------------------------------------------------------------
*/
/* MODE field */
typedef enum {
    IPC_FACTORY_AUTO_CDMA_MODE_MP3_TEST       = 0x01,   /* 0x01 : MP3 Play Test */
    IPC_FACTORY_AUTO_CDMA_MODE_WLAN_TEST,               /* 0x02 : WLAN Test to execute Test Application of PDA */
    IPC_FACTORY_AUTO_CDMA_MODE_CAMERA_TEST,             /* 0x03 : Camera Automation Test */
    IPC_FACTORY_AUTO_CDMA_MODE_EARPHONE_PATH_TEST,      /* 0x04 : Earphone path Automation Test */
    IPC_FACTORY_AUTO_CDMA_MODE_VIBRATOR_TEST,           /* 0x05 : Vibrator On/Off Test */
    IPC_FACTORY_AUTO_CDMA_MODE_SD_CHECK,                /* 0x06 : Test about SD Cards whether or not */
    IPC_FACTORY_AUTO_CDMA_MODE_RESET_VERIFY,            /* 0x07 : Factory Reset Verify */      
    IPC_FACTORY_AUTO_CDMA_MODE_SLEEP_TEST,              /* 0x08 : Sleep Current Test */
    IPC_FACTORY_AUTO_CDMA_MODE_USB_PATH_TEST,		/* 0x09 : USB Path Test */
    IPC_FACTORY_AUTO_CDMA_MODE_BACKLIGHT_TEST,       /*0x10: Backlight Tets*/
    IPC_FACTORY_AUTO_CDMA_MODE_MAX
} ipc_factory_auto_cdma_mode_e_type;

/* ACTION field */
typedef enum {
    IPC_FACTORY_AUTO_CDMA_ACTION_ON = 0x01,                         /* 0x01  : Start Autotest */
    IPC_FACTORY_AUTO_CDMA_ACTION_OFF ,                              /* 0x02  : Stop Autotest */
    IPC_FACTORY_AUTO_CDMA_ACTION_MP3_ON_MEM ,                       /* 0x03  : Execute MP3 from SD Card  */
    IPC_FACTORY_AUTO_CDMA_ACTION_CAMERA_NORMAL ,                    /* 0x04  : Normal Shoot Mode */ 
    IPC_FACTORY_AUTO_CDMA_ACTION_CAMERA_MACRO ,                     /* 0x05  : Macro Shoot Mode */
    IPC_FACTORY_AUTO_CDMA_ACTION_CAMERA_SUPER_MACRO ,               /* 0x06  : Super Macro Shoot Mode */
    IPC_FACTORY_AUTO_CDMA_ACTION_CAMERA_ZOOM_WIDE ,                 /* 0x07  : Zoom Wide Mode */
    IPC_FACTORY_AUTO_CDMA_ACTION_CAMERA_ZOOM_TELE1 ,                /* 0x08  : Zoom 1 Step */
    IPC_FACTORY_AUTO_CDMA_ACTION_CAMERA_ZOOM_TELE2 ,                /* 0x09  : Zoom 2 Step */
    IPC_FACTORY_AUTO_CDMA_ACTION_CAMERA_ZOOM_TELE3 ,                /* 0x0a  : Zoom 3 Step */
    IPC_FACTORY_AUTO_CDMA_ACTION_CAMERA_ZOOM_TELE4 ,                /* 0x0b  : Zoom 4 Step */
    IPC_FACTORY_AUTO_CDMA_ACTION_CAMERA_ZOOM_TELE5 ,                /* 0x0c  : Zoom 5 Step */
    IPC_FACTORY_AUTO_CDMA_ACTION_CAMERA_ZOOM_TELE6 ,                /* 0x0d  : Zoom 6 Step */
    IPC_FACTORY_AUTO_CDMA_ACTION_CAMERA_ZOOM_TELE7 ,                /* 0x0e  : Zoom 7 Step */
    IPC_FACTORY_AUTO_CDMA_ACTION_CAMERA_ZOOM_TELE8 ,                /* 0x0f  : Zoom 8 Step */
    IPC_FACTORY_AUTO_CDMA_ACTION_CAMERA_ZOOM_TELE9 ,                /* 0x10  : Zoom 9 Step */
    IPC_FACTORY_AUTO_CDMA_ACTION_CAMERA_ZOOM_TELE10 ,               /* 0x11  : Zoom 10 Step */
    IPC_FACTORY_AUTO_CDMA_ACTION_CAMERA_SHOOT_HALF_SHUTTER ,        /* 0x12  : Half Shutter Shoot Mode */
    IPC_FACTORY_AUTO_CDMA_ACTION_CAMERA_SHOOT_NORMAL ,              /* 0x13  : Normal Shoot Mode */
    IPC_FACTORY_AUTO_CDMA_ACTION_CAMERA_SHOOT_BLACK ,               /* 0x14  : Black Shoot Mode */
    IPC_FACTORY_AUTO_CDMA_ACTION_CAMERA_INNER_MEM ,                 /* 0x15  : Save Internal Memory */
    IPC_FACTORY_AUTO_CDMA_ACTION_CAMERA_EXT_MEM ,                   /* 0x16  : Save External Memory */
    IPC_FACTORY_AUTO_CDMA_ACTION_CAMERA_PORTABLE_OFF ,              /* 0x17  : Portable Disk Mode Off */
    IPC_FACTORY_AUTO_CDMA_ACTION_CAMERA_PORTABLE_EXT_ON ,           /* 0x18  : Portable External Disk On */
    IPC_FACTORY_AUTO_CDMA_ACTION_CAMERA_PORTABLE_INNER_ON ,         /* 0x19  : Portable Inner Disk On */
    IPC_FACTORY_AUTO_CDMA_ACTION_EAR_SEND_END_DETECT ,              /* 0x1a  : Earphone Send/End Detect */
    IPC_FACTORY_AUTO_CDMA_ACTION_AUDIO_LOOPBACK_ON ,                /* 0x1b  : Audio Loopback On */
    IPC_FACTORY_AUTO_CDMA_ACTION_AUDIO_LOOPBACK_OFF ,               /* 0x1c  : Audio Loopback Off */
    IPC_FACTORY_AUTO_CDMA_ACTION_RESET_VERIFY_CALLLOG ,             /* 0x1d  : Factory Reset Verify for Call log */
    IPC_FACTORY_AUTO_CDMA_ACTION_RESET_VERIFY_MEM ,                 /* 0x1e  : Factory Reset Verify for Memory */
    IPC_FACTORY_AUTO_CDMA_ACTION_RESET_VERIFY_MSG ,                 /* 0x1f  : Factory Reset Verify for Message */
    IPC_FACTORY_AUTO_CDMA_ACTION_RESET_VERIFY_PB ,                  /* 0x20  : Factory Reset Verify for Phonebook */
    IPC_FACTORY_AUTO_CDMA_ACTION_RESET_VERIFY_SCHEDULER ,           /* 0x21  : Factory Reset Verify for Scheduler */
    IPC_FACTORY_AUTO_CDMA_ACTION_RESET_VERIFY_ALL ,                 /* 0x22  : Factory Reset Verify for All Data */
    IPC_FACTORY_AUTO_CDMA_ACTION_SLEEP ,                            /* 0x23  : Sleep Current Mode On */
    IPC_FACTORY_AUTO_CDMA_ACTION_WAKEUP ,                           /* 0x24  : Sleep Current Mode Off */
    IPC_FACTORY_AUTO_CDMA_ACTION_CAMERA_FLASH_ON,		/* 0x25 : Camera Flash On */
    IPC_FACTORY_AUTO_CDMA_ACTION_CAMERA_FLASH_OFF,		/* 0x26 : Camera Flash Off*/
    IPC_FACTORY_AUTO_CDMA_ACTION_USB_PATH_PHONE,			/* 0x27 : USB Path Phone */
    IPC_FACTORY_AUTO_CDMA_ACTION_USB_PATH_PDA,				/* 0x28 : USB Path PDA */
    IPC_FACTORY_AUTO_CDMA_ACTION_LCD_ON,                   /*0x29 : Backlight test Lcd on*/
    IPC_FACTORY_AUTO_CDMA_ACTION_LCD_OFF,					/*0x2A : Backlight test Lcd off*/
    IPC_FACTORY_AUTO_CDMA_ACTION_CAMERA_ON_STATUS,		/*0x2B : Camera start Status for instinctq*/
    IPC_FACTORY_AUTO_CDMA_ACTION_MAX
} ipc_factory_auto_cdma_action_e_type;



/*=================================================================

  SUB_CMD(1) :    IPC_FACTORY_AUTO_TEST_GSM          0x02     Auto Test for GSM Message  

=================================================================*/
/*    IPC_CMD_EXEC                  0x01     */
/*    IPC_CMD_GET                   0x02     */
/*    IPC_CMD_EVENT                 0x05     */
/*    IPC_CMD_INDI                  0x01     */
/*    IPC_CMD_RESP                  0x02     */
//none
/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION : Auto Test for GSM Message SET 
 
 FORMAT :
-----------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MODE(1) | ACTION(1) |
-----------------------------------------------------------------------------------------
*/

/* MODE field */
typedef enum {
    IPC_FACTORY_AUTO_GSM_MODE_MP3_PLAY_FROM_SDCARD       = 0x01,
    IPC_FACTORY_AUTO_GSM_MODE_MP3_PLAY_FROM_DEVICE,
    IPC_FACTORY_AUTO_GSM_MODE_AUDIO_LOOPBACK_RECEIVER,
    IPC_FACTORY_AUTO_GSM_MODE_AUDIO_LOOPBACK_LOUDSPEAKER,
    IPC_FACTORY_AUTO_GSM_MODE_AUDIO_LOOPBACK_EARPHONE,
    IPC_FACTORY_AUTO_GSM_MODE_RF_MAX_POWER_GSM900,
    IPC_FACTORY_AUTO_GSM_MODE_RF_MAX_POWER_GSM1800,
    IPC_FACTORY_AUTO_GSM_MODE_RF_MAX_POWER_GSM1900,
    IPC_FACTORY_AUTO_GSM_MODE_RF_MAX_POWER_WCDMA2100,
    IPC_FACTORY_AUTO_GSM_MODE_MAX
} ipc_factory_auto_gsm_mode_e_type;

/* ACTION field */
typedef enum {
    IPC_FACTORY_AUTO_GSM_ACTION_STOP = 0x00,
    IPC_FACTORY_AUTO_GSM_ACTION_START,
    IPC_FACTORY_AUTO_GSM_ACTION_MAX
} ipc_factory_auto_gsm_action_e_type;

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_CFRM                     0x04

 DESCRIPTION : Auto Test for GSM Message CFRM 
 
 FORMAT :
-----------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | RESULT(1) |
-----------------------------------------------------------------------------------------
*/
/* RESULT field */
typedef enum {
    IPC_FACTORY_AUTO_GSM_AUDIO_LOOPBACK_EAR_SWITCH_OFF = 0x00,
    IPC_FACTORY_AUTO_GSM_AUDIO_LOOPBACK_EAR_SWITCH_ON,
    IPC_FACTORY_AUTO_GSM_AUDIO_LOOPBACK_EAR_SWITCH_MAX
} ipc_factory_auto_gsm_result_e_type;

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_INDI                     0x01

 DESCRIPTION : Auto Test for GSM Message INDI 
 
 FORMAT :
-----------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-----------------------------------------------------------------------------------------
*/

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION : Auto Test for GSM Message NOTI 
 
 FORMAT :
-----------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MODE(1) | ACTION(1) |
-----------------------------------------------------------------------------------------
*/
/* same as SET type */



/*=================================================================

  SUB_CMD(1) :    IPC_FACTORY_PDA_FORCE_SLEEP        0x03     PDA Force Sleep Mode Message 

=================================================================*/
/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET               0x02     */
/*    IPC_CMD_SET               0x03     */
/*    IPC_CMD_EVENT             0x05     */
/*	  IPC_CMD_RESP              0x02	 */
/* none */

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_CFRM                     0x04

 DESCRIPTION :
   Confirm Linemode on or off
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | STATUS(1)
-------------------------------------------------------------------------------------
*/
typedef enum{
  IPC_FACTORY_LINEMODE_OFF   =0x00,  	/* 0x00 : Normal Mode */
  IPC_FACTORY_LINEMODE_ON,				/* 0x01 : Linemode On : PDA Sleep Mode */
  IPC_FACTORY_LINEMODE_MAX
} ipc_factory_linemode_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_INDI                     0x01

 DESCRIPTION :
   Indication Linemode on or off
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
-------------------------------------------------------------------------------------
*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
   Notification Linemode on or off
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | STATUS(1) |
-------------------------------------------------------------------------------------
*/

/*=================================================================

  SUB_CMD(1) :    IPC_FACTORY_BT_TEST               0x04     BT Factory Test Message 

=================================================================*/
/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET               0x02     */
/*    IPC_CMD_SET               0x03     */
/*    IPC_CMD_EVENT             0x05     */
/*   IPC_CMD_RESP               0x02     */
/* none */
/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_CFRM                     0x04
 DESCRIPTION :
   Confirm BT Test
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | RESULT(1)
-------------------------------------------------------------------------------------
*/
/* RESULT field */
typedef enum {
    IPC_FACTORY_BT_STATUS_CHECK_FAIL      = 0x00,         /* 0x00 : Status Check Fail */
    IPC_FACTORY_BT_STATUS_CHECK_OK,                       /* 0x01 : Status Check OK */
    IPC_FACTORY_BT_RESULT_MAX
} ipc_factory_bt_result_e_type; 

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_INDI                     0x01
 DESCRIPTION :
   Indicate BT Test 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1)  | TEST_TYPE(1)
-------------------------------------------------------------------------------------
*/
/* TEST_TYPE field */
typedef enum {
    IPC_FACTORY_BT_ON_OFF_CHECK = 0x00,       /* 0x00 : BT MODULE CHECK CMD */ //gudam
    IPC_FACTORY_BT_SEARCH,                  /* 0x01 : BT SEARCH CMD */
    IPC_FACTORY_BT_DEACTIVATION,	        /* 0x02 : BT DEACTIVATION CMD */
    IPC_FACTORY_BT_WRITE_ADDRESS,	        /* 0x03 : BT WRITE ADDR CMD */ //gudam    
    IPC_FACTORY_BT_ACTIVATION,           /* 0x04 : BT ACTIVATION CMD */ //gudam
    IPC_FACTORY_BT_TEST_TYPE_MAX
} ipc_factory_bt_test_type_e_type;
 
/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03
 DESCRIPTION :
   Notification BT Test 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1)  | TEST_TYPE(1)
-------------------------------------------------------------------------------------
*/
/* TEST_TYPE field */
/*
See ipc_factory_bt_test_type_e_type
*/
/*=================================================================

  SUB_CMD(1) :    IPC_FACTORY_WIFI_TEST            0x05     Wi-fi Test Message 

=================================================================*/
/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET               0x02     */
/*    IPC_CMD_SET               0x03     */
/*    IPC_CMD_EVENT             0x05     */
/*	  IPC_CMD_RESP              0x02	 */
/* none */
/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_CFRM                     0x04

 DESCRIPTION :
   Trasfer Data from PDA to PC Program
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | LENGTH(1) |WIFI_DATA(13)
-------------------------------------------------------------------------------------
*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_INDI                     0x01

 DESCRIPTION :
   Trasfer Data from PC Program to PDA
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | LENGTH(1) | WIFI_DATA(16)
-------------------------------------------------------------------------------------
*/



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
   Notification  Execute WiFi app 
 FORMAT :
----------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MODE(1) | TEST_STATUS(1)
----------------------------------------------------------------------------------------
 | BT_MACADDRESS(6) | PIN_CODE(4)|
----------------------------------------------------------------------------------------
*/

/* MODE field */
/* ʵ ! 
     ϴ  , ش IPC BT  õ ڵ  Ƿ,
    Mode  ׳ Test_status  Ǿ ֽñ ٶϴ.
    ׸,  Ͻô е Test_status BT_macaddress, pin_code ؼ NULL ó ֽñ.
    PDA  Test_status BT_macaddress, pin_code   ּ.
        Ǹ ش WIFI START NOTI Field  
----------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | TEST_STATUS(1)
----------------------------------------------------------------------------------------
      ˴ϴ.  ,   ʵ  ۵Ǵ  ݵ MODE Field TEST_STATUS 
   ־ ֽñ ٶϴ. 2008.07.30

    */
typedef enum {
    IPC_FACTORY_TEST_BT_RF_TEST = 0x01,         /* 0x01 : BT RF Test */
    IPC_FACTORY_TEST_BT_AUDIO_TEST,             /* 0x02 : BT Audio Loopback Test */
    IPC_FACTORY_TEST_WIFI_TEST,                 /* 0x03 : WiFi Test */
    IPC_FACTORY_TEST_MAX
} ipc_factory_test_e_type;

/* TEST_STATUS field */
typedef enum {
    IPC_FACTORY_WIFI_TEST_START = 0x01,           /* 0x01 : WIFI Factory Test Start */
    IPC_FACTORY_WIFI_TEST_END,                    /* 0x02 : WIFI Factory Test End */
    IPC_FACTORY_WIFI_TEST_MAX
} ipc_factory_wifi_test_f_e_type;

//<<cyj_dc09
/* RESULT field */
typedef enum {
    IPC_FACTORY_WIFI_ON_OFF_STATUS_FAIL      = 0x00,         /* 0x00 : Status Check Fail */
    IPC_FACTORY_WIFI_ON_OFF_STATUS_OK,                       /* 0x01 : Status Check OK */
    IPC_FACTORY_WIFI_ON_OFF_STATUS_MAX
} ipc_factory_wifi_on_off_result_e_type; 

typedef enum {
    IPC_FACTORY_WIFI_ON = 0x00,           /* 0x00 : WIFI Factory on */
    IPC_FACTORY_WIFI_STATUS	,                /* 0x01 : WIFI Factory status */
    IPC_FACTORY_WIFI_OFF	,                /* 0x02 : WIFI Factory off */
    IPC_FACTORY_WIFI_ON_OFF_MAX
} ipc_factory_wifi_on_off_test_f_e_type;
//cyj_dc09>>

//<<cyj_dc24
/* RESULT field */
typedef enum {
    IPC_FACTORY_WIFI_MACADDR_STATUS_FAIL      = 0x00,         /* 0x00 : Status Check Fail */
    IPC_FACTORY_WIFI_MACADDR_STATUS_OK,                       /* 0x01 : Status Check OK */
    IPC_FACTORY_WIFI_MACADDR_STATUS_MAX
} ipc_factory_wifi_macaddr_result_e_type; 

typedef enum {
    IPC_FACTORY_WIFI_MACADDR_WRITE = 0x01,           /* 0x01 : WIFI Factory Write */
    IPC_FACTORY_WIFI_MACADDR_MAX
} ipc_factory_wifi_macaddr_test_f_e_type;
//cyj_dc24>>
/*=================================================================

  SUB_CMD(1) :    IPC_FACTORY_FM_RADIO_TEST            0x06     FM Radio Test Message              

=================================================================*/
/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET               0x02     */
/*    IPC_CMD_SET               0x03     */
/*    IPC_CMD_EVENT             0x05     */
/*	  IPC_CMD_RESP              0x02	 */
/*    IPC_CMD_NOTI              0x03     */
/* none */

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_CFRM                     0x04

 DESCRIPTION :
   Confirm FM Radio Test Start
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | STATUS(1)
-------------------------------------------------------------------------------------
*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_INDI                     0x01

 DESCRIPTION :
   Indicate FM Radio Test Start
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | STATUS(1)
-------------------------------------------------------------------------------------
 | FREQUENCY(2) | VOLUME(1) |OUTPUT_PATH(1)
-------------------------------------------------------------------------------------
*/
/* STATUS field */
typedef enum {
    IPC_FACTORY_FM_RADIO_STATUS_DISABLE      = 0x00,         /* 0x00 : disable FM Radio option */
    IPC_FACTORY_FM_RADIO_STATUS_ENABLE,                      /* 0x01 : enable FM Radio option */
    IPC_FACTORY_FM_RADIO_STATUS_MAX
} ipc_factory_fm_radio_status_e_type;

/* OUTPUT_PATH field */
typedef enum {
    IPC_FACTORY_FM_RADIO_OUTPUT_PATH_EARPHONE = 0x00,         /* 0x00 : Output Path EarPhone*/
    IPC_FACTORY_FM_RADIO_OUTPUT_PATH_SPEAKER,                 /* 0x01 : Output Path Speaker */
    IPC_FACTORY_FM_RADIO_OUTPUT_PATH_MAX
} ipc_factory_fm_radio_output_path_e_type;

/*=================================================================

  SUB_CMD(1) :    IPC_FACTORY_MEM_FORMAT_TEST           0x07     Memory Format Test Message

=================================================================*/
/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET               0x02     */
/*    IPC_CMD_SET               0x03     */
/*    IPC_CMD_EVENT             0x05     */
/*    IPC_CMD_RESP              0x02  */
/* none */
/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_CFRM                     0x04
 DESCRIPTION :
   Confirm Memory Format Test  "Result" or "Memory Size"
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | TEST_CASE(1) | RESULT(1)
-------------------------------------------------------------------------------------
*/
/* TEST_CASE field */
typedef enum {
   IPC_FACTORY_MEMORY_FORMAT_TEST_RESULT = 0x00,            /* 0x00 : Memory Format Test Result */
   IPC_FACTORY_MEMORY_FORMAT_SIZE,                          /* 0x01 : Memory Format Size*/
   IPC_FACTORY_MEMORY_FORMAT_STORAGE_CHECK,                 /* 0x02 : Storage Check*/
   IPC_FACTORY_MEMORY_FORMAT_MAX
} ipc_factory_memory_format_testcase_e_type; 

/* RESULT field */
typedef enum {
   IPC_FACTORY_MEMORY_FORMAT_ING = 0x00,            /* 0x00 : Memory Format isn't ended */
   IPC_FACTORY_MEMORY_FORMAT_COMPLETE,              /* 0x01 : Memory Format complete*/
   IPC_FACTORY_MEMORY_FORMAT_NO_STORAGE,            /* 0x02 : No Storage*/
   IPC_FACTORY_MEMORY_FORMAT_STORAGE_EXIST,         /* 0x03 : Storage exist */
   IPC_FACTORY_MEMORY_FORMAT_SIZE_8G,               /* 0x04 : Memory Format Size 8G*/
   IPC_FACTORY_MEMORY_FORMAT_SIZE_16G,              /* 0x05 : Memory Format Size 16G*/
   IPC_FACTORY_MEMORY_FORMAT_SIZE_32G,              /* 0x06 : Memory Format Size 32G*/
   IPC_FACTORY_MEMORY_FORMAT_RESULT_MAX
} ipc_factory_memory_format_result_e_type;  

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_IND                     0x01
 DESCRIPTION :
   Indicate Memory Format Test  "Result" or "Memory Size"
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | TEST_CASE(1)
-------------------------------------------------------------------------------------
*/
/* TEST_CASE field */
/*
See ipc_factory_memory_format_testcase_e_type
*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03
 DESCRIPTION :
   Notificate Memory Format Test Start or Stop
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1)  | STATUS(1)
-------------------------------------------------------------------------------------
*/
/* STATUS field */
typedef enum {
 IPC_FACTORY_MEMORY_FORMAT_TEST_ENDED      = 0x00,         /* 0x00 : Memory Format ENDED */
 IPC_FACTORY_MEMORY_FORMAT_TEST_START,                    /* 0x01 : Memory Format START */
 IPC_FACTORY_MEMORY_FORMAT_TEST_MAX
} ipc_factory_memory_format_test_status_e_type; 
  //none

/*=================================================================

  SUB_CMD(1) :    IPC_FACTORY_LINE_MODE_SMS_COUNT        0x08     Line SMS Count Message

=================================================================*/
/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET               0x02     */
/*    IPC_CMD_SET               0x03     */
/*    IPC_CMD_EVENT             0x05     */
/*    IPC_CMD_RESP              0x02     */
/*    IPC_CMD_NOTI		        0x03     */
/* none */
/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_CFRM                 0x04
 DESCRIPTION :
   Get the count of line mode SMS
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1)  | COUNT(1) |
-------------------------------------------------------------------------------------
*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_IND                  0x01
 DESCRIPTION :
   Get the count of line mode SMS
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1)  | 
-------------------------------------------------------------------------------------
*/



/*=================================================================

  SUB_CMD(1) :    IPC_FACTORY_LINE_MODE_SMS_READ               0x09     Line SMS Read Message

=================================================================*/
/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_GET               0x02     */
/*    IPC_CMD_SET               0x03     */
/*    IPC_CMD_EVENT             0x05     */
/*    IPC_CMD_RESP              0x02     */
/*    IPC_CMD_NOTI		        0x03     */
/* none */
/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_CFRM                 0x04
 DESCRIPTION :
   Get the count of line mode SMS
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1)  | ADDR_LENGTH(1) | ADDRESS(20) |
-------------------------------------------------------------------------------------
 | DATA_LENGTH(1) | DATA(256)
-------------------------------------------------------------------------------------
*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_IND                  0x01
 DESCRIPTION :
   Get the count of line mode SMS
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1)  | INDEX(1)
-------------------------------------------------------------------------------------
*/

/*=================================================================

  SUB_CMD(1) :    IPC_FACTORY_G_SENSOR_TEST            0x0A     G-SENSOR Test Message

=================================================================*/
/*    IPC_CMD_EXEC                  0x01     */
/*    IPC_CMD_GET                   0x02     */
/*    IPC_CMD_SET                   0x03     */
/*    IPC_CMD_EVENT                 0x05     */
/*    IPC_CMD_RESP                  0x02     */
/*    IPC_CMD_NOTI                  0x03     */
//none
/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_CFRM                     0x04

 DESCRIPTION :
   Confirm Message used to obtain coordinates value of the G-Sensor.
 FORMAT :
--------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | X- Value(4) | Y- Value(4) | Z- Value(4) |
--------------------------------------------------------------------------------------------
*/
/* Values should be written by DECIMAL via ASCII encoding including negative value */

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_INDI                     0x01

 DESCRIPTION : 

 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
-------------------------------------------------------------------------------------
*/


/*=================================================================

  SUB_CMD(1) :    IPC_FACTORY_MEM_SIZE_CHECK         0X0B     Memory Size Check Message

=================================================================*/
/*    IPC_CMD_EXEC                  0x01     */
/*    IPC_CMD_GET                   0x02     */
/*    IPC_CMD_SET                   0x03     */
/*    IPC_CMD_EVENT                 0x05     */
/*    IPC_CMD_RESP                  0x02     */
/*    IPC_CMD_NOTI                  0x03     */
/* none */

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_CFRM                     0x04

 DESCRIPTION :
   Confirm IMEI Tool Mass Storage Information Message
 FORMAT :
--------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MEMORY_TYPE(1) | TOTAL_SIZE(4) | USED_SIZE(4) |
--------------------------------------------------------------------------------------------
*/
/* MEMORY_TYPE field */
typedef enum {
    IPC_FACTORY_MEMORY_PDA_MAIN_MEMORY = 0x00,
    IPC_FACTORY_MEMORY_MASS_STORAGE,
    IPC_FACTORY_MEMORY_MAX,
} ipc_factory_mem_e_type;

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_INDI                     0x01

 DESCRIPTION :
   Indication IMEI Tool Mass Storage Information Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MEMORY_TYPE(1)
-------------------------------------------------------------------------------------
*/

/*=================================================================

  SUB_CMD(1) :    IPC_FACTORY_MEM_FILE_NUM_CHECK     0x0C     Memory File Number Check Message

=================================================================*/
/*    IPC_CMD_EXEC                  0x01     */
/*    IPC_CMD_GET                   0x02     */
/*    IPC_CMD_SET                   0x03     */
/*    IPC_CMD_EVENT                 0x05     */
/*    IPC_CMD_RESP                  0x02     */
/*    IPC_CMD_NOTI                  0x03     */
/* none */

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_CFRM                     0x04

 DESCRIPTION : Check how many files in memory CFRM
 
 FORMAT :
--------------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MEMORY_TYPE(1) | FILE_NUMBER(2) |
--------------------------------------------------------------------------------------------
*/
/* MEMORY_TYPE field */
/* see ipc_factory_mem_e_type */

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_INDI                     0x01

 DESCRIPTION : Check how many files in memory INDI

 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | MEMORY_TYPE(1) |
-------------------------------------------------------------------------------------
*/
/* MEMORY_TYPE field */
/* see ipc_factory_mem_e_type */

/*=================================================================

  SUB_CMD(1) :    IPC_FACTORY_DIAG_TOUCH_TEST        0x0D     Diagnostic Touch Test Message

=================================================================*/
/*    IPC_CMD_EXEC                  0x01     */
/*    IPC_CMD_GET                   0x02     */
/*    IPC_CMD_SET                   0x03     */
/*    IPC_CMD_CFRM                  0x04     */
/*    IPC_CMD_EVENT                 0x05     */
/*    IPC_CMD_INDI                  0x01     */
/*    IPC_CMD_RESP                  0x02     */
//none
/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
     Notification Diagnostic Key code Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | TOUCH_EVENT(2) |  
-------------------------------------------------------------------------------------
 | POS_X(2)    | POS_X(2)   | POS_X(2)    | STATUS(4)      |
-------------------------------------------------------------------------------------
*/
/* TOUCH_EVENT field */
/* Bypass the diag command */
typedef enum{
  IPC_FACTORY_TOUCH_CLICK_DOWN = 0x0f00,                    /* 0x0f00 : touch click down */
  IPC_FACTORY_TOUCH_MOVE  = 0x1000,  	    	            /* 0x1000 : touch move */  	
  IPC_FACTORY_TOUCH_CLICK_UP  = 0x1100,  	    	        /* 0x1100 : touch click up */  	
  IPC_FACTORY_TOUCH_HOLD  = 0x1200,  	    	            /* 0x1200:  touch hold */  	
  IPC_FACTORY_TOUCH_EVENT_MAX
} ipc_factory_touch_e_type;

/* POS_X, POS_Y fields */
/* Position of x, y on LCD screen, with deciman number(ASCII) */
/* POS_Z : reserved */

/* STATUS field */
/* reserved */


/*=================================================================

  SUB_CMD(1) :    IPC_FACTORY_LINE_MODE        0x0E : Line Mode Message 

=================================================================*/


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION :
     GET Factory Line Mode Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/



/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION :
     RESPONSE Factory Line Mode Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | LINE_MODE |  
-------------------------------------------------------------------------------------
*/

/* LINE_MODE */
typedef enum {
  IPC_FACTORY_LINE_MODE_OFF = 0x00,
  IPC_FACTORY_LINE_MODE_ON = 0x01,

  IPC_FACTORY_LINE_MODE_MAX
} ipc_factory_line_mode_e_type;


/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION :
     NOTIFICATION Factory Line Mode Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | LINE_MODE |  
-------------------------------------------------------------------------------------
*/

/* LINE_MODE */
// refer to ipc_factory_line_mode_e_type


/*=================================================================

	SUB_CMD(1) :		IPC_FACTORY_SHIPMENT_TEST 			 Magnetic Sensor Message 

=================================================================*/
/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_IND										 0x01
 
 DESCRIPTION :
	 Request to start factory shipment test
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | TEST_TYPE(1)
-------------------------------------------------------------------------------------
*/
 
/* TEST_TYPE field */
typedef enum {
		IPC_FACTORY_SHIPMENT_TEST_START 			= 0x00, 		 /* 0x00 : Test Start */
		IPC_FACTORY_SHIPMENT_TEST_ST_TMPS_CHECK,					 /* 0x01 : ST, TMP Check */
		IPC_FACTORY_SHIPMENT_TEST_DAC_CHECK,							 /* 0x02 : DAC Check  */
		IPC_FACTORY_SHIPMENT_TEST_ADC_CHECK,							 /* 0x03 : ADC Check  */
} ipc_factory_shipment_test_type_e_type;
 
 
/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_CFRM 										0x04
 
 DESCRIPTION :
	 Confirm factory shipment test result
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | TEST_TYPE(1) | RESULT_LENGTH(1) | 
RESULT(X)
-------------------------------------------------------------------------------------
*/

/*=================================================================

	SUB_CMD(1) :		IPC_FACTORY_PROCESS_TEST 			 Factory Process Test

=================================================================*/
/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET										 0x02
 
 DESCRIPTION :
	 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | READ_TYPE(1)
-------------------------------------------------------------------------------------
*/
 
/* READ_TYPE field */
typedef enum {
		IPC_FACTORY_PROCESS_TEST_RESULT 			= 0x00, 		 /* 0x00 : Test Result */
		IPC_FACTORY_PROCESS_TEST_HISTORY,					 		 /* 0x01 : Test History */
		IPC_FACTORY_PROCESS_TEST_MAX							 
} ipc_factory_process_test_type_e_type;
 
 
/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP 										0x02
 
 DESCRIPTION :
	 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | RESULT(X)
-------------------------------------------------------------------------------------
*/
/* RESULT field 
     Test Result - 40 byte array
     Test History - 120 byte array(ID+RESULT) */

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET										 0x03

 DESCRIPTION :

 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | TEST ID(1) | RESULT(1)
-------------------------------------------------------------------------------------*/


/*=================================================================

	SUB_CMD(1) :		IPC_FACTORY_AP_THERMISTOR 			 AP Thermistor Read Message 

=================================================================*/
/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_IND										 0x01
 
 DESCRIPTION :
	 Request to start AP Thermistor test
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
-------------------------------------------------------------------------------------
*/
 
/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_CFRM 										0x04
 
 DESCRIPTION :
	 Confirm AP Thermistor test result
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | DATA(2) 
-------------------------------------------------------------------------------------
*/	

/*=================================================================

	SUB_CMD(1) :		IPC_FACTORY_AP_BATT_CALIBRATION 			 AP Battery Calibration Message 

=================================================================*/
/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_IND										 0x01
 
 DESCRIPTION :
	 Request to start AP Battery Calibration test
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
-------------------------------------------------------------------------------------
*/
 
/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_CFRM 										0x04
 
 DESCRIPTION :
	 Confirm AP Battery Calibration test result
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | DATA(2) 
-------------------------------------------------------------------------------------
*/	

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI 										0x03

 DESCRIPTION :
		 NOTIFICATION AP Battery Calibration Message
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) 
-------------------------------------------------------------------------------------
*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET										 0x02
 
 DESCRIPTION :
	 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
-------------------------------------------------------------------------------------
*/
  
/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP 										0x02
 
 DESCRIPTION :
	 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  DATA(2)
-------------------------------------------------------------------------------------
*/


/*********************************************************************************

            Sub Command of IPC_OMADM_CMD[0x14]

**********************************************************************************/

typedef enum{
  IPC_OMADM_PRL_SIZE = 0x01,                    /* 0x01 : IPC_OMADM_PRL_SIZE  */
  IPC_OMADM_MODEL_NAME,                          /* 0x02 :IPC_OMADM_MODEL_NAME*/
  IPC_OMADM_OEM_NAME,                            /* 0x03 :IPC_OMADM_OEM_NAME */
  IPC_OMADM_SW_VER,                               /* 0x04 :IPC_OMADM_SW_VER */
  IPC_OMADM_IS683_DATA,                          /* 0x05 :IPC_OMADM_IS683_DATA */
  IPC_OMADM_PRL_READ,                             /* 0x06 :IPC_OMADM_PRL_READ */
  IPC_OMADM_PRL_WRITE,                            /* 0x07 : IPC_OMADM_PRL_WRITE*/
  IPC_OMADM_PUZL_DATA,                            /* 0x08 :IPC_OMADM_PUZL_DATA*/
  IPC_OMADM_ROOTCERT_READ,                       /* 0x09 : IPC_OMADM_ROOTCERT_READ */
  IPC_OMADM_ROOTCERT_WRITE,                      /* 0x0A :IPC_OMADM_ROOTCERT_WRITE */
  IPC_OMADM_MMC_OBJECT,                           /* 0x0B : IPC_OMADM_MMC_OBJECT*/
  IPC_OMADM_MIP_NAI_OBJECT,                      /* 0x0C : IPC_OMADM_MIP_NAI_OBJECT*/
  IPC_OMADM_CURRENT_NAI_INDEX,                   /* 0x0D : IPC_OMADM_CURRENT_NAI_INDEX*/
  IPC_OMADM_MIP_AUTH_ALGO,                        /* 0x0E : IPC_OMADM_MIP_AUTH_ALGO*/
  IPC_OMADM_NAM_INFO,                             /* 0x0F : IPC_OMADM_NAM_INFO*/
  IPC_OMADM_START_CIDC,                           /* 0x10 : IPC_OMADM_START_CIDC*/
  IPC_OMADM_START_CIFUMO,                         /* 0x11 : IPC_OMADM_START_CIFUMO*/
  IPC_OMADM_START_CIPRL,                          /* 0x12 : IPC_OMADM_START_CIPRL*/
  IPC_OMADM_START_HFA,                            /* 0x13 : IPC_OMADM_START_HFA*/
  IPC_OMADM_START_REG_HFA,                        /* 0x14 : IPC_OMADM_START_REG_HFA*/
  IPC_OMADM_SETUP_SESSION,                        /* 0x15 : IPC_OMADM_SETUP_SESSION*/
  IPC_OMADM_SERVER_START_SESSION,                /* 0x16 : IPC_OMADM_SERVER_START_SESSION*/
  IPC_OMADM_CLIENT_START_SESSION,                /* 0x17 : IPC_OMADM_CLIENT_START_SESSION*/
  IPC_OMADM_SEND_DATA,                            /* 0x18 : IPC_OMADM_SEND_DATA*/
  IPC_OMADM_ENABLE_HFA,                           /* 0x19 : IPC_OMADM_ENABLE_HFA*/
  IPC_OMADM_MAX  
} ipc_omadm_sub_cmd_type;

/*=================================================================================

   SUB_CMD(1) : IPC_OMADM_PRL_SIZE    0x01            Read PRL Size
   
===================================================================================*/
/*  ------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
------------------------------------------------------------------------------------
*/
/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) :                      IPC_CMD_GET

 DESCRIPTION :
   
 FORMAT :
---------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
---------------------------------------------------------------------
*/

/*=================================================================================

   SUB_CMD(1) : IPC_OMADM_MODEL_NAME    0x02            Get Model Name
===================================================================================*/
/*  ------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
------------------------------------------------------------------------------------
*/
/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) :                      IPC_CMD_GET

 DESCRIPTION :
   
 FORMAT :
---------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
---------------------------------------------------------------------
*/

/*=================================================================================

   SUB_CMD(1) : IPC_OMADM_OEM_NAME    0x03            Get Oem Name
===================================================================================*/
/*  ------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
------------------------------------------------------------------------------------
*/
/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) :                      IPC_CMD_GET

 DESCRIPTION :
   
 FORMAT :
---------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
---------------------------------------------------------------------
*/
/*=================================================================================

   SUB_CMD(1) : IPC_OMADM_SW_VER    0x04            Get SW Version
===================================================================================*/
/*  ------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
------------------------------------------------------------------------------------
*/
/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) :                      IPC_CMD_GET

 DESCRIPTION :
   
 FORMAT :
---------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
---------------------------------------------------------------------
*/

/*=================================================================================

   SUB_CMD(1) : IPC_OMADM_IS683_DATA    0x05           IS683 Process 
===================================================================================*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) :                      IPC_CMD_EXEC

 DESCRIPTION :
   
 FORMAT :
*/
/*---------------------------------------------------------------------------------
  -------------------------------------------------------------------
   | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  | TOTAL_SIZE(4) |
  -------------------------------------------------------------------
  -------------------------------
  | CURRENT_SIZE(2) | MORE(1) | DATA(VARIABLE) |
  -------------------------------
    ---------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) :                      IPC_CMD_GET

 DESCRIPTION :
   
 FORMAT :
*/
/*---------------------------------------------------------------------------------
  -------------------------------------------------------------------
   | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |  | TOTAL_SIZE(4) |
  -------------------------------------------------------------------
  -------------------------------
  | CURRENT_SIZE(2) | MORE(1) | DATA(VARIABLE) |
  -------------------------------
    ---------------------------------------------------------------------------------*/

/*=================================================================================

   SUB_CMD(1) : IPC_OMADM_PRL_READ    0x06         Get PRL Data 
===================================================================================*/
/*  ------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
------------------------------------------------------------------------------------
*/
/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) :                      IPC_CMD_GET

 DESCRIPTION :
   
 FORMAT :
---------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
---------------------------------------------------------------------
*/

/* RESULT */
#define OMADM_RESULT_SUCCESS					0x00		/* Success */
#define OMADM_RESULT_INVALID_PRL				0x01		/* Invalid PRL */
#define OMADM_RESULT_NV_ERR					0x02		/* NV Write Error */
/*=================================================================================

   SUB_CMD(1) : IPC_OMADM_PRL_WRITE    0x07         Set PRL Data 
===================================================================================*/
/*  ------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
------------------------------------------------------------------------------------
*/
/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) :                      IPC_CMD_GET

 DESCRIPTION :
   
 FORMAT :
---------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
---------------------------------------------------------------------
*/

/*=================================================================================

   SUB_CMD(1) : IPC_OMADM_PUZL_DATA    0x08        Get PUZL Data
===================================================================================*/
/*  ------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
------------------------------------------------------------------------------------
*/
/*-----------------------------------------------------------------------------------
 CMD_TYPE(1) :                      IPC_CMD_GET

 DESCRIPTION :
   
 FORMAT :
---------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
---------------------------------------------------------------------
*/

/*=================================================================================

   SUB_CMD(1) : IPC_OMADM_ROOTCERT_READ    0x09       RootCert Read 
===================================================================================*/
/*  ------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
------------------------------------------------------------------------------------
*/

/*=================================================================================

   SUB_CMD(1) : IPC_OMADM_ROOTCERT_WRITE    0x0A       RootCert Write 
===================================================================================*/
/*  ------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
------------------------------------------------------------------------------------
*/

/*=================================================================================

   SUB_CMD(1) : IPC_OMADM_MMC_OBJECT    0x0B     MMC Object
===================================================================================*/
/*  ------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
------------------------------------------------------------------------------------
*/
// MMC object id (Large variable size)

#define IPC_OMADM_MMC_IS683_RESULT_ID				0x00
#define IPC_OMADM_MMC_PRL_ID							0x01
#define IPC_OMADM_MMC_PUZL_ID						0x02
#define IPC_OMADM_MMC_SSLRootCert0_ID				0x03
#define IPC_OMADM_MMC_SSLRootCert1_ID				0x04
#define IPC_OMADM_MMC_SSLRootCert2_ID				0x05
#define IPC_OMADM_MMC_SSLRootCert3_ID				0x06

/*=================================================================

   SUB_CMD(1) : IPC_OMADM_MIP_NAI_OBJECT  0x0D 

=================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_SET                0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_INDI                0x01     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION : 
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | INDEX(1) | MASK(2)
-------------------------------------------------------------------------------------
*/



/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION : 
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | INDEX(1) | MASK(2) | NAI_LEN(1) | NAI(X)
-------------------------------------------------------------------------------------
*/

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION : 
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | INDEX(1) | MASK(2) | NAI_LEN(1) | NAI(X)
-------------------------------------------------------------------------------------
*/


/*=================================================================

   SUB_CMD(1) : IPC_OMADM_MIP_AUTH_ALGO  0x0E 

=================================================================*/

/*    IPC_CMD_EXEC              0x01     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_SET                0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_INDI                0x01     */
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_GET                     0x02

 DESCRIPTION : 
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | INDEX(1)
-------------------------------------------------------------------------------------
*/

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION : 
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | INDEX(1) | AAA_AUTH | HA_AUTH
-------------------------------------------------------------------------------------
*/

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_RESP                     0x02

 DESCRIPTION : 
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | INDEX(1) | AAA_AUTH | HA_AUTH
-------------------------------------------------------------------------------------
*/



/*    IPC_CMD_EXEC                  0x01     */
/*    IPC_CMD_GET                   0x02     */
/*    IPC_CMD_SET                   0x03     */
/*    IPC_CMD_CFRM                  0x04     */
/*    IPC_CMD_EVENT                 0x05     */
/*    IPC_CMD_INDI                  0x01     */
/*    IPC_CMD_RESP                  0x02     */
/*    IPC_CMD_NOTI             0x03     */


/*=================================================================

   SUB_CMD(1) :   IPC_OMADM_START_CIDC,								 0x10 : IPC_OMADM_MMC_OBJECT

=================================================================*/
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_EXEC              0x01 

 DESCRIPTION : 
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/

/*-------------------------------------------------------------------
 CMD_TYPE(1) :  IPC_CMD_RESP                  0x02   

 DESCRIPTION : 
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
-------------------------------------------------------------------------------------
*/

/*=================================================================

   SUB_CMD(1) :  IPC_OMADM_START_CIFUMO,								 0x11 : IPC_OMADM_MMC_OBJECT

=================================================================*/
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_EXEC              0x01 

 DESCRIPTION : 
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/

/*-------------------------------------------------------------------
 CMD_TYPE(1) :  IPC_CMD_RESP                  0x02   

 DESCRIPTION : 
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
-------------------------------------------------------------------------------------
*/

/*=================================================================

   SUB_CMD(1) :    IPC_OMADM_START_CIPRL,								 0x12 : IPC_OMADM_MMC_OBJECT

=================================================================*/
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_EXEC              0x01 

 DESCRIPTION : 
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/

/*-------------------------------------------------------------------
 CMD_TYPE(1) :  IPC_CMD_RESP                  0x02   

 DESCRIPTION : 
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
-------------------------------------------------------------------------------------
*/


/*=================================================================

   SUB_CMD(1) :  IPC_OMADM_START_HFA,								 0x13 : IPC_OMADM_MMC_OBJECT

=================================================================*/
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_EXEC              0x01 

 DESCRIPTION : 
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/

/*-------------------------------------------------------------------
 CMD_TYPE(1) :  IPC_CMD_RESP                  0x02   

 DESCRIPTION : 
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
-------------------------------------------------------------------------------------
*/



/*=================================================================

   SUB_CMD(1) :    IPC_OMADM_START_REG_HFA,								0x14 : IPC_OMADM_MMC_OBJECT

=================================================================*/
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_EXEC              0x01 

 DESCRIPTION : 
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------------------------------------------------
*/

/*-------------------------------------------------------------------
 CMD_TYPE(1) :  IPC_CMD_RESP                  0x02   

 DESCRIPTION : 
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | 
-------------------------------------------------------------------------------------
*/



/*================================================================================

   SUB_CMD(1) :    IPC_OMADM_SETUP_SESSION,       0x15 : IPC_OMADM_SETUP_SESSION

=================================================================*/
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION : 
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | CB1_ID(1) | CB2_ID(1) | CB3_ID(1) |
-------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------
 | ENC(1) | VER(1) | MAX_MSG_SIZE(2) | MAX_OBJ_SIZE(2) | COOKIE(1) | DISP_NAME(64) |
-------------------------------------------------------------------------------------
*/



/*================================================================================

   SUB_CMD(1) :    IPC_OMADM_SERVER_START_SESSION,        0x16 : IPC_OMADM_SERVER_START_SESSION

=================================================================*/
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION : 
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | TYPE(1) | SESSION_ID(1) | ID(64) |
-------------------------------------------------------------------------------------
*/



/*================================================================================

   SUB_CMD(1) :    IPC_OMADM_CLIENT_START_SESSION,        0x17 : IPC_OMADM_CLIENT_START_SESSION

=================================================================*/
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION : 
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | TYPE(1) | ID(64) | ALERT_TYPE(128) |
-------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------
 | ALERT_FORMAT(32) | ALERT_SRC(32) | ALERT_SORREL(32) | ALERT_MARK(32) |
-------------------------------------------------------------------------------------
--------------------
 | ALERT_DATA(32) | 
--------------------
*/



/*================================================================================

   SUB_CMD(1) :    IPC_OMADM_SEND_DATA,        0x18 : IPC_OMADM_SEND_DATA

=================================================================*/
//none

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION : 
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | TYPE(1) | ID(1) | DATA(1024) | LEN(2) |
-------------------------------------------------------------------------------------
*/


/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_NOTI                     0x03

 DESCRIPTION : 
 
 FORMAT :
-------------------------------------------------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | TYPE(1) | ID(1) | DATA(1024) | LEN(2) |
-------------------------------------------------------------------------------------
*/


/*================================================================================

   SUB_CMD(1) :    IPC_OMADM_ENABLE_HFA,        0x19 : IPC_OMADM_ENABLE_HFA

=================================================================*/

/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_CMD_SET                     0x03

 DESCRIPTION : 
 
 FORMAT :
-------------------------------------------
 | MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) |
-------------------------------------------
*/




/*********************************************************************************

                                            Sub Command of IPC_GEN_CMD[0x80]

**********************************************************************************/
typedef enum{
  IPC_GEN_PHONE_RES=0x01,  	/* 0x01 : General response command for PDA request. */
  IPC_GEN_MAX
} ipc_sub_gen_cmd_type;

/*********************************************************************************/

/*=================================================================

   SUB_CMD(1) : IPC_GEN_CMD_RESP      0x01                        General Response Message

=================================================================*/

/*    IPC_CMD_EXEC             0x01     */
/*    IPC_CMD_GET              0x02     */
/*    IPC_CMD_SET              0x03     */
/*    IPC_CMD_CFRM             0x04     */
/*    IPC_CMD_EVENT            0x05     */
/*    IPC_CMD_INDI             0x01     */
/*    IPC_CMD_NOTI             0x03     */
//none


/*-------------------------------------------------------------------
 CMD_TYPE(1) : IPC_GEN_PHONE_RES                     0x02

 DESCRIPTION :
    General Response Message
 FORMAT :
----------------------------------------------------------------------------------------
| MAIN_CMD(1) | SUB_CMD(1) | CMD_TYPE(1) | REV_MAIN_CMD(1) | REV_SUB_CMD(1) | ERROR(2) |
----------------------------------------------------------------------------------------
*/


/* CMD_TYPE(1) */
/* 0x00 : ignore ( just padding byte here ) */

/* REV_MAIN_CMD */
/* Received Main command */

/* REV_SUB_CMD */
/* Received Sub command */

/* ERROR */
/*
Refer to Table, which refer to the +CME ERROR in the AT command set. ( 3GPP TS 27.007 )
0x8000 : No Errors
if not 0x8000, then error takes place
*/

#define SS_ERR_CODE(val)  		(0x8100+val)  	/* Error Code */
#define SS_ERR_GEN(val)  		(0x8200+val)  	/* General Problem */
#define SS_ERR_INVOKE(val)  	(0x8210+val)  	/* Invoke Problem */
#define SS_ERR_RESULT(val)  	(0x8220+val)  	/* Return Result Problem */
#define SS_ERR_ERROR(val)  		(0x8230+val)  	/* Return Error Problem */


typedef enum{
  /************************************************************
  **    Errors defined in  "+CME ERROR" ,
  **    - see 3GPP TS 27.007
  **    - ranges are 0x00 ~ 0x7FFF
  ************************************************************/
  /* GENERAL ERRORS */
  IPC_GEN_ERR_PHONE_FAILURE=0,  						/* 0 */
  IPC_GEN_ERR_NO_CONNECTION_TO_PHONE,  					/* 1 */
  IPC_GEN_ERR_PHONE_ADAPTOR_LINK_RESERVED,  			/* 2 */
  IPC_GEN_ERR_OPER_NOT_ALLOWED,  						/* 3 */
  IPC_GEN_ERR_OPER_NOT_SUPPORTED,  						/* 4 */
  IPC_GEN_ERR_PH_SIM_PIN_REQU,  						/* 5 */
  IPC_GEN_ERR_PH_FSIM_PIN_REQU,  						/* 6 */
  IPC_GEN_ERR_PH_FSIM_PUK_REQU,  						/* 7 */
  IPC_GEN_ERR_SIM_NOT_INSERTED=10,  					/* 10 */
  IPC_GEN_ERR_SIM_PIN_REQU,  							/* 11 */
  IPC_GEN_ERR_SIM_PUK_REQU,  							/* 12 */
  IPC_GEN_ERR_SIM_FAILURE,  							/* 13 */
  IPC_GEN_ERR_SIM_BUSY,  								/* 14 */
  IPC_GEN_ERR_SIM_WRONG,  								/* 15 */
  IPC_GEN_ERR_INCORRECT_PW,  							/* 16 */
  IPC_GEN_ERR_SIM_PIN2_REQU,  							/* 17 */
  IPC_GEN_ERR_SIM_PUK2_REQU,  							/* 18 */
  IPC_GEN_ERR_WRONG_SIM_SLOT,  							/* 19 */
  IPC_GEN_ERR_MEM_FULL=20,  							/* 20 */
  IPC_GEN_ERR_INVALID_INDEX,  							/* 21 */
  IPC_GEN_ERR_NOT_FOUND,  								/* 22 */
  IPC_GEN_ERR_MEM_FAILURE,  							/* 23 */
  IPC_GEN_ERR_TEXT_STR_TOO_LONG,  						/* 24 */
  IPC_GEN_ERR_INVALID_CHARACTERS_IN_TEXT_STR,  			/* 25 */
  IPC_GEN_ERR_DIAL_STR_TOO_LONG,  						/* 26 */
  IPC_GEN_ERR_INVALID_CHARACTERS_IN_DIAL_STR,  			/* 27 */
  IPC_GEN_ERR_NO_NET_SVC=30,  							/* 30 */
  IPC_GEN_ERR_NET_TIMEOUT,  							/* 31 */
  IPC_GEN_ERR_NET_NOT_ALLOWED_EMERGENCY_CALLS_ONLY,  	/* 32 */
  IPC_GEN_ERR_NET_PERS_PIN_REQU=40,  					/* 40 */
  IPC_GEN_ERR_NET_PERS_PUK_REQU,  						/* 41 */
  IPC_GEN_ERR_NET_SUBSET_PERS_PIN_REQU,  				/* 42 */
  IPC_GEN_ERR_NET_SUBSET_PERS_PUK_REQU,  				/* 43 */
  IPC_GEN_ERR_SVC_PROVIDER_PERS_PIN_REQU,  				/* 44 */
  IPC_GEN_ERR_SVC_PROVIDER_PERS_PUK_REQU,  				/* 45 */
  IPC_GEN_ERR_CORPORATE_PERS_PIN_REQU,  				/* 46 */
  IPC_GEN_ERR_CORPORATE_PERS_PUK_REQU,  				/* 47 */
  IPC_GEN_ERR_HIDDEN_KEY_REQU,  						/* 48 */
  IPC_GEN_ERR_UNKNOWN=100,  							/* 100 */

  /* Errors related to a failure to perform an Attach */
  IPC_GEN_ERR_ILLEGAL_MS=103,  							/* 103 */
  IPC_GEN_ERR_ILLEGAL_ME=106,  							/* 106 */
  IPC_GEN_ERR_GPRS_SVC_NOT_ALLOWED,  					/* 107 */
  IPC_GEN_ERR_PLMN_NOT_ALLOWED=111,  					/* 111 */
  IPC_GEN_ERR_LOCATION_AREA_NOT_ALLOWED,  				/* 112 */
  IPC_GEN_ERR_ROAMING_NOT_ALLOWED_IN_THIS_LOCATION_AREA,  	/* 113 */

  /* Errors related to a failure to Activate a Context */
  /* 149 is in this category. */
  IPC_GEN_ERR_SVC_OPT_NOT_SUPPORTED=132,  				/* 132 */
  IPC_GEN_ERR_REQ_SVC_OPT_NOT_SUBSCRIBED,  				/* 133 */
  IPC_GEN_ERR_SVC_OPT_TEMPORARILY_OUT_OF_ORDER,  		/* 134 */

  /* Other GPRS errors */
  IPC_GEN_ERR_UNSPECIFIED_GPRS_ERR=148,  				/* 148 */
  IPC_GEN_ERR_PDP_AUTHENTICATION_FAILURE,  				/* 149 */
  IPC_GEN_ERR_INVALID_MOBILE_CLASS,  					/* 150 */

  /* VBS / VGCS and eMLPP -related errors */
  IPC_GEN_ERR_VBS_VGCS_NOT_SUPPORTED_BY_THE_NET=151,  	/* 151 */
  IPC_GEN_ERR_NO_SVC_SUBSCRIPTION_ON_SIM,  				/* 152 */
  IPC_GEN_ERR_NO_SUBSCRIPTION_FOR_GROUP_ID,  			/* 153 */
  IPC_GEN_ERR_GROUP_ID_NOT_ACTIVATED_ON_SIM,  			/* 154 */
  IPC_GEN_ERR_NO_MATCHING_NOTI=155,  					/* 155 */
  IPC_GEN_ERR_VBS_VGCS_CALL_ALREADY_PRESENT,  			/* 156 */
  IPC_GEN_ERR_CONGESTION,  								/* 157 */
  IPC_GEN_ERR_NET_FAILURE,  							/* 158 */
  IPC_GEN_ERR_UPLINK_BUSY,  							/* 159 */
  IPC_GEN_ERR_NO_ACCESS_RIGHTS_FOR_SIM_FILE=160,  		/* 160 */
  IPC_GEN_ERR_NO_SUBSCRIPTION_FOR_PRIORITY,  			/* 161 */
  IPC_GEN_ERR_OPER_NOT_APPLICABLE_OR_NOT_POSSIBLE,  	/* 162 */


  /************************************************************
  **                           SAMSUNG ADDED ERRORS
  ************************************************************/
  IPC_GEN_ERR_NONE=0x8000,  				/* 0x8000 : No Errors */

  /* General Common Errors : 0x8000 - 0x80FF */
  IPC_GEN_ERR_INVALID_IPC,  			/* 0x8001 : Invalid IPC_GSM Parameter or Format */
  IPC_GEN_ERR_PHONE_OFFLINE,  				/* 0x8002 : */
  IPC_GEN_ERR_CMD_NOT_ALLOWED,  			/* 0x8003 : */
  IPC_GEN_ERR_PHONE_IS_INUSE,  				/* 0x8004 : */
  IPC_GEN_ERR_INVALID_STATE=0x8005,  		/* 0x8005 : */
  
  IPC_GEN_ERR_NO_BUFFER,  					/* 0x8006 :  No internal free buffers */
  IPC_GEN_ERR_OPER_REJ,  					/* 0x8007 :  Operation Rejceted */
  IPC_GEN_ERR_INSUFFICIENT_RESOURCE,  		/* 0x8008 : insufficient resource */
  IPC_GEN_ERR_NET_NOT_RESPOND,  			/* 0x8009 : Network not responding */
  IPC_GEN_ERR_SIM_PIN_ENABLE_REQ=0x800A,  	/* 0x800A : SIM Pin Enable Required */
  IPC_GEN_ERR_SIM_PERM_BLOCKED,     /* 0x800B : SIM Permanent Blocked */    // JBG 2005.03.30
  IPC_GEN_ERR_SIM_PHONEBOOK_RESTRICTED,    /*0x800C: SIM Phonebook Restricted*/
  IPC_GEN_ERR_FIXED_DIALING_NUMBER_ONLY,   /*0x800D: Restricted By FDN Mode */
  IPC_GEN_ERR_SIM_PIN2_PERM_BLOCKED,            /*0x800E: SIM PIN2 Permanent Blocked */  // dishin, 2007-05-16 
//  IPC_GEN_ERR_PHONE_CDMA_LOCKED,                    /* 0x8003 : CDMA only */
//  IPC_GEN_ERR_FLASH_IS_INPROGRESS,                  /* 0x8004 : CDMA only */
//  IPC_GEN_ERR_E911_MODE,                            /* 0x8006 : CDMA only */

  /* Reserved : 0x800F ~ 0x80FF */
  IPC_GEN_ERR_800E_RESERVED_START=0x800F,  	/* 0x800F */

  IPC_GEN_ERR_80FF_RESERVED_END=0x80ff,  	/* 0x80FF */

  /* Suplementary Service - related errors, 
  ** see 3GPP TS 24.080
  */
  /* ERROR-TAG : 0x8100 ~ 0x81FF */
  IPC_SS_ERR_UNKNOWN_SUBSCRIBER = SS_ERR_CODE(0x01),  		/* 0x01 : unknownSubscriber  "Invalid User Number" */
  IPC_SS_ERR_ILLEGAL_SUBSCRIBER = SS_ERR_CODE(0x09),  		/* 0x09 : illegal Subscriber */
  IPC_SS_ERR_BEARER_NOT_PROVISIONED = SS_ERR_CODE(0x0a),  	/* 0x0a : BearerServiceNotProvisioned "Request Rejected" */
  IPC_SS_ERR_TELESERVICE_NOT_PROVISIONED = SS_ERR_CODE(0x0b), /* 0x0b : TeleServiceNotProvisioned */
  IPC_SS_ERR_CALL_BARRED = SS_ERR_CODE(0x0d),  				/* 0x0d : CallBarred */
  IPC_SS_ERR_ILLEGAL_EQUIPMENT = SS_ERR_CODE(0x0c),  		/* 0x0c : illegal Equipment */
  IPC_SS_ERR_ILLEGAL_SS_OPER = SS_ERR_CODE(0x10),  			/* 0x10 : illegalSS_Operation */
  IPC_SS_ERR_ERR_STATUS = SS_ERR_CODE(0x11),  				/* 0x11 : ss_ErrorStatus */
  IPC_SS_ERR_NOT_AVAIL = SS_ERR_CODE(0x12),  				/* 0x12 : ss_NotAvailable "Service not available" */
  IPC_SS_ERR_SUBSCRIPTION_VIOLATION = SS_ERR_CODE(0x13),  	/* 0x13 : ss_SubscriptionViolation */
  IPC_SS_ERR_INCOMPATIBILITY = SS_ERR_CODE(0x14),  			/* 0x14 : ss_Incompatibility */
  IPC_SS_ERR_FACILITY_NOT_SUPPORT = SS_ERR_CODE(0x15),  	/* 0x015 : FacilityNotSupported */
  IPC_SS_ERR_ABSENT_SUBSCRIBER = SS_ERR_CODE(0x1b),  		/* 0x1b : absent subscriber */
  IPC_SS_ERR_SHORT_TERM_DENIAL = SS_ERR_CODE(0x1d),  		/* 0x1d : denial because of the short term */
  IPC_SS_ERR_LONG_TERM_DENIAL = SS_ERR_CODE(0x1e),  		/* 0x1e : denial because of the long term */
  IPC_SS_ERR_SYS_FAILURE = SS_ERR_CODE(0x22),  				/* 0x22 : "SystemFailure "Please try again" */
  IPC_SS_ERR_DATA_MISSING = SS_ERR_CODE(0x23),  			/* 0x23 : DataMissing */
  IPC_SS_ERR_UNEXPECTED_DATA = SS_ERR_CODE(0x24),  			/* 0x24 : UnexpectedDataValue 0x13 */
  IPC_SS_ERR_PW_REGIST_FAIL = SS_ERR_CODE(0x25),  			/* 0x25 : PasswordRegistrationFailure -ο password Ͻ error ߻  ( ex:password mismatch ) */
  IPC_SS_ERR_NEGATIVE_PW_CHECK = SS_ERR_CODE(0x26),  		/* 0x26 : NegativePasswordCheck - password Է½ ߸ Է  */
  IPC_SS_ERR_PW_ATTEMPS_VIOLATION = SS_ERR_CODE(0x2b),  	/* 0x2B : numberOfPW_AttemptsViolation - password ִ  õ Ƚ  */
  IPC_SS_ERR_POSITION_METHOD_FAIL = SS_ERR_CODE(0x36),  	/* 0x36 : PositionMethodFailure */
  IPC_SS_ERR_UNKNOWN_ALPHA = SS_ERR_CODE(0x47),  			/* 0x47 : Unknown Alphabet */
  IPC_SS_ERR_USSD_BUSY = SS_ERR_CODE(0x48),  				/* 0x48 : ussd_Busy */
  IPC_SS_ERR_REJECTED_BY_USER = SS_ERR_CODE(0x79),  		/* 0x79 : rejected By User */
  IPC_SS_ERR_REJECTED_BY_NET = SS_ERR_CODE(0x7a),  			/* 0x7a : rejected By Network */
  IPC_SS_ERR_DEFLECTION_SUBSCRIBER = SS_ERR_CODE(0x7b),  	/* 0x7b : DeflectionToServedSubscriber "Deflected to own number " */
  IPC_SS_ERR_SPECIAL_SVC_CODE = SS_ERR_CODE(0x7c),  		/* 0x7c : SpecialServiceCode */
  IPC_SS_ERR_INVALID_DEFLECT_NUM = SS_ERR_CODE(0x7d),  		/* 0x7d : InvalidDeflectedToNumber "Invalid deflected to number " */
  IPC_SS_ERR_MPTY_PARTICIPANTS_EXCEED = SS_ERR_CODE(0x7e),  	/* 0x7e : maxNumberOfMPTY_ParticipantsExceeded */
  IPC_SS_ERR_RESOURCE_NOT_AVAIL = SS_ERR_CODE(0x7f),  		/* 0x7f : ResourcesNotAvailable */

    /* General Problem : 0x8200 ~ 0x820F  */
  IPC_SS_ERR_UNRECOGNIZED_COMPONENT=SS_ERR_GEN(0x00),             /* 0x16 : UNRECOGNIZED_COMPONENT "Please try again" */
  IPC_SS_ERR_MISTYPED_COMPONENT=SS_ERR_GEN(0x01),                      /* 0x18 : MISTYPED_COMPONENT */
  IPC_SS_ERR_BADLY_STRUCTURED_COMPONENT=SS_ERR_GEN(0x02),     /* 0x17 : BADLY_STRUCTURED_COMPONENT */

  /* Invoke Problem : 0x8210 ~ 0x821F  */
  IPC_SS_ERR_DUPLICATE_INVOKE_ID=SS_ERR_INVOKE(0x00),           /* 0x1E : DUPLICATE_INVOKE_ID "Request Rejected" */
  IPC_SS_ERR_UNRECOGNISED_OPER=SS_ERR_INVOKE(0x01),              /* 0x1F : UNRECOGNISED_OPERATION */
  IPC_SS_ERR_MISTYPED_PARAM=SS_ERR_INVOKE(0x02),                       /* 0x24 : MISTYPED_PARAMETER "Please try again" */
  IPC_SS_ERR_RESOURCE_LIMITATION=SS_ERR_INVOKE(0x03),              /* 0x25 : RESOURCE_LIMITATION */
  IPC_SS_ERR_INITIATING_REL=SS_ERR_INVOKE(0x04),                         /* 0x20 : INITIATING_RELEASE */
  IPC_SS_ERR_UNRECOGNISED_LINKED_ID=SS_ERR_INVOKE(0x05),       /* 0x21 : UNRECOGNISED_LINKED_ID */
  IPC_SS_ERR_LINKED_RESP_UNEXPECTED=SS_ERR_INVOKE(0x06),       /* 0x22 : LINKED_RESPONSE_UNEXPECTED */
  IPC_SS_ERR_UNEXPECTED_LINKED_OPER=SS_ERR_INVOKE(0x07),       /* 0x23 : UNEXPECTED_LINKED_OPERATION */

  /* Return Result Problem : 0x8220 ~ 0x822F */
  IPC_SS_ERR_RESULT_UNRECOG_INVOKE_ID=SS_ERR_RESULT(0x00),       /* 0x19 : UNRECOGNISED_INVOKE_ID "Request Rejected" */
  IPC_SS_ERR_RESULT_UNEXPECTED=SS_ERR_RESULT(0x01),  /* 0x1A : RETURN_RESULT_UNEXPECTED */
  IPC_SS_ERR_RESULT_MISTYPED_PARAM=SS_ERR_RESULT(0x02),                /* 0x1D : RE_MISTYPED_PARAMETER */
  
  /* Return Error Problem : 0x8230 ~ 0x823F  */
  IPC_SS_ERR_ERROR_UNRECOG_INVOKE_ID=SS_ERR_ERROR(0x00),       /* 0x19 : UNRECOGNISED_INVOKE_ID "Request Rejected" */
  IPC_SS_ERR_ERROR_UNEXPECTED=SS_ERR_ERROR(0x01),  /* 0x1A : RETURN_RESULT_UNEXPECTED */
  IPC_SS_ERR_ERROR_UNRECOG=SS_ERR_ERROR(0x02),                  /* 0x1C : UNRECOGNISED_ERROR */
  IPC_SS_ERR_ERROR_UNEXPECTED_ERROR=SS_ERR_ERROR(0x03),        /* 0x1B : RETURN_ERROR_UNEXPECTED */
  IPC_SS_ERR_ERROR_MISTYPED_PARAM=SS_ERR_ERROR(0x04),                /* 0x1D : RE_MISTYPED_PARAMETER */

  /* Reserved : 0x8300 ~ 0xFFFF */
  IPC_GEN_ERR_8300_RESERVED_START=0x8300,  		/* 0x8300 */

  IPC_GEN_ERR_FFFD_RESERVED_END=0xFFFD,  		/* 0xFFFF */


  /* the other errors */
  IPC_GEN_ERR_OTHERS = 0xFFFE,  				/* 0xFFFE */
  
  IPC_GEN_ERR_MAX = 0xFFFF
}ipc_gen_err_e_type;


/*    IPC_CMD_NOTI             0x03     */
/* none */

 #endif	//__IPC_V40_H__
