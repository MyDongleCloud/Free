#!/bin/sh

cd /home/ai/build
wget -nv https://dl.deno.land/release/v2.6.4/deno-aarch64-unknown-linux-gnu.zip
unzip deno*.zip
mv deno /usr/local/bin
