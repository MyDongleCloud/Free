#!/bin/sh

apt-get -y install transmission-common transmission-daemon transmission-cli
sed -i -e "s|^User=.*|User=admin\nEnvironment=TRANSMISSION_HOME=/disk/admin/modules/transmission|" /usr/lib/systemd/system/transmission-daemon.service
systemctl disable transmission-daemon.service
