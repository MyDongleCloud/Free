#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for postgres [-h -r]"
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

echo "#Reset postgresql##################"
PASSWORD=$(pwgen -B -c -y -n -r "\"\!\'\`\$@~#%^&*()+={[}]|:;<>?/" 12 1)
grep -q "local all postgres md5" /etc/postgresql/17/main/pg_hba.conf
if [ $? = 0 ]; then
	echo "Restarting postgresql service before changing password"
	sed -i -e "s/^local all postgres md5/local all postgres peer/" /etc/postgresql/17/main/pg_hba.conf
	systemctl restart postgresql.service
fi
su postgres -c "psql" << EOF
ALTER USER postgres WITH PASSWORD '$PASSWORD';
EOF
sed -i -e "s/local *all *postgres *peer/local all postgres md5/" /etc/postgresql/17/main/pg_hba.conf
systemctl restart postgresql.service
echo "{\"username\":\"postgres\", \"password\":\"${PASSWORD}\"}" > /disk/admin/modules/_config_/postgresql.json
chown admin:admin /disk/admin/modules/_config_/postgresql.json
{
cat /disk/admin/modules/_config_/adminer.json 2>/dev/null || echo '{}'
echo "{\"postgresql_username\":\"postgres\", \"postgresql_password\":\"${PASSWORD}\"}"
} | jq -s 'add' > /disk/admin/modules/_config_/adminer.json.tmp && mv /disk/admin/modules/_config_/adminer.json.tmp /disk/admin/modules/_config_/adminer.json
chown admin:admin /disk/admin/modules/_config_/adminer.json

echo -n "{ \"a\":\"status\", \"module\":\"$(basename $0 .sh)\", \"state\":\"finish\" }" | nc -w 1 localhost 8093