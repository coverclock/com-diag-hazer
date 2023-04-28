#!/bin/bash
# Copyright 2023 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# In support of testing the Fothergill project, in which gpstool
# forwards CSV-based traces over LoRa radios using HDLC-like
# framing.

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

# If the LoRaSerial device (which this script was written specifically
# to test) disconnects from the USB bus (which it is wont to do, an
# issue I am examining), framertool will receive an EOF from the standard
# I/O library and exit. This will tear down the pipeline it is part of.
# When that happens, the OS will send the process on the other end of the
# FIFO, which is gpstool, a SIGPIPE signal. When gpstool receives this, it
# will exit after logging a message. The message can be found in the ERRFIL
# established above.

cat ${FIFO} | csv2lora | framertool -D ${RADDEV} -b ${RADBPS} -8 -n -1 -h > /dev/null &
PIPE=$!

trap "kill ${PIPE} 2> /dev/null; rm -f ${FIFO} 2> /dev/null" SIGINT SIGQUIT SIGTERM EXIT

echo "${PROGRAM}: ${RADDEV} ${RADBPS} ${GPSDEV} ${GPSBPS} ${FIFO} ${ERRFIL} ${PIPE}" 1>&2

# gpstool does not emit any trace (-T) data until the GPS receiver
# establishes a fix. For a GPS cold start, this can take several minutes to
# happen. Nothing will show up in the FIFO or on the LoRa channel until
# that happens.

gpstool -D ${GPSDEV} -b ${GPSBPS} -8 -n -1 -T ${FIFO} -f 10 -E -t 10
