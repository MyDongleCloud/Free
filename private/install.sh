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

lsb_release -a | grep bookworm
if [ $? = 0 ]; then
	OS="pios"
fi
lsb_release -a | grep noble
if [ $? = 0 ]; then
	OS="ubuntu"
fi

echo "################################"
echo "Starter"
echo "################################"
mkdir /home/mdc/build
sed -i -e 's|/root|/home/mdc|' /etc/passwd
rm -rf /root
sed -i -e 's|# "\\e\[5~": history-search-backward|"\\e\[5~": history-search-backward|' /etc/inputrc
sed -i -e 's|# "\\e\[6~": history-search-forward|"\\e\[6~": history-search-forward|' /etc/inputrc
sed -i -e 's|%sudo	ALL=(ALL:ALL) ALL|%sudo	ALL=(ALL:ALL) NOPASSWD:ALL|' /etc/sudoers
sed -i -e 's|HISTSIZE=.*|HISTSIZE=-1|' /home/mdc/.bashrc
sed -i -e 's|HISTFILESIZE=.*|HISTFILESIZE=-1|' /home/mdc/.bashrc
ln -sf /lib/systemd/system/serial-getty@.service /etc/systemd/system/getty.target.wants/serial-getty@ttyGS0.service
ln -sf /etc/systemd/system/mydonglecloud-app.service /etc/systemd/system/multi-user.target.wants/mydonglecloud-app.service
ln -sf /etc/systemd/system/mydonglecloud-init.service /etc/systemd/system/sysinit.target.wants/mydonglecloud-init.service
echo -n " modules-load=dwc2,libcomposite,configs,mydonglecloud" >> /boot/firmware/cmdline.txt
echo -e "dtoverlay=dwc2\ndtoverlay=mydonglecloud\ndtoverlay=st7735s\ndtoverlay=buttons\ndtoverlay=leds\ndtparam=uart0=on\ndtparam=spi=on" >> /boot/firmware/config.txt
adduser --comment Administrator --disabled-password admin

echo "################################"
echo "Upgrade"
echo "################################"
apt-get update
apt-get -y upgrade

echo "################################"
echo "Basic"
echo "################################"
apt-get -y install liboath-dev libinput-dev libboost-dev libboost-system-dev libboost-thread-dev libboost-filesystem-dev libcurl4-openssl-dev libssl-dev libbluetooth-dev
apt-get -y install composer apache2 php php-mysql libapache2-mod-php sqlite3 postfix procmail rspamd dovecot-pop3d dovecot-imapd certbot python3-certbot-apache transgui
apt-get -y install evtest qrencode dos2unix lrzsz imagemagick squashfs-tools libpam-oath oathtool cryptsetup-bin
if [ $OS = "ubuntu" ]; then
	chmod a-x /etc/update-motd.d/*
	snap remove snapd
	apt-get -y purge snapd
	apt-get -y install bzip2 zip gpiod net-tools wireless-tools build-essential cmake
fi

echo "################################"
echo "pcpp"
echo "################################"
if [ $OS = "ubuntu" ]; then
	apt-get -y install python3-pcpp
	ln -sf pcpp-python /usr/bin/pcpp
fi
elif [ $OS = "pios" ]; then
	cd /home/mdc/build
	wget https://files.pythonhosted.org/packages/41/07/876153f611f2c610bdb8f706a5ab560d888c938ea9ea65ed18c374a9014a/pcpp-1.30.tar.gz
	tar -xpvf pcpp-1.30.tar.gz
	cd pcpp-1.30
	python setup.py install
fi

echo "################################"
echo "pyupdi"
echo "################################"
cd /home/mdc/build
git clone https://github.com/mraardvark/pyupdi
cd pyupdi
python setup.py install
cd ../..

echo "################################"
echo "Mysql"
echo "################################"
if [ $OS = "ubuntu" ]; then
	apt-get -y install mysql-server
elif [ $OS = "pios" ]; then
	apt-get -y install libaio1 libevent-pthreads-2.1-7 libmecab2
	wget http://ports.ubuntu.com/pool/main/i/icu/libicu70_70.1-2ubuntu1_arm64.deb
	wget http://ports.ubuntu.com/pool/main/p/protobuf/libprotobuf-lite23_3.12.4-1ubuntu7.22.04.2_arm64.deb
	wget http://ports.ubuntu.com/pool/main/m/mysql-defaults/mysql-common_5.8+1.0.8_all.deb
	wget http://ports.ubuntu.com/pool/main/m/mysql-8.0/mysql-server-core-8.0_8.0.42-0ubuntu0.22.04.1_arm64.deb
	wget http://ports.ubuntu.com/pool/main/m/mysql-8.0/mysql-server-8.0_8.0.42-0ubuntu0.22.04.1_arm64.deb
	wget http://ports.ubuntu.com/pool/main/m/mysql-8.0/mysql-client-core-8.0_8.0.42-0ubuntu0.22.04.1_arm64.deb
	wget http://ports.ubuntu.com/pool/main/m/mysql-8.0/mysql-client-8.0_8.0.42-0ubuntu0.22.04.1_arm64.deb
	dpkg -i libicu70* libprotobuf-lite23* mysql-common*
	dpkg -i mysql-client*
	dpkg -i mysql-server*
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
cd /tmp
wget https://github.com/qdrant/qdrant/releases/download/v1.14.1/qdrant-aarch64-unknown-linux-musl.tar.gz
tar -xpvf qdrant-aarch64-unknown-linux-musl.tar.gz
rm -f qdrant-aarch64-unknown-linux-musl.tar.gz
mv qdrant /usr/bin
chmod a+x /usr/bin/qdrant

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
	apt-get -y install linux-headers-rpi-2712 linux-image-rpi-2712
fi

echo "################################"
echo "Jitsi"
echo "################################"
curl https://download.jitsi.org/jitsi-key.gpg.key | gpg --dearmor > /usr/share/keyrings/jitsi-keyring.gpg
echo "deb [arch=arm64 signed-by=/usr/share/keyrings/jitsi-keyring.gpg] https://download.jitsi.org stable/" > /etc/apt/sources.list.d/jitsi-stable.list
apt-get update
apt-get -y install jitsi-meet

echo "################################"
echo "CyberChef"
echo "################################"
cd /home/mdc/build
git clone https://github.com/gchq/CyberChef.git
cd CyberChef
npm install
npm run build

echo "################################"
echo "QRCode"
echo "################################"
cd /home/mdc/build
git clone https://code.antopie.org/miraty/libreqr.git
git clone https://github.com/bizzycola/qrcode-generator
git clone https://github.com/mebjas/html5-qrcode

echo "################################"
echo "Upgrade and cleanup"
echo "################################"
apt-get -y upgrade
apt-get -y autoremove
rm /var/cache/apt/archives/*.deb

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

if [ $PROD = 1 ]; then
	sed -i -e 's|mdc:[^:]*:|mdc:*:|' /etc/shadow
	sed -i -e 's|mdc:[^:]*:|mdc:*:|' /etc/shadow-
fi
