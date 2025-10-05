#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for postfix [-h] domain"
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

echo "$1" > /etc/mailname
sed -i -e "s|^myhostname =.*|myhostname = smtp.$1|" /etc/postfix/main.cf
