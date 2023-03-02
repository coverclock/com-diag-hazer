#!/bin/bash
# Copyright 2022 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Software reset the zedf9t.
# Probably best to power cycle device after this.
# This is part of the "Metronome" project.

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}

PROGRAM=$(basename ${0%-*})
DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-38400}

. $(readlink -e $(dirname ${0})/../bin)/setup

# UBX-CFG-RST

exec gpstool \
    -D ${DEVICE} -b ${RATE} -8 -n -1 -R \
    -x \
    -U '\xb5\x62\x06\x04'"$(ubxval -2 4)$(ubxval -2 0xffff)$(ubxval -1 0x01)"'0x00' \
    -U ''
