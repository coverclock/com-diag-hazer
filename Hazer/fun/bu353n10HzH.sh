#!/bin/bash
# Copyright 2023 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# This script is specific to the GlobalSat BU-353N(10Hz).

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyUSB0"}
RATE=${2:-115200}
ERRFIL=${3:-"${SAVDIR}/${PROGRAM}.err"}
OUTFIL=${4:-"${SAVDIR}/${PROGRAM}.out"}
PIDFIL=${5:-"${SAVDIR}/${PROGRAM}.pid"}

mkdir -p $(dirname ${ERRFIL})
mkdir -p $(dirname ${OUTFIL})
mkdir -p $(dirname ${PIDFIL})

cp /dev/null ${ERRFIL}
exec 2>>${ERRFIL}

. $(readlink -e $(dirname ${0})/../bin)/setup

exec gpstool \
    -D ${DEVICE} -b ${RATE} -8 -n -1 \
    -H ${OUTFIL} -F 1 -t 10 \
    -O ${PIDFIL} \
    -W '\$PAIR020' \
    > /dev/null < /dev/null
