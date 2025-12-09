#!/bin/sh

apt-get -y install cockpit
rm /etc/issue.d/cockpit.issue
rm /usr/share/cockpit/issue/update-issue
