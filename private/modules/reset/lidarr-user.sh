#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for lidarr-user [-h]"
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

echo "#Create lidarr settings##################"
PORT=8686
TIMEOUT=10
echo "$TIMEOUT seconds to watch lidarr starting..."
while [ $TIMEOUT -gt 0 ]; do
    sleep 1
    TIMEOUT=$((TIMEOUT - 1))
    [ $TIMEOUT -eq 0 ] && echo "Timeout waiting for lidarr" && exit
	nc -z localhost $PORT 2> /dev/null
	if [ $? = 0 ]; then
		break
	fi
done

sleep 10
URL="http://localhost:$PORT"
PASSWD=`jq -r .password /disk/admin/modules/_config_/qbittorrent.json`
APIKEY=`jq -r .apikey /disk/admin/modules/_config_/lidarr.json`
DATA="{ \"name\":\"qBittorrent\", \"implementation\":\"QBittorrent\", \"configContract\":\"QBittorrentSettings\", \"protocol\":\"torrent\", \"enable\":true, \"priority\":25, \"tags\":[], \"fields\":[ { \"name\":\"host\", \"value\":\"localhost\"}, { \"name\":\"port\", \"value\":8109 }, { \"name\":\"username\", \"value\":\"admin\" }, { \"name\":\"password\", \"value\":\"$PASSWD\" } ], \"enable\":true }"
response=`curl -sS -X POST "$URL/api/v1/downloadclient" -H "X-Api-Key: $APIKEY" -H "Content-Type: application/json" -d "$DATA"`
echo $response
