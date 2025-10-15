#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for otp.sh [-h]"
echo "h:		Print this usage and exit"
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

SECRET=`head -10 /dev/urandom | sha512sum | cut -b 19-50`
OATH=`oathtool $SECRET`
echo -n "HOTP admin - $SECRET" > /disk/admin/.modules/pam/oath.txt
chmod 664 /disk/admin/.modules/pam/oath.txt
chown admin:admin /disk/admin/.modules/pam/oath.txt
echo "OTP: $OATH"
