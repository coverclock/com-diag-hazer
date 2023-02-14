#!/bin/bash -x
# Copyright 2023 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# In support of testing the Stagecoach program in the Codex project.
# Codex is (or will be, if and when I get it working) an SSL tunnel
# and client and server proxy for Hazer applications. It is hosted
# in the Codex project because it is built on top of Codex, an SSL
# library.

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}
mkdir -p ${SAVDIR}

PROGRAM=$(basename ${0})
PORT=${1:-24040}
HOST=${2:-"localhost"}
DEVICE=${3:-"/dev/ttyACM0"}
RATE=${4:-9600}
FIFO=${5:-${TMPDIR:="/tmp"}"/"${PROGRAM}".fifo"}
ERRFIL=${6:-"${SAVDIR}/${PROGRAM}.err"}

exec 2>>${ERRFIL}

. $(readlink -e $(dirname ${0})/../bin)/setup

rm -f ${FIFO} 2> /dev/null
mkfifo ${FIFO} 
cat ${FIFO} | csv2dgm -U ${HOST}:${PORT} -c &
PIPE=$!
trap "kill ${PIPE} 2> /dev/null; rm -f ${FIFO} 2> /dev/null" SIGINT SIGQUIT SIGTERM EXIT
echo "${PROGRAM}: ${PORT} ${HOST} ${DEVICE} ${RATE} ${FIFO} ${ERRFIL} ${PIPE}" 1>&2

# N.B. gpstool does not emit any trace (-T) data until the GPS
# receiver establishes a fix. For a cold start, this can take
# several minutes to happen. Nothing will show up in the FIFO
# or on the UDP PORT until that happens.

coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -T ${FIFO} -E -t 10
