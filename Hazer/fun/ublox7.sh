#!/bin/bash
# Copyright 2018 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-diminuto

# I use a NaviSys Technology GR-701W USB GPS device to for this.
# The GR-701W exports 1PPS via DCD, which the "-c" option below
# expects.

. $(readlink -e $(dirname ${0})/../bin)/setup

DEVICE=${1:-"/dev/ttyUSB0"}
RATE=${2:-9600}

MESSAGE1="\$PUBX,00"
MESSAGE2="\$PUBX,03"
MESSAGE3="\\x24PUBX,04"
gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -c -v -W "${MESSAGE1}" -W "${MESSAGE2}" -W "${MESSAGE3}" 2>&1 | grep "PUBX"
