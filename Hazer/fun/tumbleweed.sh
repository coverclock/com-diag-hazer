#!/bin/bash
# Copyright 2019 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

# This script is specific to the Ardusimple SimpleRTK2B.

PROGRAM=$(basename ${0})
DATAGRAM=${1:-"localhost"}
SURVEYOR=${2:-"localhost"}
DEVICE=${3:-"/dev/ttyACM0"}
RATE=${4:-115200}

. $(readlink -e $(dirname ${0})/../bin)/setup

DIR=$(readlink -e $(dirname ${0})/..)/log
mkdir -p ${DIR}

exec coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -G ${DATAGRAM}:5555 -g 4 -Y ${SURVEYOR}:2101 -y 10 -E -F -t 10 2> ${DIR}/${PROGRAM}.log
