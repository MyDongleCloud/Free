#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for sonarr [-h -r]"
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

echo "#Reset sonarr##################"
CLOUDNAME=`cat /disk/admin/modules/_config_/_cloud_.json | jq -r ".info.name"`
PASSWD=$(pwgen -B -c -y -n -r "\"\!\'\`\$@~#%^&*()+={[}]|:;<>?/" 12 1)
APIKEY=$(tr -dc 'a-f0-9' < /dev/urandom | head -c 32)
systemctl stop sonarr.service
rm -rf /disk/admin/modules/sonarr
mkdir /disk/admin/modules/sonarr
cat > /disk/admin/modules/sonarr/config.xml << EOF
<Config>
	<BindAddress>*</BindAddress>
	<Port>8989</Port>
	<SslPort>9898</SslPort>
	<EnableSsl>False</EnableSsl>
	<LaunchBrowser>True</LaunchBrowser>
	<ApiKey>${APIKEY}</ApiKey>
	<AuthenticationMethod>None</AuthenticationMethod>
	<AuthenticationRequired>Enabled</AuthenticationRequired>
	<Branch>main</Branch>
	<LogLevel>debug</LogLevel>
	<SslCertPath></SslCertPath>
	<SslCertPassword></SslCertPassword>
	<UrlBase></UrlBase>
	<InstanceName>Sonarr</InstanceName>
</Config>
EOF
echo "{\"username\":\"${CLOUDNAME}\", \"password\":\"${PASSWD}\", \"apikey\":\"${APIKEY}\"}" > /disk/admin/modules/_config_/sonarr.json
systemctl start sonarr.service
systemctl enable sonarr.service
