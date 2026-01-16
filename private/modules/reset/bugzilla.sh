#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for bugzilla [-h]"
echo "h:	Print this usage and exit"
exit 0
}

if [ "m`id -u`" != "m0" ]; then
	echo "You need to be root"
	exit 0
fi

while getopts h opt
do
	case "$opt" in
		h) helper;;
	esac
done

echo "#Reset bugzilla##################"
DATE=`date +%s`
CLOUDNAME=`cat /disk/admin/modules/_config_/_cloud_.json | jq -r ".info.name"`
PASSWD=$(pwgen -B -c -y -n -r "\"\!\'\`\$@~#%^&*()+={[}]|:;<>?/-" 12 1)

rm -rf /disk/admin/modules/bugzilla
mkdir -p /disk/admin/modules/bugzilla/data

name="Administrator"
email="admin@${CLOUDNAME}.mydongle.cloud"
username="${CLOUDNAME}"
passwd="${PASSWD}"

cd /usr/local/modules/bugzilla
./checksetup.pl
expect -c "
set timeout -1
spawn ./checksetup.pl
expect \"Enter the e-mail address of the administrator:\"
send \"$email\n\"
expect \"Enter the login name the administrator will log in with:\"
send \"$username\n\"
expect \"Enter the real name of the administrator:\"
send \"$name\n\"
expect \"Enter a password for the administrator account:\"
send \"$passwd\n\"
expect \"Please retype the password to verify:\"
send \"$passwd\n\"
expect eof
" > /tmp/reset-bugzilla-$DATE.log 2>&1
./checksetup.pl >> /tmp/reset-bugzilla-$DATE.log 2>&1

rm -f /disk/admin/modules/bugzilla/conf.txt
echo "{\"email\":\"${email}\", \"username\":\"${username}\", \"password\":\"${passwd}\"}" > /disk/admin/modules/_config_/bugzilla.json
chown admin:admin /disk/admin/modules/_config_/bugzilla.json

chown -R admin:admin /disk/admin/modules/bugzilla
chown -R www-data:admin /disk/admin/modules/bugzilla/data
chown -R www-data:admin /disk/admin/modules/bugzilla/localconfig

echo -n "{ \"a\":\"status\", \"module\":\"$(basename $0 .sh)\", \"state\":\"finish\" }" | nc -w 1 localhost 8093
