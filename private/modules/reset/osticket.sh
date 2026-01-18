#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for osticket [-h]"
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

echo "#Reset osticket##################"
DATE=`date +%s`
CLOUDNAME=`cat /disk/admin/modules/_config_/_cloud_.json | jq -r ".info.name"`
SALT=$(tr -dc 'a-f0-9' < /dev/urandom | head -c 32)
DBPASS=$(pwgen -B -c -y -n -r "\"\!\'\`\$@~#%^&*()+={[}]|:;<>?/" 12 1)
PASSWD=$(pwgen -B -c -y -n -r "\"\!\'\`\$@~#%^&*()+={[}]|:;<>?/" 12 1)

mysql --defaults-file=/disk/admin/modules/mysql/conf.txt << EOF
DROP DATABASE IF EXISTS osticketDB;
CREATE DATABASE osticketDB;
DROP USER IF EXISTS 'osticketUser'@'localhost';
CREATE USER 'osticketUser'@'localhost' IDENTIFIED BY '${DBPASS}';
GRANT ALL PRIVILEGES ON osticketDB.* TO 'osticketUser'@'localhost';
FLUSH PRIVILEGES;
EOF

rm -rf /disk/admin/modules/osticket
mkdir -p /disk/admin/modules/osticket
cp /usr/local/modules/osticket/include/ost-sampleconfig.php /disk/admin/modules/osticket/ost-config.php
chmod 666 /disk/admin/modules/osticket/ost-config.php
sed -i -e "s|^define.*SECRET_SALT.*|define('SECRET_SALT','${SALT}');|" /disk/admin/modules/osticket/ost-config.php

s="install"
name="Support Center"
email="support@${CLOUDNAME}.mydongle.cloud"
fname="First Name"
lname="Last Name"
admin_email="admin@${CLOUDNAME}.mydongle.cloud"
username="${CLOUDNAME}"
passwd="${PASSWD}"
prefix="ost_"
dbhost="localhost"
dbname="osticketDB"
dbuser="osticketUser"
dbpass="${DBPASS}"
timezone="America/Los_Angeles"

cd /usr/local/modules/osticket/include
cat > /tmp/osticket.php << EOF
<?php
\$_POST['s'] = '$s';
\$_POST['name'] = '$name';
\$_POST['email'] = '$email';
\$_POST['fname'] = '$fname';
\$_POST['lname'] = '$lname';
\$_POST['admin_email'] = '$admin_email';
\$_POST['username'] = '$username';
\$_POST['passwd'] = '$passwd';
\$_POST['passwd2'] = '$passwd';
\$_POST['prefix'] = '$prefix';
\$_POST['dbhost'] = '$dbhost';
\$_POST['dbname'] = '$dbname';
\$_POST['dbuser'] = '$dbuser';
\$_POST['dbpass'] = '$dbpass';
\$_POST['timezone'] = '$timezone';

\$_SERVER['REQUEST_METHOD'] = 'POST';

include '/usr/local/modules/osticket/setup/install.php';
?>
EOF
php /tmp/osticket.php > /tmp/reset-osticket-$DATE.log 2>&1
rm /tmp/osticket.php

chmod 644 /disk/admin/modules/osticket/ost-config.php

echo "{\"other\":\"name: ${name}, email: ${email}\", \"email\":\"${admin_email}\", \"username\":\"${username}\", \"password\":\"${passwd}\", \"dbname\":\"${dbname}\", \"dbuser\":\"${dbuser}\", \"dbpass\":\"${dbpass}\"}" > /disk/admin/modules/_config_/osticket.json

echo {" \"a\":\"status\", \"module\":\"$(basename $0 .sh)\", \"state\":\"finish\" }" | websocat -1 ws://localhost:8094
