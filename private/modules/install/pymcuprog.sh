#!/bin/sh

cd /home/ai/build
git clone https://github.com/microchip-pic-avr-tools/pymcuprog
cd pymcuprog
cat > setup.py <<EOF
from setuptools import setup
if __name__ == '__main__':
    setup()
EOF
python3 setup.py install
