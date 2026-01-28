#!/bin/sh

echo "{\"a\":\"refresh-webserver\"}" | websocat -1 ws://localhost:8094
