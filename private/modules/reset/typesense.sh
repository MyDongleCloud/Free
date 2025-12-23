#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for typesense [-h -r]"
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

echo "#Reset typesense##################"
SALT=$(tr -dc 'a-f0-9' < /dev/urandom | head -c 32)
systemctl stop typesense-server.service
rm -rf /disk/admin/modules/typesense
mkdir -p /disk/admin/modules/typesense/data
cat > /disk/admin/modules/typesense/typesense-server.ini << EOF
[server]
api-address = 127.0.0.1
api-port = 8108
data-dir = /disk/admin/modules/typesense/data
api-key = ${SALT}
log-dir = /var/log/typesense
enable-cors = true
# cors-domains = https://yourdomain.com,https://anotherdomain.com
EOF
echo "{ \"apikey\":\"${SALT}\" }" > /disk/admin/modules/_config_/typesense.json

mkdir /disk/admin/modules/typesensedashboard
cat > /disk/admin/modules/typesensedashboard/config.json << EOF
{
	"apiKey": "${SALT}",
	"node": {
		"host": "AUTO",
		"path": "/api"
	}
}
EOF
systemctl start typesense-server.service
systemctl enable typesense-server.service
