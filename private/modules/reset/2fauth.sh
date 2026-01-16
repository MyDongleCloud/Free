#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for 2fauth [-h -r]"
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


echo "#Reset 2fauth##################"
CLOUDNAME=`cat /disk/admin/modules/_config_/_cloud_.json | jq -r ".info.name"`
PASSWD=$(pwgen -B -c -y -n -r "\"\!\'\`\$@~#%^&*()+={[}]|:;<>?/" 12 1)
EMAIL=admin@${CLOUDNAME}.mydongle.cloud
KEY=$(head -c 32 /dev/urandom | base64 | tr -d '\n')
rm -rf /disk/admin/modules/2fauth
mkdir -p /disk/admin/modules/2fauth
sqlite3 /disk/admin/modules/2fauth/database.sqlite ""
cp /usr/local/modules/2fauth/env /disk/admin/modules/2fauth/env
sed -i -e "s/^SITE_OWNER=.*/SITE_OWNER=admin@${CLOUDNAME}.mydongle.cloud/" /disk/admin/modules/2fauth/env
sed -i -e "s@^APP_URL=.*@APP_URL=https://2fa.${CLOUDNAME}.mydongle.cloud@" /disk/admin/modules/2fauth/env
sed -i -e "s@^APP_KEY=.*@APP_KEY=base64:${KEY}@" /disk/admin/modules/2fauth/env
sed -i -e 's@^DB_DATABASE=.*@DB_DATABASE=/disk/admin/modules/2fauth/database.sqlite@' /disk/admin/modules/2fauth/env
cd /usr/local/modules/2fauth
php artisan 2fauth:install --no-interaction
echo "\App\Models\User::create(['name' => '${CLOUDNAME}', 'email' => '${EMAIL}', 'password' => Hash::make('${PASSWD}')]);" | php artisan tinker
chown -R admin:www-data /disk/admin/modules/2fauth
chmod 775 /disk/admin/modules/2fauth
chmod 664 /disk/admin/modules/2fauth/database.sqlite
chmod -R 775 /usr/local/modules/2fauth/storage
echo "{\"email\":\"${EMAIL}\", \"password\":\"${PASSWD}\"}" > /disk/admin/modules/_config_/2fauth.json

echo -n "{ \"a\":\"status\", \"module\":\"$(basename $0 .sh)\", \"state\":\"finish\" }" | nc -w 1 localhost 8093