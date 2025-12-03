#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for clone [-h]"
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

cd `dirname $0`
echo "Current directory is now `pwd`"
PP=`pwd`

rm -rf /tmp/clonage
mkdir /tmp/clonage
tar -xjpf img/clone.tbz2 -C /tmp/clonage
cd /tmp/clonage
echo "Do modif now and press return to finish script"
read l
echo "Finishing"
rm -f $PP/img/clone.tbz2
tar -jcpf $PP/img/clone.tbz2 .
cd $PP
rm -rf /tmp/clonage
