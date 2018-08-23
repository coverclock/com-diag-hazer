#!/bin/bash
# Copyright 2018 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

# I use a Navlocate GN-803G USB GPS device for this.
# The GN-803G uses a U-Blox 8 chipset which has multiple
# RF stages, enabling it to receive GPS and GLONASS GNSS
# signals simultaneously.

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-9600}

. $(readlink -e $(dirname ${0})/../bin)/setup

. $(readlink -e $(dirname ${0})/../fun)/ubx8

OPTIONS=""
for OPTION in ${COMMANDS}; do
    OPTIONS="${OPTIONS} -W ${OPTION}"
done

DIR=$(readlink -e $(dirname ${0})/..)/log
mkdir -p ${DIR}

eval coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -E -t 10 ${OPTIONS} 2> ${DIR}/${PROGRAM}.log
