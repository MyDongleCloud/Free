#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for pwd [-h]"
echo "h:	Print this usage and exit"
exit 0
}

while getopts h opt; do
	case "$opt" in
		h) helper;;
	esac
done

if [ "m`id -u`" != "m0" ]; then
	echo "You need to be root"
	exit 0
fi

read inp
sed -i -e "s|^admin:[^:]*\(:.*\)|admin:${inp}\1|" /etc/shadow
