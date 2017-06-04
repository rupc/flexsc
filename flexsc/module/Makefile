obj-m = syscall_hooking.o

KERNEL_DIR = /lib/modules/$(shell uname -r)/build
PWD = $(shell pwd)
ALREADY_INSTERTTED=$(shell lsmod | grep syscall_hooking | awk '{print $1;}')

default :
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD) modules

run :
ifneq ($(ALREADY_INSTERTTED),)
	sudo rmmod syscall_hooking
endif
	sudo insmod syscall_hooking.ko
	dmesg

rm :
	sudo rmmod syscall_hooking
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD) clean

clean:
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD) clean

