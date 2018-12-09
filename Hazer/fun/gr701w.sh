#!/bin/bash
# Copyright 2018 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

# I use a NaviSys Technology GR-701W USB GPS device for this.
# The GR-701W exports 1PPS via DCD, which the "-c" option below
# expects. But you could use this on any Ublox 7 GPS device and
# if it doesn't implement 1PPS, the script should still otherwise
# work.

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyUSB0"}
RATE=${2:-9600}

. $(readlink -e $(dirname ${0})/../bin)/setup

DIR=$(readlink -e $(dirname ${0})/..)/log
mkdir -p ${DIR}

eval coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -c -E -t 10 2> ${DIR}/${PROGRAM}.log
