#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for webtrees [-h -r]"
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


echo "#Reset 2fauth##################"
CLOUDNAME=`cat /disk/admin/modules/_config_/_cloud_.json | jq -r ".info.name"`
PASSWD=$(pwgen -B -c -y -n -r "\"\!\'\`\$@~#%^&*()+={[}]|:;<>?/" 12 1)
EMAIL=admin@${CLOUDNAME}.mydongle.cloud
KEY=$(head -c 32 /dev/urandom | base64 | tr -d '\n')
rm -rf /disk/admin/modules/2fauth
mkdir -p /disk/admin/modules/2fauth
cp /usr/local/modules/2fauth/database/database.sqlite /disk/admin/modules/2fauth/database.sqlite
cp /usr/local/modules/2fauth/env /disk/admin/modules/2fauth/env
sed -i -e "s@^APP_URL=.*@APP_URL=https://2fa.${CLOUDNAME}.mydongle.cloud@" /disk/admin/modules/2fauth/env
sed -i -e "s@^APP_KEY=.*@APP_KEY=base64:${KEY}@" /disk/admin/modules/2fauth/env
sed -i -e 's@^DB_DATABASE=.*@DB_DATABASE=/disk/admin/modules/2fauth/database.sqlite@' /disk/admin/modules/2fauth/env
cd /usr/local/modules/2fauth
php artisan 2fauth:install --no-interaction
echo "\App\Models\User::create(['name' => '${CLOUDNAME}', 'email' => '${EMAIL}', 'password' => Hash::make('${PASSWD}')]);" | php artisan tinker
chown -R www-data:admin /disk/admin/modules/2fauth/
chown -R www-data:www-data /usr/local/modules/2fauth/
echo "{\"email\":\"${EMAIL}\", \"password\":\"${PASSWD}\"}" > /disk/admin/modules/_config_/2fauth.json
chown admin:admin /disk/admin/modules/_config_/2fauth.json
