ifneq ($(shell uname -r | egrep "6.12"),)
EXTRA_CFLAGS= -DKERNEL612
endif

obj-m := st7735s.o mydonglecloud.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install:
	echo "Install"
	cp *.ko /lib/modules/$(shell uname -r)
	depmod -a
	sh dtbo.sh
