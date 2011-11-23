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
#include <plat/gpio-cfg.h>
#include <plat/map-base.h>
#include <mach/regs-clock.h>
#include "wm8994_samsung.h"

/* Debug Feature */
#define SUBJECT "wm8994_aries.c"

/* Definitions of tunning volumes for wm8994 */
struct gain_info_t playback_gain_table[PLAYBACK_GAIN_NUM] = {
	{ /* COMMON */
		.mode = COMMON_SET_BIT,
		.reg  = WM8994_DAC1_LEFT_VOLUME,	/* 610h */
		.mask = WM8994_DAC1L_VOL_MASK,
		.gain = WM8994_DAC1_VU | 0xC0
	}, {
		.mode = COMMON_SET_BIT,
		.reg  = WM8994_DAC1_RIGHT_VOLUME,	/* 611h */
		.mask = WM8994_DAC1R_VOL_MASK,
		.gain = WM8994_DAC1_VU | 0xC0
	}, {
		.mode = COMMON_SET_BIT,
		.reg  = WM8994_AIF1_DAC1_LEFT_VOLUME,	/* 402h */
		.mask = WM8994_AIF1DAC1L_VOL_MASK,
		.gain = WM8994_AIF1DAC1_VU | 0xC0
	}, {
		.mode = COMMON_SET_BIT,
		.reg  = WM8994_AIF1_DAC1_RIGHT_VOLUME,	/* 403h */
		.mask = WM8994_AIF1DAC1R_VOL_MASK,
		.gain = WM8994_AIF1DAC1_VU | 0xC0
	}, { /* RCV */
		.mode = PLAYBACK_RCV,
		.reg  = WM8994_OUTPUT_MIXER_5,		/* 31h */
		.mask = WM8994_DACL_MIXOUTL_VOL_MASK,
		.gain = 0x0 << WM8994_DACL_MIXOUTL_VOL_SHIFT
	}, {
		.mode = PLAYBACK_RCV,
		.reg  = WM8994_OUTPUT_MIXER_6,		/* 32h */
		.mask = WM8994_DACR_MIXOUTR_VOL_MASK,
		.gain = 0x0 << WM8994_DACR_MIXOUTR_VOL_SHIFT
	}, {
		.mode = PLAYBACK_RCV,
		.reg  = WM8994_LEFT_OPGA_VOLUME,	/* 20h */
		.mask = WM8994_MIXOUTL_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x3D
	}, {
		.mode = PLAYBACK_RCV,
		.reg  = WM8994_RIGHT_OPGA_VOLUME,	/* 21h */
		.mask = WM8994_MIXOUTR_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x3D
	}, {
		.mode = PLAYBACK_RCV,
		.reg  = WM8994_HPOUT2_VOLUME,		/* 1Fh */
		.mask = WM8994_HPOUT2_VOL_MASK,
		.gain = 0x0 << WM8994_HPOUT2_VOL_SHIFT
	}, { /* SPK */
		.mode = PLAYBACK_SPK,
		.reg  = WM8994_SPKMIXL_ATTENUATION,	/* 22h */
		.mask = WM8994_SPKMIXL_VOL_MASK,
		.gain = 0x0
	}, {
		.mode = PLAYBACK_SPK,
		.reg  = WM8994_SPKMIXR_ATTENUATION,	/* 23h */
		.mask = WM8994_SPKMIXR_VOL_MASK,
		.gain = 0x0
	}, {
		.mode = PLAYBACK_SPK,
		.reg  = WM8994_SPEAKER_VOLUME_LEFT,	/* 26h */
		.mask = WM8994_SPKOUTL_VOL_MASK,
		.gain = WM8994_SPKOUT_VU | 0x3E     /* +5dB */
	}, {
		.mode = PLAYBACK_SPK,
		.reg  = WM8994_SPEAKER_VOLUME_RIGHT,	/* 27h */
		.mask = WM8994_SPKOUTR_VOL_MASK,
		.gain = 0
	}, {
		.mode = PLAYBACK_SPK,
		.reg  = WM8994_CLASSD,			/* 25h */
		.mask = WM8994_SPKOUTL_BOOST_MASK,
		.gain = 0x05 << WM8994_SPKOUTL_BOOST_SHIFT  /* +7.5dB */
	}, { /* HP */
		.mode = PLAYBACK_HP,
		.reg  = WM8994_LEFT_OUTPUT_VOLUME,	/* 1Ch */
		.mask = WM8994_HPOUT1L_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x2F		/* -10dB */
	}, {
		.mode = PLAYBACK_HP,
		.reg  = WM8994_RIGHT_OUTPUT_VOLUME,	/* 1Dh */
		.mask = WM8994_HPOUT1R_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x2F		/* -10dB */
	}, {
		.mode = PLAYBACK_HP,
		.reg  = WM8994_LEFT_OPGA_VOLUME,	/* 20h */
		.mask = WM8994_MIXOUTL_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, {
		.mode = PLAYBACK_HP,
		.reg  = WM8994_RIGHT_OPGA_VOLUME,	/* 21h */
		.mask = WM8994_MIXOUTR_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, { /* SPK_HP */
		.mode = PLAYBACK_SPK_HP,
		.reg  = WM8994_SPKMIXL_ATTENUATION,	/* 22h */
		.mask = WM8994_SPKMIXL_VOL_MASK,
		.gain = 0x0
	}, {
		.mode = PLAYBACK_SPK_HP,
		.reg  = WM8994_SPKMIXR_ATTENUATION,	/* 23h */
		.mask = WM8994_SPKMIXR_VOL_MASK,
		.gain = 0x0
	}, {
		.mode = PLAYBACK_SPK_HP,
		.reg  = WM8994_SPEAKER_VOLUME_LEFT,	/* 26h */
		.mask = WM8994_SPKOUTL_VOL_MASK,
		.gain = WM8994_SPKOUT_VU | 0x3E
	}, {
		.mode = PLAYBACK_SPK_HP,
		.reg  = WM8994_SPEAKER_VOLUME_RIGHT,	/* 27h */
		.mask = WM8994_SPKOUTR_VOL_MASK,
		.gain = 0x0
	}, {
		.mode = PLAYBACK_SPK_HP,
		.reg  = WM8994_CLASSD,			/* 25h */
		.mask = WM8994_SPKOUTL_BOOST_MASK,
		.gain = 0x5 << WM8994_SPKOUTL_BOOST_SHIFT
	}, {
		.mode = PLAYBACK_SPK_HP,
		.reg  = WM8994_LEFT_OUTPUT_VOLUME,	/* 1Ch */
		.mask = WM8994_HPOUT1L_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x1E
	}, {
		.mode = PLAYBACK_SPK_HP,
		.reg  = WM8994_RIGHT_OUTPUT_VOLUME,	/* 1Dh */
		.mask = WM8994_HPOUT1R_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x1E
	}, {
		.mode = PLAYBACK_SPK_HP,
		.reg  = WM8994_LEFT_OPGA_VOLUME,	/* 20h */
		.mask = WM8994_MIXOUTL_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, {
		.mode = PLAYBACK_SPK_HP,
		.reg  = WM8994_RIGHT_OPGA_VOLUME,	/* 21h */
		.mask = WM8994_MIXOUTR_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, { /* RING_SPK */
		.mode = PLAYBACK_RING_SPK,
		.reg  = WM8994_SPEAKER_VOLUME_LEFT,	/* 26h */
		.mask = WM8994_SPKOUTL_VOL_MASK,
		.gain = WM8994_SPKOUT_VU | 0x3E
	}, {
		.mode = PLAYBACK_RING_SPK,
		.reg  = WM8994_CLASSD,			/* 25h */
		.mask = WM8994_SPKOUTL_BOOST_MASK,
		.gain = 0x5 << WM8994_SPKOUTL_BOOST_SHIFT
	}, { /* RING_HP */
		.mode = PLAYBACK_RING_HP,
		.reg  = WM8994_LEFT_OUTPUT_VOLUME,	/* 1Ch */
		.mask = WM8994_HPOUT1L_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x34
	}, {
		.mode = PLAYBACK_RING_HP,
		.reg  = WM8994_RIGHT_OUTPUT_VOLUME,	/* 1Dh */
		.mask = WM8994_HPOUT1R_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x34
	}, {
		.mode = PLAYBACK_RING_HP,
		.reg  = WM8994_LEFT_OPGA_VOLUME,	/* 20h */
		.mask = WM8994_MIXOUTL_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, {
		.mode = PLAYBACK_RING_HP,
		.reg  = WM8994_RIGHT_OPGA_VOLUME,	/* 21h */
		.mask = WM8994_MIXOUTR_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, { /* RING_SPK_HP */
		.mode = PLAYBACK_RING_SPK_HP,
		.reg  = WM8994_SPEAKER_VOLUME_LEFT,	/* 26h */
		.mask = WM8994_SPKOUTL_VOL_MASK,
		.gain = WM8994_SPKOUT_VU | 0x3E
	}, {
		.mode = PLAYBACK_RING_SPK_HP,
		.reg  = WM8994_CLASSD,			/* 25h */
		.mask = WM8994_SPKOUTL_BOOST_MASK,
		.gain = 0x5 << WM8994_SPKOUTL_BOOST_SHIFT
	}, {
		.mode = PLAYBACK_RING_SPK_HP,
		.reg  = WM8994_LEFT_OUTPUT_VOLUME,	/* 1Ch */
		.mask = WM8994_HPOUT1L_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x1E
	}, {
		.mode = PLAYBACK_RING_SPK_HP,
		.reg  = WM8994_RIGHT_OUTPUT_VOLUME,	/* 1Dh */
		.mask = WM8994_HPOUT1R_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x1E
	}, { /* HP_NO_MIC */
		.mode = PLAYBACK_HP_NO_MIC,
		.reg  = WM8994_LEFT_OUTPUT_VOLUME,	/* 1Ch */
		.mask = WM8994_HPOUT1L_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x2F		/* -10dB */
	}, {
		.mode = PLAYBACK_HP_NO_MIC,
		.reg  = WM8994_RIGHT_OUTPUT_VOLUME,	/* 1Dh */
		.mask = WM8994_HPOUT1R_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x2F		/* -10dB */
	}, {
		.mode = PLAYBACK_HP_NO_MIC,
		.reg  = WM8994_LEFT_OPGA_VOLUME,	/* 20h */
		.mask = WM8994_MIXOUTL_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, {
		.mode = PLAYBACK_HP_NO_MIC,
		.reg  = WM8994_RIGHT_OPGA_VOLUME,	/* 21h */
		.mask = WM8994_MIXOUTR_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, { /* EXTRA_DOCK_SPEAKER */
		.mode = PLAYBACK_EXTRA_DOCK,
		.reg  = WM8994_OUTPUT_MIXER_5,		/* 31h */
		.mask = WM8994_DACL_MIXOUTL_VOL_MASK,
		.gain = 0x0 << WM8994_DACL_MIXOUTL_VOL_SHIFT
	}, {
		.mode = PLAYBACK_EXTRA_DOCK,
		.reg  = WM8994_OUTPUT_MIXER_6,		/* 32h */
		.mask = WM8994_DACR_MIXOUTR_VOL_MASK,
		.gain = 0x0 << WM8994_DACR_MIXOUTR_VOL_SHIFT
	}, {
		.mode = PLAYBACK_EXTRA_DOCK,
		.reg  = WM8994_LEFT_OPGA_VOLUME,	/* 20h */
		.mask = WM8994_MIXOUTL_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, {
		.mode = PLAYBACK_EXTRA_DOCK,
		.reg  = WM8994_RIGHT_OPGA_VOLUME,	/* 21h */
		.mask = WM8994_MIXOUTR_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, {
		.mode = PLAYBACK_EXTRA_DOCK,
		.reg  = WM8994_LINE_OUTPUTS_VOLUME,	/* 1Eh */
		.mask = WM8994_LINEOUT2_VOL_MASK,
		.gain = 0x0 << WM8994_LINEOUT2_VOL_SHIFT
	}, { /* VOIP_RCV */
		.mode = PLAYBACK_VOIP_RCV,
		.reg  = WM8994_LEFT_LINE_INPUT_1_2_VOLUME,	/* 18h */
		.mask = WM8994_IN1L_VOL_MASK,
		.gain = WM8994_IN1L_VU | 0x17		/* +18dB */
	}, { /* VOIP_SPK */
		.mode = PLAYBACK_VOIP_SPK,
		.reg  = WM8994_LEFT_LINE_INPUT_1_2_VOLUME,	/* 18h */
		.mask = WM8994_IN1L_VOL_MASK,
		.gain = WM8994_IN1L_VU | 0x1A		/* +22.5dB */
	},
};

struct gain_info_t voicecall_gain_table[VOICECALL_GAIN_NUM] = {
	{ /* COMMON */
		.mode = COMMON_SET_BIT,
		.reg  = WM8994_DAC1_LEFT_VOLUME,	/* 610h */
		.mask = WM8994_DAC1L_VOL_MASK,
		.gain = WM8994_DAC1_VU | 0xC0		/* 0dB */
	}, {
		.mode = COMMON_SET_BIT,
		.reg  = WM8994_DAC1_RIGHT_VOLUME,	/* 611h */
		.mask = WM8994_DAC1R_VOL_MASK,
		.gain = WM8994_DAC1_VU | 0xC0		/* 0dB */
	}, {
		.mode = COMMON_SET_BIT,
		.reg  = WM8994_AIF1_DAC1_LEFT_VOLUME,	/* 402h */
		.mask = WM8994_AIF1DAC1L_VOL_MASK,
		.gain = WM8994_AIF1DAC1_VU | 0xC0	/* 0dB */
	}, {
		.mode = COMMON_SET_BIT,
		.reg  = WM8994_AIF1_DAC1_RIGHT_VOLUME,	/* 403h */
		.mask = WM8994_AIF1DAC1R_VOL_MASK,
		.gain = WM8994_AIF1DAC1_VU | 0xC0	/* 0dB */
	}, {
		.mode = COMMON_SET_BIT,
		.reg  = WM8994_DAC2_LEFT_VOLUME,	/* 612h */
		.mask = WM8994_DAC2L_VOL_MASK,
		.gain = WM8994_DAC2_VU | 0xC0		/* 0dB */
	}, {
		.mode = COMMON_SET_BIT,
		.reg  = WM8994_DAC2_RIGHT_VOLUME,	/* 613h */
		.mask = WM8994_DAC2R_VOL_MASK,
		.gain = WM8994_DAC2_VU | 0xC0		/* 0dB */
	}, { /* RCV */
		.mode = VOICECALL_RCV,
		.reg  = WM8994_LEFT_LINE_INPUT_1_2_VOLUME,	/* 18h */
		.mask = WM8994_IN1L_VOL_MASK,
		.gain = WM8994_IN1L_VU | 0x16		/* +16.5dB */
	}, {
		.mode = VOICECALL_RCV,
		.reg  = WM8994_INPUT_MIXER_3,		/* 29h */
		.mask = WM8994_IN1L_MIXINL_VOL_MASK | WM8994_MIXOUTL_MIXINL_VOL_MASK,
		.gain = 0x10				/* +30dB */
	}, {
		.mode = VOICECALL_RCV,
		.reg  = WM8994_OUTPUT_MIXER_5,		/* 31h */
		.mask = WM8994_DACL_MIXOUTL_VOL_MASK,
		.gain = 0x0 << WM8994_DACL_MIXOUTL_VOL_SHIFT
	}, {
		.mode = VOICECALL_RCV,
		.reg  = WM8994_OUTPUT_MIXER_6,		/* 32h */
		.mask = WM8994_DACR_MIXOUTR_VOL_MASK,
		.gain = 0x0 << WM8994_DACR_MIXOUTR_VOL_SHIFT
	}, {
		.mode = VOICECALL_RCV,
		.reg  = WM8994_LEFT_OPGA_VOLUME,	/* 20h */
		.mask = WM8994_MIXOUTL_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x3D
	}, {
		.mode = VOICECALL_RCV,
		.reg  = WM8994_RIGHT_OPGA_VOLUME,	/* 21h */
		.mask = WM8994_MIXOUTR_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x3D
	}, {
		.mode = VOICECALL_RCV,
		.reg  = WM8994_HPOUT2_VOLUME,		/* 1Fh */
		.mask = WM8994_HPOUT2_VOL_MASK,
		.gain = 0x0 << WM8994_HPOUT2_VOL_SHIFT
	}, { /* SPK */
		.mode = VOICECALL_SPK,
		.reg  = WM8994_INPUT_MIXER_4,		/* 2Ah */
		.mask = WM8994_IN2R_MIXINR_VOL_MASK | WM8994_MIXOUTR_MIXINR_VOL_MASK,
		.gain = 0x80				/* +30dB */
	}, {
		.mode = VOICECALL_SPK,
		.reg  = WM8994_RIGHT_LINE_INPUT_3_4_VOLUME,	/* 1Bh */
		.mask = WM8994_IN2R_MUTE_MASK | WM8994_IN2R_VOL_MASK,
		.gain = WM8994_IN2R_VU | 0x09		/* -3dB */
	}, {
		.mode = VOICECALL_SPK,
		.reg  = WM8994_SPKMIXL_ATTENUATION,	/* 22h */
		.mask = WM8994_SPKMIXL_VOL_MASK,
		.gain = 0x0				/* +0dB */
	}, {
		.mode = VOICECALL_SPK,
		.reg  = WM8994_SPKMIXR_ATTENUATION,	/* 23h */
		.mask = WM8994_SPKMIXR_VOL_MASK,
		.gain = 0x0				/* +0dB */
	}, {
		.mode = VOICECALL_SPK,
		.reg  = WM8994_SPEAKER_VOLUME_LEFT,	/* 26h */
		.mask = WM8994_SPKOUTL_VOL_MASK,
		.gain = WM8994_SPKOUT_VU | 0x3E		/* +5dB */
	}, {
		.mode = VOICECALL_SPK,
		.reg  = WM8994_SPEAKER_VOLUME_RIGHT,	/* 27h */
		.mask = WM8994_SPKOUTR_VOL_MASK,	/* -57dB */
		.gain = 0x0
	}, {
		.mode = VOICECALL_SPK,
		.reg  = WM8994_CLASSD,			/* 25h */
		.mask = WM8994_SPKOUTL_BOOST_MASK,
		.gain = 0x7 << WM8994_SPKOUTL_BOOST_SHIFT	/* +12dB */
	}, { /* HP */
		.mode = VOICECALL_HP,
		.reg  = WM8994_RIGHT_LINE_INPUT_1_2_VOLUME,	/* 1Ah */
		.mask = WM8994_IN1R_VOL_MASK,
		.gain = WM8994_IN1R_VU | 0x1D
	}, {
		.mode = VOICECALL_HP,
		.reg  = WM8994_INPUT_MIXER_4,		/* 2Ah */
		.mask = WM8994_IN1R_MIXINR_VOL_MASK | WM8994_MIXOUTR_MIXINR_VOL_MASK,
		.gain = 0x0
	}, {
		.mode = VOICECALL_HP,
		.reg  = WM8994_LEFT_OPGA_VOLUME,	/* 20h */
		.mask = WM8994_MIXOUTL_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, {
		.mode = VOICECALL_HP,
		.reg  = WM8994_RIGHT_OPGA_VOLUME,	/* 21h */
		.mask = WM8994_MIXOUTR_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, {
		.mode = VOICECALL_HP,
		.reg  = WM8994_LEFT_OUTPUT_VOLUME,	/* 1Ch */
		.mask = WM8994_HPOUT1L_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x30
	}, {
		.mode = VOICECALL_HP,
		.reg  = WM8994_RIGHT_OUTPUT_VOLUME,	/* 1Dh */
		.mask = WM8994_HPOUT1R_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x30
	}, { /* HP_NO_MIC */
		.mode = VOICECALL_HP_NO_MIC,
		.reg  = WM8994_LEFT_LINE_INPUT_1_2_VOLUME,	/* 18h */
		.mask = WM8994_IN1L_VOL_MASK,
		.gain = WM8994_IN1L_VU | 0x0D		/* 3dB */
	}, {
		.mode = VOICECALL_HP_NO_MIC,
		.reg  = WM8994_INPUT_MIXER_3,		/* 29h */
		.mask = WM8994_IN1L_MIXINL_VOL_MASK | WM8994_MIXOUTL_MIXINL_VOL_MASK,
		.gain = 0x10
	}, {
		.mode = VOICECALL_HP_NO_MIC,
		.reg  = WM8994_LEFT_OPGA_VOLUME,	/* 20h */
		.mask = WM8994_MIXOUTL_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, {
		.mode = VOICECALL_HP_NO_MIC,
		.reg  = WM8994_RIGHT_OPGA_VOLUME,	/* 21h */
		.mask = WM8994_MIXOUTR_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, {
		.mode = VOICECALL_HP_NO_MIC,
		.reg  = WM8994_LEFT_OUTPUT_VOLUME,	/* 1Ch */
		.mask = WM8994_HPOUT1L_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x30
	}, {
		.mode = VOICECALL_HP_NO_MIC,
		.reg  = WM8994_RIGHT_OUTPUT_VOLUME,	/* 1Dh */
		.mask = WM8994_HPOUT1R_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x30
	}
#ifdef FEATURE_TTY
        ,{ /* TTY_FULL */
		.mode = VOICECALL_TTY_FULL,
		.reg  = WM8994_RIGHT_LINE_INPUT_1_2_VOLUME,	/* 1Ah */
		.mask = WM8994_IN1R_VOL_MASK,
		.gain = WM8994_IN1R_VU | 0x0B
	}, {
		.mode = VOICECALL_TTY_FULL,
		.reg  = WM8994_LEFT_OPGA_VOLUME,	/* 20h */
		.mask = WM8994_MIXOUTL_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, {
		.mode = VOICECALL_TTY_FULL,
		.reg  = WM8994_RIGHT_OPGA_VOLUME,	/* 21h */
		.mask = WM8994_MIXOUTR_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, {
		.mode = VOICECALL_TTY_FULL,
		.reg  = WM8994_LEFT_OUTPUT_VOLUME,	/* 1Ch */
		.mask = WM8994_HPOUT1L_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x30
	}, {
		.mode = VOICECALL_TTY_FULL,
		.reg  = WM8994_RIGHT_OUTPUT_VOLUME,	/* 1Dh */
		.mask = WM8994_HPOUT1R_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x30
	}, { /* TTY_HCO */
		.mode = VOICECALL_TTY_HCO,
		.reg  = WM8994_RIGHT_LINE_INPUT_1_2_VOLUME,	/* 1Ah */
		.mask = WM8994_IN1R_VOL_MASK,
		.gain = WM8994_IN1R_VU | 0x0B
	}, {
		.mode = VOICECALL_TTY_HCO,
		.reg  = WM8994_LEFT_OPGA_VOLUME,	/* 20h */
		.mask = WM8994_MIXOUTL_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x3D
	}, {
		.mode = VOICECALL_TTY_HCO,
		.reg  = WM8994_RIGHT_OPGA_VOLUME,	/* 21h */
		.mask = WM8994_MIXOUTR_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x3D
	}, {
		.mode = VOICECALL_RCV,
		.reg  = WM8994_HPOUT2_VOLUME,		/* 1Fh */
		.mask = WM8994_HPOUT2_VOL_MASK,
		.gain = 0x0 << WM8994_HPOUT2_VOL_SHIFT
	}, { /* TTY_VCO */
		.mode = VOICECALL_TTY_VCO,
		.reg  = WM8994_LEFT_LINE_INPUT_1_2_VOLUME,	/* 18h */
		.mask = WM8994_IN1L_VOL_MASK,
		.gain = WM8994_IN1L_VU | 0x0B
	}, {
		.mode = VOICECALL_TTY_VCO,
		.reg  = WM8994_LEFT_OPGA_VOLUME,	/* 20h */
		.mask = WM8994_MIXOUTL_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, {
		.mode = VOICECALL_TTY_VCO,
		.reg  = WM8994_RIGHT_OPGA_VOLUME,	/* 21h */
		.mask = WM8994_MIXOUTR_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, {
		.mode = VOICECALL_TTY_VCO,
		.reg  = WM8994_LEFT_OUTPUT_VOLUME,	/* 1Ch */
		.mask = WM8994_HPOUT1L_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x30
	}, {
		.mode = VOICECALL_TTY_VCO,
		.reg  = WM8994_RIGHT_OUTPUT_VOLUME,	/* 1Dh */
		.mask = WM8994_HPOUT1R_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x30
        }
#endif
};

struct gain_info_t recording_gain_table[RECORDING_GAIN_NUM] = {
	{ /* MAIN */
		.mode = RECORDING_MAIN,
		.reg  = WM8994_LEFT_LINE_INPUT_1_2_VOLUME,	/* 18h */
		.mask = WM8994_IN1L_VOL_MASK,
		.gain = WM8994_IN1L_VU | 0x18		/* +19.5dB */
	}, {
		.mode = RECORDING_MAIN,
		.reg  = WM8994_INPUT_MIXER_3,		/* 29h */
		.mask = WM8994_IN1L_MIXINL_VOL_MASK | WM8994_MIXOUTL_MIXINL_VOL_MASK,
		.gain = 0x10				/* +30dB */
	}, {
		.mode = RECORDING_MAIN,
		.reg  = WM8994_AIF1_ADC1_LEFT_VOLUME,	/* 400h */
		.mask = WM8994_AIF1ADC1L_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xC0       /* 0dB */
	}, {
		.mode = RECORDING_MAIN,
		.reg  = WM8994_AIF1_ADC1_RIGHT_VOLUME,	/* 401h */
		.mask = WM8994_AIF1ADC1R_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xC0       /* 0dB */
	}, { /* MAIN2 */
		.mode = RECORDING_MAIN2,
		.reg  = WM8994_RIGHT_LINE_INPUT_3_4_VOLUME,	/* 1Bh */
		.mask = WM8994_IN2R_VOL_MASK,
		.gain = WM8994_IN2R_VU | 0x09		/* -3dB */
	}, {
		.mode = RECORDING_MAIN2,
		.reg  = WM8994_INPUT_MIXER_4,		/* 2Ah */
		.mask = WM8994_IN2R_MIXINR_VOL_MASK | WM8994_MIXOUTR_MIXINR_VOL_MASK,
		.gain = WM8994_IN2R_MIXINR_VOL		/* +30dB */
	}, {
		.mode = RECORDING_MAIN2,
		.reg  = WM8994_AIF1_ADC1_LEFT_VOLUME,	/* 400h */
		.mask = WM8994_AIF1ADC1L_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xC0       /* 0dB */
	}, {
		.mode = RECORDING_MAIN2,
		.reg  = WM8994_AIF1_ADC1_RIGHT_VOLUME,	/* 401h */
		.mask = WM8994_AIF1ADC1R_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xC0       /* 0dB */
	}, { /* HP */
		.mode = RECORDING_HP,
		.reg  = WM8994_RIGHT_LINE_INPUT_1_2_VOLUME,	/* 1Ah */
		.mask = WM8994_IN1R_VOL_MASK,
		.gain = WM8994_IN1R_VU | 0x15
	}, {
		.mode = RECORDING_HP,
		.reg  = WM8994_INPUT_MIXER_4,		/* 2Ah */
		.mask = WM8994_IN1R_MIXINR_VOL_MASK | WM8994_MIXOUTR_MIXINR_VOL_MASK,
		.gain = 0x10
	}, {
		.mode = RECORDING_HP,
		.reg  = WM8994_AIF1_ADC1_LEFT_VOLUME,	/* 400h */
		.mask = WM8994_AIF1ADC1L_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xC0
	}, {
		.mode = RECORDING_HP,
		.reg  = WM8994_AIF1_ADC1_RIGHT_VOLUME,	/* 401h */
		.mask = WM8994_AIF1ADC1R_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xC0
	}, { /* RECOGNITION_MAIN */
		.mode = RECORDING_REC_MAIN,
		.reg  = WM8994_LEFT_LINE_INPUT_1_2_VOLUME,	/* 18h */
		.mask = WM8994_IN1L_VOL_MASK,
		.gain = WM8994_IN1L_VU | 0x1C		/* +25.5dB */
	}, {
		.mode = RECORDING_REC_MAIN,
		.reg  = WM8994_INPUT_MIXER_3,		/* 29h */
		.mask = WM8994_IN1L_MIXINL_VOL_MASK | WM8994_MIXOUTL_MIXINL_VOL_MASK,
		.gain = 0x0				/* 0dB */
	}, {
		.mode = RECORDING_REC_MAIN,
		.reg  = WM8994_AIF1_ADC1_LEFT_VOLUME,	/* 400h */
		.mask = WM8994_AIF1ADC1L_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xD0	/* +6dB */
	}, {
		.mode = RECORDING_REC_MAIN,
		.reg  = WM8994_AIF1_ADC1_RIGHT_VOLUME,	/* 401h */
		.mask = WM8994_AIF1ADC1R_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xD0	/* +6dB */
	}, { /* RECOGNITION_MAIN2 */
		.mode = RECORDING_REC_MAIN2,
		.reg  = WM8994_RIGHT_LINE_INPUT_3_4_VOLUME,	/* 1Bh */
		.mask = WM8994_IN2R_VOL_MASK,
		.gain = WM8994_IN2R_VU | 0x1C		/* +25.5dB */
	}, {
		.mode = RECORDING_REC_MAIN2,
		.reg  = WM8994_INPUT_MIXER_4,		/* 2Ah */
		.mask = WM8994_IN2R_MIXINR_VOL_MASK | WM8994_MIXOUTR_MIXINR_VOL_MASK,
		.gain = 0x0             		/* 0dB */
	}, {
		.mode = RECORDING_REC_MAIN2,
		.reg  = WM8994_AIF1_ADC1_LEFT_VOLUME,	/* 400h */
		.mask = WM8994_AIF1ADC1L_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xC8       /* 0dB */
	}, {
		.mode = RECORDING_REC_MAIN2,
		.reg  = WM8994_AIF1_ADC1_RIGHT_VOLUME,	/* 401h */
		.mask = WM8994_AIF1ADC1R_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xC8       /* 0dB */
	}, { /* RECOGNITION_HP */
		.mode = RECORDING_REC_HP,
		.reg  = WM8994_RIGHT_LINE_INPUT_1_2_VOLUME,	/* 1Ah */
		.mask = WM8994_IN1R_VOL_MASK,
		.gain = WM8994_IN1R_VU | 0x10		/* +7.5dB */
	}, {
		.mode = RECORDING_REC_HP,
		.reg  = WM8994_INPUT_MIXER_4,		/* 2Ah */
		.mask = WM8994_IN1R_MIXINR_VOL_MASK | WM8994_MIXOUTR_MIXINR_VOL_MASK,
		.gain = 0x10   /* +30dB */
	}, {
		.mode = RECORDING_REC_HP,
		.reg  = WM8994_AIF1_ADC1_LEFT_VOLUME,	/* 400h */
		.mask = WM8994_AIF1ADC1L_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xC0
	}, {
		.mode = RECORDING_REC_HP,
		.reg  = WM8994_AIF1_ADC1_RIGHT_VOLUME,	/* 401h */
		.mask = WM8994_AIF1ADC1R_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xC0
	}, { /* CAMCORDER_MAIN */
		.mode = RECORDING_CAM_MAIN,
		.reg  = WM8994_LEFT_LINE_INPUT_1_2_VOLUME,	/* 18h */
		.mask = WM8994_IN1L_VOL_MASK,
		.gain = WM8994_IN1L_VU | 0x18		/* +19.5dB */
	}, {
		.mode = RECORDING_CAM_MAIN,
		.reg  = WM8994_INPUT_MIXER_3,		/* 29h */
		.mask = WM8994_IN1L_MIXINL_VOL_MASK | WM8994_MIXOUTL_MIXINL_VOL_MASK,
		.gain = 0x10  /* 30dB */
	}, {
		.mode = RECORDING_CAM_MAIN,
		.reg  = WM8994_AIF1_ADC1_LEFT_VOLUME,	/* 400h */
		.mask = WM8994_AIF1ADC1L_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xC0	/* +0dB */
	}, {
		.mode = RECORDING_CAM_MAIN,
		.reg  = WM8994_AIF1_ADC1_RIGHT_VOLUME,	/* 401h */
		.mask = WM8994_AIF1ADC1R_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xC0	/* +0dB */
	}, { /* CAMCORDER_MAIN2 */
		.mode = RECORDING_CAM_MAIN2,
		.reg  = WM8994_RIGHT_LINE_INPUT_3_4_VOLUME,	/* 1Bh */
		.mask = WM8994_IN2R_VOL_MASK,
		.gain = WM8994_IN2R_VU | 0x18		/* +19.5dB */
	}, {
		.mode = RECORDING_CAM_MAIN2,
		.reg  = WM8994_INPUT_MIXER_4,		/* 2Ah */
		.mask = WM8994_IN2R_MIXINR_VOL_MASK | WM8994_MIXOUTR_MIXINR_VOL_MASK,
		.gain = 0x10             		/* +30dB */
	}, {
		.mode = RECORDING_CAM_MAIN2,
		.reg  = WM8994_AIF1_ADC1_LEFT_VOLUME,	/* 400h */
		.mask = WM8994_AIF1ADC1L_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xC8       /* 0dB */
	}, {
		.mode = RECORDING_CAM_MAIN2,
		.reg  = WM8994_AIF1_ADC1_RIGHT_VOLUME,	/* 401h */
		.mask = WM8994_AIF1ADC1R_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xC8       /* 0dB */		
	}, { /* CAMCORDER_HP */
		.mode = RECORDING_CAM_HP,
		.reg  = WM8994_RIGHT_LINE_INPUT_1_2_VOLUME,	/* 1Ah */
		.mask = WM8994_IN1R_VOL_MASK,
		.gain = WM8994_IN1R_VU | 0x15		/* +15dB */
	}, {
		.mode = RECORDING_CAM_HP,
		.reg  = WM8994_INPUT_MIXER_4,		/* 2Ah */
		.mask = WM8994_IN1R_MIXINR_VOL_MASK | WM8994_MIXOUTR_MIXINR_VOL_MASK,
		.gain = 0x10				/* +30dB */
	}, {
		.mode = RECORDING_CAM_HP,
		.reg  = WM8994_AIF1_ADC1_LEFT_VOLUME,	/* 400h */
		.mask = WM8994_AIF1ADC1L_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xC0
	}, {
		.mode = RECORDING_CAM_HP,
		.reg  = WM8994_AIF1_ADC1_RIGHT_VOLUME,	/* 401h */
		.mask = WM8994_AIF1ADC1R_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xC0
	}, { /* VOIP_MAIN */
		.mode = RECORDING_VOIP_MAIN,
		.reg  = WM8994_INPUT_MIXER_3,		/* 29h */
		.mask = WM8994_IN1L_MIXINL_VOL_MASK | WM8994_MIXOUTL_MIXINL_VOL_MASK,
		.gain = 0x0				/* 0dB */
	}, {
		.mode = RECORDING_VOIP_MAIN,
		.reg  = WM8994_AIF1_ADC1_LEFT_VOLUME,	/* 400h */
		.mask = WM8994_AIF1ADC1L_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xEF	/* +17.625dB */
	}, {
		.mode = RECORDING_VOIP_MAIN,
		.reg  = WM8994_AIF1_ADC1_RIGHT_VOLUME,	/* 401h */
		.mask = WM8994_AIF1ADC1R_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xEF	/* +17.625dB */
	}, { /* VOIP_MAIN2 */
		.mode = RECORDING_VOIP_MAIN2,
		.reg  = WM8994_INPUT_MIXER_4,		/* 2Ah */
		.mask = WM8994_IN2R_MIXINR_VOL_MASK | WM8994_MIXOUTR_MIXINR_VOL_MASK,
		.gain = 0x0             		/* 0dB */
	}, {
		.mode = RECORDING_VOIP_MAIN2,
		.reg  = WM8994_AIF1_ADC1_LEFT_VOLUME,	/* 400h */
		.mask = WM8994_AIF1ADC1L_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xEF       /* +17.625dB */
	}, {
		.mode = RECORDING_VOIP_MAIN2,
		.reg  = WM8994_AIF1_ADC1_RIGHT_VOLUME,	/* 401h */
		.mask = WM8994_AIF1ADC1R_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xEF       /* +17.625dB */	
	}, { /* VOIP_HP */
		.mode = RECORDING_VOIP_HP,
		.reg  = WM8994_RIGHT_LINE_INPUT_1_2_VOLUME,	/* 1Ah */
		.mask = WM8994_IN1R_VOL_MASK,
		.gain = WM8994_IN1R_VU | 0x17		/* +22.5dB */
	}, {
		.mode = RECORDING_VOIP_HP,
		.reg  = WM8994_INPUT_MIXER_4,		/* 2Ah */
		.mask = WM8994_IN1R_MIXINR_VOL_MASK | WM8994_MIXOUTR_MIXINR_VOL_MASK,
		.gain = 0x10				/* +30dB */
	}, {
		.mode = RECORDING_VOIP_HP,
		.reg  = WM8994_AIF1_ADC1_LEFT_VOLUME,	/* 400h */
		.mask = WM8994_AIF1ADC1L_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xEF	/* +17.625dB */
	}, {
		.mode = RECORDING_VOIP_HP,
		.reg  = WM8994_AIF1_ADC1_RIGHT_VOLUME,	/* 401h */
		.mask = WM8994_AIF1ADC1R_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xEF	/* +17.625dB */
	},
};

struct gain_info_t fmradio_gain_table[FMRADIO_GAIN_NUM] = {
	{ /* COMMON */
		.mode = COMMON_SET_BIT,
		.reg  = WM8994_DAC1_LEFT_VOLUME,	/* 610h */
		.mask = WM8994_DAC1L_VOL_MASK,
		.gain = WM8994_DAC1_VU | 0xC0		/* 0dB */
	}, {
		.mode = COMMON_SET_BIT,
		.reg  = WM8994_DAC1_RIGHT_VOLUME,	/* 611h */
		.mask = WM8994_DAC1R_VOL_MASK,
		.gain = WM8994_DAC1_VU | 0xC0		/* 0dB */
	}, {
		.mode = COMMON_SET_BIT,
		.reg  = WM8994_AIF1_DAC1_LEFT_VOLUME,	/* 402h */
		.mask = WM8994_AIF1DAC1L_VOL_MASK,
		.gain = WM8994_AIF1DAC1_VU | 0xC0	/* 0dB */
	}, {
		.mode = COMMON_SET_BIT,
		.reg  = WM8994_AIF1_DAC1_RIGHT_VOLUME,	/* 403h */
		.mask = WM8994_AIF1DAC1R_VOL_MASK,
		.gain = WM8994_AIF1DAC1_VU | 0xC0	/* 0dB */
	}, {
		.mode = COMMON_SET_BIT,
		.reg  = WM8994_DAC2_LEFT_VOLUME,	/* 612h */
		.mask = WM8994_DAC2L_VOL_MASK,
		.gain = WM8994_DAC2_VU | 0xC0		/* 0dB */
	}, {
		.mode = COMMON_SET_BIT,
		.reg  = WM8994_DAC2_RIGHT_VOLUME,	/* 613h */
		.mask = WM8994_DAC2R_VOL_MASK,
		.gain = WM8994_DAC2_VU | 0xC0		/* 0dB */
	}, { /* Headset */
		.mode = FMRADIO_HP,
		.reg  = WM8994_LEFT_LINE_INPUT_3_4_VOLUME,	/* 19h */
		.mask = WM8994_IN2L_VOL_MASK,
		.gain = WM8994_IN2L_VU | 0x0B
	}, {
		.mode = FMRADIO_HP,
		.reg  = WM8994_RIGHT_LINE_INPUT_3_4_VOLUME,	/* 1Bh */
		.mask = WM8994_IN2R_VOL_MASK,
		.gain = WM8994_IN2R_VU | 0x0B
	}, {
		.mode = FMRADIO_HP,
		.reg  = WM8994_LEFT_OUTPUT_VOLUME,	/* 1Ch */
		.mask = WM8994_HPOUT1L_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x3C
	}, {
		.mode = FMRADIO_HP,
		.reg  = WM8994_RIGHT_OUTPUT_VOLUME,	/* 1Dh */
		.mask = WM8994_HPOUT1R_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x3C
	}, {
		.mode = FMRADIO_HP,
		.reg  = WM8994_LEFT_OPGA_VOLUME,	/* 20h */
		.mask = WM8994_MIXOUTL_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, {
		.mode = FMRADIO_HP,
		.reg  = WM8994_RIGHT_OPGA_VOLUME,	/* 21h */
		.mask = WM8994_MIXOUTR_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, {
		.mode = FMRADIO_HP,
		.reg  = WM8994_AIF1_DAC1_LEFT_VOLUME,	/* 402h */
		.mask = WM8994_AIF1DAC1L_VOL_MASK,
		.gain = WM8994_AIF1DAC1_VU | 0xA8
	}, {
		.mode = FMRADIO_HP,
		.reg  = WM8994_AIF1_DAC1_RIGHT_VOLUME,	/* 403h */
		.mask = WM8994_AIF1DAC1R_VOL_MASK,
		.gain = WM8994_AIF1DAC1_VU | 0xA8
	}, { /* Speaker */
		.mode = FMRADIO_SPK,
		.reg  = WM8994_LEFT_LINE_INPUT_3_4_VOLUME,	/* 19h */
		.mask = WM8994_IN2L_VOL_MASK,
		.gain = WM8994_IN2L_VU | 0x0F
	}, {
		.mode = FMRADIO_SPK,
		.reg  = WM8994_RIGHT_LINE_INPUT_3_4_VOLUME,	/* 1Bh */
		.mask = WM8994_IN2R_VOL_MASK,
		.gain = WM8994_IN2R_VU | 0x0F
	}, {
		.mode = FMRADIO_SPK,
		.reg  = WM8994_SPKMIXL_ATTENUATION,	/* 22h */
		.mask = WM8994_SPKMIXL_VOL_MASK,
		.gain = 0x0
	}, {
		.mode = FMRADIO_SPK,
		.reg  = WM8994_SPKMIXR_ATTENUATION,	/* 23h */
		.mask = WM8994_SPKMIXR_VOL_MASK,
		.gain = 0x0
	}, {
		.mode = FMRADIO_SPK,
		.reg  = WM8994_SPEAKER_VOLUME_LEFT,	/* 26h */
		.mask = WM8994_SPKOUTL_VOL_MASK,
		.gain = WM8994_SPKOUT_VU | 0x3E
	}, {
		.mode = FMRADIO_SPK,
		.reg  = WM8994_SPEAKER_VOLUME_RIGHT,	/* 27h */
		.mask = WM8994_SPKOUTR_VOL_MASK,
		.gain = WM8994_SPKOUT_VU | 0x3E
	}, {
		.mode = FMRADIO_SPK,
		.reg  = WM8994_CLASSD,			/* 25h */
		.mask = WM8994_SPKOUTL_BOOST_MASK,
		.gain = 0x6 << WM8994_SPKOUTL_BOOST_SHIFT
	}, { /* SPK_HP */
		.mode = FMRADIO_SPK_HP,
		.reg  = WM8994_LEFT_LINE_INPUT_3_4_VOLUME,	/* 19h */
		.mask = WM8994_IN2L_VOL_MASK,
		.gain = WM8994_IN2L_VU | 0x0F
	}, {
		.mode = FMRADIO_SPK_HP,
		.reg  = WM8994_RIGHT_LINE_INPUT_3_4_VOLUME,	/* 1Bh */
		.mask = WM8994_IN2R_VOL_MASK,
		.gain = WM8994_IN2R_VU | 0x0F
	}, {
		.mode = FMRADIO_SPK,
		.reg  = WM8994_SPKMIXL_ATTENUATION,	/* 22h */
		.mask = WM8994_SPKMIXL_VOL_MASK,
		.gain = 0x0
	}, {
		.mode = FMRADIO_SPK,
		.reg  = WM8994_SPKMIXR_ATTENUATION,	/* 23h */
		.mask = WM8994_SPKMIXR_VOL_MASK,
		.gain = 0x0
	}, {
		.mode = FMRADIO_SPK_HP,
		.reg  = WM8994_LEFT_OPGA_VOLUME,	/* 20h */
		.mask = WM8994_MIXOUTL_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, {
		.mode = FMRADIO_SPK_HP,
		.reg  = WM8994_RIGHT_OPGA_VOLUME,	/* 21h */
		.mask = WM8994_MIXOUTR_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, {
		.mode = FMRADIO_SPK_HP,
		.reg  = WM8994_LEFT_OUTPUT_VOLUME,	/* 1Ch */
		.mask = WM8994_HPOUT1L_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x3C
	}, {
		.mode = FMRADIO_SPK_HP,
		.reg  = WM8994_RIGHT_OUTPUT_VOLUME,	/* 1Dh */
		.mask = WM8994_HPOUT1R_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x3C
	}, {
		.mode = FMRADIO_SPK_HP,
		.reg  = WM8994_AIF1_DAC1_LEFT_VOLUME,	/* 402h */
		.mask = WM8994_AIF1DAC1L_VOL_MASK,
		.gain = WM8994_AIF1DAC1_VU | 0x70
	}, {
		.mode = FMRADIO_SPK_HP,
		.reg  = WM8994_AIF1_DAC1_RIGHT_VOLUME,	/* 403h */
		.mask = WM8994_AIF1DAC1R_VOL_MASK,
		.gain = WM8994_AIF1DAC1_VU | 0x70
	}, {
		.mode = FMRADIO_SPK_HP,
		.reg  = WM8994_CLASSD,			/* 25h */
		.mask = WM8994_SPKOUTL_BOOST_MASK,
		.gain = 0x6 << WM8994_SPKOUTL_BOOST_SHIFT
	}, {
		.mode = FMRADIO_SPK_HP,
		.reg  = WM8994_SPEAKER_VOLUME_LEFT,	/* 26h */
		.mask = WM8994_SPKOUTL_VOL_MASK,
		.gain = WM8994_SPKOUT_VU | 0x3E
	}, {
		.mode = FMRADIO_SPK_HP,
		.reg  = WM8994_SPEAKER_VOLUME_RIGHT,	/* 27h */
		.mask = WM8994_SPKOUTR_VOL_MASK,
		.gain = WM8994_SPKOUT_VU | 0x3E
	},

};
static void wait_for_dc_servo(struct snd_soc_codec *codec, unsigned int op)
{
	unsigned int reg;
	int count = 0;
	unsigned int val, start;

	val = op | WM8994_DCS_ENA_CHAN_0 | WM8994_DCS_ENA_CHAN_1;

	/* Trigger the command */
	snd_soc_write(codec, WM8994_DC_SERVO_1, val);

	start = jiffies;
	pr_debug("%s: Waiting for DC servo...\n", __func__);

	do {
		count++;
		msleep(1);
		reg = snd_soc_read(codec, WM8994_DC_SERVO_1);
		pr_debug("%s: DC servo: %x\n", __func__, reg);
	} while (reg & op && count < 400);

	pr_debug("%s: DC servo took %dms\n", __func__,
		jiffies_to_msecs(jiffies - start));

	if (reg & op)
		pr_err("%s: Timed out waiting for DC Servo\n", __func__);
}

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
	DEBUG_LOG("enable = [%d]", enable);

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
		wm8994_write(codec, WM8994_SPEAKER_MIXER, val);
		break;

	case HP:
	case HP_NO_MIC:
		/* mute DAC1 */
		wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, 0x02C0);
		wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, 0x02C0);

		/* disble the Timeslot0 to DAC1 */
		val = wm8994_read(codec, WM8994_DAC1_LEFT_MIXER_ROUTING);
		val &= ~WM8994_AIF1DAC1L_TO_DAC1L_MASK;
		wm8994_write(codec, WM8994_DAC1_LEFT_MIXER_ROUTING, val);

		val = wm8994_read(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING);
		val &= ~WM8994_AIF1DAC1R_TO_DAC1R_MASK;
		wm8994_write(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING, val);
		
		/* disable MIXOUT */
		wm8994_write(codec, WM8994_OUTPUT_MIXER_1, 0x0);
		wm8994_write(codec, WM8994_OUTPUT_MIXER_2, 0x0);

		break;

	case BT:
		val = wm8994_read(codec, WM8994_AIF1_DAC1_FILTERS_1);
		val &= ~(WM8994_AIF1DAC1_MUTE_MASK | WM8994_AIF1DAC1_MONO_MASK);
		val |= (WM8994_AIF1DAC1_MUTE);
		wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, val);
		break;

	case SPK_HP:
		val &= ~(WM8994_HPOUT1L_ENA_MASK | WM8994_HPOUT1R_ENA_MASK |
				WM8994_SPKOUTL_ENA_MASK);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

		/* Disable DAC1L to HPOUT1L path */
		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
		val &= ~(WM8994_DAC1L_TO_HPOUT1L_MASK |
				WM8994_DAC1L_TO_MIXOUTL_MASK);
		wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val);

		/* Disable DAC1R to HPOUT1R path */
		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
		val &= ~(WM8994_DAC1R_TO_HPOUT1R_MASK |
				WM8994_DAC1R_TO_MIXOUTR_MASK);
		wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);

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
		wm8994_write(codec, WM8994_SPEAKER_MIXER, val);
		break;

	default:
		DEBUG_LOG_ERR("Path[%d] is not invaild!\n", path);
		return;
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
		if(!(wm8994->codec_state & CALL_ACTIVE)) {
			val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_2);
			val &= ~(WM8994_IN1L_ENA_MASK | WM8994_MIXINL_ENA_MASK |
                                WM8994_IN2R_ENA_MASK | WM8994_MIXINR_ENA_MASK);
			wm8994_write(codec, WM8994_POWER_MANAGEMENT_2, val);

			/* Mute IN1L PGA, update volume */
			val = wm8994_read(codec,
					WM8994_LEFT_LINE_INPUT_1_2_VOLUME);
			val &= ~(WM8994_IN1L_MUTE_MASK);
			val |= (WM8994_IN1L_VU | WM8994_IN1L_MUTE);
			wm8994_write(codec, WM8994_LEFT_LINE_INPUT_1_2_VOLUME,
					val);

			/* Mute IN2R PGA, update volume */
			val = wm8994_read(codec,
					WM8994_RIGHT_LINE_INPUT_3_4_VOLUME);
			val &= ~(WM8994_IN2R_MUTE_MASK);
			val |= (WM8994_IN2R_VU | WM8994_IN2R_MUTE);
			wm8994_write(codec, WM8994_RIGHT_LINE_INPUT_3_4_VOLUME,
					val);

			/*Mute the PGA */
			val = wm8994_read(codec, WM8994_INPUT_MIXER_3);
			val &= ~(WM8994_IN1L_TO_MIXINL_MASK |
				WM8994_IN1L_MIXINL_VOL_MASK |
				WM8994_MIXOUTL_MIXINL_VOL_MASK);
			wm8994_write(codec, WM8994_INPUT_MIXER_3, val);

			val = wm8994_read(codec, WM8994_INPUT_MIXER_4);
			val &= ~(WM8994_IN2R_TO_MIXINR_MASK |
				WM8994_IN2R_MIXINR_VOL_MASK |
				WM8994_MIXOUTR_MIXINR_VOL_MASK);
			wm8994_write(codec, WM8994_INPUT_MIXER_4, val);
                        
			/* Disconnect IN1LN,IN1LP,IN2RN,IN2RP to the inputs */
			val = wm8994_read(codec, WM8994_INPUT_MIXER_2);
			val &= (WM8994_IN1LN_TO_IN1L_MASK | WM8994_IN1LP_TO_IN1L_MASK |
                                WM8994_IN2RN_TO_IN2R_MASK | WM8994_IN2RP_TO_IN2R_MASK);
			wm8994_write(codec, WM8994_INPUT_MIXER_2, val);

			/* Digital Paths */
			val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_4);
			val &= ~(WM8994_ADCL_ENA_MASK | WM8994_AIF1ADC1L_ENA_MASK |
                                WM8994_ADCR_ENA_MASK | WM8994_AIF1ADC1R_ENA_MASK);
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

		val = wm8994_read(codec, WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING);
		val &= ~(WM8994_ADC1R_TO_AIF1ADC1R);
		wm8994_write(codec, WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING, val);                
		break;

	case SUB:
		DEBUG_LOG("Disbaling SUB Mic path..\n");

		if(!(wm8994->codec_state & CALL_ACTIVE)) {

			val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_2);
			val &= ~(WM8994_IN1R_ENA_MASK | WM8994_MIXINR_ENA_MASK);
			wm8994_write(codec, WM8994_POWER_MANAGEMENT_2, val);

			/* Disable volume,unmute Right Line */
			val = wm8994_read(codec,
					WM8994_RIGHT_LINE_INPUT_1_2_VOLUME);
			val &= ~WM8994_IN1R_MUTE_MASK;	/* Unmute IN1R */
			val |= (WM8994_IN1R_VU | WM8994_IN1R_MUTE);
			wm8994_write(codec, WM8994_RIGHT_LINE_INPUT_1_2_VOLUME,
				     val);

			/* Mute right pga, set volume */
			val = wm8994_read(codec, WM8994_INPUT_MIXER_4);
			val &= ~(WM8994_IN1R_TO_MIXINR_MASK |
			      WM8994_IN1R_MIXINR_VOL_MASK |
			      WM8994_MIXOUTR_MIXINR_VOL_MASK);
			wm8994_write(codec, WM8994_INPUT_MIXER_4, val);

			/* Disconnect in1rn to inr1 and in1rp to inrp */
			val = wm8994_read(codec, WM8994_INPUT_MIXER_2);
			val &= ~(WM8994_IN1RN_TO_IN1R_MASK | WM8994_IN1RP_TO_IN1R_MASK);
			wm8994_write(codec, WM8994_INPUT_MIXER_2, val);

			/* Digital Paths */
			/* Disable right ADC and time slot */
			val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_4);
			val &= ~(WM8994_ADCR_ENA_MASK | WM8994_AIF1ADC1R_ENA_MASK);
			wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, val);
		}else {
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
		DEBUG_LOG("Disbaling BT Mic path..\n");
		if(!(wm8994->codec_state & CALL_ACTIVE)) {
			
			val = wm8994_read(codec, WM8994_AIF1_ADC1_LEFT_MIXER_ROUTING);
			val &= ~(WM8994_AIF2DACL_TO_AIF1ADC1L_MASK |
				WM8994_ADC1L_TO_AIF1ADC1L_MASK);
			wm8994_write(codec, WM8994_AIF1_ADC1_LEFT_MIXER_ROUTING, val);

			val = wm8994_read(codec, WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING);
			val &= ~(WM8994_AIF2DACR_TO_AIF1ADC1R_MASK |
				WM8994_ADC1R_TO_AIF1ADC1R_MASK);
			wm8994_write(codec, WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING, val);

			val = wm8994_read(codec, WM8994_AIF2_DAC_FILTERS_1);
			val &= ~(WM8994_AIF2DAC_MUTE_MASK);
			val |= (WM8994_AIF2DAC_MUTE);
			wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_1, val);
		}
		break;

	case MIC_OFF:
		DEBUG_LOG("Mic is already OFF!\n");
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
	wm8994_write(codec, WM8994_AIF2_CONTROL_1, 0x4010);

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

	if(!(wm8994->codec_state & CALL_ACTIVE)) {

		wm8994_write(codec, WM8994_ANTIPOP_2, 0x68);

		/* Enable high pass filter to control bounce on startup */
		val = wm8994_read(codec, WM8994_AIF1_ADC1_FILTERS);
		val &= ~(WM8994_AIF1ADC1L_HPF_MASK | WM8994_AIF1ADC1R_HPF_MASK);
		val |= (WM8994_AIF1ADC1R_HPF);
		wm8994_write(codec, WM8994_AIF1_ADC1_FILTERS, val);

		/* Enable mic bias, vmid, bias generator */
		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
		val &= ~(WM8994_BIAS_ENA_MASK | WM8994_VMID_SEL_MASK);
		val |= (WM8994_BIAS_ENA | WM8994_VMID_SEL_NORMAL);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

		val = wm8994_read(codec, WM8994_INPUT_MIXER_1);
		val &= ~(WM8994_INPUTS_CLAMP_MASK);
		val |= (WM8994_INPUTS_CLAMP);
		wm8994_write(codec, WM8994_INPUT_MIXER_1, val);

		val = (WM8994_MIXINR_ENA | WM8994_IN1R_ENA);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_2, val);


		val = (WM8994_IN1RN_TO_IN1R | WM8994_IN1RP_TO_IN1R);
		wm8994_write(codec, WM8994_INPUT_MIXER_2, val);

		val = wm8994_read(codec, WM8994_RIGHT_LINE_INPUT_1_2_VOLUME);
		val &= ~(WM8994_IN1R_MUTE_MASK);
		wm8994_write(codec, WM8994_RIGHT_LINE_INPUT_1_2_VOLUME, val);

		val = wm8994_read(codec, WM8994_INPUT_MIXER_4);
		val &= ~(WM8994_IN1R_TO_MIXINR_MASK);
		val |= (WM8994_IN1R_TO_MIXINR);
		wm8994_write(codec, WM8994_INPUT_MIXER_4 , val);

		val = wm8994_read(codec, WM8994_INPUT_MIXER_1);
		val &= ~(WM8994_INPUTS_CLAMP_MASK);
		wm8994_write(codec, WM8994_INPUT_MIXER_1, val);

		val = wm8994_read(codec, WM8994_AIF1_ADC1_RIGHT_VOLUME);
		val |= (WM8994_AIF1ADC1_VU);
		wm8994_write(codec, WM8994_AIF1_ADC1_RIGHT_VOLUME, val);

		val = wm8994_read(codec, WM8994_AIF1_ADC1_FILTERS);
		val &= ~(WM8994_AIF1ADC1L_HPF_MASK | WM8994_AIF1ADC1R_HPF_MASK);
		val |= (WM8994_AIF1ADC1R_HPF | 0x2000);
		wm8994_write(codec, WM8994_AIF1_ADC1_FILTERS, val);

		val = wm8994_read(codec, WM8994_AIF1_MASTER_SLAVE);
		val |= (WM8994_AIF1_MSTR | WM8994_AIF1_CLK_FRC | WM8994_AIF1_LRCLK_FRC);
		wm8994_write(codec, WM8994_AIF1_MASTER_SLAVE, val);

		wm8994_write(codec, WM8994_GPIO_1, 0xA101);

		/* Mixing left channel output to right channel */
		val = wm8994_read(codec, WM8994_AIF1_CONTROL_1);
		val &= ~(WM8994_AIF1ADCL_SRC_MASK | WM8994_AIF1ADCR_SRC_MASK);
		val |= (WM8994_AIF1ADCL_SRC | WM8994_AIF1ADCR_SRC);
		wm8994_write(codec, WM8994_AIF1_CONTROL_1, val);

		/* Digital Paths  */
		/* Enable right ADC and time slot */
		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_4);
		val &= ~(WM8994_ADCR_ENA_MASK | WM8994_AIF1ADC1R_ENA_MASK);
		val |= (WM8994_AIF1ADC1R_ENA | WM8994_ADCR_ENA);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, val);


		/* ADC Right mixer routing */
		val = wm8994_read(codec, WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING);
		val &= ~(WM8994_ADC1R_TO_AIF1ADC1R_MASK);
		val |= WM8994_ADC1R_TO_AIF1ADC1R;
		wm8994_write(codec, WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING, val);

		val = wm8994_read(codec, WM8994_SPEAKER_MIXER);
		val &= ~WM8994_MIXINL_TO_SPKMIXL_MASK;
		wm8994_write(codec, WM8994_SPEAKER_MIXER, val);

		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
		val &= ~WM8994_MIXINL_TO_MIXOUTL_MASK;
		wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val);

		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
		val &= ~WM8994_MIXINR_TO_MIXOUTR_MASK;
		wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);

		val = wm8994_read(codec, WM8994_DAC2_LEFT_MIXER_ROUTING);
		val &= ~(WM8994_ADC1_TO_DAC2L_MASK);
		wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING, val);

		val = wm8994_read(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING);
		val &= ~(WM8994_ADC1_TO_DAC2R_MASK);
		wm8994_write(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING, val);

		if (wm8994->input_source == RECOGNITION)
			wm8994_set_codec_gain(codec, RECORDING_MODE, RECORDING_REC_HP);
		else if (wm8994->input_source == CAMCORDER)
			wm8994_set_codec_gain(codec, RECORDING_MODE, RECORDING_CAM_HP);
		else if (wm8994->input_source == VOIP_INPUT)
			wm8994_set_codec_gain(codec, RECORDING_MODE, RECORDING_VOIP_HP);
		else
			wm8994_set_codec_gain(codec, RECORDING_MODE, RECORDING_HP);
	} else {
		val = wm8994_read(codec, WM8994_AIF1_MASTER_SLAVE);
		val |= (WM8994_AIF1_MSTR | WM8994_AIF1_CLK_FRC | WM8994_AIF1_LRCLK_FRC);
		wm8994_write(codec, WM8994_AIF1_MASTER_SLAVE, val);

		wm8994_write(codec, WM8994_GPIO_1, 0xA101);		

		val = wm8994_read(codec, WM8994_AIF1_CONTROL_1);
		val &= ~(WM8994_AIF1ADCL_SRC_MASK | WM8994_AIF1ADCR_SRC_MASK);
		val |= (WM8994_AIF1ADCL_SRC | WM8994_AIF1ADCR_SRC);
		wm8994_write(codec, WM8994_AIF1_CONTROL_1, val);

		/* Digital Paths  */
		/* Enable right ADC and time slot */
		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_4);
		val &= ~(WM8994_ADCR_ENA_MASK | WM8994_AIF1ADC1R_ENA_MASK);
		val |= (WM8994_AIF1ADC1R_ENA | WM8994_ADCR_ENA);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, val);

		/* ADC Right mixer routing */
		val = wm8994_read(codec, WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING);
		val &= ~(WM8994_ADC1R_TO_AIF1ADC1R_MASK);
		val |= WM8994_ADC1R_TO_AIF1ADC1R;
		wm8994_write(codec, WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING, val);
	}
}

void wm8994_record_main_mic(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;

	u16 val;

	DEBUG_LOG("Recording through Main Mic\n");

	audio_ctrl_mic_bias_gpio(wm8994->pdata, 1);

	if(!(wm8994->codec_state & CALL_ACTIVE)) {

		/* Main mic volume issue fix: requested H/W */
		wm8994_write(codec, WM8994_ANTIPOP_2, 0x68);

		/* High pass filter to control bounce on enable */
		val = wm8994_read(codec, WM8994_AIF1_ADC1_FILTERS);
		val &= ~(WM8994_AIF1ADC1L_HPF_MASK | WM8994_AIF1ADC1R_HPF_MASK);
		val |= (WM8994_AIF1ADC1L_HPF);
		wm8994_write(codec, WM8994_AIF1_ADC1_FILTERS, val);

		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
		val &= ~(WM8994_BIAS_ENA_MASK | WM8994_VMID_SEL_MASK);
		val |= (WM8994_BIAS_ENA | WM8994_VMID_SEL_NORMAL);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

		val = wm8994_read(codec, WM8994_INPUT_MIXER_1);
		val &= ~(WM8994_INPUTS_CLAMP_MASK);
		val |= (WM8994_INPUTS_CLAMP);
		wm8994_write(codec, WM8994_INPUT_MIXER_1, val);

		val = (WM8994_MIXINL_ENA | WM8994_IN1L_ENA);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_2, val);

		val = (WM8994_IN1LP_TO_IN1L | WM8994_IN1LN_TO_IN1L);
		wm8994_write(codec, WM8994_INPUT_MIXER_2, val);


		val = wm8994_read(codec, WM8994_LEFT_LINE_INPUT_1_2_VOLUME);
		val &= ~(WM8994_IN1L_MUTE_MASK);
		val |= WM8994_IN1L_VU;
		wm8994_write(codec, WM8994_LEFT_LINE_INPUT_1_2_VOLUME, val);

		val = wm8994_read(codec, WM8994_INPUT_MIXER_3);
		val &= ~(WM8994_IN1L_TO_MIXINL_MASK);
		val |= (WM8994_IN1L_TO_MIXINL);
		wm8994_write(codec, WM8994_INPUT_MIXER_3, val);

		val = wm8994_read(codec, WM8994_INPUT_MIXER_1);
		val &= ~(WM8994_INPUTS_CLAMP_MASK);
		wm8994_write(codec, WM8994_INPUT_MIXER_1, val);


		val = wm8994_read(codec, WM8994_AIF1_ADC1_LEFT_VOLUME);
		val |= (WM8994_AIF1ADC1_VU);
		wm8994_write(codec, WM8994_AIF1_ADC1_LEFT_VOLUME, val);

		val = wm8994_read(codec, WM8994_AIF1_ADC1_FILTERS);
		val &= ~(WM8994_AIF1ADC1L_HPF_MASK | WM8994_AIF1ADC1R_HPF_MASK);
		val |= (WM8994_AIF1ADC1L_HPF | 0x2000);
		wm8994_write(codec, WM8994_AIF1_ADC1_FILTERS, val);

		val = wm8994_read(codec, WM8994_AIF1_MASTER_SLAVE);
		val |= (WM8994_AIF1_MSTR | WM8994_AIF1_CLK_FRC | WM8994_AIF1_LRCLK_FRC);
		wm8994_write(codec, WM8994_AIF1_MASTER_SLAVE, val);

		wm8994_write(codec, WM8994_GPIO_1, 0xA101);

		val = wm8994_read(codec, WM8994_AIF1_CONTROL_1);
		val &= ~(WM8994_AIF1ADCL_SRC_MASK | WM8994_AIF1ADCR_SRC_MASK);
		val |= (WM8994_AIF1ADCR_SRC);
		wm8994_write(codec, WM8994_AIF1_CONTROL_1, val);

		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_4);
		val &= ~(WM8994_ADCL_ENA_MASK | WM8994_AIF1ADC1L_ENA_MASK);
		val |= (WM8994_AIF1ADC1L_ENA | WM8994_ADCL_ENA);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, val);

		/* Enable timeslots */
		val = wm8994_read(codec, WM8994_AIF1_ADC1_LEFT_MIXER_ROUTING);
		val |= WM8994_ADC1L_TO_AIF1ADC1L;
		wm8994_write(codec, WM8994_AIF1_ADC1_LEFT_MIXER_ROUTING, val);

		val = wm8994_read(codec, WM8994_SPEAKER_MIXER);
		val &= ~WM8994_MIXINL_TO_SPKMIXL_MASK;
		wm8994_write(codec, WM8994_SPEAKER_MIXER, val);

		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
		val &= ~WM8994_MIXINL_TO_MIXOUTL_MASK;
		wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val);

		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
		val &= ~WM8994_MIXINR_TO_MIXOUTR_MASK;
		wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);

		val = wm8994_read(codec, WM8994_DAC2_LEFT_MIXER_ROUTING);
		val &= ~(WM8994_ADC1_TO_DAC2L_MASK);
		wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING, val);

		val = wm8994_read(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING);
		val &= ~(WM8994_ADC1_TO_DAC2R_MASK);
		wm8994_write(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING, val);

		if (wm8994->input_source == RECOGNITION)
			wm8994_set_codec_gain(codec, RECORDING_MODE,
					RECORDING_REC_MAIN);
		else if (wm8994->input_source == CAMCORDER)
			wm8994_set_codec_gain(codec, RECORDING_MODE,
					RECORDING_CAM_MAIN);
		else if (wm8994->input_source == VOIP_INPUT)
			wm8994_set_codec_gain(codec, RECORDING_MODE,
					RECORDING_VOIP_MAIN);
		else
			wm8994_set_codec_gain(codec, RECORDING_MODE, RECORDING_MAIN);
	} else {
		val = wm8994_read(codec, WM8994_AIF1_MASTER_SLAVE);
		val |= (WM8994_AIF1_MSTR | WM8994_AIF1_CLK_FRC | WM8994_AIF1_LRCLK_FRC);
		wm8994_write(codec, WM8994_AIF1_MASTER_SLAVE, val);

		wm8994_write(codec, WM8994_GPIO_1, 0xA101);

		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_4);
		val &= ~(WM8994_ADCL_ENA_MASK | WM8994_AIF1ADC1L_ENA_MASK);
		val |= (WM8994_AIF1ADC1L_ENA | WM8994_ADCL_ENA);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, val);

		/* Enable timeslots */
		val = wm8994_read(codec, WM8994_AIF1_ADC1_LEFT_MIXER_ROUTING);
		val |= WM8994_ADC1L_TO_AIF1ADC1L;
		wm8994_write(codec, WM8994_AIF1_ADC1_LEFT_MIXER_ROUTING, val);

	}
}

void wm8994_record_2nd_main_mic(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;

	u16 val;

	DEBUG_LOG("Recording through 2nd Main Mic\n");

	audio_ctrl_mic_bias_gpio(wm8994->pdata, 1);

	if(!(wm8994->codec_state & CALL_ACTIVE)) {

		wm8994_write(codec, WM8994_ANTIPOP_2, 0x68);

		/* Enable high pass filter to control bounce on startup */
		val = wm8994_read(codec, WM8994_AIF1_ADC1_FILTERS);
		val &= ~(WM8994_AIF1ADC1L_HPF_MASK | WM8994_AIF1ADC1R_HPF_MASK);
		val |= (WM8994_AIF1ADC1R_HPF);
		wm8994_write(codec, WM8994_AIF1_ADC1_FILTERS, val);

		/* Enable mic bias, vmid, bias generator */
		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
		val &= ~(WM8994_BIAS_ENA_MASK | WM8994_VMID_SEL_MASK);
		val |= (WM8994_BIAS_ENA | WM8994_VMID_SEL_NORMAL);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

		val = wm8994_read(codec, WM8994_INPUT_MIXER_1);
		val &= ~(WM8994_INPUTS_CLAMP_MASK);
		val |= (WM8994_INPUTS_CLAMP);
		wm8994_write(codec, WM8994_INPUT_MIXER_1, val);

		val = (WM8994_MIXINR_ENA | WM8994_IN2R_ENA);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_2, val);

		val = (WM8994_IN2RP_TO_IN2R | WM8994_IN2RN_TO_IN2R);
		wm8994_write(codec, WM8994_INPUT_MIXER_2, val);


		val = wm8994_read(codec, WM8994_RIGHT_LINE_INPUT_3_4_VOLUME);
		val &= ~(WM8994_IN2R_MUTE_MASK);
		val |= WM8994_IN2R_VU;
		wm8994_write(codec, WM8994_RIGHT_LINE_INPUT_3_4_VOLUME, val);

		val = wm8994_read(codec, WM8994_INPUT_MIXER_4);
		val &= ~(WM8994_IN2R_TO_MIXINR_MASK);
		val |= (WM8994_IN2R_TO_MIXINR);
		wm8994_write(codec, WM8994_INPUT_MIXER_4, val);

		val = wm8994_read(codec, WM8994_INPUT_MIXER_1);
		val &= ~(WM8994_INPUTS_CLAMP_MASK);
		wm8994_write(codec, WM8994_INPUT_MIXER_1, val);

		val = wm8994_read(codec, WM8994_AIF1_ADC1_RIGHT_VOLUME);
		val |= (WM8994_AIF1ADC1_VU);
		wm8994_write(codec, WM8994_AIF1_ADC1_RIGHT_VOLUME, val);

		val = wm8994_read(codec, WM8994_AIF1_ADC1_FILTERS);
		val &= ~(WM8994_AIF1ADC1L_HPF_MASK | WM8994_AIF1ADC1R_HPF_MASK);
		val |= (WM8994_AIF1ADC1R_HPF | 0x2000);
		wm8994_write(codec, WM8994_AIF1_ADC1_FILTERS, val);

		val = wm8994_read(codec, WM8994_AIF1_MASTER_SLAVE);
		val |= (WM8994_AIF1_MSTR | WM8994_AIF1_CLK_FRC | WM8994_AIF1_LRCLK_FRC);
		wm8994_write(codec, WM8994_AIF1_MASTER_SLAVE, val);

		wm8994_write(codec, WM8994_GPIO_1, 0xA101);

		/* Mixing left channel output to right channel */
		val = wm8994_read(codec, WM8994_AIF1_CONTROL_1);
		val &= ~(WM8994_AIF1ADCL_SRC_MASK | WM8994_AIF1ADCR_SRC_MASK);
		val |= (WM8994_AIF1ADCL_SRC | WM8994_AIF1ADCR_SRC);
		wm8994_write(codec, WM8994_AIF1_CONTROL_1, val);

		/* Digital Paths  */
		/* Enable right ADC and time slot */
		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_4);
		val &= ~(WM8994_ADCR_ENA_MASK | WM8994_AIF1ADC1R_ENA_MASK);
		val |= (WM8994_AIF1ADC1R_ENA | WM8994_ADCR_ENA);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, val);


		/* ADC Right mixer routing */
		val = wm8994_read(codec, WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING);
		val &= ~(WM8994_ADC1R_TO_AIF1ADC1R_MASK);
		val |= WM8994_ADC1R_TO_AIF1ADC1R;
		wm8994_write(codec, WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING, val);

		val = wm8994_read(codec, WM8994_SPEAKER_MIXER);
		val &= ~WM8994_MIXINL_TO_SPKMIXL_MASK;
		wm8994_write(codec, WM8994_SPEAKER_MIXER, val);

		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
		val &= ~WM8994_MIXINL_TO_MIXOUTL_MASK;
		wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val);

		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
		val &= ~WM8994_MIXINR_TO_MIXOUTR_MASK;
		wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);

		val = wm8994_read(codec, WM8994_DAC2_LEFT_MIXER_ROUTING);
		val &= ~(WM8994_ADC1_TO_DAC2L_MASK);
		wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING, val);

		val = wm8994_read(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING);
		val &= ~(WM8994_ADC1_TO_DAC2R_MASK);
		wm8994_write(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING, val);

		if (wm8994->input_source == RECOGNITION)
			wm8994_set_codec_gain(codec, RECORDING_MODE,
					RECORDING_REC_MAIN2);
		else if (wm8994->input_source == CAMCORDER)
			wm8994_set_codec_gain(codec, RECORDING_MODE,
					RECORDING_CAM_MAIN2);
		else if (wm8994->input_source == VOIP_INPUT)
			wm8994_set_codec_gain(codec, RECORDING_MODE,
					RECORDING_VOIP_MAIN2);
		else
			wm8994_set_codec_gain(codec, RECORDING_MODE, RECORDING_MAIN2);
	} else {
		val = wm8994_read(codec, WM8994_AIF1_MASTER_SLAVE);
		val |= (WM8994_AIF1_MSTR | WM8994_AIF1_CLK_FRC | WM8994_AIF1_LRCLK_FRC);
		wm8994_write(codec, WM8994_AIF1_MASTER_SLAVE, val);

		wm8994_write(codec, WM8994_GPIO_1, 0xA101);

		val = wm8994_read(codec, WM8994_AIF1_CONTROL_1);
		val &= ~(WM8994_AIF1ADCL_SRC_MASK | WM8994_AIF1ADCR_SRC_MASK);
		val |= (WM8994_AIF1ADCL_SRC | WM8994_AIF1ADCR_SRC);
		wm8994_write(codec, WM8994_AIF1_CONTROL_1, val);

		/* Digital Paths  */
		/* Enable right ADC and time slot */
		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_4);
		val &= ~(WM8994_ADCR_ENA_MASK | WM8994_AIF1ADC1R_ENA_MASK);
                val |= (WM8994_AIF1ADC1R_ENA | WM8994_ADCR_ENA);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, val);

		/* ADC Right mixer routing */
		val = wm8994_read(codec, WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING);
		val &= ~(WM8994_ADC1R_TO_AIF1ADC1R_MASK);
		val |= WM8994_ADC1R_TO_AIF1ADC1R;
		wm8994_write(codec, WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING, val);
	}
}

void wm8994_record_bluetooth(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;
	u16 val;

	DEBUG_LOG("BT Record Path for Voice Command\n");

	if(!(wm8994->codec_state & CALL_ACTIVE)) {

		wm8994_set_bluetooth_common_setting(codec);

		val = wm8994_read(codec, WM8994_DAC2_LEFT_MIXER_ROUTING);
		val &= ~(WM8994_ADC1_TO_DAC2L_MASK);
		wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING, val);

		val = wm8994_read(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING);
		val &= ~(WM8994_ADC1_TO_DAC2R_MASK);
		wm8994_write(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING, val);

		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
		val &= ~(WM8994_BIAS_ENA_MASK | WM8994_VMID_SEL_MASK);
		val |= (WM8994_BIAS_ENA | WM8994_VMID_SEL_NORMAL);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

		wm8994_write(codec, WM8994_POWER_MANAGEMENT_2, 0x0000);

		wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, 0x0000);

		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_4);
		val &= ~(WM8994_AIF1ADC1L_ENA_MASK | WM8994_AIF1ADC1R_ENA_MASK);
		val |= (WM8994_AIF1ADC1L_ENA | WM8994_AIF1ADC1R_ENA);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_4 , val);

		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_5);
		val &= ~(WM8994_AIF2DACL_ENA_MASK | WM8994_AIF2DACR_ENA_MASK);
		val |= (WM8994_AIF2DACL_ENA | WM8994_AIF2DACR_ENA);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_5, val);

		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_6);
		val &= ~(WM8994_AIF3_ADCDAT_SRC_MASK | WM8994_AIF2_DACDAT_SRC_MASK);
		val |= (0x1 << WM8994_AIF3_ADCDAT_SRC_SHIFT | WM8994_AIF2_DACDAT_SRC);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_6, val);

		val = wm8994_read(codec, WM8994_AIF2_DAC_FILTERS_1);
		val &= ~(WM8994_AIF2DAC_MUTE_MASK);
		wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_1, val);

		val = wm8994_read(codec, WM8994_AIF1_ADC1_LEFT_MIXER_ROUTING);
		val &= ~(WM8994_AIF2DACL_TO_AIF1ADC1L_MASK);
		val |= (WM8994_AIF2DACL_TO_AIF1ADC1L);
		wm8994_write(codec, WM8994_AIF1_ADC1_LEFT_MIXER_ROUTING, val);

		val = wm8994_read(codec, WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING);
		val &= ~(WM8994_AIF2DACR_TO_AIF1ADC1R_MASK);
		val |= (WM8994_AIF2DACR_TO_AIF1ADC1R);
		wm8994_write(codec, WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING, val);

		wm8994_write(codec, WM8994_AIF2_CLOCKING_1, 0x0019);

		wm8994_write(codec, WM8994_OVERSAMPLING, 0X0000);

		wm8994_write(codec, WM8994_GPIO_8, WM8994_GP8_DIR | WM8994_GP8_DB);
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

	val = wm8994_read(codec, WM8994_LEFT_OPGA_VOLUME);
	val &= ~(WM8994_MIXOUTL_MUTE_N_MASK);
	val |= (WM8994_MIXOUTL_MUTE_N);
	wm8994_write(codec, WM8994_LEFT_OPGA_VOLUME, val);

	val = wm8994_read(codec, WM8994_RIGHT_OPGA_VOLUME);
	val &= ~(WM8994_MIXOUTR_MUTE_N_MASK);
	val |= (WM8994_MIXOUTR_MUTE_N);
	wm8994_write(codec, WM8994_RIGHT_OPGA_VOLUME, val);

	val = wm8994_read(codec, WM8994_HPOUT2_VOLUME);
	val &= ~(WM8994_HPOUT2_MUTE_MASK);
	wm8994_write(codec, WM8994_HPOUT2_VOLUME, val);

	/* Unmute DAC1 left */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
	val &= ~(WM8994_DAC1L_MUTE_MASK);
	wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, val);

	/* Unmute and volume ctrl RightDAC */
	val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME);
	val &= ~(WM8994_DAC1R_MUTE_MASK);
	wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);

	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
	val &= ~(WM8994_DAC1L_TO_MIXOUTL_MASK);
	val |= (WM8994_DAC1L_TO_MIXOUTL);
	wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val);

	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
	val &= ~(WM8994_DAC1R_TO_MIXOUTR_MASK);
	val |= (WM8994_DAC1R_TO_MIXOUTR);
	wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);

	val = wm8994_read(codec, WM8994_HPOUT2_MIXER);
	val &= ~(WM8994_MIXOUTLVOL_TO_HPOUT2_MASK |
			WM8994_MIXOUTRVOL_TO_HPOUT2_MASK);
	val |= (WM8994_MIXOUTRVOL_TO_HPOUT2 | WM8994_MIXOUTLVOL_TO_HPOUT2);
	wm8994_write(codec, WM8994_HPOUT2_MIXER, val);

	if (wm8994->output_source == VOIP_OUTPUT)
		wm8994_set_codec_gain(codec, PLAYBACK_MODE, PLAYBACK_VOIP_RCV);
	else
		wm8994_set_codec_gain(codec, PLAYBACK_MODE, PLAYBACK_RCV);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_5);
	val &= ~(WM8994_DAC1R_ENA_MASK | WM8994_DAC1L_ENA_MASK |
			WM8994_AIF1DAC1R_ENA_MASK | WM8994_AIF1DAC1L_ENA_MASK);
	val |= (WM8994_AIF1DAC1L_ENA | WM8994_AIF1DAC1R_ENA |
			WM8994_DAC1L_ENA | WM8994_DAC1R_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_5, val);

	val = wm8994_read(codec, WM8994_DAC1_LEFT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1L_TO_DAC1L_MASK);
	val |= (WM8994_AIF1DAC1L_TO_DAC1L);
	wm8994_write(codec, WM8994_DAC1_LEFT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1R_TO_DAC1R_MASK);
	val |= (WM8994_AIF1DAC1R_TO_DAC1R);
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

	val = wm8994_read(codec, WM8994_AIF1_DAC1_FILTERS_1);
	val &= ~(WM8994_AIF1DAC1_MUTE_MASK | WM8994_AIF1DAC1_MONO_MASK);
	val |= (WM8994_AIF1DAC1_UNMUTE | WM8994_AIF1DAC1_MONO);
	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, val);

}
void wm8994_set_headset_common(struct snd_soc_codec *codec,
	enum wm8994_dc_servo_slots slots)
{
	struct wm8994_priv *wm8994 = codec->drvdata;

	u16 val;

	u16 testreturn1 = 0;
	u16 testreturn2 = 0;
	u16 testlow1 = 0;
	u16 testhigh1 = 0;
	u8 testlow = 0;
	u8 testhigh = 0;

	/* Headset */
	wm8994_write(codec, 0x102, 0x0003);
	wm8994_write(codec, 0x56, 0x0003);
	wm8994_write(codec, 0x102, 0x0000);

	/* Charge pump control : Class W
	 * Using dynamic charge pump control mode whitch allows the power
	 * consumption to be optimised in real time, but can only be used
	 * if a single AIF source is the only signal source
	 */
	val = wm8994_read(codec, WM8994_CLASS_W_1);
	val &= ~(0x0005);
	val |= 0x0005;
	wm8994_write(codec, WM8994_CLASS_W_1, val);

	val = wm8994_read(codec, WM8994_DC_SERVO_2);
	val &= ~(0x03E0);
	val = 0x03E0;
	wm8994_write(codec, WM8994_DC_SERVO_2, val);

	/* Enable HPOUT1 intermediate stage */
	val = (WM8994_HPOUT1L_DLY | WM8994_HPOUT1R_DLY);
	wm8994_write(codec, WM8994_ANALOGUE_HP_1, val);

	/* Enable Charge Pump, this is from wolfson */
	val = wm8994_read(codec, WM8994_CHARGE_PUMP_1);
	val &= ~WM8994_CP_ENA_MASK;
	val |= WM8994_CP_ENA | WM8994_CP_ENA_DEFAULT;
	wm8994_write(codec, WM8994_CHARGE_PUMP_1, val);

	msleep(5);

	if (!wm8994->dc_servo[slots]) {
		wait_for_dc_servo(codec,
				  WM8994_DCS_TRIG_SERIES_0 |
				  WM8994_DCS_TRIG_SERIES_1);

		testreturn1 = wm8994_read(codec, WM8994_DC_SERVO_4);

		testlow = (signed char)(testreturn1 & 0xff);
		testhigh = (signed char)((testreturn1>>8) & 0xff);

		testlow1 = ((signed short)(testlow-5)) & 0x00ff;
		testhigh1 = (((signed short)(testhigh-5)<<8) & 0xff00);
		testreturn2 = testlow1|testhigh1;
	} else {
		testreturn2 = wm8994->dc_servo[slots];
	}

	wm8994_write(codec, WM8994_DC_SERVO_4, testreturn2);
	wm8994->dc_servo[slots] = testreturn2;

	wait_for_dc_servo(codec,
			  WM8994_DCS_TRIG_DAC_WR_0 | WM8994_DCS_TRIG_DAC_WR_1);

	val = wm8994_read(codec, WM8994_LEFT_OUTPUT_VOLUME);
	val &= ~(WM8994_HPOUT1L_MUTE_N_MASK);
	val |= (WM8994_HPOUT1L_MUTE_N);
	wm8994_write(codec, WM8994_LEFT_OUTPUT_VOLUME, val);

	val = wm8994_read(codec, WM8994_RIGHT_OUTPUT_VOLUME);
	val &= ~(WM8994_HPOUT1R_MUTE_N_MASK);
	val |= (WM8994_HPOUT1R_MUTE_N);
	wm8994_write(codec, WM8994_RIGHT_OUTPUT_VOLUME, val);

}

void wm8994_set_playback_headset(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;

	u16 val;

	DEBUG_LOG("");

	/* enable power in analog domain */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
	val &= ~(WM8994_BIAS_ENA_MASK | WM8994_VMID_SEL_MASK |
		WM8994_SPKOUTR_ENA_MASK | WM8994_SPKOUTL_ENA_MASK);
	val |= (WM8994_BIAS_ENA | WM8994_VMID_SEL_NORMAL |
		WM8994_HPOUT1R_ENA | WM8994_HPOUT1L_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	wm8994_set_headset_common(codec, DCS_MEDIA);

	/* intermediate HP settings */
	val = wm8994_read(codec, WM8994_ANALOGUE_HP_1);
	val &= ~(WM8994_HPOUT1R_DLY_MASK | WM8994_HPOUT1R_OUTP_MASK |
		WM8994_HPOUT1R_RMV_SHORT_MASK | WM8994_HPOUT1L_DLY_MASK |
		WM8994_HPOUT1L_OUTP_MASK | WM8994_HPOUT1L_RMV_SHORT_MASK);
	val = (WM8994_HPOUT1L_RMV_SHORT | WM8994_HPOUT1L_OUTP|
		WM8994_HPOUT1L_DLY | WM8994_HPOUT1R_RMV_SHORT |
		WM8994_HPOUT1R_OUTP | WM8994_HPOUT1R_DLY);
	wm8994_write(codec, WM8994_ANALOGUE_HP_1, val);

	/* enable the Timeslot0 to DAC1 */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1L_TO_DAC1L_MASK);
	val |= WM8994_AIF1DAC1L_TO_DAC1L;
	wm8994_write(codec, WM8994_DAC1_LEFT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1R_TO_DAC1R_MASK);
	val |= WM8994_AIF1DAC1R_TO_DAC1R;
	wm8994_write(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING, val);

	/* enable power in digital domain */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_5);
	val &= ~(WM8994_DAC1R_ENA_MASK | WM8994_DAC1L_ENA_MASK |
		WM8994_AIF1DAC1R_ENA_MASK | WM8994_AIF1DAC1L_ENA_MASK);
	val |= (WM8994_AIF1DAC1L_ENA | WM8994_AIF1DAC1R_ENA |
		WM8994_DAC1L_ENA | WM8994_DAC1R_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_5, val);

	/* enable DAC1 to HPOUT1 path */
	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
	val &=  ~(WM8994_DAC1L_TO_HPOUT1L_MASK | WM8994_DAC1L_TO_MIXOUTL_MASK);
	val |= WM8994_DAC1L_TO_MIXOUTL;
	wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val);

	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
	val &= ~(WM8994_DAC1R_TO_HPOUT1R_MASK | WM8994_DAC1R_TO_MIXOUTR_MASK);
	val |= WM8994_DAC1R_TO_MIXOUTR;
	wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);

	/* enable power in analog domain */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_3);
	val &= ~(WM8994_MIXOUTLVOL_ENA_MASK | WM8994_MIXOUTRVOL_ENA_MASK |
		WM8994_MIXOUTL_ENA_MASK | WM8994_MIXOUTR_ENA_MASK |
		WM8994_SPKRVOL_ENA_MASK | WM8994_SPKLVOL_ENA_MASK);
	val |= WM8994_MIXOUTL_ENA | WM8994_MIXOUTR_ENA;
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, val);

	/* set gain */
	if (wm8994->output_source == RING_TONE)
		wm8994_set_codec_gain(codec, PLAYBACK_MODE, PLAYBACK_RING_HP);
	else if (wm8994->output_source == VOIP_OUTPUT)
		wm8994_set_codec_gain(codec, PLAYBACK_MODE, PLAYBACK_VOIP_HP);
	else if (wm8994->cur_path == HP_NO_MIC)
		wm8994_set_codec_gain(codec, PLAYBACK_MODE, PLAYBACK_HP_NO_MIC);
	else
		wm8994_set_codec_gain(codec, PLAYBACK_MODE, PLAYBACK_HP);

	/* unmute MIXOUT */
	val = wm8994_read(codec, WM8994_LEFT_OPGA_VOLUME);
	val &= ~(WM8994_MIXOUTL_MUTE_N_MASK);
	val |= WM8994_MIXOUT_VU | WM8994_MIXOUTL_MUTE_N;
	wm8994_write(codec, WM8994_LEFT_OPGA_VOLUME, val);

	val = wm8994_read(codec, WM8994_RIGHT_OPGA_VOLUME);
	val &= ~(WM8994_MIXOUTR_MUTE_N_MASK);
	val |= WM8994_MIXOUT_VU | WM8994_MIXOUTR_MUTE_N;
	wm8994_write(codec, WM8994_RIGHT_OPGA_VOLUME, val);

	/* unmute DAC1 */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
	val &= ~WM8994_DAC1L_MUTE_MASK;
	val |= WM8994_DAC1_VU;
	wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, val);

	val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME);
	val &= ~WM8994_DAC1R_MUTE_MASK;
	val |= WM8994_DAC1_VU;
	wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);

	/* unmute the AF1DAC1 */
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

	/* Disable end point for preventing pop up noise.*/
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
	/* Unmute the SPKMIXVOLUME */
	val = wm8994_read(codec, WM8994_SPEAKER_VOLUME_LEFT);
	val &= ~(WM8994_SPKOUTL_MUTE_N_MASK);
	val |= (WM8994_SPKOUTL_MUTE_N);
	wm8994_write(codec, WM8994_SPEAKER_VOLUME_LEFT, val);

	val = wm8994_read(codec, WM8994_SPEAKER_VOLUME_RIGHT);
	val &= ~(WM8994_SPKOUTR_MUTE_N_MASK);
	wm8994_write(codec, WM8994_SPEAKER_VOLUME_RIGHT, val);

	/* Unmute DAC1 left */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
	val &= ~(WM8994_DAC1L_MUTE_MASK);
	wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, val);

	/* Unmute and volume ctrl RightDAC */
	val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME);
	val &= ~(WM8994_DAC1R_MUTE_MASK);
	wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);

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

	if (wm8994->output_source == RING_TONE)
		wm8994_set_codec_gain(codec, PLAYBACK_MODE, PLAYBACK_RING_SPK);
	else if (wm8994->output_source == VOIP_OUTPUT)
		wm8994_set_codec_gain(codec, PLAYBACK_MODE, PLAYBACK_VOIP_SPK);
	else
		wm8994_set_codec_gain(codec, PLAYBACK_MODE, PLAYBACK_SPK);

	/* enable timeslot0 to left dac */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1L_TO_DAC1L_MASK);
	val |= WM8994_AIF1DAC1L_TO_DAC1L;
	wm8994_write(codec, WM8994_DAC1_LEFT_MIXER_ROUTING, val);

	/* Enbale bias,vmid and Left speaker */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
	val &= ~(WM8994_BIAS_ENA_MASK | WM8994_VMID_SEL_MASK |
		WM8994_HPOUT1L_ENA_MASK | WM8994_HPOUT1R_ENA_MASK |
		WM8994_SPKOUTR_ENA_MASK | WM8994_SPKOUTL_ENA_MASK);
	val |= (WM8994_BIAS_ENA | WM8994_VMID_SEL_NORMAL | WM8994_SPKOUTL_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	/* Unmute */
	val = wm8994_read(codec, WM8994_AIF1_DAC1_FILTERS_1);
	val &= ~(WM8994_AIF1DAC1_MUTE_MASK | WM8994_AIF1DAC1_MONO_MASK);
	val |= (WM8994_AIF1DAC1_UNMUTE | WM8994_AIF1DAC1_MONO);
	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, val);

}

void wm8994_set_playback_speaker_headset(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;

	u16 val;

	DEBUG_LOG("");

	/* enable power in analog domain */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
	val &= ~(WM8994_BIAS_ENA_MASK | WM8994_VMID_SEL_MASK |
		WM8994_HPOUT1L_ENA_MASK | WM8994_HPOUT1R_ENA_MASK |
		WM8994_SPKOUTL_ENA_MASK);
	val |= (WM8994_BIAS_ENA | WM8994_VMID_SEL_NORMAL |
		WM8994_HPOUT1R_ENA | WM8994_HPOUT1L_ENA |
		WM8994_SPKOUTL_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	wm8994_set_headset_common(codec, DCS_SPK_HP);

	/* intermediate HP settings */
	val = wm8994_read(codec, WM8994_ANALOGUE_HP_1);
	val &= ~(WM8994_HPOUT1R_DLY_MASK | WM8994_HPOUT1R_OUTP_MASK |
		WM8994_HPOUT1R_RMV_SHORT_MASK |	WM8994_HPOUT1L_DLY_MASK |
		WM8994_HPOUT1L_OUTP_MASK | WM8994_HPOUT1L_RMV_SHORT_MASK);
	val |= (WM8994_HPOUT1L_RMV_SHORT | WM8994_HPOUT1L_OUTP |
		WM8994_HPOUT1L_DLY | WM8994_HPOUT1R_RMV_SHORT |
		WM8994_HPOUT1R_OUTP | WM8994_HPOUT1R_DLY);
	wm8994_write(codec, WM8994_ANALOGUE_HP_1, val);

	/* enable the Timeslot0 to DAC1 */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1L_TO_DAC1L_MASK);
	val |= WM8994_AIF1DAC1L_TO_DAC1L;
	wm8994_write(codec, WM8994_DAC1_LEFT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1R_TO_DAC1R_MASK);
	val |= WM8994_AIF1DAC1R_TO_DAC1R;
	wm8994_write(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING, val);

	/* enable power in digital domain */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_5);
	val &= ~(WM8994_DAC1R_ENA_MASK | WM8994_DAC1L_ENA_MASK |
		WM8994_AIF1DAC1R_ENA_MASK | WM8994_AIF1DAC1L_ENA_MASK);
	val |= (WM8994_AIF1DAC1L_ENA | WM8994_AIF1DAC1R_ENA |
		WM8994_DAC1L_ENA | WM8994_DAC1R_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_5, val);

	/* enbale DAC1 to MIXOUT */
	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
	val &=  ~(WM8994_DAC1L_TO_HPOUT1L_MASK | WM8994_DAC1L_TO_MIXOUTL_MASK);
	val |=  WM8994_DAC1L_TO_MIXOUTL;
	wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val);

	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
	val &= ~(WM8994_DAC1R_TO_HPOUT1R_MASK | WM8994_DAC1R_TO_MIXOUTR_MASK);
	val |= WM8994_DAC1R_TO_MIXOUTR;
	wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);

	/* enable DAC1L to SPKMIXL */
	val = wm8994_read(codec, WM8994_SPEAKER_MIXER);
	val &= ~(WM8994_DAC1L_TO_SPKMIXL_MASK);
	val |= WM8994_DAC1L_TO_SPKMIXL;
	wm8994_write(codec, WM8994_SPEAKER_MIXER, val);

	/* enable SPKMIXL to SPKOUTL */
	val = wm8994_read(codec, WM8994_SPKOUT_MIXERS);
	val &= ~(WM8994_SPKMIXL_TO_SPKOUTL_MASK |
		WM8994_SPKMIXR_TO_SPKOUTL_MASK |
		WM8994_SPKMIXR_TO_SPKOUTR_MASK);
	val |= WM8994_SPKMIXL_TO_SPKOUTL;
	wm8994_write(codec, WM8994_SPKOUT_MIXERS, val);

	/* enable power in analog domain */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_3);
	val &= ~(WM8994_MIXOUTLVOL_ENA_MASK | WM8994_MIXOUTRVOL_ENA_MASK |
		WM8994_MIXOUTL_ENA_MASK | WM8994_MIXOUTR_ENA_MASK |
		WM8994_SPKLVOL_ENA_MASK);
	val |= (WM8994_MIXOUTL_ENA | WM8994_MIXOUTR_ENA | WM8994_SPKLVOL_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, val);

	/* set gain */
	if (wm8994->output_source == RING_TONE)
		wm8994_set_codec_gain(codec, PLAYBACK_MODE,
				PLAYBACK_RING_SPK_HP);
	else
		wm8994_set_codec_gain(codec, PLAYBACK_MODE, PLAYBACK_SPK_HP);

	/* unmute SPKOUTL */
	val = wm8994_read(codec, WM8994_SPEAKER_VOLUME_LEFT);
	val &= ~(WM8994_SPKOUTL_MUTE_N_MASK);
	val |= WM8994_SPKOUT_VU | WM8994_SPKOUTL_MUTE_N;
	wm8994_write(codec, WM8994_SPEAKER_VOLUME_LEFT, val);

	/* ummute DAC1 */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
	val &= ~WM8994_DAC1L_MUTE_MASK;
	val |= WM8994_DAC1_VU;
	wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, val);

	val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME);
	val &= ~WM8994_DAC1R_MUTE_MASK;
	val |= WM8994_DAC1_VU;
	wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);

	/* unmute AIF1DAC1 */
	val = wm8994_read(codec, WM8994_AIF1_DAC1_FILTERS_1);
	val &= ~(WM8994_AIF1DAC1_MUTE_MASK | WM8994_AIF1DAC1_MONO_MASK);
	val |= (WM8994_AIF1DAC1_UNMUTE | WM8994_AIF1DAC1_MONO);
	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, val);

}

void wm8994_set_playback_bluetooth(struct snd_soc_codec *codec)
{
	u16 val;

	DEBUG_LOG("BT Playback Path for SCO\n");

	wm8994_set_bluetooth_common_setting(codec);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
	val &= ~(WM8994_BIAS_ENA_MASK | WM8994_VMID_SEL_MASK);
	val |= (WM8994_BIAS_ENA | WM8994_VMID_SEL_NORMAL);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_2, 0x0000);

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, 0x0000);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_4);
	val &= ~(WM8994_AIF2ADCL_ENA_MASK | WM8994_AIF2ADCR_ENA_MASK);
	val |= (WM8994_AIF2ADCL_ENA | WM8994_AIF2ADCR_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_5);
	val &= ~(WM8994_AIF2DACL_ENA_MASK | WM8994_AIF2DACR_ENA_MASK |
		WM8994_AIF1DAC1L_ENA_MASK | WM8994_AIF1DAC1R_ENA_MASK |
		WM8994_DAC1L_ENA_MASK | WM8994_DAC1R_ENA_MASK);
	val |= (WM8994_AIF2DACL_ENA | WM8994_AIF2DACR_ENA |
		WM8994_AIF1DAC1L_ENA | WM8994_AIF1DAC1R_ENA |
		WM8994_DAC1L_ENA | WM8994_DAC1R_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_5, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_6);
	val &= ~(WM8994_AIF3_ADCDAT_SRC_MASK);
	val |= (0x0001 << WM8994_AIF3_ADCDAT_SRC_SHIFT);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_6, val);

	/* Mixer Routing*/
	val = wm8994_read(codec, WM8994_DAC2_LEFT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1L_TO_DAC2L_MASK);
	val |= (WM8994_AIF1DAC1L_TO_DAC2L);
	wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1R_TO_DAC2R_MASK);
	val |= (WM8994_AIF1DAC1R_TO_DAC2R);
	wm8994_write(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING, val);

	/* Volume*/
	wm8994_write(codec, WM8994_DAC2_LEFT_VOLUME, 0x01C0);
	wm8994_write(codec, WM8994_DAC2_RIGHT_VOLUME, 0x01C0);

	wm8994_write(codec, WM8994_AIF2_CLOCKING_1, 0x0019);

	wm8994_write(codec, WM8994_OVERSAMPLING, 0X0000);

	/* GPIO Configuration*/
	wm8994_write(codec, WM8994_GPIO_8, WM8994_GP8_DIR | WM8994_GP8_DB);
	wm8994_write(codec, WM8994_GPIO_9, WM8994_GP9_DB);
	wm8994_write(codec, WM8994_GPIO_10, WM8994_GP10_DB);
	wm8994_write(codec, WM8994_GPIO_11, WM8994_GP11_DB);

	/* Un-Mute*/
	val = wm8994_read(codec, WM8994_AIF1_DAC1_FILTERS_1);
	val &= ~(WM8994_AIF1DAC1_MUTE_MASK | WM8994_AIF1DAC1_MONO_MASK);
	val |= (WM8994_AIF1DAC1_UNMUTE);
	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, val);

}

void wm8994_set_playback_extra_dock_speaker(struct snd_soc_codec *codec)
{
	u16 val;

	DEBUG_LOG("");

	/* Disable end point for preventing pop up noise */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_3);
	val &= ~(WM8994_LINEOUT2N_ENA_MASK | WM8994_LINEOUT2P_ENA_MASK);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, val);

	/* Unmute */
	val = wm8994_read(codec, WM8994_LEFT_OPGA_VOLUME);
	val &= ~(WM8994_MIXOUTL_MUTE_N_MASK);
	val |= (WM8994_MIXOUTL_MUTE_N);
	wm8994_write(codec, WM8994_LEFT_OPGA_VOLUME, val);

	val = wm8994_read(codec, WM8994_RIGHT_OPGA_VOLUME);
	val &= ~(WM8994_MIXOUTR_MUTE_N_MASK);
	val |= (WM8994_MIXOUTR_MUTE_N);
	wm8994_write(codec, WM8994_RIGHT_OPGA_VOLUME, val);

	/* For X-talk of VPS's L/R line. It's requested by H/W team. */
	wm8994_write(codec, WM8994_ADDITIONAL_CONTROL, 0x00);
	wm8994_write(codec, WM8994_ANTIPOP_1, 0x80);

	/* Enable DAC1 to mixout */
	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
	val &= ~(WM8994_DAC1L_TO_MIXOUTL_MASK);
	val |= (WM8994_DAC1L_TO_MIXOUTL);
	wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val);

	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
	val &= ~(WM8994_DAC1R_TO_MIXOUTR_MASK);
	val |= (WM8994_DAC1R_TO_MIXOUTR);
	wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);

	/* Enable mixout to lineout */
	val = wm8994_read(codec, WM8994_LINE_MIXER_2);
	val &= ~(WM8994_MIXOUTR_TO_LINEOUT2N_MASK |
		WM8994_MIXOUTL_TO_LINEOUT2N_MASK | WM8994_LINEOUT2_MODE_MASK |
		WM8994_MIXOUTR_TO_LINEOUT2P_MASK);
	val |= (WM8994_MIXOUTL_TO_LINEOUT2N | WM8994_LINEOUT2_MODE |
		WM8994_MIXOUTR_TO_LINEOUT2P);
	wm8994_write(codec, WM8994_LINE_MIXER_2, val);

	/* Enable DAC1 left/right and Timeslot left/right */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_5);
	val &= ~(WM8994_DAC1R_ENA_MASK | WM8994_DAC1L_ENA_MASK |
		WM8994_AIF1DAC1R_ENA_MASK | WM8994_AIF1DAC1L_ENA_MASK);
	val |= (WM8994_AIF1DAC1L_ENA | WM8994_AIF1DAC1R_ENA | WM8994_DAC1L_ENA |
		WM8994_DAC1R_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_5, val);

	wm8994_set_codec_gain(codec, PLAYBACK_MODE, PLAYBACK_EXTRA_DOCK);

	/* Unmute timeslot 0 */
	val = wm8994_read(codec, WM8994_AIF1_DAC1_FILTERS_1);
	val &= ~(WM8994_AIF1DAC1_MUTE_MASK | WM8994_AIF1DAC1_MONO_MASK);
	val |= (WM8994_AIF1DAC1_UNMUTE);
	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, val);

	/* Unmute Line Outputs Volume */
	val = wm8994_read(codec, WM8994_LINE_OUTPUTS_VOLUME);
	val &= ~(WM8994_LINEOUT2N_MUTE_MASK | WM8994_LINEOUT2P_MUTE_MASK);
	wm8994_write(codec, WM8994_LINE_OUTPUTS_VOLUME, val);

	/* Enable Timeslot0 to left DAC */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1L_TO_DAC1L_MASK);
	val |= (WM8994_AIF1DAC1L_TO_DAC1L);
	wm8994_write(codec, WM8994_DAC1_LEFT_MIXER_ROUTING, val);

	/* Enable Timeslot0 to right DAC */
	val = wm8994_read(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1R_TO_DAC1R_MASK);
	val |= (WM8994_AIF1DAC1R_TO_DAC1R);
	wm8994_write(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING, val);

	/* Enable AIF1 processing clock and digital mixing processor clock */
	val = wm8994_read(codec, WM8994_CLOCKING_1);
	val &= ~(WM8994_DSP_FS1CLK_ENA_MASK | WM8994_DSP_FSINTCLK_ENA_MASK);
	val |= (WM8994_DSP_FS1CLK_ENA | WM8994_DSP_FSINTCLK_ENA);
	wm8994_write(codec, WM8994_CLOCKING_1, val);

	/* Enable lineout and mixout */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_3);
	val &= ~(WM8994_LINEOUT2N_ENA_MASK | WM8994_LINEOUT1P_ENA_MASK |
		WM8994_MIXOUTLVOL_ENA_MASK | WM8994_MIXOUTRVOL_ENA_MASK |
		WM8994_MIXOUTL_ENA_MASK | WM8994_MIXOUTR_ENA_MASK);
	val |= (WM8994_LINEOUT2N_ENA | WM8994_LINEOUT2P_ENA |
		WM8994_MIXOUTL_ENA | WM8994_MIXOUTR_ENA |
		WM8994_MIXOUTRVOL_ENA | WM8994_MIXOUTLVOL_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, val);

	/* Enable bias, vmid */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
	val &= ~(WM8994_BIAS_ENA_MASK | WM8994_VMID_SEL_MASK |
		WM8994_HPOUT2_ENA_MASK | WM8994_SPKOUTR_ENA_MASK |
		WM8994_SPKOUTL_ENA_MASK);
	val |= (WM8994_BIAS_ENA | WM8994_VMID_SEL_NORMAL);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	/* Unmute DAC1 left */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
	val &= ~(WM8994_DAC1L_MUTE_MASK);
	val |= WM8994_DAC1_VU;
	wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, val);

	/* Unmute and volume ctrl RightDAC */
	val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME);
	val &= ~(WM8994_DAC1R_MUTE_MASK);
	val |= WM8994_DAC1_VU;
	wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);

}

void wm8994_set_voicecall_common(struct snd_soc_codec *codec)
{
	int val;

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
	wm8994_write(codec, WM8994_AIF2_CONTROL_1,0x4010);

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

	audio_ctrl_mic_bias_gpio(wm8994->pdata, 1);

	wm8994_set_voicecall_common(codec);

	/* Disable charge pump */
	wm8994_write(codec, WM8994_CHARGE_PUMP_1, WM8994_CP_ENA_DEFAULT);

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_2,
		WM8994_TSHUT_ENA | WM8994_TSHUT_OPDIS | WM8994_MIXINL_ENA |
		WM8994_IN1L_ENA);

	wm8994_write(codec, WM8994_INPUT_MIXER_2,
			WM8994_IN1LP_TO_IN1L | WM8994_IN1LN_TO_IN1L);

	/* Digital Path Enables and Unmutes */
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_4,
			WM8994_AIF2ADCL_ENA | WM8994_ADCL_ENA | WM8994_AIF2ADCR_ENA);

	wm8994_write(codec, WM8994_DAC2_MIXER_VOLUMES, 0x018C);
	wm8994_write(codec, WM8994_DAC2_LEFT_VOLUME, 0x01C0);
	wm8994_write(codec, WM8994_DAC2_RIGHT_VOLUME, 0x01C0);

	/* Tx -> AIF2 Path */
	wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING,
			WM8994_ADC1_TO_DAC2L);
	wm8994_write(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING,
			WM8994_ADC1_TO_DAC2R);

	wm8994_write(codec, WM8994_AIF2_CLOCKING_1, 0x0019);

	/* Unmute IN1L PGA, update volume */
	val = wm8994_read(codec, WM8994_LEFT_LINE_INPUT_1_2_VOLUME);
	val &= ~(WM8994_IN1L_MUTE_MASK);
	wm8994_write(codec, WM8994_LEFT_LINE_INPUT_1_2_VOLUME, val);
	wm8994_write(codec, WM8994_SIDETONE, 0x01C0);

	/* Unmute the PGA */
	val = wm8994_read(codec, WM8994_INPUT_MIXER_3);
	val &= ~(WM8994_IN1L_TO_MIXINL_MASK);
	val |= (WM8994_IN1L_TO_MIXINL);
	wm8994_write(codec, WM8994_INPUT_MIXER_3, val);

	/* Volume Control - Output */
	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_5);
	val &= ~(WM8994_DACL_MIXOUTL_VOL_MASK);
	wm8994_write(codec,WM8994_OUTPUT_MIXER_5, val);

	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_6);
	val &= ~(WM8994_DACR_MIXOUTR_VOL_MASK);
	wm8994_write(codec,WM8994_OUTPUT_MIXER_6, val);

	val = wm8994_read(codec, WM8994_LEFT_OPGA_VOLUME);
	val &= ~(WM8994_MIXOUTL_MUTE_N_MASK);
	val |= (WM8994_MIXOUTL_MUTE_N);
	wm8994_write(codec, WM8994_LEFT_OPGA_VOLUME, val);

	val = wm8994_read(codec, WM8994_RIGHT_OPGA_VOLUME);
	val &= ~(WM8994_MIXOUTR_MUTE_N_MASK);
	val |= (WM8994_MIXOUTR_MUTE_N);
	wm8994_write(codec, WM8994_RIGHT_OPGA_VOLUME, val);

	val = wm8994_read(codec, WM8994_HPOUT2_VOLUME);
	val &= ~(WM8994_HPOUT2_MUTE_MASK);
	wm8994_write(codec, WM8994_HPOUT2_VOLUME, val);

	wm8994_set_codec_gain(codec, VOICECALL_MODE, VOICECALL_RCV);

	/* Unmute DAC1 left */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
	val &= ~(WM8994_DAC1L_MUTE_MASK);
	wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, val);

	/* Unmute and volume ctrl RightDAC */
	val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME);
	val &= ~(WM8994_DAC1R_MUTE_MASK);
	wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);

	/* Output Mixing */
	wm8994_write(codec, WM8994_OUTPUT_MIXER_1, WM8994_DAC1L_TO_MIXOUTL);
	wm8994_write(codec, WM8994_OUTPUT_MIXER_2, WM8994_DAC1R_TO_MIXOUTR);

	/* Sidetone */
	wm8994_write(codec, WM8994_DAC1_MIXER_VOLUMES, 0x0000);
	

	/* Analogue Output Configuration */
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3,
		WM8994_MIXOUTLVOL_ENA | WM8994_MIXOUTRVOL_ENA |
		WM8994_MIXOUTL_ENA | WM8994_MIXOUTR_ENA);
	wm8994_write(codec, WM8994_HPOUT2_MIXER, WM8994_MIXOUTLVOL_TO_HPOUT2 |
			WM8994_MIXOUTRVOL_TO_HPOUT2);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1,
		WM8994_HPOUT2_ENA | WM8994_VMID_SEL_NORMAL | WM8994_BIAS_ENA);

	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, 0x0000);
	wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_1, 0x0000);

}

void wm8994_set_voicecall_headset(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;

	int val;

	DEBUG_LOG("");

#ifdef FEATURE_TTY
	switch (tty_mode) {
	case TTY_MODE_FULL :
		DEBUG_LOG("Selected TTY_FULL");

		wm8994_set_voicecall_tty_full(codec);
		return;
		break;

	case TTY_MODE_HCO :
		DEBUG_LOG("Selected TTY_HCO");

		wm8994_set_voicecall_tty_hco(codec);
		return;
		break;

	case TTY_MODE_VCO :
		DEBUG_LOG("Selected TTY_VCO");

		wm8994_set_voicecall_tty_vco(codec);
		return;
		break;

	case TTY_MODE_OFF :
	default:
		DEBUG_LOG("Selected TTY_OFF");
		break;
	}
#endif

	/* enable power in analog domain */
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, 0x0303);

	wm8994_set_headset_common(codec, DCS_VOICE);

	/* enable power in analog domain */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_2);
	val &= ~(WM8994_TSHUT_ENA_MASK | WM8994_TSHUT_OPDIS_MASK |
		WM8994_MIXINR_ENA_MASK | WM8994_IN1R_ENA_MASK);
	val |= (WM8994_TSHUT_ENA | WM8994_TSHUT_OPDIS |
		WM8994_MIXINR_ENA | WM8994_IN1R_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_2, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_3);
	val &= ~(WM8994_MIXOUTLVOL_ENA_MASK | WM8994_MIXOUTRVOL_ENA_MASK |
		WM8994_MIXOUTL_ENA_MASK | WM8994_MIXOUTR_ENA_MASK |
		WM8994_SPKRVOL_ENA_MASK | WM8994_SPKLVOL_ENA_MASK);
	val |= WM8994_MIXOUTL_ENA | WM8994_MIXOUTR_ENA;
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, val);
	
	/* enable power in digital domain */
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, 0x2001);

	/* intermediate HP settings */
	val = wm8994_read(codec, WM8994_ANALOGUE_HP_1);
	val &= ~(WM8994_HPOUT1R_DLY_MASK | WM8994_HPOUT1R_OUTP_MASK |
		WM8994_HPOUT1R_RMV_SHORT_MASK | WM8994_HPOUT1L_DLY_MASK |
		WM8994_HPOUT1L_OUTP_MASK | WM8994_HPOUT1L_RMV_SHORT_MASK);
	val = (WM8994_HPOUT1L_RMV_SHORT | WM8994_HPOUT1L_OUTP|
		WM8994_HPOUT1L_DLY | WM8994_HPOUT1R_RMV_SHORT |
		WM8994_HPOUT1R_OUTP | WM8994_HPOUT1R_DLY);
	wm8994_write(codec, WM8994_ANALOGUE_HP_1, val);

	wm8994_set_voicecall_common(codec);

	wm8994_write(codec, WM8994_AIF2_CLOCKING_1, 0x0019);

	/* digital path enables and unmutes */
	if (wm8994->hw_version == 3) {	/* H/W Rev D */
		wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING,
			WM8994_ADC2_TO_DAC2L);
		wm8994_write(codec, WM8994_DAC2_MIXER_VOLUMES, 0x0180);
		wm8994_write(codec, WM8994_SIDETONE, 0x01C0);
	} else {	/* H/W Rev B */
		wm8994_write(codec, WM8994_DAC2_MIXER_VOLUMES, 0x000C);
		wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING,
			WM8994_ADC1_TO_DAC2L);
		wm8994_write(codec, WM8994_SIDETONE, 0x01C1);
	}
	
	/* analog input configuration */
	val = wm8994_read(codec, WM8994_INPUT_MIXER_4);
	val |= WM8994_IN1R_TO_MIXINR;
	wm8994_write(codec, WM8994_INPUT_MIXER_4, val);

	val = wm8994_read(codec, WM8994_INPUT_MIXER_2);
	val |= (WM8994_IN1RP_TO_IN1R | WM8994_IN1RN_TO_IN1R);
	wm8994_write(codec, WM8994_INPUT_MIXER_2, val);

	val = wm8994_read(codec, WM8994_RIGHT_LINE_INPUT_1_2_VOLUME);
	val &= ~WM8994_IN1R_MUTE_MASK;
	val |= WM8994_IN1R_VU;
	wm8994_write(codec, WM8994_RIGHT_LINE_INPUT_1_2_VOLUME, val);

	/* analogue output configuration */
	wm8994_write(codec, WM8994_OUTPUT_MIXER_1, 0x0001);
	wm8994_write(codec, WM8994_OUTPUT_MIXER_2, 0x0001);

	wm8994_set_codec_gain(codec, VOICECALL_MODE, VOICECALL_HP);

	/* unmute MIXOUT */
	val = wm8994_read(codec, WM8994_LEFT_OPGA_VOLUME);
	val &= ~WM8994_MIXOUTL_MUTE_N_MASK;
	val |= WM8994_MIXOUT_VU | WM8994_MIXOUTL_MUTE_N;
	wm8994_write(codec, WM8994_LEFT_OPGA_VOLUME, val);

	val = wm8994_read(codec, WM8994_RIGHT_OPGA_VOLUME);
	val &= ~WM8994_MIXOUTR_MUTE_N_MASK;
	val |= WM8994_MIXOUT_VU | WM8994_MIXOUTR_MUTE_N;
	wm8994_write(codec, WM8994_RIGHT_OPGA_VOLUME, val);

	/* Unmute DAC1 */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
	val &= ~WM8994_DAC1L_MUTE_MASK;
	val |= WM8994_DAC1_VU;
	wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, val);

	val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME);
	val &= ~WM8994_DAC1R_MUTE_MASK;
	val |= WM8994_DAC1_VU;
	wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);

	/* unmute DAC2 */
	val = wm8994_read(codec, WM8994_DAC2_LEFT_VOLUME);
	val &= ~WM8994_DAC2L_MUTE_MASK;
	val |= WM8994_DAC2_VU;
	wm8994_write(codec, WM8994_DAC2_LEFT_VOLUME, val);

	/* Unmute AIF1DAC1 */
	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, 0x0000);

	/* Unmute AIF2DAC */
	wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_1, 0x0000);

}

void wm8994_set_voicecall_headphone(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;

	int val;

	DEBUG_LOG("");
	audio_ctrl_mic_bias_gpio(wm8994->pdata, 1);

	/* enable power in analog domain */
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, 0x0303);

	wm8994_set_headset_common(codec, DCS_VOICE);

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_2,
		WM8994_TSHUT_ENA | WM8994_TSHUT_OPDIS | WM8994_MIXINL_ENA |
		WM8994_IN1L_ENA);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_3);
	val &= ~(WM8994_MIXOUTLVOL_ENA_MASK | WM8994_MIXOUTRVOL_ENA_MASK |
		WM8994_MIXOUTL_ENA_MASK | WM8994_MIXOUTR_ENA_MASK |
		WM8994_SPKRVOL_ENA_MASK | WM8994_SPKLVOL_ENA_MASK);
	val |= WM8994_MIXOUTL_ENA | WM8994_MIXOUTR_ENA;
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, val);
	
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_4,
			WM8994_AIF2ADCL_ENA | WM8994_ADCL_ENA);

	/* intermediate HP settings */
	val = wm8994_read(codec, WM8994_ANALOGUE_HP_1);
	val &= ~(WM8994_HPOUT1R_DLY_MASK | WM8994_HPOUT1R_OUTP_MASK |
		WM8994_HPOUT1R_RMV_SHORT_MASK | WM8994_HPOUT1L_DLY_MASK |
		WM8994_HPOUT1L_OUTP_MASK | WM8994_HPOUT1L_RMV_SHORT_MASK);
	val = (WM8994_HPOUT1L_RMV_SHORT | WM8994_HPOUT1L_OUTP|
		WM8994_HPOUT1L_DLY | WM8994_HPOUT1R_RMV_SHORT |
		WM8994_HPOUT1R_OUTP | WM8994_HPOUT1R_DLY);
	wm8994_write(codec, WM8994_ANALOGUE_HP_1, val);

	wm8994_set_voicecall_common(codec);

	wm8994_write(codec, WM8994_AIF2_CLOCKING_1, 0x0019);

	/* digital path enables and unmutes */
	wm8994_write(codec, WM8994_DAC2_MIXER_VOLUMES, 0x000C);
	wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING,
			WM8994_ADC1_TO_DAC2L);

	/* analogue input configuration */
	val = wm8994_read(codec, WM8994_INPUT_MIXER_3);
	val &= ~WM8994_IN1L_TO_MIXINL_MASK;
	val |= WM8994_IN1L_TO_MIXINL;
	wm8994_write(codec, WM8994_INPUT_MIXER_3, val);

	wm8994_write(codec, WM8994_INPUT_MIXER_2,
			WM8994_IN1LP_TO_IN1L | WM8994_IN1LN_TO_IN1L);

	val = wm8994_read(codec, WM8994_LEFT_LINE_INPUT_1_2_VOLUME);
	val &= ~WM8994_IN1L_MUTE_MASK;
	val |= WM8994_IN1L_VU;
	wm8994_write(codec, WM8994_LEFT_LINE_INPUT_1_2_VOLUME, val);

	/* analogue output configuration */
	wm8994_write(codec, WM8994_OUTPUT_MIXER_1, 0x0001);
	wm8994_write(codec, WM8994_OUTPUT_MIXER_2, 0x0001);

	wm8994_set_codec_gain(codec, VOICECALL_MODE, VOICECALL_HP_NO_MIC);

	/* unmute MIXOUT */
	val = wm8994_read(codec, WM8994_LEFT_OPGA_VOLUME);
	val &= ~(WM8994_MIXOUTL_MUTE_N_MASK);
	val |= (WM8994_MIXOUTL_MUTE_N);
	wm8994_write(codec, WM8994_LEFT_OPGA_VOLUME, val);

	val = wm8994_read(codec, WM8994_RIGHT_OPGA_VOLUME);
	val &= ~(WM8994_MIXOUTR_MUTE_N_MASK);
	val |= (WM8994_MIXOUTR_MUTE_N);
	wm8994_write(codec, WM8994_RIGHT_OPGA_VOLUME, val);

	/* unmute DAC1 */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
	val &= ~WM8994_DAC1L_MUTE_MASK;
	val |= WM8994_DAC1_VU;
	wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, val);

	val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME);
	val &= ~WM8994_DAC1R_MUTE_MASK;
	val |= WM8994_DAC1_VU;
	wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);

	/* unmute DAC2 */
	val = wm8994_read(codec, WM8994_DAC2_LEFT_VOLUME);
	val &= ~WM8994_DAC2L_MUTE_MASK;
	val |= WM8994_DAC2_VU;
	wm8994_write(codec, WM8994_DAC2_LEFT_VOLUME, val);

	/* unmute AIF1DAC1 */
	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, 0x0000);

	/* unmute AIF2DAC */
	wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_1, 0x0000);

}

void wm8994_set_voicecall_speaker(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;

	int val;

	DEBUG_LOG("");

	audio_ctrl_mic_bias_gpio(wm8994->pdata, 1);

	wm8994_set_voicecall_common(codec);

	wm8994_write(codec, WM8994_DAC2_MIXER_VOLUMES, 0x0180);
	/* Tx -> AIF2 Path */
	wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING,
			WM8994_ADC2_TO_DAC2L);
	wm8994_write(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING, 
			WM8994_ADC2_TO_DAC2R);

	/* Analogue Input Configuration*/
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_2, 0x6120);
	wm8994_write(codec, WM8994_INPUT_MIXER_2, WM8994_IN2RP_TO_IN2R |
			WM8994_IN2RN_TO_IN2R);

	val = wm8994_read(codec, WM8994_INPUT_MIXER_4); 
	val &= ~(WM8994_IN2R_TO_MIXINR_MASK);
	val |= (WM8994_IN2R_TO_MIXINR);
	wm8994_write(codec, WM8994_INPUT_MIXER_4, val);

	val = wm8994_read(codec, WM8994_RIGHT_LINE_INPUT_3_4_VOLUME);
	val &= ~(WM8994_IN2R_MUTE_MASK);
	wm8994_write(codec, WM8994_RIGHT_LINE_INPUT_3_4_VOLUME, val);

	/* Analogue Output Configuration*/
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, 0x0300);

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_4,
			WM8994_AIF2ADCL_ENA | WM8994_ADCL_ENA | WM8994_AIF2ADCR_ENA | WM8994_ADCR_ENA);

	wm8994_write(codec, WM8994_AIF2_CLOCKING_1, 0x0019);

	val = wm8994_read(codec, WM8994_SPEAKER_VOLUME_LEFT);
	val &= ~(WM8994_SPKOUTL_MUTE_N_MASK);
	val |= (WM8994_SPKOUTL_MUTE_N);
	wm8994_write(codec, WM8994_SPEAKER_VOLUME_LEFT, val);

	val = wm8994_read(codec, WM8994_SPEAKER_VOLUME_RIGHT);
	val &= ~(WM8994_SPKOUTR_MUTE_N_MASK | WM8994_SPKOUTR_VOL_MASK);
	wm8994_write(codec, WM8994_SPEAKER_VOLUME_RIGHT, val);

	val = wm8994_read(codec, WM8994_SPKOUT_MIXERS);
	val &= ~(WM8994_SPKMIXL_TO_SPKOUTL_MASK |
		WM8994_SPKMIXR_TO_SPKOUTL_MASK |
		WM8994_SPKMIXR_TO_SPKOUTR_MASK);
	val |= WM8994_SPKMIXL_TO_SPKOUTL;
	wm8994_write(codec, WM8994_SPKOUT_MIXERS, val);

	wm8994_write(codec, WM8994_SPEAKER_MIXER, 0x0003);
	/* Digital Path Enables and Unmutes*/

	wm8994_write(codec, WM8994_SIDETONE, 0x01C0);


	wm8994_write(codec, WM8994_ANALOGUE_HP_1, 0x0000);
	wm8994_write(codec, WM8994_DC_SERVO_1, 0x0000);

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1,
		WM8994_SPKOUTL_ENA | WM8994_VMID_SEL_NORMAL | WM8994_BIAS_ENA);

	wm8994_set_codec_gain(codec, VOICECALL_MODE, VOICECALL_SPK);

	/* Unmute DAC1 left */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
	val &= ~(WM8994_DAC1L_MUTE_MASK);
	wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, val);

	/* Unmute and volume ctrl RightDAC */
	val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME);
	val &= ~(WM8994_DAC1R_MUTE_MASK);
	wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);

	wm8994_write(codec, WM8994_DAC2_LEFT_VOLUME, 0x01C0);
	wm8994_write(codec, WM8994_DAC2_RIGHT_VOLUME, 0x01C0);
	wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_1, WM8994_AIF1DAC1_UNMUTE);
	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, WM8994_AIF1DAC2_UNMUTE);
}

void wm8994_set_voicecall_bluetooth(struct snd_soc_codec *codec)
{
	int val;

	DEBUG_LOG("");

	wm8994_set_voicecall_common(codec);

	/* GPIO Configuration */
	wm8994_write(codec, WM8994_GPIO_8, WM8994_GP8_DIR | WM8994_GP8_DB);
	wm8994_write(codec, WM8994_GPIO_9, WM8994_GP9_DB);
	wm8994_write(codec, WM8994_GPIO_10, WM8994_GP10_DB);
	wm8994_write(codec, WM8994_GPIO_11, WM8994_GP11_DB);

	/* Digital Path Enables and Unmutes */
	val  = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
	val &= ~(WM8994_SPKOUTL_ENA_MASK | WM8994_HPOUT2_ENA_MASK |
		WM8994_HPOUT1L_ENA_MASK | WM8994_HPOUT1R_ENA_MASK);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_4,
		WM8994_AIF2ADCL_ENA | WM8994_ADCL_ENA);

	/* If Input MIC is enabled, bluetooth Rx is muted. */
	wm8994_write(codec, WM8994_LEFT_LINE_INPUT_1_2_VOLUME,
			WM8994_IN1L_MUTE);
	wm8994_write(codec, WM8994_RIGHT_LINE_INPUT_1_2_VOLUME,
			WM8994_IN1R_MUTE);
	wm8994_write(codec, WM8994_INPUT_MIXER_2, 0x00);
	wm8994_write(codec, WM8994_INPUT_MIXER_3, 0x00);
	wm8994_write(codec, WM8994_INPUT_MIXER_4, 0x00);

	/*
	 * for BT DTMF Play
	 * Rx Path: AIF2ADCDAT2 select
	 * CP(CALL) Path:GPIO5/DACDAT2 select
	 * AP(DTMF) Path: DACDAT1 select
	 * Tx Path: GPIO8/DACDAT3 select
	 */

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_6, 0x000C);

	/* AIF1 & AIF2 Output is connected to DAC1 */
	wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING,
		WM8994_AIF2DACL_TO_DAC2L | WM8994_AIF1DAC1L_TO_DAC2L);
	wm8994_write(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING,
		WM8994_AIF2DACR_TO_DAC2R | WM8994_AIF1DAC1R_TO_DAC2R);

	wm8994_write(codec, WM8994_AIF2_CLOCKING_1, 0x0019);

	wm8994_write(codec, WM8994_DAC2_MIXER_VOLUMES, 0x000C);

	wm8994_write(codec, WM8994_DAC2_LEFT_VOLUME, 0x01C0);
	wm8994_write(codec, WM8994_DAC2_RIGHT_VOLUME, 0x01C0);

	wm8994_write(codec, WM8994_OVERSAMPLING, 0X0000);

	wm8994_set_codec_gain(codec, VOICECALL_MODE, VOICECALL_BT);

	/* Unmute DAC1 left */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
	val &= ~(WM8994_DAC1L_MUTE_MASK);
	wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, val);

	/* Unmute and volume ctrl RightDAC */
	val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME);
	val &= ~(WM8994_DAC1R_MUTE_MASK);
	wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);

	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, 0x0000);
	wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_1, 0x0000);
}

void wm8994_disable_fmradio_path(struct snd_soc_codec *codec,
	enum fmradio_path path)
{
	//struct wm8994_priv *wm8994 = codec->drvdata;

	u16 val;

	DEBUG_LOG("Turn off fmradio_path = [%d]", path);

	switch (path) {
	case FMR_SPK:
		/* Disable end point for preventing pop up noise */
		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
		val &= ~(WM8994_SPKOUTL_ENA_MASK | WM8994_HPOUT1L_ENA_MASK |
			WM8994_HPOUT1R_ENA_MASK);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_1 , val);

		/* Mute SPKOUTL */
		val = wm8994_read(codec, WM8994_SPEAKER_VOLUME_LEFT);
		val &= ~WM8994_SPKOUTL_MUTE_N_MASK;
		val |= WM8994_SPKOUT_VU;
		wm8994_write(codec, WM8994_SPEAKER_VOLUME_LEFT, val);

		/* Mute SPKOUTR */
		val = wm8994_read(codec, WM8994_SPEAKER_VOLUME_RIGHT);
		val &= ~WM8994_SPKOUTR_MUTE_N_MASK;
		val |= WM8994_SPKOUT_VU;
		wm8994_write(codec, WM8994_SPEAKER_VOLUME_RIGHT, val);

		/* Disable SPKMIX to SPKOUT */
		val = wm8994_read(codec, WM8994_SPKOUT_MIXERS);
		val &= ~(WM8994_SPKMIXL_TO_SPKOUTL_MASK |
			WM8994_SPKMIXR_TO_SPKOUTL_MASK |
			WM8994_SPKMIXR_TO_SPKOUTR_MASK);
		wm8994_write(codec, WM8994_SPKOUT_MIXERS, val);

		/* Disable MIXIN to SPKMIX */
		val = wm8994_read(codec, WM8994_SPEAKER_MIXER);
		val &= ~(WM8994_MIXINL_TO_SPKMIXL_MASK |
			WM8994_MIXINR_TO_SPKMIXR_MASK);
		wm8994_write(codec, WM8994_SPEAKER_MIXER, val);
		break;

	case FMR_HP:

		/* cut off the power supply in analog domain */
		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
		val &= ~(WM8994_SPKOUTL_ENA_MASK | WM8994_HPOUT1L_ENA_MASK |
			WM8994_HPOUT1R_ENA_MASK);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_1 , val);

		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_3);
		val &= ~(WM8994_MIXOUTLVOL_ENA_MASK |
			WM8994_MIXOUTRVOL_ENA_MASK | WM8994_MIXOUTL_ENA_MASK |
			WM8994_MIXOUTR_ENA_MASK);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, val);

		/* Mute HPOUT1L_VOL */
		val = wm8994_read(codec, WM8994_LEFT_OUTPUT_VOLUME);
		val &= ~WM8994_HPOUT1L_MUTE_N_MASK;
		val |= WM8994_HPOUT1_VU;
		wm8994_write(codec, WM8994_LEFT_OUTPUT_VOLUME, val);

		/* Mute HPOUT1R_VOL */
		val = wm8994_read(codec, WM8994_RIGHT_OUTPUT_VOLUME);
		val &= ~WM8994_HPOUT1R_MUTE_N_MASK;
		val |= WM8994_HPOUT1_VU;
		wm8994_write(codec, WM8994_RIGHT_OUTPUT_VOLUME, val);

		/* Disable Charge Pump, this is from wolfson */
		val = wm8994_read(codec, WM8994_CHARGE_PUMP_1);
		val &= ~WM8994_CP_ENA_MASK;
		val |= WM8994_CP_ENA_DEFAULT;
		wm8994_write(codec, WM8994_CHARGE_PUMP_1, val);

		/* Intermediate HP settings */
		val = wm8994_read(codec, WM8994_ANALOGUE_HP_1);
		val &= ~(WM8994_HPOUT1R_DLY_MASK | WM8994_HPOUT1R_OUTP_MASK |
			WM8994_HPOUT1R_RMV_SHORT_MASK |
			WM8994_HPOUT1L_DLY_MASK | WM8994_HPOUT1L_OUTP_MASK |
			WM8994_HPOUT1L_RMV_SHORT_MASK);
		wm8994_write(codec, WM8994_ANALOGUE_HP_1, val);

		/* Disable Output mixer setting */
		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
		val &= ~(WM8994_DAC1L_TO_HPOUT1L_MASK |
			WM8994_MIXINL_TO_MIXOUTL_MASK);
		wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val);

		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
		val &= ~(WM8994_DAC1R_TO_HPOUT1R_MASK |
			WM8994_MIXINR_TO_MIXOUTR_MASK);
		wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);

		/* disble DAC1 to MIXOUT */
		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
		val &= ~WM8994_DAC1L_TO_MIXOUTL;
		wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val);

		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
		val &= ~WM8994_DAC1R_TO_MIXOUTR;
		wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);

		break;

	case FMR_DUAL_MIX:
		/* Disable DAC1L to SPKMIXL */
		val = wm8994_read(codec, WM8994_SPEAKER_MIXER);
		val &= ~(WM8994_DAC1L_TO_SPKMIXL_MASK);
		wm8994_write(codec, WM8994_SPEAKER_MIXER, val);

		/* Disable DAC1L to HPOUT1L, DAC1L to MIXOUTL */
		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
		val &= ~(WM8994_DAC1L_TO_HPOUT1L_MASK |
			WM8994_DAC1L_TO_MIXOUTL_MASK);
		wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val);

		/* Disable DAC1R to HPOUT1R, DAC1R to MIXOUTR */
		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
		val &= ~(WM8994_DAC1R_TO_HPOUT1R_MASK |
			WM8994_DAC1R_TO_MIXOUTR_MASK);
		wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);
		break;

	default:
		DEBUG_LOG_ERR("fmradio path[%d] is not invaild!\n", path);
		return;
		break;
	}
}

void wm8994_set_fmradio_input_active(struct snd_soc_codec *codec, int on)
{
	u16 val;

	DEBUG_LOG("on = %d", on);

	if (on) {
		/* Unmute IN2 */
		val = wm8994_read(codec, WM8994_LEFT_LINE_INPUT_3_4_VOLUME);
		val &= ~WM8994_IN2L_MUTE_MASK;
		val |= WM8994_IN2L_VU;
		wm8994_write(codec, WM8994_LEFT_LINE_INPUT_3_4_VOLUME, val);

		val = wm8994_read(codec, WM8994_RIGHT_LINE_INPUT_3_4_VOLUME);
		val &= ~WM8994_IN2R_MUTE_MASK;
		val |= WM8994_IN2R_VU;
		wm8994_write(codec, WM8994_RIGHT_LINE_INPUT_3_4_VOLUME, val);

		/* Enable IN2LN to IN2L, IN2RN to IN2R */
		val = wm8994_read(codec, WM8994_INPUT_MIXER_2);
		val = (WM8994_IN2LN_TO_IN2L | WM8994_IN2RN_TO_IN2R);
		wm8994_write(codec, WM8994_INPUT_MIXER_2, val);

		/* Enable IN2L to MIXINL */
		val = wm8994_read(codec, WM8994_INPUT_MIXER_3);
		val |= WM8994_IN2L_TO_MIXINL;
		wm8994_write(codec, WM8994_INPUT_MIXER_3, val);

		/* Enable IN2R to MIXINR */
		val = wm8994_read(codec, WM8994_INPUT_MIXER_4);
		val |= WM8994_IN2R_TO_MIXINR;
		wm8994_write(codec, WM8994_INPUT_MIXER_4, val);

	} else {
		/* Mute IN2 */
		val = wm8994_read(codec, WM8994_LEFT_LINE_INPUT_3_4_VOLUME);
		val |= WM8994_IN2L_VU | WM8994_IN2L_MUTE;
		wm8994_write(codec, WM8994_LEFT_LINE_INPUT_3_4_VOLUME, val);

		val = wm8994_read(codec, WM8994_RIGHT_LINE_INPUT_3_4_VOLUME);
		val |= WM8994_IN2R_VU | WM8994_IN2R_MUTE;
		wm8994_write(codec, WM8994_RIGHT_LINE_INPUT_3_4_VOLUME, val);

		/* Disable IN2LN to IN2L, IN2RN to IN2R */
		val = wm8994_read(codec, WM8994_INPUT_MIXER_2);
		val &= ~(WM8994_IN2LN_TO_IN2L | WM8994_IN2RN_TO_IN2R);
		wm8994_write(codec, WM8994_INPUT_MIXER_2, val);

		/* disable IN2L to MIXINL */
		val = wm8994_read(codec, WM8994_INPUT_MIXER_3);
		val &= ~WM8994_IN2L_TO_MIXINL_MASK;
		wm8994_write(codec, WM8994_INPUT_MIXER_3, val);

		/* disable IN2R to MIXINR */
		val = wm8994_read(codec, WM8994_INPUT_MIXER_4);
		val &= ~WM8994_IN2R_TO_MIXINR_MASK;
		wm8994_write(codec, WM8994_INPUT_MIXER_4, val);
	}

}

void wm8994_set_fmradio_common(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;

	u16 val;

	DEBUG_LOG("");

	/* Cross Talk (H/W requested) */
	wm8994_write(codec, WM8994_ANTIPOP_2, 0x8);

	/* GPIO settings */
	wm8994_write(codec, WM8994_GPIO_3, 0x0100);
	wm8994_write(codec, WM8994_GPIO_4, 0x0100);
	wm8994_write(codec, WM8994_GPIO_5, 0x8100);
	wm8994_write(codec, WM8994_GPIO_6, 0xA101);
	wm8994_write(codec, WM8994_GPIO_7, 0x0100);

	/* Disable AIF1 timesolot0 EQ gain */
	wm8994_write(codec, WM8994_AIF1_DAC1_EQ_GAINS_1, 0x0000);

	/* Enable AIF1CLK */
	val = wm8994_read(codec, WM8994_AIF1_CLOCKING_1);
	val |= WM8994_AIF1CLK_ENA;
	wm8994_write(codec, WM8994_AIF1_CLOCKING_1, val);

	/* AIF2CLK : FLL1 */
	val = wm8994_read(codec, WM8994_AIF2_CLOCKING_1);
	val &= ~WM8994_AIF2CLK_SRC_MASK;
	val |= (WM8994_AIF2CLK_ENA | 0x2 << WM8994_AIF2CLK_SRC_SHIFT);
	wm8994_write(codec, WM8994_AIF2_CLOCKING_1, val);

	/* Enable AIF1, AIF2 processing clock, digital mixing processor clock
	 * SYSCLK source is AIF1CLK
	 */
	val = wm8994_read(codec, WM8994_CLOCKING_1);
	val &= ~WM8994_SYSCLK_SRC_MASK;
	val |= (WM8994_DSP_FS1CLK_ENA | WM8994_DSP_FS2CLK_ENA |
		WM8994_DSP_FSINTCLK_ENA);
	wm8994_write(codec, WM8994_CLOCKING_1, val);

	/* Disable oversampling */
	wm8994_write(codec, WM8994_OVERSAMPLING, 0x0000);

	/* AIF2 Master, Forces CLK to be enabled
	 * when all AIF2 audio channels are disabled
	 */
	val = wm8994_read(codec, WM8994_AIF2_MASTER_SLAVE);
	val |= (WM8994_AIF2_LRCLK_FRC | WM8994_AIF2_CLK_FRC | WM8994_AIF2_MSTR);
	wm8994_write(codec, WM8994_AIF2_MASTER_SLAVE, val);

	/* DAC1L(601h:0x05) settings */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_MIXER_ROUTING);
	val &= ~WM8994_AIF1DAC2L_TO_DAC1L_MASK;
	val |= (WM8994_AIF2DACL_TO_DAC1L | WM8994_AIF1DAC1L_TO_DAC1L);
	wm8994_write(codec, WM8994_DAC1_LEFT_MIXER_ROUTING, val);

	/* DAC1R(602h:0x05) settings */
	val = wm8994_read(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING);
	val &= ~WM8994_AIF1DAC2R_TO_DAC1R_MASK;
	val |= (WM8994_AIF1DAC1R_TO_DAC1R | WM8994_AIF2DACR_TO_DAC1R);
	wm8994_write(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING, val);

	/* DAC2 Mixer Vol : 0dB (-36dB ~ 0dB) */
	wm8994_write(codec, WM8994_DAC2_MIXER_VOLUMES, 0x018C);

	/* Enable AIF2 Loopback */
	val = wm8994_read(codec, WM8994_AIF2_CONTROL_2);
	val |= (WM8994_AIF2_LOOPBACK);
	wm8994_write(codec, WM8994_AIF2_CONTROL_2, val);

	/* DRC for Noise-gate (AIF2) */
	wm8994_write(codec, WM8994_AIF2_ADC_FILTERS, 0xF800);
	wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_1, 0x0036);
	wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_2, 0x0010);
	wm8994_write(codec, WM8994_AIF2_DRC_2, 0x0840);
	wm8994_write(codec, WM8994_AIF2_DRC_3, 0x2400);
	wm8994_write(codec, WM8994_AIF2_DRC_4, 0x0000);
	wm8994_write(codec, WM8994_AIF2_DRC_5, 0x0000);
	wm8994_write(codec, WM8994_AIF2_DRC_1, 0x019C);

	if (wm8994->fmr_mix_path != FMR_DUAL_MIX)
		wm8994_set_fmradio_input_active(codec, 1);

	/* Enable ADC1 to DAC2L */
	val = wm8994_read(codec, WM8994_DAC2_LEFT_MIXER_ROUTING);
	val |= (WM8994_ADC1_TO_DAC2L);
	wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING, val);

	/* Enable ADC2 to DAC2R */
	val = wm8994_read(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING);
	val |= (WM8994_ADC2_TO_DAC2R);
	wm8994_write(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING, val);
}

void wm8994_set_fmradio_headset(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;

	u16 val;

	DEBUG_LOG("Routing FM Radio: Headset");

	wm8994->fmradio_path = FMR_HP;

	wm8994_disable_fmradio_path(codec, FMR_SPK);

	/* Enable HPF, cut-off freq is 370Hz */
	wm8994_write(codec, WM8994_SIDETONE, 0x01C0);

	/* enable power in analog domain */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
	val &= ~WM8994_VMID_SEL_MASK;
	val |= (WM8994_BIAS_ENA | WM8994_VMID_SEL_NORMAL | WM8994_HPOUT1R_ENA |
		WM8994_HPOUT1L_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	wm8994_set_headset_common(codec, DCS_FMRADIO);

	/* enable power in analog domain */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_2);
	val |= (WM8994_MIXINL_ENA | WM8994_MIXINR_ENA | WM8994_IN2L_ENA |
		WM8994_IN2R_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_2 , val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_3);
	val |= (WM8994_MIXOUTLVOL_ENA | WM8994_MIXOUTRVOL_ENA |
		WM8994_MIXOUTL_ENA | WM8994_MIXOUTR_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, val);

	/* enable power in digital domain */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_4);
	val |= (WM8994_AIF2ADCL_ENA | WM8994_AIF2ADCR_ENA | WM8994_ADCL_ENA |
		WM8994_ADCR_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_5);
	val |= (WM8994_AIF2DACL_ENA | WM8994_AIF2DACR_ENA |
		WM8994_AIF1DAC1L_ENA | WM8994_AIF1DAC1R_ENA |
		WM8994_DAC1L_ENA | WM8994_DAC1R_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_5, val);

	/* intermediate HP settings */
	val = wm8994_read(codec, WM8994_ANALOGUE_HP_1);
	val |= (WM8994_HPOUT1L_RMV_SHORT | WM8994_HPOUT1L_OUTP |
		WM8994_HPOUT1L_DLY | WM8994_HPOUT1R_RMV_SHORT |
		WM8994_HPOUT1R_OUTP | WM8994_HPOUT1R_DLY);
	wm8994_write(codec, WM8994_ANALOGUE_HP_1, val);

	wm8994_set_fmradio_common(codec);

	wm8994_set_codec_gain(codec, FMRADIO_MODE, FMRADIO_HP);

	/* enable DAC1 to MIXOUT */
	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
	val |= (WM8994_DAC1L_TO_MIXOUTL);
	wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val);

	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
	val |= (WM8994_DAC1R_TO_MIXOUTR);
	wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);

	/* soft unmute settings */
	wm8994_write(codec, WM8994_DAC_SOFTMUTE, WM8994_DAC_SOFTMUTEMODE |
		WM8994_DAC_MUTERATE);

	/* unmute MIXOUT */
	val = wm8994_read(codec, WM8994_LEFT_OPGA_VOLUME);
	val |= WM8994_MIXOUT_VU | WM8994_MIXOUTL_MUTE_N;
	wm8994_write(codec, WM8994_LEFT_OPGA_VOLUME, val);

	val = wm8994_read(codec, WM8994_RIGHT_OPGA_VOLUME);
	val |= WM8994_MIXOUT_VU | WM8994_MIXOUTR_MUTE_N;
	wm8994_write(codec, WM8994_RIGHT_OPGA_VOLUME, val);

	/* unmute DAC1 */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
	val &= ~WM8994_DAC1L_MUTE_MASK;
	val |= WM8994_DAC1_VU;
	wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, val);

	val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME);
	val &= ~WM8994_DAC1R_MUTE_MASK;
	val |= WM8994_DAC1_VU;
	wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);

	/* unmute DAC2 */
	val = wm8994_read(codec, WM8994_DAC2_LEFT_VOLUME);
	val &= ~WM8994_DAC2L_MUTE_MASK;
	val |= WM8994_DAC2_VU;
	wm8994_write(codec, WM8994_DAC2_LEFT_VOLUME, val);

	val = wm8994_read(codec, WM8994_DAC2_RIGHT_VOLUME);
	val &= ~WM8994_DAC2R_MUTE_MASK;
	val |= WM8994_DAC2_VU;
	wm8994_write(codec, WM8994_DAC2_RIGHT_VOLUME, val);

	/* Unmute AIF1DAC1 */
	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, 0x0000);

	/* Unmute AIF2DAC */
	val = wm8994_read(codec, WM8994_AIF2_DAC_FILTERS_1);
	val &= ~(WM8994_AIF2DAC_MUTE_MASK);
	val |= WM8994_AIF2DAC_UNMUTE_RAMP | WM8994_AIF2DAC_MUTERATE;
	wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_1, val);

}

void wm8994_set_fmradio_speaker(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;

	u16 val;

	DEBUG_LOG("Routing FM Radio: Speaker");

	wm8994_disable_fmradio_path(codec, FMR_HP);

	wm8994->fmradio_path = FMR_SPK;

	/* enable power in analog domain */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
	val &= ~WM8994_VMID_SEL_MASK;
	val |= (WM8994_BIAS_ENA | WM8994_VMID_SEL_NORMAL | WM8994_SPKOUTL_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_2);
	val |= (WM8994_MIXINL_ENA | WM8994_IN2L_ENA |
		WM8994_MIXINR_ENA | WM8994_IN2R_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_2 , val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_3);
	val |= WM8994_SPKLVOL_ENA;
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, val);

	/* enable power in digital domain */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_4);
	val |= (WM8994_AIF2ADCL_ENA | WM8994_ADCL_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_5);
	val |= (WM8994_AIF2DACL_ENA | WM8994_AIF1DAC1L_ENA |
		WM8994_DAC1L_ENA | WM8994_AIF1DAC1R_ENA |
		WM8994_DAC1R_ENA | WM8994_AIF2DACR_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_5, val);

	/* Enable themal sensor, OPCLK */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_2);
	val |= (WM8994_TSHUT_ENA | WM8994_TSHUT_OPDIS | WM8994_OPCLK_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_2, val);

	/* Unmute SPKOUTL */
	val = wm8994_read(codec, WM8994_SPEAKER_VOLUME_LEFT);
	val |= WM8994_SPKOUT_VU | WM8994_SPKOUTL_MUTE_N;
	wm8994_write(codec, WM8994_SPEAKER_VOLUME_LEFT, val);

	wm8994_set_fmradio_common(codec);

	wm8994_set_codec_gain(codec, FMRADIO_MODE, FMRADIO_SPK);

	/* Enable SPKMIXL to SPKOUTL */
	val = wm8994_read(codec, WM8994_SPKOUT_MIXERS);
	val &= ~(WM8994_SPKMIXR_TO_SPKOUTL_MASK |
		WM8994_SPKMIXR_TO_SPKOUTR_MASK);
	val |= (WM8994_SPKMIXL_TO_SPKOUTL);
	wm8994_write(codec, WM8994_SPKOUT_MIXERS, val);

	/* Enable DAC1L to SPKMIXL */
	val = wm8994_read(codec, WM8994_SPEAKER_MIXER);
	val &= ~(WM8994_MIXINL_TO_SPKMIXL_MASK | WM8994_MIXINR_TO_SPKMIXR_MASK |
		WM8994_DAC1R_TO_SPKMIXR_MASK);
	val |= (WM8994_DAC1L_TO_SPKMIXL);
	wm8994_write(codec, WM8994_SPEAKER_MIXER, val);

	/* Soft unmute settings */
	wm8994_write(codec, WM8994_DAC_SOFTMUTE, WM8994_DAC_SOFTMUTEMODE |
		WM8994_DAC_MUTERATE);

	/* Unmute DAC1L */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
	val &= ~WM8994_DAC1L_MUTE_MASK;
	val |= WM8994_DAC1_VU;
	wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, val);

	/* Unmute DAC1R */
	val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME);
	val &= ~WM8994_DAC1R_MUTE_MASK;
	val |= WM8994_DAC1_VU;
	wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);

	/* Unmute DAC2L */
	val = wm8994_read(codec, WM8994_DAC2_LEFT_VOLUME);
	val &= ~WM8994_DAC2L_MUTE_MASK;
	val |= WM8994_DAC2_VU;
	wm8994_write(codec, WM8994_DAC2_LEFT_VOLUME, val);

	/* Unmute DAC2R */
	val = wm8994_read(codec, WM8994_DAC2_RIGHT_VOLUME);
	val &= ~WM8994_DAC2R_MUTE_MASK;
	val |= WM8994_DAC2_VU;
	wm8994_write(codec, WM8994_DAC2_RIGHT_VOLUME, val);

	/* Unmute AIF2DAC */
	val = wm8994_read(codec, WM8994_AIF2_DAC_FILTERS_1);
	val &= ~(WM8994_AIF2DAC_MUTE_MASK);
	val |= WM8994_AIF2DAC_UNMUTE_RAMP | WM8994_AIF2DAC_MUTERATE;
	wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_1, val);

	/* Unmute AIF1DAC1 */
	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, 0x0000);

}

void wm8994_set_fmradio_speaker_headset_mix(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;

	u16 val;

	DEBUG_LOG("");

	if (wm8994->fmradio_path == FMR_HP)
		wm8994_disable_fmradio_path(codec, FMR_HP);
	else
		wm8994_disable_fmradio_path(codec, FMR_SPK);

	wm8994->fmradio_path = FMR_DUAL_MIX;

	/* enable power in analog domain */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
	val &= ~WM8994_VMID_SEL_MASK;
	val |= (WM8994_BIAS_ENA | WM8994_VMID_SEL_NORMAL | WM8994_HPOUT1R_ENA |
		WM8994_HPOUT1L_ENA | WM8994_SPKOUTL_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	wm8994_write(codec, 0x5D, 0x0002);

	wm8994_set_headset_common(codec, DCS_FMRADIO_SPK_HP);

	/* enable power in analog domain */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_2);
	val |= (WM8994_MIXINL_ENA | WM8994_MIXINR_ENA | WM8994_IN2L_ENA |
		WM8994_IN2R_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_2 , val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_3);
	val &= ~WM8994_SPKRVOL_ENA_MASK;
	val |= (WM8994_MIXOUTLVOL_ENA | WM8994_MIXOUTRVOL_ENA |
		WM8994_MIXOUTL_ENA | WM8994_MIXOUTR_ENA | WM8994_SPKLVOL_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, val);

	/* enable power in digital domain */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_4);
	val |= (WM8994_AIF2ADCL_ENA | WM8994_AIF2ADCR_ENA | WM8994_ADCL_ENA |
		WM8994_ADCR_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_5);
	val |= (WM8994_AIF2DACL_ENA | WM8994_AIF2DACR_ENA |
		WM8994_AIF1DAC1L_ENA | WM8994_AIF1DAC1R_ENA |
		WM8994_DAC1L_ENA | WM8994_DAC1R_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_5, val);

	/* intermediate HP settings */
	val = wm8994_read(codec, WM8994_ANALOGUE_HP_1);
	val &= ~(WM8994_HPOUT1R_DLY_MASK | WM8994_HPOUT1R_OUTP_MASK |
		WM8994_HPOUT1R_RMV_SHORT_MASK |	WM8994_HPOUT1L_DLY_MASK |
		WM8994_HPOUT1L_OUTP_MASK | WM8994_HPOUT1L_RMV_SHORT_MASK);
	val |= (WM8994_HPOUT1L_RMV_SHORT | WM8994_HPOUT1L_OUTP |
		WM8994_HPOUT1L_DLY | WM8994_HPOUT1R_RMV_SHORT |
		WM8994_HPOUT1R_OUTP | WM8994_HPOUT1R_DLY);
	wm8994_write(codec, WM8994_ANALOGUE_HP_1, val);

	wm8994_set_fmradio_common(codec);

	wm8994_set_codec_gain(codec, FMRADIO_MODE, FMRADIO_SPK_HP);

	/* DRC for Noise-gate (AIF2) */
	wm8994_write(codec, WM8994_AIF2_DRC_2 , 0x0850);
	wm8994_write(codec, WM8994_AIF2_DRC_3, 0x0800);
	wm8994_write(codec, WM8994_AIF2_DRC_4, 0x0001);
	wm8994_write(codec, WM8994_AIF2_DRC_5, 0x0008);
	wm8994_write(codec, WM8994_AIF2_DRC_1, 0x01BC);

	/* enable DAC1 to MIXOUT */
	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
	val |= (WM8994_DAC1L_TO_MIXOUTL);
	wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val);

	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
	val |= (WM8994_DAC1R_TO_MIXOUTR);
	wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);

	/* enable DAC1L to SPKMIXL */
	val = wm8994_read(codec, WM8994_SPEAKER_MIXER);
	val |= WM8994_DAC1L_TO_SPKMIXL;
	wm8994_write(codec, WM8994_SPEAKER_MIXER, val);

	/* enable SPKMIXL to SPKOUTL */
	val = wm8994_read(codec, WM8994_SPKOUT_MIXERS);
	val &= ~(WM8994_SPKMIXR_TO_SPKOUTL_MASK |
		WM8994_SPKMIXR_TO_SPKOUTR_MASK);
	val |= WM8994_SPKMIXL_TO_SPKOUTL;
	wm8994_write(codec, WM8994_SPKOUT_MIXERS, val);

	/* unmute MIXOUTL */
	val = wm8994_read(codec, WM8994_LEFT_OPGA_VOLUME);
	val |= WM8994_MIXOUT_VU | WM8994_MIXOUTL_MUTE_N;
	wm8994_write(codec, WM8994_LEFT_OPGA_VOLUME, val);

	/* unmute MIXOUTR */
	val = wm8994_read(codec, WM8994_RIGHT_OPGA_VOLUME);
	val |= WM8994_MIXOUT_VU | WM8994_MIXOUTR_MUTE_N;
	wm8994_write(codec, WM8994_RIGHT_OPGA_VOLUME, val);

	/* unmute SPKOUTL */
	val = wm8994_read(codec, WM8994_SPEAKER_VOLUME_LEFT);
	val |= WM8994_SPKOUT_VU | WM8994_SPKOUTL_MUTE_N;
	wm8994_write(codec, WM8994_SPEAKER_VOLUME_LEFT, val);

	/* soft unmute settings */
	wm8994_write(codec, WM8994_DAC_SOFTMUTE, WM8994_DAC_SOFTMUTEMODE |
		WM8994_DAC_MUTERATE);

	/* unmute DAC1 */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
	val &= ~WM8994_DAC1L_MUTE_MASK;
	val |= WM8994_DAC1_VU;
	wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, val);

	val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME);
	val &= ~WM8994_DAC1R_MUTE_MASK;
	val |= WM8994_DAC1_VU;
	wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);

	/* unmute DAC2 */
	val = wm8994_read(codec, WM8994_DAC2_LEFT_VOLUME);
	val &= ~WM8994_DAC2L_MUTE_MASK;
	val |= WM8994_DAC2_VU;
	wm8994_write(codec, WM8994_DAC2_LEFT_VOLUME, val);

	val = wm8994_read(codec, WM8994_DAC2_RIGHT_VOLUME);
	val &= ~WM8994_DAC2R_MUTE_MASK;
	val |= WM8994_DAC2_VU;
	wm8994_write(codec, WM8994_DAC2_RIGHT_VOLUME, val);

	/* unmute AIF1DAC1 , mono mix */
	val = wm8994_read(codec, WM8994_AIF1_DAC1_FILTERS_1);
	val &= ~WM8994_AIF1DAC1_MUTE_MASK;
	val |= WM8994_AIF1DAC1_MONO;
	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, val);

	/* unmute AIF2DAC */
	val = wm8994_read(codec, WM8994_AIF2_DAC_FILTERS_1);
	val &= ~WM8994_AIF2DAC_MUTE_MASK;
	wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_1, val);

}

#ifdef FEATURE_TTY
void wm8994_set_voicecall_tty_common(struct snd_soc_codec *codec)
{
	int val;


	DEBUG_LOG("");
        
	val = wm8994_read(codec, WM8994_ANTIPOP_2); 
	val &= ~(WM8994_VMID_RAMP_MASK | WM8994_VMID_BUF_ENA_MASK | 
						WM8994_STARTUP_BIAS_ENA_MASK);
	val |= (WM8994_VMID_RAMP_MASK | WM8994_VMID_BUF_ENA | 
						WM8994_STARTUP_BIAS_ENA);
	wm8994_write(codec, WM8994_ANTIPOP_2, val);		

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, 
                                WM8994_VMID_SEL_NORMAL | WM8994_BIAS_ENA_MASK);

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

	wm8994_write(codec, WM8994_CLOCKING_1, 0x000F);

	wm8994_write(codec, WM8994_OVERSAMPLING, 0x0000);

	wm8994_write(codec, WM8994_AIF2_RATE, 0x3 << WM8994_AIF2CLK_RATE_SHIFT);

	wm8994_write(codec, WM8994_AIF1_MASTER_SLAVE, WM8994_AIF1_MSTR);

	wm8994_write(codec, WM8994_AIF2_MASTER_SLAVE, WM8994_AIF2_MSTR | 
                                WM8994_AIF2_CLK_FRC | WM8994_AIF2_LRCLK_FRC);

	/* AIF2 Interface - PCM Stereo mode */
        wm8994_write(codec, WM8994_AIF2_CONTROL_1, 0x4010);

	wm8994_write(codec, WM8994_AIF2_CONTROL_2, 0x0000);

	wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_1, 0x0000);

	val = wm8994_read(codec, WM8994_DAC2_LEFT_MIXER_ROUTING);
	val &= ~(WM8994_ADC2_TO_DAC2L_MASK | WM8994_ADC1_TO_DAC2L_MASK);
	val |= (WM8994_ADC2_TO_DAC2L | WM8994_ADC1_TO_DAC2L);
	wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING);
	val &= ~(WM8994_ADC2_TO_DAC2R_MASK | WM8994_ADC1_TO_DAC2R_MASK);
	val |= (WM8994_ADC2_TO_DAC2R | WM8994_ADC1_TO_DAC2R);
	wm8994_write(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING, val); 

	/* Output Path Routing */
	val = (0x1000 | WM8994_AIF2DACL_ENA | WM8994_AIF1DAC1L_ENA | 
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

	val = (WM8994_MIXOUTLVOL_ENA | WM8994_MIXOUTRVOL_ENA | 
				WM8994_MIXOUTL_ENA | WM8994_MIXOUTR_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, val);

	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, WM8994_AIF1DAC1_MONO);
}

void wm8994_set_voicecall_tty_full(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;

	int val;


	DEBUG_LOG("");

        wm8994_set_voicecall_tty_common(codec);
        
	/* Input Path Routing */
	val = wm8994_read(codec, WM8994_INPUT_MIXER_2);
	val &= ~(WM8994_IN1RN_TO_IN1R_MASK);
	val |= (WM8994_IN1RN_TO_IN1R);
	wm8994_write(codec, WM8994_INPUT_MIXER_2, val);

	val = (0x4000 | WM8994_TSHUT_OPDIS | WM8994_MIXINR_ENA_MASK | 
		WM8994_IN2R_ENA_MASK | WM8994_IN1R_ENA_MASK);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_2, val);	

	val = wm8994_read(codec, WM8994_INPUT_MIXER_4);
	val &= ~(WM8994_IN1R_TO_MIXINR_MASK | WM8994_IN1R_MIXINR_VOL_MASK);
	val |= WM8994_IN1R_TO_MIXINR;
	wm8994_write(codec, WM8994_INPUT_MIXER_4, val);

	val = (WM8994_AIF2ADCR_ENA | WM8994_AIF2ADCL_ENA  | WM8994_ADCR_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, val);


	audio_ctrl_mic_bias_gpio(wm8994->pdata, 1);


	/* Output Path Routing */
	wm8994_write(codec, WM8994_OUTPUT_MIXER_1, WM8994_DAC1L_TO_HPOUT1L);
	wm8994_write(codec, WM8994_OUTPUT_MIXER_2, WM8994_DAC1R_TO_HPOUT1R);

	val = wm8994_read(codec, WM8994_ANALOGUE_HP_1);
	val &= ~(WM8994_HPOUT1L_RMV_SHORT_MASK | WM8994_HPOUT1L_OUTP_MASK | 
		WM8994_HPOUT1L_DLY_MASK | WM8994_HPOUT1R_RMV_SHORT_MASK | 
		WM8994_HPOUT1R_OUTP_MASK | WM8994_HPOUT1R_DLY_MASK);
	val |= (WM8994_HPOUT1L_RMV_SHORT | WM8994_HPOUT1L_OUTP | 
				WM8994_HPOUT1L_DLY | WM8994_HPOUT1R_RMV_SHORT | 
				WM8994_HPOUT1R_OUTP | WM8994_HPOUT1R_DLY);
	wm8994_write(codec, WM8994_ANALOGUE_HP_1, val);


	val = wm8994_read(codec, WM8994_RIGHT_LINE_INPUT_1_2_VOLUME);
	val &= ~WM8994_IN1R_MUTE_MASK;
	val |= WM8994_IN1R_VU;
	wm8994_write(codec, WM8994_RIGHT_LINE_INPUT_1_2_VOLUME, val);

        wm8994_write(codec, WM8994_DAC2_LEFT_VOLUME, 0x01C0);
        wm8994_write(codec, WM8994_DAC2_RIGHT_VOLUME, 0x01C0);
                
        wm8994_write(codec, WM8994_DAC2_MIXER_VOLUMES, 0x018C);

	wm8994_write(codec, WM8994_OUTPUT_MIXER_5, 0x0000);
	wm8994_write(codec, WM8994_OUTPUT_MIXER_6, 0x0000);


	wm8994_set_codec_gain(codec, VOICECALL_MODE, VOICECALL_TTY_FULL);


	/* unmute MIXOUT */
	val = wm8994_read(codec, WM8994_LEFT_OPGA_VOLUME);
	val &= ~WM8994_MIXOUTL_MUTE_N_MASK;
	val |= WM8994_MIXOUT_VU | WM8994_MIXOUTL_MUTE_N;
	wm8994_write(codec, WM8994_LEFT_OPGA_VOLUME, val);

	val = wm8994_read(codec, WM8994_RIGHT_OPGA_VOLUME);
	val &= ~WM8994_MIXOUTR_MUTE_N_MASK;
	val |= WM8994_MIXOUT_VU | WM8994_MIXOUTR_MUTE_N;
	wm8994_write(codec, WM8994_RIGHT_OPGA_VOLUME, val);

	/* Unmute DAC1 */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
	val &= ~WM8994_DAC1L_MUTE_MASK;
	val |= WM8994_DAC1_VU;
	wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, val);

	val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME);
	val &= ~WM8994_DAC1R_MUTE_MASK;
	val |= WM8994_DAC1_VU;
	wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);

	val = wm8994_read(codec,WM8994_LEFT_OUTPUT_VOLUME);
	val &= ~(WM8994_HPOUT1L_MUTE_N_MASK);
	val |= (WM8994_HPOUT1_VU | WM8994_HPOUT1L_MUTE_N);
	wm8994_write(codec, WM8994_LEFT_OUTPUT_VOLUME, val);

	val = wm8994_read(codec,WM8994_RIGHT_OUTPUT_VOLUME);
	val &= ~(WM8994_HPOUT1R_MUTE_N_MASK);
	val |= (WM8994_HPOUT1_VU | WM8994_HPOUT1R_MUTE_N);
	wm8994_write(codec, WM8994_RIGHT_OUTPUT_VOLUME, val);

	wm8994_write(codec, WM8994_SIDETONE, 0x01C0);

	val = (WM8994_SPKRVOL_ENA | WM8994_SPKLVOL_ENA | 0x0003);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	wm8994_write(codec, WM8994_AIF2_CLOCKING_1, 0x0019);
}

void wm8994_set_voicecall_tty_hco(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;
        
	int val;


	DEBUG_LOG("");

        wm8994_set_voicecall_tty_common(codec);

	/* Input Path Routing */
	val = wm8994_read(codec, WM8994_INPUT_MIXER_2);
	val &= ~(WM8994_IN1RN_TO_IN1R_MASK);
	val |= (WM8994_IN1RN_TO_IN1R);
	wm8994_write(codec, WM8994_INPUT_MIXER_2, val);

	val = (0x4000 | WM8994_TSHUT_OPDIS | WM8994_MIXINR_ENA_MASK | 
				WM8994_IN2R_ENA_MASK | WM8994_IN1R_ENA_MASK);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_2, val);	

	val = wm8994_read(codec, WM8994_INPUT_MIXER_4);
	val &= ~(WM8994_IN1R_TO_MIXINR_MASK | WM8994_IN1R_MIXINR_VOL_MASK);
	val |= WM8994_IN1R_TO_MIXINR;
	wm8994_write(codec, WM8994_INPUT_MIXER_4, val);

	val = (WM8994_AIF2ADCR_ENA | WM8994_AIF2ADCL_ENA  | WM8994_ADCR_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, val);	


	audio_ctrl_mic_bias_gpio(wm8994->pdata, 1);


	/* Output Path Routing */
	wm8994_write(codec, WM8994_OUTPUT_MIXER_1, WM8994_DAC1L_TO_MIXOUTL);
	wm8994_write(codec, WM8994_OUTPUT_MIXER_2, WM8994_DAC1R_TO_MIXOUTR);

	val = wm8994_read(codec, WM8994_HPOUT2_MIXER);
	val &= ~(WM8994_MIXOUTLVOL_TO_HPOUT2_MASK | 
					WM8994_MIXOUTRVOL_TO_HPOUT2_MASK);
	val |= (WM8994_MIXOUTLVOL_TO_HPOUT2 | WM8994_MIXOUTRVOL_TO_HPOUT2);
	wm8994_write(codec, WM8994_HPOUT2_MIXER, val);


        val = wm8994_read(codec, WM8994_RIGHT_LINE_INPUT_1_2_VOLUME);
	val &= ~WM8994_IN1R_MUTE_MASK;
	val |= WM8994_IN1R_VU;
	wm8994_write(codec, WM8994_RIGHT_LINE_INPUT_1_2_VOLUME, val);

        wm8994_write(codec, WM8994_DAC2_LEFT_VOLUME, 0x01C0);
        wm8994_write(codec, WM8994_DAC2_RIGHT_VOLUME, 0x01C0);
        
        wm8994_write(codec, WM8994_DAC2_MIXER_VOLUMES, 0x018C);

	wm8994_write(codec, WM8994_OUTPUT_MIXER_5, 0x0000);
	wm8994_write(codec, WM8994_OUTPUT_MIXER_6, 0x0000);


        wm8994_set_codec_gain(codec, VOICECALL_MODE, VOICECALL_TTY_HCO);


        /* unmute MIXOUT */
	val = wm8994_read(codec, WM8994_LEFT_OPGA_VOLUME);
	val &= ~WM8994_MIXOUTL_MUTE_N_MASK;
	val |= WM8994_MIXOUT_VU | WM8994_MIXOUTL_MUTE_N;
	wm8994_write(codec, WM8994_LEFT_OPGA_VOLUME, val);

	val = wm8994_read(codec, WM8994_RIGHT_OPGA_VOLUME);
	val &= ~WM8994_MIXOUTR_MUTE_N_MASK;
	val |= WM8994_MIXOUT_VU | WM8994_MIXOUTR_MUTE_N;
	wm8994_write(codec, WM8994_RIGHT_OPGA_VOLUME, val);

	/* Unmute DAC1 */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
	val &= ~WM8994_DAC1L_MUTE_MASK;
	val |= WM8994_DAC1_VU;
	wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, val);

	val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME);
	val &= ~WM8994_DAC1R_MUTE_MASK;
	val |= WM8994_DAC1_VU;
	wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);

	val = wm8994_read(codec, WM8994_HPOUT2_VOLUME);
	val &= ~(WM8994_HPOUT2_MUTE_MASK);
	wm8994_write(codec, WM8994_HPOUT2_VOLUME, val);        

	val = wm8994_read(codec, WM8994_ANTIPOP_1); 
	val &= ~(WM8994_HPOUT2_IN_ENA_MASK);
	val |= (WM8994_HPOUT2_IN_ENA_MASK);
	wm8994_write(codec, WM8994_ANTIPOP_1,val);

	wm8994_write(codec, WM8994_SIDETONE, 0x01C0);

	val = (WM8994_HPOUT2_ENA | WM8994_MICB2_ENA | WM8994_MICB1_ENA | 
				WM8994_VMID_SEL_NORMAL | WM8994_BIAS_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	wm8994_write(codec, WM8994_AIF2_CLOCKING_1, 0x0019);
}

void wm8994_set_voicecall_tty_vco(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;
        
	int val;


	DEBUG_LOG("");

        wm8994_set_voicecall_tty_common(codec);

	/* Input Path Routing */
	val = wm8994_read(codec, WM8994_INPUT_MIXER_2);
	val &= ~(WM8994_IN1LP_TO_IN1L_MASK | WM8994_IN1LN_TO_IN1L_MASK);
	val |= (WM8994_IN1LP_TO_IN1L | WM8994_IN1LN_TO_IN1L);
	wm8994_write(codec, WM8994_INPUT_MIXER_2, val);

	val = (0x4000 | WM8994_TSHUT_OPDIS | WM8994_MIXINL_ENA | 
							WM8994_IN1L_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_2, val);	

	val = wm8994_read(codec, WM8994_INPUT_MIXER_3);
	val &= ~(WM8994_IN1R_TO_MIXINR_MASK | WM8994_IN1R_MIXINR_VOL_MASK);
	val |= (WM8994_IN1R_TO_MIXINR | WM8994_IN1R_MIXINR_VOL);
	wm8994_write(codec, WM8994_INPUT_MIXER_3, val);

	val = (WM8994_AIF2ADCR_ENA |WM8994_AIF2ADCL_ENA |WM8994_ADCL_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, val);	


	audio_ctrl_mic_bias_gpio(wm8994->pdata, 1);


	/* Output Path Routing */
	wm8994_write(codec, WM8994_OUTPUT_MIXER_1, WM8994_DAC1L_TO_HPOUT1L);
	wm8994_write(codec, WM8994_OUTPUT_MIXER_2, WM8994_DAC1R_TO_HPOUT1R);

	val = wm8994_read(codec, WM8994_ANALOGUE_HP_1);
	val &= ~(WM8994_HPOUT1L_RMV_SHORT_MASK | WM8994_HPOUT1L_OUTP_MASK | 
		WM8994_HPOUT1L_DLY_MASK | WM8994_HPOUT1R_RMV_SHORT_MASK | 
		WM8994_HPOUT1R_OUTP_MASK | WM8994_HPOUT1R_DLY_MASK);
	val |= (WM8994_HPOUT1L_RMV_SHORT | WM8994_HPOUT1L_OUTP | 
				WM8994_HPOUT1L_DLY | WM8994_HPOUT1R_RMV_SHORT | 
				WM8994_HPOUT1R_OUTP | WM8994_HPOUT1R_DLY);
	wm8994_write(codec, WM8994_ANALOGUE_HP_1, val);


	val = wm8994_read(codec, WM8994_LEFT_LINE_INPUT_1_2_VOLUME);
	val &= ~WM8994_IN1L_MUTE_MASK;
	val |= WM8994_IN1L_VU;
	wm8994_write(codec, WM8994_LEFT_LINE_INPUT_1_2_VOLUME, val);

        wm8994_write(codec, WM8994_DAC2_LEFT_VOLUME, 0x01C0);
        wm8994_write(codec, WM8994_DAC2_RIGHT_VOLUME, 0x01C0);
                
        wm8994_write(codec, WM8994_DAC2_MIXER_VOLUMES, 0x018C);

	wm8994_write(codec, WM8994_OUTPUT_MIXER_5, 0x0000);
	wm8994_write(codec, WM8994_OUTPUT_MIXER_6, 0x0000);


	wm8994_set_codec_gain(codec, VOICECALL_MODE, VOICECALL_TTY_VCO);


	/* unmute MIXOUT */
	val = wm8994_read(codec, WM8994_LEFT_OPGA_VOLUME);
	val &= ~WM8994_MIXOUTL_MUTE_N_MASK;
	val |= WM8994_MIXOUT_VU | WM8994_MIXOUTL_MUTE_N;
	wm8994_write(codec, WM8994_LEFT_OPGA_VOLUME, val);

	val = wm8994_read(codec, WM8994_RIGHT_OPGA_VOLUME);
	val &= ~WM8994_MIXOUTR_MUTE_N_MASK;
	val |= WM8994_MIXOUT_VU | WM8994_MIXOUTR_MUTE_N;
	wm8994_write(codec, WM8994_RIGHT_OPGA_VOLUME, val);

	/* Unmute DAC1 */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
	val &= ~WM8994_DAC1L_MUTE_MASK;
	val |= WM8994_DAC1_VU;
	wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, val);

	val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME);
	val &= ~WM8994_DAC1R_MUTE_MASK;
	val |= WM8994_DAC1_VU;
	wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);

	val = wm8994_read(codec,WM8994_LEFT_OUTPUT_VOLUME);
	val &= ~(WM8994_HPOUT1L_MUTE_N_MASK);
	val |= (WM8994_HPOUT1_VU | WM8994_HPOUT1L_MUTE_N);
	wm8994_write(codec, WM8994_LEFT_OUTPUT_VOLUME, val);

	val = wm8994_read(codec,WM8994_RIGHT_OUTPUT_VOLUME);
	val &= ~(WM8994_HPOUT1R_MUTE_N_MASK);
	val |= (WM8994_HPOUT1_VU | WM8994_HPOUT1R_MUTE_N);
	wm8994_write(codec, WM8994_RIGHT_OUTPUT_VOLUME, val);

	wm8994_write(codec, WM8994_SIDETONE, 0x01C0);

	val = (WM8994_SPKRVOL_ENA | WM8994_SPKLVOL_ENA | 0x0003);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	wm8994_write(codec, WM8994_AIF2_CLOCKING_1, 0x0019);
}
#endif


int wm8994_set_codec_gain(struct snd_soc_codec *codec, u16 mode, u32 device)
{
	struct wm8994_priv *wm8994 = codec->drvdata;
	struct gain_info_t *default_gain_table_p = NULL;
	u32 gain_set_bits = COMMON_SET_BIT;
	u16 val;
	int table_num = 0;
	int i;

	if (wm8994->testmode_config_flag) {
		pr_info("%s: Testmode: skip to set codec gain\n", __func__);
		return 0;
	}

	if (mode == PLAYBACK_MODE) {
		default_gain_table_p = playback_gain_table;
		table_num = PLAYBACK_GAIN_NUM;

		switch (device) {
		case PLAYBACK_RCV:
			gain_set_bits |= PLAYBACK_RCV;
			break;
		case PLAYBACK_SPK:
			gain_set_bits |= PLAYBACK_SPK;
			break;
		case PLAYBACK_HP:
			gain_set_bits |= PLAYBACK_HP;
			break;
		case PLAYBACK_HP_NO_MIC:
			gain_set_bits |= PLAYBACK_HP_NO_MIC;
			break;
		case PLAYBACK_BT:
			gain_set_bits |= PLAYBACK_BT;
			break;
		case PLAYBACK_SPK_HP:
			gain_set_bits |= PLAYBACK_SPK_HP;
			break;
		case PLAYBACK_EXTRA_DOCK:
			gain_set_bits |= PLAYBACK_EXTRA_DOCK;
			break;
		case PLAYBACK_RING_SPK:
			gain_set_bits |= (PLAYBACK_SPK | PLAYBACK_RING_SPK);
			break;
		case PLAYBACK_RING_HP:
			gain_set_bits |= (PLAYBACK_HP | PLAYBACK_RING_HP);
			break;
		case PLAYBACK_RING_SPK_HP:
			gain_set_bits |= (PLAYBACK_SPK_HP |
					PLAYBACK_RING_SPK_HP);
			break;
		case PLAYBACK_VOIP_RCV:
			gain_set_bits |= (PLAYBACK_RCV | PLAYBACK_VOIP_RCV);
			break;
		case PLAYBACK_VOIP_SPK:
			gain_set_bits |= (PLAYBACK_SPK | PLAYBACK_VOIP_SPK);
			break;
		case PLAYBACK_VOIP_HP:
			gain_set_bits |= (PLAYBACK_HP |	PLAYBACK_VOIP_HP);
			break;
		case PLAYBACK_VOIP_BT:
			gain_set_bits |= (PLAYBACK_BT |	PLAYBACK_VOIP_BT);
			break;
		default:
			pr_err("%s: playback gain flag is wrong\n", __func__);
			break;
		}
	} else if (mode == VOICECALL_MODE) {
		default_gain_table_p = voicecall_gain_table;
		table_num = VOICECALL_GAIN_NUM;

		switch (device) {
		case VOICECALL_RCV:
			gain_set_bits |= VOICECALL_RCV;
			break;
		case VOICECALL_SPK:
			gain_set_bits |= VOICECALL_SPK;
			break;
		case VOICECALL_HP:
			gain_set_bits |= VOICECALL_HP;
			break;
		case VOICECALL_HP_NO_MIC:
			gain_set_bits |= VOICECALL_HP_NO_MIC;
			break;
		case VOICECALL_BT:
			gain_set_bits |= VOICECALL_BT;
			break;
#ifdef FEATURE_TTY                        
		case VOICECALL_TTY_FULL:
			gain_set_bits |= VOICECALL_TTY_FULL;
			break;
		case VOICECALL_TTY_HCO:
			gain_set_bits |= VOICECALL_TTY_HCO;
			break;
		case VOICECALL_TTY_VCO:
			gain_set_bits |= VOICECALL_TTY_VCO;
			break;
#endif                        
		default:
			pr_err("%s: voicecall gain flag is wrong\n", __func__);
		}
	} else if (mode  == RECORDING_MODE) {
		default_gain_table_p = recording_gain_table;
		table_num = RECORDING_GAIN_NUM;

		switch (device) {
		case RECORDING_MAIN:
			gain_set_bits |= RECORDING_MAIN;
			break;
		case RECORDING_MAIN2:
			gain_set_bits |= RECORDING_MAIN2;
			break;                        
		case RECORDING_HP:
			gain_set_bits |= RECORDING_HP;
			break;
		case RECORDING_BT:
			gain_set_bits |= RECORDING_BT;
			break;
		case RECORDING_REC_MAIN:
			gain_set_bits |= RECORDING_REC_MAIN;
			break;
		case RECORDING_REC_MAIN2:
			gain_set_bits |= RECORDING_REC_MAIN2;
			break;                        
		case RECORDING_REC_HP:
			gain_set_bits |= RECORDING_REC_HP;
			break;
		case RECORDING_REC_BT:
			gain_set_bits |= RECORDING_REC_BT;
			break;
		case RECORDING_CAM_MAIN:
			gain_set_bits |= RECORDING_CAM_MAIN;
			break;
		case RECORDING_CAM_MAIN2:
			gain_set_bits |= RECORDING_CAM_MAIN2;
			break;                        
		case RECORDING_CAM_HP:
			gain_set_bits |= RECORDING_CAM_HP;
			break;
		case RECORDING_CAM_BT:
			gain_set_bits |= RECORDING_CAM_BT;
			break;
		case RECORDING_VOIP_MAIN:
			gain_set_bits |= RECORDING_VOIP_MAIN;
			break;
		case RECORDING_VOIP_MAIN2:
			gain_set_bits |= RECORDING_VOIP_MAIN2;
			break;                        
		case RECORDING_VOIP_HP:
			gain_set_bits |= RECORDING_VOIP_HP;
			break;
		case RECORDING_VOIP_BT:
			gain_set_bits |= RECORDING_VOIP_BT;
			break;
		default:
			pr_err("%s: recording gain flag is wrong\n", __func__);
		}

	} else if (mode == FMRADIO_MODE) {
		default_gain_table_p = fmradio_gain_table;
		table_num = FMRADIO_GAIN_NUM;

		switch (device) {
		case FMRADIO_HP:
			gain_set_bits |= FMRADIO_HP;
			break;
		case FMRADIO_SPK:
			gain_set_bits |= FMRADIO_SPK;
			break;
		case FMRADIO_SPK_HP:
			gain_set_bits |= FMRADIO_SPK_HP;
			break;
		default:
			pr_err("%s: fmradio gain flag is wrong\n", __func__);
		}
	}

	DEBUG_LOG("Set gain mode = 0x%x, device = 0x%x, gain_bits = 0x%x,\
		table_num=%d\n",
		mode, device, gain_set_bits, table_num);

	/* default gain table setting */
	for (i = 0; i < table_num; i++) {
		if ((default_gain_table_p + i)->mode & gain_set_bits) {
			val = wm8994_read(codec,
				(default_gain_table_p + i)->reg);
			val &= ~((default_gain_table_p + i)->mask);
			val |= (default_gain_table_p + i)->gain;
			wm8994_write(codec, (default_gain_table_p + i)->reg,
				val);
		}
	}

	return 0;

}

