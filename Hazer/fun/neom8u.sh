#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# I'm using this with a SparkFun board with the dead reckining
# u-blox NEO-M8U module.

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-9600}

. $(readlink -e $(dirname ${0})/../bin)/setup

. $(readlink -e $(dirname ${0})/../fun)/ubx8

OPTIONS=""
for OPTION in ${COMMANDS}; do
    OPTIONS="${OPTIONS} -W ${OPTION}"
done

export COM_DIAG_DIMINUTO_LOG_MASK=${COM_DIAG_DIMINUTO_LOG_MASK:=0xfe}

eval coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -F ${OPTIONS} 2>> ERRORS
