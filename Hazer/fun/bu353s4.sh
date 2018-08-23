#!/bin/bash
# Copyright 2018 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

# I use a USGLobalSat BU-353S4 USB GPS device for this.
# The BU-353S4 uses a SiRF Star Iv chipset.

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyUSB0"}
RATE=${2:-4800}

. $(readlink -e $(dirname ${0})/../bin)/setup

DIR=$(readlink -e $(dirname ${0})/..)/log
mkdir -p ${DIR}

eval coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -E -t 10 2> ${DIR}/${PROGRAM}.log
