#!/bin/bash
# Copyright 2025 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Configure and run the U-blox ZED-X20P.

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-38400}
ONEPPS=${3:-"/dev/gpiochip4:18"} # pin 12 on Raspberry Pi 5
STROBE=${4:-"/dev/gpiochip4:16"} # pin 36 on Raspberry Pi 5

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}
mkdir -p ${SAVDIR}

ERRFIL="${SAVDIR}/${PROGRAM}.err"
mkdir -p $(dirname ${ERRFIL})
exec 2>>${ERRFIL}

OUTFIL="${SAVDIR}/${PROGRAM}.out"
mkdir -p $(dirname ${OUTFIL})

PIDFIL="${SAVDIR}/${PROGRAM}.pid"
mkdir -p $(dirname ${PIDFIL})

. $(readlink -e $(dirname ${0})/../bin)/setup

exec coreable gpstool \
    -D ${DEVICE} -b ${RATE} -8 -n -1 \
    -E -H ${OUTFIL} -a \
    -O ${PIDFIL} \
    -I ${ONEPPS} -p ${STROBE} \
    -t 10 -F 1 \
    < /dev/null > /dev/null
