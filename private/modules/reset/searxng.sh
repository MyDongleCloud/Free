#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for searxng [-h]"
echo "h:	Print this usage and exit"
exit 0
}

if [ "m`id -u`" = "m0" ]; then
	echo "You should not be root"
	exit 0
fi

while getopts h opt
do
	case "$opt" in
		h) helper;;
	esac
done

echo "#Reset searxng##################"
#CLOUDNAME=`cat /disk/admin/modules/_config_/_cloud_.json | jq -r ".info.name"`
SECRETKEY=$(tr -dc 'a-f0-9' < /dev/urandom | head -c 32)
systemctl stop searxng.service
rm -rf /disk/admin/modules/searxng
mkdir /disk/admin/modules/searxng
cp /usr/local/modules/searxng/utils/templates/etc/searxng/settings.yml /disk/admin/modules/searxng/
sed -i -e "s/^  secret_key:.*/  secret_key: \"${SECRETKEY}\"\n  bind_address: "127.0.0.1"\n  port: 8111/" /disk/admin/modules/searxng/settings.yml
#sed -i -e "s@^  # base_url:.*@  base_url: https://search.${CLOUDNAME}.mydongle.cloud@" /disk/admin/modules/searxng/
systemctl start searxng.service
systemctl enable searxng.service

echo {" \"a\":\"status\", \"module\":\"$(basename $0 .sh)\", \"state\":\"finish\" }" | websocat -1 ws://localhost:8094
