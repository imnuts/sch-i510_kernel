#define S5K5AAFA_COMPLETE
//#undef S5K5AAFA_COMPLETE
/*
 * Driver for S5K5AAFA (VGA camera) from Samsung Electronics
 * 
 * 1/4" 2.0Mp CMOS Image Sensor SoC with an Embedded Image Processor
 *
 * Copyright (C) 2009, Jinsung Yang <jsgood.yang@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#ifndef __S5K5AAFA_H__
#define __S5K5AAFA_H__

struct s5k5aafa_reg {
	unsigned char addr;
	unsigned char val;
};

struct s5k5aafa_regset_type {
	unsigned char *regset;
	int len;
};

/*
 * Macro
 */
#define REGSET_LENGTH(x)	(sizeof(x)/sizeof(s5k5aafa_reg))

/*
 * User defined commands
 */
/* S/W defined features for tune */
#define REG_DELAY	0xFF00	/* in ms */
#define REG_CMD		0xFFFF	/* Followed by command */

/*threshold */
enum threshold_table_t{
NORMAL_LIGHT_COND,
MID_LOW_LIGHT_COND,
LOW_LIGHT_COND,
MIN_SHADE_COND,
MAX_SHADE_COND,
CAM_THRESHOLD_MAX,
};

enum capture_delay_t{
LOW_LIGHT_DELAY,
MID_LOW_LIGHT_DELAY,
MID_LIGHT_DELAY,
NORMAL_LIGHT_DELAY,
CAPTURE_DELAY_MAX,
};
/* Section Enumeration */
typedef enum{
  /* MCLK */
  SECTION_MCLK = 0,

  /* INITIALIZE */
  SECTION_INIT,
  SECTION_VT_INIT,
  
  /* DTP */
  SECTION_DTP,  
  
  /* OP MODE */
  SECTION_OP_PREVIEW,
  SECTION_OP_NORMAL_LIGHT_PREVIEW,
  SECTION_OP_HIGH_LIGHT_PREVIEW,
  
  SECTION_OP_CAPTURE,
  SECTION_OP_MIDLIGHT_CAPTURE,
  SECTION_OP_MID_LOW_LIGHT_CAPTURE,
  SECTION_OP_LOW_LIGHT_CAPTURE,  

  SECTION_OP_CAMCORDER,

  /* BRIGHTNESS_EV */
  SECTION_EV_MINUS_4,
  SECTION_EV_MINUS_3,
  SECTION_EV_MINUS_2,
  SECTION_EV_MINUS_1,
  SECTION_EV_0,
  SECTION_EV_PLUS_1,
  SECTION_EV_PLUS_2,
  SECTION_EV_PLUS_3,
  SECTION_EV_PLUS_4,

  /* WHITE BALANCE */
  SECTION_WB_AUTO,
  SECTION_WB_TUNGSTEN,
  SECTION_WB_FLUORESCENT,
  SECTION_WB_SUNNY,
  SECTION_WB_CLOUDY,

  /* COLOR EFFECT */
  SECTION_EFFECT_NORMAL,
  SECTION_EFFECT_B_ANB_W,
  SECTION_EFFECT_NEGATIVE,
  SECTION_EFFECT_SEPIA,
  SECTION_EFFECT_GREEN,
  SECTION_EFFECT_AQUA,

  /* SHADE */
  SECTION_SHADE_DNP,
  SECTION_SHADE_ETC,

  /* ETC THRESHOLD */
  SECTION_SET_THRESHOLD_TABLE,

  /* CAPTURE DELAY TIME */
  SECTION_CAPTURE_DELAY_TABLE,
  
  SECTION_MAX
}reg_config_section_type;

typedef struct
{
  char      *name;
  u16       *table;
  u32       table_len;
} reg_section_table_type;


/* Following order should not be changed */
enum image_size_s5k5aafa {
	/* This SoC supports upto UXGA (1600*1200) */
#if 0
	QQVGA,	/* 160*120*/
	QCIF,	/* 176*144 */
	QVGA,	/* 320*240 */
	CIF,	/* 352*288 */
	VGA,	/* 640*480 */
#endif
	SVGA,	/* 800*600 */
#if 0
	HD720P,	/* 1280*720 */
	SXGA,	/* 1280*1024 */
	UXGA,	/* 1600*1200 */
#endif
};

/*
 * Following values describe controls of camera
 * in user aspect and must be match with index of s5k5aafa_regset[]
 * These values indicates each controls and should be used
 * to control each control
 */
enum s5k5aafa_control {
	S5K5AAFA_INIT,
	S5K5AAFA_EV,
	S5K5AAFA_AWB,
	S5K5AAFA_MWB,
	S5K5AAFA_EFFECT,
	S5K5AAFA_CONTRAST,
	S5K5AAFA_SATURATION,
	S5K5AAFA_SHARPNESS,
};

#define S5K5AAFA_REGSET(x)	{	\
	.regset = x,			\
	.len = sizeof(x)/sizeof(s5k5aafa_reg),}

//============================================================================
//
//   Register Setting Data File for 
//   SYSTEMLSI 1.3 Mega Pixel CMOS YCbCr sensor
//   Set  File version 10.06.21

static u16 MCLK_TABLE[1] = {
// ------------------------------------
// 필요한 클럭 하나만 주석 풀고 사용
// ------------------------------------
//  6144   // 6.144MHz Jitter free
// 12288  // 12.288MHz Jitter Free
// 19200  // TCXO
// 24000  // 24.0MHz
24576      // 24.576MHz Jitter Free
//25000   // 25.0MHz
// 32000  // 32.0MHz
// 34000  // 34.0MHz
// 34560  // 34.56MHz
// 48000  // 48.0MHz
// 49152  // 49.152MHz Jitter free
};



static u16 INIT_DATA[] ={ 
0xfc02,
0x3100, // cfPN Off //
0x3204, // cfPNtart frame //

0xfc00,
0x00aa, // for edS check //

0xfc07,
0x6601, // WdT on//
0xfc01,
0x0401, // aRM clock divider //

0xfc07, // Initial setting   //
0x0900,
0x0ab3,
0x0b00,
0x0ca3,
0x0e40,

0xfc00,
0x7042,

//==================================================//
//	clock setting                               //
//==================================================//
0xfc07,
0x3700, // Main clock precision //

0xfc00,
0x7278, // flicker for 24MHz //

//==================================================//
//	cIS setting                                 //
//==================================================//
0xfc02,
0x2e82, // analog offset(default 80)              //
0x2f00, // b channel line adlc tuning register    //
0x3000, // gb channel line adlc tuning register   //
0x3718, // Global Gain(default 18)                //
0x44db, // cLP_eN on                              //
0x45f0, // 1R eNd,1S eNd time(ff)                 //
0x472f, // 3_S4 adrs                              //
0x4a00, // clamp                                  //
0x4b11, // Ramp current                           //
0x4d02, // dbLR Tune                              //
0x4e2d, // SCL_SDA_CTRL_ADRS                      //
0x4faa, // noise check                            //
0x551f, // channel Line adLc on                   //
0x5700, // R channel line adlc tuning register    //
0x5800, // gr channel line adlc tuning register   //
0x5b0f, // Ob_Ref                                 //
0x6001, // ST_en                                  //
0x620d, // lew 03                                 //
0x6322, // cdS aMP current                        //
0x2d58, // double Shutter                         //
0x3100, // cfPN Off                               //

//==================================================//
//	Shutter suppress                            //
//==================================================//
0xfc00,
0x8306, // shutter off, double shutter off  //

0xfc0b,
0x5c60,
0x5d60,

//==================================================//
//	Table set for capture                       //
//==================================================//
0xfc03, // page 03h                   //
0x0005, // cis_frame_h_width_h        //
0x01a4, // cis_frame_h_width_l        //
0x0204, // cis_frame_v_depth_h        //
0x0352, // cis_frame_v_deoth_l        //
0x0405, // cis_output_h_width_h       //
0x0510, // cis_output_h_width_l       //
0x0604, // cis_output_v_depth_h       //
0x070a, // cis_output_v_depth_l       //
0x0801, // cis_h_even_inc_l           //
0x0901, // cis_h_odd_inc_l            //
0x0a01, // cis_v_even_inc_l           //
0x0b01, // cis_v_odd_inc_l            //
0x0c00, // cis_mirr_etc(average sub)  //
0x0d00, // dsp5_post_hstart_h         //
0x0e0a, // dsp5_post_hstart_l         //
0x0f05, // dsp5_post_hwidth_h         //
0x1000, // dsp5_post_hwidth_l         //
0x1100, // dsp5_post_vstart_h         //
0x120a, // dsp5_post_vstart_l         //
0x1304, // dsp5_post_vheight_h        //
0x1400, // dsp5_post_vheight_l        //
0x1505, // dsp5_img_hsizeh            //
0x1600, // dsp5_img_hsizel            //
0x1704, // dsp5_img_vsizeh            //
0x1800, // dsp5_img_vsizel            //
0x1900, // dsp5_psf                   //
0x1a00, // dsp5_msfx_h                //
0x1b00, // dsp5_msfx_l                //
0x1c00, // dsp5_msfy_h                //
0x1d00, // dsp5_msfy_l                //
0x1e05, // dsp5_dw_hsizeh             //
0x1f00, // dsp5_dw_hsizel             //
0x2004, // dso5_dw_vsizeh             //
0x2100, // dsp5_dw_vsizel             //
0x2200, // dsp5_startxh               //
0x2300, // dsp5_startxl               //
0x2400, // dsp5_startyh               //
0x2500, // dsp5_startyl               //
0x2605, // dsp5_clip_hsizeh           //
0x2700, // dsp5_clip_hsizel           //
0x2804, // dsp5_clip_vsizeh           //
0x2900, // dsp5_clip_vsizel           //
0x2a00, // dsp5_sel_main              //
0x2b00, // dsp5_htermh                //
0x2c05, // dsp5_htermm                //
0x2db2, // dsp5_hterml                //
0x2e03, // dsp1_crcb_sel              //
0x2f00, // ozone                      //

//==================================================//
//	Table set for Preview                       //
//==================================================//
0xfc04, // page 04h                  // 
0x9005, // cis_frame_h_width_h       // 
0x91a2, // cis_frame_h_width_l       // 
0x9202, // cis_frame_v_depth_h       // 
0x932c, // cis_frame_v_deoth_l       // 
0x9402, // cis_output_h_width_h      // 
0x9588, // cis_output_h_width_l      // 
0x9602, // cis_output_v_depth_h      // 
0x9704, // cis_output_v_depth_l      // 
0x9801, // cis_h_even_inc_l          // 
0x9903, // cis_h_odd_inc_l           // 
0x9a01, // cis_v_even_inc_l          // 
0x9b03, // cis_v_odd_inc_l           // 
0x9c08, // cis_mirr_etc(average sub) // 
0x9d00, // dsp5_post_hstart_h        // 
0x9e06, // dsp5_post_hstart_l        // 
0x9f02, // dsp5_post_hwidth_h        // 
0xa080, // dsp5_post_hwidth_l        // 
0xa100, // dsp5_post_vstart_h        // 
0xa204, // dsp5_post_vstart_l        // 
0xa301, // dsp5_post_vheight_h       // 
0xa4e0, // dsp5_post_vheight_l       // 
0xa502, // dsp5_img_hsizeh           // 
0xa680, // dsp5_img_hsizel           // 
0xa702, // dsp5_img_vsizeh           // 
0xa800, // dsp5_img_vsizel           // 
0xa900, // dsp5_psf                  // 
0xaa00, // dsp5_msfx_h               // 
0xab00, // dsp5_msfx_l               // 
0xac00, // dsp5_msfy_h               // 
0xad00, // dsp5_msfy_l               // 
0xae02, // dsp5_dw_hsizeh            // 
0xaf80, // dsp5_dw_hsizel            // 
0xb002, // dso5_dw_vsizeh            // 
0xb100, // dsp5_dw_vsizel            // 
0xb200, // dsp5_startxh              // 
0xb300, // dsp5_startxl              // 
0xb400, // dsp5_startyh              // 
0xb500, // dsp5_startyl              // 
0xb602, // dsp5_clip_hsizeh          // 
0xb780, // dsp5_clip_hsizel          // 
0xb802, // dsp5_clip_vsizeh          // 
0xb900, // dsp5_clip_vsizel          // 
0xba00, // dsp5_sel_main             // 
0xbb00, // dsp5_htermh               // 
0xbc05, // dsp5_htermm               // 
0xbdb2, // dsp5_hterml               // 
0xbe03, // dsp1_crcb_sel             // 
0xbf30, // ozone                     // 

//==================================================//
//	cOMMaNd setting                             //
//==================================================//
0xfc00,
0x2904, //03 brightness_H //
0x2a00, //6f 6f brightness_L //
0x2b04, //03 color level H      //
0x2c00, //90 87 color level L   //
0x3202, //02 aWb_average_Num    //
0x620a, //0a Hue control enable   ahn Test (mirror??)   //
0x6ca0, //9a 89 a0 ae target_L  //
0x6d00, //00 ae target_H        //
0x7311, //11 frame ae           //
0x7418, //14 flicker 50Hz auto  //
0x786A, //6a aGc Max            //
0x8190, //90 10 Mirror Xhading on, Ggain offset off, eIT GaMMa ON //
0x0700, //00 eIT gamma Th.	//
0x082d, //20 eIT gamma Th.	//

0xfc20,
0x010a, // Stepless_Off	new shutter //
0x0206, // flicker dgain Mode //
0x0302,
0x1001, // 2 shutter Max 35ms //
0x1148, // 70 35ms for new Led spec (50hz->35ms) // 
0x1480, // brightness offset//
0x1650, // 4c 2.65 frame ae Start (Gain 2 =40h) //
0x2501, // cintr Min //
0x2ca0, // forbidden cintc //
0x5508,
0x5608,
0x570a, // Stable_frame_ae //

//==================================================//
//	ISP setting                                 //
//==================================================//
0xfc01,
0x0101, // pclk inversion//         
0x0c02, // full Yc On //            
            
//==================================================//
//	color Matrix                                //
//==================================================//
0xfc01,
0x5108,
0x523c,
0x53fa,
0x54c2,
0x5501,
0x5602,
0x57fe,
0x58eb,
0x5906,
0x5a5d,
0x5bfe,
0x5cb9,
0x5dff,
0x5ee4,
0x5ffd,
0x6010,
0x6107,
0x620c,

//==================================================//
//	edge enhancement                            //
//==================================================//
0xfc00,
0x8903, // edge Mode on //
0xfc0b,
0x4250, // edge aGc MIN //
0x4360, // edge aGc MaX //
0x451a, // positive gain aGc MIN //
0x4912, // positive gain aGc MaX //
0x4d1a, // negative gain aGc MIN //
0x5112, // negative gain aGc MaX //       

0xfc05,
0x3440, // YaPTcLP:Y edge clip //
0x3518, // YaPTSc:Y edge Noiselice 10->8 //
0x360b, //0b eNHaNce      //
0x3f00, // NON-LIN        //
0x4041, // Y delay        //
0x4210, // eGfaLL:edge coloruppress Gainlope        //
0x4300, // HLfaLL:High-light coloruppress Gainlope  //
0x45a0, // eGRef:edge coloruppress Reference Thres.       //
0x467a, // HLRef:High-light coloruppress Reference Thres. //
0x4740, // LLRef:Low-light coloruppress Reference Thres.  //
0x480c, // [5:4]edge,[3:2]High-light,[1:0]Low-light   //
0x4931, // cSSeL  eGSeL  cS_dLY  //

//==================================================//
//	Gamma setting                               //
//==================================================//
0xfc0c, // outdoor gamma //
      
0x0008,  //04 R gamma// 
0x0118,  //0f// 
0x0248,  //3a//   
0x03b5,  //ba// 
0x0400,  //00// 
          
0x056a,  //6d// 
0x06f0,  //e5// 
0x0750, 
0x089d, 
0x095a,
          
0x0ad8, 
0x0b0d, 
0x0c31, 
0x0d51, 
0x0ebf, 
          
0x0f75, 
0x108d, 
0x11a2, 
0x12b3, 
0x13ff, 
          
0x14c4, 
0x15d0, 
0x16de, 
0x17fc,
        
0x1808,  // G gamma //
0x1918,  
0x1a48,  
0x1bb5,  
0x1c00,  
          
0x1d6a,  
0x1ef0, 
0x1f50, 
0x209d, 
0x215a, 
          
0x22d8, 
0x230d, 
0x2431, 
0x2551, 
0x26bf,
          
0x2775, 
0x288d, 
0x29a2, 
0x2ab3, 
0x2bff,
         
0x2cc4, 
0x2dd0, 
0x2ede, 
0x2ffc,  
        
0x3008,  // B gamma  //
0x3118,  
0x3248,  
0x33b5,  
0x3400,  
          
0x356a,  
0x36f0, 
0x3750, 
0x389d, 
0x395a,
          
0x3ad8,  
0x3b0d, 
0x3c31, 
0x3d51, 
0x3ebf, 
          
0x3f75, 
0x408d, 
0x41a2, 
0x42b3, 
0x43ff,
          
0x44c4, 
0x45d0, 
0x46de, 
0x47fc, 
        
0xfc0d,  // Indoor gamma  // 
        
0x000a,  // R gamma       //
0x0120,  
0x0268,  
0x03bf,  
0x0400,   
          
0x0588, 
0x0620, 
0x0780, 
0x08ce, 
0x096a, 
          
0x0a08, 
0x0b32, 
0x0c5c, 
0x0d7e, 
0x0eff, 
          
0x0f9b, 
0x10b4, 
0x11c8, 
0x12d8, 
0x13ff, 
          
0x14e4, 
0x15f0, 
0x16ff, 
0x17fc, 
          
0x180a,   // G gamma //
0x1920, 
0x1a68, 
0x1bbf, 
0x1c00, 
          
0x1d88, 
0x1e20, 
0x1f80, 
0x20ce, 
0x216a,
          
0x2208, 
0x2332, 
0x245c,  
0x257e, 
0x26ff,
          
0x279b, 
0x28b4, 
0x29c8, 
0x2ad8, 
0x2bff, 
          
0x2ce4, 
0x2df0, 
0x2eff, 
0x2ffc,
          
0x300a,   // B gamma  //
0x3120,  
0x3268,  
0x33bf,  
0x3400, 
          
0x3588,  
0x3620,  
0x3780,  
0x38ce,  
0x396a, 
          
0x3a08,  
0x3b32,  
0x3c5c,  
0x3d7e,  
0x3eff, 
          
0x3f9b,  
0x40b4,  
0x41c8,  
0x42d8,  
0x43ff, 
          
0x44e4,  
0x45f0,  
0x46ff,  
0x47fc,  

//=================================================//
//	Hue settng                                 //
//=================================================//
0xfc00,  
0x483e,  // 2000K //
0x4940,    
0x4afe,  
0x4b0a,  
0x4c42,  
0x4d56,   
0x4ee4,   
0x4ff7,  
          
0x5034, //  3e RC 3000K //    
0x5138, //  40 GC//          
0x5202, //  fe RH//              
0x53fc, //  0a GH//          
0x5438, //  42 BC//          
0x5546, //  46 YC//          
0x56f4, //  e4 BH//          
0x57eb, //  f7 YH//         
      
0x5834, //  34 RC5100K // 
0x5935, //  28 GC//               
0x5a00, //  08 RH//                
0x5b08, //  10 GH//           
0x5c4a, //  52 BC//                
0x5d30, //  38 YC//               
0x5ee0, //  e0 BH//                    
0x5ff7, //  f7 YH//      

//==================================================//
//	Suppress functions                          //
//==================================================//
0xfc00,
0x7ef4, // [7]:bPR on[6]:NR on[5]:cLPf on[4]:GrGb on //            

//==================================================//
//	bPR                                         //
//==================================================//
0xfc01,
0x3d10, // PbPR On   //

0xfc0b,
0x0b00, // ISP bPR Ontart   //
0x0c40, // Th13 aGc Min   //
0x0d58, // Th13 aGc Max   //
0x0e00, // Th1 Max H for aGcMIN    //
0x0f20, // Th1 Max L for aGcMIN    //
0x1000, // Th1 Min H for aGcMaX   //
0x1110, // Th1 Min L for aGcMaX   //
0x1200, // Th3 Max H for aGcMIN   //
0x137f, // Th3 Max L for aGcMIN   //
0x1403, // Th3 Min H for aGcMaX   //
0x15ff, // Th3 Min L for aGcMaX   //
0x1648, // Th57 aGc Min           //
0x1760, // Th57 aGc Max           //
0x1800, // Th5 Max H for aGcMIN   //
0x1900, // Th5 Max L for aGcMIN   //
0x1a00, // Th5 Min H for aGcMaX   //
0x1b20, // Th5 Min L for aGcMaX   //
0x1c00, // Th7 Max H for aGcMIN   //
0x1d00, // Th7 Max L for aGcMIN   //
0x1e00, // Th7 Min H for aGcMaX   //
0x1f20, // Th7 Min L for aGcMaX   //

//================================================ //
//	NR                                         //
//=================================================//
0xfc01,
0x4c01, // NR enable            //
0x4915, //ig_Th Mult          //
0x4b0a, // Pre_Th Mult          //

0xfc0b,
0x2800, // NR start aGc	     //
0x2900, //IG Th aGcMIN H      //
0x2a14, //IG Th aGcMIN L      //
0x2b00, //IG Th aGcMaX H      //
0x2c14, //IG Th aGcMaX L      //
0x2d00, // PRe Th aGcMIN H      //
0x2e80, // a0 90 PRe Th aGcMIN L   //
0x2f00, // PRe Th aGcMaX H      //
0x30e0, // f0 PRe Th aGcMaX L      //
0x3100, // POST Th aGcMIN H     //
0x3290, // POST Th aGcMIN L    //
0x3300, // POST Th aGcMaX H     //
0x34c0, // POST Th aGcMaX L     //

//=================================================//
//	1d-Y////c-SIGMa-LPf                        //
//=================================================//
0xfc01,
0x05c0,
      
0xfc0b,
0x3500, // YLPftart aGc  //
0x3620, // YLPf01 aGcMIN //
0x3750, // YLPf01 aGcMaX //
0x3800, // YLPfIG01 Th aGcMINH  //
0x3910, // YLPfIG01 Th aGcMINL  //
0x3a00, // YLPfIG01 Th aGcMaXH  //
0x3b30, // YLPfIG01 Th aGcMaXH  //
0x3c40, // YLPf02 aGcMIN        //
0x3d50, // YLPf02 aGcMaX        //
0x3e00, // YLPfIG02 Th aGcMINH  //
0x3f10, // YLPfIG02 Th aGcMINL  //
0x4000, // YLPfIG02 Th aGcMaXH  //
0x4140, // YLPfIG02 Th aGcMaXH  //
0xd440, // cLPf aGcMIN          //
0xd550, // cLPf aGcMaX          //
0xd6b0, // cLPfIG01 Th aGcMIN   //
0xd7f0, // cLPfIG01 Th aGcMaX   //
0xd8b0, // cLPfIG02 Th aGcMIN   //
0xd9f0, // cLPfIG02 Th aGcMaX   //

//==================================================//
//	GR// //Gb cORRecTION                        //
//==================================================//
0xfc01,
0x450c,
0xfc0b,
0x2100, // tart aGc     //
0x2226, // aGcMIN        //
0x2360, // aGcMaX        //
0x2414, // G Th aGcMIN   //
0x2520, // G Th aGcMaX   //
0x2614, // Rb Th aGcMIN  //
0x2720, // Rb Th aGcMaX  //

//==================================================//
//	color suppress                              //
//==================================================//
0xfc0b,
0x0858, // coloruppress aGc MIN  //
0x0904, // coloruppress MIN H    //
0x0a00, // coloruppress MIN L    //

//==================================================//
//	Shading                                     //
//==================================================//
0xfc09,
0x0022,
0x0502, // rx //
0x0678,
0x0702, // ry //
0x0838,  
0x0902, // gx //
0x0a75,
0x0b02, // gy //
0x0c1a,  
0x0d02, // bx //
0x0e58,
0x0f01, // by //
0x10ee, 
0x1d10, // R Right //
0x1e10,  
0x1f0f, // R Left //
0x20e0,  
0x210e, // R Top //
0x22ad,  
0x2311, // R bot //
0x2463,  
0x2510, //10 G Right //
0x2600,  
0x2710, // G Left //
0x2800,  
0x2910, // G Top //
0x2a00,  
0x2b10, // G bot //
0x2c00,  
0x2d10, // b Right //
0x2e00,  
0x2f10, // b Left //
0x3000,  
0x3111, // b Top //
0x3233,  
0x330e, // b bot //
0x34e9,          
0x3501,   
0x3604,   
0x3701,   
0x3814,   
0x3901,   
0x3a41,   
0x3b01,   
0x3c79,  
0x3d01,   
0x3e96,  
0x3f01,   
0x40b3,   
0x4101,   
0x42d8,   
0x4301,   
0x44f2,   
0x4501, // G Gain //
0x4600, 
0x4701, // 2  //
0x480b, 
0x4901, // 3  //
0x4a30, 
0x4b01, // 4  //
0x4c5c, 
0x4d01, // 5  //
0x4e73, 
0x4f01, // 6  //
0x508b, 
0x5101, // 7 //
0x52a5, // aa //
0x5301, // G Gain 8 //
0x54bc, // cf //         
0x5501, // b Gain 1 //
0x5614, // 00 //  
0x5701, // 2  //
0x5821, 
0x5901, // 3  //
0x5a40, // 35, 28 //
0x5b01, // 4  //
0x5c67, // 4e //
0x5d01, // 5 //
0x5e76, // 68 //
0x5f01, // 6 //
0x6089, // 76 //
0x6101, // 7 //
0x629d, // 8f //
0x6301, // b Gain 8 //
0x64b1, // a2 //          
0x6500,
0x665d,
0x6790,
0x6801,
0x6976,
0x6a3f,
0x6b03,
0x6c4a,
0x6d0e,
0x6e04,
0x6f7a,
0x7022,
0x7105,
0x72d8,
0x73fd,
0x7407,
0x7566,
0x76a1,
0x7709,
0x7823,
0x790c,
0x7a00,
0x7b59,
0x7c7e,
0x7d01,
0x7e65,
0x7ff9,
0x8003,
0x8125,
0x8271,
0x8304,
0x8448,
0x854b,
0x8605,
0x8797,
0x88e4,
0x8907,
0x8a14,
0x8b3d,
0x8c08,
0x8dbd,
0x8e55,
0x8f00,
0x9055,
0x91ff,
0x9201,
0x9357,
0x94fd,
0x9503,
0x9605,
0x97fa,
0x9804,
0x991d,
0x9a78,
0x9b05,
0x9c5f,
0x9df6,
0x9e06,
0x9fcd,
0xa073,
0xa108,
0xa265,
0xa3f0,
0xa4af,
0xa51d,
0xa63a,
0xa75f,
0xa823,
0xa905,
0xaa35,
0xabe1,
0xac2e,
0xadb2,
0xae29,
0xaf34,
0xb024,
0xb1dd,
0xb2b7,
0xb313,
0xb43d,
0xb506,
0xb624,
0xb79d,
0xb838,
0xb954,
0xba30,
0xbbd1,
0xbc2b,
0xbd13,
0xbe26,
0xbf8a,
0xc0be,
0xc184,
0xc23f,
0xc381,
0xc426,
0xc51a,
0xc63a,
0xc79e,
0xc832,
0xc9cd,
0xca2c,
0xcbd3,
0xcc28,
0xcd1b,

0x0002,
//==================================================//
//	X-Shading                                   //
//==================================================//
0xfc1b,
0x4900,
0x4a41,
0x4b00,
0x4c6c,
0x4d03,
0x4ef0,
0x4f00,
0x5009,
0x517b,
0x5800,
0x59b6,
0x5a01,
0x5b29,
0x5c01,
0x5d73,
0x5e01,
0x5f91,
0x6001,
0x6192,
0x6201,
0x635f,
0x6400,
0x65eb,
0x6600,
0x673a,
0x6800,
0x6973,
0x6a00,
0x6b9a,
0x6c00,
0x6dac,
0x6e00,
0x6fb2,
0x7000,
0x71a1,
0x7200,
0x7370,
0x7407,
0x75e2,
0x7607,
0x77e4,
0x7807,
0x79e0,
0x7a07,
0x7be1,
0x7c07,
0x7de3,
0x7e07,
0x7feb,
0x8007,
0x81ee,
0x8207,
0x837e,
0x8407,
0x8548,
0x8607,
0x8718,
0x8807,
0x8900,
0x8a06,
0x8bfb,
0x8c07,
0x8d1b,
0x8e07,
0x8f58,
0x9007,
0x9113,
0x9206,
0x93b2,
0x9406,
0x9569,
0x9606,
0x974a,
0x9806,
0x9946,
0x9a06,
0x9b73,
0x9c06,
0x9ddc,

0x4801, // x-shading on //

//==================================================//
//	ae Window Weight                            //
//==================================================//
0xfc20,
0x1c00, // 00=flat01=center02=manual //
      
0xfc06,
0x0140, // SXGa ae Window //
0x0398,
0x0548,
0x079a,
0x0938, // SXGa aWb window //
0x0b26,
0x0d50,
0x0f4e,
0x313a, // ubsampling ae Window //
0x3349,
0x3548,
0x3746,
0x3800, // Subsampling aWb Window //
0x3923,
0x3a00,
0x3b13,
0x3c00,
0x3d20,
0x3e00,
0x3f29,

0xfc20,
0x540a, // ae table //

0x6011,
0x6111,
0x6211,
0x6311,
0x6411,
0x6511,
0x6611,
0x6711,
0x6811,
0x6913,
0x6a31,
0x6b11,
0x6c11,
0x6d13,
0x6e31,
0x6f11,
0x7032,
0x7122,
0x7222,
0x7322,
0x7411,
0x7512,
0x7621,
0x7711,

//==================================================//
//	SaIT aWb                                    //
//==================================================//
//==================================================//
//	aWb table Margin //
//==================================================//
0xfc00,
0x8d03,

//==================================================//
//	AWB Offset setting                          //
//==================================================//
0xfc00,
0x79e9, // aWb R Gain offset //
0x7afd, // aWb b Gain offset //
0xfc07,
0x1100, // aWb G Gain offset //

//==================================================//
//	AWB Gain Offset                             //
//==================================================//
0xfc22,
0x5807, // 02 D65 R Offset //
0x59fc, // fe D65 B Offset //
0x5a03, // 5000K R Offset    //
0x5bfe, // 5000K B Offset    //
0x5c06, // 05 CWF R Offset 02   //
0x5d06, // fe CWF B Offset f2   //
0x5e0c, // 3000K R Offset    //
0x5ffc, // 3000K B Offset    //
0x6007, // Incand A R Offset //
0x61fa, // ec Incand A B Offset //                                      
0x620e, // 10 2000K R Offset  //          
0x63ef, // 2000K B Offset   //      
          
//==================================================//
//	AWB basic setting                           //
//==================================================//
0xfc01,
0xced0, // aWb Y Max   //

0xfc00,
0x3d04, // aWb Y_min Low    //
0x3e10, // aWb Y_min_Normal //
0xfc00,
0x3202, // aWb moving average 8 frame //
0xbcf0,

0xfc05,
0x6400, // darkslice R //
0x6500, // darkslice G //
0x6600, // darkslice b //

//=======================================//
// AWB ETC ; recommand after basic coef. //
//=======================================//
0xfc00,
0x8b05,
0x8c05, // added //
0xfc22,
0xde00, // LaRGe ObJecT bUG fIX //
0x70f0, // Greentablizer ratio //
0xd4f0, // Low temperature //
0x9012,
0x9112,
0x9807, // Moving equation Weight //

0xfc22, // Y up down threshold //
0x8c07,
0x8d07,
0x8e03,
0x8f05,
0xfc07,
0x6ba0, // aWb Y Max //
0x6c08, // aWb Y_min //
0xfc00,
0x3206, // aWb moving average 8 frame  //

//==================================================//
//	White Point                                 //
//==================================================//
0xfc22,
0x01df, // d65  df	//
0x039c, // d65  9c	//        
0x05d0, // 5100K  d0  //
0x07ac, // 5100K  ac  //          
0x09c2, // cWf c2	//
0x0bbf, // cWf bf	//               
0x0db5, //Incand a //
0x0e00,
0x0fd5,          
0x119d, //3100K	//
0x1200,
0x13ed,           
0x1588, // HORizon	//
0x1601,
0x171c,

//==================================================//
//	basic setting                               //
//==================================================//
0xfc22,
0xa001,
0xa12B,
0xa20F,
0xa3D8,
0xa407,
0xa5FF,
0xa610,
0xa76c,
0xa901,
0xaaDB,
0xab0E,
0xac1D,
0xad02,
0xaeBA,
0xaf2C,
0xb0B6,
0x9437,
0x9533,
0x9658,
0x9757,
0x6710,
0x6801,
0xd056,
0xd134,
0xd265,
0xd31A,
0xd4FF,
0xdb34,
0xdc00,
0xdd1A,
0xe700,
0xe8C5,
0xe900,
0xea63,
0xeb05,
0xec3D,
0xee78,

//==================================================//
//	Pixel filteretting                          //
//==================================================//
0xfc01,  
0xd94c, //4c  4c// 
0xda00, //00  00// 
0xdb39, //3a  3a// 
0xdc00, //00  00// 
0xdd5c, //5c  5c// 
0xde00, //00  00// 
0xdf4c, //48  48// 
0xe000, //00  00// 
0xe123, //23  25// //color tracking //  
0xe26a, //d6  d1//
0xe325, //25  26//
0xe40d, //4c  42//
0xe523, //21  1f//
0xe67f, //e3  d4//
0xe71b, //1a  19//
0xe8c5, //22  bb//
0xe930, //33  37//
0xea40, //40  40//
0xeb3d, //40  40//
0xec40, //40  40//
0xed40, //40  40//
0xee38, //39  3b//
0xef40, //39  35//
0xf025, //21  21//
0xf100, //00  00//

//==================================================//
//	Polygon aWb Region Tune                     //
//==================================================//
0xfc22, 
0x1800, 
0x1977, 
0x1a9a, 
0x1b00, 
0x1c97, 
0x1d80, 
0x1e00, 
0x1fb6, 
0x2068, 
0x2100, 
0x22ce, 
0x2355, 
0x2400, 
0x25d8, 
0x266c, 
0x2700, 
0x28be, 
0x2980, 
0x2a00, 
0x2ba7, 
0x2c94, 
0x2d00, 
0x2e9d, 
0x2fa4, 
0x3000, 
0x3194, 
0x32ca, 
0x3300, 
0x348c, 
0x35ec, 
0x3600, 
0x3742, 
0x38f9, 
0x3900, 
0x3a4d, 
0x3bbd, 

        
//==================================================//
//	eIT Threshold                               //
//==================================================//
0xfc22,
0xb100, // sunny //
0xb203,
0xb300,
0xb449,
0xb500, // cloudy //
0xb603,
0xb700,
0xb850,
0xd7ff, // large object //
0xd8ff,
0xd9ff,
0xdaff,

//==================================================//
//	aux Window set                              //
//==================================================//
0xfc22,
0x7a00,
0x7b00,
0x7cc0,
0x7d70,
0x7e0e,
0x7f00,
0x80aa,
0x8180,
0x8208,
0x8300,
0x84c0,
0x8570,
0x8608,
0x8700,
0x88c0,
0x8970,
0x8a0e,

//==================================================//
//	aWb Option                                  //
//==================================================//
0xfc22,
0xbd84, 

//==================================================//
//	Special effect                              //
//==================================================//
0xfc07,
0x30c0, // epia cr //
0x3120, // epia cb //
0x3240, // aqua cr //
0x33c0, // aqua cb //
0x3400, // Green cr//
0x35b0, // Green cb//

0xfc00,
0x0208,
0x7504, // 0x06// org - 7500

//---------------------------------------//
// case 21 : Preview                     //
//---------------------------------------//
0xfc00,
0x034b, 
0xfc02,
0x3c00,
0x020e,  // cIS bPR  //

0xfc01,
0x3e15,  // bPR Th1 //
0x3f7f,
0x4000,
0x4100,
0x4200,
0x4300,
0x4400,
};

static u16 INIT_VT_DATA[] ={
//============================================================//
//  Name     :  for 5K5aafa_Stealth-V VT mode       //
//  Version :  v0.s                                      //
//  PLL mode :  PcLK 24 MHz (not using PLL)              //
//  aGc gain :  68                                            //
//  fPs      :  8 fps fix 640 x 512				                            //
//============================================================//

//==================================================//
//	caMeRa INITIaL                                  //
//==================================================//
0xfc02,                             
0x3100, // CFPN OFF              //   
0x3204, // CFPN start Frame    //                                                                           
0xfc00,                             
0x00aa,                             
0xfc07,                             
0x6601,                             
0xfc01,                             
0x0401,                                                         
0xfc07,                             
0x0900,                             
0x0ab3,                             
0x0b00,                             
0x0ca3,                             
0x0e40,                             
0xfc00,                              
0x7042,

//==================================================//
//	Clock setting                             //
//==================================================//
0xfc07,
0x3700, // Main clock precision //

0xfc00,
0x7278, // 24Mhz  //
                                                     
//==================================================//
//	cIs setting                                 //
//==================================================//
0xfc02,
0x2e82, // analog offset(default 80)              //
0x2f00, // b channel line adlc tuning register    //
0x3000, // gb channel line adlc tuning register   //
0x3718, // Global Gain(default 18)                //
0x44db, // cLP_eN on                              //
0x45f0, // 1R eNd1s eNd time(ff)                 //
0x472f, // 3_s4 adrs                              //
0x4a00, // clamp                                  //
0x4b11, // Ramp current                           //
0x4d02, // dbLR Tune                              //
0x4e2d, // sCL_sDA_CTRL_ADRs                      //
0x4faa, // noise check                            //
0x551f, // channel Line adLc on                   //
0x5700, // R channel line adlc tuning register    //
0x5800, // gr channel line adlc tuning register   //
0x5b0f, // Ob_Ref                                 //
0x6001, // sT_en                                  //
0x620d, // lew 03                                 //
0x6322, // cds aMP current                        //
0x2d58, // double shutter                         //
0x3100, // cfPN Off                               //
                                                  
//==================================================//
//	shutter suppress                            //
//==================================================//       
0xfc00,
0x8300, // shutter off double shutter off  it would influent flicker //
0xfc0b,
0x5c60,
0x5d60,     

//==================================================//
//	Table set for Preview                       //
//==================================================//
0xfc04, // page 04h                  //     
0x9005, // cis_frame_h_width_h       //     
0x91b2, // cis_frame_h_width_l       //     
0x9204, // cis_frame_v_depth_h       //     
0x9310, // cis_frame_v_deoth_l       //     
0x9402, // cis_output_h_width_h      //     
0x9588, // cis_output_h_width_l      // 
0x9602, // cis_output_v_depth_h      // 
0x9708, // cis_output_v_depth_l      // 
0x9801, // cis_h_even_inc_l          // 
0x9903, // cis_h_odd_inc_l           // 
0x9a01, // cis_v_even_inc_l          // 
0x9b03, // cis_v_odd_inc_l           // 
0x9c08, // cis_mirr_etc(average sub) // 
0x9d00, // dsp5_post_hstart_h        // 
0x9e06, // dsp5_post_hstart_l        // 
0x9f02, // dsp5_post_hwidth_h        // 
0xa080, // dsp5_post_hwidth_l        // 
0xa100, // dsp5_post_vstart_h        // 
0xa206, // dsp5_post_vstart_l        // 
0xa302, // dsp5_post_vheight_h       // 
0xa400, // dsp5_post_vheight_l       // 
0xa502, // dsp5_img_hsizeh           // 
0xa680, // dsp5_img_hsizel           // 
0xa702, // dsp5_img_vsizeh           // 
0xa800, // dsp5_img_vsizel           // 
0xa900, // dsp5_psf                  // 
0xaa00, // dsp5_msfx_h               // 
0xab00, // dsp5_msfx_l               // 
0xac00, // dsp5_msfy_h               // 
0xad00, // dsp5_msfy_l               // 
0xae02, // dsp5_dw_hsizeh            // 
0xaf80, // dsp5_dw_hsizel            // 
0xb002, // dso5_dw_vsizeh            // 
0xb100, // dsp5_dw_vsizel            // 
0xb200, // dsp5_startxh              // 
0xb300, // dsp5_startxl              // 
0xb400, // dsp5_startyh              // 
0xb500, // dsp5_startyl              // 
0xb602, // dsp5_clip_hsizeh          // 
0xb780, // dsp5_clip_hsizel          // 
0xb802, // dsp5_clip_vsizeh          // 
0xb900, // dsp5_clip_vsizel          // 
0xba00, // dsp5_sel_main             // 
0xbb00, // dsp5_htermh               // 
0xbc05, // dsp5_htermm               // 
0xbdb2, // dsp5_hterml               // 
0xbe03, // dsp1_crcb_sel             // 
0xbf30, // ozone                     // 
                                            
//==================================================//
//	COMMAND setting (not modified)            //
//==================================================//
0xfc00,                                                                                                                                                                                        
0x034b,                                                                                                                                                                                        
0x2904, // Brightness_H                                     //                                                                                                                                 
0x2a00, // Brightness_L                                     //                                                                                                                                 
0x2b04, // color level H                                        //                                                                                                                              
0x2c0a, // color level L //                                     //                                                                                                                               
0x620a, // AWB_Average_Num                                      //                                                                                                                               
0x6ca0,	//94 //AE target_L 0325                                 //                                                                                                                                                                                      
0x6d00, //AE target_H                                           //
0x7301, //Frame Fixed                                           //                                                                                                                               
0x7418, //flicker 60Hz fix                                      //                                                                                                                               
0x7878, // 78 AGC Max                                               //                                                                                                                               
0x8190, //Mirror X shading on Ggain offset off EIT GAMMA ON //                                                                                                                                   
0x0704, //                                                      //                                                                                                                                                                                                           
0x0802, // 8b                                                     //
0xfc20, //                                                      //                                                                                                                               
0x010a, // stepless_Off	new shutter                   //                                                                                                                                      
0x0212, //06 Flicker Dgain Mode	                                //                                                                                                                               
0x0302, //                                                      //                                                                                                                               
0x1001, //1 shutter Max                                       //                                                                                                                                
0x1112, //45 for new LED spec (40ms = 0170h)	        //                                                                                                                                       
0x1650, //58 Frame AE start (Gain 2.9 =54h)                   //                                                                                                                                
0x2501, //Cintr Min	                                        //                                                                                                                                   
0x2ca0, //Forbidden cintc	                                //                                                                                                                                     
0x550a, //agc1                                                  //                                                                                                                               
0x560a, //agc2                                                  //                                                                                                                               
0x570c, //stable_Frame_AE	                                //                                                                                                                                   
                                                                                            
//==================================================//
//	IsP setting (not modified)              //
//==================================================//
0xfc01,  
0x0101,  // PCLK Inversion     //
0x0c02,  // Full YC Enable       //                                      
//==================================================//
//	Color Matrix                                //
//==================================================//   
0xfc01,   
0x5107,   
0x5240,   
0x53fc,   
0x5480,   
0x5500,   
0x563f,   
0x57fe,   
0x587f,   
0x5906,   
0x5a6a,   
0x5bff,   
0x5c18,   
0x5dff,   
0x5ed7,   
0x5ffd,   
0x601c,   
0x6107,   
0x620d,   
       
//==================================================//
//	Edge Enhancement (not modified)             //
//==================================================//
0xfc00, //                                                           //
0x8903, //  Edge Mode on                                             //
0xfc0b, //                                                           //
0x4250, // 30  Edge AGC MIN                                          //
0x4360, // 50  Edge AGC MAX                                          //
0x451a, //  Positive gain AGC MIN     //                           // 
0x4908, //  Positive gain AGC MAX //                               // 
0x4d1a, // 18 14 Negative gain AGC MIN     //                        //
0x5108, //  Negative gain AGC MAX //                                 //
0xfc05, //                                                           //
0x3420, //  YAPTCLP:Y Edge Clip                                      //
0x3518, //  YAPTsC:Y Edge Noise slice 10->8                    //   
0x3612, //  ENHANCE                                                  //
0x3f00, //  NON-LIN                                                  //
0x4041, //  Y delay                                                  //
0x4210, //  EGFALL:Edge Color suppress Gain slope            //    
0x4300, //  HLFALL:High-light Color suppress Gain slope      //    
0x45a0, //  EGREF:Edge Color suppress Reference Thres.       //    
0x467a, //  HLREF:High-light Color suppress Reference Thres. //    
0x4740, //  LLREF:Low-light Color suppress Reference Thres.  //    
0x480c, //  [5:4]Edge[3:2]High-light[1:0]Low-light                   //
0x4931, //  CssEL  EGsEL  Cs_DLY                             //    
                                                                      
//==================================================//
//	Gamma setting                             //
//==================================================//
0xfc0c,  // outdoor    INDOOR     //          
0x0009,                              
0x0120,                              
0x027d,                              
0x0315,                              
0x0401,                              
0x05c3,                              
0x0636,                              
0x077f,                              
0x08b8,                              
0x096a,                              
0x0ae8,                              
0x0b17,                              
0x0c3e,                              
0x0d66,                              
0x0ebf,                              
0x0f82,                              
0x109b,                              
0x11b4,                              
0x12ca,                              
0x13ff,                              
0x14df,                              
0x15f0,                              
0x16ff,                              
0x17fc,                              
0x180a,                              
0x1920,                              
0x1a7d,                              
0x1b15,                              
0x1C01,                              
0x1dc3,                              
0x1e36,                              
0x1f7f,                              
0x20b8,                              
0x216a,                              
0x22e8,                              
0x2317,                              
0x243e,                              
0x2566,                              
0x26bf,                              
0x2782,                              
0x289b,                              
0x29b4,                              
0x2aca,                              
0x2bff,                              
0x2cdf,                              
0x2df0,                              
0x2eff,                              
0x2ffc,                              
0x300a,                              
0x3120,                              
0x327d,                              
0x3315,                              
0x3401,                              
0x35c3,                              
0x3636,                              
0x377f,                              
0x38b8,                              
0x396a,                              
0x3ae8,                              
0x3b17,                              
0x3c3e,                              
0x3d66,                              
0x3ebf,                              
0x3f82,                              
0x409b,                              
0x41b4,                              
0x42ca,                              
0x43ff,                              
0x44df,                              
0x45f0,                              
0x46ff,                              
0x47fc,                              

0xfc0d,     // LOWLUX  //                    
0x0062,                            
0x0191,                            
0x02d9,                            
0x0347,                            
0x0401,                            
0x05df,                            
0x063f,                            
0x0784,                            
0x08be,                            
0x096a,                            
0x0af0,                            
0x0b20,                            
0x0c40,                            
0x0d60,                            
0x0ebf,                            
0x0f7d,                            
0x1094,                            
0x11a8,                            
0x12b8,                            
0x13ff,                            
0x14c8,                            
0x15d6,                            
0x16e0,                            
0x17fc,                            
0x1862,                            
0x1991,                            
0x1ad9,                            
0x1b47,                            
0x1c01,                            
0x1ddf,                            
0x1e3f,                            
0x1f84,                            
0x20be,                            
0x216a,                            
0x22f0,                            
0x2320,                            
0x2440,                            
0x2560,                            
0x26bf,                            
0x277d,                            
0x2894,                            
0x29a8,                            
0x2ab8,                            
0x2bff,                            
0x2cc8,                            
0x2dd6,                            
0x2ee0,                            
0x2ffc,                            
0x3062,                            
0x3191,                            
0x32d9,                            
0x3347,                            
0x3401,                            
0x35df,                            
0x363f,                            
0x3784,                            
0x38be,                            
0x396a,                            
0x3af0,                            
0x3b20,                            
0x3c40,                            
0x3d60,                            
0x3ebf,                            
0x3f7d,                            
0x4094,                            
0x41a8,                            
0x42b8,                            
0x43ff,                            
0x44c8,                            
0x45d6,                            
0x46e0,                            
0x47fc,                            

//==================================================//
//	Hue setting                               //
//==================================================//   

0xfc00,  
0x484a,  //4a  
0x494c,  //4c  
0x4afb,  //fb  
0x4b05,  //05  
0x4c68,  //68  
0x4d66,  //66  
0x4eed,  //ed  
0x4ff3,  //f3 
      
0x5048,  //48  
0x514c,  //4c  
0x52fb,  //fb  
0x5305,  //05  
0x5468,  //68  
0x5566,  //66  
0x56ed,  //ed  
0x57f3,  //f3 
       
0x5844,  //3e  
0x594c,  //4c  
0x5aff,  //fb  
0x5b05,  //05  
0x5c6e,  //68  
0x5d44,  //66  
0x5eed,  //ed  
0x5ff5,  //f3  
        
//==================================================//
//	suppress Functions (not modified)   //
//==================================================//      
0xfc00,                                                 
0x7ef4,                                      
                                                         
//==================================================//
//	BPR (not modified)                          //
//==================================================//  
0xfc01,  
0x3d10,
0xfc0b,  
0x0b00,
0x0c40,
0x0d58,
0x0e00,
0x0f20,
0x1000,
0x1110,
0x1200,
0x137f,
0x1403,
0x15ff,
0x1648,
0x1760,
0x1800,
0x1900,
0x1a00,
0x1b20,
0x1c00,
0x1d00,
0x1e00,
0x1f20,

//==================================================//
//	NR                                          //
//==================================================//
0xfc01,
0x4c01,  // 01 //
0x4914,  // 15 //
0x4b0a,  // 0a //
0xfc0b,  // 0b //
0x2800,  // 00 //
0x2900,  // 00 //
0x2a14,  // 14 //
0x2b00,  // 00 //
0x2c14,  // 14 //
0x2d00,  // 00 //
0x2e80,  // 90 //
0x2f00,  // 01 00//
0x30c0,  // 7b c0//
0x3100,  // 00 //
0x3280,  // a0 //
0x3300,  // 00 00//
0x34c0,  // d0 c0//
                                  
//==================================================//
//	1D-Y/C-sIGMA-LPF   (not modified)         //
//==================================================// 
0xfc01,
0x05c0,
0xfc0b,
0x3500,
0x3620,
0x3750,
0x3800,
0x3910,
0x3a00,
0x3b50,
0x3c30,
0x3d50,
0x3e00,
0x3f20,
0x4000,
0x4150,
0xd440,
0xd550,
0xd6b0,
0xd7f0,
0xd8b0,
0xd9f0,

//==================================================//
//	GR/GB CORRECTION                            //
//==================================================// 
0xfc01, 
0x450c, 
0xfc0b, 
0x2100, 
0x2240, 
0x2350, 
0x2410, 
0x2520, 
0x2610, 
0x2720, 
                           
//==================================================//
//	Color suppress                        //
//==================================================//
0xfc0b,
0x0850,
0x0902,
0x0a70,

//==================================================//
//	shading                                   //
//==================================================//
0xfc09,   
0x0105,   
0x0200,   
0x0304,   
0x0400,   
0x0502,                   
0x065a,                   
0x0702,                   
0x084a,                   
0x0902,                   
0x0a61,                   
0x0b02,                   
0x0c06,                   
0x0d02,                   
0x0e4c,                   
0x0f01,                   
0x10f2,                   
0x1d0f,                   
0x1edc,                   
0x1f10,                   
0x2000,                   
0x210f,                   
0x228b,                   
0x230f,                   
0x24b4,                   
0x250f,                   
0x2618,                   
0x2710,                   
0x2800,                   
0x2910,                   
0x2a00,                   
0x2b0e,                   
0x2c69,                   
0x2d10,                   
0x2e00,                   
0x2f10,                   
0x307e,                   
0x3110,                   
0x3200,                   
0x330d,                   
0x34ac,                   
0x3501,                   
0x3600,                   
0x3701,                   
0x380b,                   
0x3901,                   
0x3a25,                   
0x3b01,                   
0x3c53,                   
0x3d01,                   
0x3e6a,                   
0x3f01,                   
0x408a,                   
0x4101,                   
0x42ad,                   
0x4301,                   
0x44d0,                   
0x4501,                   
0x4600,                   
0x4701,                   
0x4806,                   
0x4901,                   
0x4a1e,                   
0x4b01,                   
0x4c3c,                   
0x4d01,                   
0x4e4e,                   
0x4f01,                   
0x5064,                   
0x5101,                   
0x5282,                   
0x5301,                   
0x54a4,                   
0x5501,                   
0x5600,                   
0x5701,                   
0x580b,                   
0x5901,                   
0x5a19,                   
0x5b01,                   
0x5c30,                   
0x5d01,                   
0x5e3b,                   
0x5f01,                   
0x6048,                   
0x6101,                   
0x6262,                   
0x6301,                   
0x647c,                   
0x6500,                   
0x665f,                   
0x67c4,                   
0x6801,                   
0x697f,                   
0x6a11,                   
0x6b03,                   
0x6c5d,                   
0x6de6,                   
0x6e04,                   
0x6f95,                   
0x7023,                   
0x7105,                   
0x72fc,                   
0x7343,                   
0x7407,                   
0x7593,                   
0x7645,                   
0x7709,                   
0x785a,                   
0x7929,                   
0x7a00,                   
0x7b59,                   
0x7c22,                   
0x7d01,                   
0x7e64,                   
0x7f87,                   
0x8003,                   
0x8122,                   
0x8230,                   
0x8304,                   
0x8443,                   
0x85de,                   
0x8605,                   
0x8792,                   
0x881d,                   
0x8907,                   
0x8a0c,                   
0x8bec,                   
0x8c08,                   
0x8db4,                   
0x8e4d,                   
0x8f00,                   
0x905a,                   
0x9167,                   
0x9201,                   
0x9369,                   
0x949e,                   
0x9503,                   
0x962d,                   
0x97a3,                   
0x9804,                   
0x9953,                   
0x9a73,                   
0x9b05,                   
0x9ca6,                   
0x9d76,                   
0x9e07,                   
0x9f26,                   
0xa0ae,                   
0xa108,                   
0xa2d4,                   
0xa319,                   
0xa4ab,                   
0xa515,                   
0xa639,                   
0xa707,                   
0xa822,                   
0xa937,                   
0xaa34,                   
0xaba4,                   
0xac2d,                   
0xad9f,                   
0xae28,                   
0xaf41,                   
0xb024,                   
0xb104,                   
0xb2b7,                   
0xb3d1,                   
0xb43d,                   
0xb545,                   
0xb624,                   
0xb7c3,                   
0xb838,                   
0xb98f,                   
0xba31,                   
0xbb04,                   
0xbc2b,                   
0xbd40,                   
0xbe26,                   
0xbfb2,                   
0xc0b5,                   
0xc13b,                   
0xc23c,                   
0xc369,                   
0xc424,                   
0xc53f,                   
0xc637,                   
0xc7c3,                   
0xc830,                   
0xc954,                   
0xca2a,                   
0xcba4,                   
0xcc26,                   
0xcd27,                   
0x0002,  // shading on    //
                    
//==================================================//  
//	X-shading                                   //
//==================================================//    
0xfc1B,
0x4900,
0x4A41,
0x4B00,
0x4C6C,
0x4D03,
0x4EF0,
0x4F00,
0x5009,
0x517B,
0x5800,
0x59B6,
0x5A01,
0x5B29,
0x5C01,
0x5D73,
0x5E01,
0x5F91,
0x6001,
0x6192,
0x6201,
0x635F,
0x6400,
0x65EB,
0x6600,
0x673A,
0x6800,
0x6973,
0x6A00,
0x6B9A,
0x6C00,
0x6DAC,
0x6E00,
0x6FB2,
0x7000,
0x71A1,
0x7200,
0x7370,
0x7407,
0x75E2,
0x7607,
0x77E4,
0x7807,
0x79E0,
0x7A07,
0x7BE1,
0x7C07,
0x7DE3,
0x7E07,
0x7FEB,
0x8007,
0x81EE,
0x8207,
0x837E,
0x8407,
0x8548,
0x8607,
0x8718,
0x8807,
0x8900,
0x8A06,
0x8BFB,
0x8C07,
0x8D1B,
0x8E07,
0x8F58,
0x9007,
0x9113,
0x9206,
0x93B2,
0x9406,
0x9569,
0x9606,
0x974A,
0x9806,
0x9946,
0x9A06,
0x9B73,
0x9C06,
0x9DDC,
    
0x4801, // x-shading on  // 

//==================================================//
//	AE Window Weight                            //
//==================================================//   

0xfc20,                                               
0x1c00,   
0xfc06,      
0x0140,                                               
0x0398,                                               
0x0548,                                               
0x079A,                                               
0x0910,                                               
0x0b27,                                               
0x0d3c,                                                        
0x0f50,                                               
0x313e,                                              
0x3349,                                                
0x3518,                                                           
0x3744,                                                
0x3800,                                                
0x390a,                                                  
0x3a00,                                               
0x3b13,                                               
0x3c00,                                                  
0x3d10,                                                  
0x3e00,                                                  
0x3f28,                                                  
0xfc20,                                                  
0x540a,                                                  
0x6011,                                                  
0x6111,                                                  
0x6211,                                               
0x6311,                                               
0x6411,                                               
0x6511,                                                  
0x6611,                                                  
0x6711,                                                  
0x6811,                                                  
0x6944,                                                  
0x6a44,                                                  
0x6b11,                                                  
0x6c11,                                                  
0x6d44,                                                  
0x6e44,                                                  
0x6f11,                                                  
0x7022,                                                  
0x7144,                                                  
0x7244,                                                  
0x7322,                                                  
0x7422,                                               
0x7522,                                                  
0x7622,                                                        
0x7722,           
                                         
//==================================================//
//	AWB table Margin                            //
//==================================================//
0xfc00,
0x8d03,
      
//==================================================//
//	AWB Offset setting                        //
//==================================================//
0xfc00,                                                     
0x79f5,     // AWB R Gain offset f6        //               
0x7a0f,     // AWB B Gain offset 08        //               
0xfc07,                                                     
0x1100,     // AWB G Gain offset 01        //
                                                      
//==================================================//   
//	AWB Gain Offset                           //       
//==================================================//  
0xfc22,                                                      
0x58f7,	// D65 R Offset             //              
0x59f7,	// D65 B Offset             //
0x5Af7,	// 5000K R Offset           // 
0x5Bf7,	// 5000K B Offset           //                
0x5C00,	// CWF R Offset             // 
0x5Dfd,	// CWF B Offset             // 
0x5Efc,	// 3000K R Offset           //                  
0x5Ffd,	// 3000K B Offset           // 
0x6000,	// Incand A R Offset        //    
0x61fd,	// Incand A B Offset        //                   
0x620f,	// 2000K R Offset  13->19   //
0x63fd,	// 2000K B Offset           //       
                                                            
//==================================================//
//	AWB Basic setting                       //
//==================================================//         
0xfc01,                                                                            
0xced0,        
0xfc00,            
0x3d04,         
0x3e10,         
0xfc00,            
0x3202,          
0xbcf0,            
0xfc05,            
0x6400,        
0x6500,                    
0x6600,   
                                                      
//========================================//
// AWB ETC ; recommand after basic coef.  //
//========================================//
0xfc00,                                                 
0x8b05,
0x8c05,  //  added//                                  
0xfc22,  //                             //                
0xde00,  //  LARGE OBJECT BUG FIX       //             
0x70f0,  //  Greentablizer ratio        //           
0xd4f0,  //  Low temperature            //              
0x9012,  //                             //                
0x9112,  //                             //                
0x9807,  //  Moving Equation Weight     //             
0xfc22,  //  Y up down threshold      //               
0x8c07,  //                             //                
0x8d07,  //                             //                
0x8e03,  //                             //                
0x8f05,  //                             //                
0xfc07,  //                             //                
0x6ba0,  //  AWB Y Max                  //          
0x6c08,  //  AWB Y_min                  //          
0xfc00,  //                             //                
0x3206,  //  AWB moving average 8 frame //
                                                      
//==================================================   
//	White Point                                  
//==================================================  
0xfc22,                                                  
0x01df,  //  D65	//      
0x039c,                 
0x05d0,  //  5100K  //	    
0x07ac,                 
0x09c2,  //  CWF	 //      
0x0bbf,                 
0x0db5,  // Incand A  //	
0x0e00,                 
0x0fcd,                 
0x119d,  //  3100K	 //  
0x1200,                 
0x13ed,                 
0x158a,  // HORizon  // 
0x1601,                                               
0x1705,                                                      
                                                      
//==================================================//
//	Basic setting                               //  
//==================================================//  
0xfc22,
0xA001,
0xA120,
0xA20F,
0xA3D8,
0xA407,
0xA5FF,
0xA610,
0xA728,
0xA901,
0xAADB,
0xAB0E,
0xAC1D,
0xAD02,
0xAEBA,
0xAF2C,
0xB0B6,
0x9437,
0x9533,
0x9658,
0x9757,
0x6710,
0x6801,
0xD056,
0xD134,
0xD265,
0xD31A,
0xD4B7,
0xDB34,
0xDC00,
0xDD1A,
0xE700,
0xE8C5,
0xE900,
0xEA63,
0xEB05,
0xEC3D,
0xEE78,
      
//==================================================//   
//	Pixel Filter setting                         //         
//==================================================// 
0xfc01, 					
0xd94a, // red max  //
0xda80,
0xdb3b, // red min  //
0xdc80, 
0xdd5b, //  b max   //
0xde00,
0xdf4c, //  b min   //
0xe080, 
0xe11a,
0xe202,
0xe321,
0xe401,
0xe524,
0xe612,
0xe71b,
0xe8ea,
0xe940,
0xea40,
0xeb40,
0xec40,
0xed17,
0xee2b,
0xef3f,
0xf026,
0xf100,         
 
//==================================================//  
//	Polygon AWB Region Tune                     //            
//==================================================//
0xfc22,
0x1800, 
0x194d, 
0x1afa, 
0x1b00, 
0x1c4f, 
0x1dbf, 
0x1e00, 
0x1f74, 
0x2097, 
0x2100, 
0x228d, 
0x2381, 
0x2400, 
0x25be, 
0x2663, 
0x2700, 
0x28e8, 
0x294d, 
0x2a00, 
0x2be9, 
0x2c65, 
0x2d00, 
0x2ecd, 
0x2f77, 
0x3000, 
0x31ab, 
0x3290, 
0x3300, 
0x34a0, 
0x359d, 
0x3600, 
0x379a, 
0x38ab, 
0x3900, 
0x3a90, 
0x3be2, 
0x3c00, 
0x3d00, 
0x3e00, 
0x3f00, 
0x4000, 
0x4100, 
0x4200, 
0x4300, 
0x4400, 

//==================================================//  
//	EIT Threshold                               //  
//==================================================//
0xfc22,                      
0xb100, // sunny test         //
0xb202, // 04 03 -> 02        //         
0xb30d, // 00 0d              //    
0xb44f, // 00 4f              //     
0xb500, //  Cloudy영역줄임    //
0xb602, //  03 -> 02          //      
0xb70d, // 0d                 //
0xb850, //                    //
0xd7ff, //  large object//   
0xd8ff,
0xd9ff,
0xdaff,
        
//==================================================//  
//	Aux Window set                              //   
//==================================================//  
0xfc22,
0x7a00,
0x7b00,
0x7cc0,
0x7d70,
0x7e0e,
0x7f00,
0x80aa,
0x8180,
0x8208,
0x8300,
0x84c0,
0x8570,
0x8608,
0x8700,
0x88c0,
0x8970,
0x8a0e,

//==================================================//  
//	AWB Option                                  //
//==================================================// 
0xfc22, 
0xbd84,  // outdoor classify enable White Option enable Greentablilizer enable    // 
       
//==================================================//
//	special Effect                              // 
//==================================================//
0xfc07,	
0x30c0, // sepia Cr  //
0x3120, // sepia Cb  //
0x3240, // Aqua Cr     //
0x33c0, // Aqua Cb     //
0x3400, // Green Cr    //
0x35b0, // Green Cb    //
0xfc00,   
0x2500, 

0xfc05,
0x6405,
0x6505,
0x6607,


0xfc00,
0x0208,  //  Mode Change //

0xfc00,
0x7504, //06,

};

static u16 DTP_DATA[] ={ 
0xfc00,
0x0200,
0xfc20,
0x6011,
0x6111,
0x6211,
0x6311,
0x6411,
0x6511,
0x6611,
0x6711,
0x6811,
0x6911,
0x6a11,
0x6b11,
0x6c11,
0x6d11,
0x6e11,
0x6f11,
0x7011,
0x7111,
0x7211,
0x7311,
0x7411,
0x7511,
0x7611,
0x7711,
0x7811,
0x7911,
0x7a11,
0x7b11,
0x7c11,
0x7d11,
0x7e11,
0x7f11,
0xfc01,
0x5104,
0x5200,
0x5300,
0x5400,
0x5500,
0x5600,
0x5700,
0x5800,
0x5904,
0x5a00,
0x5b00,
0x5c00,
0x5d00,
0x5e00,
0x5f00,
0x6000,
0x6104,
0x6200,
0xfc01,
0x6f08,
0x7010,
0x7120,
0x7240,
0x7300,
0x7480,
0x75c0,
0x7600,
0x7740,
0x7805,
0x7980,
0x7ac0,
0x7b00,
0x7c40,
0x7d5a,
0x7e80,
0x7fc0,
0x8000,
0x8140,
0x82af,
0x8380,
0x84c0,
0x85ff,
0x86fc,
0x8708,
0x8810,
0x8920,
0x8a40,
0x8b00,
0x8c80,
0x8dc0,
0x8e00,
0x8f40,
0x9005,
0x9180,
0x92c0,
0x9300,
0x9440,
0x955a,
0x9680,
0x97c0,
0x9800,
0x9940,
0x9aaf,
0x9b80,
0x9cc0,
0x9dff,
0x9efc,
0x9f08,
0xa010,
0xa120,
0xa240,
0xa300,
0xa480,
0xa5c0,
0xa600,
0xa740,
0xa805,
0xa980,
0xaac0,
0xab00,
0xac40,
0xad5a,
0xae80,
0xafc0,
0xb000,
0xb140,
0xb2af,
0xb380,
0xb4c0,
0xb5ff,
0xb6fc,
0xfc00,
0x4840,
0x4940,
0x4a00,
0x4b00,
0x4c40,
0x4d40,
0x4e00,
0x4f00,
0x5040,
0x5140,
0x5200,
0x5300,
0x5440,
0x5540,
0x5600,
0x5700,
0x5840,
0x5940,
0x5a00,
0x5b00,
0x5c40,
0x5d40,
0x5e00,
0x5f00,
0xfc00,
0x6200,
0xfc00,
0x7bff,
0x2904,
0x2a00,
0x2b04,
0x2c00,
0x7900,
0x7a00,
0xfc07,
0x1100,
0xfc05,
0x4e00,
0x4f00,
0x5044,
0x5100,
0x5200,
0x5300,
0x5400,
0x5500,
0x5644,
0x5700,
0x5800,
0x5900,
0x6780,
0x6840,
0x6980,
0xfc00,
0x7b00,
0xfc09,
0x0000,
0xfc00,
0x0348,
0xfc00,
0x0201,
0xfc01,
0x0203,
0x3d11,
};       

static u16 OP_PREVIEW_DATA[] = {
0xfc01, 
0x0c02, // full YC on // 
      
0xfc00, // aWb ON //
0x034b, // aWb ON //
      
0xfc02,        
0x3718, 
0x45f0,           
0x472f,
0x2d58,            
0x6001,      
      
0xfc00,
0x483e, //  3e RC 2000K // 
0x4940, //  40 GC//        
0x4afe, //  fe RH//        
0x4b0a, //  0a GH//        
0x4c42, //  42 BC//        
0x4d56, //  3e YC//        
0x4ee4, //  e4 BH//        
0x4ff7, //  f7 YH//        
      
0x5034, //  3e RC 3000K //    
0x5138, //  40 GC//          
0x5202, //  fe RH//              
0x53fc, //  0a GH//          
0x5438, //  42 BC//          
0x5546, //  46 YC//          
0x56f4, //  e4 BH//          
0x57eb, //  f7 YH//         
      
0x5834, //  34 RC5100K // 
0x5935, //  28 GC//               
0x5a00, //  08 RH//                
0x5b08, //  10 GH//           
0x5c4a, //  52 BC//                
0x5d30, //  38 YC//               
0x5ee0, //  e0 BH//                    
0x5ff7, //  f7 YH//  
   
0xfc05,
0x6400,
0x6500,
0x6600,         
      
0xfc20,            
0x1480,              
      
0xfc00, 
0x0208, // 640 x512 //
0x7311, // Frame AE //   
0x7ef4, // uppress ON //
0x8306,
0x8903, // edgeupress ON //
      
0xfc02,
0x3c00,
0x020e,   // CIS BPR //
      
0xfc01,
0x3e15,   // BPR Th1 //
0x3f7f,
0x4000,
0x4100,
0x4200,
0x4300,
0x4400,
      
0xfc00,
0x0208,
};

static u16 OP_NORMAL_LIGHT_PREVIEW_DATA[] = {
//hue setting for indoor//
0xfc00,
0x5034, //  3e RC 3000K //    
0x5138, //  40 GC//          
0x5202, //  fe RH//              
0x53fc, //  0a GH//          
0x5438, //  42 BC//          
0x5546, //  46 YC//          
0x56f4, //  e4 BH//          
0x57eb, //  f7 YH//          
      
0x5834, //  34 RC5100K // 
0x5935, //  28 GC//               
0x5a00, //  08 RH//                
0x5b08, //  10 GH//           
0x5c4a, //  52 BC//                
0x5d30, //  38 YC//               
0x5ee0, //  e0 BH//                    
0x5ff7, //  f7 YH// 
};

static u16 OP_HIGH_LIGHT_PREVIEW_DATA[] = {
//hue setting for outdoor//
0xfc00,
0x503e, //  RC 3000K //    
0x5140, //  GC//          
0x52fe, //  RH//              
0x530a, //  GH//          
0x5442, //  BC//          
0x5536, //  YC//          
0x56e4, //  BH//          
0x57fc, //  YH//          
      
0x5846, //  RC5100K // 
0x5950, //  GC//               
0x5af2, //  RH//                
0x5bfb, //  GH//           
0x5c50, //  BC//                
0x5d34, //  YC//               
0x5ed2, //  BH//                    
0x5f02, //  YH//  
};

static u16 OP_CAPTURE_DATA[] = {
0xfc00, 
0x0348, //4B//
0x7301, // Frame AE //   
      
0xfc20,                                   
0x1464, 
      
0xfc02,
0x3725,        
0x470f,
0x2d00,
0x6001,       
      
0xfc00,
0x0200,
};

static u16 OP_MIDLIGHT_CAPTURE_DATA[] = {
0xfc00,
0x0348,
0x7301, // Frame AE //
      
0xfc20,
0x1475,
      
0xfc02,
0x3725,
0x45f0,
0x470f,
0x2d00,
0x6001,
      
0xfc00,
0x0200, 
};

static u16 OP_MID_LOW_LIGHT_CAPTURE_DATA[] = {
0xfc00,
0x0348,
0x7301, // Frame AE //
      
0xfc20,
0x1480,
      
0xfc02,
0x3725,
0x45f0,
0x470f,
0x2d00,
0x6001,

0xfc00,
0x0200,
};

static u16 OP_LOW_LIGHT_CAPTURE_DATA[] = {
0xfc01, 
0x0c00, // full YC off //
      
0xfc01,       
0x6F2f, // R gamma //     
0x7052,
0x718f,
0x72e4,
      
0x8723, // G gamma //    
0x8846,
0x8983,
0x8Ad8,
      
0x9F23, // B gamma  //   
0xA046,
0xA183,
0xa2d8,
      
0xfc00, 
0x7301,
0x0348, // AE/AWB Off //
0x7e00,
0x8900,
      
0xfc02,
0x3725, // 25 //
0x45f0,
0x470f,
      
0xfc20,
0x145a, // 50 //
      
0xfc00,
0x0200,
      
0xfc01,
//0x3e01,	//15   // BPR Th1 //
0x4640,	//20	// GRGB_GTHR	//
0x4740,	//20	// GRGB_RGTHR	//
0x0a03,	//04	// CGAIN_H	//
0x0ba0,	//00	// CGAIN_L	//
      
//0xfc05,
//0x7700,	//30	// Y_LPF_SIGTHR01 Low	//
//0x7800,	//40	// Y_LPF_SIGTHR02	High //
//0x7905,	//00	// Y_LPF_SIGTHR_H	//
//0x2c20,	//12	// Y Pos Edge [23:16]	//
//0x3020,	//12	// Y Neg Edge //
      
//0xfc02,
//0x3c10,	//00	// Normal BPR Thr //
//0x028e,	//0e   // CIS BPR //
//0xfc01,
//0x3e00,	//15   // BPR Th1 //
//0x3fff,	//7f
//0x4003,	//00
//0x41ff,	//00
//0x42ff,	//00
//0x4300,	//00
//0x4400,	//00
};

static u16 OP_CAMCORDER_DATA[] = {
0xfc02,
0x2d58,
0x3718,
      
0xfc20,
0x1480,
      
0xfc00,
0x7301,
0x8300,
0x0208,
};

static u16  EV_MINUS_4_DATA[] = {
0xfc00,
0x2901,
0x2a75,
};

static u16 EV_MINUS_3_DATA[] = {
0xfc00,
0x2902,
0x2a17,
};

static u16 EV_MINUS_2_DATA[] = {
0xfc00,
0x2902,
0x2ab9,
};

static u16 EV_MINUS_1_DATA[] = {
0xfc00,
0x2903,
0x2a5b,
};

static u16 EV_0_DATA[] = {
0xfc00,
0x2904,
0x2a00,
};

static u16 EV_PLUS_1_DATA[] = {
0xfc00,
0x2904,
0x2a9b,
};

static u16 EV_PLUS_2_DATA[] = {
0xfc00,
0x2905,
0x2a36,
};

static u16 EV_PLUS_3_DATA[] = {
0xfc00,
0x2905,
0x2ad1,
};

static u16 EV_PLUS_4_DATA[] = {
0xfc00,
0x2906,
0x2a6c, //  a0
};

//======================================
// AWB setting
//======================================
static u16 WB_AUTO_DATA[] = {

0xfc00,
0x0400,
0x0500,
      
0xfc22,
0x05d0,//d0
0x0400,
0x07ac,//ac
      
0xfc00,
0x3000,

};

static u16 WB_TUNGSTEN_DATA[] = {
0xfc00,
0x0400,
0x0500,
      
0xfc22,
0x059d, //a9//
0x0400,
0x07dc, //dc//
      
0xfc00,
0x3002,
};      
        
static u16 WB_FLUORESCENT_DATA[] = {
0xfc00,
0x0400,
0x0500,
      
0xfc22,
0x05c8,
0x0400,
0x07c5, //c1//
      
0xfc00,
0x3002,
};

static u16 WB_SUNNY_DATA[] = {
0xfc00,
0x0400,
0x0500,
      
0xfc22,
0x05e2,  //dd//
0x0400,
0x078b,  //8e//
      
0xfc00,
0x3002,
};

static u16 WB_CLOUDY_DATA[] = {
0xfc00,
0x0400,
0x0500,
      
0xfc22,
0x050d,  //11//
0x0401,  
0x0778,  
      
0xfc00,
0x3002,
};

//-------------------------------------------------------------------------
// Usage     : Effect 
//=========================================================================
static u16 EFFECT_NORMAL_DATA[] = {
0xfc00,
0x2500,
};

static u16 EFFECT_B_AND_W_DATA[] = {
0xfc00,
0x2508,
};

static u16 EFFECT_NEGATIVE_DATA[] = {
0xfc00,
0x2504,
};

static u16 EFFECT_SEPIA_DATA[] = {
0xfc00,
0x2501,
};

static u16 EFFECT_GREEN_DATA[] = {
0xfc00,
0x2510,
};

static u16 EFFECT_AQUA_DATA[] = {
0xfc00,
0x2502,
};

static u16 SHADE_DNP_DATA[] ={
// shade_dnp //
0xfc09,
0x3500,   
0x36fd,   
0x3701,   
0x380c,   
0x3901,   
0x3a3f,   
0x3b01,   
0x3c7f,   
0x3d01,   
0x3ea0,   
0x3f01,   
0x40ba,   
0x4101,   
0x42df,   
0x4301,   
0x44f9,
      
0x5618,
0x5825,
0x0002,  // shading on //
};

static u16 SHADE_ETC_DATA[] ={
// shade_etc //              
0xfc09,
0x3501,//01   
0x3604,//02  
0x3701,//01   
0x3814,//12   
0x3901,//01   
0x3a41,//3f   
0x3b01,//01   
0x3c79,//7d   
0x3d01,//01   
0x3e96,//9a  
0x3f01,//01   
0x40b3,//b8   
0x4101,//01   
0x42d8,//dd   
0x4301,//01   
0x44f2,//f7
         
0x5614,//14
0x5821,//21
0x0002,//02  // shading on //
};

static u16 SET_THRESHOLD_DATA[] ={
0x0002, //normal light condition
0x0052, //mid_low light condition
0x006a, //low light condition
0x0034, //shade_min
0x0082, //shade_max
};

static u16 CAPTURE_DELAY_TIME_DATA[] =
{
// offset value : default value is 330ms
1000,    // low light
800,    // mid_low light
330,        // mid light
330,        // normal light
};

#endif
