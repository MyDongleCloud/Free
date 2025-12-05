#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for postfix [-h -r]"
echo "h:		Print this usage and exit"
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

echo "#Reset postfix##################"
CLOUDNAME=`cat /disk/admin/modules/_config_/_cloud_.json | jq -r ".all.name"`
echo "$CLOUDNAME.mydongle.cloud" > /etc/mailname
sed -i -e "s|^myhostname =.*|myhostname = smtp.$CLOUDNAME.mydongle.cloud|" /etc/postfix/main.cf
