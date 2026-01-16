#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for pihole [-h]"
echo "h:	Print this usage and exit"
exit 0
}

if [ "m`id -u`" = "m0" ]; then
	echo "You need to be root"
	exit 0
fi

while getopts h opt
do
	case "$opt" in
		h) helper;;
	esac
done

echo "#Reset pihole##################"
systemctl stop pihole-FTL.service
cp /usr/local/modules/pihole/pihole.toml /etc/pihole/pihole.toml
systemctl start pihole-FTL.service
systemctl enable pihole-FTL.service

echo -n "{ \"a\":\"status\", \"module\":\"$(basename $0 .sh)\", \"state\":\"finish\" }" | nc -w 1 localhost 8093
