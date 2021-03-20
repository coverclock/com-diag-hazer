#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

# This script tests the u-blox NEO-M8N GPS module, in my case on
# a SparkFun NEO-M8N board. The SparkFun board has a USB
# interface and the NEO-M8N provides 1PPS via a GPIO pin, which
# is the reason for the "-I" option below. This script also
# configures the gpstool to forward the 1PPS signal by strobing a
# GPIO pin, the "-p" option. In this case, it uses GPIO pin 16 on
# a hardware test fixture I fabricated. I run this script on a
# Raspberry Pi as user "pi", which has access to the GPIO pins
# by virtue of being in group "gpio".

SELF=$$

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}
mkdir -p ${SAVDIR}

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-115200}
ONEPPS=${3:-18}
STROBE=${4:-16}
ERRFIL=${5-"${SAVDIR}/${PROGRAM}.err"}
OUTFIL=${6-"${SAVDIR}/${PROGRAM}.out"}
PIDFIL=${7-"${SAVDIR}/${PROGRAM}.pid"}
LIMIT=${8:-$(($(stty size | cut -d ' ' -f 1) - 2))}

mkdir -p $(dirname ${ERRFIL})
mkdir -p $(dirname ${OUTFIL})
mkdir -p $(dirname ${PIDFIL})

cp /dev/null ${ERRFIL}
cp /dev/null ${OUTFIL}

exec 2>>${ERRFIL}

. $(readlink -e $(dirname ${0})/../bin)/setup

coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -I ${ONEPPS} -p ${STROBE} -H ${OUTFIL} -t 10 -O ${PIDFIL} < /dev/null 1> /dev/null &

sleep 5

cat ${ERRFIL}

DIRECTORY=$(dirname ${OUTFIL})
FILENAME=$(basename ${OUTFIL})
TASK=${FILENAME%%.*}
FILE=${FILENAME#*.}

echo peruse ${TASK} ${FILE} ${LIMIT} ${DIRECTORY}
