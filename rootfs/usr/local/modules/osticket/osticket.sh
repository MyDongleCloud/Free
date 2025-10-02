#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for osticket [-h -r]"
echo "h:	Print this usage and exit"
echo "r:	Reset osticket"
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

SPACENAME=`cat /disk/admin/.modules/mydonglecloud/space.json | jq -r ".name"`
SALT=$(tr -dc 'A-HJ-NP-Za-km-z1-9' < /dev/urandom | head -c 32)
DBPASS=$(tr -dc 'A-HJ-NP-Za-km-z1-9' < /dev/urandom | head -c 16)
USERNAME="admin2"
PASSWD=$(tr -dc 'A-HJ-NP-Za-km-z1-9' < /dev/urandom | head -c 8)

mysql --defaults-file=/disk/admin/.modules/mysql/my.cnf << EOF
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
curl -s -o /tmp/osticket-install.txt -X POST \
--data-urlencode "s=$s" \
--data-urlencode "name=$name" \
--data-urlencode "email=$email" \
--data-urlencode "fname=$fname" \
--data-urlencode "lname=$lname" \
--data-urlencode "admin_email=$admin_email" \
--data-urlencode "username=$username" \
--data-urlencode "passwd=$passwd" \
--data-urlencode "passwd2=$passwd" \
--data-urlencode "prefix=$prefix" \
--data-urlencode "dbhost=$dbhost" \
--data-urlencode "dbname=$dbname" \
--data-urlencode "dbuser=$dbuser" \
--data-urlencode "dbpass=$dbpass" \
--data-urlencode "timezone=$timezone" \
http://localhost:9236/setup/install.php

chmod 644 /disk/admin/.modules/osticket/ost-config.php

echo "Ticket name: ${name}\nTicket email: ${email}\n\nAdmin email: ${admin_email}\nUsername: ${username}\nPassword: ${passwd}\n\nDB name: ${dbname}\nDB user: ${dbuser}\nDB password: ${dbpass}\n" > /disk/admin/.modules/osticket/conf.txt
chown admin:admin /disk/admin/.modules/osticket/conf.txt
chmod 444 /disk/admin/.modules/osticket/conf.txt
