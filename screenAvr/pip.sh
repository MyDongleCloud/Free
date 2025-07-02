#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for pip.sh [-h -l -s -v version]"
echo "h:	Print this usage and exit"
echo "l:	Local"
echo "s:	Setup"
echo "v:	Set version"
exit 0
}

SETUP=0
VERSION=3.11
LOCAL=0
while getopts hilsv: opt; do
	case "$opt" in
		h) helper;;
		l) LOCAL=1;;
		s) SETUP=1;;
		v) VERSION="${OPTARG}";;
	esac
done

PYTHON=python$VERSION
FOLDER=/home/mdc/build/py

if [ $SETUP = 1 ]; then
	echo "#####################################"
	echo "SETUP"
	echo "#####################################"
	rm -rf $FOLDER
	$PYTHON -m venv $FOLDER
	LOCAL=1
fi

if [ $LOCAL = 1 ]; then
	echo "#####################################"
	echo "LOCAL"
	echo "#####################################"
	echo "PATH=$FOLDER/bin:\$PATH"
	PATH=$FOLDER/bin:$PATH
	#export PATH=$FOLDER/bin:$PATH
fi
