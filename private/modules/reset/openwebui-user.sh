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
TIMEOUT=30
echo "$TIMEOUT seconds to watch openwebui starting..."
while [ $TIMEOUT -gt 0 ]; do
    sleep 1
    TIMEOUT=$((TIMEOUT - 1))
    [ $TIMEOUT -eq 0 ] && echo "Timeout waiting for openwebui" && exit
	nc -z localhost 8101 2> /dev/null
	if [ $? = 0 ]; then
		break
	fi
done

DATE=`date +%s`
URL="http://localhost:8101"
CLOUDNAME=`cat /disk/admin/modules/_config_/_cloud_.json | jq -r ".info.name"`
PASSWD=$(pwgen -B -c -y -n -r "\"\!\'\`\$@~#%^&*()+={[}]|:;<>?/" 12 1)

email="admin@${CLOUDNAME}.mydongle.cloud"
name="${CLOUDNAME}"
passwd="${PASSWD}"

data="{ \"email\": \"$email\", \"name\": \"$name\", \"password\": \"$passwd\" }"
response=`curl -sS --fail -X POST $URL/api/v1/auths/signup -H "Content-Type: application/json" -d "$data"`
#echo $response
token=`echo $response | jq -r ".token"`
#echo "token: $token"

data="{ \"ENABLE_OPENAI_API\":true, \"OPENAI_API_BASE_URLS\":[\"https://aiproxy.mydongle.cloud/v1\"], \"OPENAI_API_KEYS\":[\"api.mistral.ai\"], \"OPENAI_API_CONFIGS\":{ \"additionalProp1\": {} } }"
response=`curl -sS --fail -X POST $URL/openai/config/update -H "Authorization: Bearer $token" -H "Content-Type: application/json" -d "$data"`
#echo $response

echo "{\"name\":\"${name}\", \"email\":\"${email}\", \"password\":\"${passwd}\"}" > /disk/admin/modules/_config_/openwebui.json
