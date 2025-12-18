#!/bin/sh

cd /home/ai
/home/ai/rootfs/usr/local/modules/mydonglecloud/pip.sh -f /usr/local/modules/openwebui -v 3.11 -s
echo "PATH before any modif: $PATH"
PATHOLD=$PATH
PATH=/usr/local/modules/openwebui/bin:$PATHOLD
export PATH=/usr/local/modules/openwebui/bin:$PATHOLD
echo "PATH new: $PATH python: `python --version`"
pip install open-webui
PATH=$PATHOLD
export PATH=$PATHOLD
echo "PATH restored: $PATH"
#ln -sf /disk/admin/modules/openwebui/data /usr/local/modules/openwebui/lib/python3.11/site-packages/open_webui/
