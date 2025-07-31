#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for update-burnt [-d disk -h -r]"
echo "d disk:	Set /dev/disk (default: /dev/sda)"
echo "h:		Print this usage and exit"
echo "r:		Reset admin"
exit 0
}

DISK=/dev/sda
RESET=0
while getopts d:hr opt; do
	case "$opt" in
		d) DISK="/dev/${OPTARG}";;
		h) helper;;
		r) RESET=1;;
	esac
done

if [ "m`id -u`" != "m0" ]; then
	echo "You need to be root"
	exit 0
fi
cd `dirname $0`
echo "Current directory is now `pwd`"

umount ${DISK}*
umount ${DISK}*
mount ${DISK}2 /tmp/2
if [ $RESET = 1 ]; then
	rm -rf /tmp/2/admin
fi
mkdir /tmp/2b
mount -t squashfs -o loop /tmp/2/fs/mdc.img /tmp/2/fs/lower
mount -t overlay -o lowerdir=/tmp/2/fs/lower,upperdir=/tmp/2/fs/upper,workdir=/tmp/2/fs/work none /tmp/2b

rm -rf /tmp/2b/home/mdc/app/ /tmp/2b/home/mdc/moduleApache2/
cp -a ../app/ ../moduleApache2/ /tmp/2b/home/mdc/
chroot /tmp/2b sh -c 'cd home/mdc/app/ && make clean && make'
chroot /tmp/2b sh -c 'cd home/mdc/moduleApache2/ && make clean && make'

umount /tmp/2b
rmdir /tmp/2b
umount /tmp/2/fs/lower
sync
umount ${DISK}*
umount ${DISK}*
sync

