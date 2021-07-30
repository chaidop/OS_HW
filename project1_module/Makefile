obj-m := project1-kyber.o

KERNELDIR ?= /lib/modules/$(shell uname -r)/build/
PWD := $(shell pwd)
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	rm -f *.txt *.o *.ko *.mod.c
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
