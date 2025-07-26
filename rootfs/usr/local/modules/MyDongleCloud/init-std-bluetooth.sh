#!/bin/sh

cd /etc/firmware/
rfkill unblock bluetooth
./brcm_patchram_plus --patchram BCM4345C0_003.001.025.0111.0205.hcd --no2bytes --baudrate 3000000 --use_baudrate_for_download /dev/ttySAC1 --enable_lpm &
sleep 1
killall brcm_patchram_plus
./brcm_patchram_plus --patchram BCM4345C0_003.001.025.0111.0205.hcd --no2bytes --baudrate 3000000 --use_baudrate_for_download /dev/ttySAC1 --enable_lpm
/usr/bin/hciattach /dev/ttySAC1 -s 3000000 bcm43xx 3000000 flow
hciconfig hci0 up pscan
