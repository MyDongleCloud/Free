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
#LABEL=rootfs  /disk           ext4    defaults,noatime  0       1
EOF
fatlabel /dev/mmcblk0p1 bootfs
e2label /dev/mmcblk0p2 rootfs
mkdir /disk
adduser --comment Administrator --home /disk/admin --disabled-password admin
usermod -a -G adm,dialout,cdrom,audio,video,plugdev,games,users,input,render,netdev,spi,i2c,gpio,bluetooth admin
mkdir -p /usr/local/modules/mydonglecloud
usermod -a -G adm,dialout,cdrom,audio,video,plugdev,games,users,input,render,netdev,spi,i2c,gpio,bluetooth mdc
usermod -a -G sudo mdc

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
apt-get -y install libprotobuf32 liboath-dev libinput-dev libboost-dev libboost-system-dev libboost-thread-dev libboost-filesystem-dev libcurl4-openssl-dev libssl-dev libbluetooth-dev libjpeg62-turbo-dev libturbojpeg0-dev libldap-dev libsasl2-dev
apt-get -y install python3-intelhex python3-certbot-apache python3-setuptools python3-attr python3-wheel python3-wheel-whl cython3 python3-dateutil python3-sniffio python3-astroid python3-tomlkit python3-appdirs python3-isort python3-mccabe python3-platformdirs python3-serial python3-dill python3-dotenv python3-pytzdata
apt-get -y install composer apache2 php php-mysql php-sqlite3 php-xml php-yaml php-json libapache2-mod-php sqlite3 certbot procmail rspamd dovecot-pop3d dovecot-imapd
apt-get -y install evtest qrencode dos2unix lrzsz imagemagick squashfs-tools libpam-oath oathtool cryptsetup-bin cmake lsof fscrypt libpam-fscrypt hdparm ffmpeg screen figlet toilet
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
	wget https://ports.ubuntu.com/pool/main/m/mysql-defaults/mysql-common_5.8+1.1.1_all.deb
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
chmod a+x qdrant
mkdir /usr/local/modules/qdrant
mv qdrant /usr/local/modules/qdrant
cd ..

echo "################################"
echo "RethinkDB"
echo "################################"
cd /home/mdc/build
wget https://download.rethinkdb.com/repository/debian-bookworm/pool/r/rethinkdb/rethinkdb_2.4.4~0bookworm_arm64.deb
dpkg -i rethinkdb*.deb
cd ..

echo "################################"
echo "Trilium Notes"
echo "################################"
cd /home/mdc/build
wget https://github.com/TriliumNext/Notes/releases/download/v0.95.0/TriliumNextNotes-Server-v0.95.0-linux-arm64.tar.xz
tar -xJpf TriliumNextNotes-Server*
mv TriliumNextNotes-Server-0.*/ /usr/local/modules/trilium
rm -rf /usr/local/modules/trilium/node
ln -sf /etc/systemd/system/trilium.service /etc/systemd/system/multi-user.target.wants/trilium.service

echo "################################"
echo "postfix-parser"
echo "################################"
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
git checkout umtprd-1.6.8
cd uMTP-Responder
make
mkdir /usr/local/modules/MTP
mv umtprd /usr/local/modules/MTP
cd ../..

echo "################################"
echo "Kernel"
echo "################################"
if [ $OS = "ubuntu" ]; then
	apt-get -y install linux-headers-raspi linux-image-raspi
elif [ $OS = "pios" ]; then
	apt-get -y install linux-headers-rpi-2712 linux-image-rpi-2712 raspi-utils-core raspi-utils-dt
	apt-get -y purge linux-headers*rpi-v8 linux-image*rpi-v8 linux-headers-6.12.25*
	rm -rf /lib/modules/6.12.25+rpt-rpi-* /lib/modules/6.12.34+rpt-rpi-v8
	rm -f /boot/cmdline.txt /boot/issue.txt /boot/config.txt /boot/overlays /boot/*6.12.25* /boot/*-v8
	rm -f /boot/firmware/bcm2710* /boot/firmware/bcm2711* /boot/firmware/kernel8.img /boot/firmware/initramfs8 /boot/firmware/LICENCE.broadcom /boot/firmware/issue.txt
fi

echo "################################"
echo "Jitsi"
echo "################################"
curl https://download.jitsi.org/jitsi-key.gpg.key | gpg --dearmor > /usr/share/keyrings/jitsi-keyring.gpg
echo "deb [arch=arm64 signed-by=/usr/share/keyrings/jitsi-keyring.gpg] https://download.jitsi.org stable/" > /etc/apt/sources.list.d/jitsi-stable.list
apt-get update

echo "################################"
echo "Node.js"
echo "################################"
cd /home/mdc/build
wget https://nodejs.org/dist/latest-v22.x/node-v22.17.1-linux-arm64.tar.xz
tar -xJpf node-v*
cp -a node-v*/bin/ node-v*/include/ node-v*/lib/ node-v*/share/ /usr/local
cd ..

echo "################################"
echo "Better-auth"
echo "################################"
cd /home/mdc/build
mkdir better-auth
cd better-auth
npm install better-auth
cd ../..

echo "################################"
echo "cc2538-prog"
echo "################################"
cd /home/mdc/build
git clone https://github.com/1248/cc2538-prog
cd cc2538-prog
make
cd ../..

echo "################################"
echo "Python3.12"
echo "################################"
if [ $OS = "pios" ]; then
	wget https://pascalroeleven.nl/deb-pascalroeleven.gpg -O /etc/apt/keyrings/deb-pascalroeleven.gpg
	cat <<EOF | sudo tee /etc/apt/sources.list.d/pascalroeleven.sources
Types: deb
URIs: http://deb.pascalroeleven.nl/python3.12
Suites: bookworm-backports
Components: main
Signed-By: /etc/apt/keyrings/deb-pascalroeleven.gpg
EOF
	apt-get update
fi
apt-get -y install python3.12 python3.12-venv binfmt-support python3.12-dev

echo "################################"
echo "Mosquitto Zigbee2mqtt"
echo "################################"
apt-get -y install mosquitto
mkdir /usr/local/modules/zigbee2mqtt
cd /usr/local/modules/zigbee2mqtt
npm install zigbee2mqtt@2.5.1
rm -rf /usr/local/modules/zigbee2mqtt/node_modules/zigbee2mqtt/data
ln -sf /etc/systemd/system/zigbee2mqtt.service /etc/systemd/system/multi-user.target.wants/zigbee2mqtt.service

echo "################################"
echo "Home Assistant"
echo "################################"
/home/mdc/rootfs/usr/local/modules/mydonglecloud/pip.sh -f /usr/local/modules/homeautomation -v 3.12 -s
echo "PATH before any modif: $PATH"
PATHOLD=$PATH
PATH=/usr/local/modules/homeautomation/bin:$PATHOLD
export PATH=/usr/local/modules/homeautomation/bin:$PATHOLD
echo "PATH new: $PATH python: `python --version`"
pip install homeassistant
pip install aiodhcpwatcher==1.0.2
pip install aiodiscover==2.1.0
pip install async-upnp-client==0.42.0
pip install av==13.1.0
pip install bleak-retry-connector==3.6.0
pip install bluetooth-auto-recovery==1.4.2
pip install cached-ipaddress==0.8.0
pip install go2rtc-client==0.1.2
pip install habluetooth==3.7.0
pip install ha-ffmpeg==3.2.2
pip install hassil==2.1.0
pip install home-assistant-frontend==20250109.2
pip install home-assistant-intents==2025.1.1
pip install mutagen==1.47.0
pip install pymicro-vad==1.0.1
pip install PyNaCl==1.5.0
pip install pyotp==2.8.0
pip install PyQRCode==1.2.1
pip install pyserial==3.5
pip install pyspeex-noise==1.0.2
pip install python-matter-server==6.6.0
pip install PyTurboJPEG==1.7.5
pip install pyudev==0.24.1
pip install zeroconf==0.136.2
pip install bluetooth-data-tools==1.20.0
pip install bluetooth-adapters==0.20.2
pip install numpy==2.2.0
pip install bleak==0.22.3

pip install dbus-fast==2.24.3
pip install gTTS==2.2.4
pip install PyMetno==0.13.0
pip install radios==0.3.2
pip install aioruuvigateway==0.1.0
pip install aioshelly==12.2.0
pip install aioesphomeapi==28.0.0
pip install esphome-dashboard-api==1.2.3
pip install bleak-esphome==2.0.0
pip install paho-mqtt==1.6.1
pip install ibeacon-ble==1.2.0
PATH=$PATHOLD
export PATH=$PATHOLD
echo "PATH restored: $PATH"
ln -sf /etc/systemd/system/homeautomation.service /etc/systemd/system/multi-user.target.wants/homeautomation.service

echo "################################"
echo "Tube Archivist"
echo "################################"
/home/mdc/rootfs/usr/local/modules/mydonglecloud/pip.sh -f /usr/local/modules/tubearchivist -s
echo "PATH before any modif: $PATH"
PATHOLD=$PATH
PATH=/usr/local/modules/tubearchivist/bin:$PATHOLD
export PATH=/usr/local/modules/tubearchivist/bin:$PATHOLD
echo "PATH new: $PATH python: `python --version`"
pip install apprise==1.9.3
pip install celery==5.5.3
pip install django-auth-ldap==5.2.0
pip install django-celery-beat==2.8.1
pip install django-cors-headers==4.7.0
pip install Django==5.2.4
pip install djangorestframework==3.16.0
pip install drf-spectacular==0.28.0
pip install Pillow==11.3.0
pip install redis==6.2.0
pip install requests==2.32.4
pip install ryd-client==0.0.6
pip install uvicorn==0.35.0
pip install whitenoise==6.9.0
pip install yt-dlp[default]==2025.6.30
cd /usr/local/modules/tubearchivist
git clone https://github.com/tubearchivist/tubearchivist
git checkout v0.5.4
PATH=$PATHOLD
export PATH=$PATHOLD
echo "PATH restored: $PATH"

echo "################################"
echo "LimeSurvey"
echo "################################"
cd /usr/local/modules
git clone https://github.com/LimeSurvey/LimeSurvey
cd LimeSurvey
git checkout 6.15.3+250708
rm -rf .git

echo "################################"
echo "Project Send"
echo "################################"
cd /usr/local/modules
git clone https://github.com/projectsend/projectsend
cd projectsend
git checkout r1720
rm -rf .git

echo "################################"
echo "phpList"
echo "################################"
cd /usr/local/modules
composer create-project -n -s dev --no-dev phplist/base-distribution phpList
cd phpList
composer update -n

echo "################################"
echo "Go Access"
echo "################################"
cd /usr/local/modules
git clone https://github.com/allinurl/goaccess
cd goaccess
git checkout v1.9.4
rm -rf .git

echo "################################"
echo "Webtrees"
echo "################################"
cd /usr/local/modules
git clone https://github.com/fisharebest/webtrees
cd webtrees
git checkout 2.2.1
rm -rf .git

echo "################################"
echo "osTicket"
echo "################################"
cd /usr/local/modules
git clone https://github.com/osTicket/osTicket
cd osTicket
git checkout v1.18.2
rm -rf .git

echo "################################"
echo "YOURLS"
echo "################################"
cd /usr/local/modules
git clone https://github.com/YOURLS/YOURLS
cd YOURLS
git checkout 1.10.1
rm -rf .git

echo "################################"
echo "PrivateBin"
echo "################################"
cd /usr/local/modules
git clone https://github.com/PrivateBin/PrivateBin
cd PrivateBin
git checkout 1.7.8
rm -rf .git

echo "################################"
echo "Flarum"
echo "################################"
cd /usr/local/modules
git clone https://github.com/flarum/flarum
cd flarum
git checkout v1.8.1
rm -rf .git

echo "################################"
echo "Discourse"
echo "################################"
cd /usr/local/modules
git clone https://github.com/discourse/discourse
cd discourse
git checkout v3.4.6
rm -rf .git

echo "################################"
echo "Grav"
echo "################################"
cd /usr/local/modules
git clone https://github.com/getgrav/grav
cd grav
git checkout 1.7.48
rm -rf .git

echo "################################"
echo "Mantis Bug Tracker"
echo "################################"
cd /usr/local/modules
git clone https://github.com/mantisbt/mantisbt
cd mantisbt
git checkout release-2.27.1
rm -rf .git

echo "################################"
echo "Bugzilla"
echo "################################"
cd /usr/local/modules
git clone https://github.com/bugzilla/bugzilla
cd bugzilla
git checkout release-5.3.3
rm -rf .git

echo "################################"
echo "Open WebUI"
echo "################################"
cd /usr/local/modules
git clone https://github.com/open-webui/open-webui
cd open-webui
git checkout v0.6.18
rm -rf .git

echo "################################"
echo "Ollama"
echo "################################"
cd /usr/local/modules
git clone https://github.com/ollama/ollama
cd ollama
git checkout v0.9.7-rc1
rm -rf .git

echo "################################"
echo "Pandoc"
echo "################################"
apt-get -y install pandoc

echo "################################"
echo "Transmission"
echo "################################"
cd /home/mdc/build
apt-get -y install libpsl-dev libminiupnpc-dev libnatpmp-dev libevent-dev googletest libdeflate-dev libutfcpp-dev
git clone https://github.com/transmission/transmission
cd transmission
git checkout 4.0.6
git submodule init
git submodule update
rm -rf .git
cmake -B build -DCMAKE_BUILD_TYPE=Release
cd build
make
make install

echo "################################"
echo "Unmanic"
echo "################################"
/home/mdc/rootfs/usr/local/modules/mydonglecloud/pip.sh -f /usr/local/modules/unmanic -s
echo "PATH before any modif: $PATH"
PATHOLD=$PATH
PATH=/usr/local/modules/unmanic/bin:$PATHOLD
export PATH=/usr/local/modules/unmanic/bin:$PATHOLD
echo "PATH new: $PATH python: `python --version`"
pip install unmanic
PATH=$PATHOLD
export PATH=$PATHOLD
echo "PATH restored: $PATH"

echo "################################"
echo "QRCode"
echo "################################"
cd /home/mdc/build
git clone https://code.antopie.org/miraty/libreqr.git
cd libreqr
git checkout 2.0.1
cd ..
git clone https://github.com/bizzycola/qrcode-generator
cd qrcode-generator
git checkout v1.8.0
cd ..
git clone https://github.com/mebjas/html5-qrcode
cd html5-qrcode
git checkout v2.3.8
cd ..

echo "################################"
echo "App and rootfs"
echo "################################"
cd /home/mdc/kernel
make
make install
cd /home/mdc/app
./lvgl.sh -b -c
make
cd /home/mdc
chown -R root:root rootfs
cp -a rootfs/* /
rm -rf rootfs
chown -R root:root /usr/local
chown -R mdc:mdc /home/mdc
chown -R admin:admin /disk/admin

echo "################################"
echo "Cleanup"
echo "################################"
apt-get -y autoremove
rm -f /var/cache/apt/archives/*.deb
rm -f /home/mdc/build/*.deb /home/mdc/build/*.xz
rm -rf /home/mdc/.cache/pip
rm -rf /root
rm -rf /lost+found
rmdir /usr/local/games
rm -rf /opt/containerd /opt/pigpio
mv /var/log /var/log.old
ln -sf /disk/admin/.modules/log/ /var/log

echo "################################"
echo "Package with interaction"
echo "################################"
apt-get -y install postfix jitsi-meet roundcube

sync
sync
echo "################################"
echo "Finish"
echo "################################"
date

if [ $PROD = 1 ]; then
	rm -rf /var/log.old
	sed -i -e 's|mdc:[^:]*:|mdc:*:|' /etc/shadow
	sed -i -e 's|mdc:[^:]*:|mdc:*:|' /etc/shadow-
fi
