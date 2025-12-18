#!/bin/sh

cat $PP/preseed_wireshark.cfg | debconf-set-selections
apt-get -y install tshark
