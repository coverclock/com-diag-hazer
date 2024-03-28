#!/bin/bash
# Copyright 2019-2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# N.B. May need to be run as root for GPIO access.

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyAMA0"}
RATE=${2:-9600}
ONEPPS=${3:-"/dev/gpiochip4:18"}

. $(readlink -e $(dirname ${0})/../bin)/setup

LOG=$(readlink -e $(dirname ${0})/..)/log
mkdir -p ${LOG}

stty sane

coreable pintool -p ${ONEPPS} -e
coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -I ${ONEPPS} -E -t 10 2> ${LOG}/${PROGRAM}.err
