#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for burn.sh [-d disk -e -i img -h]"
echo "d disk:	set /dev/disk"
echo "e:	extract image"
echo "i img:	set image to burn"
echo "h:		Print this usage and exit"
exit 0
}

DISK=/dev/sda
IMG=/work/ai.mydonglecloud/private/img/flasher-m-s.img
EXTRACT=0
while getopts d:ei:h opt; do
	case "$opt" in
		d) DISK="/dev/${OPTARG}";;
		e) EXTRACT=1;;
		i) IMG="${OPTARG}";;
		h) helper;;
	esac
done

umount ${DISK}*
umount ${DISK}*
if [ -b ${DISK}0 ]; then
	OFF=${DISK}0
elif [ ! -b ${DISK} -a ${DISK} = "/dev/mmcblk0p" ]; then
		OFF=/dev/mmcblk0
else
	OFF=${DISK}
fi
dd if=${IMG} of=${OFF} bs=31M status=progress
sync
umount ${DISK}*
umount ${DISK}*
growpart ${OFF} 2
e2fsck -f -p ${DISK}2
resize2fs ${DISK}2
e2fsck -f -p ${DISK}2
mount ${DISK}2 /tmp/2
if [ $EXTRACT = 1 ]; then
	unsquashfs -f -d /tmp/2 /tmp/2/fs/mdc.img
	rm -rf /tmp/2/fs*
	sync
fi
umount ${DISK}*
umount ${DISK}*
