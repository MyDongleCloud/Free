#!/bin/sh

cd /usr/local/modules/shields
npm ci
sync
echo 3 > /proc/sys/vm/drop_caches
npm run build
