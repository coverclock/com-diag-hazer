#!/bin/bash
# Copyright 2018 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

# 1. Consume NMEA sentences from the specified serial device
# 2. Produce NMEA datagrams to the specified IPv6 host and port.
# 3. Report on standard output.

# usage: producer [ DEVICE [ SPEED [ HOST [ PORT ] ] ] ]

. $(readlink -e $(dirname ${0})/../bin)/setup

DEVICE=${1:-"/dev/ttyUSB0"}
SPEED=${2:-"4800"}
HOST=${3:-"ip6-localhost"}
PORT=${4:-"5555"}

stty sane
clear

exec gpstool -D ${DEVICE} -b ${SPEED} -8 -n -1 -6 -c -A ${HOST} -P ${PORT} -E
