#!/bin/sh

cd /usr/local/modules/privatebin
composer -n install
ln -sf /disk/admin/modules/privatebin/conf.php cfg
sed -i '0,/basepath/s/^\(.*\basepath.*\)/\/\/ \1/; /basepath/,+5 s/^/\/\/ /' lib/Proxy/AbstractProxy.php
sed -i -e 's@class="alert alert-danger"@class="hidden alert alert-danger"@' tpl/bootstrap.php
sed -i -e 's@class="alert alert-danger"@class="hidden alert alert-danger"@' tpl/bootstrap5.php
