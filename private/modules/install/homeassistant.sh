#!/bin/sh

cd /home/ai
/home/ai/rootfs/usr/local/modules/mydonglecloud/pip.sh -f /usr/local/modules/homeassistant -s
echo "PATH before any modif: $PATH"
PATHOLD=$PATH
PATH=/usr/local/modules/homeassistant/bin:$PATHOLD
export PATH=/usr/local/modules/homeassistant/bin:$PATHOLD
echo "PATH new: $PATH python: `python --version`"
pip install homeassistant==2025.10.3 isal==1.8.0 zlib_ng==1.0.0 pyotp==2.9.0 pyqrcode==1.2.1 pycares==4.11.0 ha-ffmpeg==3.2.2 home-assistant-frontend==20251001.4 av==13.1.0 mutagen==1.47.0 numpy==2.3.2 PyTurboJPEG==1.8.0 aiodhcpwatcher==1.2.1 aiodiscover==2.7.1 cached-ipaddress==0.10.0 go2rtc-client==0.2.1 PyNaCl==1.6.0 file-read-backwards==2.0.0 python-matter-server==8.1.0 pymicro-vad==1.0.1 pyspeex-noise==1.0.2 async-upnp-client==0.45.0 home-assistant-intents==2025.10.1 hassil==3.2.0 aiousbwatcher==1.1.1 pyserial==3.5 bleak==1.0.1 bluetooth-adapters==2.1.0 bluetooth-data-tools==1.28.2 dbus-fast==2.44.3 habluetooth==5.6.4 pycountry==24.6.1 radios==0.3.2 gTTS==2.5.3 PyMetno==0.13.0 paho-mqtt==2.1.0 bleak-esphome==3.3.0 esphome-dashboard-api==1.3.0 aioesphomeapi==41.11.0 aioshelly==13.10.0 aioruuvigateway==0.1.0 xiaomi-ble==1.2.0 ibeacon-ble==1.2.0
PATH=$PATHOLD
export PATH=$PATHOLD
echo "PATH restored: $PATH"
