#
# Makefile for key management
#

#change@wtl.kSingh - enabling ECRYPTFS mode
# added ecryptfs_kernel_payload.c
obj-y := \
	gc.o \
	key.o \
	keyring.o \
	keyctl.o \
	permission.o \
	process_keys.o \
	request_key.o \
	request_key_auth.o \
	user_defined.o \
	ecryptfs_kernel_payload.o

obj-$(CONFIG_KEYS_COMPAT) += compat.o
obj-$(CONFIG_PROC_FS) += proc.o
obj-$(CONFIG_SYSCTL) += sysctl.o
