#!/bin/bash
# Copyright 2018 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

# 1. Consume NMEA datagrams from the specified IPv6 port.
# 2. Produce NMEA sentences to the specified serial device.
# 3. Report on standard output.

# usage: provider [ DEVICE [ SPEED [ PORT ] ] ]

. $(readlink -e $(dirname ${0})/../bin)/setup

DEVICE=${1:-"/dev/ttyUSB0"}
SPEED=${2:-"4800"}
PORT=${3:-"5555"}

stty sane
clear

exec gpstool -6 -P ${PORT} -O -D ${DEVICE} -b ${SPEED} -8 -n -1 -E
