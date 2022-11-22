#!/bin/bash
# Copyright 2022 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# THIS IS A WORK IN PROGRESS.
# Test the U-blox SARA-R5 on the SparkFun board. The SARA-R5
# combines an LTE-M radio with a M8 GNSS receiver.
# Usage: sarar5 [ DEVICE [ RATE [ ERRFIL [ OUTFIL [ CSVFIO [ PIDFIL ] ] ] ] ] ]
# Defaults: sarar5 /dev/ttyUSB0 115200 out/host/tmp/sarar5.err out/host/tmp/sarar5.out out/host/tmp/sarar5.csv out/host/tmp/sarar5.pid

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}
mkdir -p ${SAVDIR}

PROGRAM=$(basename -s .sh ${0})
DEVICE=${1:-"/dev/ttyUSB0"}
RATE=${2:-115200}
ERRFIL=${3-"${SAVDIR}/${PROGRAM}.err"}
OUTFIL=${4-"${SAVDIR}/${PROGRAM}.out"}
CSVFIO=${5-"${SAVDIR}/${PROGRAM}.csv"}
PIDFIL=${6-"${SAVDIR}/${PROGRAM}.pid"}

mkdir -p $(dirname ${ERRFIL})
mkdir -p $(dirname ${OUTFIL})
mkdir -p $(dirname ${CSVFIO})
mkdir -p $(dirname ${PIDFIL})

cp /dev/null ${ERRFIL}
exec 2>>${ERRFIL}

rm -f ${CSVFIO}
mkfifo -m 600 ${CSVFIO}
trap "rm -f ${CSVFIO}" SIGINT SIGQUIT SIGTERM EXIT
# socat -u PIPE:${CSVFIO} - &

. $(readlink -e $(dirname ${0})/../bin)/setup

exec coreable gpstool \
    -D ${DEVICE} -b ${RATE} -8 -n -1 -n -1 \
    -H ${OUTFIL} -F 1 -t 10 \
    -T ${CSVFIO} \
    -O ${PIDFIL}
