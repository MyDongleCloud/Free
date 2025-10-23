#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for yourls [-h -r]"
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

echo "#Reset yourls##################"
DATE=`date +%s`
SPACENAME=`cat /disk/admin/.modules/mydonglecloud/space.json | jq -r ".name"`
SALT=$(tr -dc 'A-HJ-NP-Za-km-z1-9' < /dev/urandom | head -c 32)
DBPASS=$(tr -dc 'A-HJ-NP-Za-km-z1-9' < /dev/urandom | head -c 8)
PASSWD=$(tr -dc 'A-HJ-NP-Za-km-z1-9' < /dev/urandom | head -c 8)

mysql --defaults-file=/disk/admin/.modules/mysql/conf.txt << EOF
DROP DATABASE IF EXISTS yourlsDB;
CREATE DATABASE yourlsDB;
DROP USER IF EXISTS 'yourlsUser'@'localhost';
CREATE USER 'yourlsUser'@'localhost' IDENTIFIED BY '${DBPASS}';
GRANT ALL PRIVILEGES ON yourlsDB.* TO 'yourlsUser'@'localhost';
FLUSH PRIVILEGES;
EOF

rm -rf /disk/admin/.modules/yourls
mkdir /disk/admin/.modules/yourls
cp /usr/local/modules/yourls/user/config-sample.php /disk/admin/.modules/yourls/config.php

dbuser="yourlsUser"
dbpass="${DBPASS}"
dbname="yourlsDB"
site="https://yourls.${SPACENAME}.mydongle.cloud"
username="${SPACENAME}"
passwd="${PASSWD}"
saltpass=$(tr -dc '1-9' < /dev/urandom | head -c 5)
md5=`echo -n "$saltpass$passwd" | md5sum | cut -d ' ' -f 1`
passwdMd5="md5:$saltpass:$md5"

sed -i -e "s|define( 'YOURLS_DB_USER',.*|define( 'YOURLS_DB_USER', '$dbuser' );|" /disk/admin/.modules/yourls/config.php
sed -i -e "s|define( 'YOURLS_DB_PASS',.*|define( 'YOURLS_DB_PASS', '$dbpass' );|" /disk/admin/.modules/yourls/config.php
sed -i -e "s|define( 'YOURLS_DB_NAME',.*|define( 'YOURLS_DB_NAME', '$dbname' );|" /disk/admin/.modules/yourls/config.php
sed -i -e "s|define( 'YOURLS_SITE',.*|define( 'YOURLS_SITE', '$site' );|" /disk/admin/.modules/yourls/config.php
sed -i -e "s|define( 'YOURLS_COOKIEKEY',.*|define( 'YOURLS_COOKIEKEY', '$SALT' );|" /disk/admin/.modules/yourls/config.php
sed -i -e "s|	'username' => 'password',.*|	'$username' => '$passwdMd5',|" /disk/admin/.modules/yourls/config.php

cd /usr/local/modules/yourls
cat > /tmp/yourls.php << EOF
<?php
\$_SERVER['REQUEST_METHOD'] = 'POST';
\$_SERVER['HTTP_USER_AGENT'] = 'PHP-CLI';
\$_SERVER['CONTENT_TYPE'] = 'application/x-www-form-urlencoded';
\$_POST['install'] = '';
include '/usr/local/modules/yourls/admin/install.php';
?>
EOF
php /tmp/yourls.php > /tmp/reset-yourls-$DATE.log 2>&1
rm /tmp/yourls.php

rm -f /disk/admin/.modules/yourls/conf.txt
echo "{\"user\":\"${username}\", \"password\":\"${passwd}\", \"dbname\":\"${dbname}\", \"dbuser\":\"${dbuser}\", \"dbpass\":\"${dbpass}\"}" > /disk/admin/.modules/_config_/yourls.json
