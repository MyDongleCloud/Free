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
DATE=`date +%s`
URL="http://localhost:8123"
SPACENAME=`cat /disk/admin/modules/mydonglecloud/space.json | jq -r ".name"`
PASSWD=$(tr -dc 'A-HJ-NP-Za-km-z1-9' < /dev/urandom | head -c 8)

name="$SPACENAME"
username="${SPACENAME}"
passwd="${PASSWD}"
language="en"
clientid="myclientid"

data="{ \"client_id\": \"$URL\", \"name\": \"$name\", \"username\": \"$username\", \"password\": \"$passwd\", \"language\": \"$language\" }"
response=`curl -sS --fail -X POST $URL/api/onboarding/users -H "Content-Type: application/json" -d "$data"`
auth_code=`echo $response | jq -r ".auth_code"`
echo "auth_code: $auth_code"

response=`curl -sS --fail -X POST $URL/auth/token -F "client_id=$URL" -F "code=$auth_code" -F "grant_type=authorization_code"`
token=`echo $response | jq -r ".access_token"`
bearer="Bearer $token"
echo "bearer: $bearer"

response=`curl -sS --fail -X POST $URL/api/onboarding/core_config -H "Authorization: $bearer" -H "Content-Type: application/json"`
echo "core_config: $response"

data='{ "handler": "mqtt", "data": { "broker": "localhost", "port": 1883, "client_id": "home-assistant", "discovery": true } }'
data='{ "handler": "mqtt" }'
response=`curl -sS --fail -X POST $URL/api/config/config_entries/flow -H "Authorization: $bearer" -H "Content-Type: application/json" -d "$data"`
flow_id=`echo $response | jq -r ".flow_id"`
echo "mqtt flow_id: $flow_id"

data='{ "broker": "localhost", "port": 1883 }'
response=`curl -sS --fail -X POST $URL/api/config/config_entries/flow/$flow_id -H "Authorization: $bearer" -H "Content-Type: application/json" -d "$data"`
created_at=`echo $response | jq -r ".result.created_at"`
echo "mqtt created_at: $created_at"

response=`curl -sS --fail -X POST $URL/api/onboarding/analytics -H "Authorization: $bearer" -H "Content-Type: application/json"`
echo "analytics: $response"

echo "{\"user\":\"${username}\", \"password\":\"${passwd}\"}" > /disk/admin/modules/_config_/homeassistant.json
