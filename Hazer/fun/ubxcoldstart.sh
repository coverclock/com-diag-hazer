#!/bin/bash
# Copyright 2021 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

PROGRAM=$(basename -s .sh ${0})
DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-115200}
INTERVAL=${3:-1}
WRITE=${4:-5}

. $(readlink -e $(dirname ${0})/../bin)/setup

# UBX-CFG-RST COLDSTART

exec coreable gpstool \
	-D ${DEVICE} -b ${RATE} -8 -n -1 -m -i ${INTERVAL} \
	-w ${WRITE} \
	-U '\xb5\x62\x06\x04\x04\x00\xff\xff\x04\x00' \
	-U '' \
	-v
