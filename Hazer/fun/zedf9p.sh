#!/bin/bash
# Copyright 2023 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# WORK IN PROGRESS

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}

PGMNAM=$(basename ${0})
LOCDEV=${1:-"/dev/ttyACM0"}
LOCBPS=${2:-38400}
ERRFIL=${3:-"${SAVDIR}/${PGMNAM}.err"}
OUTFIL=${4:-"${SAVDIR}/${PGMNAM}.out"}

. $(readlink -e $(dirname ${0})/../bin)/setup

mkdir -p $(dirname ${ERRFIL})
mkdir -p $(dirname ${OUTFIL})

#cp /dev/null ${ERRFIL}
exec 2>>${ERRFIL}

# UBX-CFG-MSG [ 3] UBX-NAV-HPPOSLLH=1
# UBX-MON-TXBUF [ 0]

eval gpstool \
    -H ${OUTFIL} -F 1 -t 10 \
    -D ${LOCDEV} -b ${LOCBPS} -8 -n -1 \
    -w 5 -x \
    -A '\\xb5\\x62\\x06\\x01\\x03\\x00\\x01\\x14\\x01' \
    -U '\\xb5\\x62\\x0a\\x08\\x00\\x00' \
    < /dev/null
