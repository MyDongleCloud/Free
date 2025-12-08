#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for tabby [-h -r]"
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

echo "#Reset tabby##################"
systemctl stop tabby.service
rm -rf /disk/admin/modules/tabby
mkdir /disk/admin/modules/tabby
cat > /disk/admin/modules/tabby/config.toml << EOF
[anonymousUsageTracking]
disable = true
EOF
ln -sf /disk/admin/modules/tabby /disk/admin/.tabby
systemctl start tabby.service
systemctl enable tabby.service
