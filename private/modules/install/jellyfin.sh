#!/bin/sh

cd /home/ai/build
wget -nv -O jellyfin.sh https://repo.jellyfin.org/install-debuntu.sh
SKIP_CONFIRM=1 bash jellyfin.sh
systemctl disable jellyfin
mv /etc/jellyfin /etc/jellyfin.bak
ln -sf /disk/admin/modules/jellyfin/config /etc/jellyfin
mv /var/lib/jellyfin /var/lib/jellyfin.bak
ln -sf /disk/admin/modules/jellyfin/data /var/lib/jellyfin
