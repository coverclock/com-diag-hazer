#!/bin/bash
# Copyright 2023 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# This script is specific to the GlobalSat BU-353N(10Hz).
# This scripts tries to update the display in real-time, but
# at 10Hz, there is never a time when there is not data to be
# processed, so gpstool never updates the display. So use the
# bu353n10HzH script instead, and peruse the output file.

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyUSB0"}
RATE=${2:-115200}
ERRFIL=${3:-"${SAVDIR}/${PROGRAM}.err"}

mkdir -p $(dirname ${ERRFIL})

cp /dev/null ${ERRFIL}
exec 2>>${ERRFIL}

. $(readlink -e $(dirname ${0})/../bin)/setup

eval coreable gpstool \
    -D ${DEVICE} -b ${RATE} -8 -n -1 \
    -E -F 1 -t 10 \
    -W '\$PAIR020' \
    > /dev/null < /dev/null
