KDIR := /lib/modules/$(shell uname -r)/build

obj-m += netlinkKernel.o

ccflags-y += -std=gnu11 -g -O0 -Wall -Wextra 

.PHONY: all
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	rm -rf *.o *.ko *.mod.* *.cmd .module* modules* Module* .*.cmd .tmp*
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean