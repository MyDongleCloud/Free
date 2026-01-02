This page documents the ports that are used both in the image/device and on the server.

# Used ports in the image

This is the list of all used ports in the image, separated between listening ports and outgoing ports.

### Listening ports (127.0.0.1)
- mosquitto: 1883 (zigbee2mqtt)
- webssh2: 2222 (reverseproxy)
- betterauthstudio: 3002
- librechat: 3080 (reverseproxy)
- lobechat: 3210 (reverseproxy)
- mysql: 3306, 33060
- postgres: 5432
- redis: 6379
- frp: 7400
- meilisearch: 7700
- whisparr: 6969 (reverseproxy)
- radarr: 7878 (reverseproxy)
- metube: 8089 (reverseproxy)
- triliumnotes: 8090 (reverseproxy)
- betterauth: 8091 (reverseproxy)
- mydongecloud app: 8093 (socket), 8094 (reverseproxy)
- zigbee2mqtt: 8095 (reverseproxy)
- jellyfin: 8096 (reverseproxy)
- tubesync: 8098 (reverseproxy)
- llamacpp: 8099 (reverseproxy)
- tabby: 8100 (reverseproxy)
- openwebui: 8101 (reverseproxy)
- pipedbackend: 8102 (reverseproxy)
- pipedproxy: 8103 (reverseproxy)
- shields: 8104 (reverseproxy)
- stirlingpdf: 8105 (reverseproxy)
- microbin: 8106 (reverseproxy)
- typesense: 8107, 8108 (reverseproxy)
- qbittorent: 8109 (reverseproxy)
- syncthing: 8384 (reverseproxy)
- lidarr: 8686 (reverseproxy)
- sonarr: 8989 (reverseproxy)
- portainer: 9000 (reverseproxy), 19443
- transmission: 9091 (reverseproxy)
- prowlarr: 9696 (reverseproxy)
- rspamd: 11332, 11333, 11334
- stremio: 11470 (reverseproxy)
- mongodb: 27017

### Listening ports (0.0.0.0)
- ssh: 22
- postfix: 25 (mail), 465 (smtp)
- apache: 80 (http), 443 (https), 9400-9526 (modules)
- dovecot imaps: 110 (imaps), 143 (pop3s), 993 (imaps), 995 (pop3s)
- networkmanager: 546 (dhcpv6)
- avahi: 5353 (mdns)
- transmission: 6771, 51413, 60562

### Listening ports (0.0.0.0 should be 127.0.0.1)
- homeassistant: 1900, 5353, 8123
- audiobookshelf: 8092 (reverseproxy)
- cockpit: 9090 (reverseproxy)

### Outgoing ports
- frp: 7000 (main), xxx22 (ssh), xxx25 (mail), xx465 (smtp), xx993 (imap), xx995 (pop3)
- postfix: 466


# Used listening ports on server

This is the list of all used listening ports on server side.

- frp: 80 (http), 443 (https), 7000 (clients), 7500 (stats)
- haproxy: 22 (ssh), 465 (smtp), 993 (imap), 995 (pop3)
- postfix: 25 (email)
- bind9: 53 (dns)


# Traffic paths

This is the interesting traffic paths from and to the Internet:

### Internet -> dongle

- 22: haproxy -> frps (19022) -/-> frpc (22) -> openssh
- 25: postfix -> frps (19025) -/-> frpc (25) -> postfix
- 80: frps -/-> frpc (80) -> apache2
- 443: frps -/-> frpc (443) -> apache2
- 465: haproxy -> frps (19465) -/-> frpc (465) -> postfix
- 993: haproxy -> frps (19993) -/-> frpc (993) -> dovecot
- 995: haproxy -> frps (19995) -/-> frpc (995) -> dovecot

### dongle->outside

- 465: postfix -/-> postfix (466)
