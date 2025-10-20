#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for setup [-h]"
echo "h:		Print this usage and exit"
exit 0
}

if [ "m`id -u`" != "m0" ]; then
	echo "You need to be root"
	exit 0
fi

while getopts h opt
do
	case "$opt" in
		h) helper;;
	esac
done

if [ $1 = "postfix" ]; then
	/usr/local/modules/mydonglecloud/scripts/postfix.sh -r
elif [ $1 = "mysql" ]; then
	/usr/local/modules/mydonglecloud/scripts/mysql.sh -r
elif [ $1 = "bugzilla" ]; then
	/usr/local/modules/mydonglecloud/scripts/bugzilla.sh -r
elif [ $1 = "jitsi" ]; then
	/usr/local/modules/mydonglecloud/scripts/jitsi.sh -r
elif [ $1 = "mantisbugtracker" ]; then
	su admin -c "/usr/local/modules/mydonglecloud/scripts/mantisbugtracker.sh -r"
elif [ $1 = "osticket" ]; then
	su admin -c "/usr/local/modules/mydonglecloud/scripts/osticket.sh -r"
elif [ $1 = "projectsend" ]; then
	/usr/local/modules/mydonglecloud/scripts/projectsend.sh -r
elif [ $1 = "roundcube" ]; then
	su admin -c "/usr/local/modules/mydonglecloud/scripts/roundcube.sh -r"
elif [ $1 = "yourls" ]; then
	su admin -c "/usr/local/modules/mydonglecloud/scripts/yourls.sh -r"
elif [ $1 = "webtrees" ]; then
	/usr/local/modules/mydonglecloud/scripts/webtrees.sh -r
fi
