#!/bin/sh

apt-get -y install cockpit
rm -f /etc/issue.d/cockpit.issue /run/cockpit/active.issue /usr/share/cockpit/issue/update-issue
