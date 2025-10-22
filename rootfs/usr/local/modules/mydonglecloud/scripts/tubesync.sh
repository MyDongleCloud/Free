#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for tubesync [-h -r]"
echo "h:	Print this usage and exit"
echo "r:	Reset"
exit 0
}

if [ "m`id -u`" = "m0" ]; then
	echo "You should not be root"
#	exit 0
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

echo "#Reset tubesync##################"
DATE=`date +%s`

rm -rf /disk/admin/.modules/tubesync
mkdir /disk/admin/.modules/tubesync
cp /usr/local/modules/tubesync/tubesync/db.sqlite3 /disk/admin/.modules/tubesync/
mkdir /disk/admin/.modules/tubesync/downloads
mkdir /disk/admin/.modules/tubesync/downloads/audio
mkdir /disk/admin/.modules/tubesync/downloads/video
mkdir /disk/admin/.modules/tubesync/tasks
