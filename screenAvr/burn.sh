#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage of burn.sh [-e -f -h]"
echo "e:	Erase"
echo "f:	Flash"
echo "h:	Print this usage and exit"
exit 0
}

PP=/dev/tty_attiny
if [ ! -e /dev/tty_attiny ]; then
	PP=/dev/ttyUSB0
fi
MCU=attiny202
MEDIUM=uart
ACTION=0
while getopts efh opt; do
	case "$opt" in
		e) ACTION=1;;
		f) ACTION=2;;
		h) helper;;
	esac
done

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
