#!/bin/bash
# Copyright 2018 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

# I use a GlobalSat BU-353W10 USB GPS device for this.
# The BU-353W10 uses a U-Blox 8 chipset which has multiple
# RF stages, enabling it to receive GPS and GLONASS GNSS
# signals simultaneously. This script enables debug output.

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-9600}

. $(readlink -e $(dirname ${0})/../bin)/setup

. $(readlink -e $(dirname ${0})/../fun)/ubx8

OPTIONS=""
for OPTION in ${COMMANDS}; do
    OPTIONS="${OPTIONS} ${OPTION}"
done

eval coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -R ${OPTIONS} -d
