#!/bin/bash
# Copyright 2019-2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
#
# This script tests the MakerFocus USB-Port-GPS, a USB GPS
# device that is based on a QuecTel L80-R receiver. The MakerFocus
# is unusual amongst GPS devices that have a USB interface in that
# it exports the 1PPS signal via via its own GPIO pin, although
# this script doesn't make use of that feature.

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyUSB0"}
RATE=${2:-9600}

. $(readlink -e $(dirname ${0})/../bin)/setup

LOG=$(readlink -e $(dirname ${0})/..)/log
mkdir -p ${LOG}

coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -E -t 10 2>> ${LOG}/${PROGRAM}.err
