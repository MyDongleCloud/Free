VERSION=$(shell uname -r)
ifeq ($(wildcard /lib/modules/$(VERSION)/.),)
	VERSION=$(shell ls /lib/modules/ | grep 6.12)
endif

obj-m := st7735s.o dongle.o

all:
	make -C /lib/modules/$(VERSION)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(VERSION)/build M=$(PWD) clean

install:
	cp *.ko /lib/modules/$(VERSION)
	depmod -a $(VERSION)
	sh dtb-pro.sh
