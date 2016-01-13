obj-m := hygrothermograph.o  
KDIR :=/opt/FriendlyARM/tiny4412/android/linux-3.0.86
all:  
	make -C $(KDIR) M=$(shell pwd) modules  
clean:  
	rm -f *.ko *.mod.c *.o *.mod.o *.symvers *.bak *.order
