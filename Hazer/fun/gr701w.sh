#!/bin/bash
# Copyright 2018 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

# I use a NaviSys Technology GR-701W USB GPS device to for this.
# The GR-701W exports 1PPS via DCD, which the "-c" option below
# expects.

. $(readlink -e $(dirname ${0})/../bin)/setup

DEVICE=${1:-"/dev/ttyUSB0"}
RATE=${2:-9600}

gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -c -E
