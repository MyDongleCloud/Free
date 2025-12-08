#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for install [-c -h -p]"
echo "c:	Do full clone of all modules"
echo "h:	Print this usage and exit"
echo "p:	Do production image"
exit 0
}

PROD=0
CLONE=0
while getopts chp opt; do
	case "$opt" in
		c) CLONE=1;;
		h) helper;;
		p) PROD=1;;
	esac
done

if [ "m`id -u`" != "m0" ]; then
	echo "You need to be root"
	exit 0
fi

cd `dirname $0`
echo "Current directory is now `pwd`"
PP=`pwd`

#On PC
#tar -cjpf a.tbz2 app/ auth/ kernel/ rootfs/ screenAvr/ moduleApache2/ pam/ moduleIpApache2/ private/install.sh private/modules/ private/preseed*.cfg
#scp a.tbz2 build/img/clone.tbz2 ai@192.168.10.8:/tmp
#On device
#tar -xjpf /tmp/a.tbz2
lsb_release -a | grep trixie
if [ $? = 0 ]; then
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
cd /home/ai
mkdir /home/ai/build
sed -i -e 's|/root|/home/ai|' /etc/passwd
rm -rf /root
sed -i -e 's|# "\\e\[5~": history-search-backward|"\\e\[5~": history-search-backward|' /etc/inputrc
sed -i -e 's|# "\\e\[6~": history-search-forward|"\\e\[6~": history-search-forward|' /etc/inputrc
sed -i -e 's|%sudo	ALL=(ALL:ALL) ALL|%sudo	ALL=(ALL:ALL) NOPASSWD:ALL|' /etc/sudoers
sed -i -e 's|HISTSIZE=.*|HISTSIZE=-1|' /home/ai/.bashrc
sed -i -e 's|HISTFILESIZE=.*|HISTFILESIZE=-1|' /home/ai/.bashrc
ln -sf /lib/systemd/system/serial-getty@.service /etc/systemd/system/getty.target.wants/serial-getty@ttyGS0.service
ln -sf /etc/systemd/system/dongle-app.service /etc/systemd/system/multi-user.target.wants/dongle-app.service
ln -sf /etc/systemd/system/dongle-init.service /etc/systemd/system/sysinit.target.wants/dongle-init.service
ln -sf /etc/systemd/system/dongle-otg.service /etc/systemd/system/sysinit.target.wants/dongle-otg.service
echo -n " modules-load=dwc2,libcomposite,configs,dongle" >> /boot/firmware/cmdline.txt
sed -i -e 's/ root=[^ ]* / root=LABEL=rootfs /' /boot/firmware/cmdline.txt
sed -i -e 's/console=tty1 console=serial0,115200/console=serial0,115200 console=tty1/' /boot/firmware/cmdline.txt
sed -i -e 's/cfg80211.ieee80211_regdom=US/cfg80211.ieee80211_regdom=00/' /boot/firmware/cmdline.txt
cat > /boot/firmware/config.txt <<EOF
auto_initramfs=1
arm_64bit=1
arm_boost=1

[all]
dtoverlay=dwc2
dtoverlay=dongle
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
fatlabel /dev/nvme0n1p1 bootfs
e2label /dev/nvme0n1p2 rootfs
mkdir /disk
adduser --comment Administrator --home /disk/admin --disabled-password admin
usermod -a -G adm,dialout,cdrom,audio,video,plugdev,games,users,input,render,netdev,spi,i2c,gpio,bluetooth admin
sed -i -e 's|# User privilege specification|# User privilege specification\nadmin ALL=(ALL:ALL) NOPASSWD: /sbin/shutdown -h now, /sbin/reboot, /usr/local/modules/mydonglecloud/setup.sh|' /etc/sudoers
mkdir -p /usr/local/modules/pam && echo -e "#!/bin/sh\nexit 0" > /usr/local/modules/pam/pam.sh && chmod a+x /usr/local/modules/pam/pam.sh
TT=`cat /etc/pam.d/common-auth`
cat > /etc/pam.d/common-auth <<EOF
auth [success=ignore default=1] pam_oath.so usersfile=/disk/admin/modules/pam/oath.txt
auth sufficient pam_exec.so /usr/local/modules/pam/pam.sh oath_success
auth sufficient /usr/local/modules/pam/pam_app.so
session optional pam_exec.so /usr/local/modules/pam/pam.sh
$TT
EOF

mkdir -p /usr/local/modules/mydonglecloud
usermod -a -G adm,dialout,cdrom,audio,video,plugdev,games,users,input,render,netdev,spi,i2c,gpio,bluetooth ai
usermod -a -G sudo ai

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
if [ $OS = "ubuntu" ]; then
	chmod a-x /etc/update-motd.d/*
	which snapd
	if [ $? = 0 ]; then
		snap remove snapd
		apt-get -y purge snapd
	fi
	apt-get -y install bzip2 zip gpiod net-tools wireless-tools build-essential curl wget nano initramfs-tools device-tree-compiler
fi
apt-get -y install evtest qrencode dos2unix lrzsz squashfs-tools libpam-oath oathtool cryptsetup-bin cmake lsof hdparm screen figlet toilet composer network-manager bind9 acl jq telnet netcat-openbsd pamtester nmap ncat fd-find ncdu expect rsyslog
apt-get -y install liboath-dev libinput-dev libboost-dev libboost-system-dev libboost-thread-dev libboost-filesystem-dev libcurl4-openssl-dev libssl-dev libbluetooth-dev libturbojpeg0-dev libldap-dev libsasl2-dev apache2-dev libpam0g-dev libnm-dev libjwt-dev libsystemd-dev libdb-dev
if [ $OS = "ubuntu" ]; then
	apt-get -y install libprotobuf32t64 libjpeg62-dev
elif [ $OS = "pios" ]; then
	apt-get -y install libprotobuf32 libjpeg62-turbo-dev
fi

echo "################################"
echo "python"
echo "################################"
if [ $OS = "0" ]; then
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
apt-get -y install python3-venv python3-intelhex python3-certbot-apache python3-setuptools python3-attr python3-wheel python3-wheel-whl cython3 python3-dateutil python3-sniffio python3-astroid python3-tomlkit python3-isort python3-mccabe python3-platformdirs python3-serial python3-dill python3-dotenv python3-pytzdata

echo "################################"
echo "mysql"
echo "################################"
if [ $OS = "ubuntu" ]; then
	apt-get -y install mysql-server-8.0
	apt-get -y install mysql-server
elif [ $OS = "pios" ]; then
	apt-get -y install libevent-pthreads-2.1-7 libmecab2
	cd /home/ai/build
	wget -nv https://ports.ubuntu.com/pool/main/liba/libaio/libaio1_0.3.112-13build1_arm64.deb
	wget -nv https://ports.ubuntu.com/pool/main/i/icu/libicu70_70.1-2ubuntu1_arm64.deb
	wget -nv https://ports.ubuntu.com/pool/main/p/protobuf/libprotobuf-lite23_3.12.4-1ubuntu7.22.04.4_arm64.deb
	wget -nv https://ports.ubuntu.com/pool/main/m/mysql-defaults/mysql-common_5.8+1.1.1ubuntu1_all.deb
	wget -nv https://ports.ubuntu.com/pool/main/m/mysql-8.0/mysql-server-core-8.0_8.0.44-0ubuntu0.22.04.1_arm64.deb
	wget -nv https://ports.ubuntu.com/pool/main/m/mysql-8.0/mysql-server-8.0_8.0.44-0ubuntu0.22.04.1_arm64.deb
	wget -nv https://ports.ubuntu.com/pool/main/m/mysql-8.0/mysql-client-core-8.0_8.0.44-0ubuntu0.22.04.1_arm64.deb
	wget -nv https://ports.ubuntu.com/pool/main/m/mysql-8.0/mysql-client-8.0_8.0.44-0ubuntu0.22.04.1_arm64.deb
	wget -nv https://ports.ubuntu.com/pool/main/m/mysql-8.0/libmysqlclient-dev_8.0.44-0ubuntu0.22.04.1_arm64.deb
	wget -nv https://ports.ubuntu.com/pool/main/m/mysql-8.0/libmysqlclient21_8.0.44-0ubuntu0.22.04.1_arm64.deb
	dpkg -i libaio1* libicu70* libprotobuf-lite23* mysql-common*
	dpkg -i mysql-client* mysql-server*
	dpkg -i libmysqlclient*
	cd ..
	apt-mark hold mariadb-common
fi

echo "################################"
echo "adminer"
echo "################################"
apt-get -y install adminer

echo "################################"
echo "clang"
echo "################################"
apt-get -y install clang cargo

echo "################################"
echo "postfix"
echo "################################"
cat $PP/preseed_postfix.cfg | debconf-set-selections
apt-get -y install postfix swaks s-nail

echo "################################"
echo "Modules via apt"
echo "################################"
apt-get -y install certbot dovecot-imapd dovecot-pop3d ffmpeg fscrypt goaccess hugo imagemagick libapache2-mod-php libapache2-mod-authnz-external libpam-fscrypt mosquitto nginx pandoc php php-json php-mysql php-sqlite3 php-xml php-yaml php-curl php-zip php-apcu php-memcache php-redis php-ldap procmail rspamd sqlite3 php-imagick

echo "################################"
echo "apache2"
echo "################################"
apt-get -y install apache2
rm -f /etc/apache2/sites-enabled/*
rm -f /etc/apache2/ports.conf

echo "################################"
echo "cockpit"
echo "################################"
apt-get -y install cockpit
rm /etc/issue.d/cockpit.issue

echo "################################"
echo "postgresql"
echo "################################"
apt-get -y install postgresql

echo "################################"
echo "redis"
echo "################################"
apt-get -y install redis-server
redis-cli ping

echo "################################"
echo "roundcube"
echo "################################"
cat $PP/preseed_roundcube.cfg | debconf-set-selections
apt-get -y install roundcube
cp /etc/roundcube/config.inc.php /etc/roundcube/config.inc.php.template
chmod 666 /etc/roundcube/config.inc.php
chmod 644 /etc/roundcube/config.inc.php.template
ln -sf /usr/local/modules/onetimeemail/autologin.php /usr/share/roundcube

echo "################################"
echo "kernel (Dongle Pro)"
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
	apt-get -y purge linux-headers*rpi-v8 linux-image*rpi-v8
	rm -f /boot/cmdline.txt /boot/issue.txt /boot/config.txt
	rm -f /boot/firmware/LICENCE.broadcom /boot/firmware/issue.txt
fi

echo "################################"
echo "docker"
echo "################################"
curl -fsSL https://download.docker.com/linux/debian/gpg -o /etc/apt/keyrings/docker.asc
if [ $OS = "ubuntu" ]; then
	echo "deb [arch=arm64 signed-by=/etc/apt/keyrings/docker.asc] https://download.docker.com/linux/ubuntu noble stable" > /etc/apt/sources.list.d/docker.list
elif [ $OS = "pios" ]; then
	echo "deb [arch=arm64 signed-by=/etc/apt/keyrings/docker.asc] https://download.docker.com/linux/debian trixie stable" > /etc/apt/sources.list.d/docker.list
fi
apt-get update
apt-get -y install docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin
usermod -aG docker admin

echo "################################"
echo "java"
echo "################################"
apt-get -y install openjdk-21-jdk openjdk-21-jre openjdk-21-jre-headless

echo "################################"
echo "jitsi"
echo "################################"
curl -fsSL https://download.jitsi.org/jitsi-key.gpg.key | gpg --dearmor -o /etc/apt/keyrings/jitsi.gpg
echo "deb [arch=arm64 signed-by=/etc/apt/keyrings/jitsi.gpg] https://download.jitsi.org stable/" > /etc/apt/sources.list.d/jitsi.list
apt-get update
cat $PP/preseed_jitsi.cfg | debconf-set-selections
apt-get -y install jitsi-videobridge2 jitsi-meet-web-config jitsi-meet-web
#apt-get -y install jitsi-meet

echo "################################"
echo "rethinkdb"
echo "################################"
cd /home/ai/build
wget -nv https://download.rethinkdb.com/repository/debian-bookworm/pool/r/rethinkdb/rethinkdb_2.4.4~0bookworm_arm64.deb
dpkg -i rethinkdb*.deb

echo "################################"
echo "typesense"
echo "################################"
cd /home/ai/build
wget -nv https://dl.typesense.org/releases/29.0/typesense-server-29.0-arm64-lg-page16.deb
dpkg -i typesense-server*.deb

echo "################################"
echo "mongodb"
echo "################################"
curl -fsSL https://www.mongodb.org/static/pgp/server-8.0.asc | sudo gpg --dearmor -o /etc/apt/keyrings/mongodb.gpg
echo "deb [arch=arm64 signed-by=/etc/apt/keyrings/mongodb.gpg] https://repo.mongodb.org/apt/ubuntu noble/mongodb-org/8.2 multiverse" > /etc/apt/sources.list.d/mongodb.list
apt-get update
apt-get install -y mongodb-org
sed -i -e "s|^  dbPath:.*|  dbPath: /disk/admin/modules/mongodb|" /etc/mongod.conf
echo "security.authorization: enabled" >> /etc/mongod.conf
rm -f /usr/lib/systemd/system/mongod.service

echo "################################"
echo "devmem2"
echo "################################"
cd /home/ai/build
wget -nv https://bootlin.com/pub/mirror/devmem2.c
gcc -o /usr/local/bin/devmem2 devmem2.c

echo "################################"
echo "frp"
echo "################################"
cd /home/ai/build
wget -nv https://github.com/fatedier/frp/releases/download/v0.65.0/frp_0.65.0_linux_arm64.tar.gz
tar -xpf frp_*_linux_arm64.tar.gz
mkdir /usr/local/modules/frp
mv frp_*_linux_arm64/frpc /usr/local/modules/frp

echo "################################"
echo "katana"
echo "################################"
cd /home/ai/build
wget -nv https://github.com/projectdiscovery/katana/releases/download/v1.3.0/katana_1.3.0_linux_arm64.zip
unzip katana*.zip
mv katana /usr/local/bin/

echo "################################"
echo "livecodes"
echo "################################"
cd /home/ai/build
wget -nv https://github.com/live-codes/livecodes/releases/download/v46/livecodes-v46.tar.gz
mkdir /usr/local/modules/livecodes
tar -xpf livecodes-v46.tar.gz -C /usr/local/modules/livecodes --strip-components=1

echo "################################"
echo "meilisearch"
echo "################################"
cd /home/ai/build
curl -L -sS --fail https://install.meilisearch.com | sh
chmod a+x meilisearch
mv meilisearch /usr/local/bin/

echo "################################"
echo "qdrant"
echo "################################"
cd /home/ai/build
wget -nv https://github.com/qdrant/qdrant/releases/download/v1.14.1/qdrant-aarch64-unknown-linux-musl.tar.gz
tar -xpf qdrant-aarch64-unknown-linux-musl.tar.gz
chmod a+x qdrant
mkdir /usr/local/modules/qdrant
mv qdrant /usr/local/modules/qdrant

echo "################################"
echo "triliumnotes"
echo "################################"
cd /home/ai/build
wget -nv https://github.com/TriliumNext/Notes/releases/download/v0.95.0/TriliumNextNotes-Server-v0.95.0-linux-arm64.tar.xz
tar -xJpf TriliumNextNotes-Server*
mv TriliumNextNotes-Server-0.*/ /usr/local/modules/triliumnotes
rm -rf /usr/local/modules/triliumnotes/node
ln -sf /etc/systemd/system/triliumnotes.service /etc/systemd/system/multi-user.target.wants/triliumnotes.service

echo "################################"
echo "ytdlp"
echo "################################"
curl -L -sS --fail https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp_linux_aarch64 -o /usr/local/bin/yt-dlp
chmod a+x /usr/local/bin/yt-dlp
chown admin:admin /usr/local/bin/yt-dlp
ln -sf /usr/local/bin/yt-dlp /usr/local/bin/youtube-dl

echo "################################"
echo "node"
echo "################################"
cd /home/ai/build
FILENODE=`wget -q -O - https://nodejs.org/dist/latest-v22.x/ | grep "\-linux\-arm64\.tar\.xz" | sed -E "s|.*>([^<]*)<.*|\1|"`
wget -nv https://nodejs.org/dist/latest-v22.x/$FILENODE
tar -xJpf node-v*
cp -a node-v*/bin/ node-v*/include/ node-v*/lib/ node-v*/share/ /usr/local
cd ..
npm -g update npm

echo "################################"
echo "npm Packages"
echo "################################"
npm -g install @angular/cli @ionic/cli @vue/cli cordova-res

echo "################################"
echo "zigbee2mqtt"
echo "################################"
mkdir /usr/local/modules/zigbee2mqtt
cd /usr/local/modules/zigbee2mqtt
npm install zigbee2mqtt@2.5.1
rm -rf /usr/local/modules/zigbee2mqtt/node_modules/zigbee2mqtt/data
ln -sf /etc/systemd/system/zigbee2mqtt.service /etc/systemd/system/multi-user.target.wants/zigbee2mqtt.service

echo "################################"
echo "phplist"
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
	cd /home/ai/build
	wget -nv https://files.pythonhosted.org/packages/41/07/876153f611f2c610bdb8f706a5ab560d888c938ea9ea65ed18c374a9014a/pcpp-1.30.tar.gz
	tar -xpf pcpp-1.30.tar.gz
	cd pcpp-1.30
	python3 setup.py install
fi

echo "################################"
echo "pymcuprog"
echo "################################"
cd /home/ai/build
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
echo "postfixparser"
echo "################################"
cd /home/ai/build
git clone https://github.com/Privex/python-loghelper
cd python-loghelper
python3 setup.py install
cd ..
git clone https://github.com/Privex/python-helpers
cd python-helpers
python3 setup.py install
cd ../..

echo "################################"
echo "homeassistant"
echo "################################"
cd /home/ai
/home/ai/rootfs/usr/local/modules/mydonglecloud/pip.sh -f /usr/local/modules/homeassistant -s
echo "PATH before any modif: $PATH"
PATHOLD=$PATH
PATH=/usr/local/modules/homeassistant/bin:$PATHOLD
export PATH=/usr/local/modules/homeassistant/bin:$PATHOLD
echo "PATH new: $PATH python: `python --version`"
pip install homeassistant==2025.10.3 isal==1.8.0 zlib_ng==1.0.0 pyotp==2.9.0 pyqrcode==1.2.1 ha-ffmpeg==3.2.2 home-assistant-frontend==20251001.4 av==13.1.0 mutagen==1.47.0 numpy==2.3.2 PyTurboJPEG==1.8.0 aiodhcpwatcher==1.2.1 aiodiscover==2.7.1 cached-ipaddress==0.10.0 go2rtc-client==0.2.1 PyNaCl==1.6.0 file-read-backwards==2.0.0 python-matter-server==8.1.0 pymicro-vad==1.0.1 pyspeex-noise==1.0.2 async-upnp-client==0.45.0 home-assistant-intents==2025.10.1 hassil==3.2.0 aiousbwatcher==1.1.1 pyserial==3.5 bleak==1.0.1 bluetooth-adapters==2.1.0 bluetooth-data-tools==1.28.2 dbus-fast==2.44.3 habluetooth==5.6.4 pycountry==24.6.1 radios==0.3.2 gTTS==2.5.3 PyMetno==0.13.0 paho-mqtt==2.1.0 bleak-esphome==3.3.0 esphome-dashboard-api==1.3.0 aioesphomeapi==41.11.0 aioshelly==13.10.0 aioruuvigateway==0.1.0 xiaomi-ble==1.2.0 ibeacon-ble==1.2.0
PATH=$PATHOLD
export PATH=$PATHOLD
echo "PATH restored: $PATH"

echo "################################"
echo "unmanic"
echo "################################"
cd /home/ai
/home/ai/rootfs/usr/local/modules/mydonglecloud/pip.sh -f /usr/local/modules/unmanic -s
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
echo "transmission"
echo "################################"
apt-get -y install transmission-common transmission-daemon transmission-cli
sed -i -e "s|^User=.*|User=admin\nEnvironment=TRANSMISSION_HOME=/disk/admin/modules/transmission|" /usr/lib/systemd/system/transmission-daemon.service

echo "################################"
echo "umtp"
echo "################################"
cd /home/ai/build
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
cd /home/ai/build
git clone https://github.com/1248/cc2538-prog
cd cc2538-prog
make
cp cc2538-prog /usr/local/bin/
cd ../..

echo "################################"
echo "acme"
echo "################################"
cd /usr/local/modules
mkdir /usr/local/modules/acme
cd acme
wget -nv https://raw.githubusercontent.com/acmesh-official/acme.sh/refs/tags/3.1.1/acme.sh
chmod a+x acme.sh

echo "################################"
echo "libreqr"
echo "################################"
cd /usr/local/modules
git clone https://code.antopie.org/miraty/libreqr.git libreqr
cd libreqr
git checkout 2.0.1
rm -rf .git
composer -n install
chown -R www-data:www-data /usr/local/modules/libreqr/css

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
	clone docsify docsifyjs/docsify v4.13.1
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
	clone librechat danny-avila/LibreChat v0.7.9
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
	clone tabby TabbyML/tabby v0.31.2
	clone tabby/crates/llama-cpp-server/llama.cpp ggerganov/llama.cpp 16cc3c606efe1640a165f666df0e0dc7cc2ad869
	clone tubesync meeb/tubesync v0.15.10
	clone uptime louislam/uptime-kuma 1.23.16
	clone webssh2 billchurch/webssh2 webssh2-server-v2.3.4
	clone webtrees fisharebest/webtrees 2.1.25
	clone yourls YOURLS/YOURLS 1.10.2
fi

echo "################################"
echo "audiobookshelf"
echo "################################"
cd /usr/local/modules/audiobookshelf
npm install

echo "################################"
echo "bugzilla"
echo "################################"
apt-get -y install libcgi-pm-perl libemail-mime-perl libemail-sender-perl libemail-address-perl libmath-random-isaac-perl liblist-moreutils-perl libjson-xs-perl libdatetime-timezone-perl libdbi-perl libtest-taint-perl libdbd-sqlite3-perl libencode-detect-perl libio-stringy-perl libxml-twig-perl libwww-perl libtext-multimarkdown-perl liburi-db-perl libfile-copy-recursive-perl libfile-which-perl libpod-pom-view-restructured-perl libhtml-scrubber-perl libemail-reply-perl libhtml-formattext-withlinks-perl libjson-rpc-perl libcache-memcached-fast-perl libchart-perl libgd-perl libgd-graph-perl libgd-text-perl libtemplate-plugin-gd-perl libmoox-strictconstructor-perl libtype-tiny-perl libdaemon-generic-perl libtheschwartz-perl libapache2-mod-perl2 libdbd-pg-perl libfile-mimeinfo-perl libsoap-lite-perl libxmlrpc-lite-perl
cd /usr/local/modules/bugzilla
ln -sf /disk/admin/modules/bugzilla/data
ln -sf /disk/admin/modules/bugzilla/localconfig
cd /home/ai/build
wget https://cpan.metacpan.org/authors/id/A/AB/ABW/Template-Toolkit-3.101.tar.gz
tar -xzf Template-Toolkit-3.101.tar.gz
cd Template-Toolkit-3.101
yes | perl Makefile.PL
make
make install

echo "################################"
echo "docsify"
echo "################################"
cd /usr/local/modules/docsify
npm install
npm run build

echo "################################"
echo "flarum"
echo "################################"
cd /usr/local/modules/flarum
composer -n install

echo "################################"
echo "freshrss"
echo "################################"
cd /usr/local/modules/freshrss
npm install

echo "################################"
echo "gitea"
echo "################################"
cd /usr/local/modules/gitea
npm install

echo "################################"
echo "grav"
echo "################################"
cd /usr/local/modules/grav
bin/grav install

echo "################################"
echo "joomla"
echo "################################"
cd /usr/local/modules/joomla
composer -n install
npm ci

echo "################################"
echo "librechat"
echo "################################"
cd /usr/local/modules/librechat
npm install
npm install @smithy/signature-v4 @smithy/eventstream-codec
echo 3 > /proc/sys/vm/drop_caches
npm run frontend
echo 3 > /proc/sys/vm/drop_caches
ln -sf /disk/admin/modules/librechat/.env
ln -sf /disk/admin/modules/librechat/logs
ln -sf /disk/admin/modules/librechat/librechat.yaml
ln -sf /disk/admin/modules/librechat/logs api/

echo "################################"
echo "mantisbugtracker"
echo "################################"
cd /usr/local/modules/mantisbugtracker
composer -n install
mv config config.bak
ln -sf /disk/admin/modules/mantisbugtracker/config

echo "################################"
echo "osticket"
echo "################################"
cd /usr/local/modules/osticket
ln -sf /disk/admin/modules/osticket/ost-config.php include/ost-config.php

echo "################################"
echo "projectsend"
echo "################################"
cd /usr/local/modules/projectsend
composer -n install
npm install gulp-cli
./node_modules/.bin/gulp build
rm -rf node_modules
ln -sf /disk/admin/modules/projectsend/sys.config.php includes/sys.config.php
mv upload/files upload/files.bak
ln -sf /disk/admin/modules/projectsend/files upload/
mv upload/temp upload/temp.bak
ln -sf /disk/admin/modules/projectsend/temp upload/
rm -rf cache
ln -sf /disk/admin/modules/projectsend/cache

echo "################################"
echo "metube"
echo "################################"
cd /usr/local/modules/metube
/home/ai/rootfs/usr/local/modules/mydonglecloud/pip.sh -f /usr/local/modules/metube/env -s
echo "PATH before any modif: $PATH"
PATHOLD=$PATH
PATH=/usr/local/modules/metube/env/bin:$PATHOLD
export PATH=/usr/local/modules/metube/env/bin:$PATHOLD
echo "PATH new: $PATH python: `python --version`"
pip install aiohttp yt_dlp mutagen curl-cffi watchfiles python-socketio
PATH=$PATHOLD
export PATH=$PATHOLD
echo "PATH restored: $PATH"
cd ui
npm ci
yes no | node_modules/.bin/ng build --configuration production
mkdir /disk/admin/modules/metube

echo "################################"
echo "stirlingpdf"
echo "################################"
apt-get install -y libleptonica-dev zlib1g-dev libreoffice-writer libreoffice-calc libreoffice-impress unpaper ocrmypdf
mv /usr/local/modules/stirlingpdf /home/ai/build
mkdir /usr/local/modules/stirlingpdf
cd /home/ai/build/stirlingpdf
/home/ai/rootfs/usr/local/modules/mydonglecloud/pip.sh -f /usr/local/modules/stirlingpdf -s
echo "PATH before any modif: $PATH"
PATHOLD=$PATH
PATH=/usr/local/modules/stirlingpdf/bin:$PATHOLD
export PATH=/usr/local/modules/stirlingpdf/bin:$PATHOLD
echo "PATH new: $PATH python: `python --version`"
pip install uno opencv-python-headless unoconv pngquant WeasyPrint
export JAVA_HOME=/usr/lib/jvm/java-21-openjdk-arm64
./gradlew build
cp -a scripts stirling-pdf/build/libs/stirling-pdf-*.jar /usr/local/modules/stirlingpdf
PATH=$PATHOLD
export PATH=$PATHOLD
echo "PATH restored: $PATH"
ln -sf /etc/systemd/system/stirlingpdf.service /etc/systemd/system/multi-user.target.wants/stirlingpdf.service

echo "################################"
echo "tabby"
echo "################################"
apt-get -y install protobuf-compiler libopenblas-dev graphviz
cd /usr/local/modules/tabby
echo 3 > /proc/sys/vm/drop_caches
cargo build --release
ln -sf /etc/systemd/system/tabby.service /etc/systemd/system/multi-user.target.wants/tabby.service

echo "################################"
echo "tubesync"
echo "################################"
apt-get -y install libonig-dev
cd /usr/local/modules/tubesync
/home/ai/rootfs/usr/local/modules/mydonglecloud/pip.sh -f /usr/local/modules/tubesync/env -s
echo "PATH before any modif: $PATH"
PATHOLD=$PATH
PATH=/usr/local/modules/tubesync/env/bin:$PATHOLD
export PATH=/usr/local/modules/tubesync/env/bin:$PATHOLD
echo "PATH new: $PATH python: `python --version`"
pip install django django-huey django-sass-processor pillow whitenoise gunicorn httptools django-basicauth psycopg PySocks urllib3 requests yt-dlp emoji brotli html5lib bgutil-ytdlp-pot-provider babi curl-cffi libsass django-compressor
cd tubesync
sed -i -e 's|/config/tasks/|/disk/admin/modules/tubesync/tasks/|' common/huey.py
sed -i -e 's|import yt_dlp.patch|#import yt_dlp.patch|' sync/youtube.py
cp tubesync/local_settings.py.example tubesync/local_settings.py
cat >> tubesync/local_settings.py <<EOF
DOWNLOAD_ROOT = Path('/disk/admin/modules/tubesync/downloads')
ALLOWED_HOSTS = ['*']
#CSRF_TRUSTED_ORIGINS = ['*']
EOF
./manage.py migrate
cat >> tubesync/local_settings.py <<EOF
DATABASES = {
    'default': {
        'ENGINE': 'django.db.backends.sqlite3',
        'NAME': Path('/disk/admin/modules/tubesync/db.sqlite3'),
    }
}
DATABASE_CONNECTION_STR = f'sqlite at "{DATABASES["default"]["NAME"]}"'
EOF
./manage.py compilescss
./manage.py collectstatic
chown -R admin:admin /disk/admin/modules/tubesync
PATH=$PATHOLD
export PATH=$PATHOLD
echo "PATH restored: $PATH"

echo "################################"
echo "webssh2"
echo "################################"
cd /usr/local/modules/webssh2
npm install --production
node scripts/prestart.js
cat > config.json <<EOF
{
	"listen": {
		"ip": "127.0.0.1",
		"port": 2222
	},
	"ssh": {
		"host": "localhost",
		"port": 22
	},
	"auth": {
		"method": "post"
	}
}
EOF
ln -sf /etc/systemd/system/webssh2.service /etc/systemd/system/multi-user.target.wants/webssh2.service

echo "################################"
echo "webtrees"
echo "################################"
cd /usr/local/modules/webtrees
composer -n install
mv data data.bak
ln -sf /disk/admin/modules/webtrees/data

echo "################################"
echo "yourls"
echo "################################"
cd /usr/local/modules/yourls
ln -sf /disk/admin/modules/yourls/config.php user/config.php

echo "################################"
echo "betterauth"
echo "################################"
ln -sf /etc/systemd/system/betterauth.service /etc/systemd/system/multi-user.target.wants/betterauth.service
ln -sf /etc/systemd/system/betterauth-studio.service /etc/systemd/system/multi-user.target.wants/betterauth-studio.service
cd /home/ai/auth
./prepare.sh -i

echo "################################"
echo "Services"
echo "################################"
cd /home/ai
cp -a $PP/modules/services/* /etc/systemd/system/

echo "################################"
echo "mydonglecloud and rootfs"
echo "################################"
cd /home/ai
chown -R root:root rootfs
chown -R admin:admin rootfs/disk/admin
cp -a rootfs/* /
rm -rf rootfs
cd /home/ai/kernel
make
mkdir -p /boot/firmware/overlays
make install
cd /home/ai/app
./lvgl.sh -b -c
make
cd /home/ai/moduleApache2
make
cd /home/ai/pam
make
chown -R root:root /usr/local
chown -R ai:ai /home/ai
chown -R www-data:admin /disk/admin/modules/roundcube
mv /var/lib/mysql /disk/admin/modules
chown mysql:mysql /disk/admin/modules/mysql
ln -sf /disk/admin/modules/mysql /var/lib/mysql
chown -R admin:admin /disk/admin/modules/metube

echo "################################"
echo "Cleanup"
echo "################################"
if [ $OS = "pios" ]; then
	apt-get -y purge python3-rpi-lgpio rpicam-apps-core rpicam-apps-lite
fi
apt-get -y autoremove
rm -f /etc/systemd/system/multi-user.target.wants/nginx.service
rm -f /etc/systemd/system/multi-user.target.wants/named.service
rm -f /etc/systemd/system/multi-user.target.wants/sshswitch.service
rm -rf /var/cache/apt/archives/*.deb /home/ai/build/*.deb /home/ai/build/*.xz /home/ai/build/*.gz /home/ai/.cache/*
rm -rf /root /lost+found /usr/local/games /opt/containerd /opt/pigpio
rm -rf /var/lib/bluetooth /var/lib/docker /var/lib/raspberrypi /var/lib/NetworkManager /var/cache-admin
mkdir /var/cache-admin /var/log/mydonglecloud /var/log/zigbee2mqtt /var/log/triliumnotes
chown admin:admin /var/cache-admin /var/log/mydonglecloud /var/log/zigbee2mqtt /var/log/triliumnotes

echo "################################"
echo "Finish install"
echo "################################"
sync
sync
date
DATEFINISH=`date +%s`
DELTA=$((DATEFINISH - DATESTART))
echo "Duration: $((DELTA / 3600))h $(((DELTA % 3600) / 60))m $((DELTA % 60))s"

if [ $PROD = 1 ]; then
	sed -i -e 's|ai:[^:]*:|ai:*:|' /etc/shadow
	sed -i -e 's|ai:[^:]*:|ai:*:|' /etc/shadow-
	rm -rf /home/ai
	mkdir /home/ai
	chown -R 1000:1000 /home/ai
fi
