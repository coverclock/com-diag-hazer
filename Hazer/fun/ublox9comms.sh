#!/bin/bash
# Copyright 2019 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

# Display the COMMS configuration of the UBX device.

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-38400}

. $(readlink -e $(dirname ${0})/../bin)/setup

# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-I2C-ENABLED 0
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-SPI-ENABLED 0
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-UART1-ENABLED 0
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-UART2-ENABLED 0
# UBX-MON-COMMS [0]
# UBX-MON-TXBUF [0] (Deprecated)

exec coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 \
	-U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x03\x00\x51\x10\x00' \
	-U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x06\x00\x64\x10\x00' \
	-U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x05\x00\x52\x10\x00' \
	-U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x05\x00\x53\x10\x00' \
	-W '\xb5\x62\x0a\x36\x00\x00' \
	-W '\xb5\x62\x0a\x08\x00\x00' \
	-R -v
