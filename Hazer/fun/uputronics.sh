#!/bin/bash
# Copyright 2019 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# N.B. Probably needs to be ruy as root for GPIO access.

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyAMA0"}
RATE=${2:-9600}
ONEPPS=${3:-18}

. $(readlink -e $(dirname ${0})/../bin)/setup

LOG=$(readlink -e $(dirname ${0})/..)/log
mkdir -p ${LOG}

export COM_DIAG_DIMINUTO_LOG_MASK=0xfe

stty sane

coreable pintool -p ${ONEPPS} -n 2> >(log -S -N ${PROGRAM}) || true

coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -I ${ONEPPS} -E -t 10 2> ${LOG}/${PROGRAM}.err
