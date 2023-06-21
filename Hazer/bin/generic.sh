#!/bin/bash
# Copyright 2018-2023 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

# 1. Consume input from the specified serial device.
# 2. Report on standard output.

# usage: generic [ DEVICE [ RATE ] ]

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-"9600"}

LOG=$(readlink -e $(dirname ${0})/..)/tmp
mkdir -p ${LOG}

. $(readlink -e $(dirname ${0})/../bin)/setup

stty sane
clear

exec coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -E 2> ${LOG}/${PROGRAM}.err
