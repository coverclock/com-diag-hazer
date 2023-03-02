#!/bin/bash
# Copyright 2021 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# This script tests the u-blox NEO-M8N GPS module, in this case on
# a RaceDayQuads Micro M8N board. The RDQ board is purpose-made for
# hobbiest aerial drones and has a four wire serial connection:
# red:3.3V, black:GND, green:RX (relative to the drone), white:TX (ditto).

SELF=$$

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}
mkdir -p ${SAVDIR}

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyS0"}
RATE=${2:-9600}
ERRFIL=${3-"${SAVDIR}/${PROGRAM}.err"}
OUTFIL=${4-"${SAVDIR}/${PROGRAM}.out"}
FREQUENCY=${5:-1}

mkdir -p $(dirname ${ERRFIL})
mkdir -p $(dirname ${OUTFIL})

cp /dev/null ${ERRFIL}
cp /dev/null ${OUTFIL}

exec 2>>${ERRFIL}

. $(readlink -e $(dirname ${0})/../bin)/setup

exec coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -t 10 -H ${OUTFIL} -F ${FREQUENCY}
