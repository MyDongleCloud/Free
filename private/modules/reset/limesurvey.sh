#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for limesurvey [-h]"
echo "h:	Print this usage and exit"
exit 0
}

if [ "m`id -u`" != "m0" ]; then
	echo "You need to be root"
	exit 0
fi

RESET=0
while getopts h opt
do
	case "$opt" in
		h) helper;;
	esac
done

if [ $RESET != 1 ]; then
	exit 0
fi

echo "#Reset limesurvey##################"
CLOUDNAME=`cat /disk/admin/modules/_config_/_cloud_.json | jq -r ".info.name"`
DBPASS=$(pwgen -B -c -y -n -r "\"\!\'\`\$@~#%^&*()+={[}]|:;<>?/" 12 1)
PASSWD=$(pwgen -B -c -y -n -r "\"\!\'\`\$@~#%^&*()+={[}]|:;<>?/" 12 1)

mysql --defaults-file=/disk/admin/modules/mysql/conf.txt << EOF
DROP DATABASE IF EXISTS limesurveyDB;
CREATE DATABASE limesurveyDB;
DROP USER IF EXISTS 'limesurveyUser'@'localhost';
CREATE USER 'limesurveyUser'@'localhost' IDENTIFIED BY '${DBPASS}';
GRANT ALL PRIVILEGES ON limesurveyDB.* TO 'limesurveyUser'@'localhost';
FLUSH PRIVILEGES;
EOF

rm -rf /disk/admin/modules/limesurvey
mkdir /disk/admin/modules/limesurvey
cp /usr/local/modules/limesurvey/application/config/config.php.bak /disk/admin/modules/limesurvey/
cp /usr/local/modules/limesurvey/application/config/security.php.bak /disk/admin/modules/limesurvey/
cp -a /usr/local/modules/limesurvey/upload.bak /disk/admin/modules/limesurvey/upload

user="${CLOUDNAME}"
pass="${PASSWD}"
email="admin@${CLOUDNAME}.mydongle.cloud"
dbname="limesurveyDB"
dbuser="limesurveyUser"
dbpass="${DBPASS}"

cd /usr/local/modules/limesurvey

echo "{\"email\":\"${email}\", \"username\":\"${user}\", \"password\":\"${pass}\", \"dbname\":\"${dbname}\", \"dbuser\":\"${dbuser}\", \"dbpass\":\"${dbpass}\"}" > /disk/admin/modules/_config_/limesurvey.json
chown admin:admin /disk/admin/modules/_config_/limesurvey.json

chown -R www-data:admin /disk/admin/modules/limesurvey

echo -n "{ \"a\":\"status\", \"module\":\"$(basename $0 .sh)\", \"state\":\"finish\" }" | nc -w 1 localhost 8093