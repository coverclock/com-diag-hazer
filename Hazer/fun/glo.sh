#!/bin/bash
# Copyright 2021 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# This uses a Bluetooth-connected GNSS device like the Garmin GLO.
# N.B. The GLO has a high update rate (10Hz), but a low transmission
# rate (4800bps), relative to other GNSS devices (typically 1Hz or
# sometimes 5Hz, at 9600 or even 115800bps). That means that the NMEA
# stream seldom if ever looks drained and idle to gpstool. This turned
# out to be remarkably difficult to handle to my satisfaction.
# N.B. The GLO works okay, but I've never had any luck pairing the
# Bad Elf GPS Pro+ Bluetooth-connected GNSS device with the RPi.

# NOTES
#
# sudo bluetoothctl
# power on
# agent on
# scan on
# :
# (Wait for something like "Device 01:23:45:67:89:AB Garmin GLO #12345".)
# :
# scan off
# pair 01:23:45:67:89:AB
# quit
# sudo rfcomm bind 0 01:23:45:67:89:AB 1
# sudo chmod 666 /dev/rfcomm0
#
# socat -u OPEN:/dev/rfcomm0,b4800 -
#
# serialtool -D /dev/rfcomm0 -b 4800 -8 -n -1
#
# gpstool -D /dev/rfcomm0 -b 4800 -8 -n -1 -E
#
# sudo rfcomm release all
# sudo bluetoothctl
# power off
# quit

PGMDIR=$(dirname ${0})
PGMNAM=$(basename -s .sh ${0})

BASDIR=$(readlink -e ${PGMDIR}/..)
SAVDIR=${COM_DIAG_HAZER_SAVDIR:-${BASDIR}/tmp}

GPSDEV=${1:-"/dev/rfcomm0"}
GPSBPS=${2:-"4800"}
INPSEC=${3:-"1"}
OUTSEC=${4:-"1"}
ERRFIL=${5:-"${SAVDIR}/${PGMNAM}.err"}
OUTFIL=${6:-"${SAVDIR}/${PGMNAM}.out"}

mkdir -p $(dirname ${ERRFIL})
cp /dev/null ${ERRFIL}
exec 2>>${ERRFIL}

mkdir -p $(dirname ${OUTFIL})
cp /dev/null ${OUTFIL}

gpstool -D ${GPSDEV} -b ${GPSBPS} -8 -n -1 -t 10 -E -H ${OUTFIL} -R -i ${INPSEC} -F ${OUTSEC} < /dev/null > /dev/null
