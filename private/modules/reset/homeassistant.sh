#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for homeassistant [-h -r]"
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
TIMEOUT=10
echo "10 seconds to watch homeassistant starting..."
while [ $TIMEOUT -gt 0 ]; do
    sleep 1
    TIMEOUT=$((TIMEOUT - 1))
    [ $TIMEOUT -eq 0 ] && echo "Timeout waiting for homeassistant" && break
	nc -z localhost 8123 2> /dev/null
	if [ $? = 0 ]; then
		break
	fi
done
/usr/local/modules/mydonglecloud/reset/homeassistant-user.sh
