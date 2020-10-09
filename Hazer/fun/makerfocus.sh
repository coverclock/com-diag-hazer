#!/bin/bash
# Copyright 2019-2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
#
# This script tests the MakerFocus USB-Port-GPS, a USB GPS
# device that is based on a QuecTel L80-R receiver. The MakerFocus
# is unusual amongst GPS devices that have a USB interface in that
# it exports the 1PPS signal via via its own GPIO pin, which
# is the reason for the "-I" option below. This script also
# configures the gpstool to forward the 1PPS signal by strobing a
# GPIO pin, the "-p" option. In this case, it uses GPIO pin 16 on
# a hardware test fixture I fabricated. I run this script on a
# Raspberry Pi as user "pi", which has access to the GPIO pins
# by virtue of being in group "gpio".

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyUSB0"}
RATE=${2:-9600}
ONEPPS=${3:-18}
STROBE=${4:-16}

. $(readlink -e $(dirname ${0})/../bin)/setup

LOG=$(readlink -e $(dirname ${0})/..)/log
mkdir -p ${LOG}

export COM_DIAG_DIMINUTO_LOG_MASK=0xfe

coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -I ${ONEPPS} -p ${STROBE} -E -t 10 2>> ${LOG}/${PROGRAM}.err
