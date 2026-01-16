#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for llamacpp [-h]"
echo "h:	Print this usage and exit"
exit 0
}

if [ "m`id -u`" = "m0" ]; then
	echo "You should not be root"
#	exit 0
fi

while getopts h opt
do
	case "$opt" in
		h) helper;;
	esac
done

echo "#Reset llamacpp##################"
systemctl stop llamacpp.service
systemctl start llamacpp.service
systemctl enable llamacpp.service

echo -n "{ \"a\":\"status\", \"module\":\"$(basename $0 .sh)\", \"state\":\"finish\" }" | nc -w 1 localhost 8093
