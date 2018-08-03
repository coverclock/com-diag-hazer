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

. $(readlink -e $(dirname ${0})/../bin)/setup

DEVICE=${1:-"/dev/ttyUSB0"}
RATE=${2:-9600}

. $(readlink -e $(dirname ${0})/../fun)/ubx7

OPTIONS=""
for OPTION in ${COMMANDS}; do
    OPTIONS="${OPTIONS} -W ${OPTION}"
done

eval gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -c -v -R ${OPTIONS}
