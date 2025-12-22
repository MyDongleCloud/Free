#!/bin/sh

cd /usr/local/modules/limesurvey
npm install
composer -n install
mv upload upload.bak
ln -sf /disk/admin/modules/limesurvey/config.php application/config/
ln -sf /disk/admin/modules/limesurvey/security.php application/config/
ln -sf /disk/admin/modules/limesurvey/upload
chown -R www-data:www-data tmp
chown -R www-data:www-data application/config
