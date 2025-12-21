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

installModule() {
	if [ -z $1 ]; then
		return
	fi
	LIST2=`jq -r '.default.setupDependencies | join(" ")' $PP/modules/$1.json 2> /dev/null`
	for tt in $LIST2; do
		installModule $tt
	done
	if [ ! -f /home/ai/build/_modulesInstalled/$1 -a -f $PP/modules/install/$1.sh ]; then
		echo "################################"
		echo "Module: $1"
		echo "################################"
		OS=$OS PP=$PP $PP/modules/install/$1.sh
		touch /home/ai/build/_modulesInstalled/$1
		DATEFINISHM=`date +%s`
		DELTAM=$((DATEFINISHM - DATESTARTM))
		echo "Done in $((DELTAM / 60))m $((DELTAM % 60))s"
	fi
}

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
usermod -a -G adm,dialout,cdrom,audio,video,plugdev,games,users,input,render,netdev,spi,i2c,gpio,bluetooth ai
usermod -a -G sudo ai
mkdir -p /usr/local/modules/mydonglecloud
mkdir -p /home/ai/build/_modulesInstalled
mkdir -p /disk/admin/modules/_config_

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
apt-get -y install evtest qrencode dos2unix lrzsz libpam-oath oathtool cryptsetup-bin cmake lsof hdparm screen figlet toilet composer network-manager bind9 acl jq telnet netcat-openbsd pamtester expect rsyslog
apt-get -y install liboath-dev libinput-dev libboost-dev libboost-system-dev libboost-thread-dev libboost-filesystem-dev libcurl4-openssl-dev libssl-dev libbluetooth-dev libturbojpeg0-dev libldap-dev libsasl2-dev apache2-dev libpam0g-dev libnm-dev libjwt-dev libsystemd-dev libdb-dev libsqlite3-dev
if [ $OS = "ubuntu" ]; then
	apt-get -y install libprotobuf32t64 libjpeg62-dev
elif [ $OS = "pios" ]; then
	apt-get -y install libprotobuf32 libjpeg62-turbo-dev
fi

echo "################################"
echo "python"
echo "################################"
apt-get -y install python3-venv python3-intelhex python3-certbot-apache python3-setuptools python3-attr python3-wheel python3-wheel-whl cython3 python3-dateutil python3-sniffio python3-astroid python3-tomlkit python3-isort python3-mccabe python3-platformdirs python3-serial python3-dill python3-dotenv python3-pytzdata

echo "################################"
echo "Early install"
echo "################################"
installModule mysql
installModule postfix
installModule python

echo "################################"
echo "Modules via apt"
echo "################################"
apt-get -y install certbot dovecot-imapd dovecot-pop3d ffmpeg fscrypt goaccess hugo imagemagick libapache2-mod-php libapache2-mod-authnz-external libpam-fscrypt mosquitto nginx pandoc php php-json php-mysql php-gd php-sqlite3 php-xml php-yaml php-curl php-zip php-apcu php-memcache php-redis php-ldap procmail rspamd sqlite3 php-imagick

echo "################################"
echo "devmem2"
echo "################################"
cd /home/ai/build
wget -nv https://bootlin.com/pub/mirror/devmem2.c
gcc -o /usr/local/bin/devmem2 devmem2.c

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
npm -g install @angular/cli @ionic/cli @vue/cli cordova-res pnpm

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
echo "cc2538-prog"
echo "################################"
cd /home/ai/build
git clone https://github.com/1248/cc2538-prog
cd cc2538-prog
make
cp cc2538-prog /usr/local/bin/
cd ../..

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
	clone beautifierweb beautifier/beautifier.io a1fa4975
	clone bugzilla bugzilla/bugzilla release-5.3.3
	clone changedetection dgtlmoon/changedetection.io 0.50.7
	clone convertx C4illin/ConvertX v0.14.1
	clone crystal crystal-lang/crystal 1.18.2
	clone cssunminifier mrcoles/cssunminifier c5cad8ab
	clone cyberchef gchq/CyberChef v10.19.4
	clone discourse discourse/discourse v3.4.6
	clone docsify docsifyjs/docsify v4.13.1
	clone docusaurus facebook/docusaurus v3.9.2
	clone flarum flarum/flarum v1.8.1
	clone freshrss FreshRSS/FreshRSS 1.26.3
	clone gitea go-gitea/gitea v1.24.3
	clone grav getgrav/grav 1.7.48
	clone html5qrcode mebjas/html5-qrcode v2.3.8
	clone immich immich-app/immich v1.135.3
	clone invidious iv-org/invidious v2.20250913.0
	clone iopaint Sanster/IOPaint iopaint-1.5.3
	clone jellyfin jellyfin/jellyfin v10.10.7
	clone joomla joomla/joomla-cms 5.3.2
	clone joplin laurent22/joplin server-v3.4.1
	clone jstinker johncipponeri/jstinker master
	clone karakeep karakeep-app/karakeep v0.26.0
	clone librechat danny-avila/LibreChat v0.7.9
	clone librephotos LibrePhotos/librephotos HEAD
	clone limesurvey LimeSurvey/LimeSurvey 6.15.3+250708
	clone lobechat lobehub/lobe-chat v1.143.0
	clone mantisbugtracker mantisbt/mantisbt release-2.27.1
	clone markdowneditor jbt/markdown-editor v2
	clone maybe maybe-finance/maybe v0.5.0
	clone mermaid mermaid-js/mermaid mermaid@11.9.0
	clone metube alexta69/metube HEAD
	clone minio minio/minio RELEASE.2025-07-18T21-56-31Z
	clone mkdocs mkdocs/mkdocs 1.6.1
	clone ollama ollama/ollama v0.9.7-rc1
	clone osticket osTicket/osTicket v1.18.2
	clone passbolt passbolt/passbolt_api v5.7.2
	clone photoprism photoprism/photoprism 250707-d28b3101e
	clone photoview photoview/photoview v2.4.0
	clone phpbb phpbb/phpbb release-3.3.15
	clone phpsandbox Corveda/PHPSandbox v3.1
	clone pihole pi-hole/pi-hole v.6.1.4
	clone piped TeamPiped/Piped 1c3cfd23
	clone pipedbackend TeamPiped/Piped-Backend c5921f6b
	clone pipedproxy TeamPiped/Piped-Proxy b195686c
	clone prettier prettier/prettier 3.7.4
	clone privatebin PrivateBin/PrivateBin 1.7.8
	clone projectsend projectsend/projectsend r1720
	clone qrcodegenerator bizzycola/qrcode-generator HEAD
	clone radaar Radarr/Radarr v5.27.2.10142
	clone searxng searxng/searxng 74ec225a
	clone shields badges/shields server-2025-12-06
	clone silverbullet silverbulletmd/silverbullet 2.3.0
	clone sonarr Sonarr/Sonarr v4.0.15.2941
	clone stackedit benweet/stackedit v5.15.4
	clone stirlingpdf Stirling-Tools/Stirling-PDF v1.0.2
	clone stremio Stremio/stremio-web v5.0.0-beta.29
	clone sunrisecms cityssm/sunrise-cms v1.0.0-alpha.19
	clone superset apache/superset 5.0.0
	clone syncthing syncthing/syncthing v1.30.0
	clone tabby TabbyML/tabby v0.31.2
	clone tabby/crates/llama-cpp-server/llama.cpp ggerganov/llama.cpp 16cc3c606efe1640a165f666df0e0dc7cc2ad869
	clone transform ritz078/transform e1294208
	clone tubesync meeb/tubesync v0.15.10
	clone typesensedashboard bfritscher/typesense-dashboard v2.4.1
	clone umtpresponder viveris/uMTP-Responder umtprd-1.6.8
	clone uptime louislam/uptime-kuma 1.23.16
	clone webssh2 billchurch/webssh2 webssh2-server-v2.3.4
	clone webtrees fisharebest/webtrees 2.1.25
	clone yourls YOURLS/YOURLS 1.10.2
	clone zola getzola/zola v0.21.0
fi

echo "################################"
echo "Install via modules scripts"
echo "################################"
LIST=`find $PP/modules/ -name "*.json" -printf "%f\n" | sort`
for NAME in $LIST; do
	NAME="${NAME%?????}"
	installModule $NAME
done
for NAME in $LIST; do
	NAME="${NAME%???}"
	if [ ! -f /home/ai/build/_modulesInstalled/$NAME ]; then
		echo "Error: install script $NAME not executed"
	fi
done

echo "################################"
echo "Services"
echo "################################"
cd /home/ai
cp -a $PP/modules/services/* /etc/systemd/system/

echo "################################"
echo "Rootfs"
echo "################################"
cd /home/ai
chown -R root:root rootfs
chown -R admin:admin rootfs/disk/admin
cp -a rootfs/* /
rm -rf rootfs
chown -R root:root /usr/local
chown -R www-data:www-data /usr/local/modules/libreqr/css
chown -R ai:ai /home/ai
chown -R www-data:admin /disk/admin/modules/roundcube
chown -R admin:admin /usr/local/modules/lobechat/.next/cache /usr/local/modules/lobechat/.next/server
chown -R admin:admin /usr/local/modules/openwebui/lib/python3.11/site-packages/open_webui/static/
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
