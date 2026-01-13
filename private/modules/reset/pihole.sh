#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for pihole [-h -r]"
echo "h:	Print this usage and exit"
echo "r:	Reset"
exit 0
}

if [ "m`id -u`" = "m0" ]; then
	echo "You should not be root"
	exit 0
fi

RESET=0
while getopts hr opt
do
	case "$opt" in
		h) helper;;
		r) RESET=1;;
	esac
done

if [ $RESET != 1 ]; then
	exit 0
fi

echo "#Reset pihole##################"
systemctl stop pihole-FTL.service
rm -rf /disk/admin/modules/pihole
mkdir /disk/admin/modules/pihole
cp /etc/pihole/pihole.toml.bak /disk/admin/modules/pihole/pihole.toml
systemctl enable pihole-FTL.service
