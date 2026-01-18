#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for jellyfin-user [-h]"
echo "h:	Print this usage and exit"
exit 0
}

if [ "m`id -u`" != "m0" ]; then
	echo "You need to be root"
	exit 0
fi

while getopts h opt
do
	case "$opt" in
		h) helper;;
	esac
done

echo "#Create user jellyfin##################"
PORT=8096
URL="http://localhost:$PORT"
TIMEOUT=20
while [ $TIMEOUT -gt 0 ]; do
    sleep 3
    TIMEOUT=$((TIMEOUT - 1))
    [ $TIMEOUT -eq 0 ] && echo "Timeout port waiting for jellyfin" && exit
	nc -z localhost $PORT 2> /dev/null
	if [ $? = 0 ]; then
		break
	fi
done
sleep 10
echo "Doing jellyfin user"

CLOUDNAME=`cat /disk/admin/modules/_config_/_cloud_.json | jq -r ".info.name"`
PASSWD=$(pwgen -B -c -y -n -r "\"\!\'\`\$@~#%^&*()+={[}]|:;<>?/" 12 1)

curl -sS -X POST "$URL/Startup/Configuration" -H "Content-Type: application/json" -d '{"UICulture":"en-US", "MetadataCountryCode":"US", "PreferredMetadataLanguage":"en"}'
curl -sS -X GET "$URL/Startup/FirstUser"
data="{ \"Name\":\"$CLOUDNAME\", \"Password\": \"$PASSWD\" }"
curl -sS -X POST $URL/Startup/User -H "Content-Type: application/json" -d "$data"
curl -sS -X POST "$URL/Startup/Complete"

data="{ \"Username\":\"$CLOUDNAME\", \"Pw\": \"$PASSWD\" }"
response=`curl -sS -X POST "$URL/Users/AuthenticateByName" -H "Content-Type: application/json" -H 'X-Emby-Authorization: MediaBrowser Client="AutomatedScript", Device="Linux", DeviceId="12345", Version="1.0.0"' -d "$data"`
#echo $response
token=`echo $response | jq -r ".AccessToken"`
#echo "token: $token"

curl -sS -X POST "$URL/Auth/Keys?App=ApiKey" -H "X-Emby-Token:$token" -H "Content-Type: application/json"
response=`curl -sS -X GET "$URL/Auth/Keys" -H "X-Emby-Token:$token"`
#echo $reponse
APIKEY=`echo $response | jq -r ".Items[0].AccessToken"`

echo "{\"username\":\"${CLOUDNAME}\", \"password\":\"${PASSWD}\", \"apikey\":\"${APIKEY}\"}" > /disk/admin/modules/_config_/jellyfin.json

echo -n "{ \"a\":\"status\", \"module\":\"$(basename $0 -user.sh)\", \"state\":\"finish\" }" | nc -w 1 localhost 8093
