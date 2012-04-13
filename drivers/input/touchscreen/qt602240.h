/* drivers/input/touchscreen/qt602240.h
 *
 * QT602240 TSP driver.
 *
 * Copyright (C) 2009 Samsung Electronics Co. Ltd.
 *
 */


/*
 *  option for updating (cte options to) the flash, if config error comes...!!!
 *  used to update the line sizes and only in 7" LCD p1-p2 boards
 *  Enable this and load the zImage once. Then switch off the device 
 *  then disable this option and recompile for the normal operation. 
 */

#define __VER_1_4__
#define __BUILD_0x0B__
#define PRINTF_ENABLE
#define OPTION_WRITE_CONFIG
#define QT_printf  printk
#define QT_FIRMUP_ENABLE
#define QT_STYLUS_ENABLE
#define FEATUE_QT_INFOBLOCK_STATIC
#define USE_TSP_EARLY_SUSPEND
#define _SUPPORT_MULTITOUCH_
#define DEBUG_LOG_ENABLE 0
#define DRIVER_FILTER 0
#define MXT224E_TA_CONFIG_MODE 1
#define TOUCH_CPU_FREQ 0

#ifdef QT_FIRMUP_ENABLE
#include "mxt224_firmware.h"
#include "mxt224e_firmware.h"

#define QT_ATCOM_TEST
#define QT_WAITING_BOOTLOAD_COMMAND 0xC0
#define QT_WAITING_FRAME_DATA       0x80
#define QT_FRAME_CRC_CHECK          0x02
#define QT_FRAME_CRC_PASS           0x04
#define QT_FRAME_CRC_FAIL           0x03

#define I2C_APPL_ADDR_0         0x94//0x4Au
#define I2C_APPL_ADDR_1         0x96//0x4Bu
#define I2C_BOOT_ADDR_0         0x48//0x24u
#define I2C_BOOT_ADDR_1         0x4A//0x25u

#define NUM_OF_I2C_ADDR    4

uint8_t i2c_addresses[] =
{
	I2C_APPL_ADDR_0,
	I2C_APPL_ADDR_1,
	I2C_BOOT_ADDR_0,
	I2C_BOOT_ADDR_1,
};

//static void __iomem *gpio_pend_mask_mem; /* not use */

#define INT_PEND_BASE 0xE0200A44

uint8_t QT_i2c_address;

void QT_reprogram(void);
uint8_t QT_Boot_no_reset(void);
uint8_t QT_Boot(void);
#endif

#define UPDATE_QT602240_FLASH   0

#define QT602240_I2C_ADDR       0x4A
#define QT602240_I2C_BOOT_ADDR  0x24


#define IRQ_TOUCH_INT (IRQ_EINT_GROUP18_BASE+5) /* J0_5 */

#undef QT602240_DEBUG

#ifdef QT602240_DEBUG
#define DEBUG printk("[QT] %s/%d\n",__func__,__LINE__)
#define DEBUG_MSG(p, x...)			printk("[QT602240]:[%s] ", __func__); printk(p, ## x);
#define ENTER_FUNC	{ printk("[QT602240] +%s\n", __func__); }
#define LEAVE_FUNC	{ printk("[QT602240] -%s\n", __func__); }
#else
#define DEBUG
#define DEBUG_MSG(p, x...)
#define ENTER_FUNC
#define LEAVE_FUNC
#endif


#define DEBUG_PRESS 0

#if DEBUG_PRESS 
#define dprintk printk 
#define PRINT_MSG(p, x...)		{ printk("[QT602240]:[%s] ", __func__); printk(p, ## x); }
#else
#define print_msg()
#define dprintk(...)
#define PRINT_MSG(p, x...)
#endif


#define USE_PERF_LEVEL_TS 1

#define USE_TS_EARLY_SUSPEND 1

#define USE_TS_TA_DETECT_CHANGE_REG 1 //sooo.shin

//static	void	__iomem		*gpio_mask_mem;


/*---------------------------------------------------------/
 *
 * Quantum Code Block
 *
 * -------------------------------------------------------*/
#define U16    unsigned short int 
#define U8      __u8
#define u8      __u8
#define S16     signed short int
#define U16     unsigned short int
#define S32     signed long int
#define U32     unsigned long int
#define S64     signed long long int
#define U64     unsigned long long int
#define F32     float
#define F64     double


typedef enum 
{
	NO_COMMAND = 0u,
	COMM_MODE1 = 1u,
	COMM_MODE2 = 2u,
	COMM_MODE3 = 3u
}comm_cfg;



/* \brief Defines CHANGE line active mode. */
/* \brief Defines CHANGE line active mode. */
#define CHANGELINE_NEGATED          0u
#define CHANGELINE_ASSERTED         1u

/* This sets the I2C frequency to 400kHz (it's a feature in I2C driver that the
   actual value needs to be double that). */
#define I2C_SPEED                   800000u

#define CONNECT_OK                  1u
#define CONNECT_ERROR               2u

#define READ_MEM_OK                 1u
#define READ_MEM_FAILED             2u

#define MESSAGE_READ_OK             1u
#define MESSAGE_READ_FAILED         2u

#define WRITE_MEM_OK                1u
#define WRITE_MEM_FAILED            2u

#define CFG_WRITE_OK                1u
#define CFG_WRITE_FAILED            2u

#define I2C_INIT_OK                 1u
#define I2C_INIT_FAIL               2u

#define CRC_CALCULATION_OK          1u
#define CRC_CALCULATION_FAILED      2u

#define ID_MAPPING_OK               1u
#define ID_MAPPING_FAILED           2u

#define ID_DATA_OK                  1u
#define ID_DATA_NOT_AVAILABLE       2u


enum driver_setup_t {DRIVER_SETUP_OK, DRIVER_SETUP_INCOMPLETE};

/*! \brief Returned by get_object_address() if object is not found. */
#define OBJECT_NOT_FOUND   0u

/*! Address where object table starts at touch IC memory map. */
#define OBJECT_TABLE_START_ADDRESS      7U

/*! Size of one object table element in touch IC memory map. */
#define OBJECT_TABLE_ELEMENT_SIZE       6U

/*! Offset to RESET register from the beginning of command processor. */
#define RESET_OFFSET                    0u

/*! Offset to BACKUP register from the beginning of command processor. */
#define BACKUP_OFFSET       1u

/*! Offset to CALIBRATE register from the beginning of command processor. */
#define CALIBRATE_OFFSET    2u

/*! Offset to REPORTALL register from the beginning of command processor. */
#define REPORTATLL_OFFSET   3u

/*! Offset to DEBUG_CTRL register from the beginning of command processor. */
#define DEBUG_CTRL_OFFSET   4u

/*! Offset to DIAGNOSTIC_CTRL register from the beginning of command processor. */
#define DIAGNOSTIC_OFFSET   5u


//*****************************************************************************
//
//
//		std_objects_driver
//
//
//*****************************************************************************

/*! ===Header File Version Number=== */
#define OBJECT_LIST__VERSION_NUMBER     0x11


#define RESERVED_T0                               0u
#define RESERVED_T1                               1u
#define DEBUG_DELTAS_T2                           2u
#define DEBUG_REFERENCES_T3                       3u
#define DEBUG_SIGNALS_T4                          4u
#define GEN_MESSAGEPROCESSOR_T5                   5u
#define GEN_COMMANDPROCESSOR_T6                   6u
#define GEN_POWERCONFIG_T7                        7u
#define GEN_ACQUISITIONCONFIG_T8                  8u
#define TOUCH_MULTITOUCHSCREEN_T9                 9u
#define TOUCH_SINGLETOUCHSCREEN_T10               10u
#define TOUCH_XSLIDER_T11                         11u
#define TOUCH_YSLIDER_T12                         12u
#define TOUCH_XWHEEL_T13                          13u
#define TOUCH_YWHEEL_T14                          14u
#define TOUCH_KEYARRAY_T15                        15u
#define PROCG_SIGNALFILTER_T16                    16u
#define PROCI_LINEARIZATIONTABLE_T17              17u
#define SPT_COMCONFIG_T18                         18u
#define SPT_GPIOPWM_T19                           19u
#define PROCI_GRIPFACESUPPRESSION_T20             20u
#define RESERVED_T21                              21u
#define PROCG_NOISESUPPRESSION_T22                22u
#define TOUCH_PROXIMITY_T23                       23u
#define PROCI_ONETOUCHGESTUREPROCESSOR_T24        24u
#define SPT_SELFTEST_T25                          25u
#define DEBUG_CTERANGE_T26                        26u
#define PROCI_TWOTOUCHGESTUREPROCESSOR_T27        27u
#define SPT_CTECONFIG_T28                         28u
#define SPT_GPI_T29                               29u
#define SPT_GATE_T30                              30u
#define TOUCH_KEYSET_T31                          31u
#define TOUCH_XSLIDERSET_T32                      32u
#define RESERVED_T33                              33u
#define GEN_MESSAGEBLOCK_T34                      34u
#define SPT_GENERICDATA_T35                       35u
#define RESERVED_T36                              36u
#define DEBUG_DIAGNOSTIC_T37                      37u
#define SPT_USERDATA_T38                          38u
#define SPARE_T39                                 39u
#define PROCI_GRIPSUPPRESSION_T40                 40u
#define SPARE_T41                                 41u
#define PROCI_TOUCHSUPPRESSION_T42                42u
#define SPARE_T43                                 43u
#define SPARE_T44                                 44u
#define SPARE_T45                                 45u
#define SPT_CTECONFIG_T46                         46u
#define PROCI_STYLUS_T47                          47u
#define PROCG_NOISESUPPRESSION_T48                48u
#define SPARE_T49                                 49u
#define SPARE_T50                                 50u
/*
 * All entries spare up to 255
*/
#define RESERVED_T255                             255u

/*! @} */

/*----------------------------------------------------------------------------
  type definitions
----------------------------------------------------------------------------*/

typedef struct
{
	uint8_t reset;       /*!< Force chip reset             */
	uint8_t backupnv;    /*!< Force backup to eeprom/flash */
	uint8_t calibrate;   /*!< Force recalibration          */
	uint8_t reportall;   /*!< Force all objects to report  */
	uint8_t reserve;   /*!< Turn on output of debug data */
	uint8_t diagnostic;  /*!< Controls the diagnostic object */
}__packed gen_commandprocessor_t6_config_t;



typedef struct
{
	uint8_t idleacqint;    /*!< Idle power mode sleep length in ms           */
	uint8_t actvacqint;    /*!< Active power mode sleep length in ms         */
	uint8_t actv2idleto;   /*!< Active to idle power mode delay length in units of 0.2s*/
}__packed gen_powerconfig_t7_config_t;



typedef struct
{ 
	uint8_t chrgtime;          /*!< Charge-transfer dwell time             */  
	uint8_t reserved;          /*!< reserved                               */
	uint8_t tchdrift;          /*!< Touch drift compensation period        */
	uint8_t driftst;           /*!< Drift suspend time                     */
	uint8_t tchautocal;        /*!< Touch automatic calibration delay in units of 0.2s*/
	uint8_t sync;              /*!< Measurement synchronisation control    */
	uint8_t atchcalst;         /*!< recalibration suspend time after last detection */
	uint8_t atchcalsthr;       /*!< Anti-touch calibration suspend threshold */
}__packed gen_acquisitionconfig_t8_config_t;

typedef struct
{ 
	uint8_t chrgtime;          /*!< Charge-transfer dwell time             */  
	uint8_t reserved;          /*!< reserved                               */
	uint8_t tchdrift;          /*!< Touch drift compensation period        */
	uint8_t driftst;           /*!< Drift suspend time                     */
	uint8_t tchautocal;        /*!< Touch automatic calibration delay in units of 0.2s*/
	uint8_t sync;              /*!< Measurement synchronisation control    */
	uint8_t atchcalst;         /*!< recalibration suspend time after last detection */
	uint8_t atchcalsthr;       /*!< Anti-touch calibration suspend threshold */
	uint8_t atchfrccalthr;     /*!< Antitouch force calibration threshold */
	uint8_t atchfrccalratio;    /*!< Antitouch force calibration ratio*/
}__packed gen_acquisitionconfig_t8_config_t_e;



typedef struct
{
	/* Screen Configuration */
	uint8_t ctrl;            /*!< ACENABLE LCENABLE Main configuration field  */

	/* Physical Configuration */
	uint8_t xorigin;         /*!< LCMASK ACMASK Object x start position on matrix  */
	uint8_t yorigin;         /*!< LCMASK ACMASK Object y start position on matrix  */
	uint8_t xsize;           /*!< LCMASK ACMASK Object x size (i.e. width)         */
	uint8_t ysize;           /*!< LCMASK ACMASK Object y size (i.e. height)        */

	/* Detection Configuration */
	uint8_t akscfg;          /*!< Adjacent key suppression config     */
	uint8_t blen;            /*!< Sets the gain of the analog circuits in front of the ADC. The gain should be set in
                            conjunction with the burst length to optimize the signal acquisition. Maximum gain values for
                            a given object/burst length can be obtained following a full calibration of the system. GAIN
                            has a maximum setting of 4; settings above 4 are capped at 4.*/
	uint8_t tchthr;          /*!< ACMASK Threshold for all object channels   */
	uint8_t tchdi;           /*!< Detect integration config           */

	uint8_t orient;  /*!< LCMASK Controls flipping and rotating of touchscreen
                            *   object */
	uint8_t mrgtimeout; /*!< Timeout on how long a touch might ever stay
                            *   merged - units of 0.2s, used to tradeoff power
                            *   consumption against being able to detect a touch
                            *   de-merging early */

	/* Position Filter Configuration */
	uint8_t movhysti;   /*!< Movement hysteresis setting used after touchdown */
	uint8_t movhystn;   /*!< Movement hysteresis setting used once dragging   */
	uint8_t movfilter;  /*!< Position filter setting controlling the rate of  */

	/* Multitouch Configuration */
	uint8_t numtouch;   /*!< The number of touches that the screen will attempt
                            *   to track */
	uint8_t mrghyst;    /*!< The hysteresis applied on top of the merge threshold
                            *   to stop oscillation */
	uint8_t mrgthr;     /*!< The threshold for the point when two peaks are
                            *   considered one touch */

	uint8_t amphyst;          /*!< TBD */

	/* Resolution Controls */
	uint16_t xrange;       /*!< LCMASK */
	uint16_t yrange;       /*!< LCMASK */
	uint8_t xloclip;       /*!< LCMASK */
	uint8_t xhiclip;       /*!< LCMASK */
	uint8_t yloclip;       /*!< LCMASK */
	uint8_t yhiclip;       /*!< LCMASK */
	/* edge correction controls */
	uint8_t xedgectrl;     /*!< LCMASK */
	uint8_t xedgedist;     /*!< LCMASK */
	uint8_t yedgectrl;     /*!< LCMASK */
	uint8_t yedgedist;     /*!< LCMASK */
	uint8_t jumplimit;
}__packed touch_multitouchscreen_t9_config_t;

typedef struct
{
	uint8_t ctrl;
	uint8_t xorigin;
	uint8_t yorigin;
	uint8_t xsize;
	uint8_t ysize;
	uint8_t akscfg;
	uint8_t blen;
	uint8_t tchthr;
	uint8_t tchdi;
	uint8_t orient;
	uint8_t mrgtimeout;
	uint8_t movhysti;
	uint8_t movhystn;
	uint8_t movfilter;
	uint8_t numtouch;
	uint8_t mrghyst;
	uint8_t mrgthr;
	uint8_t amphyst;
	uint16_t xrange;
	uint16_t yrange;
	uint8_t xloclip;
	uint8_t xhiclip;
	uint8_t yloclip;
	uint8_t yhiclip;
	uint8_t xedgectrl;
	uint8_t xedgedist;
	uint8_t yedgectrl;
	uint8_t yedgedist;
	uint8_t jumplimit;     /*!< maximum position jump */
	uint8_t tchhyst;       /*!< touch threshold hysteresis */
	uint8_t xpitch;        /*!< x line pitch */
	uint8_t ypitch;        /*!< y line pitch */
	uint8_t nexttchdi;     /*!< touch detect integration for subsequent touches */
}__packed touch_multitouchscreen_t9_config_t_e;



typedef struct
{
	/* Key Array Configuration */
	uint8_t ctrl;           /*!< ACENABLE LCENABLE Main configuration field           */

	/* Physical Configuration */
	uint8_t xorigin;        /*!< ACMASK LCMASK Object x start position on matrix  */
	uint8_t yorigin;        /*!< ACMASK LCMASK Object y start position on matrix  */
	uint8_t size;           /*!< ACMASK LCMASK Object x size (i.e. width)         */

	/* Detection Configuration */
	uint8_t akscfg;         /*!< Adjacent key suppression config     */
	uint8_t blen;           /*!< ACMASK Burst length for all object channels*/
	uint8_t tchthr;         /*!< ACMASK Threshold for all object channels   */
	uint8_t tchdi;          /*!< Detect integration config           */
	uint8_t reserved[2];    /*!< Spare x2 */
	uint8_t movhysti;       /*!< Movement hysteresis setting used after touchdown */
	uint8_t movhystn;       /*!< Movement hysteresis setting used once dragging */
	uint8_t movfilter;      /*!< Position filter setting controlling the rate of  */
}__packed touch_slider_wheel_t11_t12_t13_t14_config_t;



typedef struct
{
	/* Key Array Configuration */
	uint8_t ctrl;               /*!< ACENABLE LCENABLE Main configuration field           */

	/* Physical Configuration */
	uint8_t xorigin;            /*!< ACMASK LCMASK Object x start position on matrix  */
	uint8_t yorigin;            /*!< ACMASK LCMASK Object y start position on matrix  */
	uint8_t xsize;              /*!< ACMASK LCMASK Object x size (i.e. width)         */
	uint8_t ysize;              /*!< ACMASK LCMASK Object y size (i.e. height)        */

	/* Detection Configuration */
	uint8_t akscfg;             /*!< Adjacent key suppression config     */
	uint8_t blen;               /*!< ACMASK Burst length for all object channels*/
	uint8_t tchthr;             /*!< ACMASK LCMASK Threshold for all object channels   */
	uint8_t tchdi;              /*!< Detect integration config           */
	uint8_t reserved[2];        /*!< Spare x2 */
}__packed touch_keyarray_t15_config_t;



typedef struct
{
	uint8_t ctrl;
	uint16_t xoffset;
	uint8_t  xsegment[16];
	uint16_t yoffset;
	uint8_t  ysegment[16];
}__packed proci_linearizationtable_t17_config_t;



typedef struct
{
	uint8_t  ctrl;
	uint8_t  cmd;
}__packed spt_comcconfig_t18_config_t;



typedef struct
{
	/* GPIOPWM Configuration */
	uint8_t ctrl;             /*!< Main configuration field           */
	uint8_t reportmask;       /*!< Event mask for generating messages to
                                  *   the host */
	uint8_t dir;              /*!< Port DIR register   */
	uint8_t intpullup;        /*!< Port pull-up per pin enable register */
	uint8_t out;              /*!< Port OUT register*/
	uint8_t wake;             /*!< Port wake on change enable register  */
	uint8_t pwm;              /*!< Port pwm enable register    */
	uint8_t period;           /*!< PWM period (min-max) percentage*/
	uint8_t duty[4];          /*!< PWM duty cycles percentage */
}__packed spt_gpiopwm_t19_config_t;



typedef struct
{
	uint8_t ctrl;
	uint8_t xlogrip;
	uint8_t xhigrip;
	uint8_t ylogrip;
	uint8_t yhigrip;
	uint8_t maxtchs;
	uint8_t reserved;
	uint8_t szthr1;
	uint8_t szthr2;
	uint8_t shpthr1;
	uint8_t shpthr2;
	uint8_t supextto;
}__packed proci_gripfacesuppression_t20_config_t;



typedef struct
{
	uint8_t ctrl;

	uint8_t reserved;

	uint8_t reserved1;
	int16_t gcaful;
	int16_t gcafll;

	uint8_t actvgcafvalid;        /* LCMASK */

	uint8_t noisethr;
	uint8_t reserved2;
	uint8_t freqhopscale;

	uint8_t freq[5u];             /* LCMASK ACMASK */
	uint8_t idlegcafvalid;        /* LCMASK */
}__packed procg_noisesuppression_t22_config_t;



typedef struct
{
	/* Prox Configuration */
	uint8_t ctrl;              /*!< ACENABLE LCENABLE Main configuration field           */

	/* Physical Configuration */
	uint8_t xorigin;           /*!< ACMASK LCMASK Object x start position on matrix  */
	uint8_t yorigin;           /*!< ACMASK LCMASK Object y start position on matrix  */
	uint8_t xsize;             /*!< ACMASK LCMASK Object x size (i.e. width)         */
	uint8_t ysize;             /*!< ACMASK LCMASK Object y size (i.e. height)        */
	uint8_t reserved_for_future_aks_usage;
	/* Detection Configuration */
	uint8_t blen;              /*!< ACMASK Burst length for all object channels*/
	uint16_t tchthr;           /*!< LCMASK Threshold    */
	uint8_t tchdi;             /*!< Detect integration config           */
	uint8_t average;           /*!< LCMASK Sets the filter length on the prox signal */
	uint16_t rate;             /*!< Sets the rate that prox signal must exceed */
}__packed touch_proximity_t23_config_t;



typedef struct
{
	uint8_t  ctrl;
	uint8_t  numgest;
	uint16_t gesten;
	uint8_t  pressproc;
	uint8_t  tapto;
	uint8_t  flickto;
	uint8_t  dragto;
	uint8_t  spressto;
	uint8_t  lpressto;
	uint8_t  reppressto;
	uint16_t flickthr;
	uint16_t dragthr;
	uint16_t tapthr;
	uint16_t throwthr;
}__packed proci_onetouchgestureprocessor_t24_config_t;



typedef struct
{
	uint16_t upsiglim;              /* LCMASK */
	uint16_t losiglim;              /* LCMASK */

} siglim_t;

/*! = Config Structure = */

typedef struct
{
	uint8_t ctrl;                 /* LCENABLE */
	uint8_t cmd;
#ifdef NUM_OF_TOUCH_OBJECTS
	siglim_t siglim[NUM_OF_TOUCH_OBJECTS];   /* LCMASK */
#endif
}__packed spt_selftest_t25_config_t;



typedef struct
{
	uint8_t ctrl;          /*!< Bit 0 = object enable, bit 1 = report enable */
	uint8_t numgest;       /*!< Runtime control for how many two touch gestures  to process */
	uint8_t reserved2;
	uint8_t gesten;        /*!< Control for turning particular gestures on or  off */
	uint8_t rotatethr;
	uint16_t zoomthr;
}__packed proci_twotouchgestureprocessor_t27_config_t;



typedef struct
{
	uint8_t ctrl;          /*!< Ctrl field reserved for future expansion */
	uint8_t cmd;           /*!< Cmd field for sending CTE commands */
	uint8_t mode;          /*!< LCMASK CTE mode configuration field */
	uint8_t idlegcafdepth; /*!< LCMASK The global gcaf number of averages when idle */
	uint8_t actvgcafdepth; /*!< LCMASK The global gcaf number of averages when active */
	uint8_t voltage;
}__packed spt_cteconfig_t28_config_t;



typedef struct
{
	uint8_t mode;
	uint8_t page;
	uint8_t data[128];    
}__packed debug_diagnositc_t37_t;

typedef struct
{
	uint8_t mode;
	uint8_t page;
	int8_t data[128];    
}__packed debug_diagnositc_t37_delta_t;

typedef struct
{
	uint8_t mode;
	uint8_t page;
	uint16_t data[64];    
}__packed debug_diagnositc_t37_reference_t;

typedef struct
{
	uint8_t mode;
	uint8_t page;
	uint8_t data[128];    
}__packed debug_diagnositc_t37_cte_t;



typedef struct
{
	uint8_t ctrl;         /*<! enable or disable grip suppression */
	uint8_t xlogrip;      /*<! grip suppression x low boundary */
	uint8_t xhigrip;      /*<! grip suppression x high boundary */
	uint8_t ylogrip;      /*<! grip suppression y low boundary */
	uint8_t yhigrip;      /*<! grip suppression y high boundary */
}__packed proci_gripsuppression_t40_config_t;



typedef struct
{
	uint8_t ctrl;             /*<! enable or disable touch suppression */
	uint8_t apprthr;          /*<! approach threshold */
	uint8_t maxapprarea;      /*<! maximum approach area threshold */
	uint8_t maxtcharea;       /*<! maximum touch area threshold */
	uint8_t supstrength;      /*<! suppression aggressiveness */
	uint8_t supextto;         /*<! suppression extension timeout */
	uint8_t maxnumtchs;       /*<! maximum touches */
	uint8_t shapestrength;    /*<! shaped-based aggressiveness */
}__packed proci_touchsuppression_t42_config_t;



typedef struct
{
	uint8_t ctrl;              /*<!  */
	uint8_t mode;              /*<! xy mode */
	uint8_t idlesyncsperx;     /*<! number of sync groups per x when idle */
	uint8_t actvsyncsperx;     /*<! number of sync groups per x when active */
	uint8_t adcspersync;       /*<! number of adc conversions per sync group */
	uint8_t pulsesperadc;      /*<! number of pulses for each adc conversion */
	uint8_t xslew;             /*<! x pulse slew rate */
	uint16_t syncdelay;        /*<! delay from sync */
}__packed spt_cteconfig_t46_config_t;



typedef struct
{
	uint8_t ctrl;              /*<!  */
	uint8_t contmin;
	uint8_t contmax;
	uint8_t stability;
	uint8_t maxtcharea;
	uint8_t amplthr;
	uint8_t styshape;
	uint8_t hoversup;
	uint8_t confthr;
	uint8_t syncsperx;
}__packed proci_stylus_t47_config_t;



typedef struct
{
	uint8_t ctrl;                /*<!  */
	uint8_t cfg;
	uint8_t calcfg;
	uint8_t basefreq;
	uint8_t reserved1[4];
	uint8_t mffreq[2];
	uint8_t reserved2[3];
	uint8_t gcactvinvldadcs;
	uint8_t gcidlenivldadcs;
	uint8_t reserved3[2];
	uint8_t gcmaxadcsperx;
	uint8_t gclimitmin;
	uint8_t gclimitmax;
	uint16_t gccountmintgt;
	uint8_t mfinvlddiffthr;
	uint16_t mfincadcspxthr;
	uint16_t mferrorthr;
	uint8_t selfreqmax;
	uint8_t reserved4[6];
	uint8_t blen;                /*<! T9 setting block */
	uint8_t tchthr;
	uint8_t tchdi;
	uint8_t movhysti;
	uint8_t movhystn;
	uint8_t movfilter;
	uint8_t numtouch;
	uint8_t mrghyst;
	uint8_t mrgthr;
	uint8_t xloclip;
	uint8_t xhiclip;
	uint8_t yloclip;
	uint8_t yhiclip;
	uint8_t xedgectrl;
	uint8_t xedgedist;
	uint8_t yedgectrl;
	uint8_t yedgedist;
	uint8_t jumplimit;
	uint8_t tchhyst;
	uint8_t nexttchdi;
}__packed procg_noisesuppression_t48_config_t;

typedef struct
{
	uint8_t fherr_setting;
	uint8_t fherr_count;
	uint8_t fherr_num;
	uint8_t t9_blen_for_fherr;
	uint8_t t9_thr_for_fherr;
	uint8_t t9_movfilter_for_fherr;
	uint8_t t22_noisethr_for_fherr;
	uint8_t t22_freqscale_for_fherr;
	uint8_t freq_for_fherr1[5];
	uint8_t freq_for_fherr2[5];
	uint8_t freq_for_fherr3[5];
	uint8_t freq_for_fherr4[5];
}__packed t22_freq_table_config_t;


#define ENABLE_NOISE_TEST_MODE	1

#if ENABLE_NOISE_TEST_MODE

#define TEST_POINT_NUM      5
typedef enum 
{
	QT_PAGE_UP         = 0x01,
	QT_PAGE_DOWN       = 0x02,
	QT_DELTA_MODE      = 0x10,
	QT_REFERENCE_MODE  = 0x11,
	QT_CTE_MODE        = 0x31
}diagnostic_debug_command;
#endif

//*****************************************************************************
//
//
//		info_block_driver
//
//
//*****************************************************************************

/*! \brief Object table element struct. */
typedef struct
{
	uint8_t object_type;     /*!< Object type ID. */
	uint16_t i2c_address;    /*!< Start address of the obj config structure. */
	uint8_t size;            /*!< Byte length of the obj config structure -1.*/
	uint8_t instances;       /*!< Number of objects of this obj. type -1. */
	uint8_t num_report_ids;  /*!< The max number of touches in a screen,
	                                             *  max number of sliders in a slider array, etc.*/
} object_t;


/*! \brief Info ID struct. */
typedef struct
{
	uint8_t family_id;            /* address 0 */
	uint8_t variant_id;           /* address 1 */

	uint8_t version;              /* address 2 */
	uint8_t build;                /* address 3 */

	uint8_t matrix_x_size;        /* address 4 */
	uint8_t matrix_y_size;        /* address 5 */
	
	/*! Number of entries in the object table. The actual number of objects
	* can be different if any object has more than one instance. */
	uint8_t num_declared_objects; /* address 6 */
} info_id_t;



typedef struct
{
	/*! Info ID struct. */
	info_id_t info_id;

	/*! Pointer to an array of objects. */
	object_t *objects;

	/*! CRC field, low bytes. */
	uint16_t CRC;

	/*! CRC field, high byte. */
	uint8_t CRC_hi;
} info_block_t;



typedef struct
{
	uint8_t object_type;     /*!< Object type. */
	uint8_t instance;        /*!< Instance number. */
} report_id_map_t;

#ifdef _SUPPORT_MULTITOUCH_
//#define _SUPPORT_TOUCH_AMPLITUDE_
typedef struct
{
	uint16_t size;      /*!<  size */
	int16_t pressure;   /*!< dn>0, up=0, none=-1 */
	int16_t x;          /*!< X */
	int16_t y;          /*!< Y */
} report_finger_info_t;

//#define MAX_NUM_FINGER	10		// Maximum possible fingering
#define MAX_USING_FINGER_NUM	5
#endif

#define FINGER_NUM	      5 //for multi touch
struct input_info {
	int max_x;
	int max_y;
	int state;
	int x;
	int y;
	int z;
	int width;
	int finger_id;
};

/* Each client has this additional data */
struct qt602240_data {
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct work_struct ts_event_work;
	unsigned int irq;
	struct early_suspend early_suspend;
	struct input_info info[FINGER_NUM+1];
};


/*------------------------------ functions prototype -----------------------------------*/
uint8_t init_touch_driver(uint8_t I2C_address);
uint8_t close_touch_driver(void);
uint8_t reset_chip(void);
uint8_t calibrate_chip(void);
uint8_t diagnostic_chip(uint8_t mode);
uint8_t backup_config(void);
uint8_t get_variant_id(uint8_t *variant);
uint8_t get_family_id(uint8_t *family_id);
uint8_t get_build_number(uint8_t *build);
uint8_t get_version(uint8_t *version);
uint8_t write_power_config(gen_powerconfig_t7_config_t power_config);
uint8_t write_acquisition_config(gen_acquisitionconfig_t8_config_t acq_config);
uint8_t write_acquisition_config_e(gen_acquisitionconfig_t8_config_t_e acq_config);
uint8_t write_multitouchscreen_config(uint8_t screen_number, touch_multitouchscreen_t9_config_t cfg);
uint8_t write_multitouchscreen_config_e(uint8_t screen_number, touch_multitouchscreen_t9_config_t_e cfg);
uint8_t write_keyarray_config(uint8_t key_array_number, touch_keyarray_t15_config_t cfg);
uint8_t write_linearization_config(uint8_t instance, proci_linearizationtable_t17_config_t cfg);
uint8_t write_comc_config(uint8_t instance, spt_comcconfig_t18_config_t cfg);
uint8_t write_gpio_config(uint8_t instance, spt_gpiopwm_t19_config_t cfg);
uint8_t write_gripsuppression_config(uint8_t instance, proci_gripfacesuppression_t20_config_t cfg);
uint8_t write_noisesuppression_config(uint8_t instance, procg_noisesuppression_t22_config_t cfg);
uint8_t write_proximity_config(uint8_t instance, touch_proximity_t23_config_t cfg);
uint8_t write_onetouchgesture_config(uint8_t instance, proci_onetouchgestureprocessor_t24_config_t cfg);
uint8_t write_selftest_config(uint8_t instance, spt_selftest_t25_config_t cfg);
uint8_t write_twotouchgesture_config(uint8_t instance, proci_twotouchgestureprocessor_t27_config_t cfg);
uint8_t write_CTE_config(spt_cteconfig_t28_config_t cfg);
uint8_t write_grip_suppression_config(proci_gripsuppression_t40_config_t cfg);
uint8_t write_touch_suppression_config(proci_touchsuppression_t42_config_t cfg);
uint8_t write_stylus_config(proci_stylus_t47_config_t cfg);
uint8_t write_CTE_config_e(uint8_t instance, spt_cteconfig_t46_config_t cfg);
uint8_t write_noisesuppression_config_e(uint8_t instance, procg_noisesuppression_t48_config_t cfg);
uint8_t write_simple_config(uint8_t object_type, uint8_t instance, void *cfg);
uint8_t get_object_size(uint8_t object_type);
uint8_t type_to_report_id(uint8_t object_type, uint8_t instance);
uint8_t report_id_to_type(uint8_t report_id, uint8_t *instance);
uint8_t read_id_block(info_id_t *id);
uint8_t get_max_report_id(void);
uint16_t get_object_address(uint8_t object_type, uint8_t instance);
uint32_t get_stored_infoblock_crc(void);
uint8_t calculate_infoblock_crc(uint32_t *crc_pointer);
uint32_t CRC_24(uint32_t crc, uint8_t byte1, uint8_t byte2);
void write_message_to_usart(uint8_t msg[], uint8_t length);
void TSP_Restart(void);

U8 read_changeline(void);
U8 init_I2C(U8 I2C_address_arg);
U8 read_mem(U16 start, U8 size, U8 *mem);
U8 read_U16(U16 start, U16 *mem);
U8 write_mem(U16 start, U8 size, U8 *mem);

void read_all_register(void);

#ifdef USE_TSP_EARLY_SUSPEND
static void qt602240_early_suspend(struct early_suspend *);
static void qt602240_late_resume(struct early_suspend *);
#endif	/* USE_TSP_EARLY_SUSPEND */


/* Module information */
MODULE_DESCRIPTION("AT42QT602240 Touchscreen driver");
MODULE_LICENSE("GPL");
