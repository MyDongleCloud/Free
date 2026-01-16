#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for portainer [-h -w]"
echo "h:	Print this usage and exit"
echo "w:	Wait for user creation"
exit 0
}

if [ "m`id -u`" = "m0" ]; then
	echo "You should not be root"
	exit 0
fi

RESET=0
WAIT=0
while getopts hrw opt
do
	case "$opt" in
		h) helper;;
		w) WAIT=1;;
	esac
done

if [ $RESET != 1 ]; then
	exit 0
fi

echo "#Reset portainer##################"
systemctl stop portainer.service
rm -rf /disk/admin/modules/portainer
mkdir /disk/admin/modules/portainer
systemctl start portainer.service
systemctl enable portainer.service
if [ $WAIT = 1 ]; then
	/usr/local/modules/mydonglecloud/reset/portainer-user.sh
else
	/usr/local/modules/mydonglecloud/reset/portainer-user.sh &
fi
