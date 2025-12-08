#!/bin/sh

apt-get -y install redis-server
redis-cli ping
