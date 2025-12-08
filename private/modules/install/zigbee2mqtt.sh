#!/bin/sh

mkdir /usr/local/modules/zigbee2mqtt
cd /usr/local/modules/zigbee2mqtt
npm install zigbee2mqtt@2.5.1
rm -rf /usr/local/modules/zigbee2mqtt/node_modules/zigbee2mqtt/data
ln -sf /etc/systemd/system/zigbee2mqtt.service /etc/systemd/system/multi-user.target.wants/zigbee2mqtt.service
