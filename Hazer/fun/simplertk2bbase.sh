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

# UBX-CFG-VALGET [ 8] V0 RAM 0 0 CFG-NMEA-PROTVER : 41 = "4.10"
# UBX-CFG-VALSET [ 9] V0 RAM 0 0 CFG-TMODE-MODE SURVEY_IN
# UBX-CFG-VALSET [12] V0 RAM 0 0 CFG_TMODE_SVIN_MIN_DUR 86400 (seconds)
# UBX-CFG-VALSET [12] V0 RAM 0 0 CFG_TMODE_SVIN_ACC_LIMIT 1000 (0.1mm)

exec coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -E -t 10 \
    -U '\xb5\x62\x06\x8b\x08\x00\x00\x00\x00\x00\x01\x00\x93\x20' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x00\x00\x00\x01\x00\x03\x20\x01' \
    -U '\xb5\x62\x06\x8a\x0c\x00\x00\x00\x00\x00\x10\x00\x03\x40\x80\x51\x01\x00' \
    -U '\xb5\x62\x06\x8a\x0c\x00\x00\x00\x00\x00\x11\x00\x03\x40\xe8\x03\x00\x00' \
	2> >(log -S -N ${PROGRAM})
