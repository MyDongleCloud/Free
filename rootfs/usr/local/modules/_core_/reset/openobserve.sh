#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for openobserve [-h]"
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

echo "#Reset openobserve##################"
CLOUDNAME=`cat /disk/admin/modules/_config_/_cloud_.json | jq -r ".info.name"`
EMAIL="admin@$CLOUDNAME.mydongle.cloud"
PASSWD=$(pwgen -B -c -y -n -r "\"\!\'\`\$@~#%^&*()+={[}]|:;<>?/" 12 1)
systemctl stop openobserve.service
rm -rf /disk/admin/modules/openobserve
mkdir -p /disk/admin/modules/openobserve/data
cat > /disk/admin/modules/openobserve/env << EOF
ZO_ROOT_USER_EMAIL=$EMAIL
ZO_ROOT_USER_PASSWORD=$PASSWD
EOF
systemctl start openobserve.service
systemctl enable openobserve.service

echo "{\"email\":\"${EMAIL}\", \"password\":\"${PASSWD}\"}" > /disk/admin/modules/_config_/openobserve.json

echo {" \"a\":\"status\", \"module\":\"$(basename $0 .sh)\", \"state\":\"finish\" }" | websocat -1 ws://localhost:8094
