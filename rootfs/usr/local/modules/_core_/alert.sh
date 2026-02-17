#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for alert [-h] type name"
echo "h:	Print this usage and exit"
exit 0
}

while getopts h opt
do
	case "$opt" in
		h) helper;;
	esac
done

echo "{\"a\":\"alert\", \"type\":\"$1\", \"name\":\"$2\"}" | websocat -1 ws://localhost:8094
