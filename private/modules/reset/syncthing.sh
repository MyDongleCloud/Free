#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for syncthing [-h]"
echo "h:	Print this usage and exit"
exit 0
}

if [ "m`id -u`" = "m0" ]; then
	echo "You should not be root"
	exit 0
fi

while getopts h opt
do
	case "$opt" in
		h) helper;;
	esac
done

echo "#Reset syncthing##################"
systemctl stop syncthing.service
rm -rf /disk/admin/modules/syncthing
mkdir /disk/admin/modules/syncthing
systemctl start syncthing.service
systemctl enable syncthing.service

echo -n "{ \"a\":\"status\", \"module\":\"$(basename $0 .sh)\", \"state\":\"finish\" }" | nc -w 1 localhost 8093
