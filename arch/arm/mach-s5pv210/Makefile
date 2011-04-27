# arch/arm/mach-s5pv210/Makefile
#
# Copyright (c) 2010 Samsung Electronics Co., Ltd.
# 		http://www.samsung.com/
#
# Licensed under GPLv2

obj-y				:=
obj-m				:=
obj-n				:=
obj-				:=

# Core support for S5PV210 system

obj-$(CONFIG_CPU_S5PV210)	+= cpu.o init.o clock.o gpio.o dma.o pd.o irq-eint-group.o irq-eint.o

# machine support

obj-$(CONFIG_MACH_SMDKV210)	+= mach-smdkv210.o
obj-$(CONFIG_MACH_SMDKC110)	+= mach-smdkc110.o

obj-$(CONFIG_MACH_S5PC110_P1)	+= mach-p1.o p1-rfkill.o

ifeq ($(CONFIG_ARIES_NTT),y)
obj-$(CONFIG_MACH_S5PC110_ARIES)	+= mach-aries.o aries-rfkill.o
endif
ifeq ($(CONFIG_ARIES_EUR),y)
obj-$(CONFIG_MACH_S5PC110_ARIES)	+= mach-aries.o aries-rfkill.o 
endif

ifeq ($(CONFIG_STEALTHV),y)
obj-$(CONFIG_MACH_S5PC110_ARIES)	+= mach-stealthv.o aries-rfkill.o
endif
ifeq ($(CONFIG_PRESTO),y)
obj-$(CONFIG_MACH_S5PC110_ARIES)	+= mach-presto.o
endif

obj-$(CONFIG_MACH_S5PC110_ARIES)    += aries-watchdog.o
obj-$(CONFIG_S5PV210_PM)	+= pm.o
obj-$(CONFIG_PM_PWR_GATING)	+= powergating.o
obj-$(CONFIG_S5PV210_SETUP_SDHCI)       += setup-sdhci.o

obj-$(CONFIG_CPU_FREQ)	+= cpu-freq.o
obj-$(CONFIG_REGULATOR_MAX8998) += max8998_consumer.o

# Device setup - ADCTS
obj-$(CONFIG_S5PV210_ADCTS)		+= s5pc11x-adcts.o
obj-$(CONFIG_S5P_ADC)			+= adc.o

obj-$(CONFIG_S5PV210_SETUP_FB)		+= setup-fb.o
obj-$(CONFIG_S5PV210_SETUP_FIMC0)	+= setup-fimc0.o
obj-$(CONFIG_S5PV210_SETUP_FIMC1)	+= setup-fimc1.o
obj-$(CONFIG_S5PV210_SETUP_FIMC2)	+= setup-fimc2.o
obj-$(CONFIG_S5PV210_SETUP_CSIS)	+= setup-csis.o

obj-$(CONFIG_PRESTO_DEBUG_LED)	+= presto-led.o

obj-$(CONFIG_CPU_IDLE)		+= cpuidle.o idle2.o
