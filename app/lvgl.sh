#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for pip.sh [-b -c -h]"
echo "b:	do build"
echo "c:	Do clean"
echo "h:	Print this usage and exit"
exit 0
}

BUILD=0
CLEAN=0
while getopts bch opt; do
	case "$opt" in
		b) BUILD=1;;
		c) CLEAN=1;;
		h) helper;;
	esac
done
PW=`pwd`

if [ $BUILD = 1 ]; then
	echo "#####################################"
	echo "BUILD"
	echo "#####################################"
	mkdir -p ../build
	cd ../build
	rm -rf lvgl
	git clone https://github.com/lvgl/lvgl
	cd lvgl
	git checkout a78c4a476ebc58f5f754c25b25d8374c71aa0add
	ln -s ../../app/lv_conf.h lv_conf.h
	mkdir build
	cd build
	cmake ..
	make
	cd $PW
fi

if [ $CLEAN = 1 ]; then
	echo "#####################################"
	echo "CLEAN"
	echo "#####################################"
	cd ../build
	find lvgl \( -name "*.h" -o -name "*.a" \) -print0 | tar -cjpvf a.tbz2 --null -T -
	rm -rf lvgl
	tar -xjpvf a.tbz2
	rm a.tbz2
	cd $PW
fi
