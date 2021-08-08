#!/bin/bash
# Copyright 2018-2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Like the bu353w10 script but uses socat to read from the device
# and send to gpstool over a FIFO as a source.

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-9600}
FIFO=${3:-${TMPDIR:="/tmp"}"/"${PROGRAM}".fifo"}
BYTES=${4:-"8192"}

. $(readlink -e $(dirname ${0})/../bin)/setup

. $(readlink -e $(dirname ${0})/../fun)/ubx8

LOG=$(readlink -e $(dirname ${0})/..)/log
mkdir -p ${LOG}

OPTIONS=""
for OPTION in ${COMMANDS}; do
    OPTIONS="${OPTIONS} ${OPTION}"
done

eval coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 ${OPTIONS} -U '' 2> ${LOG}/${PROGRAM}-initialize.err

mkfifo ${FIFO} 

socat -u -b ${BYTES} OPEN:${DEVICE},b${RATE},cs8,rawer PIPE:${FIFO} &

trap "rm -f ${FIFO}" SIGINT SIGQUIT SIGTERM EXIT

coreable gpstool -S ${FIFO} -E 2> ${LOG}/${PROGRAM}-source.err
