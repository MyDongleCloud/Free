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
	rm -rf node_modules /disk/admin/.modules/MyDongleCloud/jwk.pub /disk/admin/.modules/BetterAuth/
	mkdir /disk/admin/.modules/BetterAuth/
	npm install
	sed -i -e 's/PRODUCTION=false/PRODUCTION=true/' .env
	npx @better-auth/cli migrate -y
	rm -f /disk/admin/.modules/BetterAuth/secret.txt
	npm run build
	rm -rf /usr/local/modules/BetterAuth
	cp -a BetterAuth /usr/local/modules
	cp -a node_modules /usr/local/modules/BetterAuth
	exit 0
elif [ $CLEAN = 1 ]; then
	#On PC only
	rm -rf node_modules ../rootfs/disk/admin/.modules/MyDongleCloud/jwk.pub ../rootfs/disk/admin/.modules/BetterAuth/
	exit 0
else
	#On PC only
	rm -f ../rootfs/disk/admin/.modules/MyDongleCloud/jwk.pub ../rootfs/disk/admin/.modules/BetterAuth/
	mkdir ../rootfs/disk/admin/.modules/BetterAuth/
	npm install
	npx @better-auth/cli migrate -y
	npm run dev
fi
