#!/bin/bash
# Copyright 2023 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# usage: gps18xpcnmea [ DEVICE [ RATE ] ]
# Script for the Garmin GPS-18x USB; this is the unit with the USB
# connection.
# REFERENCES: Garmin, GPS 18x Tech Spec, Rev. D, 4.1.4, p. 14

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-"4800"}

ERRFIL=$(readlink -e $(dirname ${0})/..)/tmp/gps18xusb.err
mkdir -p $(dirname ${ERRFIL})
exec 2>>${ERRFIL}

. $(readlink -e $(dirname ${0})/../bin)/setup

# I was surprised as anyone to find a U-blox device, probably a gen 8,
# inside the Garmin GPS-18x USB, after my experience with the Garmin
# GPS-18x PC, which seems to have something else since it understands
# Garmin's proprietary binary commands. The commands I pass to the
# GPS-18x USB below are what I used to probe it. The Garmin proprietary
# NMEA-like commands do not work, but the U-blox proprietary binary
# command and the NMEA-like commands do.

# PGRMO
# PGRMC1
# PGRMI
# NMEA-PUBX-POSITION
# NMEA-PUBX-SVSTATUS
# NMEA-PUBX-TIME
# UBX-MON-VER [0]

coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -E \
	-W '$PGRMO,GPALM,1' \
	-W '$PGRMC1,1,1,,,,,2,W,N,,,,1,,1' \
	-W '$PGRMI,,,,,,,R' \
    	-W '$PUBX,00' \
    	-W '$PUBX,03' \
    	-W '$PUBX,04' \
    	-U '\xb5\x62\x0a\x04\x00\x00'
'
