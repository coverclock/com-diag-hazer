#!/bin/bash
# Copyright 2022 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# EXPERIMENTAL
# This script tests the Bad Elf GPS Pro+ via BlueTooth. This appears
# to have started pairing with FW revision 3.0.0, but still doesn't quite
# work.

# NOTES
#
# sudo bluetoothctl
# power on
# agent on
# scan on
# :
# (Wait for something like "Device 01:23:45:67:89:AB Bad Elf GPS #020661".)
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
# sudo rfcomm release 0
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
