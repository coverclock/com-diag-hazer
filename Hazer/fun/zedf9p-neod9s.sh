#!/bin/bash
# Copyright 2023 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
#
# REFERENCES
#
# "NEO-D9S and ZED-F9 configuration SPARTN L-band correction data reception
# Application Note", UBX-22008160-R02, U-blox AG, 2022-07-22

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}

PGMNAM=$(basename ${0})
FILNAM=${PGMNAM%-*}
LOCDEV=${1:-"/dev/ttyACM0"}
LOCBPS=${2:-38400}
CORDEV=${3:-"/dev/ttyACM1"}
CORBPS=${4:-9600}
ERRFIL=${5:-"${SAVDIR}/${FILNAM}.err"}
OUTFIL=${6:-"${SAVDIR}/${PGMNAM}.out"}

. $(readlink -e $(dirname ${0})/../bin)/setup

mkdir -p $(dirname ${ERRFIL})
mkdir -p $(dirname ${OUTFIL})

#cp /dev/null ${ERRFIL}
exec 2>>${ERRFIL}

#####
# QUERY THE UBX-NEO-D9S INMARSAT RECEIVER.
#####

MESSAGE="${PGMNAM}: Querying UBX-NEO-D9S ${CORDEV} ${CORBPS}"
log -I -N ${PGMNAM} -n "${MESSAGE}"
echo "${MESSAGE}"

# UBX-MON-TXBUF [ 0]

gpstool \
    -R \
    -D ${CORDEV} -b ${CORBPS} -8 -n -1 \
    -w 5 -x \
    -U '\xb5\x62\x0a\x08\x00\x00' \
    -U '' \
    < /dev/null

# UBX-CFG-MSG [ 3] UBX-NAV-HPPOSLLH=1

#####
# PROCESS THE UBX-ZED-F9P GNSS RECEIVER.
#####

MESSAGE="${PGMNAM}: Processing UBX-ZED-F9P ${LOCDEV} ${LOCBPS}"
log -I -N ${PGMNAM} -n "${MESSAGE}"
echo "${MESSAGE}"

exec gpstool \
    -H ${OUTFIL} -F 1 -t 10 \
    -D ${LOCDEV} -b ${LOCBPS} -8 -n -1 \
    -w 5 -x \
    -A '\xb5\x62\x06\x01\x03\x00\x01\x14\x01' \
    < /dev/null
