#!/bin/sh

cd /usr/local/modules/webtrees
composer -n install
mv data data.bak
ln -sf /disk/admin/modules/webtrees/data
