
EXTRA_CFLAGS += -I$(PRJROOT)/modules/include

obj-m := dpram_recovery.o

all:
	@$(MAKE) --no-print-directory -C $(KDIR) SUBDIRS=$(CURDIR) modules

clean:
	rm -f *.ko *.mod.c *.mod.o *~ .*.cmd .*.o.cmd Module.symvers modules.order $(obj-m)

install:
	@$(MAKE) --no-print-directory -C $(KDIR) SUBDIRS=$(CURDIR) modules_install
