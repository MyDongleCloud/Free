#!/bin/sh

apt-get -y install protobuf-compiler libopenblas-dev graphviz
cd /usr/local/modules/tabby
sync
echo 3 > /proc/sys/vm/drop_caches
cargo build --release
sync
echo 3 > /proc/sys/vm/drop_caches
cp /usr/local/modules/tabby/target/release/llama-server /usr/local/bin
cp /usr/local/modules/tabby/target/release/tabby /usr/local/bin
rm -rf /usr/local/modules/tabby/target/
