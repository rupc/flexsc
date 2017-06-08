name=syshook
obj-m := $(addsuffix .o,$(name))
# syshook-objs := print.o

KERNEL_DIR = /lib/modules/$(shell uname -r)/build
PWD = $(shell pwd)
ALREADY_INSTERTTED=$(shell lsmod | grep $(name) | awk '{print $$1;}')

ccflags-y=-I$(PWD)


default :
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD) modules

run :
ifneq ($(ALREADY_INSTERTTED),)
	sudo rmmod $(name)
endif
	sudo insmod $(name).ko
	# dmesg
	../user/user-program

user:


rm :
	sudo rmmod $(name)
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD) clean
	rm $(obj) $(exe)

clean:
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD) clean

