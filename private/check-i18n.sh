#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for check-i18n [-h]"
echo "h:	Print this usage and exit"
exit 0
}

while getopts h opt; do
	case "$opt" in
		h) helper;;
	esac
done

if [ "m`id -u`" = "m0" ]; then
	echo "You should not be root"
	exit 0
fi

process() {
	if [ -z $1 ]; then
		PAGE="home"
	else
		PAGE=$1
	fi
	grep -q "$2" client/src/app/$PAGE/$PAGE.page.html
	if [ $? != 0 ]; then
		echo "Doesn't exist in page $PAGE: $2"
	fi
}

echo "#################################"
echo "pages-en.json vs. *.page.html"
echo "#################################"
eval "$(jq -r 'to_entries[] | .key as $p | .value | to_entries[] | "process \($p|@sh) \(.key|@sh)"' client/src/assets/i18n/pages-en.json)"

compare() {
	jq -n '
		[input | to_entries[]] as $f1 |
		[input | to_entries[]] as $f2 |
		($f1 + $f2 | map(.key) | unique[]) as $top_key |
		($f1 | map(select(.key == $top_key) | .value | keys) | add // []) as $k1 |
		($f2 | map(select(.key == $top_key) | .value | keys) | add // []) as $k2 |
		(($k1 - $k2) + ($k2 - $k1))[] | "\($top_key): \(.)"
	' "$1" "$2" -r
}

echo "#################################"
echo "pages-en.json vs. pages-fr.json"
echo "#################################"
compare client/src/assets/i18n/pages-en.json client/src/assets/i18n/pages-fr.json

echo "#################################"
echo "global-en.json vs. global-fr.json"
echo "#################################"
compare client/src/assets/i18n/global-en.json client/src/assets/i18n/global-fr.json
