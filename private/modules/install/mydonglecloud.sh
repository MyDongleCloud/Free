#!/bin/sh

mkdir -p /usr/local/modules/mydonglecloud
cp ../rootfs/usr/local/modules/mydonglecloud/version.txt /usr/local/modules/mydonglecloud/version.txt
cd /home/ai/app
./lvgl.sh -b -c
make
ln -sf /etc/systemd/system/dongle-app.service /etc/systemd/system/multi-user.target.wants/dongle-app.service

mkdir -p /usr/local/modules/apache2
cd /home/ai/moduleApache2
make
cd /home/ai/moduleIpApache2
make

mkdir -p /usr/local/modules/pam && echo -e "#!/bin/sh\nexit 0" > /usr/local/modules/pam/pam.sh && chmod a+x /usr/local/modules/pam/pam.sh
TT=`cat /etc/pam.d/common-auth`
cat > /etc/pam.d/common-auth <<EOF
auth [success=ignore default=1] pam_oath.so usersfile=/disk/admin/modules/pam/oath.txt
auth sufficient pam_exec.so /usr/local/modules/pam/pam.sh oath_success
auth sufficient /usr/local/modules/pam/pam_app.so
session optional pam_exec.so /usr/local/modules/pam/pam.sh
$TT
EOF
cd /home/ai/pam
make

ln -sf /etc/systemd/system/dongle-init.service /etc/systemd/system/sysinit.target.wants/dongle-init.service
ln -sf /etc/systemd/system/dongle-otg.service /etc/systemd/system/sysinit.target.wants/dongle-otg.service
