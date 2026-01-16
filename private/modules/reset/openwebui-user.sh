#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for openwebui-user [-h]"
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

echo "#Create user openwebui##################"
PORT=8101
URL="http://localhost:$PORT"
TIMEOUT=60
while [ $TIMEOUT -gt 0 ]; do
    sleep 1
    TIMEOUT=$((TIMEOUT - 1))
    [ $TIMEOUT -eq 0 ] && echo "Timeout port waiting for openwebui" && exit
	nc -z localhost $PORT 2> /dev/null
	if [ $? = 0 ]; then
		break
	fi
done
TIMEOUT=40
while [ $TIMEOUT -gt 0 ]; do
    sleep 1
    TIMEOUT=$((TIMEOUT - 1))
    [ $TIMEOUT -eq 0 ] && echo "Timeout api waiting for openwebui" && exit
	STATUS=`curl -s -o /dev/null -w "%{http_code}" "$URL/health"`
	if [ $STATUS = 200 ]; then
		break
	fi
done
echo "Doing openwebui user"

CLOUDNAME=`cat /disk/admin/modules/_config_/_cloud_.json | jq -r ".info.name"`
PASSWD=$(pwgen -B -c -y -n -r "\"\!\'\`\$@~#%^&*()+={[}]|:;<>?/" 12 1)

email="admin@${CLOUDNAME}.mydongle.cloud"
name="${CLOUDNAME}"
passwd="${PASSWD}"

data="{ \"email\": \"$email\", \"name\": \"$name\", \"password\": \"$passwd\" }"
response=`curl -sS -X POST $URL/api/v1/auths/signup -H "Content-Type: application/json" -d "$data"`
token=`echo $response | jq -r ".token"`
#echo "token: $token"

data="{ \"ENABLE_OPENAI_API\":true, \"OPENAI_API_BASE_URLS\":[\"https://aiproxy.mydongle.cloud/v1\"], \"OPENAI_API_KEYS\":[\"api.mistral.ai\"], \"OPENAI_API_CONFIGS\":{ \"additionalProp1\": {} } }"
response=`curl -sS -X POST $URL/openai/config/update -H "Authorization: Bearer $token" -H "Content-Type: application/json" -d "$data"`
#echo $response

echo "{\"name\":\"${name}\", \"email\":\"${email}\", \"password\":\"${passwd}\"}" > /disk/admin/modules/_config_/openwebui.json

echo -n "{ \"a\":\"status\", \"module\":\"$(basename $0 .sh)\", \"state\":\"finish\" }" | nc -w 1 localhost 8093
