#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for install [-c -h -n -p]"
echo "c:	Do full clone of all modules"
echo "h:	Print this usage and exit"
echo "n:	Not native (via chroot)"
echo "p:	Do production image"
exit 0
}

PROD=0
NATIVE=1
CLONE=0
while getopts chnp opt; do
	case "$opt" in
		c) CLONE=1;;
		h) helper;;
		n) NATIVE=0;;
		p) PROD=1;;
	esac
done

if [ "m`id -u`" != "m0" ]; then
	echo "You need to be root"
	exit 0
fi

lsb_release -a | grep bookworm
if [ $? = 0 ]; then
#On PC
#tar -cjpvf a.tbz2 app/ auth/ kernel/ rootfs/ screenAvr/ moduleApache2/ pam/ private/install.sh private/preseed*.cfg
#scp a.tbz2 private/img/clone.tbz2 mdc@192.168.10.41:/tmp
#On device
#tar -xjpvf /tmp/a.tbz2
	OS="pios"
fi
lsb_release -a | grep noble
if [ $? = 0 ]; then
	OS="ubuntu"
fi

echo "################################"
echo "Start install"
echo "################################"
date
DATESTART=`date +%s`

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
ln -sf /etc/systemd/system/mydonglecloud-otg.service /etc/systemd/system/sysinit.target.wants/mydonglecloud-otg.service
if [ $NATIVE = 1 ]; then
	echo -n " modules-load=dwc2,libcomposite,configs,mydonglecloud" >> /boot/firmware/cmdline.txt
	sed -i -e 's/ root=[^ ]* / root=LABEL=rootfs /' /boot/firmware/cmdline.txt
	sed -i -e 's/cfg80211.ieee80211_regdom=US/cfg80211.ieee80211_regdom=00/' /boot/firmware/cmdline.txt
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
fi
cat > /etc/fstab <<EOF
proc            /proc           proc    defaults          0       0
LABEL=rootfs  /               ext4    defaults,noatime  0       1
#LABEL=rootfs  /disk           ext4    defaults,noatime  0       1
EOF
if [ $NATIVE = 1 ]; then
	fatlabel /dev/mmcblk0p1 bootfs
	e2label /dev/mmcblk0p2 rootfs
fi
mkdir /disk
adduser --comment Administrator --home /disk/admin --disabled-password admin
usermod -a -G adm,dialout,cdrom,audio,video,plugdev,games,users,input,render,netdev,spi,i2c,gpio,bluetooth admin
sed -i -e 's|# User privilege specification|# User privilege specification\nadmin ALL=(ALL:ALL) NOPASSWD: /sbin/shutdown -h now, /sbin/reboot, /usr/bin/systemctl reload apache2, /usr/bin/systemctl start frp.service|' /etc/sudoers
mkdir -p /usr/local/modules/pam && echo -e "#!/bin/sh\nexit 0" > /usr/local/modules/pam/pam.sh && chmod a+x /usr/local/modules/pam/pam.sh
sed -i '1i auth sufficient pam_oath.so usersfile=/disk/admin/.modules/pam/oath.txt' /etc/pam.d/common-auth
sed -i '2i auth sufficient /usr/local/modules/pam/pam_mydonglecloud.so' /etc/pam.d/common-auth
sed -i '3i session optional pam_exec.so /usr/local/modules/pam/pam.sh' /etc/pam.d/common-auth
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
if [ $OS = "pios" ]; then
	apt-get -y -o Dpkg::Options::="--force-confnew" -o Dpkg::Options::="--force-confdef" install initramfs-tools-core
fi
apt-get -y upgrade

echo "################################"
echo "Basic"
echo "################################"
if [ $OS = "ubuntu" ]; then
	chmod a-x /etc/update-motd.d/*
	which snapd
	if [ $? = 0 ]; then
		snap remove snapd
		apt-get -y purge snapd
	fi
	apt-get -y install bzip2 zip gpiod net-tools wireless-tools build-essential curl wget nano initramfs-tools device-tree-compiler nmap ncat fd-find ncdu
fi
apt-get -y install evtest qrencode dos2unix lrzsz squashfs-tools libpam-oath oathtool cryptsetup-bin cmake lsof hdparm screen figlet toilet composer network-manager bind9 acl jq telnet netcat-openbsd pamtester
apt-get -y install liboath-dev libinput-dev libboost-dev libboost-system-dev libboost-thread-dev libboost-filesystem-dev libcurl4-openssl-dev libssl-dev libbluetooth-dev libturbojpeg0-dev libldap-dev libsasl2-dev apache2-dev libpam0g-dev libnm-dev libjwt-dev
if [ $OS = "ubuntu" ]; then
	apt-get -y install libprotobuf32t64 libjpeg62-dev
elif [ $OS = "pios" ]; then
	apt-get -y install libprotobuf32 libjpeg62-turbo-dev
fi

echo "################################"
echo "Python"
echo "################################"
if [ $OS = "pios" ]; then
	wget -nv https://pascalroeleven.nl/deb-pascalroeleven.gpg -O /etc/apt/keyrings/deb-pascalroeleven.gpg
	cat <<EOF | sudo tee /etc/apt/sources.list.d/pascalroeleven.sources
Types: deb
URIs: http://deb.pascalroeleven.nl/python3.12
Suites: bookworm-backports
Components: main
Signed-By: /etc/apt/keyrings/deb-pascalroeleven.gpg
EOF
	apt-get update
	apt-get -y install python3.12 python3.12-venv binfmt-support python3.12-dev
fi
apt-get -y install python3-venv python3-intelhex python3-certbot-apache python3-setuptools python3-attr python3-wheel python3-wheel-whl cython3 python3-dateutil python3-sniffio python3-astroid python3-tomlkit python3-appdirs python3-isort python3-mccabe python3-platformdirs python3-serial python3-dill python3-dotenv python3-pytzdata

echo "################################"
echo "Mysql"
echo "################################"
if [ $OS = "ubuntu" ]; then
	apt-get -y install mysql-server-8.0
	apt-get -y install mysql-server
elif [ $OS = "pios" ]; then
	apt-get -y install libaio1 libevent-pthreads-2.1-7 libmecab2
	cd /home/mdc/build
	wget -nv https://ports.ubuntu.com/pool/main/i/icu/libicu70_70.1-2ubuntu1_arm64.deb
	wget -nv https://ports.ubuntu.com/pool/main/p/protobuf/libprotobuf-lite23_3.12.4-1ubuntu7.22.04.4_arm64.deb
	wget -nv https://ports.ubuntu.com/pool/main/m/mysql-defaults/mysql-common_5.8+1.1.1ubuntu1_all.deb
	wget -nv https://ports.ubuntu.com/pool/main/m/mysql-8.0/mysql-server-core-8.0_8.0.43-0ubuntu0.22.04.1_arm64.deb
	wget -nv https://ports.ubuntu.com/pool/main/m/mysql-8.0/mysql-server-8.0_8.0.43-0ubuntu0.22.04.1_arm64.deb
	wget -nv https://ports.ubuntu.com/pool/main/m/mysql-8.0/mysql-client-core-8.0_8.0.43-0ubuntu0.22.04.1_arm64.deb
	wget -nv https://ports.ubuntu.com/pool/main/m/mysql-8.0/mysql-client-8.0_8.0.43-0ubuntu0.22.04.1_arm64.deb
	dpkg -i libicu70* libprotobuf-lite23* mysql-common*
	dpkg -i mysql-client* mysql-server*
	cd ..
fi

echo "################################"
echo "Postfix"
echo "################################"
cat /home/mdc/private/preseed_postfix.cfg | debconf-set-selections
apt-get -y install postfix

echo "################################"
echo "Modules via apt"
echo "################################"
apt-get -y install certbot dovecot-imapd dovecot-pop3d ffmpeg fscrypt goaccess hugo imagemagick libapache2-mod-php libpam-fscrypt mosquitto nginx pandoc php php-json php-mysql php-sqlite3 php-xml php-yaml php-imap php-curl php-zip php-apcu php-memcache php-redis php-ldap procmail rspamd sqlite3

echo "################################"
echo "Apache2"
echo "################################"
apt-get -y install apache2
rm -f /etc/apache2/sites-enabled/*
rm -f /etc/apache2/ports.conf

echo "################################"
echo "Roundcube"
echo "################################"
cat /home/mdc/private/preseed_roundcube.cfg | debconf-set-selections
apt-get -y install roundcube

echo "################################"
echo "Kernel (Dongle Pro)"
echo "################################"
if [ $OS = "ubuntu" ]; then
	echo "apt-get -y install linux-headers-rpi-2712 linux-image-rpi-2712 linux-headers-6.12.34+rpt-common-rpi linux-headers-6.12.34+rpt-rpi-2712 linux-image-6.12.34+rpt-rpi-2712 linux-kbuild-6.12.34+rpt"
	wget -nv https://archive.raspberrypi.org/debian/pool/main/l/linux/linux-headers-rpi-2712_6.12.34-1+rpt1_arm64.deb
	wget -nv https://archive.raspberrypi.org/debian/pool/main/l/linux/linux-image-rpi-2712_6.12.34-1+rpt1_arm64.deb
	wget -nv https://archive.raspberrypi.org/debian/pool/main/l/linux/linux-headers-6.12.34+rpt-common-rpi_6.12.34-1+rpt1_all.deb
	wget -nv https://archive.raspberrypi.org/debian/pool/main/l/linux/linux-headers-6.12.34+rpt-rpi-2712_6.12.34-1+rpt1_arm64.deb
	wget -nv https://archive.raspberrypi.org/debian/pool/main/l/linux/linux-image-6.12.34+rpt-rpi-2712_6.12.34-1+rpt1_arm64.deb
	wget -nv https://archive.raspberrypi.org/debian/pool/main/l/linux/linux-kbuild-6.12.34+rpt_6.12.34-1+rpt1_arm64.deb
	apt-get -y install cpp-14-aarch64-linux-gnu gcc-14 gcc-14-aarch64-linux-gnu libgcc-14-dev pahole
	dpkg -i linux-*.deb
elif [ $OS = "pios" ]; then
	apt-get -y install linux-headers-rpi-2712 linux-image-rpi-2712 raspi-utils-core raspi-utils-dt
	apt-get -y purge linux-headers*rpi-v8 linux-image*rpi-v8 linux-headers-6.12.25* linux-kbuild-6.12.25*
	rm -rf /lib/modules/6.12.25+rpt-rpi-* /lib/modules/6.12.34+rpt-rpi-v8
	rm -f /boot/cmdline.txt /boot/issue.txt /boot/config.txt /boot/overlays /boot/*6.12.25* /boot/*-v8
	rm -f /boot/firmware/bcm2710* /boot/firmware/bcm2711* /boot/firmware/kernel8.img /boot/firmware/initramfs8 /boot/firmware/LICENCE.broadcom /boot/firmware/issue.txt
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
echo "Jitsi"
echo "################################"
curl https://download.jitsi.org/jitsi-key.gpg.key | gpg --dearmor > /usr/share/keyrings/jitsi-keyring.gpg
echo "deb [arch=arm64 signed-by=/usr/share/keyrings/jitsi-keyring.gpg] https://download.jitsi.org stable/" > /etc/apt/sources.list.d/jitsi-stable.list
apt-get update
cat /home/mdc/private/preseed_jitsi.cfg | debconf-set-selections
apt-get -y install jitsi-videobridge2 jitsi-meet-web-config jitsi-meet-web
#apt-get -y install jitsi-meet

echo "################################"
echo "RethinkDB"
echo "################################"
cd /home/mdc/build
wget -nv https://download.rethinkdb.com/repository/debian-bookworm/pool/r/rethinkdb/rethinkdb_2.4.4~0bookworm_arm64.deb
dpkg -i rethinkdb*.deb
cd ..

echo "################################"
echo "devmem2"
echo "################################"
cd /home/mdc/build
wget -nv https://bootlin.com/pub/mirror/devmem2.c
gcc -o /usr/bin/local/devmem2 devmem2.c

echo "################################"
echo "frp"
echo "################################"
cd /home/mdc/build
wget -nv https://github.com/fatedier/frp/releases/download/v0.63.0/frp_0.63.0_linux_arm64.tar.gz
tar -xpf frp_*_linux_arm64.tar.gz
mkdir /usr/local/modules/frp
mv frp_*_linux_arm64/frpc /usr/local/modules/frp
cd ..

echo "################################"
echo "LiveCodes"
echo "################################"
cd /home/mdc/build
wget -nv https://github.com/live-codes/livecodes/releases/download/v46/livecodes-v46.tar.gz
mkdir /usr/local/modules/livecodes
tar -xpvf livecodes-v46.tar.gz -C /usr/local/modules/livecodes --strip-components=1
cd ..

echo "################################"
echo "Qdrant"
echo "################################"
cd /home/mdc/build
wget -nv https://github.com/qdrant/qdrant/releases/download/v1.14.1/qdrant-aarch64-unknown-linux-musl.tar.gz
tar -xpf qdrant-aarch64-unknown-linux-musl.tar.gz
chmod a+x qdrant
mkdir /usr/local/modules/qdrant
mv qdrant /usr/local/modules/qdrant
cd ..

echo "################################"
echo "Trilium Notes"
echo "################################"
cd /home/mdc/build
wget -nv https://github.com/TriliumNext/Notes/releases/download/v0.95.0/TriliumNextNotes-Server-v0.95.0-linux-arm64.tar.xz
tar -xJpf TriliumNextNotes-Server*
mv TriliumNextNotes-Server-0.*/ /usr/local/modules/triliumnotes
rm -rf /usr/local/modules/triliumnotes/node
ln -sf /etc/systemd/system/triliumnotes.service /etc/systemd/system/multi-user.target.wants/triliumnotes.service

echo "################################"
echo "Node.js"
echo "################################"
cd /home/mdc/build
FILENODE=`wget -q -O - https://nodejs.org/dist/latest-v22.x/ | grep "\-linux\-arm64\.tar\.xz" | sed -E "s|.*>([^<]*)<.*|\1|"`
wget -nv https://nodejs.org/dist/latest-v22.x/$FILENODE
tar -xJpf node-v*
cp -a node-v*/bin/ node-v*/include/ node-v*/lib/ node-v*/share/ /usr/local
cd ..

echo "################################"
echo "npm packages"
echo "################################"
npm -g install @angular/cli @ionic/cli @vue/cli cordova-res

echo "################################"
echo "Zigbee2MQTT"
echo "################################"
mkdir /usr/local/modules/zigbee2mqtt
cd /usr/local/modules/zigbee2mqtt
npm install zigbee2mqtt@2.5.1
rm -rf /usr/local/modules/zigbee2mqtt/node_modules/zigbee2mqtt/data
ln -sf /etc/systemd/system/zigbee2mqtt.service /etc/systemd/system/multi-user.target.wants/zigbee2mqtt.service

echo "################################"
echo "phpList"
echo "################################"
cd /usr/local/modules
wget -nv https://versaweb.dl.sourceforge.net/project/phplist/phplist/3.6.16/phplist-3.6.16.tgz
tar -xpf phplist-*
rm phplist-*.tgz
mv phplist-* phplist

echo "################################"
echo "pcpp"
echo "################################"
if [ $OS = "ubuntu" ]; then
	apt-get -y install python3-pcpp
	ln -sf pcpp-python /usr/bin/pcpp
elif [ $OS = "pios" ]; then
	cd /home/mdc/build
	wget -nv https://files.pythonhosted.org/packages/41/07/876153f611f2c610bdb8f706a5ab560d888c938ea9ea65ed18c374a9014a/pcpp-1.30.tar.gz
	tar -xpf pcpp-1.30.tar.gz
	cd pcpp-1.30
	python3 setup.py install
fi

echo "################################"
echo "PyMCUProg"
echo "################################"
cd /home/mdc/build
git clone https://github.com/microchip-pic-avr-tools/pymcuprog
cd pymcuprog
cat > setup.py <<EOF
from setuptools import setup
if __name__ == '__main__':
    setup()
EOF
python3 setup.py install
cd ../..

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
echo "HomeAssistant"
echo "################################"
cd /home/mdc
/home/mdc/rootfs/usr/local/modules/mydonglecloud/pip.sh -f /usr/local/modules/homeassistant -v 3.12 -s
echo "PATH before any modif: $PATH"
PATHOLD=$PATH
PATH=/usr/local/modules/homeassistant/bin:$PATHOLD
export PATH=/usr/local/modules/homeassistant/bin:$PATHOLD
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
pip install isal==1.7.2
pip install zlib_ng==0.5.1

pip install universal-silabs-flasher==0.0.25
pip install zha==0.0.45
PATH=$PATHOLD
export PATH=$PATHOLD
echo "PATH restored: $PATH"
ln -sf /etc/systemd/system/homeassistant.service /etc/systemd/system/multi-user.target.wants/homeassistant.service

echo "################################"
echo "TubeArchivist"
echo "################################"
cd /home/mdc
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
cd tubearchivist
git checkout v0.5.4
rm -rf .git
PATH=$PATHOLD
export PATH=$PATHOLD
echo "PATH restored: $PATH"

echo "################################"
echo "Unmanic"
echo "################################"
cd /home/mdc
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
echo "Transmission"
echo "################################"
if [ $OS = "ubuntu" ]; then
	apt-get -y install transmission-common transmission-daemon transmission-cli
elif [ $OS = "pios" ]; then
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
fi

echo "################################"
echo "uMTP"
echo "################################"
cd /home/mdc/build
git clone https://github.com/viveris/uMTP-Responder
git checkout umtprd-1.6.8
cd uMTP-Responder
make
mkdir /usr/local/modules/mtp
cp umtprd /usr/local/modules/mtp
cd ../..

echo "################################"
echo "cc2538-prog"
echo "################################"
cd /home/mdc/build
git clone https://github.com/1248/cc2538-prog
cd cc2538-prog
make
cp cc2538-prog /usr/local/bin/
cd ../..

echo "################################"
echo "WebSSH2"
echo "################################"
cd /usr/local/modules
git clone https://github.com/billchurch/webssh2 webssh2
cd webssh2
git checkout v0.2.24
rm -rf .git
cd app
npm install --production
cd ../..
ln -sf /etc/systemd/system/webssh2.service /etc/systemd/system/multi-user.target.wants/webssh2.service

echo "################################"
echo "Acme.sh"
echo "################################"
cd /usr/local/modules
mkdir /usr/local/modules/acme
cd Acme
wget -nv https://raw.githubusercontent.com/acmesh-official/acme.sh/refs/tags/3.1.1/acme.sh
chmod a+x acme.sh

echo "################################"
echo "Libreqr"
echo "################################"
cd /usr/local/modules
git clone https://code.antopie.org/miraty/libreqr.git libreqr
cd Libreqr
git checkout 2.0.1
rm -rf .git

if [ -f "/tmp/clone.tbz2" ]; then
	echo "################################"
	echo "Extraction instead of cloning"
	echo "################################"
	tar -xjpf /tmp/clone.tbz2 -C /usr/local/modules
	sync
	rm -f /tmp/clone.tbz2
else
	clone() {
		PATHB=/usr/local/modules
		echo "################################"
		echo "$1"
		echo "################################"
		cd $PATHB
		git clone https://github.com/$2 $1
		cd $1
		git checkout $3
		rm -rf .git
	}

	clone audiobookshelf advplyr/audiobookshelf v2.26.2
	clone bugzilla bugzilla/bugzilla release-5.3.3
	clone changedetection dgtlmoon/changedetection.io 0.50.7
	clone convertx C4illin/ConvertX v0.14.1
	clone cyberchef gchq/CyberChef v10.19.4
	clone discourse discourse/discourse v3.4.6
	clone flarum flarum/flarum v1.8.1
	clone freshrss FreshRSS/FreshRSS 1.26.3
	clone gitea go-gitea/gitea v1.24.3
	clone grav getgrav/grav 1.7.48
	clone html5qrcode mebjas/html5-qrcode v2.3.8
	clone immich immich-app/immich v1.135.3
	clone iopaint Sanster/IOPaint iopaint-1.5.3
	clone jellyfin jellyfin/jellyfin v10.10.7
	clone joomla joomla/joomla-cms 5.3.2
	clone joplin laurent22/joplin server-v3.4.1
	clone jstinker johncipponeri/jstinker master
	clone karakeep karakeep-app/karakeep v0.26.0
	clone librephotos LibrePhotos/librephotos HEAD
	clone limesurvey LimeSurvey/LimeSurvey 6.15.3+250708
	clone mantisbugtracker mantisbt/mantisbt release-2.27.1
	clone markdowneditor jbt/markdown-editor v2
	clone maybe maybe-finance/maybe v0.5.0
	clone mermaid mermaid-js/mermaid mermaid@11.9.0
	clone metube alexta69/metube HEAD
	clone minio minio/minio RELEASE.2025-07-18T21-56-31Z
	clone ollama ollama/ollama v0.9.7-rc1
	clone openwebui open-webui/open-webui v0.6.18
	clone osticket osTicket/osTicket v1.18.2
	clone photoprism photoprism/photoprism 250707-d28b3101e
	clone photoview photoview/photoview v2.4.0
	clone phpbb phpbb/phpbb release-3.3.15
	clone phpsandbox Corveda/PHPSandbox v3.1
	clone pihole pi-hole/pi-hole v.6.1.4
	clone privatebin PrivateBin/PrivateBin 1.7.8
	clone projectsend projectsend/projectsend r1720
	clone qrcodegenerator bizzycola/qrcode-generator HEAD
	clone radaar Radarr/Radarr v5.27.2.10142
	clone sonarr Sonarr/Sonarr v4.0.15.2941
	clone stackedit benweet/stackedit v5.15.4
	clone stirlingpdf Stirling-Tools/Stirling-PDF v1.0.2
	clone sunrisecms cityssm/sunrise-cms v1.0.0-alpha.19
	clone superset apache/superset 5.0.0
	clone syncthing syncthing/syncthing v1.30.0
	clone uptime louislam/uptime-kuma 1.23.16
	clone webtrees fisharebest/webtrees 2.2.1
	clone yourls YOURLS/YOURLS 1.10.1
fi

echo "################################"
echo "Libreqr"
echo "################################"
cd /usr/local/modules/libreqr
composer -n install
chown www-data:www-data css

echo "################################"
echo "Audiobookshelf"
echo "################################"
cd /usr/local/modules/audiobookshelf
npm install

echo "################################"
echo "Flarum"
echo "################################"
cd /usr/local/modules/flarum
composer -n install

echo "################################"
echo "FreshRSS"
echo "################################"
cd /usr/local/modules/freshrss
npm install

echo "################################"
echo "Gitea"
echo "################################"
cd /usr/local/modules/gitea
npm install

echo "################################"
echo "Grav"
echo "################################"
cd /usr/local/modules/grav
bin/grav install

echo "################################"
echo "Joomla"
echo "################################"
cd /usr/local/modules/joomla
composer -n install
npm ci

echo "################################"
echo "osTicket"
echo "################################"
cd /usr/local/modules/osticket
mkdir -p /disk/admin/.modules/osticket
ln -sf /disk/admin/.modules/osticket/ost-config.php include/ost-config.php

echo "################################"
echo "Better Auth"
echo "################################"
ln -sf /etc/systemd/system/betterauth.service /etc/systemd/system/multi-user.target.wants/betterauth.service
ln -sf /etc/systemd/system/betterauth-studio.service /etc/systemd/system/multi-user.target.wants/betterauth-studio.service
cd /home/mdc/auth
./prepare.sh -i

echo "################################"
echo "MyDongleCloud stuff and rootfs"
echo "################################"
cd /home/mdc
chown -R root:root rootfs
cp -a rootfs/* /
rm -rf rootfs
cd /home/mdc/kernel
make
mkdir -p /boot/firmware/overlays
make install
cd /home/mdc/app
./lvgl.sh -b -c
make
cd /home/mdc/moduleApache2
make
cd /home/mdc/pam
make
chown -R root:root /usr/local
chown -R mdc:mdc /home/mdc
chown -R admin:admin /disk/admin /var/cache-admin

echo "################################"
echo "Cleanup"
echo "################################"
if [ $OS = "pios" ]; then
	apt-get -y purge python3-rpi-lgpio rpicam-apps-core rpicam-apps-lite
fi
apt-get -y autoremove
rm -f /var/cache/apt/archives/*.deb
rm -f /home/mdc/build/*.deb /home/mdc/build/*.xz /home/mdc/build/*.gz
rm -rf /home/mdc/.cache/pip
rm -rf /root
rm -rf /lost+found
rmdir /usr/local/games
if [ $OS = "pios" ]; then
	rm -rf /opt/containerd /opt/pigpio
fi
mv /var/log/* /disk/admin/.log/
rmdir /var/log
ln -sf /disk/admin/.log/ /var/log

sync
sync
echo "################################"
echo "Finish install"
echo "################################"
date
DATEFINISH=`date +%s`
DELTA=$((DATEFINISH - DATESTART))
echo "Duration: $((DELTA / 3600))h $(((DELTA % 3600) / 60))m $((DELTA % 60))s"

if [ $PROD = 1 ]; then
	sed -i -e 's|mdc:[^:]*:|mdc:*:|' /etc/shadow
	sed -i -e 's|mdc:[^:]*:|mdc:*:|' /etc/shadow-
	rm -rf /home/mdc
	mkdir /home/mdc
	chown -R 1000:1000 /home/mdc
fi
