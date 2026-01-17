#!/bin/sh

cd /home/ai
/home/ai/rootfs/usr/local/modules/mydonglecloud/pip.sh -f /usr/local/modules/wgdashboard/penv -s
echo "PATH before any modif: $PATH"
PATHOLD=$PATH
PATH=/usr/local/modules/wgdashboard/penv/bin:$PATHOLD
export PATH=/usr/local/modules/wgdashboard/penv/bin:$PATHOLD
echo "PATH new: $PATH python: `python --version`"
cd /usr/local/modules/wgdashboard/src
pip install -r requirements.txt
PATH=$PATHOLD
export PATH=$PATHOLD
echo "PATH restored: $PATH"
ln -sf /disk/admin/modules/wgdashboard/db
ln -sf /disk/admin/modules/wgdashboard/attachments
mkdir /var/log/wgdashboard
ln -sf /var/log/wgdashboard log
cat > ssl-tls.ini << EOF
[SSL/TLS]
certificate_path = 
private_key_path = 
EOF
cat > wg-dashboard.ini.bak << EOF
[Account]
username = user
password = pass
enable_totp = false
totp_verified = false
totp_key = 

[Server]
wg_conf_path = /etc/wireguard
awg_conf_path = /etc/amnezia/amneziawg
app_prefix = 
app_ip = 127.0.0.1
app_port = 10086
auth_req = true
version = v4.3.1
dashboard_refresh_interval = 60000
dashboard_peer_list_display = grid
dashboard_sort = status
dashboard_theme = dark
dashboard_api_key = false
dashboard_language = en-US

[Peers]
peer_global_dns = 1.1.1.1
peer_endpoint_allowed_ip = 0.0.0.0/0
peer_display_mode = grid
remote_endpoint = 192.168.10.8
peer_mtu = 1420
peer_keep_alive = 21

[Other]
welcome_session = false

[Database]
type = sqlite
host = 
port = 
username = 
password = 

[Email]
server = 
port = 
encryption = 
username = 
email_password = 
authentication_required = true
send_from = 
email_template = 

[OIDC]
admin_enable = false
client_enable = false

[Clients]
enable = true

[WireGuardConfiguration]
autostart = 
EOF
