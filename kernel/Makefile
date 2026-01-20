VERSION := $(shell ls -1 /lib/modules | sort -V | tail -n 1)

obj-m := st7735s.o dongle.o

all:
	make -C /lib/modules/$(VERSION)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(VERSION)/build M=$(PWD) clean

install:
	cp *.ko /lib/modules/$(VERSION)
	depmod -a $(VERSION)
	sh dtb-pro.sh
