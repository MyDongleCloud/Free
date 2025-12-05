#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for initramfs [-h]"
echo "f:	get original initramfs"
echo "h:	Print this usage and exit"
exit 0
}

DISK=/dev/sda
FULL=0
while getopts fh opt; do
	case "$opt" in
		f) FULL=1;;
		h) helper;;
	esac
done

if [ "m`id -u`" != "m0" ]; then
	echo "You need to be root"
	exit 0
fi
cd `dirname $0`
echo "Current directory is now `pwd`"
PP=`pwd`

umount ${DISK}*
umount ${DISK}*
sync
if [ ! -b ${DISK}1 ]; then
	echo "No /dev${DISK}1..."
	exit 0
fi

rm -f ../build/img/initramfs_2712 /tmp/initramfs_
rm -rf /tmp/initramfs
umount ${DISK}*
umount ${DISK}*
if [ $FULL = 1 ]; then
	mount ${DISK}1 /tmp/1
	cp /tmp/1/initramfs_2712 ../build/img/initramfs_2712.orig
	umount /${DISK}1
fi
mount ${DISK}2 /tmp/2
zstd -q -d ../build/img/initramfs_2712.orig -o /tmp/initramfs_
mkdir /tmp/initramfs
cd /tmp/initramfs
cat /tmp/initramfs_ | cpio -idmv 2> /dev/null
if [ $? != 0 ]; then
	echo "Error"
fi
rm -f /tmp/initramfs_
KERNEL=`ls /tmp/2/lib/modules/ | egrep ^6`
mkdir -p /tmp/initramfs/usr/lib/modules/$KERNEL/kernel/fs/squashfs
mkdir -p /tmp/initramfs/usr/lib/modules/$KERNEL/kernel/fs/overlayfs
unxz -k /tmp/2/lib/modules/$KERNEL/kernel/fs/squashfs/squashfs.ko.xz
mv /tmp/2/lib/modules/$KERNEL/kernel/fs/squashfs/squashfs.ko /tmp/initramfs/usr/lib/modules/$KERNEL/kernel/fs/squashfs
unxz -k /tmp/2/lib/modules/$KERNEL/kernel/fs/overlayfs/overlay.ko.xz
mv /tmp/2/lib/modules/$KERNEL/kernel/fs/overlayfs/overlay.ko /tmp/initramfs/usr/lib/modules/$KERNEL/kernel/fs/overlayfs
patch -p1 < ${PP}/initramfs.patch
find . | cpio -o -H newc > /tmp/initramfs_
cd ${PP}
rm -rf /tmp/initramfs
zstd -q /tmp/initramfs_ -o ../build/img/initramfs_2712
chmod 755 ../build/img/initramfs_2712
rm -f /tmp/initramfs_
umount ${DISK}*
umount ${DISK}*
