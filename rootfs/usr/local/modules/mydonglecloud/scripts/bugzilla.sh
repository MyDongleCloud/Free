#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for bugzilla [-h -r]"
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

echo "#Reset bugzilla##################"
DATE=`date +%s`
SPACENAME=`cat /disk/admin/.modules/mydonglecloud/space.json | jq -r ".name"`
PASSWD=$(tr -dc 'A-HJ-NP-Za-km-z1-9' < /dev/urandom | head -c 8)

rm -rf /disk/admin/.modules/bugzilla
mkdir -p /disk/admin/.modules/bugzilla/data

name="Administrator"
email="admin@${SPACENAME}.mydongle.cloud"
username="${SPACENAME}"
passwd="${PASSWD}"

cd /usr/local/modules/bugzilla
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

rm -f /disk/admin/.modules/bugzilla/conf.txt
echo "Email: ${email}\nUser: ${username}\nPassword: ${passwd}" > /disk/admin/.modules/bugzilla/conf.txt
chmod 444 /disk/admin/.modules/bugzilla/conf.txt

chown -R admin:admin /disk/admin/.modules/bugzilla
chown -R www-data:admin /disk/admin/.modules/bugzilla/data
