zreladdr-y	:= 0x20008000
params_phys-y	:= 0x20000100

# override for Herring
zreladdr-$(CONFIG_MACH_HERRING)	:= 0x30008000
params_phys-$(CONFIG_MACH_HERRING)	:= 0x30000100

# override for Aries
zreladdr-$(CONFIG_MACH_ARIES)	:= 0x30008000
params_phys-$(CONFIG_MACH_ARIES)	:= 0x30000100

# override for Stealthv
zreladdr-$(CONFIG_MACH_STEALTHV)	:= 0x30008000
params_phys-$(CONFIG_MACH_STEALTHV)	:= 0x30000100

# override for Aegis
zreladdr-$(CONFIG_MACH_AEGIS)	:= 0x30008000
params_phys-$(CONFIG_MACH_AEGIS)	:= 0x30000100

# override for Viper
zreladdr-$(CONFIG_MACH_VIPER)	:= 0x30008000
params_phys-$(CONFIG_MACH_VIPER)	:= 0x30000100

# override for Chief
zreladdr-$(CONFIG_MACH_CHIEF)	:= 0x30008000
params_phys-$(CONFIG_MACH_CHIEF)	:= 0x30000100
