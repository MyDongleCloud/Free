#!/bin/sh

cd /usr/local/modules/privatebin
composer -n install
ln -sf /disk/admin/modules/privatebin/conf.php cfg
