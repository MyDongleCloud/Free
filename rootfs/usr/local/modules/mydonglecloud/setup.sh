#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for setup [-a -h] | [-u u -r] name"
echo "a:	do All"
echo "h:	Print this usage and exit"
echo "u u:	do as user (0:admin, 1:root, -1:read from json (default)"
echo "r:	do reset"
exit 0
}

if [ "m`id -u`" != "m0" ]; then
	echo "You need to be root"
	exit 0
fi

ALL=0
RESET=0
USER=-1
while getopts ahu:r opt
do
	case "$opt" in
		a) ALL=1;RESET=1;;
		h) helper;;
		u) USER=$OPTARG;;
		r) RESET=1;;
	esac
done
PP=`dirname $0`

module() {
	if [ $2 = 1 ]; then
		$PP/scripts/$1.sh -r
	else
		su admin -c "$PP/scripts/$1.sh -r"
	fi
	COUNT=$((COUNT + 1))
	P=$(($COUNT * 100 / $TOTAL))
	echo "{ \"a\":\"setup-status\", \"p\":$P, \"n\":\"$1\" }" | nc -w 1 localhost 8093
}
if [ $ALL = 1 ]; then
	TOTAL=-1
	COUNT=0
	jq -r '[to_entries[] | select(.value | has("setup")) | {priority: (.value.setupPriority // 0), key: .key, root: (if .value.setupRoot then 1 else 0 end)}] | "\(length)", (sort_by(.priority)[] | "\(.key) \(.root)")' "$PP/modulesdefault.json" | while read l; do
		if [ $TOTAL = -1 ]; then
			TOTAL=$l
		else
			module $l
		fi
	done
	exit 0
fi

shift $((OPTIND -1))
NAME=$1
if [ $USER = -1 ]; then
	USER=`jq -r ".$NAME.resetRoot" "$PP/modulesdefault.json"`
	if [ $USER = "true" ]; then
		USER=1
	else
		USER=0
	fi
fi
if [ ! -f $PP/scripts/$NAME.sh ]; then
	echo "#Doing nothing for $NAME##################"
else
	ARG=""
	if [ $RESET = 1 ]; then
		ARG="-r"
	fi
	if [ $USER = 1 ]; then
		$PP/scripts/$NAME.sh $ARG
	else
		su admin -c "$PP/scripts/$NAME.sh $ARG"
	fi
fi
