#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for pip.sh [-b -h]"
echo "h:	Print this usage and exit"
echo "v:	Set version"
exit 0
}

BUILD=0
while getopts bh opt; do
	case "$opt" in
		h) helper;;
		b) BUILD=1;;
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
	make -j
	cd $PW
fi
