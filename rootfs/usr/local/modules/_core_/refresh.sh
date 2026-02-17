#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for refresh [-h]"
echo "h:	Print this usage and exit"
exit 0
}

while getopts h opt
do
	case "$opt" in
		h) helper;;
	esac
done

echo "{\"a\":\"refresh-webserver\"}" | websocat -1 ws://localhost:8094
