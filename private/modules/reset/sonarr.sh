#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for sonarr [-h -w]"
echo "h:	Print this usage and exit"
echo "w:	Wait for user creation"
exit 0
}

if [ "m`id -u`" = "m0" ]; then
	echo "You should not be root"
	exit 0
fi

RESET=0
WAIT=0
while getopts hrw opt
do
	case "$opt" in
		h) helper;;
		w) WAIT=1;;
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
mkdir -p /disk/admin/modules/sonarr/downloads
cat > /disk/admin/modules/sonarr/config.xml << EOF
<Config>
	<BindAddress>127.0.0.1</BindAddress>
	<Port>8989</Port>
	<SslPort>9898</SslPort>
	<EnableSsl>False</EnableSsl>
	<LaunchBrowser>True</LaunchBrowser>
	<ApiKey>${APIKEY}</ApiKey>
	<AuthenticationMethod>External</AuthenticationMethod>
	<AuthenticationRequired>DisabledForLocalAddresses</AuthenticationRequired>
	<Branch>main</Branch>
	<LogLevel>debug</LogLevel>
	<SslCertPath></SslCertPath>
	<SslCertPassword></SslCertPassword>
	<UrlBase></UrlBase>
	<InstanceName>Sonarr</InstanceName>
</Config>
EOF
echo "{\"apikey\":\"${APIKEY}\"}" > /disk/admin/modules/_config_/sonarr.json
systemctl start sonarr.service
systemctl enable sonarr.service
if [ $WAIT = 1 ]; then
	/usr/local/modules/mydonglecloud/reset/sonarr-user.sh
else
	/usr/local/modules/mydonglecloud/reset/sonarr-user.sh &
fi
