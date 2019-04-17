#!/bin/bash
# Copyright 2019 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Configure and run the Ardusimple SimpleRTK2B as a mobile Rover.

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-9600}

. $(readlink -e $(dirname ${0})/../bin)/setup

# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-TMODE-MODE DISABLED
# UBX-CFG-VALSET [12] V0 RAM 0 0 CFG-UART2-BAUDRATE 38400
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-UART2-STOPBITS 1 (1)
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-UART2-DATABITS 0 (8)
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-UART2-PARITY 0 (none)
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-UART2-ENABLED 1
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-UART2INPROT-RTCM3X 1
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-UART2OUTPROT-RTCM3X 0
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-MSGOUT-UBX_RXM_RTCM_USB 1

gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x01\x00\x03\x20\x00' \
    -U '\xb5\x62\x06\x8a\x0c\x00\x00\x01\x00\x00\x01\x00\x53\x40\x00\x96\x00\x00' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x02\x00\x53\x20\x01' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x03\x00\x53\x20\x00' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x04\x00\x53\x20\x00' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x05\x00\x53\x10\x01' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x04\x00\x75\x10\x01' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x04\x00\x76\x10\x00' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x6b\x02\x91\x20\x01' \
    -W '' \
     2> >(log -S -N ${PROGRAM}) || exit 1

exec gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -F -x -t 10 \
    1> /dev/tty 2> >(log -S -N ${PROGRAM})
