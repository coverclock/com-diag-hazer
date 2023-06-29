#!/bin/bash
# Copyright 2023 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# usage: gps18xusb2nmea [ DEVICE [ FAST [ SLOW ] ] ]
# Script for the Garmin GPS-18x USB; this is the unit with the USB connection.
# It places the device in NMEA output mode, resets it, and then exits.
# Assumes the device is in CPO mode.
# Note that when in NMEA mode the device operate at 4800 BPS, and when
# in CPO (binary) mode the device operates at 9600 BPS.
# REFERENCES: Garmin, GPS 18x Tech Spec, Rev. D, 4.1.4, p. 14

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-"9600"}

ERRFIL=$(readlink -e $(dirname ${0})/..)/tmp/${PROGRAM}.err
mkdir -p $(dirname ${ERRFIL})
exec 2>>${ERRFIL}

. $(readlink -e $(dirname ${0})/../bin)/setup

coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -E \
	-Z '\x10\x0A\x02\x26\x00\xCE\x10\x03' \
	-W '$PGRMC1,1,1,,,,,2,W,N,,,,1,,1' \
	-W '$PGRMI,,,,,,,R' \
	-W ''
