#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage of burn.sh [-e -f -h -s]"
echo "e:	Erase"
echo "f:	Flash"
echo "h:	Print this usage and exit"
echo "s:	Setup"
exit 0
}

PP=/dev/ttyUSB0
BASE=/tmp/test
if [ -f /dev/ttyAMA0 ]; then
	BASE=/home/mdc/build
	PP=/dev/ttyAMA0
fi
MCU=attiny202
MEDIUM=uart
ACTION=0
SETUP=0
while getopts efhs opt; do
	case "$opt" in
		e) ACTION=1;;
		f) ACTION=2;;
		h) helper;;
		s) SETUP=1;;
	esac
done

if [ $SETUP = 1 ]; then
	~/Executables/pip.sh -s
	PATH=$BASE/bin:$PATH
	pip install pymcuprog
	exit 0
fi
PATH=$BASE/bin:$PATH

if [ $ACTION = 0 ]; then
	pymcuprog ping -t $MEDIUM -u $PP -d $MCU
fi
if [ $ACTION = 1 ]; then
	pymcuprog erase -t $MEDIUM -u $PP -d $MCU
fi
if [ $ACTION = 2 ]; then
	make
	pymcuprog write -t $MEDIUM -u $PP -d $MCU -f main.hex --erase
fi
