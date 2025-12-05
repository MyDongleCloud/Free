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
PASSWORD=$(tr -dc 'A-HJ-NP-Za-km-z1-9' < /dev/urandom | head -c 8)
#systemctl stop postgresql.service
sudo -u postgres psql <<-EOF
ALTER USER postgres WITH PASSWORD '$PASSWORD';
EOF
#systemctl start postgresql.service
echo "{\"username\":\"postgres\", \"password\":\"${PASSWORD}\"}" > /disk/admin/modules/_config_/postgresql.json
chown admin:admin /disk/admin/modules/_config_/postgresql.json
