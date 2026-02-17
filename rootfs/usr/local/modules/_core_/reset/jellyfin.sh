#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for jellyfin [-h]"
echo "h:	Print this usage and exit"
exit 0
}

if [ "m`id -u`" != "m0" ]; then
	echo "You need to be root"
	exit 0
fi

while getopts h opt
do
	case "$opt" in
		h) helper;;
	esac
done

echo "#Reset jellyfin##################"
systemctl stop jellyfin.service
rm -rf /disk/admin/modules/jellyfin
mkdir /disk/admin/modules/jellyfin
cp -a /etc/jellyfin.bak /disk/admin/modules/jellyfin/config
cp -a /var/lib/jellyfin.bak /disk/admin/modules/jellyfin/data
chown -R admin:admin /disk/admin/modules/jellyfin/
chown -R jellyfin:jellyfin /disk/admin/modules/jellyfin/config/
chown -R jellyfin:jellyfin /disk/admin/modules/jellyfin/data/
systemctl start jellyfin.service
systemctl enable jellyfin.service

/usr/local/modules/_core_/reset/jellyfin-user.sh &
