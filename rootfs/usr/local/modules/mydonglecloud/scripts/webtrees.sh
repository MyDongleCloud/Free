#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for webtrees [-h -r]"
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

echo "#Reset webtrees##################"
DATE=`date +%s`
CLOUDNAME=`cat /disk/admin/modules/_config_/_cloud_.json | jq -r ".all.name"`
DBPASS=$(tr -dc 'A-HJ-NP-Za-km-z1-9' < /dev/urandom | head -c 8)
PASSWD=$(tr -dc 'A-HJ-NP-Za-km-z1-9' < /dev/urandom | head -c 8)

mysql --defaults-file=/disk/admin/modules/mysql/conf.txt << EOF
DROP DATABASE IF EXISTS webtreesDB;
CREATE DATABASE webtreesDB;
DROP USER IF EXISTS 'webtreesUser'@'localhost';
CREATE USER 'webtreesUser'@'localhost' IDENTIFIED BY '${DBPASS}';
GRANT ALL PRIVILEGES ON webtreesDB.* TO 'webtreesUser'@'localhost';
FLUSH PRIVILEGES;
EOF

rm -rf /disk/admin/modules/webtrees
mkdir /disk/admin/modules/webtrees
cp -a /usr/local/modules/webtrees/data.bak /disk/admin/modules/webtrees/data

wtname="${CLOUDNAME}"
wtuser="${CLOUDNAME}"
wtpass="${PASSWD}"
wtemail="admin@${CLOUDNAME}.mydongle.cloud"
prefix="ost_"
dbhost="localhost"
dbname="webtreesDB"
dbuser="webtreesUser"
dbpass="${DBPASS}"

cd /usr/local/modules/webtrees
cat > /tmp/webtrees.php << EOF
<?php
\$_POST['lang'] = 'en-US';
\$_POST['dbtype'] = 'mysql';
\$_POST['dbhost'] = '$dbhost';
\$_POST['dbport'] = '3306';
\$_POST['dbuser'] = '$dbuser';
\$_POST['dbpass'] = '$dbpass';
\$_POST['dbname'] = '$dbname';
\$_POST['tblpfx'] = '$prefix';
\$_POST['baseurl'] = '';
\$_POST['wtname'] = '$wtname';
\$_POST['wtuser'] = '$wtuser';
\$_POST['wtpass'] = '$wtpass';
\$_POST['wtemail'] = '$wtemail';
\$_POST['step'] = '6';

\$_SERVER['REQUEST_METHOD'] = 'POST';
\$_SERVER['REQUEST_URI'] = '/';
\$_SERVER['HTTP_HOST'] = 'localhost';
\$_SERVER['REMOTE_ADDR'] = '127.0.0.1';
\$_SERVER['HTTP_USER_AGENT'] = 'PHP-CLI';
\$_SERVER['CONTENT_TYPE'] = 'application/x-www-form-urlencoded';

include '/usr/local/modules/webtrees/index.php';
?>
EOF

sed -i -e "s/'cli'/'cli2'/" /usr/local/modules/webtrees/app/Webtrees.php
php /tmp/webtrees.php > /tmp/reset-webtrees-$DATE.log 2>&1
sed -i -e "s/'cli2'/'cli'/" /usr/local/modules/webtrees/app/Webtrees.php
rm /tmp/webtrees.php

rm -f /disk/admin/modules/webtrees/conf.txt
echo "{\"email\":\"${wtemail}\", \"username\":\"${wtuser}\", \"password\":\"${wtpass}\", \"dbname\":\"${dbname}\", \"dbuser\":\"${dbuser}\", \"dbpass\":\"${dbpass}\"}" > /disk/admin/modules/_config_/webtrees.json
chown admin:admin /disk/admin/modules/_config_/bugzilla.json

chown -R admin:admin /disk/admin/modules/webtrees
chown -R www-data:admin /disk/admin/modules/webtrees/data

sed -i -e "s|if (str_starts_with(\$content_type, 'text/')) {|if (str_starts_with(\$content_type, 'text/')) {return false;|" /usr/local/modules/webtrees/app/Http/Middleware/CompressResponse.php
