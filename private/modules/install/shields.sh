#!/bin/sh

cd /usr/local/modules/shields
npm ci
sync
echo 3 > /proc/sys/vm/drop_caches
NODE_OPTIONS="--max-old-space-size=2048" npm run build
