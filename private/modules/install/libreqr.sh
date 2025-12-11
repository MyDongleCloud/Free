#!/bin/sh

cd /usr/local/modules
if [ ! -d libreqr ]; then
	git clone https://code.antopie.org/miraty/libreqr.git libreqr
	cd libreqr
	git checkout 2.0.1
	rm -rf .git
else
	cd libreqr
fi
composer -n install
chown -R www-data:www-data /usr/local/modules/libreqr/css
