#!/bin/sh

apt-get -y install protobuf-compiler libopenblas-dev graphviz
cd /usr/local/modules/tabby
echo 3 > /proc/sys/vm/drop_caches
cargo build --release
ln -sf /etc/systemd/system/tabby.service /etc/systemd/system/multi-user.target.wants/tabby.service
