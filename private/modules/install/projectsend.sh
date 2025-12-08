#!/bin/sh

cd /usr/local/modules/projectsend
composer -n install
npm install gulp-cli
./node_modules/.bin/gulp build
rm -rf node_modules
ln -sf /disk/admin/modules/projectsend/sys.config.php includes/sys.config.php
mv upload/files upload/files.bak
ln -sf /disk/admin/modules/projectsend/files upload/
mv upload/temp upload/temp.bak
ln -sf /disk/admin/modules/projectsend/temp upload/
rm -rf cache
ln -sf /disk/admin/modules/projectsend/cache
