#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for triliumnotes [-h -r]"
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

echo "#Reset triliumnotes##################"
PASSWD=$(pwgen -B -c -y -n -r "\"\!\'\`\$@~#%^&*()+={[}]|:;<>?/" 12 1)
systemctl stop triliumnotes.service
rm -rf /disk/admin/modules/triliumnotes
mkdir /disk/admin/modules/triliumnotes
cat > /disk/admin/modules/triliumnotes/config.ini << EOF
[General]
instanceName=
noAuthentication=false
noBackup=false

[Network]
host=127.0.0.1
port=8090
https=false
certPath=
keyPath=
trustedReverseProxy=false

[Session]
cookieMaxAge=1814400

[Sync]

[MultiFactorAuthentication]
oauthBaseUrl=
oauthClientId=
oauthClientSecret=
oauthIssuerBaseUrl=
oauthIssuerName=
oauthIssuerIcon=
EOF
echo "{\"password\":\"${PASSWD}\"}" > /disk/admin/modules/_config_/triliumnotes.json
systemctl start triliumnotes.service
systemctl enable triliumnotes.service
