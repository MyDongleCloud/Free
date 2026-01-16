#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for openwebui [-h]"
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

echo "#Reset openwebui##################"
systemctl stop openwebui.service
rm -rf /disk/admin/modules/openwebui
mkdir -p /disk/admin/modules/openwebui/data
SALT=$(tr -dc 'a-f0-9' < /dev/urandom | head -c 32)
cat > /disk/admin/modules/openwebui/.env << EOF
WEBUI_SECRET_KEY=${SALT}
OFFLINE_MODE=true
HF_HUB_OFFLINE=1
DATA_DIR=/disk/admin/modules/openwebui/data
EOF
systemctl start openwebui.service
systemctl enable openwebui.service

/usr/local/modules/mydonglecloud/reset/openwebui-user.sh &
