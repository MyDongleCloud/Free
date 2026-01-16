#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for lobechat [-h -r]"
echo "h:	Print this usage and exit"
echo "r:	Reset"
exit 0
}

if [ "m`id -u`" != "m0" ]; then
	echo "You need to be root"
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

echo "#Reset lobechat##################"
systemctl stop lobechat.service
CLOUDNAME=`cat /disk/admin/modules/_config_/_cloud_.json | jq -r ".info.name"`
KEY_VAULTS_SECRET=$(tr -dc 'a-f0-9' < /dev/urandom | head -c 32)
dbpass=$(pwgen -B -c -y -n -r "\"\!\'\`\$@~#%^&*()+={[}]|:;<>?/" 12 1)

export PGPASSWORD=`jq -r .password /disk/admin/modules/_config_/postgresql.json`
su postgres -c "psql" << EOF
DROP DATABASE IF EXISTS lobechatdb;
CREATE DATABASE lobechatdb;
DROP USER IF EXISTS lobechatuser;
CREATE USER lobechatuser WITH ENCRYPTED PASSWORD '${dbpass}';
GRANT ALL PRIVILEGES ON DATABASE lobechatdb TO lobechatuser;
\c lobechatdb
CREATE EXTENSION vector;
\dx
\q
EOF
unset PGPASSWORD

rm -rf /disk/admin/modules/lobechat
mkdir /disk/admin/modules/lobechat
cat > /disk/admin/modules/lobechat/.env << EOF
REDIS_URL=redis://localhost:6379
REDIS_PREFIX=lobechat

SMTP_HOST=${CLOUDNAME}.mydongle.cloud
SMTP_PORT=465
SMTP_SECURE=false
SMTP_USER=admin@${CLOUDNAME}.mydongle.cloud
SMTP_PASS=demodemo

DATABASE_URL=postgres://lobechatuser:${dbpass}@127.0.0.1:5432/lobechatdb

KEY_VAULTS_SECRET=${KEY_VAULTS_SECRET}
EOF

echo "{\"dbname\":\"lobechatdb\", \"dbuser\":\"lobechatuser\", \"dbpass\":\"${dbpass}\"}" > /disk/admin/modules/_config_/lobechat.json
chown admin:admin /disk/admin/modules/_config_/lobechat.json

chown -R admin:admin /disk/admin/modules/lobechat
systemctl start lobechat.service
systemctl enable lobechat.service

echo -n "{ \"a\":\"status\", \"module\":\"$(basename $0 .sh)\", \"state\":\"finish\" }" | nc -w 1 localhost 8093