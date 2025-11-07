#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for mongodb [-h -r]"
echo "h:	Print this usage and exit"
echo "r:	Reset"
exit 0
}

if [ "m`id -u`" != "m0" ]; then
	echo "You need to be root"
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

echo "#Reset mongodb##################"
DATE=`date +%s`
PASSWORD=$(tr -dc 'A-HJ-NP-Za-km-z1-9' < /dev/urandom | head -c 8)
systemctl stop mongodb.service
rm -rf /disk/admin/modules/mongodb
mkdir /disk/admin/modules/mongodb
mongod --fork --logpath /var/log/mongodb/mongod.log --config /etc/mongod.conf --bind_ip 127.0.0.1 --noauth > /tmp/reset-mongodb-$DATE.log 2>&1
TIMEOUT=10
echo "10 seconds to watch MongoDB starting..."
while [ $TIMEOUT -gt 0 ]; do
    sleep 1
    TIMEOUT=$((TIMEOUT - 1))
    [ $TIMEOUT -eq 0 ] && echo "Timeout waiting for MongoDB starting" && exit 1
    mongosh --host 127.0.0.1 --eval "db.adminCommand('ping')" > /dev/null 2>&1
    if [ $? = 0 ]; then
        break
    fi
done
echo "Done"
mongosh --host 127.0.0.1 > /dev/null <<EOF
use admin
db.createUser({
	user: "admin",
	pwd: "$PASSWORD",
	roles: [
		{ role: "userAdminAnyDatabase", db: "admin" },
		{ role: "readWriteAnyDatabase", db: "admin" },
		{ role: "clusterAdmin", db: "admin" }
	]
})
db.auth("admin", "$PASSWORD")
db.adminCommand({ shutdown: 1 })
EOF
TIMEOUT=10
echo "10 seconds to watch MongoDB stopping..."
while [ $TIMEOUT -gt 0 ]; do
	ps avx | grep mongod
	pgrep -x mongod
	pgrep -x mongod > /dev/null
    if [ $? = 1 ]; then
        break
    fi
    sleep 1
    TIMEOUT=$((TIMEOUT - 1))
    [ $TIMEOUT -eq 0 ] && killall mongod
done
echo "Done"
rm -f /tmp/mongodb-27017.sock /var/log/mongodb/mongod.log
chown -R mongodb:mongodb /disk/admin/modules/mongodb /var/log/mongodb
systemctl start mongodb.service
systemctl enable mongodb.service
echo "{\"username\":\"admin\", \"password\":\"${PASSWORD}\"}" > /disk/admin/modules/_config_/mongodb.json
chown admin:admin /disk/admin/modules/_config_/mongodb.json
