#!/bin/sh

apt-get -y install cockpit network-manager-openvpn
rm -f /etc/issue.d/cockpit.issue /run/cockpit/active.issue /usr/share/cockpit/issue/update-issue
cat > /etc/cockpit/cockpit.conf << EOF
[WebService]
ProtocolHeader = X-Forwarded-Proto
AllowUnencrypted = true
EOF
