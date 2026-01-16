#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for whisparr-user [-h]"
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

echo "#Create whisparr settings##################"
PORT=6969
URL="http://localhost:$PORT"
TIMEOUT=40
while [ $TIMEOUT -gt 0 ]; do
    sleep 1
    TIMEOUT=$((TIMEOUT - 1))
    [ $TIMEOUT -eq 0 ] && echo "Timeout port waiting for whisparr" && exit
	nc -z localhost $PORT 2> /dev/null
	if [ $? = 0 ]; then
		break
	fi
done
TIMEOUT=40
while [ $TIMEOUT -gt 0 ]; do
    sleep 1
    TIMEOUT=$((TIMEOUT - 1))
    [ $TIMEOUT -eq 0 ] && echo "Timeout api waiting for whisparr" && exit
	STATUS=`curl -s -o /dev/null -w "%{http_code}" "$URL"`
	if [ $STATUS = 200 ]; then
		break
	fi
done
echo "Doing whisparr settings"

PASSWD=`jq -r .password /disk/admin/modules/_config_/qbittorrent.json`
APIKEY=`jq -r .apikey /disk/admin/modules/_config_/whisparr.json`
DATA="{ \"name\":\"qBittorrent\", \"implementation\":\"QBittorrent\", \"configContract\":\"QBittorrentSettings\", \"protocol\":\"torrent\", \"enable\":true, \"priority\":25, \"tags\":[], \"fields\":[ { \"name\":\"host\", \"value\":\"localhost\"}, { \"name\":\"port\", \"value\":8109 }, { \"name\":\"username\", \"value\":\"admin\" }, { \"name\":\"password\", \"value\":\"$PASSWD\" }, { \"name\": \"initialState\", \"value\": 2 } ], \"enable\":true }"
response=`curl -sS -X POST "$URL/api/v3/downloadclient" -H "X-Api-Key: $APIKEY" -H "Content-Type: application/json" -d "$DATA"`
#echo $response

DATA="{\"path\":\"/disk/admin/modules/whisparr/downloads\"}"
response=`curl -sS -X POST "$URL/api/v3/rootfolder" -H "X-Api-Key: $APIKEY" -H "Content-Type: application/json" -d "$DATA"`
#echo $response

echo -n "{ \"a\":\"status\", \"module\":\"$(basename $0 .sh)\", \"state\":\"finish\" }" | nc -w 1 localhost 8093
