#!/bin/sh

cat $PP/preseed_postfix.cfg | debconf-set-selections
apt-get -y install postfix swaks s-nail
