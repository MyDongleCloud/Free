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
PORT=9000
URL="http://localhost:$PORT"
TIMEOUT=40
while [ $TIMEOUT -gt 0 ]; do
    sleep 1
    TIMEOUT=$((TIMEOUT - 1))
    [ $TIMEOUT -eq 0 ] && echo "Timeout port waiting for portainer" && exit
	nc -z localhost $PORT 2> /dev/null
	if [ $? = 0 ]; then
		break
	fi
done
TIMEOUT=40
while [ $TIMEOUT -gt 0 ]; do
    sleep 1
    TIMEOUT=$((TIMEOUT - 1))
    [ $TIMEOUT -eq 0 ] && echo "Timeout api waiting for portainer" && exit
	STATUS=`curl -s -o /dev/null -w "%{http_code}" "$URL/api/system/status"`
	if [ $STATUS = 200 ]; then
		break
	fi
done
echo "Doing portainer user"

CLOUDNAME=`cat /disk/admin/modules/_config_/_cloud_.json | jq -r ".info.name"`
PASSWD=$(pwgen -B -c -y -n -r "\"\!\'\`\$@~#%^&*()+={[}]|:;<>?/" 12 1)
data="{\"Username\":\"${CLOUDNAME}\", \"Password\":\"${PASSWD}\"}"
response=`curl -sS -X POST "${URL}/api/users/admin/init" -H "Content-Type: application/json" -d "$data"`
#echo $reponse

echo "{\"username\":\"${CLOUDNAME}\", \"password\":\"${PASSWD}\"}" > /disk/admin/modules/_config_/portainer.json

echo -n "{ \"a\":\"status\", \"module\":\"$(basename $0 -user.sh)\", \"state\":\"finish\" }" | nc -w 1 localhost 8093
