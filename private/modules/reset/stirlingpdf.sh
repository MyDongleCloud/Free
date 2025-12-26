#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for stirlingpdf [-h -r]"
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
