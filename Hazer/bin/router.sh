#!/bin/bash
# Copyright 2019 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

PROGRAM=$(basename ${0})
ROUTER=${1:-"localhost:21010"}
DEVICE=${2:-"/dev/ttyACM0"}
RATE=${3:-115200}

. $(readlink -e $(dirname ${0})/../bin)/setup

LOG=$(readlink -e $(dirname ${0})/..)/log
mkdir -p ${LOG}

exec coreable rtktool -p ${ROUTER} -t 30 < /dev/null 1> /dev/null 2> ${LOG}/${PROGRAM}.err
