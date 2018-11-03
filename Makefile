ifneq ($(KERNELRELEASE),) 
obj-m := ioctl_basic.o
	
else

	KERNELDIR ?= /lib/modules/$(shell uname -r)/build 

	PWD := $(shell pwd)

default: 
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	 
endif


clean:
	rmmod ioctl_basic.ko
	rm user_app 
	rm ioctl_basic.ko
	rm /dev/temp
	

user:
	gcc user_basic_ioctl.c -o user_app
	insmod ioctl_basic.ko
	

