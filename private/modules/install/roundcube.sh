#!/bin/sh

cat $PP/preseed_roundcube.cfg | debconf-set-selections
apt-get -y install roundcube
cp /etc/roundcube/config.inc.php /etc/roundcube/config.inc.php.template
chmod 666 /etc/roundcube/config.inc.php
chmod 644 /etc/roundcube/config.inc.php.template
ln -sf /usr/local/modules/onetimeemail/autologin.php /usr/share/roundcube
