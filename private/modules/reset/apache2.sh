#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for apache2 [-h]"
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

echo "#Reset apache2##################"
rm -rf /disk/admin/modules/apache2/www
mkdir -p /disk/admin/modules/apache2/www
cat > /disk/admin/modules/apache2/www/index.html <<EOF
<html>
<body>
You can edit this home page in the folder: /disk/admin/modules/apache2/www<br><br>
Go to the main <a href="/m/app">App</a> of the system
</body>
</html>
EOF

echo -n "{ \"a\":\"status\", \"module\":\"$(basename $0 .sh)\", \"state\":\"finish\" }" | nc -w 1 localhost 8093
