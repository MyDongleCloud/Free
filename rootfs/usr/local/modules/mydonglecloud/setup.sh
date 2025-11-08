#!/bin/sh

helper() {
echo "*******************************************************"
echo "Usage for setup [-h -a]"
echo "h:		Print this usage and exit"
exit 0
}

if [ "m`id -u`" != "m0" ]; then
	echo "You need to be root"
	exit 0
fi

ALL=0
while getopts ha opt
do
	case "$opt" in
		a) ALL=1;;
		h) helper;;
	esac
done

module() {
	if [ $2 = 1 ]; then
		su admin -c "/usr/local/modules/mydonglecloud/scripts/$1.sh -r"
	else
		/usr/local/modules/mydonglecloud/scripts/$1.sh -r
	fi
	COUNT=$((COUNT + 1))
	P=$(($COUNT * 100 / $TOTAL))
	echo "{ \"a\":\"setup-status\", \"p\":$P, \"n\":\"$1\" }" | nc -w 1 localhost 8093
}

if [ $ALL = 1 ]; then
	TOTAL=`jq '[.[] | select(has("setup"))] | length' /usr/local/modules/mydonglecloud/modulesdefault.json`
	COUNT=0
	module meilisearch 1
	module mongodb 0
	module mysql 0
	module postgresql 0
	module apache2 1
	module bugzilla 0
	module homeassistant 1
	module jitsimeet 0
	module librechat 1
	module mantisbugtracker 1
	module metube 1
	module osticket 1
	module postfix 0
	module privatebin 0
	module projectsend 0
	module roundcube 1
	module tubesync 1
	module webtrees 0
	module yourls 1
	exit 0
fi

if [ $1 = "apache2" ]; then
	su admin -c "/usr/local/modules/mydonglecloud/scripts/apache2.sh -r"
elif [ $1 = "bugzilla" ]; then
	/usr/local/modules/mydonglecloud/scripts/bugzilla.sh -r
elif [ $1 = "homeassistant" ]; then
	su admin -c "/usr/local/modules/mydonglecloud/scripts/homeassistant.sh -r"
elif [ $1 = "jitsimeet" ]; then
	/usr/local/modules/mydonglecloud/scripts/jitsimeet.sh -r
elif [ $1 = "librechat" ]; then
	su admin -c "/usr/local/modules/mydonglecloud/scripts/librechat.sh -r"
elif [ $1 = "mantisbugtracker" ]; then
	su admin -c "/usr/local/modules/mydonglecloud/scripts/mantisbugtracker.sh -r"
elif [ $1 = "meilisearch" ]; then
	su admin -c "/usr/local/modules/mydonglecloud/scripts/meilisearch.sh -r"
elif [ $1 = "metube" ]; then
	su admin -c "/usr/local/modules/mydonglecloud/scripts/metube.sh -r"
elif [ $1 = "mongodb" ]; then
	/usr/local/modules/mydonglecloud/scripts/mongodb.sh -r
elif [ $1 = "mysql" ]; then
	/usr/local/modules/mydonglecloud/scripts/mysql.sh -r
elif [ $1 = "osticket" ]; then
	su admin -c "/usr/local/modules/mydonglecloud/scripts/osticket.sh -r"
elif [ $1 = "postfix" ]; then
	/usr/local/modules/mydonglecloud/scripts/postfix.sh -r
elif [ $1 = "postgresql" ]; then
	/usr/local/modules/mydonglecloud/scripts/postgresql.sh -r
elif [ $1 = "privatebin" ]; then
	/usr/local/modules/mydonglecloud/scripts/privatebin.sh -r
elif [ $1 = "projectsend" ]; then
	/usr/local/modules/mydonglecloud/scripts/projectsend.sh -r
elif [ $1 = "roundcube" ]; then
	su admin -c "/usr/local/modules/mydonglecloud/scripts/roundcube.sh -r"
elif [ $1 = "tubesync" ]; then
	su admin -c "/usr/local/modules/mydonglecloud/scripts/tubesync.sh -r"
elif [ $1 = "webtrees" ]; then
	/usr/local/modules/mydonglecloud/scripts/webtrees.sh -r
elif [ $1 = "yourls" ]; then
	su admin -c "/usr/local/modules/mydonglecloud/scripts/yourls.sh -r"
else
	echo "#Doing nothing for $1##################"
fi
