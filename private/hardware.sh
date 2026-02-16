#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for hardware [-h]"
echo "h:	Print this usage and exit"
exit 0
}

while getopts h opt; do
	case "$opt" in
		h) helper;;
	esac
done

#rpi-eeprom-config -e
#BOOT_UART=1
#BOOT_ORDER=0xf2416
#PCIE_PROBE=1
#PSU_MAX_CURRENT=3000
