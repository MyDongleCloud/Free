#!/bin/sh


mkdir -p /etc/pihole
cat > /etc/pihole/setupVars.conf << EOF
PIHOLE_INTERFACE=eth0
IPV4_ADDRESS=192.168.10.8/24
IPV6_ADDRESS=
PIHOLE_DNS_1=8.8.8.8
PIHOLE_DNS_2=8.8.4.4
QUERY_LOGGING=true
PRIVACY_LEVEL=0
EOF
/usr/local/modules/pihole/automated\ install/basic-install.sh --unattended
sleep 10
echo "" | pihole setpassword
sed -i -e 's@  port = "80o.*@  port = "8110"@' /etc/pihole/pihole.toml
sed -i -e 's@    webroot =.*@    webroot = "/usr/local/modules/pihole/admin"@' /etc/pihole/pihole.toml
sed -i -e 's@    webhome =.*@    webhome = "/"@' /etc/pihole/pihole.toml
sleep 2
systemctl stop pihole-FTL.service
mv /etc/pihole/pihole.toml /etc/pihole/pihole.toml.bak
ln -sf /disk/admin/modules/pihole/pihole.toml /etc/pihole/
