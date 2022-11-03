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

# sudo rfcomm release all

# serialtool -D /dev/rfcomm0 -b 4800 -8 -n -1

PGMDIR=$(dirname ${0})
PGMNAM=$(basename -s .sh ${0})

BASDIR=$(readlink -e ${PGMDIR}/..)
SAVDIR=${COM_DIAG_HAZER_SAVDIR:-${BASDIR}/tmp}
RUNDIR=${XDG_RUNTIME_DIR:-"/run/user/${UID}"}

GPSDEV=${1:-"/dev/rfcomm0"}
GPSBPS=${2:-"4800"}
INPSEC=${3:-"1"}
OUTSEC=${4:-"1"}
ERRFIL=${5:-"${SAVDIR}/${PGMNAM}.err"}
OUTFIL=${6:-"${RUNDIR}/${PGMNAM}.out"}

HEDDIR=$(dirname ${OUTFIL})
HEDFIL=$(basename ${OUTFIL})
HEDTSK=${HEDFIL%%.*}
HEDTYP=${HEDFIL#*.}
HEDLIM=$(($(stty size | cut -d ' ' -f 1) - 2))

mkdir -p $(dirname ${ERRFIL})
cp /dev/null ${ERRFIL}
exec 2>>${ERRFIL}

echo "${PGMNAM}: RUNDIR=${RUNDIR}" 1>&2
echo "${PGMNAM}: PGMDIR=${PGMDIR}" 1>&2
echo "${PGMNAM}: BASDIR=${BASDIR}" 1>&2
echo "${PGMNAM}: SAVDIR=${SAVDIR}" 1>&2
echo "${PGMNAM}: GPSDEV=${GPSDEV}" 1>&2
echo "${PGMNAM}: GPSBPS=${GPSBPS}" 1>&2
echo "${PGMNAM}: INPSEC=${INPSEC}" 1>&2
echo "${PGMNAM}: OUTSEC=${OUTSEC}" 1>&2
echo "${PGMNAM}: ERRFIL=${ERRFIL}" 1>&2
echo "${PGMNAM}: OUTFIL=${OUTFIL}" 1>&2
echo "${PGMNAM}: HEDDIR=${HEDDIR}" 1>&2
echo "${PGMNAM}: HEDFIL=${HEDFIL}" 1>&2
echo "${PGMNAM}: HEDTSK=${HEDTSK}" 1>&2
echo "${PGMNAM}: HEDTYP=${HEDTYP}" 1>&2
echo "${PGMNAM}: HEDLIM=${HEDLIM}" 1>&2

mkdir -p $(dirname ${OUTFIL})
cp /dev/null ${OUTFIL}

. ${BASDIR}/bin/setup

# exec coreable gpstool -D ${GPSDEV} -b ${GPSBPS} -8 -n -1 -t 10 -E -i ${INPSEC} -F ${OUTSEC} < /dev/null

coreable gpstool -D ${GPSDEV} -b ${GPSBPS} -8 -n -1 -t 10 -H ${OUTFIL} -R -i ${INPSEC} -F ${OUTSEC} < /dev/null > /dev/null &
GPSPID=$!
echo "${PGMNAM}: GPSPID=${GPSPID}" 1>&2

GLOPID=$$
trap "trap '' SIGINT SIGQUIT SIGTERM; kill -TERM -- -${GLOPID} ${GPSPID} 2> /dev/null; exit 0" SIGINT SIGQUIT SIGTERM
echo "${PGMNAM}: GLOPID=${GLOPID}" 1>&2

peruse ${HEDTSK} ${HEDTYP} ${HEDLIM} ${HEDDIR} < /dev/null
