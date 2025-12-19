#!/bin/sh

apt-get install -y libleptonica-dev zlib1g-dev libreoffice-writer libreoffice-calc libreoffice-impress unpaper ocrmypdf
mv /usr/local/modules/stirlingpdf /home/ai/build
mkdir /usr/local/modules/stirlingpdf
cd /home/ai/build/stirlingpdf
/home/ai/rootfs/usr/local/modules/mydonglecloud/pip.sh -f /usr/local/modules/stirlingpdf -s
echo "PATH before any modif: $PATH"
PATHOLD=$PATH
PATH=/usr/local/modules/stirlingpdf/bin:$PATHOLD
export PATH=/usr/local/modules/stirlingpdf/bin:$PATHOLD
echo "PATH new: $PATH python: `python --version`"
pip install uno opencv-python-headless unoconv pngquant WeasyPrint
export JAVA_HOME=/usr/lib/jvm/java-21-openjdk-arm64
sync
echo 3 > /proc/sys/vm/drop_caches
./gradlew build
sync
echo 3 > /proc/sys/vm/drop_caches
cp -a scripts stirling-pdf/build/libs/stirling-pdf-*.jar /usr/local/modules/stirlingpdf
PATH=$PATHOLD
export PATH=$PATHOLD
echo "PATH restored: $PATH"
ln -sf /etc/systemd/system/stirlingpdf.service /etc/systemd/system/multi-user.target.wants/stirlingpdf.service
