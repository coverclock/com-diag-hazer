#!/bin/bash
# Copyright 2022 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Run the U-blox ZED-UBX-F9T.
# This is part of the "Metronome" sub-project.

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-38400}
ERRFIL=${3-"${SAVDIR}/${PROGRAM}.err"}
OUTFIL=${4-"${SAVDIR}/${PROGRAM}.out"}
PIDFIL=${5-"${SAVDIR}/${PROGRAM}.pid"}

mkdir -p $(dirname ${ERRFIL})
mkdir -p $(dirname ${OUTFIL})
mkdir -p $(dirname ${PIDFIL})

cp /dev/null ${ERRFIL}
exec 2>>${ERRFIL}

. $(readlink -e $(dirname ${0})/../bin)/setup

# UBX-MON-VER [0]

exec gpstool \
    -H ${OUTFIL} -F 1 -t 10 \
    -O ${PIDFIL} \
    -D ${DEVICE} -b ${RATE} -8 -n -1 \
    -U '\xb5\x62\x0a\x04\x00\x00' \
    < /dev/null 1> /dev/null
