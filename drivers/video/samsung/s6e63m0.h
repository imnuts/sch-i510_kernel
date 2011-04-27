#ifndef S6E63M0_H
#define S6E63M0_H

#define SLEEPMSEC		0x1000
#define ENDDEF			0x2000
#define	DEFMASK			0xFF00
#define COMMAND_ONLY		0xFE
#define DATA_ONLY		0xFF

#define M2_LCD_ID1      0xA1
#define M2_LCD_ID2      0x02
#define M2_LCD_ID3      0x11

#define SM2_LCD_ID1      0xA1
#define SM2_LCD_ID2      0x12
#define SM2_LCD_ID3      0x11

typedef enum {
    LCD_PANEL_M2 = 1,
    LCD_PANEL_SM2
}lcd_panel_type;

extern const unsigned short s6e63m0_SEQ_DISPLAY_ON[];
extern const unsigned short s6e63m0_SEQ_DISPLAY_OFF[];
extern const unsigned short s6e63m0_SEQ_STANDBY_ON[];
extern const unsigned short s6e63m0_SEQ_STANDBY_OFF[];
extern const unsigned short s6e63m0_SEQ_SETTING[];
extern const unsigned short SEQ_ETC_CONDITION_SET[];
extern const unsigned short SEQ_PANEL_CONDITION_SET[];
extern const unsigned short SEQ_DISPLAY_CONDITION_SET[];
extern const unsigned short SEQ_GTCON_CONTROL_SET[];
extern const unsigned short SEQ_ELVSS_SETTING[];

extern const unsigned short s6e63m0_SEQ_STANDBY_ON_HWREV_8[];
extern const unsigned short s6e63m0_SEQ_STANDBY_OFF_HWREV_8[];
extern const unsigned short SEQ_PANEL_CONDITION_SET_HWREV_8[];
extern const unsigned short SEQ_DISPLAY_CONDITION_SET_HWREV_8[];
extern const unsigned short SEQ_POWER_CONTROL_SET_HWREV_8[];
extern const unsigned short SEQ_POWER_CONTROL_SET_SM2[];
extern const unsigned short SEQ_ELVSS_SETTING_HWREV_8[];

#if 0
extern const unsigned short SEQ_STAND_BY_OFF[];
extern const unsigned short SEQ_DISPLAY_ON[];
#endif

extern const unsigned short SEQ_ELVSS_SM2_CONTROL[];    //ELVSS control set
extern const unsigned short SEQ_ELVSS_SM2_LEVEL4[];     //210CD to 300CD
extern const unsigned short SEQ_ELVSS_SM2_LEVEL3[];     //170CD to 200CD
extern const unsigned short SEQ_ELVSS_SM2_LEVEL2[];     //110CD to 160CD
extern const unsigned short SEQ_ELVSS_SM2_LEVEL1[];     //30CD to 100CD

extern const unsigned short SEQ_ELVSS_LEVEL4_REVE[];     //210CD to 300CD
extern const unsigned short SEQ_ELVSS_LEVEL4[];     //210CD to 300CD
extern const unsigned short SEQ_ELVSS_LEVEL3[];     //170CD to 200CD
extern const unsigned short SEQ_ELVSS_LEVEL2[];     //110CD to 160CD
extern const unsigned short SEQ_ELVSS_LEVEL1[];     //30CD to 100CD

extern const unsigned short SEQ_GAMMA_30_SETTING_SM2[];
extern const unsigned short SEQ_GAMMA_40_SETTING_SM2[];
extern const unsigned short SEQ_GAMMA_50_SETTING_SM2[];
extern const unsigned short SEQ_GAMMA_60_SETTING_SM2[];
extern const unsigned short SEQ_GAMMA_70_SETTING_SM2[];
extern const unsigned short SEQ_GAMMA_80_SETTING_SM2[];
extern const unsigned short SEQ_GAMMA_90_SETTING_SM2[];
extern const unsigned short SEQ_GAMMA_100_SETTING_SM2[];
extern const unsigned short SEQ_GAMMA_110_SETTING_SM2[];
extern const unsigned short SEQ_GAMMA_120_SETTING_SM2[];
extern const unsigned short SEQ_GAMMA_130_SETTING_SM2[];
extern const unsigned short SEQ_GAMMA_140_SETTING_SM2[];
extern const unsigned short SEQ_GAMMA_150_SETTING_SM2[];
extern const unsigned short SEQ_GAMMA_160_SETTING_SM2[];
extern const unsigned short SEQ_GAMMA_170_SETTING_SM2[];
extern const unsigned short SEQ_GAMMA_180_SETTING_SM2[];
extern const unsigned short SEQ_GAMMA_190_SETTING_SM2[];
extern const unsigned short SEQ_GAMMA_200_SETTING_SM2[];
extern const unsigned short SEQ_GAMMA_210_SETTING_SM2[];
extern const unsigned short SEQ_GAMMA_220_SETTING_SM2[];
extern const unsigned short SEQ_GAMMA_230_SETTING_SM2[];
extern const unsigned short SEQ_GAMMA_240_SETTING_SM2[];
extern const unsigned short SEQ_GAMMA_250_SETTING_SM2[];
extern const unsigned short SEQ_GAMMA_260_SETTING_SM2[];
extern const unsigned short SEQ_GAMMA_270_SETTING_SM2[];
extern const unsigned short SEQ_GAMMA_280_SETTING_SM2[];
extern const unsigned short SEQ_GAMMA_290_SETTING_SM2[];
extern const unsigned short SEQ_GAMMA_300_SETTING_SM2[];

extern const unsigned short SEQ_GAMMA_50_SETTING[];
extern const unsigned short SEQ_GAMMA_70_SETTING[];
extern const unsigned short SEQ_GAMMA_80_SETTING[];
extern const unsigned short SEQ_GAMMA_90_SETTING[];
extern const unsigned short SEQ_GAMMA_100_SETTING[];
extern const unsigned short SEQ_GAMMA_110_SETTING[];
extern const unsigned short SEQ_GAMMA_120_SETTING[];
extern const unsigned short SEQ_GAMMA_130_SETTING[];
extern const unsigned short SEQ_GAMMA_140_SETTING[];
extern const unsigned short SEQ_GAMMA_150_SETTING[];
extern const unsigned short SEQ_GAMMA_160_SETTING[];
extern const unsigned short SEQ_GAMMA_170_SETTING[];
extern const unsigned short SEQ_GAMMA_180_SETTING[];  
extern const unsigned short SEQ_GAMMA_190_SETTING[];    
extern const unsigned short SEQ_GAMMA_200_SETTING[]; 
extern const unsigned short SEQ_GAMMA_210_SETTING[];    
extern const unsigned short SEQ_GAMMA_220_SETTING[];    
extern const unsigned short SEQ_GAMMA_230_SETTING[];    
extern const unsigned short SEQ_GAMMA_240_SETTING[];    
extern const unsigned short SEQ_GAMMA_250_SETTING[];    
extern const unsigned short SEQ_GAMMA_260_SETTING[]; 
extern const unsigned short SEQ_GAMMA_270_SETTING[];    
extern const unsigned short SEQ_GAMMA_280_SETTING[];    
extern const unsigned short SEQ_GAMMA_290_SETTING[];    
extern const unsigned short SEQ_GAMMA_300_SETTING[];     

extern const unsigned short SEQ_GAMMA_30_SETTING_HWREV_8[];
extern const unsigned short SEQ_GAMMA_40_SETTING_HWREV_8[];
extern const unsigned short SEQ_GAMMA_50_SETTING_HWREV_8[];
extern const unsigned short SEQ_GAMMA_70_SETTING_HWREV_8[];
extern const unsigned short SEQ_GAMMA_80_SETTING_HWREV_8[];
extern const unsigned short SEQ_GAMMA_90_SETTING_HWREV_8[];
extern const unsigned short SEQ_GAMMA_100_SETTING_HWREV_8[];
extern const unsigned short SEQ_GAMMA_110_SETTING_HWREV_8[];
extern const unsigned short SEQ_GAMMA_120_SETTING_HWREV_8[];
extern const unsigned short SEQ_GAMMA_130_SETTING_HWREV_8[];
extern const unsigned short SEQ_GAMMA_140_SETTING_HWREV_8[];
extern const unsigned short SEQ_GAMMA_150_SETTING_HWREV_8[];
extern const unsigned short SEQ_GAMMA_160_SETTING_HWREV_8[];
extern const unsigned short SEQ_GAMMA_170_SETTING_HWREV_8[];
extern const unsigned short SEQ_GAMMA_180_SETTING_HWREV_8[];  
extern const unsigned short SEQ_GAMMA_190_SETTING_HWREV_8[];    
extern const unsigned short SEQ_GAMMA_200_SETTING_HWREV_8[]; 
extern const unsigned short SEQ_GAMMA_210_SETTING_HWREV_8[];    
extern const unsigned short SEQ_GAMMA_220_SETTING_HWREV_8[];    
extern const unsigned short SEQ_GAMMA_230_SETTING_HWREV_8[];    
extern const unsigned short SEQ_GAMMA_240_SETTING_HWREV_8[];    
extern const unsigned short SEQ_GAMMA_250_SETTING_HWREV_8[];    
extern const unsigned short SEQ_GAMMA_260_SETTING_HWREV_8[]; 
extern const unsigned short SEQ_GAMMA_270_SETTING_HWREV_8[];    
extern const unsigned short SEQ_GAMMA_280_SETTING_HWREV_8[];    
extern const unsigned short SEQ_GAMMA_290_SETTING_HWREV_8[];    
extern const unsigned short SEQ_GAMMA_300_SETTING_HWREV_8[];     

extern const unsigned short gamma_update[];
extern const unsigned short s6e63m0_22gamma_300cd[];
extern const unsigned short s6e63m0_22gamma_290cd[];
extern const unsigned short s6e63m0_22gamma_280cd[];
extern const unsigned short s6e63m0_22gamma_270cd[];
extern const unsigned short s6e63m0_22gamma_260cd[];
extern const unsigned short s6e63m0_22gamma_250cd[];
extern const unsigned short s6e63m0_22gamma_240cd[];
extern const unsigned short s6e63m0_22gamma_230cd[];
extern const unsigned short s6e63m0_22gamma_220cd[];
extern const unsigned short s6e63m0_22gamma_210cd[];
extern const unsigned short s6e63m0_22gamma_200cd[];
extern const unsigned short s6e63m0_22gamma_190cd[];
extern const unsigned short s6e63m0_22gamma_180cd[];
extern const unsigned short s6e63m0_22gamma_170cd[];
extern const unsigned short s6e63m0_22gamma_160cd[];
extern const unsigned short s6e63m0_22gamma_150cd[];
extern const unsigned short s6e63m0_22gamma_140cd[];
extern const unsigned short s6e63m0_22gamma_130cd[];
extern const unsigned short s6e63m0_22gamma_120cd[];
extern const unsigned short s6e63m0_22gamma_110cd[];
extern const unsigned short s6e63m0_22gamma_100cd[];
extern const unsigned short s6e63m0_22gamma_90cd[];
extern const unsigned short s6e63m0_22gamma_80cd[];
extern const unsigned short s6e63m0_22gamma_70cd[];
extern const unsigned short s6e63m0_22gamma_60cd[];
extern const unsigned short s6e63m0_22gamma_50cd[];
extern const unsigned short s6e63m0_22gamma_40cd[];
extern const unsigned short s6e63m0_22gamma_30cd[];
#if defined(CONFIG_ARIES_NTT) // Modify NTTS1
extern const unsigned short s6e63m0_22gamma_20cd[];
#endif

extern const unsigned short s6e63m0_19gamma_300cd[];
extern const unsigned short s6e63m0_19gamma_290cd[];
extern const unsigned short s6e63m0_19gamma_280cd[];
extern const unsigned short s6e63m0_19gamma_270cd[];
extern const unsigned short s6e63m0_19gamma_260cd[];
extern const unsigned short s6e63m0_19gamma_250cd[];
extern const unsigned short s6e63m0_19gamma_240cd[];
extern const unsigned short s6e63m0_19gamma_230cd[];
extern const unsigned short s6e63m0_19gamma_220cd[];
extern const unsigned short s6e63m0_19gamma_210cd[];
extern const unsigned short s6e63m0_19gamma_200cd[];
extern const unsigned short s6e63m0_19gamma_190cd[];
extern const unsigned short s6e63m0_19gamma_180cd[];
extern const unsigned short s6e63m0_19gamma_170cd[];
extern const unsigned short s6e63m0_19gamma_160cd[];
extern const unsigned short s6e63m0_19gamma_150cd[];
extern const unsigned short s6e63m0_19gamma_140cd[];
extern const unsigned short s6e63m0_19gamma_130cd[];
extern const unsigned short s6e63m0_19gamma_120cd[];
extern const unsigned short s6e63m0_19gamma_110cd[];
extern const unsigned short s6e63m0_19gamma_100cd[];
extern const unsigned short s6e63m0_19gamma_90cd[];
extern const unsigned short s6e63m0_19gamma_80cd[];
extern const unsigned short s6e63m0_19gamma_70cd[];
extern const unsigned short s6e63m0_19gamma_60cd[];
extern const unsigned short s6e63m0_19gamma_50cd[];
extern const unsigned short s6e63m0_19gamma_40cd[];
extern const unsigned short s6e63m0_19gamma_30cd[];
#if defined(CONFIG_ARIES_NTT) // Modify NTTS1
extern const unsigned short s6e63m0_19gamma_20cd[];
#endif


extern const unsigned short s6e63m0_19gamma_300cd_sm2[];
extern const unsigned short s6e63m0_19gamma_290cd_sm2[];
extern const unsigned short s6e63m0_19gamma_280cd_sm2[];
extern const unsigned short s6e63m0_19gamma_270cd_sm2[];
extern const unsigned short s6e63m0_19gamma_260cd_sm2[];
extern const unsigned short s6e63m0_19gamma_250cd_sm2[];
extern const unsigned short s6e63m0_19gamma_240cd_sm2[];
extern const unsigned short s6e63m0_19gamma_230cd_sm2[];
extern const unsigned short s6e63m0_19gamma_220cd_sm2[];
extern const unsigned short s6e63m0_19gamma_210cd_sm2[];
extern const unsigned short s6e63m0_19gamma_200cd_sm2[];
extern const unsigned short s6e63m0_19gamma_190cd_sm2[];
extern const unsigned short s6e63m0_19gamma_180cd_sm2[];
extern const unsigned short s6e63m0_19gamma_170cd_sm2[];
extern const unsigned short s6e63m0_19gamma_160cd_sm2[];
extern const unsigned short s6e63m0_19gamma_150cd_sm2[];
extern const unsigned short s6e63m0_19gamma_140cd_sm2[];
extern const unsigned short s6e63m0_19gamma_130cd_sm2[];
extern const unsigned short s6e63m0_19gamma_120cd_sm2[];
extern const unsigned short s6e63m0_19gamma_110cd_sm2[];
extern const unsigned short s6e63m0_19gamma_100cd_sm2[];
extern const unsigned short s6e63m0_19gamma_90cd_sm2[];
extern const unsigned short s6e63m0_19gamma_80cd_sm2[];
extern const unsigned short s6e63m0_19gamma_70cd_sm2[];
extern const unsigned short s6e63m0_19gamma_60cd_sm2[];
extern const unsigned short s6e63m0_19gamma_50cd_sm2[];
extern const unsigned short s6e63m0_19gamma_40cd_sm2[];
extern const unsigned short s6e63m0_19gamma_30cd_sm2[];

extern const unsigned short s6e63m0_19gamma_300cd_hwrev_8[];
extern const unsigned short s6e63m0_19gamma_290cd_hwrev_8[];
extern const unsigned short s6e63m0_19gamma_280cd_hwrev_8[];
extern const unsigned short s6e63m0_19gamma_270cd_hwrev_8[];
extern const unsigned short s6e63m0_19gamma_260cd_hwrev_8[];
extern const unsigned short s6e63m0_19gamma_250cd_hwrev_8[];
extern const unsigned short s6e63m0_19gamma_240cd_hwrev_8[];
extern const unsigned short s6e63m0_19gamma_230cd_hwrev_8[];
extern const unsigned short s6e63m0_19gamma_220cd_hwrev_8[];
extern const unsigned short s6e63m0_19gamma_210cd_hwrev_8[];
extern const unsigned short s6e63m0_19gamma_200cd_hwrev_8[];
extern const unsigned short s6e63m0_19gamma_190cd_hwrev_8[];
extern const unsigned short s6e63m0_19gamma_180cd_hwrev_8[];
extern const unsigned short s6e63m0_19gamma_170cd_hwrev_8[];
extern const unsigned short s6e63m0_19gamma_160cd_hwrev_8[];
extern const unsigned short s6e63m0_19gamma_150cd_hwrev_8[];
extern const unsigned short s6e63m0_19gamma_140cd_hwrev_8[];
extern const unsigned short s6e63m0_19gamma_130cd_hwrev_8[];
extern const unsigned short s6e63m0_19gamma_120cd_hwrev_8[];
extern const unsigned short s6e63m0_19gamma_110cd_hwrev_8[];
extern const unsigned short s6e63m0_19gamma_100cd_hwrev_8[];
extern const unsigned short s6e63m0_19gamma_90cd_hwrev_8[];
extern const unsigned short s6e63m0_19gamma_80cd_hwrev_8[];
extern const unsigned short s6e63m0_19gamma_70cd_hwrev_8[];
extern const unsigned short s6e63m0_19gamma_60cd_hwrev_8[];
extern const unsigned short s6e63m0_19gamma_50cd_hwrev_8[];
extern const unsigned short s6e63m0_19gamma_40cd_hwrev_8[];
extern const unsigned short s6e63m0_19gamma_30cd_hwrev_8[];
#if defined(CONFIG_ARIES_NTT) // Modify NTTS1
extern const unsigned short s6e63m0_19gamma_20cd_hwrev_8[];
#endif

#ifdef CONFIG_FB_S3C_TL2796_ACL
extern const unsigned short acl_cutoff_off[];
extern const unsigned short acl_cutoff_init[];
extern const unsigned short acl_cutoff_8p[];
extern const unsigned short acl_cutoff_14p[];
extern const unsigned short acl_cutoff_20p[];
extern const unsigned short acl_cutoff_24p[];
extern const unsigned short acl_cutoff_28p[];
extern const unsigned short acl_cutoff_32p[];
extern const unsigned short acl_cutoff_35p[];
extern const unsigned short acl_cutoff_37p[];
extern const unsigned short acl_cutoff_38p[];
extern const unsigned short acl_cutoff_40p[];
extern const unsigned short acl_cutoff_43p[];
extern const unsigned short acl_cutoff_45p[];
extern const unsigned short acl_cutoff_47p[];
extern const unsigned short acl_cutoff_48p[];
extern const unsigned short acl_cutoff_50p[];
extern const unsigned short acl_cutoff_60p[];
extern const unsigned short acl_cutoff_75p[];
#endif /* CONFIG_FB_S3C_TL2796_ACL */

#endif /* S6E63M0_H */
