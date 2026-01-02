#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for setup [-d v -h -j v -u v -r -R v] name"
echo "d v:	Do dependencies, 0:no 1:yes (default)"
echo "h:	Print this usage and exit"
echo "j v:	Update json, 0:no 1:yes (default)"
echo "u v:	Do as user, 0:admin, 1:root, -1:read from json (default)"
echo "r:	Do reset"
echo "R v:	Do reset, 0:no (default) 1:yes"
exit 0
}

if [ "m`id -u`" != "m0" ]; then
	echo "You need to be root"
	exit 0
fi

RESET=0
USER=-1
DEPENDENCIES=1
JSON=1
while getopts d:hj:u:rR: opt
do
	case "$opt" in
		d) DEPENDENCIES=$OPTARG;;
		h) helper;;
		j) JSON=$OPTARG;;
		u) USER=$OPTARG;;
		r) RESET=1;;
		R) RESET=$OPTARG;;
	esac
done

cd `dirname $0`
PP=`pwd`

MODULES=/disk/admin/modules/_config_/_modules_.json

shift $((OPTIND -1))
NAME=$1
if [ $DEPENDENCIES = 1 ]; then
	LIST2=`jq -r ".$NAME.setupDependencies | join(\" \")" $PP/modulesdefault.json 2> /dev/null`
	for tt in $LIST2; do
		ALREADYDONE=`jq -r ".$tt.setupDone" $MODULES 2> /dev/null`
		if [ "$ALREADYDONE" != "true" ]; then
			$PP/setup.sh -d $DEPENDENCIES -j $JSON -R $RESET $tt
		fi
	done
fi
if [ $USER = -1 ]; then
	USER=`jq -r ".$NAME.setupRoot | if . == true then 1 else 0 end" $PP/modulesdefault.json 2> /dev/null`
fi
if [ $RESET = 1 ]; then
	ARGr="-r"
fi
if [ ! -f $PP/reset/$NAME.sh ]; then
	echo "#Doing nothing for $NAME##################"
else
	if [ $USER = 1 ]; then
		$PP/reset/$NAME.sh $ARGr
	else
		su admin -c "$PP/reset/$NAME.sh $ARGr"
	fi
	if [ $JSON = 1 ]; then
		jq ".$NAME.setupDone = true" $MODULES > $MODULES.tmp && mv $MODULES.tmp $MODULES
		chown admin:admin $MODULES
	fi
fi
