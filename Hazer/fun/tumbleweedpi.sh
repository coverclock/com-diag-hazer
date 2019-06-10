#!/bin/bash
# Copyright 2019 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-115200}

. $(readlink -e $(dirname ${0})/../bin)/setup

DIR=$(readlink -e $(dirname ${0})/..)/log
mkdir -p ${DIR}

exec coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -F -H ${DIR}/${PROGRAM}.out -t 10 2> ${DIR}/${PROGRAM}.log
