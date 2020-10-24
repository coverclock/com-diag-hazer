#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

# This script tests the u-blox NEO-M8N GPS module, in my case on
# a SparkFun NEO-M8N board.-GPS, The SparkFun board has a USB
# interface and the NEO-M8N provides 1PPS via a GPIO pin, which
# is the reason for the "-I" option below. This script also
# configures the gpstool to forward the 1PPS signal by strobing a
# GPIO pin, the "-p" option. In this case, it uses GPIO pin 16 on
# a hardware test fixture I fabricated. I run this script on a
# Raspberry Pi as user "pi", which has access to the GPIO pins
# by virtue of being in group "gpio".

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-115200}
ONEPPS=${3:-18}
STROBE=${4:-16}

. $(readlink -e $(dirname ${0})/../bin)/setup

LOG=$(readlink -e $(dirname ${0})/..)/log
mkdir -p ${LOG}

export COM_DIAG_DIMINUTO_LOG_MASK=0xfe

coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -I ${ONEPPS} -p ${STROBE} -E -t 10 2>> ${LOG}/${PROGRAM}.err
