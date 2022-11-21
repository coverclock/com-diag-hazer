#!/bin/bash
# Copyright 2022 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

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
mkfifo ${CSVFIO}
trap "rm -f ${CSVFIO}" SIGINT SIGQUIT SIGTERM EXIT
# socat -u PIPE:${CSVFIO} - &

. $(readlink -e $(dirname ${0})/../bin)/setup

exec coreable gpstool \
    -D ${DEVICE} -b ${RATE} -8 -n -1 -n -1 \
    -H ${OUTFIL} -F 1 -t 10 \
    -T ${CSVFIO} \
    -O ${PIDFIL}
