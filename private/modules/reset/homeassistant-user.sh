#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for homeassistant-user [-h]"
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

echo "#Create user homeassistant##################"
PORT=8123
URL="http://localhost:$PORT"
TIMEOUT=40
while [ $TIMEOUT -gt 0 ]; do
    sleep 1
    TIMEOUT=$((TIMEOUT - 1))
    [ $TIMEOUT -eq 0 ] && echo "Timeout port waiting for homeassistant" && exit
	nc -z localhost $PORT 2> /dev/null
	if [ $? = 0 ]; then
		break
	fi
done
TIMEOUT=40
while [ $TIMEOUT -gt 0 ]; do
    sleep 1
    TIMEOUT=$((TIMEOUT - 1))
    [ $TIMEOUT -eq 0 ] && echo "Timeout api waiting for homeassistant" && exit
	STATUS=`curl -s -o /dev/null -w "%{http_code}" "$URL/api/onboarding"`
	if [ $STATUS = 200 ]; then
		break
	fi
done
echo "Doing homeassistant user"

CLOUDNAME=`cat /disk/admin/modules/_config_/_cloud_.json | jq -r ".info.name"`
PASSWD=$(pwgen -B -c -y -n -r "\"\!\'\`\$@~#%^&*()+={[}]|:;<>?/" 12 1)

name="$CLOUDNAME"
username="${CLOUDNAME}"
passwd="${PASSWD}"
language="en"

data="{ \"client_id\": \"$URL\", \"name\": \"$name\", \"username\": \"$username\", \"password\": \"$passwd\", \"language\": \"$language\" }"
response=`curl -sS -X POST $URL/api/onboarding/users -H "Content-Type: application/json" -d "$data"`
auth_code=`echo $response | jq -r ".auth_code"`
#echo "auth_code: $auth_code"

response=`curl -sS -X POST $URL/auth/token -F "client_id=$URL" -F "code=$auth_code" -F "grant_type=authorization_code"`
token=`echo $response | jq -r ".access_token"`
bearer="Bearer $token"
#echo "bearer: $bearer"

response=`curl -sS -X POST $URL/api/onboarding/core_config -H "Authorization: $bearer" -H "Content-Type: application/json"`
#echo "core_config: $response"

data='{ "handler": "mqtt", "data": { "broker": "localhost", "port": 1883, "client_id": "home-assistant", "discovery": true } }'
data='{ "handler": "mqtt" }'
response=`curl -sS -X POST $URL/api/config/config_entries/flow -H "Authorization: $bearer" -H "Content-Type: application/json" -d "$data"`
flow_id=`echo $response | jq -r ".flow_id"`
#echo "mqtt flow_id: $flow_id"

data='{ "broker": "localhost", "port": 1883 }'
response=`curl -sS -X POST $URL/api/config/config_entries/flow/$flow_id -H "Authorization: $bearer" -H "Content-Type: application/json" -d "$data"`
created_at=`echo $response | jq -r ".result.created_at"`
#echo "mqtt created_at: $created_at"

response=`curl -sS -X POST $URL/api/onboarding/analytics -H "Authorization: $bearer" -H "Content-Type: application/json"`
#echo "analytics: $response"

echo "{\"username\":\"${username}\", \"password\":\"${passwd}\"}" > /disk/admin/modules/_config_/homeassistant.json

echo -n "{ \"a\":\"status\", \"module\":\"$(basename $0 .sh)\", \"state\":\"finish\" }" | nc -w 1 localhost 8093
