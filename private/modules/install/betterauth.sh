#!/bin/sh

cd /home/ai/auth
./prepare.sh -i
ln -sf /etc/systemd/system/betterauth.service /etc/systemd/system/multi-user.target.wants/betterauth.service
ln -sf /etc/systemd/system/betterauth-studio.service /etc/systemd/system/multi-user.target.wants/betterauth-studio.service
