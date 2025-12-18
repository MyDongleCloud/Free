#!/bin/sh

cd /usr/local/modules/piped
npm install
npm run build
find dist -type f -exec sed -i -e 's@https://pipedapi.kavin.rocks@" + window.location.origin + "/_api_@g'  {} \;
find dist -type f -exec sed -i -e 's@https://pipedproxy.kavin.rocks@" + window.location.origin + "/_proxy_@g' {} \;

cd pipedbackend
echo 3 > /proc/sys/vm/drop_caches
./gradlew shadowJar

cd ../pipedproxy
cargo build --release
cp /usr/local/modules/piped/pipedproxy/target/release/piped-proxy /usr/local/bin
#rm -rf /usr/local/modules/tabby/target/
