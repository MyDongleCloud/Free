#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for jitsi [-h -r] spacename"
echo "h:		Print this usage and exit"
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
shift `expr $OPTIND - 1`

echo "#Reset Jitsi##################"
#find /etc -exec sed -i -e "s/m_unique_d_unique_c/$1/" {} \;
