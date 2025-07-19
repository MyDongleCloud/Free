#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for install [-h -p]"
echo "h:		Print this usage and exit"
echo "p:		Do production image"
exit 0
}

PROD=0
while getopts hp opt; do
	case "$opt" in
		h) helper;;
		p) PROD=1;;
	esac
done

if [ "m`id -u`" != "m0" ]; then
	echo "You need to be root"
	exit 0
fi

#On PC
#tar -cjpvf a.tbz2 app/ kernel/ rootfs/ screenAvr/ private/install.sh
#scp a.tbz2 mdc@192.168.10.41:/tmp
#On device
#tar -xjpvf /tmp/a.tbz2

lsb_release -a | grep bookworm
if [ $? = 0 ]; then
	OS="pios"
fi
lsb_release -a | grep noble
if [ $? = 0 ]; then
	OS="ubuntu"
fi

echo "################################"
echo "Start"
echo "################################"
date

echo "################################"
echo "Initial"
echo "################################"
cd /home/mdc
mkdir /home/mdc/build
sed -i -e 's|/root|/home/mdc|' /etc/passwd
rm -rf /root/
sed -i -e 's|# "\\e\[5~": history-search-backward|"\\e\[5~": history-search-backward|' /etc/inputrc
sed -i -e 's|# "\\e\[6~": history-search-forward|"\\e\[6~": history-search-forward|' /etc/inputrc
sed -i -e 's|%sudo	ALL=(ALL:ALL) ALL|%sudo	ALL=(ALL:ALL) NOPASSWD:ALL|' /etc/sudoers
sed -i -e 's|HISTSIZE=.*|HISTSIZE=-1|' /home/mdc/.bashrc
sed -i -e 's|HISTFILESIZE=.*|HISTFILESIZE=-1|' /home/mdc/.bashrc
ln -sf /lib/systemd/system/serial-getty@.service /etc/systemd/system/getty.target.wants/serial-getty@ttyGS0.service
ln -sf /etc/systemd/system/mydonglecloud-app.service /etc/systemd/system/multi-user.target.wants/mydonglecloud-app.service
ln -sf /etc/systemd/system/mydonglecloud-init.service /etc/systemd/system/sysinit.target.wants/mydonglecloud-init.service
echo -n " modules-load=dwc2,libcomposite,configs,mydonglecloud" >> /boot/firmware/cmdline.txt
sed -i -e 's/ root=[^ ]* / root=LABEL=rootfs /' /boot/firmware/cmdline.txt
cat > /boot/firmware/config.txt <<EOF
auto_initramfs=1
arm_64bit=1
arm_boost=1

[all]
dtoverlay=dwc2
dtoverlay=mydonglecloud
dtoverlay=st7735s
dtoverlay=buttons
dtoverlay=leds
dtoverlay=uart2
dtparam=uart0=on
dtparam=spi=on
dtparam=pciex1=1
dtparam=nvme
EOF
cat > /etc/fstab <<EOF
proc            /proc           proc    defaults          0       0
LABEL=rootfs  /               ext4    defaults,noatime  0       1
LABEL=rootfs  /disk           ext4    defaults,noatime  0       1
EOF
fatlabel /dev/mmcblk0p1 bootfs
e2label /dev/mmcblk0p2 rootfs
adduser --comment Administrator --disabled-password admin
mkdir /disk

echo "################################"
echo "Fix locale"
echo "################################"
sed -i -e 's/# en_US.UTF-8 UTF-8/en_US.UTF-8 UTF-8/' /etc/locale.gen
locale-gen

echo "################################"
echo "Upgrade"
echo "################################"
apt-get update
apt-get -y upgrade

echo "################################"
echo "Basic"
echo "################################"
apt-get -y install liboath-dev libinput-dev libboost-dev libboost-system-dev libboost-thread-dev libboost-filesystem-dev libcurl4-openssl-dev libssl-dev libbluetooth-dev
apt-get -y install composer apache2 php php-mysql libapache2-mod-php sqlite3 postfix procmail rspamd dovecot-pop3d dovecot-imapd certbot python3-certbot-apache transgui python3-setuptools python3-attr python3-wheel python3-wheel-whl python3-intelhex
apt-get -y install evtest qrencode dos2unix lrzsz imagemagick squashfs-tools libpam-oath oathtool cryptsetup-bin cmake lsof fscrypt libpam-fscrypt
if [ $OS = "ubuntu" ]; then
	chmod a-x /etc/update-motd.d/*
	snap remove snapd
	apt-get -y purge snapd
	apt-get -y install bzip2 zip gpiod net-tools wireless-tools build-essential
fi

echo "################################"
echo "pcpp"
echo "################################"
if [ $OS = "ubuntu" ]; then
	apt-get -y install python3-pcpp
	ln -sf pcpp-python /usr/bin/pcpp
elif [ $OS = "pios" ]; then
	cd /home/mdc/build
	wget https://files.pythonhosted.org/packages/41/07/876153f611f2c610bdb8f706a5ab560d888c938ea9ea65ed18c374a9014a/pcpp-1.30.tar.gz
	tar -xpf pcpp-1.30.tar.gz
	cd pcpp-1.30
	python setup.py install
fi

echo "################################"
echo "pymcuprog"
echo "################################"
cd /home/mdc/build
git clone https://github.com/microchip-pic-avr-tools/pymcuprog
cd pymcuprog
cat > setup.py <<EOF
from setuptools import setup
if __name__ == '__main__':
    setup()
EOF
python setup.py install
cd ../..

echo "################################"
echo "Mysql"
echo "################################"
if [ $OS = "ubuntu" ]; then
	apt-get -y install mysql-server
elif [ $OS = "pios" ]; then
	apt-get -y install libaio1 libevent-pthreads-2.1-7 libmecab2
	cd /home/mdc/build
	wget https://ports.ubuntu.com/pool/main/i/icu/libicu70_70.1-2ubuntu1_arm64.deb
	wget https://ports.ubuntu.com/pool/main/p/protobuf/libprotobuf-lite23_3.12.4-1ubuntu7.22.04.4_arm64.deb
	wget https://ports.ubuntu.com/pool/main/m/mysql-defaults/mysql-common_5.8+1.0.8_all.deb
	wget https://ports.ubuntu.com/pool/main/m/mysql-8.0/mysql-server-core-8.0_8.0.42-0ubuntu0.22.04.1_arm64.deb
	wget https://ports.ubuntu.com/pool/main/m/mysql-8.0/mysql-server-8.0_8.0.42-0ubuntu0.22.04.1_arm64.deb
	wget https://ports.ubuntu.com/pool/main/m/mysql-8.0/mysql-client-core-8.0_8.0.42-0ubuntu0.22.04.1_arm64.deb
	wget https://ports.ubuntu.com/pool/main/m/mysql-8.0/mysql-client-8.0_8.0.42-0ubuntu0.22.04.1_arm64.deb
	dpkg -i libicu70* libprotobuf-lite23* mysql-common*
	dpkg -i mysql-client* mysql-server*
	cd ..
fi

echo "################################"
echo "Docker"
echo "################################"
curl -fsSL https://download.docker.com/linux/debian/gpg -o /etc/apt/keyrings/docker.asc
if [ $OS = "ubuntu" ]; then
	echo "deb [arch=arm64 signed-by=/etc/apt/keyrings/docker.asc] https://download.docker.com/linux/ubuntu noble stable" > /etc/apt/sources.list.d/docker.list
elif [ $OS = "pios" ]; then
	echo "deb [arch=arm64 signed-by=/etc/apt/keyrings/docker.asc] https://download.docker.com/linux/debian bookworm stable" > /etc/apt/sources.list.d/docker.list
fi
apt-get update
apt-get -y install docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin
usermod -aG docker admin

echo "################################"
echo "Qdrant"
echo "################################"
cd /home/mdc/build
wget https://github.com/qdrant/qdrant/releases/download/v1.14.1/qdrant-aarch64-unknown-linux-musl.tar.gz
tar -xpf qdrant-aarch64-unknown-linux-musl.tar.gz
mv qdrant /usr/bin
chmod a+x /usr/bin/qdrant
cd ..

echo "################################"
echo "RethinkDB"
echo "################################"
cd /home/mdc/build
apt-get -y install libprotobuf32
wget https://download.rethinkdb.com/repository/debian-bookworm/pool/r/rethinkdb/rethinkdb_2.4.4~0bookworm_arm64.deb
dpkg -i rethinkdb*.deb
cd ..

echo "################################"
echo "Trilium Notes"
echo "################################"
cd /home/mdc/build
wget https://github.com/TriliumNext/Notes/releases/download/v0.95.0/TriliumNextNotes-Server-v0.95.0-linux-arm64.tar.xz
tar -xJpf TriliumNextNotes-Server*
mv TriliumNextNotes-Server-0.*/ /usr/local/trilium
rm -rf /usr/local/trilium/node/
mkdir /disk/admin/.trilium/
ln -sf /etc/systemd/system/trilium.service /etc/systemd/system/multi-user.target.wants/trilium.service

echo "################################"
echo "postfix-parser"
echo "################################"
apt-get -y install python3-dotenv python3-pytzdata
cd /home/mdc/build
git clone https://github.com/Privex/python-loghelper
cd python-loghelper
python3 setup.py install
cd ..
git clone https://github.com/Privex/python-helpers
cd python-helpers
python3 setup.py install
cd ../..

echo "################################"
echo "uMTP"
echo "################################"
cd /home/mdc/build
git clone https://github.com/viveris/uMTP-Responder
cd uMTP-Responder
make
cp umtprd /usr/bin
cd ../..

echo "################################"
echo "Kernel"
echo "################################"
if [ $OS = "ubuntu" ]; then
	apt-get -y install linux-headers-raspi linux-image-raspi
elif [ $OS = "pios" ]; then
	apt-get -y install linux-headers-rpi-2712 linux-image-rpi-2712 raspi-utils-core raspi-utils-dt
	apt-get -y purge linux-headers-rpi-v8 linux-image-rpi-v8
	rm -rf /lib/modules/6.12.25+rpt-rpi-* /lib/modules/6.12.34+rpt-rpi-v8/
	rm -f /boot/firmware/bcm2710* /boot/firmware/bcm2711* /boot/firmware/kernel8.img /boot/firmware/initramfs8 /boot/firmware/LICENCE.broadcom /boot/firmware/issue.txt
fi

echo "################################"
echo "Jitsi"
echo "################################"
curl https://download.jitsi.org/jitsi-key.gpg.key | gpg --dearmor > /usr/share/keyrings/jitsi-keyring.gpg
echo "deb [arch=arm64 signed-by=/usr/share/keyrings/jitsi-keyring.gpg] https://download.jitsi.org stable/" > /etc/apt/sources.list.d/jitsi-stable.list
apt-get update
apt-get -y install jitsi-meet

echo "################################"
echo "Node.js"
echo "################################"
cd /home/mdc/build
wget https://nodejs.org/dist/latest-v22.x/node-v22.17.1-linux-arm64.tar.xz
tar -xJpf node-v*
cp -a node-v*/bin/ node-v*/include/ node-v*/lib/ node-v*/share/ /usr/local/
cd ..

echo "################################"
echo "Better-auth"
echo "################################"
cd /home/mdc/build
mkdir better-auth
cd better-auth
npm install better-auth
cd ..

echo "################################"
echo "cc2538-prog"
echo "################################"
cd /home/mdc/build
git clone https://github.com/1248/cc2538-prog/
cd cc2538-prog
make
cd ../..

echo "################################"
echo "Zigbee"
echo "################################"
cd /home/mdc/build
mkdir zigbee2mqtt
cd zigbee2mqtt
npm install zigbee2mqtt
cd ..

echo "################################"
echo "QRCode"
echo "################################"
cd /home/mdc/build
git clone https://code.antopie.org/miraty/libreqr.git
git clone https://github.com/bizzycola/qrcode-generator
git clone https://github.com/mebjas/html5-qrcode
cd ..

echo "################################"
echo "App and rootfs"
echo "################################"
cd /home/mdc
tar -xjpvf /tmp/a.tbz2
cd kernel && make && make install && cd ..
cd app && ./lvgl.sh && make && cd ..
chown -R root:root rootfs
cp -a rootfs/* /
rm -rf rootfs

echo "################################"
echo "Upgrade and cleanup"
echo "################################"
apt-get -y upgrade
apt-get -y autoremove
rm -f /var/cache/apt/archives/*.deb
rm -f /home/mdc/*.deb
rm -rf /root/
rm -rf /lost+found/

sync
sync
echo "################################"
echo "Finish"
echo "################################"
date

if [ $PROD = 1 ]; then
	sed -i -e 's|mdc:[^:]*:|mdc:*:|' /etc/shadow
	sed -i -e 's|mdc:[^:]*:|mdc:*:|' /etc/shadow-
fi
