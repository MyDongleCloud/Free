#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for updating firmware [-b -d disk -h -o -u]"
echo "b:		Put bootdelay=1"
echo "d disk:	set /dev/disk[1-2] (sda or mmcblk0p)"
echo "h:		Print this usage and exit"
echo "o:		Do only the first partition"
echo "u:		Do uboot"
exit 0
}

DISK=/dev/sdcard
SECONDPART=1
BOOTDELAY=0
UBOOT=0
SECURITY=1
while getopts bd:hosu opt; do
	case "$opt" in
		b) BOOTDELAY=1;;
		d) DISK="/dev/${OPTARG}";;
		h) helper;;
		o) SECONDPART=0;;
		u) UBOOT=1;;
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

if [ -b ${DISK}0 ]; then
	OFF=${DISK}0
elif [ ! -b ${DISK} -a ${DISK} = "/dev/mmcblk0p" ]; then
		OFF=/dev/mmcblk0
else
	OFF=${DISK}
fi
if [ $UBOOT = 1 ]; then
	IFF=/work/ai.mydonglecloud/private/img/sdcard-bootdelay1-m-s
	dd if=$IFF of=$OFF
	sfdisk -f $OFF << EOF
16384,1048576,c
1064960,14884864,83
EOF
	sync
	sleep 1
fi
mount ${DISK}1 /tmp/1
cp /work/ai.inout/build/linux-artik/arch/arm64/boot/Image /tmp/1
cp /work/ai.inout/build/linux-artik/arch/arm64/boot/dts/nexell/mydonglecd.dtb /tmp/1
umount ${DISK}*
umount ${DISK}*
sync

if [ $SECONDPART = 0 ]; then
	exit 0
fi
mount ${DISK}2 /tmp/2
tar -xjpf /work/ai.inout/private/img/modules-artik.tbz2 -C /tmp/2/lib/modules/
rm -rf /tmp/2/home/mdc/app/
cp -a /work/ai.mydonglecloud/app/ /tmp/2/home/mdc/
chroot /tmp/2 sh -c 'cd home/mdc/app/ && make clean && make'
sleep 2
sync
umount ${DISK}*
umount ${DISK}*
sync
