#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for leds [b backlight -h -l mode]"
echo "b:		Turn on off screen backlight"
echo "h:		Print this usage and exit"
echo "l:		Set mode for leds"
echo "		normal: G:heartbeat R:off"
echo "		warning: G:flash R:flash"
echo "		warning-fast: G:flash-fast R:flash-fast"
echo "		error: G:off R:flash"
echo "		error-fast: G:off R:flash-fast"
echo "		off: G:off R:off"
exit 0
}

LEDS="normal"
BACKLIGHT=1
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
	echo $BACKLIGHT > /dev/dongle_screen/backlight
fi
