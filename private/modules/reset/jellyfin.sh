#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for jellyfin [-h -r -w]"
echo "h:	Print this usage and exit"
echo "r:	Reset"
echo "w:	Wait for user creation"
exit 0
}

if [ "m`id -u`" != "m0" ]; then
	echo "You need to be root"
	exit 0
fi

RESET=0
WAIT=0
while getopts hrw opt
do
	case "$opt" in
		h) helper;;
		r) RESET=1;;
		w) WAIT=1;;
	esac
done

if [ $RESET != 1 ]; then
	exit 0
fi

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
if [ $WAIT = 1 ]; then
	/usr/local/modules/mydonglecloud/reset/jellyfin-user.sh
else
	/usr/local/modules/mydonglecloud/reset/jellyfin-user.sh &
fi
