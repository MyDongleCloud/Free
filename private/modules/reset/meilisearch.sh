#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for meilisearch [-h -r]"
echo "h:	Print this usage and exit"
echo "r:	Reset"
exit 0
}

if [ "m`id -u`" = "m0" ]; then
	echo "You should not be root"
	exit 0
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

echo "#Reset meilisearch##################"
PASSWORD=$(pwgen -B -c -y -n -r "\"\!\'\`\$@~#%^&*()+={[}]|:;<>?/" 12 1)

systemctl stop meilisearch.service
rm -rf /disk/admin/modules/meilisearch
mkdir -p /disk/admin/modules/meilisearch
echo "MEILI_HOST=127.0.0.1\nMEILI_MASTER_KEY=$PASSWORD" > /disk/admin/modules/meilisearch/meilisearch.env
echo "{\"key\":\"${PASSWORD}\"}" > /disk/admin/modules/_config_/meilisearch.json
systemctl start meilisearch.service
systemctl enable meilisearch.service
