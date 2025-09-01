#!/bin/sh

helper() {
echo "*******************************************************"
echo "usage of dtb-pro [-h -l]"
echo "h:	Print this usage and exit"
echo "l:	Do local"
exit 0
}

PRE="/boot/firmware"
while getopts hl opt; do
	case "$opt" in
		h) helper;;
		l) PRE="/tmp/1";;
	esac
done

if [ ! -f mydonglecloud-overlay.dts ]; then
	echo "Not in the right directory"
	exit 0
fi
dtc -@ -I dts -O dtb -o ${PRE}/overlays/mydonglecloud.dtbo mydonglecloud-overlay.dts
dtc -@ -I dts -O dtb -o ${PRE}/overlays/buttons.dtbo buttons-overlay.dts
dtc -@ -I dts -O dtb -o ${PRE}/overlays/leds.dtbo leds-overlay.dts
dtc -@ -I dts -O dtb -o ${PRE}/overlays/st7735s.dtbo st7735s-overlay.dts
