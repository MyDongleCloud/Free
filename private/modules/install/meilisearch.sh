#!/bin/sh

cd /home/ai/build
curl -L -sS --fail https://install.meilisearch.com | sh
chmod a+x meilisearch
mv meilisearch /usr/local/bin/
