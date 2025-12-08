#!/bin/sh

cd /usr/local/modules
git clone https://code.antopie.org/miraty/libreqr.git libreqr
cd libreqr
git checkout 2.0.1
rm -rf .git
composer -n install
chown -R www-data:www-data /usr/local/modules/libreqr/css
