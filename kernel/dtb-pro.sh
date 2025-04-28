#!/bin/sh

dtc -@ -I dts -O dtb -o /boot/firmware/overlays/mydonglecloud.dtbo mydonglecloud-overlay.dts
dtc -@ -I dts -O dtb -o /boot/firmware/overlays/buttons.dtbo buttons-overlay.dts
dtc -@ -I dts -O dtb -o /boot/firmware/overlays/leds.dtbo leds-overlay.dts
dtc -@ -I dts -O dtb -o /boot/firmware/overlays/st7735s.dtbo st7735s-overlay.dts
