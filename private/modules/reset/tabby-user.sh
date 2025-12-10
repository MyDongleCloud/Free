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
DATE=`date +%s`
URL="http://localhost:8100"
CLOUDNAME=`cat /disk/admin/modules/_config_/_cloud_.json | jq -r ".info.name"`
PASSWD="$(tr -dc 'A-HJ-NP-Za-km-z1-9' < /dev/urandom | head -c 8)aA.1"
EMAIL="admin@$CLOUDNAME.mydongle.cloud"
URL2="https://aicode.$CLOUDNAME.mydongle.cloud"

curl -sS --fail -X POST $URL/graphql -H "Content-Type: application/json" --data-binary "{\"operationName\":\"register\",\"query\":\"mutation register(\$name: String!, \$email: String!, \$password1: String!, \$password2: String!, \$invitationCode: String) {\n  register(\n    name: \$name\n    email: \$email\n    password1: \$password1\n    password2: \$password2\n    invitationCode: \$invitationCode\n  ) {\n    accessToken\n    refreshToken\n    __typename\n  }\n}\",\"variables\":{\"email\":\"$EMAIL\",\"name\":\"$CLOUDNAME\",\"password1\":\"$PASSWD\",\"password2\":\"$PASSWD\"}}"

#curl -sS --fail -X POST $URL/graphql -H "Content-Type: application/json" --data-binary "{\"operationName\":\"updateNetworkSettingMutation\",\"query\":\"mutation updateNetworkSettingMutation(\$input: NetworkSettingInput!) {\n  updateNetworkSetting(input: \$input)\n}\",\"variables\":{\"input\":{\"externalUrl\":\"$URL2\"}}}"

echo "{\"name\":\"${CLOUDNAME}\", \"email\":\"${EMAIL}\", \"password\":\"${PASSWD}\"}" > /disk/admin/modules/_config_/tabby.json
