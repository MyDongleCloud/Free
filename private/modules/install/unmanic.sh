#!/bin/sh

cd /home/ai
/home/ai/rootfs/usr/local/modules/_core_/pip.sh -f /usr/local/modules/unmanic -s
echo "PATH before any modif: $PATH"
PATHOLD=$PATH
PATH=/usr/local/modules/unmanic/bin:$PATHOLD
export PATH=/usr/local/modules/unmanic/bin:$PATHOLD
echo "PATH new: $PATH python: `python --version`"
pip install unmanic
PATH=$PATHOLD
export PATH=$PATHOLD
echo "PATH restored: $PATH"
