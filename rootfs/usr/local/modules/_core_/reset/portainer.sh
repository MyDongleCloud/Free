#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for portainer [-h]"
echo "h:	Print this usage and exit"
echo "w:	Wait for user creation"
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

echo "#Reset portainer##################"
systemctl stop portainer.service
rm -rf /disk/admin/modules/portainer
mkdir /disk/admin/modules/portainer
systemctl start portainer.service
systemctl enable portainer.service

/usr/local/modules/_core_/reset/portainer-user.sh &
