#!/bin/bash
# Copyright 2023 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Just runs the headless gpstool against the SPARTN-enabled UBX-ZED-F9P.
# Does not touch the companion UBX-NEO-D9S.
#
# REFERENCES
#
# "NEO-D9S and ZED-F9 configuration SPARTN L-band correction data reception
# Application Note", UBX-22008160-R02, U-blox AG, 2022-07-22

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}

PGMNAM=$(basename ${0})
FILNAM="spartan"
LOCDEV=${1:-"/dev/ttyACM0"}
LOCBPS=${2:-38400}
ERRFIL=${3:-"${SAVDIR}/${FILNAM}.err"}
OUTFIL=${4:-"${SAVDIR}/${FILNAM}.out"}
CSVFIL=${5:-"${SAVDIR}/${FILNAM}.csv"}
PIDFIL=${6:-"${SAVDIR}/${FILNAM}.pid"}

. $(readlink -e $(dirname ${0})/../bin)/setup

mkdir -p $(dirname ${ERRFIL})
mkdir -p $(dirname ${OUTFIL})
mkdir -p $(dirname ${CSVFIL})
mkdir -p $(dirname ${PIDFIL})

exec 2>>${ERRFIL}

MESSAGE="${PGMNAM}: Starting ${LOCDEV}"
log -I -N ${PGMNAM} -n "${MESSAGE}"

MESSAGE="${PGMNAM}: Processing UBX-ZED-F9P ${LOCDEV} ${LOCBPS}"
log -I -N ${PGMNAM} -n "${MESSAGE}"

gpstool \
    -D ${LOCDEV} -b ${LOCBPS} -8 -n -1 \
    -O ${PIDFIL} \
    -H ${OUTFIL} -F 1 -t 10 \
    -T ${CSVFIL} -f 1 \
    < /dev/null

if (( $? != 0 )); then
    echo "${PGMNAM}: ${LOCDEV} failed!" 1>&2
    exit 1
fi

MESSAGE="${PGMNAM}: Ending ${LOCDEV}"
log -I -N ${PGMNAM} -n "${MESSAGE}"

exit 0
