#!/bin/sh

apt-get -y install protobuf-compiler libopenblas-dev graphviz
cd /usr/local/modules/tabby
echo 3 > /proc/sys/vm/drop_caches
cargo build --release
cp /usr/local/modules/tabby/target/release/llama-server /usr/local/bin
cp /usr/local/modules/tabby/target/release/tabby /usr/local/bin
