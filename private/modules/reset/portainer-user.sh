#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for portainer-user [-h]"
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

echo "#Create user portainer##################"
TIMEOUT=30
echo "$TIMEOUT seconds to watch portainer starting..."
while [ $TIMEOUT -gt 0 ]; do
    sleep 1
    TIMEOUT=$((TIMEOUT - 1))
    [ $TIMEOUT -eq 0 ] && echo "Timeout waiting for jellyfin" && exit
	nc -z localhost 9000 2> /dev/null
	if [ $? = 0 ]; then
		break
	fi
done

echo -n "Waiting 10 seconds... "
sleep 10
echo "done"

URL="http://localhost:9000"
CLOUDNAME=`cat /disk/admin/modules/_config_/_cloud_.json | jq -r ".info.name"`
PASSWD=$(pwgen -B -c -y -n -r "\"\!\'\`\$@~#%^&*()+={[}]|:;<>?/" 12 1)
data="{\"Username\":\"${CLOUDNAME}\", \"Password\":\"${PASSWD}\"}"
response=`curl -X POST "${URL}/api/users/admin/init" -H "Content-Type: application/json" -d "$data"`

echo "{\"username\":\"${CLOUDNAME}\", \"password\":\"${PASSWD}\"}" > /disk/admin/modules/_config_/portainer.json
