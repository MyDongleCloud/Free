#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for setup [-h -r]"
echo "h:		Print this usage and exit"
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

SPACENAME=`jq -r ".name" /disk/admin/.modules/mydonglecloud/space.json`

if [ $SPACENAME != "null" -a $SPACENAME != "" ]; then
	echo "#Setup ##################"
	/usr/local/modules/mydonglecloud/scripts/postfix.sh -r $SPACENAME
	/usr/local/modules/mydonglecloud/scripts/mysql.sh -r
	su admin -c "/usr/local/modules/mydonglecloud/scripts/osticket.sh -r"
	su admin -c "/usr/local/modules/mydonglecloud/scripts/roundcube.sh -r $SPACENAME"
	/usr/local/modules/mydonglecloud/scripts/jitsi.sh -r $SPACENAME
fi
