#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for otg.sh [-h -i -l moed]"
echo "h:		Print this usage and exit"
echo "i:		Do init"
echo "l:		Set mode for leds"
exit 0
}

if [ "m`id -u`" != "m0" ]; then
	echo "You need to be root"
	exit 0
fi

INIT=0
LEDS="off"
while getopts hil: opt
do
	case "$opt" in
		h) helper;;
		i) init=1;;
		l) LEDS=${OPTARG};;
	esac
done

if [ $OPTIND = 1 ]; then
	INIT=1
	LEDS="normal"
fi

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
else
	echo timer > /sys/class/leds/LED_GREEN/trigger
	echo 0 > /sys/class/leds/LED_GREEN/delay_off
	echo 10000 > /sys/class/leds/LED_GREEN/delay_on
	echo timer > /sys/class/leds/LED_RED/trigger
	echo 0 > /sys/class/leds/LED_RED/delay_off
	echo 10000 > /sys/class/leds/LED_RED/delay_on
fi

if [ $INIT = 1 ]; then
	if [ ! -d /dev/mydonglecloud_platform ]; then
		ln -sf /sys/devices/platform/mydonglecloud /dev/mydonglecloud_platform
		chmod 222 /dev/mydonglecloud_platform/printk
		chmod 222 /dev/mydonglecloud_platform/buzzer
		chmod 222 /dev/mydonglecloud_platform/buzzerClick
		chmod 222 /dev/mydonglecloud_platform/buzzerFreq
		chmod 444 /dev/mydonglecloud_platform/hardwareVersion
		chmod 444 /dev/mydonglecloud_platform/serialNumber

		ln -sf /sys/bus/spi/devices/spi0.0 /dev/mydonglecloud_screen
		chmod 666 /dev/mydonglecloud_screen/backlight
		chmod 666 /dev/mydonglecloud_screen/rotation
		chmod 222 /dev/mydonglecloud_screen/reset
		chmod 222 /dev/mydonglecloud_screen/init
		chmod 222 /dev/mydonglecloud_screen/rect
		chmod 222 /dev/mydonglecloud_screen/update
		chmod 666 /dev/mydonglecloud_screen_f

		/usr/bin/mydonglecloud-otg.sh

		echo "Init done"
	fi
fi
