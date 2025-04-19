#!/bin/sh

if [ "m`id -u`" != "m0" ]; then
	echo "You need to be root"
	exit 0
fi

/etc/mydonglecloud-otg.sh
