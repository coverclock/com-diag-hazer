#!/bin/bash
# Copyright 2019 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

# This script is specific to the Ardusimple SimpleRTK2B rover (mobile) side.

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-9600}

. $(readlink -e $(dirname ${0})/../bin)/setup

# UBX-CFG-VALGET [ 8] V0 RAM 0 0 CFG-NMEA-PROTVER : 41 = "4.10"
# UBX-CFG-VALSET [ 9] V0 RAM 0 0 CFG-TMODE-MODE DISABLED

exec coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -E -t 10 \
    -U '\xb5\x62\x06\x8b\x08\x00\x00\x00\x00\x00\x01\x00\x93\x20' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x00\x00\x00\x01\x00\x03\x20\x00' \
    2> >(log -S -N ${PROGRAM})
