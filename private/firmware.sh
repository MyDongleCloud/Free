#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for firmware [-f -h -l NB]"
echo "f:		Create final binaries"
echo "h:		Print this usage and exit"
echo "l:		Set loop number"
exit 0
}

DISK=/dev/sdcard
LOSETUP=/dev/loop0
POSTNAME=""
FINAL=0
while getopts fhl: opt; do
	case "$opt" in
		f) POSTNAME="-final";FINAL=1;;
		h) helper;;
		l) LOSETUP=/dev/loop${OPTARG};;
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

rm -f /work/ai.mydonglecloud/private/img/flasher-m${POSTNAME}-s.img /tmp/mdc.zip /tmp/mdc.img
cd /tmp
umount ${DISK}*
umount ${DISK}*
mount ${DISK}1 /tmp/1
mount ${DISK}2 /tmp/2
cp /work/ai.inout/build/linux-artik/arch/arm64/boot/Image /tmp/1/
cp /work/ai.inout/build/linux-artik/arch/arm64/boot/dts/nexell/mydonglecd.dtb /tmp/1/
tar -xjpvf /work/ai.inout/private/img/modules.tbz2 -C /tmp/2/lib/modules/
cd /tmp/1
zip -r /tmp/mdc.zip *
cd /tmp/2
mksquashfs . /tmp/mdc.img
cd /tmp
sync
umount ${DISK}*
umount ${DISK}*
mv /tmp/mdc.zip /work/ai.mydonglecloud/private/img/mdc.zip
mv /tmp/mdc.img /work/ai.mydonglecloud/private/img/mdc.img

#4+2044=2048MB=4+64+remaining
rm -f /work/ai.mydonglecloud/private/img/flasher-m${POSTNAME}-s.img
dd if=/work/ai.mydonglecloud/private/img/sdcard-bootdelay1-m-s of=/work/ai.mydonglecloud/private/img/flasher-m${POSTNAME}-s.img bs=1024
dd if=/dev/zero of=/work/ai.mydonglecloud/private/img/flasher-m${POSTNAME}-s.img bs=1024 count=$((2044 * 1024)) seek=$((4 * 1024)) conv=notrunc
losetup --show ${LOSETUP} /work/ai.mydonglecloud/private/img/flasher-m${POSTNAME}-s.img
sfdisk -f ${LOSETUP} << EOF
8192,131072,c
139264,
EOF
sync
partprobe ${LOSETUP}
sync
losetup -d ${LOSETUP}
sync
sync
losetup --partscan --show ${LOSETUP} /work/ai.mydonglecloud/private/img/flasher-m${POSTNAME}-s.img
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
unzip -d /tmp/1/ /work/ai.mydonglecloud/private/img/mdc.zip
mount ${LOSETUP}p2 /tmp/2
rm -rf /tmp/2/lost+found/
mkdir -p /tmp/2/fs/upper/ /tmp/2/fs_/lower/ /tmp/2/fs_/overlay/ /tmp/2/fs_/work/
cp /work/ai.mydonglecloud/private/img/mdc.img /tmp/2/fs/
sync
sync
umount ${LOSETUP}*
umount ${LOSETUP}*
losetup -d ${LOSETUP}

if [ $FINAL = 1 ]; then
	rm -f /work/ai.mydonglecloud/private/img/firmware.bin
	zip -P MyDongleCloud.MyD0ngleCl0ud -j /work/ai.mydonglecloud/private/img/firmware.bin /work/ai.mydonglecloud/private/img/mdc.zip /work/ai.mydonglecloud/private/img/mdc.img

	cd /work/ai.mydonglecloud/client
	ionic build --prod
	ionic cap sync android --prod

	echo "*******************************************************"
	echo -n "\e[34m"
	echo "cd /work/ai.mydonglecloud/client"
	echo "tar -cjpf a.tbz2 app && scp a.tbz2 gregoire@server:/home/gregoire/ && rm -f a.tbz2"
	echo "cd /work/ai.mydonglecloud/private/img"
	echo "scp firmware.bin flasher-final.img gregoire@server:/home/gregoire"
	echo -n "\e[m"
	echo "*******************************************************"
	echo -n "\e[31m"
	cat <<EOF
cd /var/www/mydongle
rm -rf app && tar -xjpf ~/a.tbz2 && rm ~/a.tbz2
sed -i -e 's|<base href="/"|<base href="/app/"|' app/index.html

RELEASE=`date +'%Y-%m-%m'`
cd /var/www/mydongle/firmware
mkdir -p \$RELEASE
cd \$RELEASE
mv ~/flasher-final.img flasher-1.x-\$RELEASE.img
touch -t \${RELEASE//\-/}1048 f* .f* .l*

cd ..
ln -sf \$RELEASE/firmware-\$RELEASE.bin firmware.bin
ln -sf \$RELEASE/.firmware-\$RELEASE.md5sum firmware.md5sum
ln -sf \$RELEASE/flasher-\$RELEASE.img flasher.img
ln -sf \$RELEASE/.flasher-\$RELEASE.md5sum flasher.md5sum

echo "release: \"\$RELEASE\"" > /var/www/mydongle-support/user/config/data.yaml

chown -R www-data:www-data /var/www/

echo -n "\$RELEASE" > version
echo -n "\$RELEASE" > version-app
EOF
	echo -n "\e[m"
	echo "*******************************************************"
fi
