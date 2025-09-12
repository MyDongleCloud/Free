#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for firmware [-c -d disk -f -h -l NB -n]"
echo "c:	Clean build"
echo "d disk:	set /dev/disk[1-2] (sda or mmcblk0p)"
echo "f:	Create final binaries"
echo "h:	Print this usage and exit"
echo "l:	Set loop number"
echo "n:	Not native (via chroot)"
exit 0
}

DISK=/dev/sda
LOSETUP=/dev/loop3
POSTNAME=""
FINAL=0
CLEAN=0
NATIVE=1
while getopts cd:fhl:n opt; do
	case "$opt" in
		c) CLEAN=1;;
		d) DISK="/dev/${OPTARG}";;
		f) POSTNAME="-final";FINAL=1;;
		h) helper;;
		l) LOSETUP=/dev/loop${OPTARG};;
		n) NATIVE=0;;
	esac
done

if [ "m`id -u`" != "m0" ]; then
	echo "You need to be root"
	exit 0
fi
cd `dirname $0`
echo "Current directory is now `pwd`"
PP=`pwd`

if [ $NATIVE = 1 ]; then
	umount ${DISK}*
	umount ${DISK}*
	sync
	if [ ! -b ${DISK}1 ]; then
		echo "No /dev${DISK}1..."
		exit 0
	fi

	umount ${DISK}*
	umount ${DISK}*
	rm -f img/flasher-m${POSTNAME}-s.img img/upgrade.bin img/partition1.zip
	mount ${DISK}1 /tmp/1
	mount ${DISK}2 /tmp/2
	cd /tmp/1
	zip -q -r ${PP}/img/partition1.zip *
	cd ${PP}
	ROOTFS=/tmp/2
else
	ROOTFS=../build/rootfs
fi
if [ $CLEAN = 1 ]; then
	rm -f /tmp/mdc${POSTNAME}.img
fi
if [ -f /tmp/mdc${POSTNAME}.img ]; then
	echo "No creation as /tmp/mdc${POSTNAME}.img already exists"
else
	tar -xjpf /work/ai.inout/private/img/modules-artik.tbz2 -C ${ROOTFS}/lib/modules/
	rm -rf ${ROOTFS}/home/mdc/app/ ${ROOTFS}/home/mdc/moduleApache2/
	cp -a ../app/ ../moduleApache2/ ${ROOTFS}/home/mdc/
	chroot ${ROOTFS} sh -c 'cd home/mdc/app/ && make clean && make'
	chroot ${ROOTFS} sh -c 'cd home/mdc/moduleApache2/ && make clean && make'
	cd ../client
	rm -rf web
	ionic --prod build
	cd -
	rm -rf ${ROOTFS}/usr/local/modules/MyDongleCloud/web
	cp -a ../client/web ${ROOTFS}/usr/local/modules/MyDongleCloud
	cd ../login
	rm -rf login
	ionic --prod build
	cd -
	rm -rf ${ROOTFS}/usr/local/modules/Apache2/pages/login
	cp -a ../login/login ${ROOTFS}/usr/local/modules/Apache2/pages
	rm -rf ${ROOTFS}/var/cache-admin
	mkdir ${ROOTFS}/var/cache-admin
	chown -R 1001:1001 ${ROOTFS}/var/cache-admin
	rm -rf ${ROOTFS}/home/admin.default
	cp -a ${ROOTFS}/disk/admin ${ROOTFS}/home/admin.default
	rm -rf ${ROOTFS}/home/admin.default/.log
	mkdir ${ROOTFS}/home/admin.default/.log
	mkdir ${ROOTFS}/home/admin.default/.log/Apache2
	ln -sf Apache2/ ${ROOTFS}/home/admin.default/.log/apache2
	mkdir ${ROOTFS}/home/admin.default/.log/MyDongleCloud
	mkdir ${ROOTFS}/home/admin.default/.log/Zigbee2MQTT
	chown -R 1001:1001 ${ROOTFS}/home/admin.default
	rm -f /tmp/squashfs-exclude.txt
	if [ $FINAL = 1 ]; then
		cp squashfs-exclude.txt /tmp/squashfs-exclude.txt
	else
		rm -f /tmp/squashfs-exclude.txt
		touch /tmp/squashfs-exclude.txt
	fi
	cat >> /tmp/squashfs-exclude.txt <<EOF
./disk/
EOF
	sed -i -e 's|#LABEL=rootfs  /disk|LABEL=rootfs  /disk|' ${ROOTFS}/etc/fstab
	echo "######## WARNING ########"
	echo "${ROOTFS}/etc/fstab has been modified. It will be reverted once squashfs is finished"
	cd ${ROOTFS}
	mksquashfs . /tmp/mdc${POSTNAME}.img -ef /tmp/squashfs-exclude.txt
	cd ${PP}
	sed -i -e 's|LABEL=rootfs  /disk|#LABEL=rootfs  /disk|' ${ROOTFS}/etc/fstab
	rm -f /tmp/squashfs-exclude.txt
fi
if [ $NATIVE = 1 ]; then
	sync
	umount ${DISK}*
	umount ${DISK}*
fi

dd if=img/sdcard-bootdelay1-m-s of=img/flasher-m${POSTNAME}-s.img bs=1024
SIZE=$((`stat -c %s /tmp/mdc${POSTNAME}.img` * 125 / 100 / 1024))
echo "Size: ${SIZE}kB"
dd if=/dev/zero of=img/flasher-m${POSTNAME}-s.img bs=1024 count=$SIZE seek=$((4 * 1024)) conv=notrunc
losetup --show ${LOSETUP} img/flasher-m${POSTNAME}-s.img
sfdisk -f ${LOSETUP} << EOF
8192,262144,c
270336,
EOF
sync
partprobe ${LOSETUP}
sync
losetup -d ${LOSETUP}
sync
sync
losetup --partscan --show ${LOSETUP} img/flasher-m${POSTNAME}-s.img
if [ $? != 0 ]; then
	echo "ERROR losetup"
	exit 1
fi
mkfs.fat -F 32 ${LOSETUP}p1
fatlabel ${LOSETUP}p1 bootfs
mkfs.ext4 ${LOSETUP}p2
tune2fs -O encrypt ${LOSETUP}p2
e2label ${LOSETUP}p2 rootfs
sync
umount ${LOSETUP}*
umount ${LOSETUP}*
mount ${LOSETUP}p1 /tmp/1
unzip -q -d /tmp/1/ img/partition1.zip
cp img/initramfs_2712 /tmp/1
mount ${LOSETUP}p2 /tmp/2
rm -rf /tmp/2/lost+found/
mkdir -p /tmp/2/fs/upper/ /tmp/2/fs/lower/ /tmp/2/fs/overlay/ /tmp/2/fs/work/
cp /tmp/mdc${POSTNAME}.img /tmp/2/fs/
sync
sync
umount ${LOSETUP}*
umount ${LOSETUP}*
e2fsck -f -p ${LOSETUP}p2
mount ${LOSETUP}p2 /tmp/2
rm -rf /tmp/2/lost+found
umount ${LOSETUP}*
umount ${LOSETUP}*
sync
losetup -d ${LOSETUP}

if [ $FINAL = 1 ]; then
	zip -j img/upgrade.bin img/partition1.zip /tmp/mdc${POSTNAME}.img

	cd ../client
	ionic build --prod
	ionic cap sync android --prod
	cd ${PP}

	echo "*******************************************************"
	echo -n "\e[34m"
	echo "cd /work/ai.mydonglecloud/client"
	echo "tar -cjpf a.tbz2 app && scp a.tbz2 gregoire@server:/home/gregoire/ && rm -f a.tbz2"
	echo "cd /work/ai.mydonglecloud/private/img"
	echo "scp upgrade.bin flasher-m-final-s.img gregoire@server:/home/gregoire"
	echo -n "\e[m"
	echo "*******************************************************"
	echo -n "\e[31m"
	cat <<EOF
cd /var/www/mydonglecloud
rm -rf app && tar -xjpf ~/a.tbz2 && rm ~/a.tbz2
sed -i -e 's|<base href="/"|<base href="/app/"|' app/index.html

RELEASE=`date +'%Y-%m-%m'`
cd /var/www/mydonglecloud/firmware
mkdir -p \$RELEASE
cd \$RELEASE
mv ~/flasher-m-final-s.img flasher-\$RELEASE.img
touch -t \${RELEASE//\-/}1048 f* .f* .l*

cd ..
ln -sf \$RELEASE/upgrade-\$RELEASE.bin upgrade.bin
ln -sf \$RELEASE/.upgrade-\$RELEASE.md5sum upgrade.md5sum
ln -sf \$RELEASE/flasher-\$RELEASE.img flasher.img
ln -sf \$RELEASE/.flasher-\$RELEASE.md5sum flasher.md5sum

echo "release: \"\$RELEASE\"" > /var/www/mydonglecloud-support/user/config/data.yaml

chown -R www-data:www-data /var/www/

echo -n "\$RELEASE" > version
echo -n "\$RELEASE" > version-app
EOF
	echo -n "\e[m"
	echo "*******************************************************"
fi
