#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for metube [-h]"
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

echo "#Reset metube##################"
systemctl stop metube.service
rm -rf /disk/admin/modules/metube
mkdir /disk/admin/modules/metube
systemctl start metube.service
systemctl enable metube.service

echo {" \"a\":\"status\", \"module\":\"$(basename $0 .sh)\", \"state\":\"finish\" }" | websocat -1 ws://localhost:8094
