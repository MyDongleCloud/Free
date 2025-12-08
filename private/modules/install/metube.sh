#!/bin/sh

cd /usr/local/modules/metube
/home/ai/rootfs/usr/local/modules/mydonglecloud/pip.sh -f /usr/local/modules/metube/env -s
echo "PATH before any modif: $PATH"
PATHOLD=$PATH
PATH=/usr/local/modules/metube/env/bin:$PATHOLD
export PATH=/usr/local/modules/metube/env/bin:$PATHOLD
echo "PATH new: $PATH python: `python --version`"
pip install aiohttp yt_dlp mutagen curl-cffi watchfiles python-socketio
PATH=$PATHOLD
export PATH=$PATHOLD
echo "PATH restored: $PATH"
cd ui
npm ci
yes no | node_modules/.bin/ng build --configuration production
mkdir /disk/admin/modules/metube
