KDIR := /lib/modules/$(shell uname -r)/build

obj-m += netlinkKernel.o

ccflags-y += -std=gnu11 -g -O0 -Wall -Wextra 

.PHONY: all
all: kernal_module netlinkUser

kernal_module:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

netlinkUser: netlinkUser.o
	$(CC) $(CFLAGS) -o $@ $^

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o *.ko *.mod.* *.cmd .module* modules* Module* .*.cmd .tmp* netlinkUser
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean