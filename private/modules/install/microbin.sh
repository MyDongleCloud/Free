#!/bin/sh

cd /usr/local/modules/microbin
cargo build --release
cp target/release/microbin /usr/local/bin/
