#!/bin/sh

if [ "m`id -u`" != "m0" ]; then
	echo "You need to be root"
	exit 0
fi

USER=$1
EMAIL=$2
echo -n "$USER:$EMAIL" > /etc/mydonglecloud-spaces.name
#Create user $USER
