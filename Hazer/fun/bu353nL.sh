#!/bin/bash
# Copyright 2023 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# This script is specific to the GlobalSat BU-353N.

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyUSB0"}
RATE=${2:-4800}

. $(readlink -e $(dirname ${0})/../bin)/setup

LOG=$(readlink -e $(dirname ${0})/..)/tmp
mkdir -p ${LOG}

coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -E -t 10 -L ${LOG}/${PROGRAM}.lst -W '\$PAIR020' 2> ${LOG}/${PROGRAM}.err
