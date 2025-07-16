#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for pip.sh [-f folder -h -l -s]"
echo "f fold:	Set folder"
echo "h:	Print this usage and exit"
echo "l:	Local"
echo "s:	Setup"
exit 0
}

SETUP=0
VERSION=3.11
PYTHON=python$VERSION
LOCAL=0
FOLDER=/tmp/test
while getopts f:hls opt; do
	case "$opt" in
		f) FOLDER="${OPTARG}";;
		h) helper;;
		l) LOCAL=1;;
		s) SETUP=1;;
	esac
done

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
