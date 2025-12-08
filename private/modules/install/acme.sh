#!/bin/sh

cd /usr/local/modules
mkdir /usr/local/modules/acme
cd acme
wget -nv https://raw.githubusercontent.com/acmesh-official/acme.sh/refs/tags/3.1.1/acme.sh
chmod a+x acme.sh
