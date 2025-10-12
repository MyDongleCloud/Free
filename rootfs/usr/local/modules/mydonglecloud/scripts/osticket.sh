#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for osticket [-h -r]"
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

echo "#Reset osticket##################"
SPACENAME=`cat /disk/admin/.modules/mydonglecloud/space.json | jq -r ".name"`
SALT=$(tr -dc 'A-HJ-NP-Za-km-z1-9' < /dev/urandom | head -c 32)
DBPASS=$(tr -dc 'A-HJ-NP-Za-km-z1-9' < /dev/urandom | head -c 16)
USERNAME="admin2"
PASSWD=$(tr -dc 'A-HJ-NP-Za-km-z1-9' < /dev/urandom | head -c 8)

mysql --defaults-file=/disk/admin/.modules/mysql/conf.txt << EOF
DROP DATABASE IF EXISTS osticketDB;
CREATE DATABASE osticketDB;
DROP USER IF EXISTS 'osticketUser'@'localhost';
CREATE USER 'osticketUser'@'localhost' IDENTIFIED BY '${DBPASS}';
GRANT ALL PRIVILEGES ON osticketDB.* TO 'osticketUser'@'localhost';
FLUSH PRIVILEGES;
EOF

cp /usr/local/modules/osticket/include/ost-sampleconfig.php /disk/admin/.modules/osticket/ost-config.php
chmod 666 /disk/admin/.modules/osticket/ost-config.php
sed -i -e "s|^define.*SECRET_SALT.*|define('SECRET_SALT','${SALT}');|" /disk/admin/.modules/osticket/ost-config.php

s="install"
name="Support Center"
email="support@${SPACENAME}.mydongle.cloud"
fname="First Name"
lname="Last Name"
admin_email="admin@${SPACENAME}.mydongle.cloud"
username="${SPACENAME}";
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
php /tmp/osticket.php > /tmp/osticket.log

chmod 644 /disk/admin/.modules/osticket/ost-config.php

rm -f /disk/admin/.modules/osticket/conf.txt
echo "Ticket name: ${name}\nTicket email: ${email}\n\nAdmin email: ${admin_email}\nUsername: ${username}\nPassword: ${passwd}\n\nDB name: ${dbname}\nDB user: ${dbuser}\nDB password: ${dbpass}\n" > /disk/admin/.modules/osticket/conf.txt
chmod 444 /disk/admin/.modules/osticket/conf.txt
