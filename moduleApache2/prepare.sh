#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for prepare [-h -m]"
echo "h:	Print this usage and exit"
echo "m:	Minify"
exit 0
}

MINIFY=0
while getopts hm opt; do
	case "$opt" in
		h) helper;;
		m) MINIFY=1;;
	esac
done

if [ $MINIFY = 1 ]; then
	JS=$(cat login.js | sed 's/\&/\\&/g')
	JS=$(echo $JS | tr '\n' ' ')
	SEARCH=$(grep -oE '\<_app_[a-zA-Z0-9_]*\>' login.js | sort -u | tr '\n' ' ')
	for ss in $SEARCH; do
		rr=$(tr -dc 'A-Za-z' < /dev/urandom | head -c 16)
		JS=$(echo $JS | sed "s|$ss|$rr|g")
	done
	JS=$(echo $JS | sed 's/"/\\\\"/g')
	sed "/#define INJECTION/d" login.c | sed "s|</script>\"|#define INJECTION \"<script type='module'>${JS}</script>\"|" > login_.c
else
	sed 's/$/\\n\\/g' login.js | sed 's/"/\\"/g' > login.js.tmp
	sed "/<script type='module'>/ {
		r login.js.tmp
	}" login.c > login_.c
	rm -f login.js.tmp
fi
