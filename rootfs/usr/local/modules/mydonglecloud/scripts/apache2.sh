#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for apache2 [-h -r]"
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

echo "#Reset apache2##################"
rm -rf /disk/admin/.modules/apache2/www
mkdir -p /disk/admin/.modules/apache2/www
cat > /disk/admin/.modules/apache2/www/index.html <<EOF
<html>
<body>
You can edit this file in the folder: /disk/admin/.modules/apache2/www<br><br>
Go to <a href="/m/app">App</a>
</body>
</html>
EOF
