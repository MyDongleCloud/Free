#!/bin/sh

cd /home/ai/build
wget -nv https://github.com/gravitl/netmaker/releases/download/v1.4.0/nmctl-linux-arm64
chmod a+x nmctl-linux-arm64
mv nmctl-linux-arm64 /usr/local/bin/nmctl
