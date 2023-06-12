#!/bin/bash
# Copyright 2023 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# usage: gps18xpcbinary [ DEVICE [ RATE ] ]
# THIS IS A WORK IN PROGRESS
# Script for the Garmin GPS-18x PC; this is the unit with the DB9 RS-232
# connector and the cigarette lighter socket power plug. It places the
# device in binary output mode.

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyUSB0"}
RATE=${2:-"4800"}

LOG=$(readlink -e $(dirname ${0})/..)/tmp
mkdir -p ${LOG}

. $(readlink -e $(dirname ${0})/../bin)/setup

exec coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -E 2 \
	-W '$PGRMC1,1,2,,,,,2,W,N,,,,1,,1' \
	2> ${LOG}/${PROGRAM}.err
