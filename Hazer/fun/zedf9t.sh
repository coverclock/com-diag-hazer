#!/bin/bash
# Copyright 2022 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Configure and run the U-blox ZED-UBX-F9T as a high precision
# time and frequence reference in "survey-in" mode.

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}
mkdir -p ${SAVDIR}

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-38400}
DURATION=${3:-300}
ACCURACY=${4:-250}
ERRFIL=${5-"${SAVDIR}/${PROGRAM}.err"}
OUTFIL=${6-"${SAVDIR}/${PROGRAM}.out"}
PIDFIL=${7-"${SAVDIR}/${PROGRAM}.pid"}

mkdir -p $(dirname ${ERRFIL})
mkdir -p $(dirname ${OUTFIL})
mkdir -p $(dirname ${PIDFIL})

cp /dev/null ${ERRFIL}
exec 2>>${ERRFIL}

. $(readlink -e $(dirname ${0})/../bin)/setup

SVIN_MIN_DUR=$(ubxval -4 ${DURATION})
SVIN_ACC_LIMIT=$(ubxval -4 ${ACCURACY})

log -I -N ${PROGRAM} -i SVIN_MIN_DUR="\"${SVIN_MIN_DUR}\""
log -I -N ${PROGRAM} -i SVIN_ACC_LIMIT="\"${SVIN_ACC_LIMIT}\""

# UBX-MON-VER [0]
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-TMODE-MODE SURVEY_IN
# UBX-CFG-VALSET [12] V0 RAM 0 0 CFG-TMODE-SVIN_MIN_DUR 300 (@sec = 5min)
# UBX-CFG-VALSET [12] V0 RAM 0 0 CFG-TMODE-SVIN_ACC_LIMIT 250 (@0.1mm = 2.5cm = ~1in)

exec coreable gpstool \
    -H ${OUTFIL} -F 1 -t 10 \
    -O ${PIDFIL} \
    -D ${DEVICE} -b ${RATE} -8 -n -1 \
    -x \
    -U '\xb5\x62\x0a\x04\x00\x00' \
    -A '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x01\x00\x03\x20\x01' \
    -A '\xb5\x62\x06\x8a\x0c\x00\x00\x01\x00\x00\x10\x00\x03\x40'"${SVIN_MIN_DUR}" \
    -A '\xb5\x62\x06\x8a\x0c\x00\x00\x01\x00\x00\x11\x00\x03\x40'"${SVIN_ACC_LIMIT}" \
    < /dev/null 1> /dev/null
