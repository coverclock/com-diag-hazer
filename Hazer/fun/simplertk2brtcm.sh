#!/bin/bash
# Copyright 2019 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

# This script is specific to the Ardusimple SimpleRTK2B and its separate RTCM
# output stream available via the XBee3 FTDI port when in base mode and when
# the hardware is modified correctly (XBee3 TX tied to Xbee3 RX, IOREF tied
# to 3.3V). Note the different DEVICE name and higher data RATE.

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyUSB0"}
RATE=${2:-38400}

. $(readlink -e $(dirname ${0})/../bin)/setup

LOG=$(readlink -e $(dirname ${0})/..)/log
mkdir -p ${LOG}

export COM_DIAG_DIMINUTO_LOG_MASK=${COM_DIAG_DIMINUTO_LOG_MASK:=0xfe}

exec coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -E -t 10 2> ${LOG}/${PROGRAM}.err
