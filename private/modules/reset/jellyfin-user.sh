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
TIMEOUT=30
echo "$TIMEOUT seconds to watch jellyfin starting..."
while [ $TIMEOUT -gt 0 ]; do
    sleep 1
    TIMEOUT=$((TIMEOUT - 1))
    [ $TIMEOUT -eq 0 ] && echo "Timeout waiting for jellyfin" && exit
	nc -z localhost 8096 2> /dev/null
	if [ $? = 0 ]; then
		break
	fi
done

echo -n "Waiting 20 seconds... "
sleep 20
echo "done"

URL="http://localhost:8096"
CLOUDNAME=`cat /disk/admin/modules/_config_/_cloud_.json | jq -r ".info.name"`
PASSWD=$(pwgen -B -c -y -n -r "\"\!\'\`\$@~#%^&*()+={[}]|:;<>?/" 12 1)

curl -sS --fail -X POST "$URL/Startup/Configuration" -H "Content-Type: application/json" -d '{"UICulture":"en-US", "MetadataCountryCode":"US", "PreferredMetadataLanguage":"en"}'
curl -sS --fail -X GET "$URL/Startup/FirstUser"
data="{ \"Name\":\"$CLOUDNAME\", \"Password\": \"$PASSWD\" }"
curl -sS --fail -X POST $URL/Startup/User -H "Content-Type: application/json" -d "$data"
curl -sS --fail -X POST "$URL/Startup/Complete"

data="{ \"Username\":\"$CLOUDNAME\", \"Pw\": \"$PASSWD\" }"
response=`curl -sS --fail -X POST "$URL/Users/AuthenticateByName" -H "Content-Type: application/json" -H 'X-Emby-Authorization: MediaBrowser Client="AutomatedScript", Device="Linux", DeviceId="12345", Version="1.0.0"' -d "$data"`
token=`echo $response | jq -r ".AccessToken"`
echo "token: $token"
curl -sS --fail -X POST "$URL/Auth/Keys?App=ApiKey" -H "X-Emby-Token:$token" -H "Content-Type: application/json"
response=`curl -sS --fail -X GET "$URL/Auth/Keys" -H "X-Emby-Token:$token"`
APIKEY=`echo $response | jq -r ".Items[0].AccessToken"`

echo "{\"username\":\"${CLOUDNAME}\", \"password\":\"${PASSWD}\", \"apikey\":\"${APIKEY}\"}" > /disk/admin/modules/_config_/jellyfin.json

echo -n "{ \"a\":\"status\", \"module\":\"$(basename $0 .sh)\", \"state\":\"finish\" }" | nc -w 1 localhost 8093