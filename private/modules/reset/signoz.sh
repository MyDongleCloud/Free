#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for signoz [-h]"
echo "h:	Print this usage and exit"
exit 0
}

if [ "m`id -u`" = "m0" ]; then
	echo "You should not be root"
	exit 0
fi

while getopts h opt
do
	case "$opt" in
		h) helper;;
	esac
done

echo "#Reset signoz##################"
CLOUDNAME=`cat /disk/admin/modules/_config_/_cloud_.json | jq -r ".info.name"`
EMAIL="admin@$CLOUDNAME.mydongle.cloud"
PASSWD=$(pwgen -B -c -y -n -r "\"\!\'\`\$@~#%^&*()+={[}]|:;<>?/" 12 1)
dbpass=$(pwgen -B -c -y -n -r "\"\!\'\`\$@~#%^&*()+={[}]|:;<>?/" 12 1)

systemctl stop signoz.service
rm -rf /disk/admin/modules/signoz
mkdir /disk/admin/modules/signoz
cat > /disk/admin/modules/signoz/config.yaml << EOF
instrumentation:
  logs:
    level: info
  traces:
    enabled: false
    processors:
      batch:
        exporter:
          otlp:
            endpoint: localhost:4317
  metrics:
    enabled: true
    readers:
      pull:
        exporter:
          prometheus:
            host: "127.0.0.1"
            port: 8113

web:
  enabled: true
  prefix: /
  directory: /usr/local/modules/signoz/web

sqlstore:
  provider: sqlite
  max_open_conns: 100
  sqlite:
    path: /disk/admin/modules/signoz/signoz.db
    mode: delete
    busy_timeout: 10s

alertmanager:
  signoz:
    external_url: https://$CLOUDNAME.mydongle.cloud
    global:
      smtp_from: $EMAIL
      smtp_smarthost: localhost:465
      smtp_require_tls: false
      smtp_tls_config:
        insecure_skip_verify: true

EOF
systemctl start signoz.service
systemctl enable signoz.service
echo "{\"email\":\"${EMAIL}\", \"password\":\"${PASSWD}\"}" > /disk/admin/modules/_config_/signoz.json

echo {" \"a\":\"status\", \"module\":\"$(basename $0 .sh)\", \"state\":\"finish\" }" | websocat -1 ws://localhost:8094
