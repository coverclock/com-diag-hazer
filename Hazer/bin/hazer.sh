#!/bin/bash
# Copyright 2018 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

# 1. Consume NMEA sentences from the specified serial device
# 2. Report on standard output.

# usage: hazer [ DEVICE [ RATE ] ]

. $(readlink -e $(dirname ${0})/../bin)/setup

DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-"9600"}

stty sane
clear

exec coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -E
