#!/bin/sh

cd /usr/local/modules/lobechat
pnpm install
pnpm install @clerk/shared @playwright/browser-chromium @scarf/scarf @tree-sitter-grammars/tree-sitter-yaml canvas core-js core-js-pure es5-ext esbuild protobufjs sharp tree-sitter tree-sitter-json unrs-resolver

KEY_VAULTS_SECRET=$(tr -dc 'a-f0-9' < /dev/urandom | head -c 32)
dbpass=$(pwgen -B -c -y -n -r "\"\!\'\`\$@~#%^&*()+={[}]|:;<>?/" 12 1)

sudo -u postgres psql << EOF
DROP DATABASE IF EXISTS lobechatdb;
CREATE DATABASE lobechatdb;
DROP USER IF EXISTS lobechatuser;
CREATE USER lobechatuser WITH ENCRYPTED PASSWORD '${dbpass}';
GRANT ALL PRIVILEGES ON DATABASE lobechatdb TO lobechatuser;
\c lobechatdb
CREATE EXTENSION vector;
\dx
\q
EOF

cat > .env << EOF
REDIS_URL=redis://localhost:6379
REDIS_PREFIX=lobechat
DATABASE_URL=postgres://lobechatuser:${dbpass}@127.0.0.1:5432/lobechatdb
KEY_VAULTS_SECRET=${KEY_VAULTS_SECRET}
EOF

export APP_URL=http://localhost:3210
sync
echo 3 > /proc/sys/vm/drop_caches
sed -i -e "s/NODE_OPTIONS=--max-old-space-size=6144/NODE_OPTIONS=--max-old-space-size=4096/" package.json
pnpm run build

rm -f .env
ln -sf /disk/admin/modules/lobechat/.env .env
