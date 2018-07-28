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

COMMAND1="\$PUBX,00"
COMMAND2="\$PUBX,03"
COMMAND3="\$PUBX,04"
COMMAND4="\\xb5\\x62\\x0a\\x04\\x00\\x00"
COMMAND5="\\xb5\\x62\\x06\\x31\\x00\\x00"

gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -c -E -W "${COMMAND1}" -W "${COMMAND2}" -W "${COMMAND3}" -W "${COMMAND4}" -W "${COMMAND5}"
