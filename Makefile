KERNELDIR :=/opt/FriendlyARM/mini6410/linux/linux-2.6.38
PWD       := $(shell pwd)
	
modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	
obj-m	:= dht11.o

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions
