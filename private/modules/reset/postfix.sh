#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for postfix [-h]"
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

echo "#Reset postfix##################"
CLOUDNAME=`cat /disk/admin/modules/_config_/_cloud_.json | jq -r ".info.name"`
echo "$CLOUDNAME.mydongle.cloud" > /etc/mailname
sed -i -e "s|^myhostname =.*|myhostname = smtp.$CLOUDNAME.mydongle.cloud|" /etc/postfix/main.cf

echo -n "{ \"a\":\"status\", \"module\":\"$(basename $0 .sh)\", \"state\":\"finish\" }" | nc -w 1 localhost 8093
