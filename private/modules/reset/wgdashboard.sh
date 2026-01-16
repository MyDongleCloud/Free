#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for webtrees [-h]"
echo "h:	Print this usage and exit"
exit 0
}

if [ "m`id -u`" != "m0" ]; then
	echo "You need to be root"
	exit 0
fi

while getopts h opt
do
	case "$opt" in
		h) helper;;
	esac
done

echo "#Reset wgdashboard##################"
systemctl stop wgdashboard.service
rm -rf /disk/admin/modules/wgdashboard
mkdir /disk/admin/modules/wgdashboard
mkdir /disk/admin/modules/wgdashboard/db
mkdir /disk/admin/modules/wgdashboard/attachments
CLOUDNAME=`cat /disk/admin/modules/_config_/_cloud_.json | jq -r ".info.name"`
PASSWD=$(pwgen -B -c -y -n -r "\"\!\'\`\$@~#%^&*()+={[}]|:;<>?/" 12 1)
EXTERNALIP=`wget https://mydongle.cloud/master/ip.json -O- | jq -r .ip`
KEY=$(tr -dc 'A-Z0-9' < /dev/urandom | head -c 32)
HASH=$(python3 -c "import bcrypt; print(bcrypt.hashpw('$PASSWD'.encode('utf-8'), bcrypt.gensalt()).decode('utf-8'))")
ESCAPEDHASH=$(echo "$HASH" | sed 's/\//\\\//g')
cp /usr/local/modules/wgdashboard/src/wg-dashboard.ini.bak /usr/local/modules/wgdashboard/src/wg-dashboard.ini
sed -i "s/^username = user.*/username = $CLOUDNAME/" /usr/local/modules/wgdashboard/src/wg-dashboard.ini
sed -i "s/^password = pass.*/password = $ESCAPEDHASH/" /usr/local/modules/wgdashboard/src/wg-dashboard.ini
sed -i "s/^totp_key =.*/totp_key = $KEY/" /usr/local/modules/wgdashboard/src/wg-dashboard.ini
sed -i "s/^remote_endpoint =.*/remote_endpoint = $EXTERNALIP/" /usr/local/modules/wgdashboard/src/wg-dashboard.ini
systemctl start wgdashboard.service
systemctl enable wgdashboard.service
echo "{\"username\":\"${CLOUDNAME}\", \"password\":\"${PASSWD}\"}" > /disk/admin/modules/_config_/wgdashboard.json
chown admin:admin /disk/admin/modules/_config_/wgdashboard.json

echo -n "{ \"a\":\"status\", \"module\":\"$(basename $0 .sh)\", \"state\":\"finish\" }" | nc -w 1 localhost 8093
