#!/bin/bash
# Copyright 2018 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

# 1. Consume NMEA datagrams from the specified IPv6 port.
# 2. Produce NMEA sentences to the specified serial device.
# 3. Report on standard output.

# usage: provider [ DEVICE [ SPEED [ PORT ] ] ]

# example: nohup provider &

. $(readlink -e $(dirname ${0})/../bin)/setup

DEVICE=${1:-"/dev/ttyUSB1"}
SPEED=${2:-"4800"}
PORT=${3:-"hazer"}

stty sane
clear

exec coreable gpstool -G :${PORT} -K -D ${DEVICE} -b ${SPEED} -8 -n -1
