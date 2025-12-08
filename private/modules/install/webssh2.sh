#!/bin/sh

cd /usr/local/modules/webssh2
npm install --production
node scripts/prestart.js
cat > config.json <<EOF
{
	"listen": {
		"ip": "127.0.0.1",
		"port": 2222
	},
	"ssh": {
		"host": "localhost",
		"port": 22
	},
	"auth": {
		"method": "post"
	}
}
EOF
ln -sf /etc/systemd/system/webssh2.service /etc/systemd/system/multi-user.target.wants/webssh2.service
