#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# This script tests the u-blox SAM-M8Q GPS module, in my case on
# a SparkFun SAM-M8Q board.

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}
mkdir -p ${SAVDIR}

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyS0"}
RATE=${2:-9600}
ONEPPS=${3:-"/dev/gpiochip4:18"}
STROBE=${4:-"/dev/gpiochip4:16"}
ERRFIL=${5-"${SAVDIR}/${PROGRAM}.err"}
OUTFIL=${6-"${SAVDIR}/${PROGRAM}.out"}
PIDFIL=${7-"${SAVDIR}/${PROGRAM}.pid"}

mkdir -p $(dirname ${ERRFIL})
mkdir -p $(dirname ${OUTFIL})
mkdir -p $(dirname ${PIDFIL})

cp /dev/null ${ERRFIL}
cp /dev/null ${OUTFIL}

exec 2>>${ERRFIL}

. $(readlink -e $(dirname ${0})/../bin)/setup

exec coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -I ${ONEPPS} -p ${STROBE} -H ${OUTFIL} -t 10 -F 1 -O ${PIDFIL}
