#!/bin/bash
# Copyright 2022 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# THIS IS A WORK IN PROGRESS.
# Test the U-blox SARA-R5 on the SparkFun board. The SARA-R5
# combines an LTE-M radio with a M8 GNSS receiver.
# Usage: sarar5 [ DEVICE [ RATE [ HOST [ PORT [ ERRFIL [ OUTFIL [ CSVFIO [ PIDFIL ] ] ] ] ] ] ] ]
# Defaults: sarar5 /dev/ttyUSB0 115200 localhost 23030 out/host/tmp/...

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}
mkdir -p ${SAVDIR}

PROGRAM=$(basename -s .sh ${0})
DEVICE=${1:-"/dev/ttyUSB0"}
RATE=${2:-115200}
HOST=${3:-"localhost"}
PORT=${4:-"23030"}
ERRFIL=${5-"${SAVDIR}/${PROGRAM}.err"}
OUTFIL=${6-"${SAVDIR}/${PROGRAM}.out"}
PIDFIL=${7-"${SAVDIR}/${PROGRAM}.pid"}
CSVFIO=${8-"${SAVDIR}/${PROGRAM}.fio"}

mkdir -p $(dirname ${ERRFIL})
mkdir -p $(dirname ${OUTFIL})
mkdir -p $(dirname ${PIDFIL})
mkdir -p $(dirname ${CSVFIO})

cp /dev/null ${ERRFIL}
exec 2>>${ERRFIL}

rm -f ${CSVFIO}
mkfifo -m 600 ${CSVFIO}
socat -u PIPE:${CSVFIO} UDP-DATAGRAM:${HOST}:${PORT} & CATPID=$!
# socat -u UDP-RECV:${PORT} -
trap "kill ${CATPID}; rm -f ${CSVFIO}" SIGINT SIGQUIT SIGTERM EXIT

. $(readlink -e $(dirname ${0})/../bin)/setup

exec gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -H ${OUTFIL} -F 1 -t 10 -T ${CSVFIO} -O ${PIDFIL} < /dev/null
