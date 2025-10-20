#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for roundcube [-h -r]"
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

echo "#Reset roundcube##################"
DATE=`date +%s`
SPACENAME=`cat /disk/admin/.modules/mydonglecloud/space.json | jq -r ".name"`
EMAIL="admin@$SPACENAME.mydongle.cloud"
PASSWD=$(tr -dc 'A-HJ-NP-Za-km-z1-9' < /dev/urandom | head -c 8)
PWD=`doveadm pw -s SHA512-CRYPT -p "$PASSWD"`

sed -e "s|\$config['smtp_host'].*|\$config['smtp_host'] = 'ssl://localhost:465'; \$config['smtp_conn_options'] = [ 'ssl' => [ 'verify_peer' => false, 'verify_peer_name' => false ] ];|" /etc/roundcube/config.inc.php.template > /etc/roundcube/config.inc.php
rm -rf /disk/admin/.modules/mail/$SPACENAME.mydongle.cloud
mkdir -p /disk/admin/.modules/mail/$SPACENAME.mydongle.cloud/admin
echo "$EMAIL $SPACENAME.mydongle.cloud/admin/" > /disk/admin/.modules/mail/virtualmaps
postmap /disk/admin/.modules/mail/virtualmaps
echo "" > /disk/admin/.modules/mail/virtualalias
postmap /disk/admin/.modules/mail/virtualalias
echo "$EMAIL:{SHA512-CRYPT}$PWD" > /disk/admin/.modules/mail/password-$SPACENAME.mydongle.cloud

rm -f /disk/admin/.modules/roundcube/conf.txt
echo "User: ${EMAIL}\nPassword: ${PASSWD}" > /disk/admin/.modules/roundcube/conf.txt
chmod 444 /disk/admin/.modules/roundcube/conf.txt
