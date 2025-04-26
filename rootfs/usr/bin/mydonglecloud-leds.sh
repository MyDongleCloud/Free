#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for otg.sh [b backlight -h -l mode]"
echo "b:		Turn on off screen backlight"
echo "h:		Print this usage and exit"
echo "l:		Set mode for leds"
exit 0
}

if [ "m`id -u`" != "m0" ]; then
	echo "You need to be root"
	exit 0
fi

LEDS=""
BACKLIGHT=-1
while getopts b:hl: opt
do
	case "$opt" in
		b) BACKLIGHT=${OPTARG};;
		h) helper;;
		l) LEDS=${OPTARG};;
	esac
done

if [ $LEDS = "normal" ]; then
	echo heartbeat > /sys/class/leds/LED_GREEN/trigger
	echo timer > /sys/class/leds/LED_RED/trigger
	echo 0 > /sys/class/leds/LED_RED/delay_off
	echo 10000 > /sys/class/leds/LED_RED/delay_on
elif [ $LEDS = "warning" ]; then
	echo timer > /sys/class/leds/LED_GREEN/trigger
	echo 500 > /sys/class/leds/LED_GREEN/delay_off
	echo 500 > /sys/class/leds/LED_GREEN/delay_on
	echo timer > /sys/class/leds/LED_RED/trigger
	echo 500 > /sys/class/leds/LED_RED/delay_off
	echo 500 > /sys/class/leds/LED_RED/delay_on
elif [ $LEDS = "warning-fast" ]; then
	echo timer > /sys/class/leds/LED_GREEN/trigger
	echo 50 > /sys/class/leds/LED_GREEN/delay_off
	echo 50 > /sys/class/leds/LED_GREEN/delay_on
	echo timer > /sys/class/leds/LED_RED/trigger
	echo 50 > /sys/class/leds/LED_RED/delay_off
	echo 50 > /sys/class/leds/LED_RED/delay_on
elif [ $LEDS = "error" ]; then
	echo timer > /sys/class/leds/LED_GREEN/trigger
	echo 0 > /sys/class/leds/LED_GREEN/delay_off
	echo 10000 > /sys/class/leds/LED_GREEN/delay_on
	echo timer > /sys/class/leds/LED_RED/trigger
	echo 500 > /sys/class/leds/LED_RED/delay_off
	echo 500 > /sys/class/leds/LED_RED/delay_on
elif [ $LEDS = "error-fast" ]; then
	echo timer > /sys/class/leds/LED_GREEN/trigger
	echo 0 > /sys/class/leds/LED_GREEN/delay_off
	echo 10000 > /sys/class/leds/LED_GREEN/delay_on
	echo timer > /sys/class/leds/LED_RED/trigger
	echo 50 > /sys/class/leds/LED_RED/delay_off
	echo 50 > /sys/class/leds/LED_RED/delay_on
elif [ $LEDS = "off" ]; then
	echo timer > /sys/class/leds/LED_GREEN/trigger
	echo 0 > /sys/class/leds/LED_GREEN/delay_off
	echo 10000 > /sys/class/leds/LED_GREEN/delay_on
	echo timer > /sys/class/leds/LED_RED/trigger
	echo 0 > /sys/class/leds/LED_RED/delay_off
	echo 10000 > /sys/class/leds/LED_RED/delay_on
fi

if [ $BACKLIGHT = 0 -o $BACKLIGHT = 1 ]; then
	echo $BACKLIGHT > /dev/mydonglecloud_screen/backlight
fi
