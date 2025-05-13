#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for firmware [-h]"
echo "h:		Print this usage and exit"
exit 0
}

DISK=/dev/sdcard
while getopts h opt; do
	case "$opt" in
		h) helper;;
	esac
done

if [ "m`id -u`" != "m0" ]; then
	echo "You need to be root"
	exit 0
fi

umount ${DISK}*
umount ${DISK}*
sync
if [ ! -b ${DISK}1 ]; then
	echo "No ${DISK}1..."
	exit 0
fi

rm -f /tmp/boot /tmp/img
cd /tmp
umount ${DISK}*
umount ${DISK}*
dd if=${DISK}0 of=/tmp/boot bs=1M count=520 status=progress
mount ${DISK}2 /tmp/2
cd /tmp/2
mksquashfs . /tmp/img
cd /tmp
umount ${DISK}*
umount ${DISK}*
