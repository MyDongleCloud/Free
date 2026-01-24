#!/bin/sh

echo "{\"a\":\"refresh\"}" | websocat -1 ws://localhost:8094
