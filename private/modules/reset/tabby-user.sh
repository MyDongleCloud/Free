#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for tabby-user [-h]"
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

echo "#Create user tabby##################"
PORT=8100
URL="http://localhost:$PORT"
TIMEOUT=20
while [ $TIMEOUT -gt 0 ]; do
	sleep 3
	TIMEOUT=$((TIMEOUT - 1))
	[ $TIMEOUT -eq 0 ] && echo "Timeout port waiting for tabby" && exit
	nc -z localhost $PORT 2> /dev/null
	if [ $? = 0 ]; then
		break
	fi
done
TIMEOUT=20
while [ $TIMEOUT -gt 0 ]; do
	sleep 3
	TIMEOUT=$((TIMEOUT - 1))
	[ $TIMEOUT -eq 0 ] && echo "Timeout api waiting for jellyfin" && exit
	response=`curl -sS -X POST http://localhost:8100/graphql -H "Content-Type: application/json" --data-binary "{}"`
	if [ $? = 0 ]; then
		break
	fi
done
echo "Doing tabby user"

CLOUDNAME=`cat /disk/admin/modules/_config_/_cloud_.json | jq -r ".info.name"`
PASSWD=$(pwgen -B -c -y -n -r "\"\!\'\`\$@~#%^&*()+={[}]|:;<>?/" 12 1)
EMAIL="admin@$CLOUDNAME.mydongle.cloud"
URL2="https://aicode.$CLOUDNAME.mydongle.cloud"

curl -sS --fail -o /tmp/tabby.txt -X POST $URL/graphql -H "Content-Type: application/json" --data-binary "{\"operationName\":\"register\",\"query\":\"mutation register(\$name: String!, \$email: String!, \$password1: String!, \$password2: String!, \$invitationCode: String) {\n register(\n name: \$name\n email: \$email\n password1: \$password1\n password2: \$password2\n invitationCode: \$invitationCode\n ) {\n accessToken\n refreshToken\n __typename\n }\n}\",\"variables\":{\"email\":\"$EMAIL\",\"name\":\"$CLOUDNAME\",\"password1\":\"$PASSWD\",\"password2\":\"$PASSWD\"}}"
token=`jq -r ".data.register.accessToken" /tmp/tabby.txt`
rm -f /tmp/tabby.txt
echo "{\"name\":\"${CLOUDNAME}\", \"email\":\"${EMAIL}\", \"password\":\"${PASSWD}\"}" > /disk/admin/modules/_config_/tabby.json
respone=`curl -sS -X POST $URL/graphql -H "Content-Type: application/json" -H "Authorization: Bearer $token" --data-binary "{\"operationName\":\"updateNetworkSettingMutation\",\"query\":\"mutation updateNetworkSettingMutation(\$input: NetworkSettingInput!) {\n updateNetworkSetting(input: \$input)\n}\",\"variables\":{\"input\":{\"externalUrl\":\"$URL2\"}}}"`
#echo $response

echo {" \"a\":\"status\", \"module\":\"$(basename $0 -user.sh)\", \"state\":\"finish\" }" | websocat -1 ws://localhost:8094
