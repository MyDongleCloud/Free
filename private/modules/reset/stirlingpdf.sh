#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for stirlingpdf [-h]"
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

echo "#Reset stirlingpdf##################"
systemctl stop stirlingpdf.service
rm -rf /disk/admin/modules/stirlingpdf
mkdir -p /disk/admin/modules/stirlingpdf/configs
cat > /disk/admin/modules/stirlingpdf/configs/custom_settings.yml << EOF
server:
  address: 127.0.0.1
  port: 8105

system:
  enableAnalytics: false

customPaths:
  pipeline:
    watchedFoldersDir: "/disk/admin/modules/stirlingpdf/documents"
    finishedFoldersDir: "/disk/admin/modules/stirlingpdf/documents"
EOF
systemctl start stirlingpdf.service
systemctl enable stirlingpdf.service

echo {" \"a\":\"status\", \"module\":\"$(basename $0 .sh)\", \"state\":\"finish\" }" | websocat -1 ws://localhost:8094
