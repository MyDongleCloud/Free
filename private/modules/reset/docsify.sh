#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for docsify [-h]"
echo "h:	Print this usage and exit"
exit 0
}

if [ "m`id -u`" = "m0" ]; then
	echo "You should not be root"
	exit 0
fi

RESET=0
while getopts h opt
do
	case "$opt" in
		h) helper;;
	esac
done

if [ $RESET != 1 ]; then
	exit 0
fi

echo "#Reset docsify##################"
CLOUDNAME=`cat /disk/admin/modules/_config_/_cloud_.json | jq -r ".info.name"`
rm -rf /disk/admin/modules/docsify
mkdir -p /disk/admin/modules/docsify

ln -sf /usr/local/modules/docsify/lib/docsify.min.js /disk/admin/modules/docsify/
ln -sf /usr/local/modules/docsify/lib/plugins /disk/admin/modules/docsify/
ln -sf /usr/local/modules/docsify/lib/themes /disk/admin/modules/docsify/

cat > /disk/admin/modules/docsify/_sidebar.md <<EOF
- [Home](home)
EOF

cat > /disk/admin/modules/docsify/home.md <<EOF
# Docs of $CLOUDNAME

You can edit this page in the markdown file "/disk/admin/modules/docsify/home.md".

> [!HELPER]
> Read the [docsify documentation](https://docsify.js.org) for more information how to tune a docsify website.
EOF

cat > /disk/admin/modules/docsify/index.html <<EOF
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1, minimum-scale=1.0, shrink-to-fit=no, viewport-fit=cover">
<title>Docs of $CLOUDNAME</title>
<meta name="description" content="Docs of $CLOUDNAME">
<link rel="stylesheet" href="themes/vue.css">
</head>
<body>
<div id="app"></div>
<script>
window.\$docsify = {
	name: 'Docs of $CLOUDNAME',
	homepage: 'home.md',
	auto2top: true,
	loadSidebar: true,
	maxLevel: 0,
	subMaxLevel: 3,
	search: {
		placeholder: 'Type to search',
		noData: 'No matches found.',
		depth: 2,
	}
};
</script>
<script src="docsify.min.js"></script>
<script src="plugins/search.min.js"></script>
</body>
</html>
EOF

echo -n "{ \"a\":\"status\", \"module\":\"$(basename $0 .sh)\", \"state\":\"finish\" }" | nc -w 1 localhost 8093