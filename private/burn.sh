#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for burn.sh [-d disk -i img -h]"
echo "d disk:	set /dev/disk"
echo "i img:	set image to burn"
echo "h:		Print this usage and exit"
exit 0
}

DISK=/dev/sda
IMG=/work/ai.mydonglecloud/private/img/flasher-m-s.img
while getopts d:i:h opt; do
	case "$opt" in
		d) DISK="/dev/${OPTARG}";;
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
umount ${DISK}*
umount ${DISK}*
