#!/bin/bash
# Copyright 2021 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

PROGRAM=$(basename -s .sh ${0})
DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-115200}
INTERVAL=${3:-1}

. $(readlink -e $(dirname ${0})/../bin)/setup

# UBX-CFG-CFG CLEAR ALL-BUT-IOPORT LOAD ALL-BUT-IOPORT BBR

exec coreable gpstool \
	-D ${DEVICE} -b ${RATE} -8 -n -1 -m -i ${INTERVAL} \
	-x \
	-A '\xb5\x62\x06\x09\x0d\x00\x1e\x1f\x00\x00\x00\x00\x00\x00\x1e\x1f\x00\x00\x01' \
	-A '' \
	-v
