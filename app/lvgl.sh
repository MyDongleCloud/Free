#!/bin/sh

cd ../build
rm -rf lvgl
git clone https://github.com/lvgl/lvgl
cd lvgl
git checkout a78c4a476ebc58f5f754c25b25d8374c71aa0add
ln -s ../../app/lv_conf.h
mkdir build
cd build
cmake ..
make -j
