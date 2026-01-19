#!/bin/sh

mkdir -p /etc/pihole
cat > /etc/pihole/setupVars.conf << EOF
PIHOLE_INTERFACE=eth0
IPV4_ADDRESS=192.168.10.5/24
IPV6_ADDRESS=
PIHOLE_DNS_1=8.8.8.8
PIHOLE_DNS_2=8.8.4.4
QUERY_LOGGING=true
PRIVACY_LEVEL=0
EOF
/usr/local/modules/pihole/automated\ install/basic-install.sh --unattended
sleep 5
echo "" | pihole setpassword
sed -i -e 's@^  port = ".*@  port = "8110"@' /etc/pihole/pihole.toml
sed -i -e 's@^	webroot =.*@	webroot = "/usr/local/modules/pihole/admin"@' /etc/pihole/pihole.toml
sed -i -e 's@^	webhome =.*@	webhome = "/"@' /etc/pihole/pihole.toml
sleep 1
systemctl stop pihole-FTL.service
systemctl disable pihole-FTL.service
mkdir /usr/local/modules/pihole
cp /etc/pihole/pihole.toml /usr/local/modules/pihole/pihole.toml
mv /var/www/html/admin/ /usr/local/modules/pihole/
