#!/bin/sh

apt-get -y install fonts-open-sans libevent-dev libevent-extra-2.1-7t64 libevent-openssl-2.1-7t64 libreadline-dev librsvg2-bin libyaml-dev
cd /usr/local/modules
git clone https://github.com/iv-org/invidious
cd invidious
git checkout 5f84a5b
make
ln -sf /disk/admin/modules/invidious/config.yml /usr/local/modules/invidious/config/
mkdir /var/log/invidious/

git clone https://github.com/iv-org/invidious-companion
cd invidious-companion
git checkout 2668927
deno task compile
