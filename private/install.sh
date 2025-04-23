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

sed -i -e 's|/root|/home/mydonglecloud|' /etc/passwd
rm -rf /root
sed -i -e 's|# "\\e\[5~": history-search-backward|"\\e\[5~": history-search-backward|' /etc/inputrc
sed -i -e 's|# "\\e\[6~": history-search-forward|"\\e\[6~": history-search-forward|' /etc/inputrc
sed -i -e 's|%sudo	ALL=(ALL:ALL) ALL|%sudo	ALL=(ALL:ALL) NOPASSWD:ALL|' /etc/sudoers
sed -i -e 's|HISTSIZE=.*|HISTSIZE=-1|' /home/mdc/.bashrc
sed -i -e 's|HISTFILESIZE=.*|HISTFILESIZE=-1|' /home/mdc/.bashrc
ln -sf /lib/systemd/system/serial-getty@.service /etc/systemd/system/getty.target.wants/serial-getty@ttyGS0.service
ln -sf /etc/systemd/system/mydonglecloud-app.service /etc/systemd/system/multi-user.target.wants/mydonglecloud-app.service
ln -sf /etc/systemd/system/mydonglecloud-init.service /etc/systemd/system/sysinit.target.wants/mydonglecloud-init.service
ln -sf pcpp-python /usr/bin/pcpp
echo -n " modules-load=dwc2,libcomposite,configs,mydonglecloud" >> /boot/firmware/cmdline.txt
echo -e "dtoverlay=dwc2\ndtoverlay=mydonglecloud\ndtoverlay=st7735s\ndtoverlay=buttons\ndtoverlay=leds\ndtparam=uart0=on\ndtparam=spi=on" >> /boot/firmware/config.txt
apt-get update
apt-get -y upgrade
snap remove snapd
apt-get -y purge snapd
adduser admin
apt-get -y install bzip2 net-tools wireless-tools libpam-oath oathtool liboath-dev qrencode apache2 mysql-server php php-mysql libapache2-mod-php sqlite3 postfix procmail dos2unix composer lrzsz imagemagick squashfs-tools spamassassin spamc build-essential evtest cmake python3-pcpp libinput-dev libboost-dev libboost-system-dev libboost-thread-dev libboost-filesystem-dev libcurl4-openssl-dev libssl-dev libbluetooth-dev
apt-get -y install linux-headers-6.8.0-1018-raspi
chmod a-x /etc/update-motd.d/*
apt-get -y autoremove
rm /var/cache/apt/archives/*.deb
cd /home/mdc
tar -xjpvf /tmp/a.tbz2
cd kernel && make && make install && cd ..
cd app && ./lvgl.sh && make && cd ..
cp -a rootfs/* /
rm -rf rootfs

#cd build
#git clone https://github.com/viveris/uMTP-Responder
#cd uMTP-Responder
#make
#cp umtprd /usr/bin

#cd /tmp
#wget https://github.com/qdrant/qdrant/releases/download/v1.13.6/qdrant-aarch64-unknown-linux-musl.tar.gz
#tar -xpvf qdrant-aarch64-unknown-linux-musl.tar.gz
#rm -f qdrant-aarch64-unknown-linux-musl.tar.gz
#mv qdrant /usr/bin
#chmod a+x /usr/bin/qdrant

if [ $PROD = 1 ]; then
	sed -i -e 's|mdc:[^:]*:|mdc:*:|' /etc/shadow
	sed -i -e 's|mdc:[^:]*:|mdc:*:|' /etc/shadow-
fi
