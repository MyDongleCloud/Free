#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for prowlarr [-h -r]"
echo "h:	Print this usage and exit"
echo "r:	Reset"
exit 0
}

if [ "m`id -u`" = "m0" ]; then
	echo "You should not be root"
	exit 0
fi

RESET=0
while getopts hr opt
do
	case "$opt" in
		h) helper;;
		r) RESET=1;;
	esac
done

if [ $RESET != 1 ]; then
	exit 0
fi

echo "#Reset prowlarr##################"
CLOUDNAME=`cat /disk/admin/modules/_config_/_cloud_.json | jq -r ".info.name"`
PASSWD=$(pwgen -B -c -y -n -r "\"\!\'\`\$@~#%^&*()+={[}]|:;<>?/" 12 1)
APIKEY=$(tr -dc 'a-f0-9' < /dev/urandom | head -c 32)
systemctl stop prowlarr.service
rm -rf /disk/admin/modules/prowlarr
mkdir /disk/admin/modules/prowlarr
cat > /disk/admin/modules/prowlarr/config.xml << EOF
<Config>
	<BindAddress>*</BindAddress>
	<Port>9696</Port>
	<SslPort>9898</SslPort>
	<EnableSsl>False</EnableSsl>
	<LaunchBrowser>True</LaunchBrowser>
	<ApiKey>${APIKEY}</ApiKey>
	<AuthenticationMethod>None</AuthenticationMethod>
	<AuthenticationRequired>Enabled</AuthenticationRequired>
	<Branch>master</Branch>
	<LogLevel>debug</LogLevel>
	<SslCertPath></SslCertPath>
	<SslCertPassword></SslCertPassword>
	<UrlBase></UrlBase>
	<InstanceName>Prowlarr</InstanceName>
</Config>
EOF
echo "{\"username\":\"${CLOUDNAME}\", \"password\":\"${PASSWD}\", \"apikey\":\"${APIKEY}\"}" > /disk/admin/modules/_config_/prowlarr.json
systemctl start prowlarr.service
systemctl enable prowlarr.service
