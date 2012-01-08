/*
 * wm8994_aries.c  --  WM8994 ALSA Soc Audio driver related Aries
 *
 *  Copyright (C) 2010 Samsung Electronics.
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/fsa9480.h>
#include <plat/gpio-cfg.h>
#include <plat/map-base.h>
#include <mach/regs-clock.h>
#include "wm8994_samsung.h"
#ifdef CONFIG_SND_VOODOO
#include "wm8994_voodoo.h"
#endif

/* Debug Feature */
#define SUBJECT "wm8994_stealthv.c"

/* Definitions of tunning volumes for wm8994 */
/* Call common gain */
#define TUNING_DAC1L_VOL			0xC0	// 610h
#define TUNING_DAC1R_VOL			0xC0	// 611h

/* Call reciever gain */
#define TUNING_CALL_RCV_INPUTMIX_VOL		0x09	// 18h
#define TUNING_HPOUT2_VOL			0x0	// 1Fh
#define TUNING_RCV_OPGAL_VOL			0x3B	// 20h
#define TUNING_RCV_OPGAR_VOL			0x3B	// 21h
#define TUNING_RCV_OUTMIX5_VOL			0x0	// 31h
#define TUNING_RCV_OUTMIX6_VOL			0x0	// 32h

/* Call headset gain */
#define TUNING_CALL_EAR_INPUTMIX_VOL		0x1D	// 1Ah
#define TUNING_CALL_OUTPUTL_VOL			0x35	// 1Ch
#define TUNING_CALL_OUTPUTR_VOL			0x35	// 1Dh
#define TUNING_CALL_OPGAL_VOL			0x39	// 20h
#define TUNING_CALL_OPGAR_VOL			0x39	// 21h

/* Call speaker gain */
#define TUNING_CALL_SPK_INPUTMIX_VOL		0x09	// 18h
#define TUNING_SPKMIXL_ATTEN			0x0	// 22h
#define TUNING_SPKMIXR_ATTEN			0x0	// 23h
#define TUNING_CALL_CLASSD_VOL			0x7	// 25h
#define TUNING_CALL_SPKL_VOL			0x3F	// 26h
#define TUNING_CALL_SPK_MIXER_VOL		0x80	// 29h

#ifdef FEATURE_TTY
/* TTY Full gain */
#define TUNING_CALL_TTYFULL_OUTPUTL_VOL		0x30	// 1Ch
#define TUNING_CALL_TTYFULL_OUTPUTR_VOL		0x30	// 1Dh
#define TUNING_CALL_TTYFULL_OPGAL_VOL		0x39	// 20h
#define TUNING_CALL_TTYFULL_OPGAR_VOL		0x39	// 21
#define TUNING_CALL_TTYFULL_DAC1L_VOL		0x1C0	// 610h
#define TUNING_CALL_TTYFULL_DAC1R_VOL		0x1C0	// 611h

/* TTY HCO gain */
#define TUNING_CALL_TTYHCO_OUTPUTL_VOL		0x30	// 1Ch
#define TUNING_CALL_TTYHCO_OUTPUTR_VOL		0x30	// 1Dh
#define TUNING_CALL_TTYHCO_OPGAL_VOL		0x3D	// 20h
#define TUNING_CALL_TTYHCO_OPGAR_VOL		0x3D	// 21
#define TUNING_CALL_TTYHCO_DAC1L_VOL		0x1C0	// 610h
#define TUNING_CALL_TTYHCO_DAC1R_VOL		0x1C0	// 611h

/* TTY VCO gain */
#define TUNING_CALL_TTYVCO_INPUTMIX_VOL		0x16	// 18h
#define TUNING_CALL_TTYVCO_OUTPUTL_VOL		0x30	// 1Ch
#define TUNING_CALL_TTYVCO_OUTPUTR_VOL		0x30	// 1Dh
#define TUNING_CALL_TTYVCO_OPGAL_VOL		0x39	// 20h
#define TUNING_CALL_TTYVCO_OPGAR_VOL		0x39	// 21
#define TUNING_CALL_TTYVCO_DAC1L_VOL		0x1C0	// 610h
#define TUNING_CALL_TTYVCO_DAC1R_VOL		0x1C0	// 611h
#endif

#ifdef FEATURE_FACTORY_LOOPBACK
/* PBA ear loopback gain */
#define TUNING_LOOPBACK_EAR_INPUTMIX_VOL	0x1D	// 1Ah
#define TUNING_LOOPBACK_OUTPUTL_VOL		0x37	// 1Ch
#define TUNING_LOOPBACK_OUTPUTR_VOL		0x37	// 1Dh
#define TUNING_LOOPBACK_OPGAL_VOL		0x39	// 20h
#define TUNING_LOOPBACK_OPGAR_VOL		0x39	// 21h
#endif

#define TUNING_RING_OUTPUTL_VOL			0x34	// 1Ch
#define TUNING_RING_OUTPUTR_VOL			0x34	// 1Dh
#define TUNING_RING_DUAL_OUTPUTL_VOL		0x1E	// 1Ch
#define TUNING_RING_DUAL_OUTPUTR_VOL		0x1E	// 1Dh
#define TUNING_RING_OPGAL_VOL			0x39	// 20h
#define TUNING_RING_OPGAR_VOL			0x39	// 21h
#define TUNING_RING_CLASSD_VOL			0x5	// 25h
#define TUNING_RING_SPKL_VOL			0x3E	// 26h
#define TUNING_EXTRA_DOCK_SPK_OUTMIX5_VOL	0x1	// 31h
#define TUNING_EXTRA_DOCK_SPK_OUTMIX6_VOL	0x1	// 32h

#define TUNING_MP3_OUTPUTL_VOL			0x35	// 1Ch
#define TUNING_MP3_OUTPUTR_VOL			0x35	// 1Dh
#define TUNING_MP3_DUAL_OUTPUTL_VOL		0x1E	// 1Ch
#define TUNING_MP3_DUAL_OUTPUTR_VOL		0x1E	// 1Dh
#define TUNING_MP3_OPGAL_VOL			0x3B	// 20h
#define TUNING_MP3_OPGAR_VOL			0x3B	// 21h
#define TUNING_MP3_CLASSD_VOL			0x5	// 25h
#define TUNING_MP3_SPKL_VOL			0x3E	// 26h
#define TUNING_MP3_EXTRA_DOCK_SPK_OPGAL_VOL	0x3C	// 20h
#define TUNING_MP3_EXTRA_DOCK_SPK_OPGAR_VOL	0x3C	// 21h
#define TUNING_MP3_EXTRA_DOCK_SPK_VOL		0x0	//1Eh

/* Record path gain */
/* Sub mic */
#define TUNING_RECORD_SUB_INPUTMIX_VOL		0x15	// 1Ah
#define TUNING_RECORD_SUB_AIF1ADCL_VOL		0xC0	// 400h
#define TUNING_RECORD_SUB_AIF1ADCR_VOL		0xC0	// 401h

#define TUNING_RECOGNITION_SUB_INPUTMIX_VOL	0x1F	// 1Ah
#define TUNING_RECOGNITION_SUB_AIF1ADCL_VOL	0xD5	// 400h
#define TUNING_RECOGNITION_SUB_AIF1ADCR_VOL	0xD5	// 401

/* Main mic */
#define TUNING_RECORD_MAIN_INPUTLINE_VOL	0x1D	// 18h
#define TUNING_RECORD_MAIN_AIF1ADCL_VOL		0xEF	// 400h
#define TUNING_RECORD_MAIN_AIF1ADCR_VOL		0xEF	// 401h

#define TUNING_RECOGNITION_MAIN_INPUTLINE_VOL	0x1C	// 18h
#define TUNING_RECOGNITION_MAIN_AIF1ADCL_VOL	0xC8	// 400h
#define TUNING_RECOGNITION_MAIN_AIF1ADCR_VOL	0xC8	// 401h

/* Bluetooth mic */
#define TUNING_RECORD_BT_AIF1ADCL_VOL		0xD4	// 400h
#define TUNING_RECORD_BT_AIF1ADCR_VOL		0xD4	// 401h


/* S5P_SLEEP_CONFIG must be controlled by codec if codec use XUSBTI */
int wm8994_configure_clock(struct snd_soc_codec *codec, int en)
{
	struct wm8994_priv *wm8994 = codec->drvdata;

	if (en) {
		clk_enable(wm8994->codec_clk);
		DEBUG_LOG("USBOSC Enabled in Sleep Mode\n");
	} else {
		clk_disable(wm8994->codec_clk);
		DEBUG_LOG("USBOSC disable in Sleep Mode\n");
	}

	return 0;
}

void audio_ctrl_mic_bias_gpio(struct wm8994_platform_data *pdata, int enable)
{
	//DEBUG_LOG("enable = [%d]", enable);

	if (!pdata)
		pr_err("%s: failed to turn off micbias pin\n", __func__);
	else {
		if (enable)
			pdata->set_mic_bias(true);
		else
			pdata->set_mic_bias(false);
	}
}

/* Audio Routing routines for the universal board..wm8994 codec */
void wm8994_disable_path(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;
	u16 val;
	enum audio_path path = wm8994->cur_path;


	DEBUG_LOG("Path = [%d]", path);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);	

	switch (path) {
	case RCV:		
		/* Disbale the HPOUT2 */
		val &= ~(WM8994_HPOUT2_ENA_MASK);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

		/* Disable left MIXOUT */
		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
		val &= ~(WM8994_DAC1L_TO_MIXOUTL_MASK);
		wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val);

		/* Disable right MIXOUT */
		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
		val &= ~(WM8994_DAC1R_TO_MIXOUTR_MASK);
		wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);

		/* Disable HPOUT Mixer */
		val = wm8994_read(codec, WM8994_HPOUT2_MIXER);
		val &= ~(WM8994_MIXOUTLVOL_TO_HPOUT2_MASK | 
			WM8994_MIXOUTRVOL_TO_HPOUT2_MASK);
		wm8994_write(codec, WM8994_HPOUT2_MIXER, val);

		/* Disable mixout volume control */
		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_3);
		val &= ~(WM8994_MIXOUTLVOL_ENA_MASK | 
						WM8994_MIXOUTRVOL_ENA_MASK |
						WM8994_MIXOUTL_ENA_MASK |
						WM8994_MIXOUTR_ENA_MASK);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, val);
		
		break;

	case SPK:
		/* Disbale the SPKOUTL */
		val &= ~(WM8994_SPKOUTL_ENA_MASK); 
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

		/* Disable SPKLVOL */
		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_3);
		val &= ~(WM8994_SPKLVOL_ENA_MASK);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, val);

		/* Disable SPKOUT mixer */
		val = wm8994_read(codec, WM8994_SPKOUT_MIXERS);
		val &= ~(WM8994_SPKMIXL_TO_SPKOUTL_MASK | 
					WM8994_SPKMIXR_TO_SPKOUTL_MASK |
					WM8994_SPKMIXR_TO_SPKOUTR_MASK);
		wm8994_write(codec, WM8994_SPKOUT_MIXERS, val);

		/* Mute Speaker mixer */
		val = wm8994_read(codec, WM8994_SPEAKER_MIXER);
		val &= ~(WM8994_DAC1L_TO_SPKMIXL_MASK);
		wm8994_write(codec, WM8994_SPEAKER_MIXER ,val);
		
		break;

	case HP:
	case HP_NO_MIC:
		if (wm8994->codec_state & CALL_ACTIVE) {
			val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
			val &= ~(0x02C0);
			val |= 0x02C0;
			wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, 0x02C0);

			val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME);
			val &= ~(0x02C0);
			val |= 0x02C0;
			wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, 0x02C0);

			val = wm8994_read(codec, WM8994_ANALOGUE_HP_1);
			val &= ~(0x0022);
			val |= 0x0022;
			wm8994_write(codec, WM8994_ANALOGUE_HP_1, 0x0022);

			val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
			val &= ~(0x0);
			val |= 0x0;
			wm8994_write(codec, WM8994_OUTPUT_MIXER_1, 0x0);

			val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
			val &= ~(0x0);
			val |= 0x0;
			wm8994_write(codec, WM8994_OUTPUT_MIXER_2, 0x0);

			val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_5);
			val &= ~(0x0300);
			val |= 0x0300;
			wm8994_write(codec, WM8994_POWER_MANAGEMENT_5, 0x0300);

			val = wm8994_read(codec, WM8994_CHARGE_PUMP_1);
			val &= ~(0x1F25);
			val |= 0x1F25;
			wm8994_write(codec, WM8994_CHARGE_PUMP_1, 0x1F25);
		}
		
		break;

	case BT :
		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_6);
		val &= ~(WM8994_AIF3_ADCDAT_SRC_MASK | 
						WM8994_AIF2_ADCDAT_SRC_MASK |
						WM8994_AIF2_DACDAT_SRC_MASK |
						WM8994_AIF1_DACDAT_SRC_MASK);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_6, val);

		val = wm8994_read(codec, WM8994_AIF1_DAC1_FILTERS_1);
		val &= ~(WM8994_AIF1DAC1_MUTE_MASK | WM8994_AIF1DAC1_MONO_MASK);
		val |= (WM8994_AIF1DAC1_MUTE);
		wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, val); 
		
		break;

	case SPK_HP :
		if (wm8994->codec_state & CALL_ACTIVE) {
			val &= ~(WM8994_HPOUT1L_ENA_MASK |
						WM8994_HPOUT1R_ENA_MASK |
						WM8994_SPKOUTL_ENA_MASK);
			wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

			/*  ------------ Ear path setting ------------ */
			/* Disable DAC1L to HPOUT1L path */
			val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
			val &= ~(WM8994_DAC1L_TO_HPOUT1L_MASK | 
						WM8994_DAC1L_TO_MIXOUTL_MASK);
			wm8994_write(codec,WM8994_OUTPUT_MIXER_1, val);

			/* Disable DAC1R to HPOUT1R path */
			val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
			val &= ~(WM8994_DAC1R_TO_HPOUT1R_MASK | 
						WM8994_DAC1R_TO_MIXOUTR_MASK);
			wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);

			/* Disable Charge Pump */
			val = wm8994_read(codec, WM8994_CHARGE_PUMP_1);
			val &= ~WM8994_CP_ENA_MASK ;
			val |= WM8994_CP_ENA_DEFAULT;
			wm8994_write(codec, WM8994_CHARGE_PUMP_1, val);

			/* Intermediate HP settings */
			val = wm8994_read(codec, WM8994_ANALOGUE_HP_1); 	
			val &= ~(WM8994_HPOUT1R_DLY_MASK |
						WM8994_HPOUT1R_OUTP_MASK |
						WM8994_HPOUT1R_RMV_SHORT_MASK |
						WM8994_HPOUT1L_DLY_MASK |
						WM8994_HPOUT1L_OUTP_MASK | 
						WM8994_HPOUT1L_RMV_SHORT_MASK);
			wm8994_write(codec, WM8994_ANALOGUE_HP_1, val);

			/* ------------ Spk path setting ------------ */
			/* Disable SPKLVOL */
			val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_3);
			val &= ~(WM8994_SPKLVOL_ENA_MASK);
			wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, val);

			/* Disable SPKOUT mixer */
			val = wm8994_read(codec, WM8994_SPKOUT_MIXERS);
			val &= ~(WM8994_SPKMIXL_TO_SPKOUTL_MASK | 
					WM8994_SPKMIXR_TO_SPKOUTL_MASK |
					WM8994_SPKMIXR_TO_SPKOUTR_MASK);
			wm8994_write(codec, WM8994_SPKOUT_MIXERS, val);

			/* Mute Speaker mixer */
			val = wm8994_read(codec, WM8994_SPEAKER_MIXER);
			val &= ~(WM8994_DAC1L_TO_SPKMIXL_MASK);
			wm8994_write(codec, WM8994_SPEAKER_MIXER ,val);
		}
		
		break;

	default:
		DEBUG_LOG_ERR("Path[%d] is not invaild!\n", path);
		break;
	}
}

void wm8994_disable_rec_path(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;
	u16 val;
	enum mic_path mic = wm8994->rec_path;


	wm8994->rec_path = MIC_OFF;

	if (!(wm8994->codec_state & CALL_ACTIVE))
		audio_ctrl_mic_bias_gpio(wm8994->pdata, 0);

	switch (mic) {
	case MAIN:
	case MAIN2:
		DEBUG_LOG("Disabling MAIN Mic Path..\n");

		if (!(wm8994->codec_state & CALL_ACTIVE)) {
			val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_2);
			val &= ~(WM8994_IN1L_ENA_MASK | WM8994_MIXINL_ENA_MASK |
				WM8994_IN2R_ENA_MASK | WM8994_MIXINR_ENA_MASK);
			wm8994_write(codec, WM8994_POWER_MANAGEMENT_2, val);

			if (!wm8994->testmode_config_flag) {	
				/*  Mute IN1L PGA, update volume */
				val = wm8994_read(codec, 
					WM8994_LEFT_LINE_INPUT_1_2_VOLUME);
				val &= ~(WM8994_IN1L_MUTE_MASK | 
							WM8994_IN1L_VOL_MASK);
				val |= (WM8994_IN1L_VU | WM8994_IN1L_MUTE);
				wm8994_write(codec, 
					WM8994_LEFT_LINE_INPUT_1_2_VOLUME, val);

				val = wm8994_read(codec, 
					WM8994_RIGHT_LINE_INPUT_3_4_VOLUME);
				val &= ~(WM8994_IN2R_MUTE_MASK | 
							WM8994_IN2R_VOL_MASK);
				val |= (WM8994_IN2R_VU |WM8994_IN2R_MUTE);
				wm8994_write(codec, 
					WM8994_RIGHT_LINE_INPUT_3_4_VOLUME,val);

				/* Mute the PGA */
				val = wm8994_read(codec, WM8994_INPUT_MIXER_3);
				val&= ~(WM8994_IN1L_TO_MIXINL_MASK | 
						WM8994_IN1L_MIXINL_VOL_MASK | 
						WM8994_MIXOUTL_MIXINL_VOL_MASK);
				wm8994_write(codec, WM8994_INPUT_MIXER_3, val); 
			}

			/* Disconnect IN1LN ans IN1LP to the inputs */
			val = wm8994_read(codec, WM8994_INPUT_MIXER_2); 
			val &= ~(WM8994_IN1LN_TO_IN1L_MASK | 
						WM8994_IN1LP_TO_IN1L_MASK);
			wm8994_write(codec, WM8994_INPUT_MIXER_2, val);

			/* Digital Paths */ 	
			val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_4);
			val &= ~(WM8994_ADCL_ENA_MASK |
						WM8994_AIF1ADC1L_ENA_MASK);
			wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, val);
		} else {				
			/* Digital Paths */ 	
			val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_4);
			val &= ~(WM8994_AIF1ADC1L_ENA_MASK | 
						WM8994_AIF1ADC1R_ENA_MASK);
			wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, val);
		}

		/* Disable timeslots */
		val = wm8994_read(codec, WM8994_AIF1_ADC1_LEFT_MIXER_ROUTING);
		val &= ~(WM8994_ADC1L_TO_AIF1ADC1L);
		wm8994_write(codec, WM8994_AIF1_ADC1_LEFT_MIXER_ROUTING, val);	

		/* Disable timeslots */
		val = wm8994_read(codec, WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING);
		val &= ~(WM8994_ADC1R_TO_AIF1ADC1R);
		wm8994_write(codec, WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING, val);
		
		break;

	case SUB:
		DEBUG_LOG("Disbaling SUB Mic path..\n");

		if (!(wm8994->codec_state & CALL_ACTIVE)) {
			val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_2);
			val &= ~(WM8994_IN1R_ENA_MASK |WM8994_MIXINR_ENA_MASK);
			wm8994_write(codec, WM8994_POWER_MANAGEMENT_2, val);

			if (!wm8994->testmode_config_flag) {	
				/* Disable volume,unmute Right Line */
				val = wm8994_read(codec,
					WM8994_RIGHT_LINE_INPUT_1_2_VOLUME);	
				val &= ~(WM8994_IN1R_MUTE_MASK | 
							WM8994_IN1R_VOL_MASK);
				val |= (WM8994_IN1R_VU | WM8994_IN1R_MUTE);
				wm8994_write(codec, 
					WM8994_RIGHT_LINE_INPUT_1_2_VOLUME,val);

				/* Mute right pga, set volume */ 
				val = wm8994_read(codec, WM8994_INPUT_MIXER_4);
				val&= ~(WM8994_IN1R_TO_MIXINR_MASK | 
						WM8994_IN1R_MIXINR_VOL_MASK |
						WM8994_MIXOUTR_MIXINR_VOL_MASK);
				wm8994_write(codec, WM8994_INPUT_MIXER_4, val);
			}

			/* Disconnect in1rn to inr1 and in1rp to inrp */
			val = wm8994_read(codec, WM8994_INPUT_MIXER_2);
			val &= ~(WM8994_IN1RN_TO_IN1R_MASK | 
						WM8994_IN1RP_TO_IN1R_MASK);
			wm8994_write(codec, WM8994_INPUT_MIXER_2, val);

			/* Digital Paths */ 
			/* Disable right ADC and time slot */
			val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_4);
			val &= ~(WM8994_ADCR_ENA_MASK |
						WM8994_AIF1ADC1R_ENA_MASK);
			wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, val);			
		} else {				
			/* Digital Paths */ 
			/* Disable right ADC and time slot */
			val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_4);
			val &= ~(WM8994_AIF1ADC1R_ENA_MASK);
			wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, val);

			val = wm8994_read(codec, WM8994_AIF1_CONTROL_1);
			val &= ~(WM8994_AIF1ADCL_SRC_MASK);
			wm8994_write(codec, WM8994_AIF1_CONTROL_1, val);
		}

		/* ADC Right mixer routing */
		val = wm8994_read(codec, WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING);
		val &= ~(WM8994_ADC1R_TO_AIF1ADC1R_MASK);
		wm8994_write(codec, WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING, val);
		
		break;

	case BT_REC:
		if(!(wm8994->codec_state & CALL_ACTIVE)) {
			val = wm8994_read(codec, 
					WM8994_AIF1_ADC1_LEFT_MIXER_ROUTING);
			val &= ~(WM8994_AIF2DACL_TO_AIF1ADC1L_MASK | 
						WM8994_ADC1L_TO_AIF1ADC1L_MASK);
			wm8994_write(codec, 
				WM8994_AIF1_ADC1_LEFT_MIXER_ROUTING, val);

			val = wm8994_read(codec, 
					WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING);
			val &= ~(WM8994_AIF2DACR_TO_AIF1ADC1R_MASK | 
						WM8994_ADC1R_TO_AIF1ADC1R_MASK);
			wm8994_write(codec, 
				WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING, val);

			val = wm8994_read(codec, WM8994_AIF2_DAC_FILTERS_1);
			val &= ~(WM8994_AIF2DAC_MUTE_MASK);
			val |= WM8994_AIF2DAC_MUTE;
			wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_1, val); 
		}
		
		break;

	default:
		DEBUG_LOG_ERR("Path[%d] is not invaild!\n", mic);
		break;
	}
}

void wm8994_set_bluetooth_common_setting(struct snd_soc_codec *codec)
{
	u32 val;


	wm8994_write(codec, WM8994_GPIO_1, 0xA101);
	wm8994_write(codec, WM8994_GPIO_2, 0x8100);
	wm8994_write(codec, WM8994_GPIO_3, 0x0100);
	wm8994_write(codec, WM8994_GPIO_4, 0x0100);
	wm8994_write(codec, WM8994_GPIO_5, 0x8100);
	wm8994_write(codec, WM8994_GPIO_6, 0xA101);
	wm8994_write(codec, WM8994_GPIO_7, 0x0100);
	wm8994_write(codec, WM8994_GPIO_8, 0xA101);
	wm8994_write(codec, WM8994_GPIO_9, 0xA101);
	wm8994_write(codec, WM8994_GPIO_10, 0xA101);
	wm8994_write(codec, WM8994_GPIO_11, 0xA101);

	wm8994_write(codec, WM8994_FLL2_CONTROL_2, 0x0700);
	wm8994_write(codec, WM8994_FLL2_CONTROL_3, 0x3126);
	wm8994_write(codec, WM8994_FLL2_CONTROL_4, 0x0100);
	wm8994_write(codec, WM8994_FLL2_CONTROL_5, 0x0C88);
	wm8994_write(codec, WM8994_FLL2_CONTROL_1,
				WM8994_FLL2_FRACN_ENA | WM8994_FLL2_ENA);

	val = wm8994_read(codec, WM8994_AIF2_CLOCKING_1);
	if (!(val & WM8994_AIF2CLK_ENA))
		wm8994_write(codec, WM8994_AIF2_CLOCKING_1, 0x0018);

	wm8994_write(codec, WM8994_AIF2_RATE, 0x9 << WM8994_AIF2CLK_RATE_SHIFT);

	/* AIF2 Interface - PCM Stereo mode */
	if (HWREV >= 0x0A) {
		/* FMT : I2S */
		wm8994_write(codec, WM8994_AIF2_CONTROL_1, 0x4010);
		DEBUG_LOG("AIF2 I2S format");
	} else {
		/* FMT : Left Justified */
		wm8994_write(codec, WM8994_AIF2_CONTROL_1, 0x4008);
		DEBUG_LOG("AIF2 Left Justified format");
	}

	wm8994_write(codec, WM8994_AIF2_BCLK, 0x70);
	wm8994_write(codec, WM8994_AIF2_CONTROL_2, 0x0000);
	wm8994_write(codec, WM8994_AIF2_MASTER_SLAVE, WM8994_AIF2_MSTR |
				WM8994_AIF2_CLK_FRC | WM8994_AIF2_LRCLK_FRC);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_5);
	val &= ~(WM8994_AIF2DACL_ENA_MASK | WM8994_AIF2DACR_ENA_MASK |
			WM8994_AIF1DAC1L_ENA_MASK | WM8994_AIF1DAC1R_ENA_MASK |
			WM8994_DAC1L_ENA_MASK | WM8994_DAC1R_ENA_MASK);
	val |= (WM8994_AIF2DACL_ENA | WM8994_AIF2DACR_ENA |
				WM8994_AIF1DAC1L_ENA | WM8994_AIF1DAC1R_ENA |
				WM8994_DAC1L_ENA | WM8994_DAC1R_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_5, val);

	/* Clocking */
	val = wm8994_read(codec, WM8994_CLOCKING_1);
	val |= (WM8994_DSP_FS2CLK_ENA | WM8994_SYSCLK_SRC);
	wm8994_write(codec, WM8994_CLOCKING_1, val);

	/* AIF1 & AIF2 Output is connected to DAC1 */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1L_TO_DAC1L_MASK |
		WM8994_AIF2DACL_TO_DAC1L_MASK);
	val |= (WM8994_AIF1DAC1L_TO_DAC1L | WM8994_AIF2DACL_TO_DAC1L);
	wm8994_write(codec, WM8994_DAC1_LEFT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1R_TO_DAC1R_MASK |
		WM8994_AIF2DACR_TO_DAC1R_MASK);
	val |= (WM8994_AIF1DAC1R_TO_DAC1R | WM8994_AIF2DACR_TO_DAC1R);
	wm8994_write(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING, val);
}

void wm8994_record_headset_mic(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;
	u16 val;


	DEBUG_LOG("Recording through Headset Mic\n");
	audio_ctrl_mic_bias_gpio(wm8994->pdata, 0);

	if (!(wm8994->codec_state & CALL_ACTIVE)) {
		/* Disable FM radio path */
		val = wm8994_read(codec, WM8994_SPEAKER_MIXER);
		val &= ~(WM8994_MIXINL_TO_SPKMIXL_MASK);
		wm8994_write(codec, WM8994_SPEAKER_MIXER, val);

		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
		val &= ~(WM8994_MIXINL_TO_MIXOUTL_MASK);
		wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val); 

		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
		val &= ~(WM8994_MIXINR_TO_MIXOUTR_MASK);
		wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);

		val = wm8994_read(codec, WM8994_DAC2_LEFT_MIXER_ROUTING);
		val &= ~(WM8994_ADC1_TO_DAC2L_MASK);
		wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING, val);

		val = wm8994_read(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING);
		val &= ~(WM8994_ADC1_TO_DAC2R_MASK);
		wm8994_write(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING, val);

		/* Mixing left channel output to right channel. */
		val = wm8994_read(codec, WM8994_AIF1_CONTROL_1);
		val &= ~(WM8994_AIF1ADCL_SRC_MASK | WM8994_AIF1ADCR_SRC_MASK);
		val |= (WM8994_AIF1ADCL_SRC | WM8994_AIF1ADCR_SRC);
		wm8994_write(codec, WM8994_AIF1_CONTROL_1, val);

		wm8994_write(codec, WM8994_ANTIPOP_2, 0x68);

		/* Enable mic bias, vmid, bias generator */
		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
		val &= ~(WM8994_BIAS_ENA_MASK | WM8994_VMID_SEL_MASK);
		val |= (WM8994_BIAS_ENA | WM8994_VMID_SEL_NORMAL);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

		wm8994_write(codec, WM8994_POWER_MANAGEMENT_2, 
					(WM8994_MIXINR_ENA | WM8994_IN1R_ENA));

		if (!wm8994->testmode_config_flag) {	
			/*  Enable volume,unmute Right Line */	
			val = wm8994_read(codec, 
					WM8994_RIGHT_LINE_INPUT_1_2_VOLUME);	
			val &= ~(WM8994_IN1R_MUTE_MASK | WM8994_IN1R_VOL_MASK);
			if (wm8994->recognition_active == RECG_ON)
				val |= (WM8994_IN1R_VU | 
					TUNING_RECOGNITION_SUB_INPUTMIX_VOL);
			else
				val |= (WM8994_IN1R_VU | 
						TUNING_RECORD_SUB_INPUTMIX_VOL);
			wm8994_write(codec, 
				WM8994_RIGHT_LINE_INPUT_1_2_VOLUME, val);

			/* unmute right pga, set volume */
			val = wm8994_read(codec, WM8994_INPUT_MIXER_4);
			val &= ~(WM8994_IN1R_TO_MIXINR_MASK | 
						WM8994_IN1R_MIXINR_VOL_MASK | 
						WM8994_MIXOUTR_MIXINR_VOL_MASK);
			if (wm8994->recognition_active == RECG_ON)
				val |= WM8994_IN1R_TO_MIXINR;
			else
				val |= (WM8994_IN1R_TO_MIXINR | 
							WM8994_IN1R_MIXINR_VOL);
			wm8994_write(codec, WM8994_INPUT_MIXER_4, val);
		}

		val = wm8994_read(codec, WM8994_AIF1_ADC1_LEFT_VOLUME);
		val &= ~(WM8994_AIF1ADC1L_VOL_MASK);
		if (wm8994->recognition_active == RECG_ON)		
			val |= (WM8994_AIF1ADC1_VU | 
					TUNING_RECOGNITION_SUB_AIF1ADCL_VOL); 
		else
			val |= (WM8994_AIF1ADC1_VU | 
						TUNING_RECORD_SUB_AIF1ADCL_VOL);
		wm8994_write(codec, WM8994_AIF1_ADC1_LEFT_VOLUME, val);

		val = wm8994_read(codec, WM8994_AIF1_ADC1_RIGHT_VOLUME);
		val &= ~(WM8994_AIF1ADC1R_VOL_MASK);
		if (wm8994->recognition_active == RECG_ON)		
			val |= (WM8994_AIF1ADC1_VU | 
					TUNING_RECOGNITION_SUB_AIF1ADCR_VOL);
		else
			val |= (WM8994_AIF1ADC1_VU | 
						TUNING_RECORD_SUB_AIF1ADCR_VOL);
		wm8994_write(codec, WM8994_AIF1_ADC1_RIGHT_VOLUME, val);

		val = (WM8994_IN1RN_TO_IN1R | WM8994_IN1RP_TO_IN1R) ; 
		wm8994_write(codec, WM8994_INPUT_MIXER_2, val);
	} else {
		/* Mixing left channel output to right channel. */
		val = wm8994_read(codec, WM8994_AIF1_CONTROL_1);
		val &= ~(WM8994_AIF1ADCL_SRC_MASK | WM8994_AIF1ADCR_SRC_MASK);
		val |= (WM8994_AIF1ADCL_SRC | WM8994_AIF1ADCR_SRC);
		wm8994_write(codec, WM8994_AIF1_CONTROL_1, val);
	}

	/* Digital Paths */	
	/* Enable right ADC and time slot */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_4);
	val &= ~(WM8994_ADCR_ENA_MASK |WM8994_AIF1ADC1R_ENA_MASK);
	val |= (WM8994_AIF1ADC1R_ENA | WM8994_ADCR_ENA );
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, val);

	/* ADC Right mixer routing */
	val = wm8994_read(codec, WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING);
	val &= ~(WM8994_ADC1R_TO_AIF1ADC1R_MASK);
	val |= WM8994_ADC1R_TO_AIF1ADC1R;
	wm8994_write(codec, WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_AIF1_MASTER_SLAVE);
	val |= (WM8994_AIF1_MSTR | WM8994_AIF1_CLK_FRC | WM8994_AIF1_LRCLK_FRC);	
	wm8994_write(codec, WM8994_AIF1_MASTER_SLAVE, val);

	/* GPIO1 is Input Enable */
	wm8994_write(codec, WM8994_GPIO_1, 0xA101);
}

void wm8994_record_main_mic(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;
	u16 val;


	DEBUG_LOG("Recording through Main Mic\n");
	audio_ctrl_mic_bias_gpio(wm8994->pdata, 1);

	if (!(wm8994->codec_state & CALL_ACTIVE)) {
		/* Disable FM radio path */
		val = wm8994_read(codec, WM8994_SPEAKER_MIXER);
		val &= ~(WM8994_MIXINL_TO_SPKMIXL_MASK);
		wm8994_write(codec, WM8994_SPEAKER_MIXER, val);

		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
		val &= ~(WM8994_MIXINL_TO_MIXOUTL_MASK);
		wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val); 

		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
		val &= ~(WM8994_MIXINR_TO_MIXOUTR_MASK);
		wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val); 

		val = wm8994_read(codec, WM8994_DAC2_LEFT_MIXER_ROUTING);
		val &= ~(WM8994_ADC1_TO_DAC2L_MASK);
		wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING, val);

		val = wm8994_read(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING);
		val &= ~(WM8994_ADC1_TO_DAC2R_MASK);
		wm8994_write(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING, val);

		/* Mixing left channel output to right channel. */
		val = wm8994_read(codec, WM8994_AIF1_CONTROL_1);
		val &= ~(WM8994_AIF1ADCL_SRC_MASK | WM8994_AIF1ADCR_SRC_MASK);
		wm8994_write(codec, WM8994_AIF1_CONTROL_1, val);

		wm8994_write(codec, WM8994_ANTIPOP_2, 0x68);

		/* Enable micbias,vmid,mic1 */
		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
		val &= ~(WM8994_BIAS_ENA_MASK | WM8994_VMID_SEL_MASK);
		val |= (WM8994_BIAS_ENA | WM8994_VMID_SEL_NORMAL);  
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

		wm8994_write(codec, WM8994_POWER_MANAGEMENT_2, 
                                        (WM8994_MIXINL_ENA | WM8994_IN1L_ENA));

		val = wm8994_read(codec, WM8994_AIF1_ADC1_FILTERS);
		val &= ~(WM8994_AIF1ADC1L_HPF_MASK | WM8994_AIF1ADC1R_HPF_MASK);
		val |= (WM8994_AIF1ADC1L_HPF | WM8994_AIF1ADC1R_HPF);  	
		wm8994_write(codec, WM8994_AIF1_ADC1_FILTERS, val);


		if (!wm8994->testmode_config_flag) {
			/* Unmute IN1L PGA, update volume */
			val = wm8994_read(codec, 
					WM8994_LEFT_LINE_INPUT_1_2_VOLUME);	
			val &= ~(WM8994_IN1L_MUTE_MASK | WM8994_IN1L_VOL_MASK);
			if (wm8994->recognition_active == RECG_ON)
				val |= (WM8994_IN1L_VU | 
					TUNING_RECOGNITION_MAIN_INPUTLINE_VOL);
			else
				val |= (WM8994_IN1L_VU |
					TUNING_RECORD_MAIN_INPUTLINE_VOL);
			wm8994_write(codec, 
					WM8994_LEFT_LINE_INPUT_1_2_VOLUME, val);

			/* Unmute IN2R PGA, update volume */
			val = wm8994_read(codec, 
					WM8994_RIGHT_LINE_INPUT_3_4_VOLUME);	
			val |= WM8994_IN2R_MUTE;
			wm8994_write(codec, 
				WM8994_RIGHT_LINE_INPUT_3_4_VOLUME, val);

			/* Unmute the PGA */
			val = wm8994_read(codec, WM8994_INPUT_MIXER_3);
			val&= ~(WM8994_IN1L_TO_MIXINL_MASK | 
						WM8994_IN1L_MIXINL_VOL_MASK | 
						WM8994_MIXOUTL_MIXINL_VOL_MASK);
			if (wm8994->recognition_active == RECG_ON)		
				val |= WM8994_IN1L_TO_MIXINL;
			else
				val |= WM8994_IN1L_TO_MIXINL;
			wm8994_write(codec, WM8994_INPUT_MIXER_3, val); 

			wm8994_write(codec, WM8994_INPUT_MIXER_4, 0x0000);
		}

		wm8994_write(codec, WM8994_INPUT_MIXER_2, 
                                WM8994_IN1LP_TO_IN1L | WM8994_IN1LN_TO_IN1L);

		/* Digital Paths */
		val = wm8994_read(codec, WM8994_AIF1_ADC1_LEFT_VOLUME);
		val &= ~(WM8994_AIF1ADC1L_VOL_MASK);
		if (wm8994->recognition_active == RECG_ON)		
			val |= (WM8994_AIF1ADC1_VU | 
					TUNING_RECOGNITION_MAIN_AIF1ADCL_VOL);
		else
			val |= (WM8994_AIF1ADC1_VU | 
					TUNING_RECORD_MAIN_AIF1ADCL_VOL);
		wm8994_write(codec, WM8994_AIF1_ADC1_LEFT_VOLUME, val);

		val = wm8994_read(codec, WM8994_AIF1_ADC1_RIGHT_VOLUME);
		val &= ~(WM8994_AIF1ADC1R_VOL_MASK);
		if (wm8994->recognition_active == RECG_ON)		
			val |= (WM8994_AIF1ADC1_VU | 
					TUNING_RECOGNITION_MAIN_AIF1ADCR_VOL);
		else
			val |= (WM8994_AIF1ADC1_VU | 
					TUNING_RECORD_MAIN_AIF1ADCR_VOL);
		wm8994_write(codec, WM8994_AIF1_ADC1_RIGHT_VOLUME, val);
        } else {
                val = wm8994_read(codec, WM8994_AIF1_CONTROL_1);
                val &= ~(WM8994_AIF1ADCL_SRC_MASK | WM8994_AIF1ADCR_SRC_MASK);
                wm8994_write(codec, WM8994_AIF1_CONTROL_1, val);
	}

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_4);
	val &= ~(WM8994_ADCL_ENA_MASK |WM8994_AIF1ADC1L_ENA_MASK);
	val |= (WM8994_AIF1ADC1L_ENA | WM8994_ADCL_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, val);

	/* Enable timeslots */
	val = wm8994_read(codec, WM8994_AIF1_ADC1_LEFT_MIXER_ROUTING);
	val |= WM8994_ADC1L_TO_AIF1ADC1L ;  
	wm8994_write(codec, WM8994_AIF1_ADC1_LEFT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_AIF1_MASTER_SLAVE);
	val |= (WM8994_AIF1_MSTR | WM8994_AIF1_CLK_FRC | WM8994_AIF1_LRCLK_FRC);	
	wm8994_write(codec, WM8994_AIF1_MASTER_SLAVE, val);

	/* GPIO1 is Input Enable */
	wm8994_write(codec, WM8994_GPIO_1, 0xA101);

	/* For avoiding pop noise when start google voice search */
	if (wm8994->recognition_active == RECG_ON)
		msleep(60);

}

void wm8994_record_2nd_main_mic(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;
	u16 val;


	DEBUG_LOG("Recording through 2nd Main Mic\n");
	audio_ctrl_mic_bias_gpio(wm8994->pdata, 1);

	if (!(wm8994->codec_state & CALL_ACTIVE)) {
		/* Disable FM radio path */
		val = wm8994_read(codec, WM8994_SPEAKER_MIXER);
		val &= ~(WM8994_MIXINL_TO_SPKMIXL_MASK);
		wm8994_write(codec, WM8994_SPEAKER_MIXER, val);

		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
		val &= ~(WM8994_MIXINL_TO_MIXOUTL_MASK);
		wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val); 

		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
		val &= ~(WM8994_MIXINR_TO_MIXOUTR_MASK);
		wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);

		val = wm8994_read(codec, WM8994_DAC2_LEFT_MIXER_ROUTING);
		val &= ~(WM8994_ADC1_TO_DAC2L_MASK);
		wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING, val);

		val = wm8994_read(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING);
		val &= ~(WM8994_ADC1_TO_DAC2R_MASK);
		wm8994_write(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING, val);

		/* Mixing left channel output to right channel. */
		val = wm8994_read(codec, WM8994_AIF1_CONTROL_1);
		val &= ~(WM8994_AIF1ADCL_SRC_MASK | WM8994_AIF1ADCR_SRC_MASK);
		val |= (WM8994_AIF1ADCL_SRC | WM8994_AIF1ADCR_SRC);
		wm8994_write(codec, WM8994_AIF1_CONTROL_1, val);

		wm8994_write(codec, WM8994_ANTIPOP_2, 0x68);

		/* Enable micbias,vmid,mic1 */
		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
		val &= ~(WM8994_BIAS_ENA_MASK | WM8994_VMID_SEL_MASK);
		val |= (WM8994_MICB1_ENA | WM8994_BIAS_ENA | 
							WM8994_VMID_SEL_NORMAL);	
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

		wm8994_write(codec, WM8994_POWER_MANAGEMENT_2, 
					WM8994_TSHUT_ENA |WM8994_TSHUT_OPDIS |
					WM8994_MIXINR_ENA | WM8994_IN2R_ENA);

		val = wm8994_read(codec, WM8994_AIF1_ADC1_LEFT_VOLUME);
		val &= ~(WM8994_AIF1ADC1L_VOL_MASK);
		val |= (WM8994_AIF1ADC1_VU | TUNING_RECORD_SUB_AIF1ADCL_VOL);
		wm8994_write(codec, WM8994_AIF1_ADC1_LEFT_VOLUME, val);

		val = wm8994_read(codec, WM8994_AIF1_ADC1_RIGHT_VOLUME);
		val &= ~(WM8994_AIF1ADC1R_VOL_MASK);
		val |= (WM8994_AIF1ADC1_VU | TUNING_RECORD_SUB_AIF1ADCR_VOL);
		wm8994_write(codec, WM8994_AIF1_ADC1_RIGHT_VOLUME, val);
	} else {
		/* Mixing left channel output to right channel. */
		val = wm8994_read(codec, WM8994_AIF1_CONTROL_1);
		val &= ~(WM8994_AIF1ADCL_SRC_MASK | WM8994_AIF1ADCR_SRC_MASK);
		val |= (WM8994_AIF1ADCL_SRC | WM8994_AIF1ADCR_SRC);
		wm8994_write(codec, WM8994_AIF1_CONTROL_1, val);
	}

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_4);
	val &= ~(WM8994_ADCL_ENA_MASK |WM8994_AIF1ADC1R_ENA_MASK | 
			WM8994_AIF1ADC1L_ENA_MASK | WM8994_ADCR_ENA_MASK);
	val |= (WM8994_AIF1ADC1L_ENA | WM8994_AIF1ADC1R_ENA | 
					WM8994_ADCL_ENA | WM8994_ADCR_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, val);

	/* Enable timeslots */
	val = wm8994_read(codec, WM8994_AIF1_ADC1_LEFT_MIXER_ROUTING);
	val |= WM8994_ADC1L_TO_AIF1ADC1L ;  
	wm8994_write(codec, WM8994_AIF1_ADC1_LEFT_MIXER_ROUTING, val);

	/* ADC Right mixer routing */
	val = wm8994_read(codec, WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING);
	val &= ~(WM8994_ADC1R_TO_AIF1ADC1R_MASK);
	val |= WM8994_ADC1R_TO_AIF1ADC1R;
	wm8994_write(codec, WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_AIF1_MASTER_SLAVE);
	val |= (WM8994_AIF1_MSTR | WM8994_AIF1_CLK_FRC | WM8994_AIF1_LRCLK_FRC);	
	wm8994_write(codec, WM8994_AIF1_MASTER_SLAVE, val);

	/* GPIO1 is Input Enable */		
	wm8994_write(codec, WM8994_GPIO_1, 0xA101);

	wm8994_write(codec, 0x1B, 0x109);
	wm8994_write(codec, 0x28, 0x0004);
	wm8994_write(codec, 0x2a, 0x180);
}

void wm8994_record_bluetooth(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;
	u16 val;


	DEBUG_LOG("BT Record Path for Voice Command\n");

	if (!(wm8994->codec_state & CALL_ACTIVE)) {
		wm8994_set_bluetooth_common_setting(codec);
	
		val = wm8994_read(codec, WM8994_DAC2_LEFT_MIXER_ROUTING);
		val &= ~(WM8994_ADC1_TO_DAC2L_MASK);
		wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING, val);

		val = wm8994_read(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING);
		val &= ~(WM8994_ADC1_TO_DAC2R_MASK);
		wm8994_write(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING, val);

		/* R1(1h) - 0x0003 -normal vmid */
		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
		val &= ~(WM8994_BIAS_ENA_MASK | WM8994_VMID_SEL_MASK);
		val |= (WM8994_BIAS_ENA | WM8994_VMID_SEL_NORMAL);	
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

		/*Digital Path Enables and Unmutes*/	
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_2, 0x0000);

		wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, 0x0000);

		/* AIF1ADC1(Left/Right) Output */
		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_4);
		val &= ~(WM8994_AIF1ADC1L_ENA_MASK |WM8994_AIF1ADC1R_ENA_MASK);
		val |= (WM8994_AIF1ADC1L_ENA | WM8994_AIF1ADC1R_ENA);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_4 , val);

		/* AIF2DAC(Left/Right) Input */
		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_5);
		val &= ~(WM8994_AIF2DACL_ENA_MASK | WM8994_AIF2DACR_ENA_MASK);
		val |= (WM8994_AIF2DACL_ENA | WM8994_AIF2DACR_ENA);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_5, val);

		/* AIF2_DACDAT_SRC(GPIO8/DACDAT3) */
		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_6);
		val &= ~(WM8994_AIF3_ADCDAT_SRC_MASK | 
						WM8994_AIF2_DACDAT_SRC_MASK);
		val |= (0x1 << WM8994_AIF3_ADCDAT_SRC_SHIFT |
			WM8994_AIF2_DACDAT_SRC);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_6, val);

		/* Un-Mute */
		/* AIF2DAC_MUTE(Un-mute) */
		val = wm8994_read(codec, WM8994_AIF2_DAC_FILTERS_1);
		val &= ~(WM8994_AIF2DAC_MUTE_MASK);
		wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_1, val); 

		/* Mixer Routing */
		/* AIF2DACL_TO_AIF1ADC1L */
		val = wm8994_read(codec, WM8994_AIF1_ADC1_LEFT_MIXER_ROUTING);
		val &= ~(WM8994_AIF2DACL_TO_AIF1ADC1L_MASK);
		val |= (WM8994_AIF2DACL_TO_AIF1ADC1L);
		wm8994_write(codec, WM8994_AIF1_ADC1_LEFT_MIXER_ROUTING, val);

		/* AIF2DACR_TO_AIF1ADC1R */
		val = wm8994_read(codec, WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING);
		val &= ~(WM8994_AIF2DACR_TO_AIF1ADC1R_MASK);
		val |= (WM8994_AIF2DACR_TO_AIF1ADC1R);
		wm8994_write(codec, WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING, val);

		wm8994_write(codec, WM8994_AIF2_CLOCKING_1, 0x0019);

		wm8994_write(codec, WM8994_OVERSAMPLING, 0X0000);

		/* AIF1_ADC1 volume */
		val = wm8994_read(codec, WM8994_AIF1_ADC1_LEFT_VOLUME);
		val &= ~(WM8994_AIF1ADC1L_VOL_MASK);
		val |= TUNING_RECORD_BT_AIF1ADCL_VOL;
		wm8994_write(codec, WM8994_AIF1_ADC1_LEFT_VOLUME, val);

		val = wm8994_read(codec, WM8994_AIF1_ADC1_RIGHT_VOLUME);
		val &= ~(WM8994_AIF1ADC1R_VOL_MASK);
		val |= (WM8994_AIF1ADC1_VU | TUNING_RECORD_BT_AIF1ADCR_VOL);
		wm8994_write(codec, WM8994_AIF1_ADC1_RIGHT_VOLUME, val);

		/* GPIO Configuration */		
		wm8994_write(codec, WM8994_GPIO_8, WM8994_GP8_DIR | 
								WM8994_GP8_DB);
		wm8994_write(codec, WM8994_GPIO_9, WM8994_GP9_DB);
		wm8994_write(codec, WM8994_GPIO_10, WM8994_GP10_DB);
		wm8994_write(codec, WM8994_GPIO_11, WM8994_GP11_DB); 
	}
}

void wm8994_set_playback_receiver(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;
	u16 val;


	DEBUG_LOG("");

	if (!wm8994->testmode_config_flag) {
		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_5);
		val &= ~(WM8994_DACL_MIXOUTL_VOL_MASK);
		val |= TUNING_RCV_OUTMIX5_VOL << WM8994_DACL_MIXOUTL_VOL_SHIFT;
		wm8994_write(codec, WM8994_OUTPUT_MIXER_5, val);

		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_6);
		val &= ~(WM8994_DACR_MIXOUTR_VOL_MASK);
		val |= TUNING_RCV_OUTMIX6_VOL << WM8994_DACR_MIXOUTR_VOL_SHIFT;
		wm8994_write(codec, WM8994_OUTPUT_MIXER_6, val);

		val = wm8994_read(codec, WM8994_LEFT_OPGA_VOLUME);
		val &= ~(WM8994_MIXOUTL_MUTE_N_MASK | WM8994_MIXOUTL_VOL_MASK);
		val |= (WM8994_MIXOUT_VU |WM8994_MIXOUTL_MUTE_N | 
							TUNING_RCV_OPGAL_VOL);
		wm8994_write(codec, WM8994_LEFT_OPGA_VOLUME, val);

		val = wm8994_read(codec, WM8994_RIGHT_OPGA_VOLUME);
		val &= ~(WM8994_MIXOUTR_MUTE_N_MASK | WM8994_MIXOUTR_VOL_MASK);
		val |= (WM8994_MIXOUT_VU | WM8994_MIXOUTR_MUTE_N | 
							TUNING_RCV_OPGAR_VOL);
		wm8994_write(codec, WM8994_RIGHT_OPGA_VOLUME, val);

		val = wm8994_read(codec, WM8994_HPOUT2_VOLUME);
		val &= ~(WM8994_HPOUT2_MUTE_MASK | WM8994_HPOUT2_VOL_MASK);
		val |= TUNING_HPOUT2_VOL << WM8994_HPOUT2_VOL_SHIFT;
		wm8994_write(codec, WM8994_HPOUT2_VOLUME, val);

		/* Unmute DAC1 left */
		val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
		val &= ~(WM8994_DAC1L_MUTE_MASK | WM8994_DAC1L_VOL_MASK);
		val |= TUNING_DAC1L_VOL; 
		wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME ,val);

		/* Unmute and volume ctrl RightDAC */
		val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME); 
		val &= ~(WM8994_DAC1R_MUTE_MASK | WM8994_DAC1R_VOL_MASK);
		val |= TUNING_DAC1R_VOL;
		wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME,val);

		val = wm8994_read(codec, WM8994_AIF1_DAC1_LEFT_VOLUME);
		val &= ~(WM8994_AIF1DAC1L_VOL_MASK);
		val |= (WM8994_AIF1DAC1_VU | TUNING_DAC1L_VOL);
		wm8994_write(codec, WM8994_AIF1_DAC1_LEFT_VOLUME, val);

		val = wm8994_read(codec, WM8994_AIF1_DAC1_RIGHT_VOLUME);
		val &= ~(WM8994_AIF1DAC1R_VOL_MASK);
		val |= (WM8994_AIF1DAC1_VU | TUNING_DAC1R_VOL);
		wm8994_write(codec, WM8994_AIF1_DAC1_RIGHT_VOLUME, val);
	}

	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
	val &= ~(WM8994_DAC1L_TO_MIXOUTL_MASK);
	val |= WM8994_DAC1L_TO_MIXOUTL;
	wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val);

	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
	val &= ~(WM8994_DAC1R_TO_MIXOUTR_MASK);
	val |= WM8994_DAC1R_TO_MIXOUTR;
	wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);

	val = wm8994_read(codec, WM8994_HPOUT2_MIXER);
	val &= ~(WM8994_MIXOUTLVOL_TO_HPOUT2_MASK | 
		WM8994_MIXOUTRVOL_TO_HPOUT2_MASK);
	val |= (WM8994_MIXOUTRVOL_TO_HPOUT2 | WM8994_MIXOUTLVOL_TO_HPOUT2);
	wm8994_write(codec, WM8994_HPOUT2_MIXER, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_5);
	val &= ~(WM8994_DAC1R_ENA_MASK | WM8994_DAC1L_ENA_MASK | 
		WM8994_AIF1DAC1R_ENA_MASK | WM8994_AIF1DAC1L_ENA_MASK);
	val |= (WM8994_AIF1DAC1L_ENA | WM8994_AIF1DAC1R_ENA | 
		WM8994_DAC1L_ENA | WM8994_DAC1R_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_5, val);

	val = wm8994_read(codec, WM8994_AIF1_DAC1_FILTERS_1);
	val &= ~(WM8994_AIF1DAC1_MUTE_MASK |WM8994_AIF1DAC1_MONO_MASK);
	val |= (WM8994_AIF1DAC1_UNMUTE | WM8994_AIF1DAC1_MONO);
	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, val);

	val = wm8994_read(codec, WM8994_DAC1_LEFT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1L_TO_DAC1L_MASK);
	val |= WM8994_AIF1DAC1L_TO_DAC1L;
	wm8994_write(codec, WM8994_DAC1_LEFT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1R_TO_DAC1R_MASK);
	val |= WM8994_AIF1DAC1R_TO_DAC1R;
	wm8994_write(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_CLOCKING_1);
	val &= ~(WM8994_DSP_FS1CLK_ENA_MASK | WM8994_DSP_FSINTCLK_ENA_MASK);
	val |= (WM8994_DSP_FS1CLK_ENA | WM8994_DSP_FSINTCLK_ENA);
	wm8994_write(codec, WM8994_CLOCKING_1, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_3);
	val &= ~(WM8994_MIXOUTLVOL_ENA_MASK | WM8994_MIXOUTRVOL_ENA_MASK | 
			WM8994_MIXOUTL_ENA_MASK | WM8994_MIXOUTR_ENA_MASK);
	val |= (WM8994_MIXOUTL_ENA | WM8994_MIXOUTR_ENA | 
				WM8994_MIXOUTRVOL_ENA | WM8994_MIXOUTLVOL_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
	val &= ~(WM8994_BIAS_ENA_MASK | WM8994_VMID_SEL_MASK | 
			WM8994_HPOUT2_ENA_MASK | WM8994_HPOUT1L_ENA_MASK |
			WM8994_HPOUT1R_ENA_MASK | WM8994_SPKOUTL_ENA_MASK);
	val |= (WM8994_BIAS_ENA | WM8994_VMID_SEL_NORMAL | WM8994_HPOUT2_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);
}

void wm8994_set_playback_headset(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;
	u16 val;

	u16 TestReturn1 = 0;
	u16 TestReturn2 = 0;
	u16 TestLow1 = 0;
	u16 TestHigh1 = 0;
	u8 TestLow = 0;
	u8 TestHigh = 0;


	DEBUG_LOG("");

	/* Enable the Timeslot0 to DAC1L */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_MIXER_ROUTING); 	
	val &= ~(WM8994_AIF1DAC1L_TO_DAC1L_MASK);
	val |= WM8994_AIF1DAC1L_TO_DAC1L;
	wm8994_write(codec, WM8994_DAC1_LEFT_MIXER_ROUTING, val);

	/* Enable the Timeslot0 to DAC1R */
	val = wm8994_read(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING); 	
	val &= ~(WM8994_AIF1DAC1R_TO_DAC1R_MASK);
	val |= WM8994_AIF1DAC1R_TO_DAC1R;
	wm8994_write(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING, val);

	val = wm8994_read(codec, 0x102); 	
	val &= ~(0x0003);
	val = 0x0003;
	wm8994_write(codec, 0x102, val);

	val = wm8994_read(codec, 0x56); 	
	val &= ~(0x0003);
	val = 0x0003;
	wm8994_write(codec, 0x56, val);

	val = wm8994_read(codec, 0x102); 	
	val &= ~(0x0000);
	val = 0x0000;
	wm8994_write(codec, 0x102, val);

	val = wm8994_read(codec, WM8994_CLASS_W_1); 	
	val &= ~(0x0005);
	val |= 0x0005;
	wm8994_write(codec, WM8994_CLASS_W_1, val);

	if (!wm8994->testmode_config_flag) {
		val = wm8994_read(codec, WM8994_LEFT_OUTPUT_VOLUME);
		val &= ~(WM8994_HPOUT1L_MUTE_N_MASK | WM8994_HPOUT1L_VOL_MASK);
		if (wm8994->output_source == RING_TONE)
			val |= (WM8994_HPOUT1_VU | WM8994_HPOUT1L_MUTE_N | 
						TUNING_RING_OUTPUTL_VOL);
		else
			val |= (WM8994_HPOUT1_VU | WM8994_HPOUT1L_MUTE_N | 
							TUNING_MP3_OUTPUTL_VOL);
		wm8994_write(codec, WM8994_LEFT_OUTPUT_VOLUME, val);


		val = wm8994_read(codec, WM8994_RIGHT_OUTPUT_VOLUME);
		val &= ~(WM8994_HPOUT1R_MUTE_N_MASK | WM8994_HPOUT1R_VOL_MASK);
		if (wm8994->output_source == RING_TONE)
			val |= (WM8994_HPOUT1_VU | WM8994_HPOUT1R_MUTE_N | 
						TUNING_RING_OUTPUTR_VOL);
		else
			val |= (WM8994_HPOUT1_VU | WM8994_HPOUT1R_MUTE_N | 
							TUNING_MP3_OUTPUTR_VOL);
		wm8994_write(codec, WM8994_RIGHT_OUTPUT_VOLUME, val);


		val = wm8994_read(codec, WM8994_LEFT_OPGA_VOLUME);
		val &= ~(WM8994_MIXOUTL_MUTE_N_MASK | WM8994_MIXOUTL_VOL_MASK);
		if (wm8994->output_source == RING_TONE)
			val |= (WM8994_MIXOUT_VU | WM8994_MIXOUTL_MUTE_N | 
							TUNING_RING_OPGAL_VOL);
		else
			val |= (WM8994_MIXOUT_VU | WM8994_MIXOUTL_MUTE_N | 
							TUNING_MP3_OPGAL_VOL);
		wm8994_write(codec, WM8994_LEFT_OPGA_VOLUME, val);

		val = wm8994_read(codec, WM8994_RIGHT_OPGA_VOLUME);
		val &= ~(WM8994_MIXOUTR_MUTE_N_MASK | WM8994_MIXOUTR_VOL_MASK);
		if (wm8994->output_source == RING_TONE)
			val |= (WM8994_MIXOUT_VU | WM8994_MIXOUTR_MUTE_N | 
							TUNING_RING_OPGAR_VOL);
		else
			val |= (WM8994_MIXOUT_VU | WM8994_MIXOUTR_MUTE_N | 
							TUNING_MP3_OPGAR_VOL);
		wm8994_write(codec, WM8994_RIGHT_OPGA_VOLUME, val);

		val = wm8994_read(codec, WM8994_AIF1_DAC1_LEFT_VOLUME);
		val &= ~(WM8994_AIF1DAC1L_VOL_MASK);
		val |= (WM8994_AIF1DAC1_VU | TUNING_DAC1L_VOL);
		wm8994_write(codec, WM8994_AIF1_DAC1_LEFT_VOLUME, val);

		val = wm8994_read(codec, WM8994_AIF1_DAC1_RIGHT_VOLUME);
		val &= ~(WM8994_AIF1DAC1R_VOL_MASK);
		val |= (WM8994_AIF1DAC1_VU |TUNING_DAC1R_VOL);
		wm8994_write(codec, WM8994_AIF1_DAC1_RIGHT_VOLUME, val);
	}

	val = wm8994_read(codec, WM8994_DC_SERVO_2); 	
	val &= ~(0x03E0);
	val = 0x03E0;
	wm8994_write(codec, WM8994_DC_SERVO_2, val);

	/* Enable vmid,bias, hp left and right */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
	val &= ~(WM8994_BIAS_ENA_MASK | WM8994_VMID_SEL_MASK |
			WM8994_HPOUT1L_ENA_MASK |WM8994_HPOUT1R_ENA_MASK |
			WM8994_SPKOUTR_ENA_MASK | WM8994_SPKOUTL_ENA_MASK);
	val |= (WM8994_BIAS_ENA | WM8994_VMID_SEL_NORMAL |
				WM8994_HPOUT1R_ENA | WM8994_HPOUT1L_ENA);  
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	val = wm8994_read(codec, WM8994_ANALOGUE_HP_1); 	
	val &= ~(0x0022);
	val = 0x0022;
	wm8994_write(codec, WM8994_ANALOGUE_HP_1, val);

	/* Enable Charge Pump */	
	val = wm8994_read(codec, WM8994_CHARGE_PUMP_1);
	val &= ~WM8994_CP_ENA_MASK;
	val |= WM8994_CP_ENA | WM8994_CP_ENA_DEFAULT ;
	wm8994_write(codec, WM8994_CHARGE_PUMP_1, val);

	msleep(5);

	/* Enable Dac1 and DAC2 and the Timeslot0 for AIF1 */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_5); 	
	val &= ~(WM8994_DAC1R_ENA_MASK | WM8994_DAC1L_ENA_MASK |
			WM8994_AIF1DAC1R_ENA_MASK | WM8994_AIF1DAC1L_ENA_MASK);
	val |= (WM8994_AIF1DAC1L_ENA | WM8994_AIF1DAC1R_ENA | 
					WM8994_DAC1L_ENA | WM8994_DAC1R_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_5 ,val);

	/* Enable DAC1L to HPOUT1L path */
	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
	val &=  ~(WM8994_DAC1L_TO_HPOUT1L_MASK | WM8994_DAC1L_TO_MIXOUTL_MASK);
	val |= WM8994_DAC1L_TO_MIXOUTL ;  	
	wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val);

	/* Enable DAC1R to HPOUT1R path */
	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
	val &= ~(WM8994_DAC1R_TO_HPOUT1R_MASK | WM8994_DAC1R_TO_MIXOUTR_MASK);
	val |= WM8994_DAC1R_TO_MIXOUTR;
	wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, 0x0030);

	val = wm8994_read(codec, WM8994_DC_SERVO_1); 	
	val &= ~(0x0303);
	val = 0x0303;
	wm8994_write(codec, WM8994_DC_SERVO_1, val);

	msleep(160);

	TestReturn1 = wm8994_read(codec, WM8994_DC_SERVO_4);

	TestLow = (signed char)(TestReturn1 & 0xff);
	TestHigh = (signed char)((TestReturn1>>8) & 0xff);

	TestLow1 = ((signed short)(TestLow-5))&0x00ff;
	TestHigh1 = (((signed short)(TestHigh-5)<<8)&0xff00);
	TestReturn2 = TestLow1 | TestHigh1;
	wm8994_write(codec, WM8994_DC_SERVO_4, TestReturn2);

	val = wm8994_read(codec, WM8994_DC_SERVO_1); 	
	val &= ~(0x000F);
	val = 0x000F;
	wm8994_write(codec, WM8994_DC_SERVO_1, val);

	msleep(20);

	/* Intermediate HP settings */
	val = wm8994_read(codec, WM8994_ANALOGUE_HP_1); 	
	val &= ~(WM8994_HPOUT1R_DLY_MASK |WM8994_HPOUT1R_OUTP_MASK |
		WM8994_HPOUT1R_RMV_SHORT_MASK | WM8994_HPOUT1L_DLY_MASK |
		WM8994_HPOUT1L_OUTP_MASK | WM8994_HPOUT1L_RMV_SHORT_MASK);
	val = (WM8994_HPOUT1L_RMV_SHORT | WM8994_HPOUT1L_OUTP |
				WM8994_HPOUT1L_DLY |WM8994_HPOUT1R_RMV_SHORT |
				WM8994_HPOUT1R_OUTP | WM8994_HPOUT1R_DLY);
	wm8994_write(codec, WM8994_ANALOGUE_HP_1, val);

	/* Unmute DAC1 left */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
	val &= ~(WM8994_DAC1L_MUTE_MASK | WM8994_DAC1L_VOL_MASK);
	val |= TUNING_DAC1L_VOL; 
	wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, val);

	/* Unmute and volume ctrl RightDAC */
	val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME); 
	val &= ~(WM8994_DAC1R_MUTE_MASK | WM8994_DAC1R_VOL_MASK);
	val |= TUNING_DAC1R_VOL;
	wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);	

	/* Unmute the AF1DAC1 */	
	val = wm8994_read(codec, WM8994_AIF1_DAC1_FILTERS_1); 	
	val &= ~(WM8994_AIF1DAC1_MUTE_MASK | WM8994_AIF1DAC1_MONO_MASK);
	val |= WM8994_AIF1DAC1_UNMUTE;
	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, val);
}

void wm8994_set_playback_speaker(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;
	u16 val;


	DEBUG_LOG("");

	/* Disable end point for preventing pop up noise. */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
	val &= ~(WM8994_SPKOUTL_ENA_MASK);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_3);
	val &= ~(WM8994_MIXOUTLVOL_ENA_MASK | WM8994_MIXOUTRVOL_ENA_MASK | 
			WM8994_MIXOUTL_ENA_MASK | WM8994_MIXOUTR_ENA_MASK | 
			WM8994_SPKRVOL_ENA_MASK | WM8994_SPKLVOL_ENA_MASK);
	val |= WM8994_SPKLVOL_ENA;
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, val);

	/* Speaker Volume Control */
	if (!wm8994->testmode_config_flag) {
		/* Unmute the SPKMIXVOLUME */
		val = wm8994_read(codec, WM8994_SPKMIXL_ATTENUATION);
		val &= ~(WM8994_SPKMIXL_VOL_MASK);
		val |= TUNING_SPKMIXL_ATTEN;	
		wm8994_write(codec, WM8994_SPKMIXL_ATTENUATION, val);

		val = wm8994_read(codec,WM8994_SPKMIXR_ATTENUATION);
		val &= ~(WM8994_SPKMIXR_VOL_MASK);
		wm8994_write(codec, WM8994_SPKMIXR_ATTENUATION, val);

		val = wm8994_read(codec, WM8994_SPEAKER_VOLUME_LEFT);
		val &= ~(WM8994_SPKOUTL_MUTE_N_MASK | WM8994_SPKOUTL_VOL_MASK);
		if (wm8994->output_source == RING_TONE)
			val |= (WM8994_SPKOUT_VU | WM8994_SPKOUTL_MUTE_N | 
							TUNING_RING_SPKL_VOL);
		else
			val |= (WM8994_SPKOUT_VU | WM8994_SPKOUTL_MUTE_N | 
							TUNING_MP3_SPKL_VOL);
		wm8994_write(codec, WM8994_SPEAKER_VOLUME_LEFT, val);

		val = wm8994_read(codec, WM8994_SPEAKER_VOLUME_RIGHT);
		val &= ~(WM8994_SPKOUTR_MUTE_N_MASK | WM8994_SPKOUTR_VOL_MASK);
		wm8994_write(codec, WM8994_SPEAKER_VOLUME_RIGHT, val);

		val = wm8994_read(codec, WM8994_CLASSD);
		val &= ~(WM8994_SPKOUTL_BOOST_MASK);
		if (wm8994->output_source == RING_TONE)
			val |= TUNING_RING_CLASSD_VOL << 
						WM8994_SPKOUTL_BOOST_SHIFT;
		else
			val |= TUNING_MP3_CLASSD_VOL << 
						WM8994_SPKOUTL_BOOST_SHIFT;
		wm8994_write(codec, WM8994_CLASSD, val);

		/* Unmute DAC1 left */
		val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
		val &= ~(WM8994_DAC1L_MUTE_MASK | WM8994_DAC1L_VOL_MASK);
		val |= TUNING_DAC1L_VOL; 
		wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, val);

		/* Unmute and volume ctrl RightDAC */
		val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME); 
		val &= ~(WM8994_DAC1R_MUTE_MASK | WM8994_DAC1R_VOL_MASK);
		val |= TUNING_DAC1R_VOL;
		wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);

		val = wm8994_read(codec, WM8994_AIF1_DAC1_LEFT_VOLUME);
		val &= ~(WM8994_AIF1DAC1L_VOL_MASK);
		val |= (WM8994_AIF1DAC1_VU | TUNING_DAC1L_VOL);
		wm8994_write(codec, WM8994_AIF1_DAC1_LEFT_VOLUME, val);

		val = wm8994_read(codec, WM8994_AIF1_DAC1_RIGHT_VOLUME);
		val &= ~(WM8994_AIF1DAC1R_VOL_MASK);
		val |= (WM8994_AIF1DAC1_VU | TUNING_DAC1R_VOL);
		wm8994_write(codec, WM8994_AIF1_DAC1_RIGHT_VOLUME, val);
	}

	val = wm8994_read(codec, WM8994_SPKOUT_MIXERS);
	val &= ~(WM8994_SPKMIXL_TO_SPKOUTL_MASK | 
					WM8994_SPKMIXR_TO_SPKOUTL_MASK | 
					WM8994_SPKMIXR_TO_SPKOUTR_MASK);
	val |= WM8994_SPKMIXL_TO_SPKOUTL;
	wm8994_write(codec, WM8994_SPKOUT_MIXERS, val);

	/* Unmute the DAC path */
	val = wm8994_read(codec, WM8994_SPEAKER_MIXER);
	val &= ~(WM8994_DAC1L_TO_SPKMIXL_MASK);
	val |= WM8994_DAC1L_TO_SPKMIXL;
	wm8994_write(codec, WM8994_SPEAKER_MIXER, val);

	/* Eable DAC1 Left and timeslot left */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_5);	
	val &= ~(WM8994_DAC1L_ENA_MASK | WM8994_AIF1DAC1R_ENA_MASK | 
						WM8994_AIF1DAC1L_ENA_MASK);
	val |= (WM8994_AIF1DAC1L_ENA | WM8994_AIF1DAC1R_ENA | WM8994_DAC1L_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_5, val);   

	/* Unmute */
	val = wm8994_read(codec, WM8994_AIF1_DAC1_FILTERS_1);
	val &= ~(WM8994_AIF1DAC1_MUTE_MASK | WM8994_AIF1DAC1_MONO_MASK);
	val |= (WM8994_AIF1DAC1_UNMUTE | WM8994_AIF1DAC1_MONO);
	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, val);

	/* enable timeslot0 to left dac */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1L_TO_DAC1L_MASK);
	val |= WM8994_AIF1DAC1L_TO_DAC1L;
	wm8994_write(codec, WM8994_DAC1_LEFT_MIXER_ROUTING, val);

	/* Enbale bias,vmid and Left speaker */
	val = wm8994_read(codec,WM8994_POWER_MANAGEMENT_1);
	val &= ~(WM8994_BIAS_ENA_MASK | WM8994_VMID_SEL_MASK | 
			WM8994_HPOUT1L_ENA_MASK | WM8994_HPOUT1R_ENA_MASK | 
			WM8994_SPKOUTR_ENA_MASK | WM8994_SPKOUTL_ENA_MASK);
	val |= (WM8994_BIAS_ENA | WM8994_VMID_SEL_NORMAL | WM8994_SPKOUTL_ENA);  
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);
}

void wm8994_set_playback_speaker_headset(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;
	u16 val;

	u16 nReadServo4Val = 0;
	u16 ncompensationResult = 0;
	u16 nCompensationResultLow = 0;
	u16 nCompensationResultHigh = 0;
	u8  nServo4Low = 0;
	u8  nServo4High = 0;


	DEBUG_LOG("");

	/* ------------------  Common Settings ------------------ */
	/* Enable the Timeslot0 to DAC1L */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_MIXER_ROUTING); 	
	val &= ~(WM8994_AIF1DAC1L_TO_DAC1L_MASK);
	val |= WM8994_AIF1DAC1L_TO_DAC1L;
	wm8994_write(codec, WM8994_DAC1_LEFT_MIXER_ROUTING, val);

	/* Enable the Timeslot0 to DAC1R */
	val = wm8994_read(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING); 	
	val &= ~(WM8994_AIF1DAC1R_TO_DAC1R_MASK);
	val |= WM8994_AIF1DAC1R_TO_DAC1R;
	wm8994_write(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_AIF1_DAC1_LEFT_VOLUME);
	val &= ~(WM8994_AIF1DAC1L_VOL_MASK);
	val |= (WM8994_AIF1DAC1_VU | TUNING_DAC1L_VOL);
	wm8994_write(codec, WM8994_AIF1_DAC1_LEFT_VOLUME, val);

	val = wm8994_read(codec, WM8994_AIF1_DAC1_RIGHT_VOLUME);
	val &= ~(WM8994_AIF1DAC1R_VOL_MASK);
	val |= (WM8994_AIF1DAC1_VU |TUNING_DAC1R_VOL);
	wm8994_write(codec, WM8994_AIF1_DAC1_RIGHT_VOLUME, val);

	/*------------------  Speaker Path Settings ------------------ */
	/* Speaker Volume Control */
	if (!wm8994->testmode_config_flag) {
		val = wm8994_read(codec, WM8994_SPEAKER_VOLUME_LEFT);
		val &= ~(WM8994_SPKOUTL_MUTE_N_MASK | WM8994_SPKOUTL_VOL_MASK);
		if (wm8994->output_source == RING_TONE)
			val |= (WM8994_SPKOUT_VU | WM8994_SPKOUTL_MUTE_N | 
							TUNING_RING_SPKL_VOL);
		else
			val |= (WM8994_SPKOUT_VU | WM8994_SPKOUTL_MUTE_N | 
							TUNING_MP3_SPKL_VOL);
		wm8994_write(codec, WM8994_SPEAKER_VOLUME_LEFT, val);

		val = wm8994_read(codec, WM8994_SPEAKER_VOLUME_RIGHT);
		val &= ~(WM8994_SPKOUTR_MUTE_N_MASK | WM8994_SPKOUTR_VOL_MASK);
		wm8994_write(codec, WM8994_SPEAKER_VOLUME_RIGHT, val);

		val = wm8994_read(codec, WM8994_CLASSD);
		val &= ~(WM8994_SPKOUTL_BOOST_MASK);
		if (wm8994->output_source == RING_TONE)
			val |= TUNING_RING_CLASSD_VOL << 
						WM8994_SPKOUTL_BOOST_SHIFT;
		else
			val |= TUNING_MP3_CLASSD_VOL << 
						WM8994_SPKOUTL_BOOST_SHIFT;
		wm8994_write(codec, WM8994_CLASSD, val);
	}

	val = wm8994_read(codec, WM8994_SPKOUT_MIXERS);
	val &= ~(WM8994_SPKMIXL_TO_SPKOUTL_MASK | 
					WM8994_SPKMIXR_TO_SPKOUTL_MASK | 
					WM8994_SPKMIXR_TO_SPKOUTR_MASK);
	val |= WM8994_SPKMIXL_TO_SPKOUTL;
	wm8994_write(codec, WM8994_SPKOUT_MIXERS, val);

	/* Unmute the DAC path */
	val = wm8994_read(codec, WM8994_SPEAKER_MIXER);
	val &= ~(WM8994_DAC1L_TO_SPKMIXL_MASK);
	val |= WM8994_DAC1L_TO_SPKMIXL ;
	wm8994_write(codec, WM8994_SPEAKER_MIXER, val);

	/* ------------------  Ear Path Settings ------------------ */
	/* Configuring the Digital Paths */
	val = wm8994_read(codec, 0x102);
	val &= ~(0x0003);
	val = 0x0003;
	wm8994_write(codec, 0x102, val);

	val = wm8994_read(codec, 0x56); 	
	val &= ~(0x0003);
	val = 0x0003;
	wm8994_write(codec, 0x56, val);

	val = wm8994_read(codec, 0x102); 	
	val &= ~(0x0000);
	val = 0x0000;
	wm8994_write(codec, 0x102, val);

	val = wm8994_read(codec, WM8994_CLASS_W_1); 	
	val &= ~(0x0005);
	val = 0x0005;
	wm8994_write(codec, WM8994_CLASS_W_1, val);

	if (!wm8994->testmode_config_flag) {
		val = wm8994_read(codec, WM8994_LEFT_OUTPUT_VOLUME);
		val &= ~(WM8994_HPOUT1L_MUTE_N_MASK | WM8994_HPOUT1L_VOL_MASK);
		if (wm8994->output_source == RING_TONE)
			val |= (WM8994_HPOUT1_VU | WM8994_HPOUT1L_MUTE_N | 
						TUNING_RING_DUAL_OUTPUTL_VOL);
		else
			val |= (WM8994_HPOUT1_VU | WM8994_HPOUT1L_MUTE_N | 
						TUNING_MP3_DUAL_OUTPUTL_VOL);
		wm8994_write(codec, WM8994_LEFT_OUTPUT_VOLUME, val);

		val = wm8994_read(codec, WM8994_RIGHT_OUTPUT_VOLUME);
		val &= ~(WM8994_HPOUT1R_MUTE_N_MASK | WM8994_HPOUT1R_VOL_MASK);
		if (wm8994->output_source == RING_TONE)
			val |= (WM8994_HPOUT1_VU | WM8994_HPOUT1R_MUTE_N | 
						TUNING_RING_DUAL_OUTPUTR_VOL);
		else
			val |= (WM8994_HPOUT1_VU | WM8994_HPOUT1L_MUTE_N | 
						TUNING_MP3_DUAL_OUTPUTR_VOL);
		wm8994_write(codec, WM8994_RIGHT_OUTPUT_VOLUME, val);
	}

	/* DC Servo Series Count */
	val = 0x03E0;
	wm8994_write(codec, WM8994_DC_SERVO_2, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
	val &= ~(WM8994_BIAS_ENA_MASK | WM8994_VMID_SEL_MASK |
			WM8994_HPOUT1L_ENA_MASK |WM8994_HPOUT1R_ENA_MASK |
			WM8994_SPKOUTL_ENA_MASK);
	val |= (WM8994_BIAS_ENA | WM8994_VMID_SEL_NORMAL |
				WM8994_HPOUT1R_ENA | WM8994_HPOUT1L_ENA | 
				WM8994_SPKOUTL_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	val = (WM8994_HPOUT1L_DLY | WM8994_HPOUT1R_DLY);
	wm8994_write(codec, WM8994_ANALOGUE_HP_1, val);

	/* Enable Charge Pump	*/
	val = wm8994_read(codec, WM8994_CHARGE_PUMP_1);
	val &= ~WM8994_CP_ENA_MASK ;
	val |= WM8994_CP_ENA | WM8994_CP_ENA_DEFAULT;
	wm8994_write(codec, WM8994_CHARGE_PUMP_1, val);

	msleep(5);

	/* Enable DAC1 and DAC2 and the Timeslot0 for AIF1 */
	val = wm8994_read(codec,WM8994_POWER_MANAGEMENT_5 );	
	val &= ~(WM8994_DAC1R_ENA_MASK | WM8994_DAC1L_ENA_MASK |
			WM8994_AIF1DAC1R_ENA_MASK | WM8994_AIF1DAC1L_ENA_MASK);
	val |= (WM8994_AIF1DAC1L_ENA | WM8994_AIF1DAC1R_ENA  | 
					WM8994_DAC1L_ENA | WM8994_DAC1R_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_5, val);   

	/* Enbale DAC1L to HPOUT1L path */
	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
	val &=  ~(WM8994_DAC1L_TO_HPOUT1L_MASK | WM8994_DAC1L_TO_MIXOUTL_MASK);
	val |=  WM8994_DAC1L_TO_MIXOUTL;
	wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val);

	/* Enbale DAC1R to HPOUT1R path */
	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
	val &= ~(WM8994_DAC1R_TO_HPOUT1R_MASK | WM8994_DAC1R_TO_MIXOUTR_MASK);
	val |= WM8994_DAC1R_TO_MIXOUTR;
	wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);

	/* Enbale bias,vmid, hp left and right and Left speaker */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_3);
	val &= ~(WM8994_MIXOUTLVOL_ENA_MASK | WM8994_MIXOUTRVOL_ENA_MASK | 
			WM8994_MIXOUTL_ENA_MASK | WM8994_MIXOUTR_ENA_MASK | 
			WM8994_SPKLVOL_ENA_MASK);
	val |= (WM8994_MIXOUTL_ENA | WM8994_MIXOUTR_ENA | WM8994_SPKLVOL_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3 ,val);

	/* DC Servo */
	val = (WM8994_DCS_TRIG_SERIES_1 | WM8994_DCS_TRIG_SERIES_0 | 
				WM8994_DCS_ENA_CHAN_1 | WM8994_DCS_ENA_CHAN_0);
	wm8994_write(codec, WM8994_DC_SERVO_1, 0x0303);

	msleep(160);

	nReadServo4Val = wm8994_read(codec, WM8994_DC_SERVO_4);
	nServo4Low = (signed char)(nReadServo4Val & 0xff);
	nServo4High = (signed char)((nReadServo4Val>>8) & 0xff);

	nCompensationResultLow = ((signed short)nServo4Low -5)&0x00ff;
	nCompensationResultHigh = ((signed short)(nServo4High -5)<<8)&0xff00;
	ncompensationResult = nCompensationResultLow | nCompensationResultHigh;
	wm8994_write(codec, WM8994_DC_SERVO_4, ncompensationResult);

	val = (WM8994_DCS_TRIG_DAC_WR_1 | WM8994_DCS_TRIG_DAC_WR_0 | 
				WM8994_DCS_ENA_CHAN_1 | WM8994_DCS_ENA_CHAN_0);
	wm8994_write(codec, WM8994_DC_SERVO_1, val);

	msleep(15);	

	/* Intermediate HP settings */
	val = wm8994_read(codec, WM8994_ANALOGUE_HP_1); 	
	val &= ~(WM8994_HPOUT1R_DLY_MASK |WM8994_HPOUT1R_OUTP_MASK |
		WM8994_HPOUT1R_RMV_SHORT_MASK | WM8994_HPOUT1L_DLY_MASK |
		WM8994_HPOUT1L_OUTP_MASK | WM8994_HPOUT1L_RMV_SHORT_MASK);
	val |= (WM8994_HPOUT1L_RMV_SHORT | WM8994_HPOUT1L_OUTP |
				WM8994_HPOUT1L_DLY | WM8994_HPOUT1R_RMV_SHORT | 
				WM8994_HPOUT1R_OUTP | WM8994_HPOUT1R_DLY);
	wm8994_write(codec, WM8994_ANALOGUE_HP_1, val);

	if (!wm8994->testmode_config_flag) {
		/* Unmute DAC1 left */
		val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
		val &= ~(WM8994_DAC1L_MUTE_MASK | WM8994_DAC1L_VOL_MASK);
		val |= TUNING_DAC1L_VOL; 
		wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME ,val);

		/* Unmute and volume ctrl RightDAC */
		val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME ); 
		val &= ~(WM8994_DAC1R_MUTE_MASK | WM8994_DAC1R_VOL_MASK);
		val |= TUNING_DAC1R_VOL;
		wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);
	}

	/* ------------------  Common Settings ------------------ */
	/* Unmute the AIF1DAC1 */
	val = wm8994_read(codec, WM8994_AIF1_DAC1_FILTERS_1); 	
	val &= ~(WM8994_AIF1DAC1_MUTE_MASK | WM8994_AIF1DAC1_MONO_MASK);
	val |= (WM8994_AIF1DAC1_UNMUTE | WM8994_AIF1DAC1_MONO);
	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, val);

	if (!wm8994->testmode_config_flag) {
		/* Unmute the SPKMIXVOLUME */
		val = wm8994_read(codec, WM8994_SPKMIXL_ATTENUATION);
		val &= ~(WM8994_SPKMIXL_VOL_MASK);
		val |= TUNING_SPKMIXL_ATTEN;	
		wm8994_write(codec, WM8994_SPKMIXL_ATTENUATION, val);

		val = wm8994_read(codec, WM8994_SPKMIXR_ATTENUATION);
		val &= ~(WM8994_SPKMIXR_VOL_MASK);
		wm8994_write(codec, WM8994_SPKMIXR_ATTENUATION, val);
	}
}

void wm8994_set_playback_bluetooth(struct snd_soc_codec *codec)
{
	u16 val;


	DEBUG_LOG("");

	wm8994_set_bluetooth_common_setting(codec);

	/* R1(1h) - 0x0003 -normal vmid */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
	val &= ~(WM8994_BIAS_ENA_MASK | WM8994_VMID_SEL_MASK);
	val |= (WM8994_BIAS_ENA | WM8994_VMID_SEL_NORMAL);	
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);
	
	/* Digital Path Enables and Unmutes */	
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_2, 0x0000);

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, 0x0000);

 	/* AIF2ADC(Left/Right) Output */
 	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_4);
	val &= ~(WM8994_AIF2ADCL_ENA_MASK | WM8994_AIF2ADCR_ENA_MASK);
	val |= (WM8994_AIF2ADCL_ENA | WM8994_AIF2ADCR_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_4 , val);

	/* AIF1DAC1(Left/Right) Input */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_5);
		val &= ~(WM8994_AIF2DACL_ENA_MASK | WM8994_AIF2DACR_ENA_MASK | 
			WM8994_AIF1DAC1L_ENA_MASK | WM8994_AIF1DAC1R_ENA_MASK |
			WM8994_DAC1L_ENA_MASK | WM8994_DAC1R_ENA_MASK);
	val |= (WM8994_AIF2DACL_ENA | WM8994_AIF2DACR_ENA | 
				WM8994_AIF1DAC1L_ENA | WM8994_AIF1DAC1R_ENA |
				WM8994_DAC1L_ENA | WM8994_DAC1R_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_5, val);
	
	/* AIF3_ADCDAT_SRC(AIF2ADCDAT2) */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_6);
	val &= ~(WM8994_AIF3_ADCDAT_SRC_MASK);
	val |= (0x0001 << WM8994_AIF3_ADCDAT_SRC_SHIFT);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_6, val);

	/* Un-Mute */
	/* AIF1DAC1_MUTE(Un-mute) */
	val = wm8994_read(codec, WM8994_AIF1_DAC1_FILTERS_1);
	val &= ~(WM8994_AIF1DAC1_MUTE_MASK | WM8994_AIF1DAC1_MONO_MASK);
	val |= (WM8994_AIF1DAC1_UNMUTE);
	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, val);  

	/* Mixer Routing */
	/* AIF2DACL_TO_DAC2L */
	val = wm8994_read(codec, WM8994_DAC2_LEFT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1L_TO_DAC2L_MASK);
	val |= (WM8994_AIF1DAC1L_TO_DAC2L);
	wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING, val);
	
	/* AIF2DACR_TO_DAC2R */
	val = wm8994_read(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1R_TO_DAC2R_MASK);
	val |= (WM8994_AIF1DAC1R_TO_DAC2R);
	wm8994_write(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING, val);

	/* Volume */
	wm8994_write(codec, WM8994_DAC2_LEFT_VOLUME, 0x01C0);
	wm8994_write(codec, WM8994_DAC2_RIGHT_VOLUME, 0x01C0); 

	wm8994_write(codec, WM8994_AIF2_CLOCKING_1, 0x0019);

	wm8994_write(codec, WM8994_OVERSAMPLING, 0X0000);

	/* GPIO Configuration */		
	wm8994_write(codec, WM8994_GPIO_8, WM8994_GP8_DIR | WM8994_GP8_DB);
	wm8994_write(codec, WM8994_GPIO_9, WM8994_GP9_DB);
	wm8994_write(codec, WM8994_GPIO_10, WM8994_GP10_DB);
	wm8994_write(codec, WM8994_GPIO_11, WM8994_GP11_DB); 
}

void wm8994_set_playback_extra_dock_speaker(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;
	u16 val;


	DEBUG_LOG("");

	/* OUTPUT mute */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_3);
	val &= ~(WM8994_LINEOUT2N_ENA_MASK | WM8994_LINEOUT2P_ENA_MASK);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, val);

	if (!wm8994->testmode_config_flag) {
		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_5);
		val &= ~(WM8994_DACL_MIXOUTL_VOL_MASK);
		val |= (TUNING_EXTRA_DOCK_SPK_OUTMIX5_VOL << 
						WM8994_DACL_MIXOUTL_VOL_SHIFT);
		wm8994_write(codec, WM8994_OUTPUT_MIXER_5, val);

		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_6);
		val &= ~(WM8994_DACR_MIXOUTR_VOL_MASK);
		val |= (TUNING_EXTRA_DOCK_SPK_OUTMIX6_VOL << 
						WM8994_DACR_MIXOUTR_VOL_SHIFT);
		wm8994_write(codec, WM8994_OUTPUT_MIXER_6, val);

		val = wm8994_read(codec, WM8994_LEFT_OPGA_VOLUME);
		val &= ~(WM8994_MIXOUTL_MUTE_N_MASK | WM8994_MIXOUTL_VOL_MASK);
		val |= (WM8994_MIXOUT_VU | WM8994_MIXOUTL_MUTE_N | 
					TUNING_MP3_EXTRA_DOCK_SPK_OPGAL_VOL);
		wm8994_write(codec, WM8994_LEFT_OPGA_VOLUME, val);

		val = wm8994_read(codec, WM8994_RIGHT_OPGA_VOLUME);
		val &= ~(WM8994_MIXOUTR_MUTE_N_MASK | WM8994_MIXOUTR_VOL_MASK);
		val |= (WM8994_MIXOUT_VU | WM8994_MIXOUTR_MUTE_N | 
					TUNING_MP3_EXTRA_DOCK_SPK_OPGAR_VOL);
		wm8994_write(codec, WM8994_RIGHT_OPGA_VOLUME, val);

		val = wm8994_read(codec, WM8994_LINE_OUTPUTS_VOLUME);
		val &= ~(WM8994_LINEOUT2_VOL_MASK);
		val |= (TUNING_MP3_EXTRA_DOCK_SPK_VOL << 
						WM8994_LINEOUT2_VOL_SHIFT);
		wm8994_write(codec, WM8994_LINE_OUTPUTS_VOLUME, val);

		/* Unmute DAC1 left */
		val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
		val &= ~(WM8994_DAC1L_MUTE_MASK | WM8994_DAC1L_VOL_MASK);
		val |= TUNING_DAC1L_VOL; 
		wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, val);

		/* Unmute and volume ctrl RightDAC */
		val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME); 
		val &= ~(WM8994_DAC1R_MUTE_MASK | WM8994_DAC1R_VOL_MASK);
		val |= TUNING_DAC1R_VOL;
		wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);

		val = wm8994_read(codec, WM8994_AIF1_DAC1_LEFT_VOLUME);
		val &= ~(WM8994_AIF1DAC1L_VOL_MASK);
		val |= (WM8994_AIF1DAC1_VU | TUNING_DAC1L_VOL);
		wm8994_write(codec, WM8994_AIF1_DAC1_LEFT_VOLUME, val);

		val = wm8994_read(codec, WM8994_AIF1_DAC1_RIGHT_VOLUME);
		val &= ~(WM8994_AIF1DAC1R_VOL_MASK);
		val |= (WM8994_AIF1DAC1_VU |TUNING_DAC1R_VOL);
		wm8994_write(codec, WM8994_AIF1_DAC1_RIGHT_VOLUME, val);
	}

	/* For X-talk of VPS's L/R line. It's requested by H/W team. */
	wm8994_write(codec, WM8994_ADDITIONAL_CONTROL, 0x00);

	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
	val &= ~(WM8994_DAC1L_TO_MIXOUTL_MASK);
	val |= (WM8994_DAC1L_TO_MIXOUTL);
	wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val);

	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
	val &= ~(WM8994_DAC1R_TO_MIXOUTR_MASK);
	val |= (WM8994_DAC1R_TO_MIXOUTR);
	wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);

	val = wm8994_read(codec, WM8994_LINE_MIXER_2);
	val &= ~(WM8994_MIXOUTR_TO_LINEOUT2N_MASK | 
		WM8994_MIXOUTL_TO_LINEOUT2N_MASK | WM8994_LINEOUT2_MODE_MASK | 
		WM8994_MIXOUTR_TO_LINEOUT2P_MASK);
	val |= (WM8994_MIXOUTL_TO_LINEOUT2N | WM8994_LINEOUT2_MODE | 
						WM8994_MIXOUTR_TO_LINEOUT2P);
	wm8994_write(codec, WM8994_LINE_MIXER_2, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_5);
	val &= ~(WM8994_DAC1R_ENA_MASK | WM8994_DAC1L_ENA_MASK | 
			WM8994_AIF1DAC1R_ENA_MASK | WM8994_AIF1DAC1L_ENA_MASK);
	val |= (WM8994_AIF1DAC1L_ENA | WM8994_AIF1DAC1R_ENA | 
					WM8994_DAC1L_ENA | WM8994_DAC1R_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_5, val);

	val = wm8994_read(codec, WM8994_AIF1_DAC1_FILTERS_1);
	val &= ~(WM8994_AIF1DAC1_MUTE_MASK | WM8994_AIF1DAC1_MONO_MASK);
	val |= WM8994_AIF1DAC1_UNMUTE;
	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, val);

	val = wm8994_read(codec, WM8994_LINE_OUTPUTS_VOLUME);
	val &= ~(WM8994_LINEOUT2N_MUTE_MASK | WM8994_LINEOUT2P_MUTE_MASK);
	wm8994_write(codec, WM8994_LINE_OUTPUTS_VOLUME, val);

	val = wm8994_read(codec, WM8994_DAC1_LEFT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1L_TO_DAC1L_MASK);
	val |= WM8994_AIF1DAC1L_TO_DAC1L;
	wm8994_write(codec, WM8994_DAC1_LEFT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1R_TO_DAC1R_MASK);
	val |= WM8994_AIF1DAC1R_TO_DAC1R;
	wm8994_write(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_CLOCKING_1);
	val &= ~(WM8994_DSP_FS1CLK_ENA_MASK | WM8994_DSP_FSINTCLK_ENA_MASK);
	val |= (WM8994_DSP_FS1CLK_ENA | WM8994_DSP_FSINTCLK_ENA);
	wm8994_write(codec, WM8994_CLOCKING_1, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_3);
	val &= ~(WM8994_LINEOUT2N_ENA_MASK | WM8994_LINEOUT1P_ENA_MASK | 
		WM8994_MIXOUTLVOL_ENA_MASK | WM8994_MIXOUTRVOL_ENA_MASK | 
		WM8994_MIXOUTL_ENA_MASK | WM8994_MIXOUTR_ENA_MASK);
	val |= (WM8994_LINEOUT2N_ENA | WM8994_LINEOUT2P_ENA | 
				WM8994_MIXOUTL_ENA | WM8994_MIXOUTR_ENA | 
				WM8994_MIXOUTRVOL_ENA | WM8994_MIXOUTLVOL_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
	val &= ~(WM8994_BIAS_ENA_MASK | WM8994_VMID_SEL_MASK | 
							WM8994_HPOUT2_ENA_MASK);
	val |= (WM8994_BIAS_ENA | WM8994_VMID_SEL_NORMAL);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	wm8994_write(codec, WM8994_ANTIPOP_1, WM8994_LINEOUT_VMID_BUF_ENA);

	wm8994_write(codec, WM8994_ANTIPOP_2, WM8994_VMID_BUF_ENA);

	/* Switching USB Path for Dock & Car Cradle */
	fsa9480_manual_switching(SWITCH_PORT_AUDIO);
}

void wm8994_set_voicecall_common(struct snd_soc_codec *codec)
{
	int val;


	/* Reduced tick noise when changing call path. */
	val = wm8994_read(codec, WM8994_AIF2_DAC_FILTERS_1);
	val &= ~(WM8994_AIF2DAC_MUTE_MASK);
	val |= (WM8994_AIF2DAC_MUTE);
	wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_1, val);
	
	/* GPIO Configuration */
	wm8994_write(codec, WM8994_GPIO_1, 0xA101);
	wm8994_write(codec, WM8994_GPIO_2, 0x8100);
	wm8994_write(codec, WM8994_GPIO_3, 0x0100);
	wm8994_write(codec, WM8994_GPIO_4, 0x0100);
	wm8994_write(codec, WM8994_GPIO_5, 0x8100);
	wm8994_write(codec, WM8994_GPIO_6, 0xA101);
	wm8994_write(codec, WM8994_GPIO_7, 0x0100);
	wm8994_write(codec, WM8994_GPIO_8, 0xA101);
	wm8994_write(codec, WM8994_GPIO_9, 0xA101);
	wm8994_write(codec, WM8994_GPIO_10, 0xA101);
	wm8994_write(codec, WM8994_GPIO_11, 0xA101);

	wm8994_write(codec, WM8994_FLL2_CONTROL_2, 0x2F00);
	wm8994_write(codec, WM8994_FLL2_CONTROL_3, 0x3126);
	wm8994_write(codec, WM8994_FLL2_CONTROL_4, 0x0100);
	wm8994_write(codec, WM8994_FLL2_CONTROL_5, 0x0C88);
	wm8994_write(codec, WM8994_FLL2_CONTROL_1,
				WM8994_FLL2_FRACN_ENA | WM8994_FLL2_ENA);

	val = wm8994_read(codec, WM8994_AIF2_CLOCKING_1);
	if (!(val & WM8994_AIF2CLK_ENA))
		wm8994_write(codec, WM8994_AIF2_CLOCKING_1, 0x0018);

	wm8994_write(codec, WM8994_AIF2_RATE, 0x3 << WM8994_AIF2CLK_RATE_SHIFT);

	/* AIF2 Interface - PCM Stereo mode */
	if (HWREV >= 0x0A) {
		/* FMT : I2S */
		wm8994_write(codec, WM8994_AIF2_CONTROL_1, 0x4010);	
		DEBUG_LOG("I2S format");				
	} else {
		/* FMT : Left Justified */
		wm8994_write(codec, WM8994_AIF2_CONTROL_1, 0x4008);
		DEBUG_LOG("Left Justified format");			
	}

	/*HPF Enable */
	wm8994_write(codec, WM8994_AIF2_ADC_FILTERS, 0x3800);

	wm8994_write(codec, WM8994_AIF2_BCLK, 0x70);
	wm8994_write(codec, WM8994_AIF2_CONTROL_2, 0x0000);
	wm8994_write(codec, WM8994_AIF2_MASTER_SLAVE, WM8994_AIF2_MSTR |
				WM8994_AIF2_CLK_FRC | WM8994_AIF2_LRCLK_FRC);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_5);
	val &= ~(WM8994_AIF2DACL_ENA_MASK | WM8994_AIF2DACR_ENA_MASK |
			WM8994_AIF1DAC1L_ENA_MASK | WM8994_AIF1DAC1R_ENA_MASK |
			WM8994_DAC1L_ENA_MASK | WM8994_DAC1R_ENA_MASK);
	val |= (WM8994_AIF2DACL_ENA | WM8994_AIF2DACR_ENA |
				WM8994_AIF1DAC1L_ENA | WM8994_AIF1DAC1R_ENA |
				WM8994_DAC1L_ENA | WM8994_DAC1R_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_5, val);

	/* Clocking */
	val = wm8994_read(codec, WM8994_CLOCKING_1);
	val |= (WM8994_DSP_FS2CLK_ENA);
	wm8994_write(codec, WM8994_CLOCKING_1, val);

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_6, 0x0);

	/* AIF1 & AIF2 Output is connected to DAC1 */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1L_TO_DAC1L_MASK |
						WM8994_AIF2DACL_TO_DAC1L_MASK);
	val |= (WM8994_AIF1DAC1L_TO_DAC1L | WM8994_AIF2DACL_TO_DAC1L);
	wm8994_write(codec, WM8994_DAC1_LEFT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1R_TO_DAC1R_MASK |
						WM8994_AIF2DACR_TO_DAC1R_MASK);
	val |= (WM8994_AIF1DAC1R_TO_DAC1R | WM8994_AIF2DACR_TO_DAC1R);
	wm8994_write(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING, val);
}

void wm8994_set_voicecall_receiver(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;
	int val;


	DEBUG_LOG("");

	/* Reduced tick noise when changing call path(mute analogue output) */
	wm8994_write(codec,WM8994_POWER_MANAGEMENT_1, 0x0003);  
	msleep(60);

	audio_ctrl_mic_bias_gpio(wm8994->pdata, 1);

	/* Anti Pop2 */
	wm8994_write(codec, 0x0039, 0x0068);

	/* To remove the robotic sound */
	wm8994_write(codec, 0x0102, 0x0003);   
	wm8994_write(codec, 0x0817, 0x0000);
	wm8994_write(codec, 0x0102, 0x0003);

	wm8994_set_voicecall_common(codec);

	if (((NC_ON == wm8994->ncbypass_active)
#ifdef FEATURE_FACTORY_LOOPBACK		
			|| (loopback_mode == PBA_LOOPBACK_MODE_ON)
#endif			
		)
#ifdef FEATURE_FACTORY_LOOPBACK		
		&& (loopback_mode != SIMPLETEST_LOOPBACK_MODE_ON)
#endif		
		&& (HWREV >= 0x0A)
	) {
		/* FMT : I2S */
		wm8994_write(codec, WM8994_AIF2_CONTROL_1, 0x4010);

		DEBUG_LOG("NC H/W Bypass I2S");

		/* Analogue Input Configuration */
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_2,
					WM8994_TSHUT_ENA | WM8994_TSHUT_OPDIS |
					WM8994_MIXINL_ENA | WM8994_IN1L_ENA);

		wm8994_write(codec, WM8994_INPUT_MIXER_2, WM8994_IN1LP_TO_IN1L |
							WM8994_IN1LN_TO_IN1L);


		/* Tx -> AIF2 Path */
		wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING, 
							WM8994_ADC1_TO_DAC2L);	
		wm8994_write(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING, 
							WM8994_ADC1_TO_DAC2R);

		/* Digital Path Enables and Unmutes*/	
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, 
					WM8994_AIF2ADCL_ENA | WM8994_ADCL_ENA | 
					WM8994_AIF2ADCR_ENA);
	} else {
		/* FMT : Left Justified */
		wm8994_write(codec, WM8994_AIF2_CONTROL_1, 0x4008);

		DEBUG_LOG("NC H/W Bypass Left Justified");

		/* Analogue Input Configuration */
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_2, 
					WM8994_TSHUT_ENA | WM8994_TSHUT_OPDIS |
					WM8994_MIXINL_ENA | WM8994_MIXINR_ENA |
					WM8994_IN1L_ENA | WM8994_IN2R_ENA);

		wm8994_write(codec, WM8994_INPUT_MIXER_2, WM8994_IN1LP_TO_IN1L |
				WM8994_IN1LN_TO_IN1L | WM8994_IN2RP_TO_IN2R |
				WM8994_IN2RN_TO_IN2R);

		/* Tx -> AIF2 Path */
		wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING, 
							WM8994_ADC1_TO_DAC2L);	
		wm8994_write(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING, 
							WM8994_ADC2_TO_DAC2R);

		if (!wm8994->testmode_config_flag) {
			/* Volume Control - Input */
			val = wm8994_read(codec, WM8994_INPUT_MIXER_4);
			val&= ~(WM8994_IN2R_TO_MIXINR_MASK | 
						WM8994_IN2R_MIXINR_VOL_MASK |
						WM8994_MIXOUTR_MIXINR_VOL_MASK);
			val |= (WM8994_IN2R_TO_MIXINR | 0x80);
			wm8994_write(codec, WM8994_INPUT_MIXER_4, val); 

			val = wm8994_read(codec, 
					WM8994_RIGHT_LINE_INPUT_3_4_VOLUME);	
			val &= ~(WM8994_IN2R_MUTE_MASK | WM8994_IN2R_VOL_MASK);
			val |= (WM8994_IN2R_VU | TUNING_CALL_RCV_INPUTMIX_VOL);
			wm8994_write(codec, 
				WM8994_RIGHT_LINE_INPUT_3_4_VOLUME , val);
		}

		/* Digital Path Enables and Unmutes*/	
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, 
					WM8994_AIF2ADCL_ENA | WM8994_ADCL_ENA | 
					WM8994_AIF2ADCR_ENA | WM8994_ADCR_ENA);
	}

	/* Turn off charge pump */
	wm8994_write(codec, WM8994_CHARGE_PUMP_1, WM8994_CP_ENA_DEFAULT);

	/* Sidetone */
	wm8994_write(codec, 0x0621, 0x01C0);

	wm8994_write(codec, WM8994_DAC2_MIXER_VOLUMES, 0x018C);	
	wm8994_write(codec, WM8994_DAC2_LEFT_VOLUME, 0x01C0 );	
	wm8994_write(codec, WM8994_DAC2_RIGHT_VOLUME, 0x01C0);	

	wm8994_write(codec, WM8994_AIF2_CLOCKING_1, 0x0019);

	if (!wm8994->testmode_config_flag) {	
		/* Unmute IN1L PGA, update volume */
		val = wm8994_read(codec, WM8994_LEFT_LINE_INPUT_1_2_VOLUME);	
		val &= ~(WM8994_IN1L_MUTE_MASK | WM8994_IN1L_VOL_MASK);
		if (((NC_ON == wm8994->ncbypass_active)
#ifdef FEATURE_FACTORY_LOOPBACK			
				|| (loopback_mode == PBA_LOOPBACK_MODE_ON)
#endif				
			)
#ifdef FEATURE_FACTORY_LOOPBACK			
			&& (loopback_mode != SIMPLETEST_LOOPBACK_MODE_ON)
#endif			
		) {
			val |= (WM8994_IN1L_VU |0x0B);
		} else {
			val |= (WM8994_IN1L_VU |0x0A);
		}
		wm8994_write(codec, WM8994_LEFT_LINE_INPUT_1_2_VOLUME, val);
		/* Sidetone */
		wm8994_write(codec, 0x0621, 0x01C0);

		/* Unmute the PGA */
		val = wm8994_read(codec, WM8994_INPUT_MIXER_3);
		val&= ~(WM8994_IN1L_TO_MIXINL_MASK | 
						WM8994_IN1L_MIXINL_VOL_MASK |
						WM8994_MIXOUTL_MIXINL_VOL_MASK);
		val |= (WM8994_IN1L_TO_MIXINL |WM8994_IN1L_MIXINL_VOL);
		wm8994_write(codec, WM8994_INPUT_MIXER_3, val); 

		/* Volume Control - Output */
		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_5);
		val &= ~(WM8994_DACL_MIXOUTL_VOL_MASK);
		val |= TUNING_RCV_OUTMIX5_VOL << WM8994_DACL_MIXOUTL_VOL_SHIFT;
		wm8994_write(codec,WM8994_OUTPUT_MIXER_5, val);

		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_6);
		val &= ~(WM8994_DACR_MIXOUTR_VOL_MASK);
		val |= TUNING_RCV_OUTMIX6_VOL << WM8994_DACR_MIXOUTR_VOL_SHIFT;
		wm8994_write(codec,WM8994_OUTPUT_MIXER_6, val);

		val = wm8994_read(codec, WM8994_LEFT_OPGA_VOLUME);
		val &= ~(WM8994_MIXOUTL_MUTE_N_MASK | WM8994_MIXOUTL_VOL_MASK);
		if (HWREV >= 0x0A) {
			if (((NC_OFF == wm8994->ncbypass_active)
#ifdef FEATURE_FACTORY_LOOPBACK				
					|| (loopback_mode == 
						SIMPLETEST_LOOPBACK_MODE_ON)
#endif					
				)
#ifdef FEATURE_FACTORY_LOOPBACK				
				&& (loopback_mode != PBA_LOOPBACK_MODE_ON)
#endif				
			) {
				val |= 0x0179;
			} else {
				val |= (WM8994_MIXOUT_VU | 
							WM8994_MIXOUTL_MUTE_N |
							TUNING_RCV_OPGAL_VOL);
			}
		} else {
			val |= (WM8994_MIXOUT_VU | WM8994_MIXOUTL_MUTE_N |
							TUNING_RCV_OPGAL_VOL);
		}
		wm8994_write(codec,WM8994_LEFT_OPGA_VOLUME, val);

		val = wm8994_read(codec, WM8994_RIGHT_OPGA_VOLUME);
		val &= ~(WM8994_MIXOUTR_MUTE_N_MASK | WM8994_MIXOUTR_VOL_MASK);
		if (HWREV >= 0x0A) {
			if(((NC_OFF == wm8994->ncbypass_active)
#ifdef FEATURE_FACTORY_LOOPBACK				
					|| (loopback_mode == 
						SIMPLETEST_LOOPBACK_MODE_ON)
#endif					
				)
#ifdef FEATURE_FACTORY_LOOPBACK				
				&& (loopback_mode != PBA_LOOPBACK_MODE_ON)
#endif				
			) {
				val |= 0x0179;
			} else {
				val |= (WM8994_MIXOUT_VU | 
							WM8994_MIXOUTR_MUTE_N |
							TUNING_RCV_OPGAR_VOL);
			}
		} else {
			val |= (WM8994_MIXOUT_VU | WM8994_MIXOUTR_MUTE_N |
							TUNING_RCV_OPGAR_VOL);
		}
		wm8994_write(codec, WM8994_RIGHT_OPGA_VOLUME, val);

		val = wm8994_read(codec, WM8994_HPOUT2_VOLUME);
		val &= ~(WM8994_HPOUT2_MUTE_MASK | WM8994_HPOUT2_VOL_MASK);
		val |= TUNING_HPOUT2_VOL << WM8994_HPOUT2_VOL_SHIFT;
		wm8994_write(codec, WM8994_HPOUT2_VOLUME, val);

		/* Unmute DAC1 left */
		val = wm8994_read(codec,WM8994_DAC1_LEFT_VOLUME );
		val &= ~(WM8994_DAC1L_MUTE_MASK | WM8994_DAC1L_VOL_MASK);
		val |= TUNING_DAC1L_VOL; 
		wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME ,val);

		/* Unmute and volume ctrl RightDAC */
		val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME ); 
		val &= ~(WM8994_DAC1R_MUTE_MASK | WM8994_DAC1R_VOL_MASK);
		val |= TUNING_DAC1R_VOL;
		wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME,val);
	}

	/* Output Mixing */
	wm8994_write(codec, WM8994_OUTPUT_MIXER_1, WM8994_DAC1L_TO_MIXOUTL);
	wm8994_write(codec, WM8994_OUTPUT_MIXER_2, WM8994_DAC1R_TO_MIXOUTR);

	/* Sidetone */
	wm8994_write(codec, 0x0600, 0x0020);

	/* Analogue Output Configuration */
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, 
				WM8994_MIXOUTLVOL_ENA | WM8994_MIXOUTRVOL_ENA |
				WM8994_MIXOUTL_ENA | WM8994_MIXOUTR_ENA);
	wm8994_write(codec, WM8994_HPOUT2_MIXER, WM8994_MIXOUTLVOL_TO_HPOUT2 |
						WM8994_MIXOUTRVOL_TO_HPOUT2); 

#ifdef CONFIG_SND_SOC_A1026
	if (HWREV >= 0x0A) {
#ifdef FEATURE_FACTORY_LOOPBACK
		if (loopback_mode == SIMPLETEST_LOOPBACK_MODE_ON) {
			A1026SetBypass(0);
			A1026Wakeup();
			A1026SetFeature(BYPASSMODE);
		} else if(loopback_mode == PBA_LOOPBACK_MODE_ON) {
			A1026SetBypass(1);
			A1026Sleep();
		} else
#endif
		if (NC_ON == wm8994->ncbypass_active) {
			A1026SetBypass(1);
			A1026Sleep();
		} else {
			A1026SetBypass(0);
			A1026Wakeup();
			A1026SetFeature(CLOSETALK);
		}
	} else {
		A1026Wakeup();

#ifdef FEATURE_FACTORY_LOOPBACK
		if ((loopback_mode == PBA_LOOPBACK_MODE_ON)
			|| (loopback_mode == SIMPLETEST_LOOPBACK_MODE_ON)
		) {
			A1026SetFeature(BYPASSMODE);	
		} else
#endif
		if (NC_ON == wm8994->ncbypass_active) {
			A1026SetFeature(CT_VPOFF);
		} else {
			A1026SetFeature(CLOSETALK);	
		}
	}
#endif

	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, 0x0000);
	wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_1, 0x0000);

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, WM8994_HPOUT2_ENA | 
				WM8994_VMID_SEL_NORMAL | WM8994_BIAS_ENA);	

        if(wm8994->rec_path != MIC_OFF)
                wm8994_record_main_mic(codec);
}

void wm8994_set_voicecall_headset(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;
	int val;
	
	u16 TestReturn1 = 0;
	u16 TestReturn2 = 0;
	u16 TestLow1 = 0;
	u16 TestHigh1 = 0;
	u8 TestLow = 0;
	u8 TestHigh = 0;


	DEBUG_LOG("");

	/* Reduced tick noise when changing call path(mute analogue output) */
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, 0x0003);  
	msleep(60);

#ifdef FEATURE_TTY
	switch (tty_mode) {
	case TTY_MODE_FULL :
		DEBUG_LOG("Selected TTY_FULL");

		wm8994_set_voicecall_tty_full(codec);

#ifdef CONFIG_SND_SOC_A1026
		if (HWREV >= 0x0A) {
			A1026SetBypass(1);
			A1026Sleep();
		} else {
			A1026Wakeup();
			A1026SetFeature(BYPASSMODE);
		}
#endif
		return;
		break;

	case TTY_MODE_HCO :
		DEBUG_LOG("Selected TTY_HCO");

		wm8994_set_voicecall_tty_hco(codec);

#ifdef CONFIG_SND_SOC_A1026
		if (HWREV >= 0x0A) {
			A1026SetBypass(1);				
			A1026Sleep();
		} else {
			A1026Wakeup();
			A1026SetFeature(BYPASSMODE);
		}
#endif
		return;
		break;

	case TTY_MODE_VCO :
		DEBUG_LOG("Selected TTY_VCO");

		wm8994_set_voicecall_tty_vco(codec);
		
#ifdef CONFIG_SND_SOC_A1026
		if(HWREV >= 0x0A) {
			A1026SetBypass(1);				
			A1026Sleep();
		} else {
			A1026Wakeup();
			A1026SetFeature(BYPASSMODE);
		}
#endif
		return;
		break;

	case TTY_MODE_OFF :
	default:
		DEBUG_LOG("Selected TTY_OFF");
		break;
	}
#endif

	audio_ctrl_mic_bias_gpio(wm8994->pdata, 0);

	/* To remove the robotic sound */
	wm8994_write(codec, 0x0102, 0x0003); 
	wm8994_write(codec, 0x0817, 0x0000);
	wm8994_write(codec, 0x0102, 0x0003);

	wm8994_set_voicecall_common(codec);

	/* Digital Path Enables and Unmutes */	
	wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING, 
							WM8994_ADC2_TO_DAC2L);
	wm8994_write(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING, 
							WM8994_ADC2_TO_DAC2R);        
	wm8994_write(codec, WM8994_DAC2_MIXER_VOLUMES, 0x0180);  
	wm8994_write(codec, WM8994_SIDETONE, 0x01C0);

	/* Analogue Input Configuration */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_2);	
	val &= ~(WM8994_TSHUT_ENA_MASK |WM8994_TSHUT_OPDIS_MASK |
		WM8994_MIXINR_ENA_MASK |WM8994_IN1R_ENA_MASK);	
	val |= (WM8994_TSHUT_ENA |WM8994_TSHUT_OPDIS |WM8994_MIXINR_ENA 
		| WM8994_IN1R_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_2, 0x6110);

	if (!wm8994->testmode_config_flag) {
		val = wm8994_read(codec, WM8994_RIGHT_LINE_INPUT_1_2_VOLUME);	
		val &= ~(WM8994_IN1R_MUTE_MASK | WM8994_IN1R_VOL_MASK);
		val |= (WM8994_IN1R_VU | TUNING_CALL_EAR_INPUTMIX_VOL);
		wm8994_write(codec, WM8994_RIGHT_LINE_INPUT_1_2_VOLUME, val);

		/* unmute right pga, set volume */
		val = wm8994_read(codec,WM8994_INPUT_MIXER_4);
		val&= ~(WM8994_IN1R_TO_MIXINR_MASK | 
						WM8994_IN1R_MIXINR_VOL_MASK | 
						WM8994_MIXOUTR_MIXINR_VOL_MASK);
		val |= WM8994_IN1R_TO_MIXINR;
		wm8994_write(codec, WM8994_INPUT_MIXER_4, val);
	}	

	val = wm8994_read(codec,WM8994_INPUT_MIXER_2);
	val&= ~(WM8994_IN1RP_TO_IN1R_MASK | WM8994_IN1RN_TO_IN1R_MASK);
	val |= (WM8994_IN1RP_TO_IN1R|WM8994_IN1RN_TO_IN1R);
	wm8994_write(codec,WM8994_INPUT_MIXER_2, 0x0003 );	 

	/* Unmute*/
	if (!wm8994->testmode_config_flag) {
		val = wm8994_read(codec, WM8994_LEFT_OPGA_VOLUME);
		val &= ~(WM8994_MIXOUTL_MUTE_N_MASK | WM8994_MIXOUTL_VOL_MASK);
		val |= (WM8994_MIXOUT_VU | WM8994_MIXOUTL_MUTE_N | 
							TUNING_CALL_OPGAL_VOL);
		wm8994_write(codec,WM8994_LEFT_OPGA_VOLUME, val);

		val = wm8994_read(codec, WM8994_RIGHT_OPGA_VOLUME);
		val &= ~(WM8994_MIXOUTR_MUTE_N_MASK | WM8994_MIXOUTR_VOL_MASK);
		val |= (WM8994_MIXOUT_VU | WM8994_MIXOUTR_MUTE_N | 
							TUNING_CALL_OPGAR_VOL);
		wm8994_write(codec, WM8994_RIGHT_OPGA_VOLUME, val);
	}

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, WM8994_AIF2ADCL_ENA | 
		WM8994_ADCL_ENA | WM8994_AIF2ADCR_ENA | WM8994_ADCR_ENA);   

	val = wm8994_read(codec, 0x102); 	
	val &= ~(0x0003);
	val = 0x0003;
	wm8994_write(codec, 0x102,val);

	val = wm8994_read(codec, 0x56); 	
	val &= ~(0x0003);
	val = 0x0003;
	wm8994_write(codec, 0x56,val);

	val = wm8994_read(codec, 0x102); 	
	val &= ~(0x0000);
	val = 0x0000;
	wm8994_write(codec, 0x102, val);

	val = wm8994_read(codec, WM8994_CLASS_W_1); 	
	val &= ~(0x0005);
	val |= 0x0005;
	wm8994_write(codec, WM8994_CLASS_W_1, val);

	if (!wm8994->testmode_config_flag) {
		val = wm8994_read(codec, WM8994_LEFT_OUTPUT_VOLUME);
		val &= ~(WM8994_HPOUT1L_MUTE_N_MASK | WM8994_HPOUT1L_VOL_MASK);
		val |= (WM8994_HPOUT1_VU | WM8994_HPOUT1L_MUTE_N |
						TUNING_CALL_OUTPUTL_VOL);
		wm8994_write(codec, WM8994_LEFT_OUTPUT_VOLUME, val);

		val = wm8994_read(codec, WM8994_RIGHT_OUTPUT_VOLUME);
		val &= ~(WM8994_HPOUT1R_MUTE_N_MASK | WM8994_HPOUT1R_VOL_MASK);
		val |= (WM8994_HPOUT1_VU | WM8994_HPOUT1R_MUTE_N |
						TUNING_CALL_OUTPUTR_VOL);
		wm8994_write(codec, WM8994_RIGHT_OUTPUT_VOLUME, val);
	}

	val = wm8994_read(codec, WM8994_DC_SERVO_2); 	
	val &= ~(0x03E0);
	val = 0x03E0;
	wm8994_write(codec, WM8994_DC_SERVO_2, val);

	wm8994_write(codec, WM8994_ANALOGUE_HP_1, 0x0022);  
	wm8994_write(codec, WM8994_CHARGE_PUMP_1, 0x9F25);  

	msleep(5);

	/* Analogue Output Configuration */	
	wm8994_write(codec, WM8994_OUTPUT_MIXER_1, 0x0001);   
	wm8994_write(codec, WM8994_OUTPUT_MIXER_2, 0x0001);   

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, 0x0030);   

	wm8994_write(codec, WM8994_AIF2_CLOCKING_1, 0x0019);

	wm8994_write(codec, WM8994_DC_SERVO_1, 0x303);  

	msleep(160);

	TestReturn1 = wm8994_read(codec, WM8994_DC_SERVO_4);

	TestLow = (signed char)(TestReturn1 & 0xff);
	TestHigh = (signed char)((TestReturn1>>8) & 0xff);

	TestLow1 = ((signed short)TestLow-5)&0x00ff;
	TestHigh1 = (((signed short)(TestHigh-5)<<8)&0xff00);
	TestReturn2 = TestLow1|TestHigh1;
	wm8994_write(codec, WM8994_DC_SERVO_4, TestReturn2);

	val = wm8994_read(codec, WM8994_DC_SERVO_1); 	
	val &= ~(0x000F);
	val = 0x000F;
	wm8994_write(codec, WM8994_DC_SERVO_1, val);

	msleep(15);

	wm8994_write(codec, WM8994_ANALOGUE_HP_1, 0x00EE);  

	/* Unmute DAC1 left */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
	val &= ~(WM8994_DAC1L_MUTE_MASK | WM8994_DAC1L_VOL_MASK);
	val |= TUNING_DAC1L_VOL; 
	wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, val);

	/* Unmute and volume ctrl RightDAC */
	val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME); 
	val &= ~(WM8994_DAC1R_MUTE_MASK | WM8994_DAC1R_VOL_MASK);
	val |= TUNING_DAC1R_VOL;
	wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);

	wm8994_write(codec, WM8994_DAC2_LEFT_VOLUME, 0x01C0);
	wm8994_write(codec, WM8994_DAC2_RIGHT_VOLUME, 0x01C0);

#ifdef FEATURE_FACTORY_LOOPBACK
	if (loopback_mode == PBA_LOOPBACK_MODE_ON) {
		DEBUG_LOG("PBA_LOOPBACK_MODE_ON");

		val = wm8994_read(codec, WM8994_RIGHT_LINE_INPUT_1_2_VOLUME);	
		val &= ~(WM8994_IN1R_MUTE_MASK | WM8994_IN1R_VOL_MASK);
		val |= (WM8994_IN1R_VU | TUNING_LOOPBACK_EAR_INPUTMIX_VOL);
		wm8994_write(codec, WM8994_RIGHT_LINE_INPUT_1_2_VOLUME, val);

		/* unmute right pga, set volume */
		val = wm8994_read(codec, WM8994_INPUT_MIXER_4);
		val&= ~(WM8994_IN1R_TO_MIXINR_MASK | 
						WM8994_IN1R_MIXINR_VOL_MASK |
						WM8994_MIXOUTR_MIXINR_VOL_MASK);
		val |= (WM8994_IN1R_TO_MIXINR);
		wm8994_write(codec, WM8994_INPUT_MIXER_4, val);

		val = wm8994_read(codec, WM8994_LEFT_OPGA_VOLUME);
		val &= ~(WM8994_MIXOUTL_MUTE_N_MASK | WM8994_MIXOUTL_VOL_MASK);
		val |= (WM8994_MIXOUTL_MUTE_N | TUNING_LOOPBACK_OPGAL_VOL);
		wm8994_write(codec, WM8994_LEFT_OPGA_VOLUME, val);

		val = wm8994_read(codec, WM8994_RIGHT_OPGA_VOLUME);
		val &= ~(WM8994_MIXOUTR_MUTE_N_MASK | WM8994_MIXOUTR_VOL_MASK);
		val |= (WM8994_MIXOUT_VU | WM8994_MIXOUTR_MUTE_N |
						TUNING_LOOPBACK_OPGAR_VOL);
		wm8994_write(codec, WM8994_RIGHT_OPGA_VOLUME, val);

		val = wm8994_read(codec, WM8994_LEFT_OUTPUT_VOLUME);
		val &= ~(WM8994_HPOUT1L_MUTE_N_MASK | WM8994_HPOUT1L_VOL_MASK);
		val |= (WM8994_HPOUT1L_MUTE_N | TUNING_LOOPBACK_OUTPUTL_VOL);
		wm8994_write(codec, WM8994_LEFT_OUTPUT_VOLUME, val);

		val = wm8994_read(codec, WM8994_RIGHT_OUTPUT_VOLUME);
		val &= ~(WM8994_HPOUT1R_MUTE_N_MASK | WM8994_HPOUT1R_VOL_MASK);
		val |= (WM8994_HPOUT1_VU | WM8994_HPOUT1R_MUTE_N |
						TUNING_LOOPBACK_OUTPUTR_VOL);
		wm8994_write(codec, WM8994_RIGHT_OUTPUT_VOLUME, val);
	}
#endif

#ifdef CONFIG_SND_SOC_A1026
	if (HWREV >= 0x0A) {
		A1026SetBypass(1);
		A1026Sleep();
	} else {
		A1026Wakeup();
		A1026SetFeature(EAR_AECON);
	}
#endif

	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, 0x0000);  
	wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_1, 0x0000);

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, 0x0303);  	

        if(wm8994->rec_path != MIC_OFF)
                wm8994_record_headset_mic(codec);
}

void wm8994_set_voicecall_headphone(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;
	int val;
	
	u16 TestReturn1 = 0;
	u16 TestReturn2 = 0;
	u16 TestLow1 = 0;
	u16 TestHigh1 = 0;
	u8 TestLow = 0;
	u8 TestHigh = 0;


	DEBUG_LOG("");

	/* Reduced tick noise when changing call path(mute analogue output) */
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, 0x0003);  

	msleep(60);

	audio_ctrl_mic_bias_gpio(wm8994->pdata, 1);

	/* To remove the robotic sound */
	wm8994_write(codec, 0x0102, 0x0003);
	wm8994_write(codec, 0x0817, 0x0000);
	wm8994_write(codec, 0x0102, 0x0003);

	wm8994_set_voicecall_common(codec);

	/* Analogue Input Configuration -Main MIC */
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_2, 
					WM8994_TSHUT_ENA | WM8994_TSHUT_OPDIS |
					WM8994_MIXINL_ENA | WM8994_IN1L_ENA);

	wm8994_write(codec, WM8994_INPUT_MIXER_2, WM8994_IN1LP_TO_IN1L | 
							WM8994_IN1LN_TO_IN1L);

	/* Tx -> AIF2 Path */
	wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING, 
							WM8994_ADC1_TO_DAC2L);	
	wm8994_write(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING, 
							WM8994_ADC1_TO_DAC2R);

	/* Digital Path Enables and Unmutes*/	
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, WM8994_AIF2ADCL_ENA | 
					WM8994_ADCL_ENA | WM8994_AIF2ADCR_ENA);	

	/* Turn off charge pump */
	wm8994_write(codec, WM8994_CHARGE_PUMP_1, WM8994_CP_ENA_DEFAULT);

	/* Sidetone */
	wm8994_write(codec, 0x0621, 0x01C0);

	wm8994_write(codec, WM8994_DAC2_MIXER_VOLUMES, 0x018C); 
	wm8994_write(codec, WM8994_DAC2_LEFT_VOLUME, 0x01C0 );	
	wm8994_write(codec, WM8994_DAC2_RIGHT_VOLUME, 0x01C0); 

	wm8994_write(codec, WM8994_AIF2_CLOCKING_1, 0x0019);

	/* Unmute IN1L PGA, update volume */
	val = wm8994_read(codec, WM8994_LEFT_LINE_INPUT_1_2_VOLUME);	
	val &= ~(WM8994_IN1L_MUTE_MASK | WM8994_IN1L_VOL_MASK);
	val |= (WM8994_IN1L_VU | 0x0B);

	wm8994_write(codec, WM8994_LEFT_LINE_INPUT_1_2_VOLUME, val);

	/* Unmute the PGA */
	val = wm8994_read(codec, WM8994_INPUT_MIXER_3);
	val&= ~(WM8994_IN1L_TO_MIXINL_MASK | WM8994_IN1L_MIXINL_VOL_MASK |
						WM8994_MIXOUTL_MIXINL_VOL_MASK);
	val |= (WM8994_IN1L_TO_MIXINL | WM8994_IN1L_MIXINL_VOL);
	wm8994_write(codec, WM8994_INPUT_MIXER_3, val); 

	/* Unmute */
	if (!wm8994->testmode_config_flag) {
		val = wm8994_read(codec, WM8994_LEFT_OPGA_VOLUME);
		val &= ~(WM8994_MIXOUTL_MUTE_N_MASK | WM8994_MIXOUTL_VOL_MASK);
		val |= (WM8994_MIXOUT_VU | WM8994_MIXOUTL_MUTE_N |
							TUNING_CALL_OPGAL_VOL);
		wm8994_write(codec,WM8994_LEFT_OPGA_VOLUME, val );

		val = wm8994_read(codec, WM8994_RIGHT_OPGA_VOLUME);
		val &= ~(WM8994_MIXOUTR_MUTE_N_MASK | WM8994_MIXOUTR_VOL_MASK);
		val |= (WM8994_MIXOUT_VU | WM8994_MIXOUTR_MUTE_N |
							TUNING_CALL_OPGAR_VOL);
		wm8994_write(codec,WM8994_RIGHT_OPGA_VOLUME, val );
	}

	val = wm8994_read(codec, 0x102); 	
	val &= ~(0x0003);
	val = 0x0003;
	wm8994_write(codec, 0x102, val);

	val = wm8994_read(codec, 0x56); 	
	val &= ~(0x0003);
	val = 0x0003;
	wm8994_write(codec, 0x56, val);

	val = wm8994_read(codec, 0x102); 	
	val &= ~(0x0000);
	val = 0x0000;
	wm8994_write(codec, 0x102, val);

	val = wm8994_read(codec, WM8994_CLASS_W_1); 	
	val &= ~(0x0005);
	val |= 0x0005;
	wm8994_write(codec, WM8994_CLASS_W_1, val);

	if (!wm8994->testmode_config_flag) {
		val = wm8994_read(codec, WM8994_LEFT_OUTPUT_VOLUME);
		val &= ~(WM8994_HPOUT1L_MUTE_N_MASK | WM8994_HPOUT1L_VOL_MASK);
		val |= (WM8994_HPOUT1_VU | WM8994_HPOUT1L_MUTE_N |
						TUNING_CALL_OUTPUTL_VOL);
		wm8994_write(codec, WM8994_LEFT_OUTPUT_VOLUME, val);

		val = wm8994_read(codec, WM8994_RIGHT_OUTPUT_VOLUME);
		val &= ~(WM8994_HPOUT1R_MUTE_N_MASK | WM8994_HPOUT1R_VOL_MASK);
		val |= (WM8994_HPOUT1_VU | WM8994_HPOUT1R_MUTE_N |
						TUNING_CALL_OUTPUTR_VOL);
		wm8994_write(codec, WM8994_RIGHT_OUTPUT_VOLUME, val);
	}

	val = wm8994_read(codec, WM8994_DC_SERVO_2); 	
	val &= ~(0x03E0);
	val = 0x03E0;
	wm8994_write(codec, WM8994_DC_SERVO_2, val);

	wm8994_write(codec, WM8994_ANALOGUE_HP_1, 0x0022);  
	wm8994_write(codec, WM8994_CHARGE_PUMP_1, 0x9F25);  

	msleep(5);

	/* Analogue Output Configuration */	
	wm8994_write(codec, WM8994_OUTPUT_MIXER_1, 0x0001);   
	wm8994_write(codec, WM8994_OUTPUT_MIXER_2, 0x0001);   

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, 0x0030);   

	wm8994_write(codec, WM8994_AIF2_CLOCKING_1, 0x0019);

	wm8994_write(codec, WM8994_DC_SERVO_1, 0x303);  

	msleep(160);

	TestReturn1=wm8994_read(codec, WM8994_DC_SERVO_4);

	TestLow = (signed char)(TestReturn1 & 0xff);
	TestHigh = (signed char)((TestReturn1>>8) & 0xff);

	TestLow1 = ((signed short)TestLow-5)&0x00ff;
	TestHigh1 = (((signed short)(TestHigh-5)<<8)&0xff00);
	TestReturn2 = TestLow1|TestHigh1;
	wm8994_write(codec, WM8994_DC_SERVO_4, TestReturn2);

	val = wm8994_read(codec, WM8994_DC_SERVO_1); 	
	val &= ~(0x000F);
	val = 0x000F;
	wm8994_write(codec, WM8994_DC_SERVO_1, val);

	msleep(15);

	wm8994_write(codec, WM8994_ANALOGUE_HP_1, 0x00EE);  

	/* Unmute DAC1 left */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
	val &= ~(WM8994_DAC1L_MUTE_MASK | WM8994_DAC1L_VOL_MASK);
	val |= TUNING_DAC1L_VOL; 
	wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME ,val);

	/* Unmute and volume ctrl RightDAC */
	val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME ); 
	val &= ~(WM8994_DAC1R_MUTE_MASK | WM8994_DAC1R_VOL_MASK);
	val |= TUNING_DAC1R_VOL;
	wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);

#ifdef CONFIG_SND_SOC_A1026
	if (HWREV >= 0x0A) {
		A1026SetBypass(1);
		A1026Sleep();
	} else {
		A1026Wakeup();
		A1026SetFeature(BYPASSMODE);
	}
#endif

	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, 0x0000);  
	wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_1, 0x0000);

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, 0x0303);  	

        if(wm8994->rec_path != MIC_OFF)
                wm8994_record_main_mic(codec);
}

void wm8994_set_voicecall_speaker(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;
	int val;


	DEBUG_LOG("");

	/* Reduced tick noise when changing call path(mute analogue output) */
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, 0x0003);

	msleep(60);

	audio_ctrl_mic_bias_gpio(wm8994->pdata, 1);

	/* Anti Pop 2 */
	wm8994_write(codec, 0x0039, 0x006C);

	/* To remove the robotic sound */
	wm8994_write(codec, 0x0102, 0x0003);
	wm8994_write(codec, 0x0817, 0x0000);
	wm8994_write(codec, 0x0102, 0x0003);

	wm8994_set_voicecall_common(codec);

	wm8994_write(codec, 0x601, 0x0005);
	wm8994_write(codec, 0x602, 0x0005);
	wm8994_write(codec, 0x603, 0x0180);

	/* Tx -> AIF2 Path */
	wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING, 
							WM8994_ADC2_TO_DAC2L);	
	wm8994_write(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING, 
							WM8994_ADC2_TO_DAC2R);

	/*Analogue Input Configuration*/
	wm8994_write(codec, 0x02, 0x6120);
	wm8994_write(codec, WM8994_INPUT_MIXER_2, WM8994_IN2RP_TO_IN2R | 
							WM8994_IN2RN_TO_IN2R);

	if (!wm8994->testmode_config_flag) {
		/* Volume Control - Input */
		val = wm8994_read(codec, WM8994_INPUT_MIXER_4);
		val&= ~(WM8994_IN2R_TO_MIXINR_MASK | 
						WM8994_IN2R_MIXINR_VOL_MASK |
						WM8994_MIXOUTR_MIXINR_VOL_MASK);
		val |= (WM8994_IN2R_TO_MIXINR | TUNING_CALL_SPK_MIXER_VOL);
		wm8994_write(codec, WM8994_INPUT_MIXER_4, val); 

		val = wm8994_read(codec, WM8994_RIGHT_LINE_INPUT_3_4_VOLUME);	
		val &= ~(WM8994_IN2R_MUTE_MASK | WM8994_IN2R_VOL_MASK);
		val |= (WM8994_IN2R_VU | TUNING_CALL_SPK_INPUTMIX_VOL);
		wm8994_write(codec, WM8994_RIGHT_LINE_INPUT_3_4_VOLUME , val);
	}

	/* Analogue Output Configuration */	
	wm8994_write(codec,0x03, 0x0300);

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, WM8994_AIF2ADCL_ENA | 
		WM8994_ADCL_ENA | WM8994_AIF2ADCR_ENA | WM8994_ADCR_ENA);	

	wm8994_write(codec, WM8994_AIF2_CLOCKING_1, 0x0019);


	if (!wm8994->testmode_config_flag) {
		/* Volume Control - Output */
		/* Unmute the SPKMIXVOLUME */
		val = wm8994_read(codec, WM8994_SPKMIXL_ATTENUATION);
		val &= ~(WM8994_SPKMIXL_VOL_MASK);
		val |= TUNING_SPKMIXL_ATTEN;	
		wm8994_write(codec, WM8994_SPKMIXL_ATTENUATION, val);

		val = wm8994_read(codec,WM8994_SPKMIXR_ATTENUATION);
		val &= ~(WM8994_SPKMIXR_VOL_MASK);
		val |= TUNING_SPKMIXR_ATTEN;	
		wm8994_write(codec, WM8994_SPKMIXR_ATTENUATION, val);

		val = wm8994_read(codec,WM8994_SPEAKER_VOLUME_LEFT);
		val &= ~(WM8994_SPKOUTL_MUTE_N_MASK | WM8994_SPKOUTL_VOL_MASK);
		val |= (WM8994_SPKOUT_VU | WM8994_SPKOUTL_MUTE_N | 
			TUNING_CALL_SPKL_VOL);
		wm8994_write(codec, WM8994_SPEAKER_VOLUME_LEFT, val);

		val = wm8994_read(codec,WM8994_SPEAKER_VOLUME_RIGHT);
		val &= ~(WM8994_SPKOUTR_MUTE_N_MASK | WM8994_SPKOUTR_VOL_MASK);
		wm8994_write(codec, WM8994_SPEAKER_VOLUME_RIGHT, val);

		val = wm8994_read(codec,WM8994_CLASSD);
		val &= ~(WM8994_SPKOUTL_BOOST_MASK);
		val |= TUNING_CALL_CLASSD_VOL << WM8994_SPKOUTL_BOOST_SHIFT;
		wm8994_write(codec, WM8994_CLASSD, val);
	}

	val = wm8994_read(codec, WM8994_SPKOUT_MIXERS);
	val &= ~(WM8994_SPKMIXL_TO_SPKOUTL_MASK | 
					WM8994_SPKMIXR_TO_SPKOUTL_MASK | 
					WM8994_SPKMIXR_TO_SPKOUTR_MASK);
	val |= WM8994_SPKMIXL_TO_SPKOUTL;
	wm8994_write(codec, WM8994_SPKOUT_MIXERS, val);

	wm8994_write(codec, 0x36, 0x0003);

	wm8994_write(codec,WM8994_SIDETONE, 0x01C0);   

	wm8994_write(codec, WM8994_ANALOGUE_HP_1, 0x0000); 
	wm8994_write(codec, WM8994_DC_SERVO_1, 0x0000); 

	if (!wm8994->testmode_config_flag) {
		/* Unmute DAC1 left */
		val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
		val &= ~(WM8994_DAC1L_MUTE_MASK | WM8994_DAC1L_VOL_MASK);
		val |= TUNING_DAC1L_VOL; 
		wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME ,val);

		/* Unmute and volume ctrl RightDAC */
		val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME ); 
		val &= ~(WM8994_DAC1R_MUTE_MASK | WM8994_DAC1R_VOL_MASK);
		val |= TUNING_DAC1R_VOL;	
		wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);
	}		

	wm8994_write(codec, WM8994_DAC2_LEFT_VOLUME, 0x01C0);   
	wm8994_write(codec, WM8994_DAC2_RIGHT_VOLUME, 0x01C0);       

#ifdef CONFIG_SND_SOC_A1026
	if (HWREV >= 0x0A) {
		A1026SetBypass(1);
		A1026Sleep();
	} else {
		A1026Wakeup();
#ifdef FEATURE_FACTORY_LOOPBACK
		if ((loopback_mode == SIMPLETEST_LOOPBACK_MODE_ON)
			|| (loopback_mode == PBA_LOOPBACK_MODE_ON)
		) {
			A1026SetFeature(BYPASSMODE);
		} else 
#endif
		{
			A1026SetFeature(FARTALK);
		}
	}
#endif

	wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_1, 0x0000);   
	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, 0x0000);   

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, 
		WM8994_SPKOUTL_ENA | WM8994_VMID_SEL_NORMAL | WM8994_BIAS_ENA);	

        if(wm8994->rec_path != MIC_OFF)
                wm8994_record_2nd_main_mic(codec);
}

void wm8994_set_voicecall_bluetooth(struct snd_soc_codec *codec)
{
	int val;

	DEBUG_LOG("");


	/* Reduced tick noise when changing call path(mute analogue output) */
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, 0x0003);  

	/* To remove the robotic sound */
	wm8994_write(codec, 0x0102, 0x0003);
	wm8994_write(codec, 0x0817, 0x0000);
	wm8994_write(codec, 0x0102, 0x0003);

	wm8994_set_voicecall_common(codec);

	/* GPIO Configuration */		
	wm8994_write(codec, WM8994_GPIO_8, WM8994_GP8_DIR | WM8994_GP8_DB);
	wm8994_write(codec, WM8994_GPIO_9, WM8994_GP9_DB);
	wm8994_write(codec, WM8994_GPIO_10, WM8994_GP10_DB);
	wm8994_write(codec, WM8994_GPIO_11, WM8994_GP11_DB);

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, WM8994_AIF2ADCL_ENA |
		WM8994_AIF2ADCR_ENA);

	/* If Input MIC is enabled, bluetooth Rx is muted */
	wm8994_write(codec, WM8994_LEFT_LINE_INPUT_1_2_VOLUME, 
							WM8994_IN1L_MUTE);
	wm8994_write(codec, WM8994_LEFT_LINE_INPUT_3_4_VOLUME, 
							WM8994_IN2L_MUTE);
	wm8994_write(codec, WM8994_RIGHT_LINE_INPUT_1_2_VOLUME, 
							WM8994_IN1R_MUTE);
	wm8994_write(codec, WM8994_RIGHT_LINE_INPUT_3_4_VOLUME, 
							WM8994_IN2R_MUTE);
	wm8994_write(codec, WM8994_INPUT_MIXER_2, 0x00);
	wm8994_write(codec, WM8994_INPUT_MIXER_3, 0x00);
	wm8994_write(codec, WM8994_INPUT_MIXER_4, 0x00);

	/* 
		for BT DTMF Play
		Rx Path: AIF2ADCDAT2 select
		CP(CALL) Path:GPIO5/DACDAT2 select
		AP(DTMF) Path: DACDAT1 select
		Tx Path: GPIO8/DACDAT3 select 
	*/
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_6, 0x000C);

	/* AIF1 & AIF2 Output is connected to DAC1 */
	wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING, 
			WM8994_AIF2DACL_TO_DAC2L | WM8994_AIF1DAC1L_TO_DAC2L);
	wm8994_write(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING, 
			WM8994_AIF2DACR_TO_DAC2R | WM8994_AIF1DAC1R_TO_DAC2R);

	wm8994_write(codec, WM8994_AIF2_CLOCKING_1, 0x0019);

	wm8994_write(codec, WM8994_DAC2_MIXER_VOLUMES, 0x018C);
	wm8994_write(codec, WM8994_DAC2_LEFT_VOLUME, 0x01C0);
	wm8994_write(codec, WM8994_DAC2_RIGHT_VOLUME, 0x01C0);

	wm8994_write(codec, WM8994_OVERSAMPLING, 0X0000);

	/* Unmute DAC1 left */
	val = wm8994_read(codec,WM8994_DAC1_LEFT_VOLUME );
	val &= ~(WM8994_DAC1L_MUTE_MASK | WM8994_DAC1L_VOL_MASK);
	val |= TUNING_DAC1L_VOL;
	wm8994_write(codec,WM8994_DAC1_LEFT_VOLUME ,val);

	/* Unmute and volume ctrl RightDAC */
	val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME ); 
	val &= ~(WM8994_DAC1R_MUTE_MASK | WM8994_DAC1R_VOL_MASK);
	val |= TUNING_DAC1R_VOL;
	wm8994_write(codec,WM8994_DAC1_RIGHT_VOLUME,val);

#ifdef CONFIG_SND_SOC_A1026
	if (HWREV >= 0x0A) {
		A1026SetBypass(1);
		A1026Sleep();
	} else {
		A1026Wakeup();
		A1026SetFeature(BYPASSMODE);
	}
#endif

	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, 0x0000);  
	wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_1, 0x0000); 	
}

#ifdef FEATURE_TTY
void wm8994_set_voicecall_tty_full(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;
	int val;


	DEBUG_LOG("");

	val = wm8994_read(codec, WM8994_ANTIPOP_2); 
	val &= ~(WM8994_VMID_RAMP_MASK | WM8994_VMID_BUF_ENA_MASK | 
						WM8994_STARTUP_BIAS_ENA_MASK);
	val |= (WM8994_VMID_RAMP_MASK | WM8994_VMID_BUF_ENA | 
						WM8994_STARTUP_BIAS_ENA);
	wm8994_write(codec, WM8994_ANTIPOP_2, val);		

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1); 
	val = 0x0000;
	val |= (WM8994_VMID_SEL_NORMAL | WM8994_BIAS_ENA_MASK);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	/* To remove the robotic sound */
	wm8994_write(codec, 0x0102, 0x0003);
	wm8994_write(codec, 0x0817, 0x0000);
	wm8994_write(codec, 0x0102, 0x0003);

	wm8994_write(codec, WM8994_CHARGE_PUMP_1, 0x9F25);

	wm8994_write(codec, WM8994_GPIO_3, 0x0100);
	wm8994_write(codec, WM8994_GPIO_4, 0x0100);
	wm8994_write(codec, WM8994_GPIO_5, 0x8100);
	wm8994_write(codec, WM8994_GPIO_7, 0x0100);

	msleep(150);

	/* FLL2 Setting */
	wm8994_write(codec, WM8994_FLL2_CONTROL_2, 0x2F00);
	wm8994_write(codec, WM8994_FLL2_CONTROL_3, 0x3126);
	wm8994_write(codec, WM8994_FLL2_CONTROL_4, 0x0100);
	wm8994_write(codec, WM8994_FLL2_CONTROL_5, 0x0C88);
	wm8994_write(codec, WM8994_FLL2_CONTROL_1, WM8994_FLL2_FRACN_ENA | 
							WM8994_FLL2_ENA);

	/* Audio Interface & Clock Setting */
	val = wm8994_read(codec, WM8994_AIF2_CLOCKING_1); 
	val &= ~(WM8994_AIF2CLK_SRC_MASK);
	val |= (WM8994_AIF2CLK_SRC_MASK);
	wm8994_write(codec, WM8994_AIF2_CLOCKING_1, val);

	val = wm8994_read(codec, WM8994_CLOCKING_1);
	val &= ~(WM8994_DSP_FS1CLK_ENA_MASK | WM8994_DSP_FS2CLK_ENA_MASK | 
			WM8994_DSP_FSINTCLK_ENA_MASK | WM8994_SYSCLK_SRC_MASK);
	val |= (WM8994_DSP_FS1CLK_ENA | WM8994_DSP_FS2CLK_ENA | 
				WM8994_DSP_FSINTCLK_ENA | WM8994_SYSCLK_SRC);
	wm8994_write(codec, WM8994_CLOCKING_1, 0x000F);

	val = wm8994_read(codec, WM8994_OVERSAMPLING);
	val &= ~(WM8994_ADC_OSR128_MASK | WM8994_DAC_OSR128_MASK);
	val = 0x0000;
	wm8994_write(codec, WM8994_OVERSAMPLING, 0x0000);

	wm8994_write(codec, WM8994_AIF2_RATE, 0x3 << WM8994_AIF2CLK_RATE_SHIFT);

	val = wm8994_read(codec, WM8994_AIF1_MASTER_SLAVE);
	val &= ~(WM8994_AIF1_MSTR_MASK);
	val = 0x00;
	val |= (WM8994_AIF1_MSTR);
	wm8994_write(codec, WM8994_AIF1_MASTER_SLAVE, val);

	wm8994_write(codec, WM8994_AIF2_MASTER_SLAVE, WM8994_AIF2_MSTR | 
		WM8994_AIF2_CLK_FRC | WM8994_AIF2_LRCLK_FRC);

	if (HWREV >= 0x0A) {
		/* FMT : I2S */
		DEBUG_LOG("NC H/W Bypass I2S");

		wm8994_write(codec, WM8994_AIF2_CONTROL_1, 0xC010);
		A1026SetBypass(1);
	} else {
		/* FMT : Left Justified */
		wm8994_write(codec, WM8994_AIF2_CONTROL_1, 0xC008);
	}

	wm8994_write(codec, WM8994_AIF2_CONTROL_2, 0x0000);

	val = wm8994_read(codec, WM8994_AIF2_DAC_FILTERS_1);
	val &= ~(WM8994_AIF2DAC_MONO_MASK);
	val = 0x0000;
	wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_1, val);

	/* Input Path Routing */
	val = wm8994_read(codec, WM8994_INPUT_MIXER_2);
	val &= ~(WM8994_IN1RN_TO_IN1R_MASK);
	val |= (WM8994_IN1RN_TO_IN1R );
	wm8994_write(codec, WM8994_INPUT_MIXER_2, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_2);
	val = 0x0000;
	val |= (0x4000 | WM8994_TSHUT_OPDIS | WM8994_MIXINR_ENA_MASK | 
		WM8994_IN2R_ENA_MASK |WM8994_IN1R_ENA_MASK);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_2, val);	

	val = wm8994_read(codec, WM8994_INPUT_MIXER_4);
	val &= ~(WM8994_IN1R_TO_MIXINR_MASK | WM8994_IN1R_MIXINR_VOL_MASK);
	val |= WM8994_IN1R_TO_MIXINR;
	wm8994_write(codec, WM8994_INPUT_MIXER_4, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_4);
	val = 0x0000;
	val |= (WM8994_AIF2ADCR_ENA | WM8994_AIF2ADCL_ENA  | WM8994_ADCR_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, val);

	val = wm8994_read(codec, WM8994_DAC2_LEFT_MIXER_ROUTING);
	val &= ~(WM8994_ADC2_TO_DAC2L_MASK | WM8994_ADC1_TO_DAC2L_MASK);
	val |= (WM8994_ADC2_TO_DAC2L | WM8994_ADC1_TO_DAC2L);
	wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING);
	val &= ~(WM8994_ADC2_TO_DAC2R_MASK | WM8994_ADC1_TO_DAC2R_MASK);
	val |= (WM8994_ADC2_TO_DAC2R | WM8994_ADC1_TO_DAC2R);
	wm8994_write(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING, val);

	audio_ctrl_mic_bias_gpio(wm8994->pdata, 1);

	/* Output Path Routing */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_5);
	val = 0x0000;
	val |= (0x1000 | WM8994_AIF2DACL_ENA | WM8994_AIF1DAC1L_ENA | 
		WM8994_AIF1DAC1R_ENA | WM8994_DAC1L_ENA | WM8994_DAC1R_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_5, val);

	val = wm8994_read(codec, WM8994_DAC1_LEFT_MIXER_ROUTING);
	val &= ~(WM8994_AIF2DACL_TO_DAC1L_MASK | 
						WM8994_AIF1DAC1L_TO_DAC1L_MASK);
	val |= (WM8994_AIF2DACL_TO_DAC1L | WM8994_AIF1DAC1L_TO_DAC1L);
	wm8994_write(codec, WM8994_DAC1_LEFT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING);
	val &= ~(WM8994_AIF2DACR_TO_DAC1R_MASK | 
						WM8994_AIF1DAC1R_TO_DAC1R_MASK);
	val |= (WM8994_AIF2DACR_TO_DAC1R | WM8994_AIF1DAC1R_TO_DAC1R);
	wm8994_write(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
	val = 0x0000;
	val |= (WM8994_DAC1L_TO_HPOUT1L);
	wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val);

	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
	val = 0x0000;
	val |= (WM8994_DAC1R_TO_HPOUT1R);
	wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_3);
	val = 0x0000;
	val |= (WM8994_MIXOUTLVOL_ENA | WM8994_MIXOUTRVOL_ENA | 
				WM8994_MIXOUTL_ENA | WM8994_MIXOUTR_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, val);

	val = wm8994_read(codec, WM8994_ANALOGUE_HP_1);
	val &= ~(WM8994_HPOUT1L_RMV_SHORT_MASK | WM8994_HPOUT1L_OUTP_MASK | 
		WM8994_HPOUT1L_DLY_MASK | WM8994_HPOUT1R_RMV_SHORT_MASK | 
		WM8994_HPOUT1R_OUTP_MASK | WM8994_HPOUT1R_DLY_MASK);
	val |= (WM8994_HPOUT1L_RMV_SHORT | WM8994_HPOUT1L_OUTP | 
				WM8994_HPOUT1L_DLY | WM8994_HPOUT1R_RMV_SHORT | 
				WM8994_HPOUT1R_OUTP | WM8994_HPOUT1R_DLY);
	wm8994_write(codec, WM8994_ANALOGUE_HP_1, val);

	val = wm8994_read(codec, WM8994_AIF1_DAC1_FILTERS_1);
	val &= ~(WM8994_AIF1DAC1_MONO_MASK);
	val = 0x0000;
	val |= (WM8994_AIF1DAC1_MONO);
	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, val);

	/* Input Path Volume */
	if (!wm8994->testmode_config_flag) {
		val = wm8994_read(codec, WM8994_RIGHT_LINE_INPUT_1_2_VOLUME);
		val = 0x0000;
		val |= (WM8994_IN1R_VU | 0x0B);
		wm8994_write(codec, WM8994_RIGHT_LINE_INPUT_1_2_VOLUME, val);

		val = wm8994_read(codec, WM8994_DAC2_RIGHT_VOLUME);
		val = 0x0000;
		val |= (WM8994_DAC2_VU | 0x00C0);
		wm8994_write(codec, WM8994_DAC2_RIGHT_VOLUME, val);

		val = wm8994_read(codec, WM8994_DAC2_MIXER_VOLUMES);
		val = 0x0000;
		val |= 0x018C;
		wm8994_write(codec, WM8994_DAC2_MIXER_VOLUMES, val);
	}

	/* Output Path Volume */
	if (!wm8994->testmode_config_flag) {
		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_5);
		val = 0x0000;
		wm8994_write(codec, WM8994_OUTPUT_MIXER_5, val);

		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_6);
		val = 0x0000;
		wm8994_write(codec, WM8994_OUTPUT_MIXER_6, val);

		val = wm8994_read(codec, WM8994_LEFT_OPGA_VOLUME);
		val &= ~(WM8994_MIXOUTL_MUTE_N_MASK | WM8994_MIXOUTL_VOL_MASK);
		val |= (WM8994_MIXOUT_VU | WM8994_MIXOUTL_MUTE_N | 
						TUNING_CALL_TTYFULL_OPGAL_VOL);
		wm8994_write(codec, WM8994_LEFT_OPGA_VOLUME, val );

		val = wm8994_read(codec, WM8994_RIGHT_OPGA_VOLUME);
		val &= ~(WM8994_MIXOUTR_MUTE_N_MASK | WM8994_MIXOUTR_VOL_MASK);
		val |= (WM8994_MIXOUT_VU | WM8994_MIXOUTR_MUTE_N | 
						TUNING_CALL_TTYFULL_OPGAR_VOL);
		wm8994_write(codec,WM8994_RIGHT_OPGA_VOLUME, val );

		if(wm8994->codec_state & CALL_ACTIVE) {
			val = wm8994_read(codec,WM8994_LEFT_OUTPUT_VOLUME);
			val &= ~(WM8994_HPOUT1L_MUTE_N_MASK | 
						WM8994_HPOUT1L_VOL_MASK);
			val |= (WM8994_HPOUT1_VU | WM8994_HPOUT1L_MUTE_N | 
					TUNING_CALL_TTYFULL_OUTPUTL_VOL);
			wm8994_write(codec, WM8994_LEFT_OUTPUT_VOLUME, val);

			val = wm8994_read(codec,WM8994_RIGHT_OUTPUT_VOLUME);
			val &= ~(WM8994_HPOUT1R_MUTE_N_MASK | 
						WM8994_HPOUT1R_VOL_MASK);
			val |= (WM8994_HPOUT1_VU | WM8994_HPOUT1R_MUTE_N | 
					TUNING_CALL_TTYFULL_OUTPUTR_VOL);
			wm8994_write(codec, WM8994_RIGHT_OUTPUT_VOLUME, val);
		} else {
			val = wm8994_read(codec,WM8994_LEFT_OUTPUT_VOLUME);
			val &= ~(WM8994_HPOUT1L_MUTE_N_MASK | 
						WM8994_HPOUT1L_VOL_MASK);
			val |= (WM8994_HPOUT1_VU);
			wm8994_write(codec, WM8994_LEFT_OUTPUT_VOLUME, val);

			val = wm8994_read(codec,WM8994_RIGHT_OUTPUT_VOLUME);
			val &= ~(WM8994_HPOUT1R_MUTE_N_MASK | 
						WM8994_HPOUT1R_VOL_MASK);
			val |= (WM8994_HPOUT1_VU );
			wm8994_write(codec, WM8994_RIGHT_OUTPUT_VOLUME, val);
		}

		val = wm8994_read(codec,WM8994_DAC1_LEFT_VOLUME);
		val &= ~(WM8994_DAC1L_MUTE_MASK | WM8994_DAC1L_VOL_MASK);
		val = TUNING_CALL_TTYFULL_DAC1L_VOL; 
		wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, val);

		val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME); 
		val &= ~(WM8994_DAC1R_MUTE_MASK | WM8994_DAC1R_VOL_MASK);
		val = TUNING_CALL_TTYFULL_DAC1R_VOL; 
		wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);
	}

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_6, 0x0000);

	wm8994_write(codec, 0x0621, 0x01C0);

	wm8994_write(codec, 0x0001, 0x0303);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
	val = 0x0000;
	val |= (WM8994_SPKRVOL_ENA | WM8994_SPKLVOL_ENA | 0x0003);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	wm8994_write(codec, 0x0204, 0x0019);
	
	val = wm8994_read(codec, WM8994_AIF2_CLOCKING_1);
	val &= ~(WM8994_AIF2CLK_SRC_MASK |WM8994_AIF2CLK_ENA_MASK);
	val |= (WM8994_AIF2CLK_SRC_MASK |WM8994_AIF2CLK_ENA);
	wm8994_write(codec, WM8994_AIF2_CLOCKING_1, val);
}

void wm8994_set_voicecall_tty_hco(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;
	int val;


	DEBUG_LOG("");

	val = wm8994_read(codec, WM8994_ANTIPOP_2); 
	val &= ~(WM8994_VMID_RAMP_MASK | WM8994_VMID_BUF_ENA_MASK | 
						WM8994_STARTUP_BIAS_ENA_MASK);
	val |= (WM8994_VMID_RAMP_MASK | WM8994_VMID_BUF_ENA | 
						WM8994_STARTUP_BIAS_ENA);
	wm8994_write(codec, WM8994_ANTIPOP_2, val);		

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1); 
	val = 0x0000;
	val |= (WM8994_VMID_SEL_NORMAL | WM8994_BIAS_ENA_MASK);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	/* To remove the robotic sound */
	wm8994_write(codec, 0x0102, 0x0003);
	wm8994_write(codec, 0x0817, 0x0000);
	wm8994_write(codec, 0x0102, 0x0003);

	wm8994_write(codec, WM8994_CHARGE_PUMP_1, 0x9F25);

	wm8994_write(codec, WM8994_GPIO_3, 0x0100);
	wm8994_write(codec, WM8994_GPIO_4, 0x0100);
	wm8994_write(codec, WM8994_GPIO_5, 0x8100);
	wm8994_write(codec, WM8994_GPIO_7, 0x0100);

	msleep(150);

	/* FLL2 Setting */
	wm8994_write(codec, WM8994_FLL2_CONTROL_2, 0x2F00);
	wm8994_write(codec, WM8994_FLL2_CONTROL_3, 0x3126);
	wm8994_write(codec, WM8994_FLL2_CONTROL_4, 0x0100);
	wm8994_write(codec, WM8994_FLL2_CONTROL_5, 0x0C88);
	wm8994_write(codec, WM8994_FLL2_CONTROL_1, WM8994_FLL2_FRACN_ENA | 
							WM8994_FLL2_ENA);

	/* Audio Interface & Clock Setting */
	val = wm8994_read(codec, WM8994_AIF2_CLOCKING_1); 
	val &= ~(WM8994_AIF2CLK_SRC_MASK);
	val |= (WM8994_AIF2CLK_SRC_MASK);
	wm8994_write(codec, WM8994_AIF2_CLOCKING_1, val);

	val = wm8994_read(codec, WM8994_CLOCKING_1);
	val &= ~(WM8994_DSP_FS1CLK_ENA_MASK | WM8994_DSP_FS2CLK_ENA_MASK | 
			WM8994_DSP_FSINTCLK_ENA_MASK | WM8994_SYSCLK_SRC_MASK);
	val |= (WM8994_DSP_FS1CLK_ENA | WM8994_DSP_FS2CLK_ENA | 
				WM8994_DSP_FSINTCLK_ENA | WM8994_SYSCLK_SRC);
	wm8994_write(codec, WM8994_CLOCKING_1, 0x000F);

	val = wm8994_read(codec, WM8994_OVERSAMPLING);
	val &= ~(WM8994_ADC_OSR128_MASK | WM8994_DAC_OSR128_MASK);
	val = 0x0000;
	wm8994_write(codec, WM8994_OVERSAMPLING, 0x0000);

	wm8994_write(codec, WM8994_AIF2_RATE, 0x3 << WM8994_AIF2CLK_RATE_SHIFT);

	val = wm8994_read(codec, WM8994_AIF1_MASTER_SLAVE);
	val &= ~(WM8994_AIF1_MSTR_MASK);
	val = 0x00;
	val |= WM8994_AIF1_MSTR;
	wm8994_write(codec, WM8994_AIF1_MASTER_SLAVE, val);

	wm8994_write(codec, WM8994_AIF2_MASTER_SLAVE, WM8994_AIF2_MSTR | 
		WM8994_AIF2_CLK_FRC | WM8994_AIF2_LRCLK_FRC);

	if (HWREV >= 0x0A) {
		/* FMT : I2S */
		DEBUG_LOG("NC H/W Bypass I2S");

		wm8994_write(codec, WM8994_AIF2_CONTROL_1, 0xC010);
		A1026SetBypass(1);
	} else {
		/* FMT : Left Justified */
		wm8994_write(codec, WM8994_AIF2_CONTROL_1, 0xC008); 
	}

	wm8994_write(codec, WM8994_AIF2_CONTROL_2, 0x0000);

	val = wm8994_read(codec, WM8994_AIF2_DAC_FILTERS_1);
	val &= ~(WM8994_AIF2DAC_MONO_MASK);
	val = 0x0000;
	wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_1, val);

	/* Input Path Routing */
	val = wm8994_read(codec, WM8994_INPUT_MIXER_2);
	val &= ~(WM8994_IN1RN_TO_IN1R_MASK);
	val |= (WM8994_IN1RN_TO_IN1R);
	wm8994_write(codec, WM8994_INPUT_MIXER_2, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_2);
	val = 0x0000;
	val |= (0x4000 | WM8994_TSHUT_OPDIS | WM8994_MIXINR_ENA_MASK | 
				WM8994_IN2R_ENA_MASK | WM8994_IN1R_ENA_MASK);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_2, val);	

	val = wm8994_read(codec, WM8994_INPUT_MIXER_4);
	val &= ~(WM8994_IN1R_TO_MIXINR_MASK | WM8994_IN1R_MIXINR_VOL_MASK);
	val |= WM8994_IN1R_TO_MIXINR;
	wm8994_write(codec, WM8994_INPUT_MIXER_4, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_4);
	val = 0x0000;
	val |= (WM8994_AIF2ADCR_ENA | WM8994_AIF2ADCL_ENA  | WM8994_ADCR_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, val);	

	val = wm8994_read(codec, WM8994_DAC2_LEFT_MIXER_ROUTING);
	val &= ~(WM8994_ADC2_TO_DAC2L_MASK | WM8994_ADC1_TO_DAC2L_MASK);
	val |= (WM8994_ADC2_TO_DAC2L | WM8994_ADC1_TO_DAC2L);
	wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING);
	val &= ~(WM8994_ADC2_TO_DAC2R_MASK | WM8994_ADC1_TO_DAC2R_MASK);
	val |= (WM8994_ADC2_TO_DAC2R | WM8994_ADC1_TO_DAC2R);
	wm8994_write(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING, val);

	audio_ctrl_mic_bias_gpio(wm8994->pdata, 1);

	/* Output Path Routing */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_5);
	val = 0x0000;
	val |= (0x1000 | WM8994_AIF2DACL_ENA | WM8994_AIF1DAC1L_ENA | 
		WM8994_AIF1DAC1R_ENA | WM8994_DAC1L_ENA | WM8994_DAC1R_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_5, val);

	val = wm8994_read(codec, WM8994_DAC1_LEFT_MIXER_ROUTING);
	val &= ~(WM8994_AIF2DACL_TO_DAC1L_MASK | 
						WM8994_AIF1DAC1L_TO_DAC1L_MASK);
	val |= (WM8994_AIF2DACL_TO_DAC1L | WM8994_AIF1DAC1L_TO_DAC1L);
	wm8994_write(codec, WM8994_DAC1_LEFT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING);
	val &= ~(WM8994_AIF2DACR_TO_DAC1R_MASK | 
						WM8994_AIF1DAC1R_TO_DAC1R_MASK);
	val |= (WM8994_AIF2DACR_TO_DAC1R | WM8994_AIF1DAC1R_TO_DAC1R);
	wm8994_write(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
	val = 0x0000;
	val |= WM8994_DAC1L_TO_MIXOUTL;
	wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val);

	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
	val = 0x0000;
	val |= WM8994_DAC1R_TO_MIXOUTR;
	wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_3);
	val = 0x0000;
	val |= (WM8994_MIXOUTLVOL_ENA | WM8994_MIXOUTRVOL_ENA |
				WM8994_MIXOUTL_ENA | WM8994_MIXOUTR_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, val);

	val = wm8994_read(codec, WM8994_HPOUT2_MIXER);
	val &= ~(WM8994_MIXOUTLVOL_TO_HPOUT2_MASK | 
					WM8994_MIXOUTRVOL_TO_HPOUT2_MASK);
	val |= (WM8994_MIXOUTLVOL_TO_HPOUT2 | WM8994_MIXOUTRVOL_TO_HPOUT2);
	wm8994_write(codec, WM8994_HPOUT2_MIXER, val);

	val = wm8994_read(codec, WM8994_AIF1_DAC1_FILTERS_1);
	val &= ~(WM8994_AIF1DAC1_MONO_MASK);
	val = 0x0000;
	val |= (WM8994_AIF1DAC1_MONO);
	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, val);

	/* Input Path Volume */
	if (!wm8994->testmode_config_flag) {
		val = wm8994_read(codec, WM8994_RIGHT_LINE_INPUT_1_2_VOLUME);
		val = 0x0000;
		val |= (WM8994_IN1R_VU | 0x0B);
		wm8994_write(codec, WM8994_RIGHT_LINE_INPUT_1_2_VOLUME, val);

		val = wm8994_read(codec, WM8994_DAC2_RIGHT_VOLUME);
		val = 0x0000;
		val |= (WM8994_DAC2_VU | 0x00C0);
		wm8994_write(codec, WM8994_DAC2_RIGHT_VOLUME, val);

		val = wm8994_read(codec, WM8994_DAC2_MIXER_VOLUMES);
		val = 0x0000;
		val |= 0x018C;
		wm8994_write(codec, WM8994_DAC2_MIXER_VOLUMES, val);
	}

	/* Output Path Volume */
	if (!wm8994->testmode_config_flag) {
		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_5);
		val = 0x0000;
		wm8994_write(codec, WM8994_OUTPUT_MIXER_5, val);

		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_6);
		val = 0x0000;
		wm8994_write(codec, WM8994_OUTPUT_MIXER_6, val);

		val = wm8994_read(codec, WM8994_LEFT_OPGA_VOLUME);
		val &= ~(WM8994_MIXOUTL_MUTE_N_MASK | 
			WM8994_MIXOUTL_VOL_MASK);
		val |= (WM8994_MIXOUT_VU | WM8994_MIXOUTL_MUTE_N | 
			TUNING_CALL_TTYHCO_OPGAL_VOL);
		wm8994_write(codec, WM8994_LEFT_OPGA_VOLUME, val);

		val = wm8994_read(codec, WM8994_RIGHT_OPGA_VOLUME);
		val &= ~(WM8994_MIXOUTR_MUTE_N_MASK | WM8994_MIXOUTR_VOL_MASK);
		val |= (WM8994_MIXOUT_VU | WM8994_MIXOUTR_MUTE_N | 
			TUNING_CALL_TTYHCO_OPGAR_VOL);
		wm8994_write(codec, WM8994_RIGHT_OPGA_VOLUME, val);

		val = wm8994_read(codec,WM8994_DAC1_LEFT_VOLUME );
		val &= ~(WM8994_DAC1L_MUTE_MASK | WM8994_DAC1L_VOL_MASK);
		val = TUNING_CALL_TTYHCO_DAC1L_VOL; 
		wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME ,val);

		val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME ); 
		val &= ~(WM8994_DAC1R_MUTE_MASK | WM8994_DAC1R_VOL_MASK);
		val = TUNING_CALL_TTYHCO_DAC1R_VOL; 
		wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME,val);

		if(wm8994->codec_state & CALL_ACTIVE) {
			val = wm8994_read(codec, WM8994_HPOUT2_VOLUME); 
			val = 0x0000; 
			wm8994_write(codec, WM8994_HPOUT2_VOLUME,val);
		} else {
			wm8994_write(codec, 0x001F, 0x0020);
			val = wm8994_read(codec, WM8994_HPOUT2_VOLUME); 
			val &= ~(WM8994_HPOUT2_MUTE_MASK);
			val |= WM8994_HPOUT2_MUTE_MASK; 
			wm8994_write(codec, WM8994_HPOUT2_VOLUME,val);
		}
	}

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_6, 0x0000);

	val = wm8994_read(codec, WM8994_ANTIPOP_1); 
	val &= ~(WM8994_HPOUT2_IN_ENA_MASK);
	val |= (WM8994_HPOUT2_IN_ENA_MASK);
	wm8994_write(codec, WM8994_ANTIPOP_1,val);

	wm8994_write(codec, 0x0621, 0x01C0);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
	val = 0x0000;
	val |= (WM8994_HPOUT2_ENA | WM8994_MICB2_ENA | WM8994_MICB1_ENA | 
				WM8994_VMID_SEL_NORMAL | WM8994_BIAS_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	val = wm8994_read(codec, WM8994_AIF2_CLOCKING_1);
	val &= ~(WM8994_AIF2CLK_SRC_MASK |WM8994_AIF2CLK_ENA_MASK);
	val |= (WM8994_AIF2CLK_SRC_MASK |WM8994_AIF2CLK_ENA);
	wm8994_write(codec, WM8994_AIF2_CLOCKING_1, val);
}

void wm8994_set_voicecall_tty_vco(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;
	int val;


	DEBUG_LOG("");

	val = wm8994_read(codec, WM8994_ANTIPOP_2); 
	val &= ~(WM8994_VMID_RAMP_MASK | WM8994_VMID_BUF_ENA_MASK | 
						WM8994_STARTUP_BIAS_ENA_MASK);
	val |= (WM8994_VMID_RAMP_MASK | WM8994_VMID_BUF_ENA | 
						WM8994_STARTUP_BIAS_ENA);
	wm8994_write(codec, WM8994_ANTIPOP_2, val);		

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1); 
	val = 0x0000;
	val |= (WM8994_VMID_SEL_NORMAL | WM8994_BIAS_ENA_MASK);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	/* To remove the robotic sound */
	wm8994_write(codec, 0x0102, 0x0003);
	wm8994_write(codec, 0x0817, 0x0000);
	wm8994_write(codec, 0x0102, 0x0003);

	wm8994_write(codec, WM8994_CHARGE_PUMP_1, 0x9F25);

	wm8994_write(codec, WM8994_GPIO_3, 0x0100);
	wm8994_write(codec, WM8994_GPIO_4, 0x0100);
	wm8994_write(codec, WM8994_GPIO_5, 0x8100);
	wm8994_write(codec, WM8994_GPIO_7, 0x0100);

	msleep(150);

	/* FLL2 Setting */
	wm8994_write(codec, WM8994_FLL2_CONTROL_2, 0x2F00);
	wm8994_write(codec, WM8994_FLL2_CONTROL_3, 0x3126);
	wm8994_write(codec, WM8994_FLL2_CONTROL_4, 0x0100);
	wm8994_write(codec, WM8994_FLL2_CONTROL_5, 0x0C88);
	wm8994_write(codec, WM8994_FLL2_CONTROL_1, WM8994_FLL2_FRACN_ENA | 
							WM8994_FLL2_ENA);

	/* Audio Interface & Clock Setting */
	val = wm8994_read(codec, WM8994_AIF2_CLOCKING_1); 
	val &= ~(WM8994_AIF2CLK_SRC_MASK);
	val |= WM8994_AIF2CLK_SRC_MASK;
	wm8994_write(codec, WM8994_AIF2_CLOCKING_1, val);

	val = wm8994_read(codec, WM8994_CLOCKING_1);
	val &= ~(WM8994_DSP_FS1CLK_ENA_MASK | WM8994_DSP_FS2CLK_ENA_MASK | 
			WM8994_DSP_FSINTCLK_ENA_MASK | WM8994_SYSCLK_SRC_MASK);
	val |= (WM8994_DSP_FS1CLK_ENA | WM8994_DSP_FS2CLK_ENA | 
				WM8994_DSP_FSINTCLK_ENA | WM8994_SYSCLK_SRC);
	wm8994_write(codec, WM8994_CLOCKING_1, 0x000F);

	val = wm8994_read(codec, WM8994_OVERSAMPLING);
	val &= ~(WM8994_ADC_OSR128_MASK | WM8994_DAC_OSR128_MASK);
	val = 0x0000;
	wm8994_write(codec, WM8994_OVERSAMPLING, 0x0000);

	wm8994_write(codec, WM8994_AIF2_RATE, 0x3 << WM8994_AIF2CLK_RATE_SHIFT);

	val = wm8994_read(codec, WM8994_AIF1_MASTER_SLAVE);
	val &= ~(WM8994_AIF1_MSTR_MASK);
	val = 0x00;
	val |= WM8994_AIF1_MSTR;
	wm8994_write(codec, WM8994_AIF1_MASTER_SLAVE, val);

	wm8994_write(codec, WM8994_AIF2_MASTER_SLAVE, WM8994_AIF2_MSTR | 
				WM8994_AIF2_CLK_FRC | WM8994_AIF2_LRCLK_FRC);

	if (HWREV >= 0x0A) {
		/* FMT : I2S */
		DEBUG_LOG("NC H/W Bypass I2S");

		wm8994_write(codec, WM8994_AIF2_CONTROL_1, 0x4010); 
		A1026SetBypass(1);
	} else {
		/* FMT : Left Justified */
		wm8994_write(codec, WM8994_AIF2_CONTROL_1, 0x4008);
	}

	wm8994_write(codec, WM8994_AIF2_CONTROL_2, 0x0000);

	val = wm8994_read(codec, WM8994_AIF2_DAC_FILTERS_1);
	val &= ~(WM8994_AIF2DAC_MONO_MASK);
	val = 0x0000;
	wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_1, val);

	/* Input Path Routing */
	val = wm8994_read(codec, WM8994_INPUT_MIXER_2);
	val &= ~(WM8994_IN1LP_TO_IN1L_MASK | WM8994_IN1LN_TO_IN1L_MASK);
	val |= (WM8994_IN1LP_TO_IN1L | WM8994_IN1LN_TO_IN1L);
	wm8994_write(codec, WM8994_INPUT_MIXER_2, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_2);
	val = 0x0000;
	val |= (0x4000 | WM8994_TSHUT_OPDIS | WM8994_MIXINL_ENA | 
							WM8994_IN1L_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_2, val);	

	val = wm8994_read(codec, WM8994_INPUT_MIXER_3);
	val &= ~(WM8994_IN1R_TO_MIXINR_MASK | WM8994_IN1R_MIXINR_VOL_MASK);
	val |= (WM8994_IN1R_TO_MIXINR | WM8994_IN1R_MIXINR_VOL);
	wm8994_write(codec, WM8994_INPUT_MIXER_3, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_4);
	val = 0x0000;
	val |= (WM8994_AIF2ADCR_ENA |WM8994_AIF2ADCL_ENA |WM8994_ADCL_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, val);	

	val = wm8994_read(codec, WM8994_DAC2_LEFT_MIXER_ROUTING);
	val &= ~(WM8994_ADC2_TO_DAC2L_MASK | WM8994_ADC1_TO_DAC2L_MASK);
	val |= (WM8994_ADC2_TO_DAC2L | WM8994_ADC1_TO_DAC2L);
	wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING);
	val &= ~(WM8994_ADC2_TO_DAC2R_MASK | WM8994_ADC1_TO_DAC2R_MASK);
	val |= (WM8994_ADC2_TO_DAC2R | WM8994_ADC1_TO_DAC2R);
	wm8994_write(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING, val);

	audio_ctrl_mic_bias_gpio(wm8994->pdata, 1);

	/* Output Path Routing */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_5);
	val = 0x0000;
	val |= (0x1000 | WM8994_AIF2DACL_ENA | WM8994_AIF1DAC1L_ENA | 
		WM8994_AIF1DAC1R_ENA | WM8994_DAC1L_ENA | WM8994_DAC1R_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_5, val);

	val = wm8994_read(codec, WM8994_DAC1_LEFT_MIXER_ROUTING);
	val &= ~(WM8994_AIF2DACL_TO_DAC1L_MASK | 
						WM8994_AIF1DAC1L_TO_DAC1L_MASK);
	val |= (WM8994_AIF2DACL_TO_DAC1L | WM8994_AIF1DAC1L_TO_DAC1L);
	wm8994_write(codec, WM8994_DAC1_LEFT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING);
	val &= ~(WM8994_AIF2DACR_TO_DAC1R_MASK | 
						WM8994_AIF1DAC1R_TO_DAC1R_MASK);
	val |= (WM8994_AIF2DACR_TO_DAC1R | WM8994_AIF1DAC1R_TO_DAC1R);
	wm8994_write(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
	val = 0x0000;
	val |= WM8994_DAC1L_TO_HPOUT1L;
	wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val);

	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
	val = 0x0000;
	val |= WM8994_DAC1R_TO_HPOUT1R;
	wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_3);
	val = 0x0000;
	val |= (WM8994_MIXOUTLVOL_ENA | WM8994_MIXOUTRVOL_ENA | 
		WM8994_MIXOUTL_ENA | WM8994_MIXOUTR_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, val);

	val = wm8994_read(codec, WM8994_ANALOGUE_HP_1);
	val &= ~(WM8994_HPOUT1L_RMV_SHORT_MASK | WM8994_HPOUT1L_OUTP_MASK | 
		WM8994_HPOUT1L_DLY_MASK | WM8994_HPOUT1R_RMV_SHORT_MASK | 
		WM8994_HPOUT1R_OUTP_MASK | WM8994_HPOUT1R_DLY_MASK);
	val |= (WM8994_HPOUT1L_RMV_SHORT | WM8994_HPOUT1L_OUTP | 
				WM8994_HPOUT1L_DLY | WM8994_HPOUT1R_RMV_SHORT | 
				WM8994_HPOUT1R_OUTP | WM8994_HPOUT1R_DLY);
	wm8994_write(codec, WM8994_ANALOGUE_HP_1, val);

	val = wm8994_read(codec, WM8994_AIF1_DAC1_FILTERS_1);
	val &= ~(WM8994_AIF1DAC1_MONO_MASK);
	val = 0x0000;
	val |= WM8994_AIF1DAC1_MONO;
	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, val);

	/* Input Path Volume */
	if (!wm8994->testmode_config_flag) {
		val = wm8994_read(codec, WM8994_LEFT_LINE_INPUT_1_2_VOLUME);
		val = 0x0000;
		val |= (WM8994_IN1L_VU | TUNING_CALL_TTYVCO_INPUTMIX_VOL);
		wm8994_write(codec, WM8994_LEFT_LINE_INPUT_1_2_VOLUME, val);

		wm8994_write(codec, 0x0612, 0x01C0);
		val = wm8994_read(codec, WM8994_DAC2_LEFT_VOLUME);
		val = 0x0000;
		val |= (WM8994_DAC2_VU | 0x00C0);
		wm8994_write(codec, WM8994_DAC2_LEFT_VOLUME, val);

		val = wm8994_read(codec, WM8994_DAC2_MIXER_VOLUMES);
		val = 0x0000;
		val |= 0x018C;
		wm8994_write(codec, WM8994_DAC2_MIXER_VOLUMES, val);
	}

	/* Output Path Volume */
	if (!wm8994->testmode_config_flag) {
		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_5);
		val = 0x0000;
		wm8994_write(codec, WM8994_OUTPUT_MIXER_5, val);

		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_6);
		val = 0x0000;
		wm8994_write(codec, WM8994_OUTPUT_MIXER_6, val);

		val = wm8994_read(codec, WM8994_LEFT_OPGA_VOLUME);
		val &= ~(WM8994_MIXOUTL_MUTE_N_MASK | WM8994_MIXOUTL_VOL_MASK);
		val |= (WM8994_MIXOUT_VU | WM8994_MIXOUTL_MUTE_N | 
						TUNING_CALL_TTYVCO_OPGAL_VOL);
		wm8994_write(codec, WM8994_LEFT_OPGA_VOLUME, val );

		val = wm8994_read(codec, WM8994_RIGHT_OPGA_VOLUME);
		val &= ~(WM8994_MIXOUTR_MUTE_N_MASK | WM8994_MIXOUTR_VOL_MASK);
		val |= (WM8994_MIXOUT_VU | WM8994_MIXOUTR_MUTE_N | 
						TUNING_CALL_TTYVCO_OPGAR_VOL);
		wm8994_write(codec, WM8994_RIGHT_OPGA_VOLUME, val );

		if(wm8994->codec_state & CALL_ACTIVE) {
			val = wm8994_read(codec, WM8994_LEFT_OUTPUT_VOLUME);
			val &= ~(WM8994_HPOUT1L_MUTE_N_MASK | 
						WM8994_HPOUT1L_VOL_MASK);
			val |= (WM8994_HPOUT1_VU | WM8994_HPOUT1L_MUTE_N | 
				TUNING_CALL_TTYVCO_OUTPUTL_VOL);
			wm8994_write(codec, WM8994_LEFT_OUTPUT_VOLUME, val);

			val = wm8994_read(codec, WM8994_RIGHT_OUTPUT_VOLUME);
			val &= ~(WM8994_HPOUT1R_MUTE_N_MASK | 
						WM8994_HPOUT1R_VOL_MASK);
			val |= (WM8994_HPOUT1_VU | WM8994_HPOUT1R_MUTE_N | 
				TUNING_CALL_TTYVCO_OUTPUTR_VOL);
			wm8994_write(codec, WM8994_RIGHT_OUTPUT_VOLUME, val);
		} else {
			val = wm8994_read(codec, WM8994_LEFT_OUTPUT_VOLUME);
			val &= ~(WM8994_HPOUT1L_MUTE_N_MASK | 
						WM8994_HPOUT1L_VOL_MASK);
			val |= (WM8994_HPOUT1_VU );
			wm8994_write(codec, WM8994_LEFT_OUTPUT_VOLUME, val);

			val = wm8994_read(codec, WM8994_RIGHT_OUTPUT_VOLUME);
			val &= ~(WM8994_HPOUT1R_MUTE_N_MASK | 
						WM8994_HPOUT1R_VOL_MASK);
			val |= (WM8994_HPOUT1_VU );
			wm8994_write(codec, WM8994_RIGHT_OUTPUT_VOLUME, val);
		}

		val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
		val &= ~(WM8994_DAC1L_MUTE_MASK | WM8994_DAC1L_VOL_MASK);
		val = TUNING_CALL_TTYVCO_DAC1L_VOL; 
		wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME ,val);

		val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME); 
		val &= ~(WM8994_DAC1R_MUTE_MASK | WM8994_DAC1R_VOL_MASK);
		val = TUNING_CALL_TTYVCO_DAC1R_VOL; 
		wm8994_write(codec,WM8994_DAC1_RIGHT_VOLUME,val);
	}

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_6, 0x0000);

	wm8994_write(codec, 0x0621, 0x01C0);

	wm8994_write(codec, 0x0001, 0x0303);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
	val = 0x0000;
	val |= (WM8994_SPKRVOL_ENA | WM8994_SPKLVOL_ENA | 0x0003);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	wm8994_write(codec, 0x0204, 0x0019);

	val = wm8994_read(codec, WM8994_AIF2_CLOCKING_1);
	val &= ~(WM8994_AIF2CLK_SRC_MASK |WM8994_AIF2CLK_ENA_MASK);
	val |= (WM8994_AIF2CLK_SRC_MASK |WM8994_AIF2CLK_ENA);
	wm8994_write(codec, WM8994_AIF2_CLOCKING_1, val);
}
#endif

void wm8994_disable_fmradio_path(struct snd_soc_codec *codec,
	enum fmradio_path path)
{
        //Not used
}

void wm8994_set_fmradio_input_active(struct snd_soc_codec *codec, int on)
{
        //Not used
}

void wm8994_set_fmradio_common(struct snd_soc_codec *codec)
{
        //Not used
}

void wm8994_set_fmradio_headset(struct snd_soc_codec *codec)
{
        //Not used
}

void wm8994_set_fmradio_speaker(struct snd_soc_codec *codec)
{
        //Not used
}

void wm8994_set_fmradio_speaker_headset_mix(struct snd_soc_codec *codec)
{
        //Not used
}

