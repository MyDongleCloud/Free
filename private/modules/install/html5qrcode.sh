#!/bin/sh

cd /usr/local/modules/html5qrcode
npm ci
npm run build
cp dist/html5-qrcode.min.js examples/html5
