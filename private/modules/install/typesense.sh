#!/bin/sh

cd /home/ai/build
wget -nv https://dl.typesense.org/releases/29.0/typesense-server-29.0-arm64-lg-page16.deb
dpkg -i typesense-server*.deb
