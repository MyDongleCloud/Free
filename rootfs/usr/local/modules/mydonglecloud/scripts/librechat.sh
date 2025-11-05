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

echo "#Reset librechat##################"
systemctl stop librechat.service
rm -rf /disk/admin/modules/librechat
mkdir -p /disk/admin/modules/librechat/logs
cp /usr/local/modules/librechat/.env.example /disk/admin/modules/librechat/env
systemctl start librechat.service
systemctl enable librechat.service
