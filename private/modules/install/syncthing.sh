#!/bin/sh

cd /usr/local/modules/syncthing
go run build.go
cp bin/syncthing /usr/local/bin
