#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for setup [-h -u u] name"
echo "h:	Print this usage and exit"
echo "u u:	Launch as user 0:admin, 1:root, -1:read from json (default)"
exit 0
}

if [ "m`id -u`" != "m0" ]; then
	echo "You need to be root"
	exit 0
fi

USER=-1
while getopts hu: opt
do
	case "$opt" in
		h) helper;;
		u) USER=$OPTARG;;
	esac
done

cd `dirname $0`
PP=`pwd`

shift $((OPTIND -1))
NAME=$1
if [ $USER = -1 ]; then
	USER=`jq -r ".\"$NAME\".setupRoot | if . == true then 1 else 0 end" $PP/modulesdefault.json 2> /dev/null`
fi
if [ ! -f $PP/reset/$NAME.sh ]; then
	echo "#Doing nothing for $NAME##################"
else
	if [ $USER = 1 ]; then
		$PP/reset/$NAME.sh
	else
		su admin -c "$PP/reset/$NAME.sh"
	fi
fi
