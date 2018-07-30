#!/bin/bash

# 1. Consume NMEA datagrams from the specified IPv6 port.
# 2. Produce NMEA sentences to the specified serial device.
# 3. Report on standard output.

. $(readlink -e $(dirname ${0})/../bin)/setup

SPEED=${1:-"9600"}
DEVICE=${2:-"/dev/ttyS0"}
PORT=${3:-"5555"}

stty sane
clear

exec gpstool -6 -P ${PORT} -D ${DEVICE} -b ${SPEED} -8 -n -1 -O -E
