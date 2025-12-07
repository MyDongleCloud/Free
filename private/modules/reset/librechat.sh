#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for metube [-h -r]"
echo "h:	Print this usage and exit"
echo "r:	Reset"
exit 0
}

if [ "m`id -u`" = "m0" ]; then
	echo "You should not be root"
	exit 0
fi

RESET=0
while getopts hr opt
do
	case "$opt" in
		h) helper;;
		r) RESET=1;;
	esac
done

if [ $RESET != 1 ]; then
	exit 0
fi

echo "#Reset librechat##################"
CLOUDNAME=`cat /disk/admin/modules/_config_/_cloud_.json | jq -r ".info.name"`
MEILISEARCH_KEY=`cat /disk/admin/modules/_config_/meilisearch.json | jq -r ".key"`
cd /disk/admin/modules
systemctl stop librechat.service

MONGODB_PASSWORD=`cat /disk/admin/modules/_config_/mongodb.json | jq -r ".password"`
DBPASS=$(tr -dc 'A-HJ-NP-Za-km-z1-9' < /dev/urandom | head -c 8)
mongosh --host 127.0.0.1 -u admin --authenticationDatabase admin -p $MONGODB_PASSWORD <<EOF
use librechatDB
db.dropDatabase()
db.dropUser("librechatUser")
db.createUser({
	user: "librechatUser",
	pwd: "$DBPASS",
	roles: [
		{ role: "readWrite", db: "librechatDB" },
		{ role: "dbAdmin", db: "librechatDB" }
	]
});
EOF

EMAIL="admin@$CLOUDNAME.mydongle.cloud"
PASSWD=$(tr -dc 'A-HJ-NP-Za-km-z1-9' < /dev/urandom | head -c 8)
NAME=$CLOUDNAME
USERNAME=$CLOUDNAME

rm -rf /disk/admin/modules/librechat
mkdir -p /disk/admin/modules/librechat/logs
cp /usr/local/modules/librechat/.env.example /disk/admin/modules/librechat/.env
cp /usr/local/modules/librechat/librechat.example.yaml /disk/admin/modules/librechat/librechat.yaml
sed -i -e "s|^MEILI_HOST=.*|MEILI_HOST=http://127.0.0.1:7700|" /disk/admin/modules/librechat/.env
sed -i -e "s|^MEILI_MASTER_KEY=.*|MEILI_MASTER_KEY=$MEILISEARCH_KEY|" /disk/admin/modules/librechat/.env
sed -i -e "s|^MONGO_URI=.*|MONGO_URI=mongodb://librechatUser:$DBPASS@127.0.0.1:27017/librechatDB|" /disk/admin/modules/librechat/.env
chown admin:admin /disk/admin/modules/librechat

cd /usr/local/modules/librechat
NODE_DEBUG=mongoose npm run create-user -- $EMAIL $NAME $USERNAME $PASSWD --email-verified=false
echo "{\"email\":\"${EMAIL}\", \"username\":\"${USERNAME}\", \"password\":\"${PASSWD}\"}" > /disk/admin/modules/_config_/librechat.json

systemctl start librechat.service
systemctl enable librechat.service
