#!/bin/bash
# Copyright 2022 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Reload the ZED-F9T from non-persistent configuration (if any).
# This is part of the "Metronome" project.

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}

PROGRAM=$(basename ${0%-*})
DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-38400}

. $(readlink -e $(dirname ${0})/../bin)/setup

# UBX-CFG-CFG

exec gpstool \
    -D ${DEVICE} -b ${RATE} -8 -n -1 -R \
    -x \
    -A '\xb5\x62\x06\x09'"$(ubxval -2 13)$(ubxval -4 0x00000000)$(ubxval -4 0x00000000)$(ubxval -4 0xffffffff)$(ubxval -1 0xff)" \
    -U ''
