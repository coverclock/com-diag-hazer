#!/bin/bash
# Copyright 2018-2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
#
# This script tests the NaviSys Technology GR-701W, a USB GPS
# device that is based on a Ublox 7 receiver. The GR-701W is
# unusual amongst GPS devices that have a USB interface in that
# it exports the 1PPS signal via data carrier detect (DCD), which
# is the reason for the "-c" option below. This script also
# configures the gpstool to forward the 1PPS signal by strobing a
# GPIO pin, the "-p" option. In this case, it uses GPIO pin 20 on
# a hardware test fixture I fabricated. I run this script on a
# Raspberry Pi as user "pi", which has access to the GPIO pins
# by virtue of being in group "gpio".

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyUSB0"}
RATE=${2:-9600}
STROBE=${3:-"-20"}

. $(readlink -e $(dirname ${0})/../bin)/setup

DIR=$(readlink -e $(dirname ${0})/..)/log
mkdir -p ${DIR}

export COM_DIAG_DIMINUTO_LOG_MASK=0xfe

coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -c  -p ${STROBE} -E -t 10 2>> ${DIR}/${PROGRAM}.err
