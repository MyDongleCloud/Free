#!/bin/sh

echo '{"a":"refresh"}' | nc -w 1 localhost 8093
