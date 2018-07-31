#!/bin/bash
# Copyright 2018 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

# 1. Consume NMEA sentences from the specified serial device
# 2. Produce NMEA datagrams to the specified IPv6 host and port.
# 3. Report on standard output.

. $(readlink -e $(dirname ${0})/../bin)/setup

SPEED=${1:-"9600"}
DEVICE=${2:-"/dev/ttyUSB0"}
PORT=${3:-"5555"}
HOST=${4:-"ip6-localhost"}

stty sane
clear

exec gpstool -D ${DEVICE} -b ${SPEED} -8 -n -1 -6 -c -A ${HOST} -P ${PORT} -E
