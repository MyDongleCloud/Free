#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for jitsimeet [-h]"
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

echo "#Reset jitsimeet##################"

echo {" \"a\":\"status\", \"module\":\"$(basename $0 .sh)\", \"state\":\"finish\" }" | websocat -1 ws://localhost:8094
