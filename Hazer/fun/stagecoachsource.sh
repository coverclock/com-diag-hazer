#!/bin/bash
# Copyright 2023 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}
mkdir -p ${SAVDIR}

PROGRAM=$(basename ${0})
PORT=${1:-24040}
DEVICE=${2:-"/dev/ttyACM0"}
RATE=${3:-9600}
FIFO=${4:-${TMPDIR:="/tmp"}"/"${PROGRAM}".fifo"}
ERRFIL=${3:-"${SAVDIR}/${PROGRAM}.err"}

. $(readlink -e $(dirname ${0})/../bin)/setup

mkfifo ${FIFO} 

csv2dgm -U localhost:${PORT} -c < ${FIFO} &
PIPE=$!

trap "kill ${PIPE}; rm -f ${FIFO}" SIGINT SIGQUIT SIGTERM EXIT

coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -T ${FIFO} -E -t 10 2> ${ERRFIL}
