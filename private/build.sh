#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for build [-b -h -u]"
echo "b:		Do build"
echo "h:		Print this usage and exit"
echo "u:		Do update"
exit 0
}

BUILD=0
UPDATE=0
ROOTFS=../build/rootfs
while getopts bhu opt; do
	case "$opt" in
		b) BUILD=1;;
		h) helper;;
		u) UPDATE=1;;
	esac
done

if [ "m`id -u`" != "m0" ]; then
	echo "You need to be root"
	exit 0
fi
cd `dirname $0`
echo "Current directory is now `pwd`"

if [ ! -d ${ROOTFS} ]; then
	BUILD=1
fi
if [ $BUILD = 1 ]; then
	echo "################################"
	echo "Start"
	echo "################################"
	date

	mkdir -p ../build
	mkdir -p ../private/img
	rm -rf ${ROOTFS}
	apt-get -y install debootstrap qemu-user-static binfmt-support
	mkdir ${ROOTFS}
	echo "################################"
	echo "Debootstrap"
	echo "################################"
	debootstrap --arch=arm64 --foreign noble ${ROOTFS} http://ports.ubuntu.com/ubuntu-ports/
	chroot ${ROOTFS} /debootstrap/debootstrap --second-stage

	echo "################################"
	echo "Begin work"
	echo "################################"
	mount -t proc proc ${ROOTFS}/proc
	mount -t sysfs sys ${ROOTFS}/sys
	mount -o bind /dev ${ROOTFS}/dev
	mount -o bind /dev/pts ${ROOTFS}/dev/pts
	chroot ${ROOTFS} /bin/bash -c 'apt-get update; apt-get -y install software-properties-common; add-apt-repository -y universe; adduser --comment MDC --disabled-password mdc'
	cp -a ../app/ ../kernel/ ../rootfs/ ../screenAvr/ ../private/install.sh ${ROOTFS}/home/mdc
	echo "################################"
	echo "Middle work"
	echo "################################"
	chroot ${ROOTFS} /home/mdc/install.sh -n
	umount ${ROOTFS}/dev/pts
	umount ${ROOTFS}/dev
	umount ${ROOTFS}/sys
	umount ${ROOTFS}/proc
	echo "################################"
	echo "End work"
	echo "################################"

	tar -xjpf /work/ai.inout/private/img/modules-artik.tbz2 -C ${ROOTFS}/lib/modules/
	./firmware.sh -c -n
	echo "################################"
	echo "Finish"
	echo "################################"
	date
	exit
fi

mount -t proc proc ${ROOTFS}/proc
mount -t sysfs sys ${ROOTFS}/sys
mount -o bind /dev ${ROOTFS}/dev
mount -o bind /dev/pts ${ROOTFS}/dev/pts
if [ $UPDATE = 1 ]; then
	rm -rf ${ROOTFS}/home/mdc/app/
	cp -a ../app/ ${ROOTFS}/home/mdc/
	chroot ${ROOTFS} sh -c 'cd home/mdc/app/ && make clean && make'
fi
chroot ${ROOTFS} /bin/bash
umount ${ROOTFS}/dev/pts
umount ${ROOTFS}/dev
umount ${ROOTFS}/sys
umount ${ROOTFS}/proc
