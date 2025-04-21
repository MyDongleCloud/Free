#!/bin/sh

ln -sf /sys/devices/platform/mydonglecloud /dev/mydonglecloud_platform
chmod 222 /dev/mydonglecloud_platform/printk
chmod 222 /dev/mydonglecloud_platform/buzzer
chmod 222 /dev/mydonglecloud_platform/buzzerClick
chmod 666 /dev/mydonglecloud_platform/buzzerFreq
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
