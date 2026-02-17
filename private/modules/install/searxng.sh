#!/bin/sh

apt-get -y install python3-dev python3-babel python3-venv python-is-python3 uwsgi uwsgi-plugin-python3 git build-essential libxslt-dev zlib1g-dev libffi-dev libssl-dev python-is-python3

cd /usr/local/modules/searxng
/home/ai/rootfs/usr/local/modules/_core_/pip.sh -f /usr/local/modules/searxng/penv -s -v 3.11
echo "PATH before any modif: $PATH"
PATHOLD=$PATH
PATH=/usr/local/modules/searxng/penv/bin:$PATHOLD
export PATH=/usr/local/modules/searxng/penv/bin:$PATHOLD
echo "PATH new: $PATH python: `python --version`"
pip install -r requirements.txt
pip install --use-pep517 --no-build-isolation .
PATH=$PATHOLD
export PATH=$PATHOLD
echo "PATH restored: $PATH"
