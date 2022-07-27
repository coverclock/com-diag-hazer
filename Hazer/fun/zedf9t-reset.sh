#!/bin/bash
# Copyright 2022 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Clear Flash and Battery-Backed RAM (BBR) configurations (if any)
# in the ZED-F9T, and software reset the device.
# This is part of the "Metronome" project.

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}

PROGRAM=$(basename ${0%-*})
DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-38400}

. $(readlink -e $(dirname ${0})/../bin)/setup

# UBX-CFG-CFG
# UBX-CFG-RST

exec gpstool \
    -D ${DEVICE} -b ${RATE} -8 -n -1 -R \
    -x \
    -A '\xb5\x62\x06\x09'"$(ubxval -2 13)$(ubxval -4 0xffffffff)$(ubxval -4 0x00000000)$(ubxval -4 0x00000000)$(ubxval -1 0x03)" \
    -U '\xb5\x62\x06\x04'"$(ubxval -2 4)$(ubxval -2 0xffff)$(ubxval -1 0x01)"'0x00' \
    -U ''
