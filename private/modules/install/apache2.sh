#!/bin/sh

apt-get -y install apache2
rm -f /etc/apache2/sites-enabled/*
rm -f /etc/apache2/ports.conf
