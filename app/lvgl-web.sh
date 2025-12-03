#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for lvgl-web [-b -c -h -l -s]"
echo "b:	Build"
echo "c:	Clean"
echo "h:	Print this usage and exit"
echo "l:	Local"
echo "s:	Setup"
exit 0
}

BUILD=0
LOCAL=0
SETUP=0
CLEAN=0
while getopts bchls opt; do
	case "$opt" in
		b) LOCAL=1;BUILD=1;;
		c) CLEAN=1;;
		h) helper;;
		l) LOCAL=1;;
		s) SETUP=1;;
	esac
done
PW=`pwd`

if [ $SETUP = 1 ]; then
	echo "#####################################"
	echo "SETUP"
	echo "#####################################"
	cd /opt
	rm -rf /opt/emsdk
	git clone https://github.com/emscripten-core/emsdk.git
	cd emsdk
	git pull
	./emsdk install latest
	./emsdk activate latest
	LOCAL=1
	BUILD=1
fi

if [ $LOCAL = 1 ]; then
	echo "#####################################"
	echo "LOCAL"
	echo "#####################################"
	echo "PATH=/opt/emsdk:/opt/emsdk/upstream/emscripten:\$PATH"
	PATH=/opt/emsdk:/opt/emsdk/upstream/emscripten:$PATH
	#export PATH=/opt/emsdk:/opt/emsdk/upstream/emscripten:$PATH
fi

#Not used
if [ $BUILD = 2 ]; then
	mkdir -p ../build
	cd ../build
	rm -rf lv_web_emscripten
	git clone --recursive https://github.com/lvgl/lv_web_emscripten.git
	cd lv_web_emscripten
	mkdir cmbuild
	cd cmbuild
	emcmake cmake ..
	emmake make -j
	cd $PW
fi

if [ $BUILD = 1 ]; then
	echo "#####################################"
	echo "BUILD"
	echo "#####################################"
	mkdir -p ../build
	cd ../build
	rm -rf lvgl-web
	git clone https://github.com/lvgl/lvgl lvgl-web
	cd lvgl-web
	git checkout c29651105d7c150789b9c53de7dab8bff67daee4
	ln -s ../../app/lv_conf-web.h lv_conf.h
	echo 'set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -s USE_SDL=2")' >> CMakeLists.txt
	mkdir cmbuild
	cd cmbuild
	emcmake cmake ..
	emmake make -j
	cd $PW
fi

if [ $CLEAN = 1 ]; then
	echo "#####################################"
	echo "CLEAN"
	echo "#####################################"
	cd ../build
	find lvgl-web \( -name "*.h" -o -name "*.a" \) -print0 | tar -cjpvf a.tbz2 --null -T -
	rm -rf lvgl-web
	tar -xjpvf a.tbz2
	rm a.tbz2
	cd $PW
fi
