#
# USB peripheral controller drivers
#
ifeq ($(CONFIG_USB_GADGET_DEBUG),y)
	EXTRA_CFLAGS		+= -DDEBUG
endif

obj-$(CONFIG_USB_DUMMY_HCD)	+= dummy_hcd.o
obj-$(CONFIG_USB_NET2280)	+= net2280.o
obj-$(CONFIG_USB_AMD5536UDC)	+= amd5536udc.o
obj-$(CONFIG_USB_PXA25X)	+= pxa25x_udc.o
obj-$(CONFIG_USB_PXA27X)	+= pxa27x_udc.o
obj-$(CONFIG_USB_IMX)		+= imx_udc.o
obj-$(CONFIG_USB_GOKU)		+= goku_udc.o
obj-$(CONFIG_USB_OMAP)		+= omap_udc.o
obj-$(CONFIG_USB_LH7A40X)	+= lh7a40x_udc.o
obj-$(CONFIG_USB_S3C2410)	+= s3c2410_udc.o
obj-$(CONFIG_USB_S3C_OTGD)	+= s3c_udc_otg.o
obj-$(CONFIG_USB_AT91)		+= at91_udc.o
obj-$(CONFIG_USB_ATMEL_USBA)	+= atmel_usba_udc.o
obj-$(CONFIG_USB_FSL_USB2)	+= fsl_usb2_udc.o
fsl_usb2_udc-objs		:= fsl_udc_core.o
ifeq ($(CONFIG_ARCH_MXC),y)
fsl_usb2_udc-objs		+= fsl_mx3_udc.o
endif
obj-$(CONFIG_USB_M66592)	+= m66592-udc.o
obj-$(CONFIG_USB_R8A66597)	+= r8a66597-udc.o
obj-$(CONFIG_USB_FSL_QE)	+= fsl_qe_udc.o
obj-$(CONFIG_USB_CI13XXX)	+= ci13xxx_udc.o
obj-$(CONFIG_USB_S3C_HSOTG)	+= s3c-hsotg.o
obj-$(CONFIG_USB_LANGWELL)	+= langwell_udc.o

#
# USB gadget drivers
#

C_UTILS =	composite.o usbstring.o config.o epautoconf.o epautoconf_realloc.o

g_zero-objs			:= zero.o
g_audio-objs			:= audio.o
g_ether-objs			:= ether.o
g_serial-objs			:= serial.o
g_midi-objs			:= gmidi.o
gadgetfs-objs			:= inode.o
g_file_storage-objs		:= file_storage.o
g_printer-objs			:= printer.o
g_cdc-objs			:= cdc2.o
ifeq ($(CONFIG_ARIES_NTT),y)
g_android-objs			:= adb_ums_acm_mtp_rndis.o f_adb.o f_mass_storage.o u_serial.o f_acm.o serial_acm.o f_mtp.o fsa9480_i2c.o composite.o $(C_UTILS)
else
#g_android-objs			:= adb_ums_acm_mtp_rndis.o f_adb.o f_mass_storage.o u_serial.o f_acm.o serial_acm.o f_diag.o f_nmea.o u_ether.o	rndis.o	f_rndis.o	f_mtp.o fsa9480_i2c.o composite.o $(C_UTILS)
g_android-objs			:= acm_dm_nmea_cdrom_adb.o f_adb.o f_mass_storage.o u_serial.o f_acm.o serial_acm.o f_diag.o f_nmea.o u_ether.o	rndis.o	f_rndis.o	f_mtp.o fsa9480_i2c.o composite.o $(C_UTILS)
#g_android-objs			:= rndis_acm_dm_nmea_cdrom_adb.o f_adb.o f_mass_storage.o u_serial.o f_acm.o serial_acm.o f_diag.o f_nmea.o u_ether.o	rndis.o	f_rndis.o	f_mtp.o fsa9480_i2c.o composite.o $(C_UTILS)
#g_android-objs			:= rndis_acm_adb.o f_adb.o f_mass_storage.o u_serial.o f_acm.o serial_acm.o f_diag.o f_nmea.o u_ether.o	rndis.o	f_rndis.o	f_mtp.o fsa9480_i2c.o composite.o $(C_UTILS)
endif

obj-$(CONFIG_USB_ZERO)		+= g_zero.o
obj-$(CONFIG_USB_AUDIO)		+= g_audio.o
obj-$(CONFIG_USB_ETH)		+= g_ether.o
obj-$(CONFIG_USB_GADGETFS)	+= gadgetfs.o
obj-$(CONFIG_USB_FILE_STORAGE)	+= g_file_storage.o
obj-$(CONFIG_USB_G_SERIAL)	+= g_serial.o
obj-$(CONFIG_USB_G_PRINTER)	+= g_printer.o
obj-$(CONFIG_USB_MIDI_GADGET)	+= g_midi.o
obj-$(CONFIG_USB_CDC_COMPOSITE) += g_cdc.o
obj-$(CONFIG_USB_ANDROID)	+= g_android.o

