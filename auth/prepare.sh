#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for run [-c -h -i]"
echo "c:	Clean on PC"
echo "h:	Print this usage and exit"
echo "i:	Install during install"
exit 0
}

CLEAN=0
INSTALL=0
while getopts chi opt; do
	case "$opt" in
		c) CLEAN=1;;
		h) helper;;
		i) INSTALL=1;;
	esac
done

cd `dirname $0`
if [ $INSTALL = 1 ]; then
	#During install only
	rm -rf node_modules betterauth /disk/admin/.modules/betterauth
	mkdir /disk/admin/.modules/betterauth/
	npm install
	sed -i -e 's/PRODUCTION=false/PRODUCTION=true/' .env
	npx @better-auth/cli migrate -y
	rm -f /disk/admin/.modules/betterauth/secret.txt
	chown -R admin:admin /disk/admin/.modules/betterauth
	npm run build
	rm -rf /usr/local/modules/betterauth
	cp -a betterauth /usr/local/modules
	cp -a node_modules /usr/local/modules/betterauth
	exit 0
elif [ $CLEAN = 1 ]; then
	#On PC only
	rm -rf node_modules betterauth ../rootfs/disk/admin/.modules/betterauth
	exit 0
else
	#On PC only
	if [ ! -d ../rootfs/disk/admin/.modules/betterauth/ -o ! -d node_modules ]; then
		rm -rf node_modules ../rootfs/disk/admin/.modules/betterauth
		mkdir -p ../rootfs/disk/admin/.modules/betterauth
		npm install
		npx @better-auth/cli migrate -y
		(sleep 3 && ./test.sh -c) &
	fi
	npm run dev
fi
