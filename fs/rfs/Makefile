#
# Makefile for the linux rfs-filesystem
#

ifeq ($(CONFIG_GCOV_PROFILE),y)
ifeq ($(PATCHLEVEL),4)
include Makefile.gcov
else
include $(srctree)/fs/rfs/Makefile.gcov
endif
# In profile mode, we always turn on log replay test without power-off
EXTRA_CFLAGS   += -D_RFS_INTERNAL_RECOVERY_TEST
endif

#
# RFS internal configurations
#

# trace writing block
#EXTRA_CFLAGS += -D_RFS_INTERNAL_STAT_BH

# Flush user cache
#EXTRA_CFLAGS  += -D_RFS_INTERNAL_FLUSH_CACHE

# Sanity check
#EXTRA_CFLAGS  += -D_RFS_INTERNAL_SANITY_CHECK

ifeq ($(PATCHLEVEL),4)
include Makefile.24
else
include $(srctree)/fs/rfs/Makefile.26
endif
