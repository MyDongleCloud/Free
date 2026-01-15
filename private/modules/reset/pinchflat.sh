#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for pinchflat [-h -r]"
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

echo "#Reset pinchflat##################"
SECRET_KEY_BASE=$(tr -dc 'a-f0-9' < /dev/urandom | head -c 64)
systemctl stop pinchflat.service
rm -rf /disk/admin/modules/pinchflat
mkdir /disk/admin/modules/pinchflat/
mkdir /disk/admin/modules/pinchflat/downloads
mkdir /disk/admin/modules/pinchflat/config
echo "SECRET_KEY_BASE=${SECRET_KEY_BASE}" > /disk/admin/modules/pinchflat/config/env
export SECRET_KEY_BASE=${SECRET_KEY_BASE}
export CONFIG_PATH=/disk/admin/modules/pinchflat/config
cd /usr/local/modules/pinchflat/_build/prod/rel/pinchflat
/usr/local/modules/pinchflat/_build/prod/rel/pinchflat/bin/pinchflat eval "Pinchflat.Release.migrate"
systemctl start pinchflat.service
systemctl enable pinchflat.service
