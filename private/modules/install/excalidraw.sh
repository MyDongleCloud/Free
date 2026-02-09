#!/bin/sh

cd /usr/local/modules/excalidraw
yarn --non-interactive install
NODE_OPTIONS="--max-old-space-size=4096" yarn build
