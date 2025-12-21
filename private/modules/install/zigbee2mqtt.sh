#!/bin/sh

mkdir /usr/local/modules/zigbee2mqtt
cd /usr/local/modules/zigbee2mqtt
npm install zigbee2mqtt@2.5.1
rm -rf /usr/local/modules/zigbee2mqtt/node_modules/zigbee2mqtt/data
