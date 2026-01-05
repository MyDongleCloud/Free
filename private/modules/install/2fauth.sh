#!/bin/sh

rm -rf /usr/local/modules/2fauth
cd /usr/local/modules
git clone https://github.com/Bubka/2FAuth 2fauth
cd /usr/local/modules/2fauth
git checkout v5.6.1
rm -rf .git

cd /usr/local/modules/2fauth
composer -n install

cat > .env << EOF
APP_NAME=2FAuth
APP_ENV=local
APP_TIMEZONE=UTC
APP_DEBUG=false
SITE_OWNER=mail@example.com
APP_KEY=base64:
APP_URL=http://localhost
APP_SUBDIRECTORY=
IS_DEMO_APP=false
LOG_CHANNEL=daily
LOG_LEVEL=notice
CACHE_DRIVER=file
SESSION_DRIVER=file
DB_CONNECTION=sqlite
DB_DATABASE=/usr/local/modules/2fauth/database/database.sqlite
DB_HOST=
DB_PORT=
DB_USERNAME=
DB_PASSWORD=
MYSQL_ATTR_SSL_CA=
MAIL_MAILER=log
MAIL_HOST=localhost
MAIL_PORT=465
MAIL_USERNAME=null
MAIL_PASSWORD=null
MAIL_ENCRYPTION=null
MAIL_FROM_NAME=null
MAIL_FROM_ADDRESS=null
MAIL_VERIFY_SSL_PEER=true
THROTTLE_API=60
LOGIN_THROTTLE=5
AUTHENTICATION_GUARD=web-guard
AUTHENTICATION_LOG_RETENTION=365
AUTH_PROXY_HEADER_FOR_USER=null
AUTH_PROXY_HEADER_FOR_EMAIL=null
PROXY_LOGOUT_URL=null
WEBAUTHN_NAME=2FAuth
WEBAUTHN_ID=null
WEBAUTHN_USER_VERIFICATION=preferred
TRUSTED_PROXIES=null
PROXY_FOR_OUTGOING_REQUESTS=null
CONTENT_SECURITY_POLICY=true
BROADCAST_DRIVER=log
QUEUE_DRIVER=sync
REDIS_HOST=127.0.0.1
REDIS_PASSWORD=null
REDIS_PORT=6379
PUSHER_APP_ID=
PUSHER_APP_KEY=
PUSHER_APP_SECRET=
PUSHER_APP_CLUSTER=mt1
VITE_PUSHER_APP_KEY="${PUSHER_APP_KEY}"
VITE_PUSHER_APP_CLUSTER="${PUSHER_APP_CLUSTER}"
MIX_ENV=local
EOF
KEY=$(head -c 32 /dev/urandom | base64 | tr -d '\n')
sed -i -e "s@^APP_KEY=.*@APP_KEY=base64:${KEY}@" .env
sqlite3 database/database.sqlite ""
php artisan 2fauth:install --no-interaction
mv .env env
ln -sf /disk/admin/modules/2fauth/env .env
