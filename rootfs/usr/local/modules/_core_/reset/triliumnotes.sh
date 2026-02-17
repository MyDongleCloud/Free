#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for triliumnotes [-h]"
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

echo {" \"a\":\"status\", \"module\":\"$(basename $0 .sh)\", \"state\":\"finish\" }" | websocat -1 ws://localhost:8094
