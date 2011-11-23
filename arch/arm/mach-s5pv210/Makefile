# arch/arm/mach-s5pv210/Makefile
#
# Copyright (c) 2010 Samsung Electronics Co., Ltd.
#		http://www.samsung.com/
#
# Licensed under GPLv2

obj-y				:=
obj-m				:=
obj-n				:=
obj-				:=

# Core support for S5PV210 system

obj-$(CONFIG_CPU_S5PV210)	+= cpu.o init.o clock.o dma.o gpiolib.o
obj-$(CONFIG_CPU_S5PV210)	+= setup-i2c0.o setup-i2c1.o setup-i2c2.o
obj-$(CONFIG_PM)	+= pm.o sleep.o

obj-$(CONFIG_S5PV210_POWER_DOMAIN)	+= power-domain.o

# machine support

obj-$(CONFIG_MACH_AQUILA)	+= mach-aquila.o
obj-$(CONFIG_MACH_SMDKV210)	+= mach-smdkv210.o
obj-$(CONFIG_MACH_SMDKC110)	+= mach-smdkc110.o
obj-$(CONFIG_MACH_GONI)		+= mach-goni.o
obj-$(CONFIG_MACH_HERRING)	+= mach-herring.o herring-rfkill.o
obj-$(CONFIG_MACH_HERRING)	+= herring-vibrator.o
obj-$(CONFIG_MACH_HERRING)	+= herring-btlpm.o
obj-$(CONFIG_MACH_HERRING)	+= herring-watchdog.o
obj-$(CONFIG_MACH_HERRING)	+= herring-panel.o
#obj-$(CONFIG_MACH_HERRING)	+= dev-herring-phone.o

obj-$(CONFIG_MACH_ARIES)	+= mach-aries.o aries-rfkill.o
obj-$(CONFIG_MACH_ARIES)	+= sec-watchdog.o
obj-$(CONFIG_MACH_ARIES)	+= aries-panel.o
obj-$(CONFIG_MACH_ARIES)    += sec_switch.o
obj-$(CONFIG_MACH_ARIES)	+= dev-s1-phone.o

ifeq ($(CONFIG_TIKAL_USCC),y)
obj-$(CONFIG_MACH_STEALTHV)	+= mach-tikal_uscc.o stealthv-rfkill.o
else
obj-$(CONFIG_MACH_STEALTHV)	+= mach-stealthv.o stealthv-rfkill.o
endif
obj-$(CONFIG_MACH_STEALTHV)	+= sec-watchdog.o
obj-$(CONFIG_MACH_STEALTHV)	+= stealthv-panel.o
obj-$(CONFIG_MACH_STEALTHV) += sec_switch_lte.o
obj-$(CONFIG_MACH_STEALTHV)	+= dev-stealthv-phone.o

obj-$(CONFIG_MACH_AEGIS)	+= mach-aegis.o aegis-rfkill.o
obj-$(CONFIG_MACH_AEGIS)	+= sec-watchdog.o
obj-$(CONFIG_MACH_AEGIS)	+= aegis-panel.o
obj-$(CONFIG_MACH_AEGIS)    += sec_switch_lte.o
obj-$(CONFIG_MACH_AEGIS)	+= dev-aegis-phone.o

obj-$(CONFIG_MACH_VIPER)	+= mach-viper.o viper-rfkill.o
obj-$(CONFIG_MACH_VIPER)	+= sec-watchdog.o
obj-$(CONFIG_MACH_VIPER)	+= viper-panel.o
obj-$(CONFIG_MACH_VIPER)    += sec_switch.o
#obj-$(CONFIG_MACH_VIPER)	+= dev-viper-phone.o

obj-$(CONFIG_MACH_CHIEF)	+= mach-chief.o chief-rfkill.o
obj-$(CONFIG_MACH_CHIEF)	+= sec-watchdog.o
obj-$(CONFIG_MACH_CHIEF)	+= chief-panel.o
obj-$(CONFIG_MACH_CHIEF)    += sec_switch.o
#obj-$(CONFIG_MACH_CHIEF)	+= dev-viper-phone.o


obj-$(CONFIG_CPU_FREQ)		+= cpu-freq.o
obj-$(CONFIG_S5PV210_SETUP_SDHCI)       += setup-sdhci.o

# device support

obj-y				+= dev-audio.o
obj-$(CONFIG_FIQ_DEBUGGER)	+= dev-fiqdbg.o
obj-$(CONFIG_S3C64XX_DEV_SPI)	+= dev-spi.o
obj-$(CONFIG_S5PC110_DEV_ONENAND) += dev-onenand.o


obj-$(CONFIG_S5P_ADC) += adc.o

obj-$(CONFIG_S5PV210_SETUP_FB)		+= setup-fb.o
obj-$(CONFIG_S5PV210_SETUP_FIMC0)	+= setup-fimc0.o
obj-$(CONFIG_S5PV210_SETUP_FIMC1)	+= setup-fimc1.o
obj-$(CONFIG_S5PV210_SETUP_FIMC2)	+= setup-fimc2.o
obj-$(CONFIG_S5PV210_SETUP_CSIS)	+= setup-csis.o


obj-$(CONFIG_CPU_IDLE)		+= cpuidle.o didle.o
