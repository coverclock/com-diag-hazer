#!/bin/bash

BROWSER="/Applications/Firefox.app/Contents/MacOS/firefox"
APIKEY="$(cat ~/googlemapapikey.txt)"
ZOOM="15"
SIZE="400x400"
SCALE="2"
PREFIX="https://maps.googleapis.com/maps/api/staticmap?zoom=${ZOOM}&size=${SIZE}&scale=${SCALE}&key=${KEY}"
#markers=39.794218,-105.153390
#center=39.794218,-105.153390

socat
