#!/bin/sh

if [ "$(id -u)" = "0" ]; then
	echo "You should not be root"
	exit 0
fi

echo "#Reset portainer##################"
systemctl stop portainer.service
rm -rf /disk/admin/modules/portainer
mkdir /disk/admin/modules/portainer
systemctl start portainer.service
systemctl enable portainer.service

/usr/local/modules/_core_/reset/portainer-user.sh &
