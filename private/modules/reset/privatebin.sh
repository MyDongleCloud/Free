#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for privatebin [-h -r]"
echo "h:	Print this usage and exit"
echo "r:	Reset"
exit 0
}

if [ "m`id -u`" != "m0" ]; then
	echo "You need to be root"
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

echo "#Reset privatebin##################"
rm -rf /disk/admin/modules/privatebin
mkdir -p /disk/admin/modules/privatebin/data
cp /usr/local/modules/privatebin/cfg/conf.sample.php /disk/admin/modules/privatebin/conf.php
chown -R www-data:admin /disk/admin/modules/privatebin
