#!/bin/sh

cd /usr/local/modules/joomla
composer -n install
npm ci
