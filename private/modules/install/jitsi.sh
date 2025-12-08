#!/bin/sh

curl -fsSL https://download.jitsi.org/jitsi-key.gpg.key | gpg --dearmor -o /etc/apt/keyrings/jitsi.gpg
echo "deb [arch=arm64 signed-by=/etc/apt/keyrings/jitsi.gpg] https://download.jitsi.org stable/" > /etc/apt/sources.list.d/jitsi.list
apt-get update
cat $PP/preseed_jitsi.cfg | debconf-set-selections
apt-get -y install jitsi-videobridge2 jitsi-meet-web-config jitsi-meet-web
#apt-get -y install jitsi-meet
