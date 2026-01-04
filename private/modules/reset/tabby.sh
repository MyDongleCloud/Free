#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for tabby [-h -r -w]"
echo "h:	Print this usage and exit"
echo "r:	Reset"
echo "w:	Wait for user creation"
exit 0
}

if [ "m`id -u`" = "m0" ]; then
	echo "You should not be root"
#	exit 0
fi

RESET=0
WAIT=0
while getopts hrw opt
do
	case "$opt" in
		h) helper;;
		r) RESET=1;;
		w) WAIT=1;;
	esac
done

if [ $RESET != 1 ]; then
	exit 0
fi

echo "#Reset tabby##################"
TIMEOUT=10
echo "10 seconds to watch for llam.cpp..."
while [ $TIMEOUT -gt 0 ]; do
    sleep 1
    TIMEOUT=$((TIMEOUT - 1))
    [ $TIMEOUT -eq 0 ] && echo "Timeout waiting for llamacpp" && break
	nc -z localhost 8099 2> /dev/null
	if [ $? = 0 ]; then
		break
	fi
done

systemctl stop tabby.service
rm -rf /disk/admin/modules/tabby
rm -rf /disk/admin/.tabby
mkdir /disk/admin/modules/tabby
cat > /disk/admin/modules/tabby/config.toml << EOF
[model.embedding.http]
kind = "llama.cpp/embedding"
api_endpoint = "http://localhost:8099"

[anonymousUsageTracking]
disable = true
EOF
ln -sf /disk/admin/modules/tabby /disk/admin/.tabby
systemctl start tabby.service
systemctl enable tabby.service
if [ $WAIT = 1 ]; then
	/usr/local/modules/mydonglecloud/reset/tabby-user.sh
else
	/usr/local/modules/mydonglecloud/reset/tabby-user.sh &
fi
