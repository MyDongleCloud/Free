#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for init [-h]"
echo "h:		Print this usage and exit"
exit 0
}

if [ "m`id -u`" != "m0" ]; then
	echo "You need to be root"
	exit 0
fi

while getopts h opt
do
	case "$opt" in
		h) helper;;
	esac
done

if [ `cat /dev/dongle_platform/model` = "Dongle Std" ]; then
	modprobe -r dhd
	modprobe dhd nvram_path=/etc/wifi/nvram.txt firmware_path=/etc/wifi/fw.bin
	/usr/local/modules/_core_/init-std-bluetooth.sh &
fi

echo "Init done"
