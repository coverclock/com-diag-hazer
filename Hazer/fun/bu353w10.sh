#!/bin/bash
# Copyright 2018 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

# This script is specific to the GlobalSat BU-353W10, but
# because that device uses the same U-Blox 8 chipset as
# the Navlocate GN-803G (but is easily acquireable via
# Amazon.com with two-day shipping, unilike the GN which
# comes from eBay by way of China), this script is the
# same as the one for that device.

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-9600}

. $(readlink -e $(dirname ${0})/../bin)/setup

DIR=$(readlink -e $(dirname ${0})/..)/log
mkdir -p ${DIR}

eval coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -E -t 10 2>> ${DIR}/${PROGRAM}.log
