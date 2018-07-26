#!/bin/bash

# 1. Consume NMEA sentences from the specified serial device
# 2. Report on standard output.

. $(readlink -e $(dirname ${0})/../bin)/setup

SPEED=${1:-"9600"}
DEVICE=${2:-"/dev/ttyUSB0"}

stty sane
clear

exec gpstool -D ${DEVICE} -b ${SPEED} -8 -n -1 -E
