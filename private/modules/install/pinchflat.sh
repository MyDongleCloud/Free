#!/bin/sh

apt-get -y install elixir erlang-dev erlang-nox
cd /usr/local/modules/pinchflat
export MIX_ENV=prod
mix deps.get
yes | yarn --cwd assets install
mix assets.deploy
mix compile
mix release
apt-get -y install python3-argcomplete python3-psutil python3-userpath apprise
