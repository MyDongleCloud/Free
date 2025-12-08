#!/bin/sh

cd /usr/local/modules/mantisbugtracker
composer -n install
mv config config.bak
ln -sf /disk/admin/modules/mantisbugtracker/config
