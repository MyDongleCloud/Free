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
CLOUDNAME=`cat /disk/admin/modules/_config_/_cloud_.json | jq -r ".info.name"`
SHORTNAME=`cat /disk/admin/modules/_config_/_cloud_.json | jq -r ".info.shortname"`
DOMAINS=`cat /disk/admin/modules/_config_/_cloud_.json | jq -r ".info.domains[]"`
EMAIL="admin@$CLOUDNAME.mydongle.cloud"
PASSWD=$(tr -dc 'A-HJ-NP-Za-km-z1-9' < /dev/urandom | head -c 8)
PWD=`doveadm pw -s SHA512-CRYPT -p "$PASSWD"`


sed -e "s|^\$config\\['smtp_host'\\].*|\$config['smtp_host'] = 'ssl://localhost:465'; \$config['smtp_conn_options'] = [ 'ssl' => [ 'verify_peer' => false, 'verify_peer_name' => false ] ];|" /etc/roundcube/config.inc.php.template > /etc/roundcube/config.inc.php
rm -rf /disk/admin/modules/mail
mkdir -p /disk/admin/modules/mail/$CLOUDNAME.mydongle.cloud/admin
echo "$EMAIL $CLOUDNAME.mydongle.cloud/admin/" > /disk/admin/modules/mail/virtualmaps
postmap /disk/admin/modules/mail/virtualmaps
echo "" > /disk/admin/modules/mail/virtualalias
postmap /disk/admin/modules/mail/virtualalias
cat > /disk/admin/modules/mail/virtualhosts <<EOF
$CLOUDNAME.mydongle.cloud
$CLOUDNAME.mondongle.cloud
$SHORTNAME.myd.cd
$DOMAINS
EOF
PASSWD2=`cat /disk/admin/modules/_config_/_cloud_.json | jq -r ".postfix.token"`
echo "[server.mydongle.cloud]:466 $EMAIL:$PASSWD2" > /disk/admin/modules/mail/relay
postmap /disk/admin/modules/mail/relay
echo "$EMAIL:$PWD" > /disk/admin/modules/mail/password-$CLOUDNAME.mydongle.cloud
echo "{\"email\":\"${EMAIL}\", \"username\":\"${EMAIL}\", \"password\":\"${PASSWD}\"}" > /disk/admin/modules/_config_/roundcube.json
ln -sf roundcube.json /disk/admin/modules/_config_/postfix.json
systemctl restart postfix.service
systemctl restart dovecot.service
