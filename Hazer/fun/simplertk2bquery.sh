#!/bin/bash
# Copyright 2019 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

# This script is specific to the Ardusimple SimpleRTK2B.

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-9600}

. $(readlink -e $(dirname ${0})/../bin)/setup

# UBX-MON-VER [0]
# UBX-CFG-VALGET [8] V0 RAM 0 0 CFG-NMEA-PROTVER
# UBX-CFG-VALGET [8] V0 RAM 0 0 CFG-TMODE-MODE
# UBX-CFG-VALGET [8] V0 RAM 0 0 CFG-TMODE-SVIN-MIN-DUR
# UBX-CFG-VALGET [8] V0 RAM 0 0 CFG-TMODE-SVIN-ACC-LIMIT
# UBX-CFG-VALGET [8] V0 RAM 0 0 CFG-UART2-BAUDRATE
# UBX-CFG-VALGET [8] V0 RAM 0 0 CFG-UART2-STOPBITS
# UBX-CFG-VALGET [8] V0 RAM 0 0 CFG-UART2-DATABITS
# UBX-CFG-VALGET [8] V0 RAM 0 0 CFG-UART2-PARITY
# UBX-CFG-VALGET [8] V0 RAM 0 0 CFG-UART2-ENABLED
# UBX-CFG-VALGET [8] V0 RAM 0 0 CFG-MSGOUT-RTCM_3X_TYPE1005_UART2
# UBX-CFG-VALGET [8] V0 RAM 0 0 CFG-MSGOUT-RTCM_3X_TYPE1074_UART2
# UBX-CFG-VALGET [8] V0 RAM 0 0 CFG-MSGOUT-RTCM_3X_TYPE1084_UART2
# UBX-CFG-VALGET [8] V0 RAM 0 0 CFG-MSGOUT-RTCM_3X_TYPE1094_UART2
# UBX-CFG-VALGET [8] V0 RAM 0 0 CFG-MSGOUT-RTCM_3X_TYPE1124_UART2
# UBX-CFG-VALGET [8] V0 RAM 0 0 CFG-MSGOUT-RTCM_3X_TYPE1230_UART2
# UBX-CFG-VALGET [8] V0 RAM 0 0 CFG-UART2INPROT-RTCM3X
# UBX-CFG-VALGET [8] V0 RAM 0 0 CFG-UART2OUTPROT-RTCM3X
# UBX-CFG-VALGET [8] V0 RAM 0 0 CFG-MSGOUT-UBX_NAV_SVIN_UART1
# UBX-CFG-VALGET [8] V0 RAM 0 0 CFG-MSGOUT-UBX_RXM_RTCM_UART1

exec coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 \
    -W '\xb5\x62\x0a\x04\x00\x00' \
    -U '\xb5\x62\x06\x8b\x08\x00\x00\x00\x00\x00\x01\x00\x93\x20' \
    -U '\xb5\x62\x06\x8b\x08\x00\x00\x00\x00\x00\x01\x00\x03\x20' \
    -U '\xb5\x62\x06\x8b\x08\x00\x00\x00\x00\x00\x10\x00\x03\x40' \
    -U '\xb5\x62\x06\x8b\x08\x00\x00\x00\x00\x00\x11\x00\x03\x40' \
    -U '\xb5\x62\x06\x8b\x08\x00\x00\x00\x00\x00\x01\x00\x53\x40' \
    -U '\xb5\x62\x06\x8b\x08\x00\x00\x00\x00\x00\x02\x00\x53\x20' \
    -U '\xb5\x62\x06\x8b\x08\x00\x00\x00\x00\x00\x03\x00\x53\x20' \
    -U '\xb5\x62\x06\x8b\x08\x00\x00\x00\x00\x00\x04\x00\x53\x20' \
    -U '\xb5\x62\x06\x8b\x08\x00\x00\x00\x00\x00\x05\x00\x53\x10' \
    -U '\xb5\x62\x06\x8b\x08\x00\x00\x00\x00\x00\xbf\x02\x91\x20' \
    -U '\xb5\x62\x06\x8b\x08\x00\x00\x00\x00\x00\x60\x03\x91\x20' \
    -U '\xb5\x62\x06\x8b\x08\x00\x00\x00\x00\x00\x65\x03\x91\x20' \
    -U '\xb5\x62\x06\x8b\x08\x00\x00\x00\x00\x00\x6a\x03\x91\x20' \
    -U '\xb5\x62\x06\x8b\x08\x00\x00\x00\x00\x00\x6f\x03\x91\x20' \
    -U '\xb5\x62\x06\x8b\x08\x00\x00\x00\x00\x00\x05\x03\x91\x20' \
    -U '\xb5\x62\x06\x8b\x08\x00\x00\x00\x00\x00\x04\x00\x75\x10' \
    -U '\xb5\x62\x06\x8b\x08\x00\x00\x00\x00\x00\x04\x00\x76\x10' \
    -U '\xb5\x62\x06\x8b\x08\x00\x00\x00\x00\x00\x89\x00\x91\x20' \
    -U '\xb5\x62\x06\x8b\x08\x00\x00\x00\x00\x00\x69\x02\x91\x20' \
    -x
