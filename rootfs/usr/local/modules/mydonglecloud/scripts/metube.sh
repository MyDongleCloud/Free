#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for metube [-h -r]"
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

echo "#Reset metube##################"
DATE=`date +%s`

systemctl stop metube.service
rm -rf /disk/admin/.modules/metube
mkdir /disk/admin/.modules/metube
systemctl start metube.service
