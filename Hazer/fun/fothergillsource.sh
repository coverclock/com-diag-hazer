#!/bin/bash
# Copyright 2023 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# In support of testing the Fothergill project, in which gpstool
# forwards CSV-based traces over LoRa radios using HDLC-like
# framing. Note that the LoRa radio channel is sucky slow and
# will disconnect if overrun, which happens easily.

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}
mkdir -p ${SAVDIR}

PROGRAM=$(basename ${0})
RADDEV=${1:-"/dev/ttyACM0"}
RADBPS=${2:-57600}
GPSDEV=${3:-"/dev/ttyACM1"}
GPSBPS=${4:-9600}
FIFO=${5:-${TMPDIR:="/tmp"}"/"${PROGRAM}".fifo"}
ERRFIL=${6:-"${SAVDIR}/${PROGRAM}.err"}

exec 2>>${ERRFIL}

. $(readlink -e $(dirname ${0})/../bin)/setup

rm -f ${FIFO} 2> /dev/null
mkfifo ${FIFO}
cat ${FIFO} | framertool -D ${RADDEV} -b ${RADBPS} -8 -n -1 > /dev/null &
PIPE=$!
trap "kill ${PIPE} 2> /dev/null; rm -f ${FIFO} 2> /dev/null" SIGINT SIGQUIT SIGTERM EXIT
echo "${PROGRAM}: ${RADDEV} ${RADBPS} ${GPSDEV} ${GPSBPS} ${FIFO} ${ERRFIL} ${PIPE}" 1>&2

# N.B. gpstool does not emit any trace (-T) data until the GPS
# receiver establishes a fix. For a cold start, this can take
# several minutes to happen. Nothing will show up in the FIFO
# or on the LoRa channel until that happens.

coreable gpstool -D ${GPSDEV} -b ${GPSBPS} -8 -n -1 -T ${FIFO} -f 10 -E -t 10
