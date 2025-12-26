#!/bin/sh

cd /usr/local/modules/typesensedashboard
npm install
./node_modules/.bin/quasar prepare
PUBLIC_PATH=./ ./node_modules/.bin/quasar build
ln -sf /disk/admin/modules/typesensedashboard/config.json /usr/local/modules/typesensedashboard/dist/spa/
