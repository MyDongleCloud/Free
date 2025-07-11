#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for initramfs [-h]"
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
	echo "No /dev${DISK}1..."
	exit 0
fi

rm -f /work/ai.mydonglecloud/private/img/initramfs_2712 /tmp/initramfs_
rm -rf /tmp/initramfs
cd /tmp
umount ${DISK}*
umount ${DISK}*
mount ${DISK}2 /tmp/2
zstd -d /work/ai.mydonglecloud/private/img/initramfs_2712.orig -o /tmp/initramfs_
mkdir /tmp/initramfs
cd /tmp/initramfs
cat /tmp/initramfs_ | cpio -idmv
rm -f /tmp/initramfs_
KERNEL=`ls /tmp/2/lib/modules/ | egrep ^6`
mkdir -p /tmp/initramfs/usr/lib/modules/$KERNEL/kernel/fs/squashfs
mkdir -p /tmp/initramfs/usr/lib/modules/$KERNEL/kernel/fs/overlayfs
unxz -k /tmp/2/lib/modules/$KERNEL/kernel/fs/squashfs/squashfs.ko.xz
mv /tmp/2/lib/modules/$KERNEL/kernel/fs/squashfs/squashfs.ko /tmp/initramfs/usr/lib/modules/$KERNEL/kernel/fs/squashfs
unxz -k /tmp/2/lib/modules/$KERNEL/kernel/fs/overlayfs/overlay.ko.xz
mv /tmp/2/lib/modules/$KERNEL/kernel/fs/overlayfs/overlay.ko /tmp/initramfs/usr/lib/modules/$KERNEL/kernel/fs/overlayfs
patch -p1 < /work/ai.mydonglecloud/private/initramfs.patch
find . | cpio -o -H newc > /tmp/initramfs_
cd /tmp
rm -rf /tmp/initramfs
zstd /tmp/initramfs_ -o /work/ai.mydonglecloud/private/img/initramfs_2712
chmod 755 /work/ai.mydonglecloud/private/img/initramfs_2712
rm -f /tmp/initramfs_
umount ${DISK}*
umount ${DISK}*
