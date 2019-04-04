#!/bin/bash
# Copyright 2019 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

# This script is specific to the Ardusimple SimpleRTK2B base (stationary) side.

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-9600}

. $(readlink -e $(dirname ${0})/../bin)/setup

exec coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -E -t 10 \
    -U '\xb5\x62\x06\x8b\x08\x00\0x00\0x00\x00\x00\x20\x93\x00\x01' \
    -U '\xb5\x62\x06\x8b\x08\x00\0x00\0x00\x00\x00\x01\x00\x93\x20' \
	2> >(log -S -N ${PROGRAM})
