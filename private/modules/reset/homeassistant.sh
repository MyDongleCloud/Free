#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for homeassistant [-h -w]"
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

echo "#Reset homeassistant##################"
systemctl stop homeassistant.service
rm -rf /disk/admin/modules/homeassistant
mkdir /disk/admin/modules/homeassistant
cat > /disk/admin/modules/homeassistant/configuration.yaml <<EOF
default_config:

frontend:
  themes: !include_dir_merge_named themes

http:
  use_x_forwarded_for: true
  trusted_proxies:
    - ::1
EOF
systemctl start homeassistant.service
systemctl enable homeassistant.service
if [ $WAIT = 1 ]; then
	/usr/local/modules/mydonglecloud/reset/homeassistant-user.sh
else
	/usr/local/modules/mydonglecloud/reset/homeassistant-user.sh &
fi
