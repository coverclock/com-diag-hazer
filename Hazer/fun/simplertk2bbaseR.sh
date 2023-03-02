#!/bin/bash
# Copyright 2019 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

# This script is specific to the Ardusimple SimpleRTK2B base (stationary) side.

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-9600}

. $(readlink -e $(dirname ${0})/../bin)/setup

# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-TMODE-MODE SURVEY_IN
# UBX-CFG-VALSET [12] V0 RAM 0 0 CFG-TMODE-SVIN-MIN-DUR 60 (seconds)
# UBX-CFG-VALSET [12] V0 RAM 0 0 CFG-TMODE-SVIN-ACC-LIMIT 100000 (x 0.1mm)
# UBX-CFG-VALSET [12] V0 RAM 0 0 CFG-UART2-BAUDRATE 38400
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-UART2-STOPBITS 1 (1)
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-UART2-DATABITS 0 (8)
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-UART2-PARITY 0 (none)
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-UART2-ENABLED 1
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-MSGOUT-RTCM_3X_TYPE1005_UART2 1
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-MSGOUT-RTCM_3X_TYPE1074_UART2 1
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-MSGOUT-RTCM_3X_TYPE1084_UART2 1
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-MSGOUT-RTCM_3X_TYPE1094_UART2 1
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-MSGOUT-RTCM_3X_TYPE1124_UART2 1
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-MSGOUT-RTCM_3X_TYPE1230_UART2 1
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-UART2INPROT-RTCM3X 0
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-UART2OUTPROT-RTCM3X 1
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-MSGOUT-UBX_NAV_SVIN_USB 1
# UBX-CFG-MSG [3] UBX-NAV-HPPOSLLH 1

exec coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -R \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x01\x00\x03\x20\x01' \
    -U '\xb5\x62\x06\x8a\x0c\x00\x00\x01\x00\x00\x10\x00\x03\x40\x3c\x00\x00\x00' \
    -U '\xb5\x62\x06\x8a\x0c\x00\x00\x01\x00\x00\x11\x00\x03\x40\xA0\x86\x01\x00' \
    -U '\xb5\x62\x06\x8a\x0c\x00\x00\x01\x00\x00\x01\x00\x53\x40\x00\x96\x00\x00' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x02\x00\x53\x20\x01' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x03\x00\x53\x20\x00' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x04\x00\x53\x20\x00' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x05\x00\x53\x10\x01' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\xbf\x02\x91\x20\x01' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x60\x03\x91\x20\x01' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x65\x03\x91\x20\x01' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x6a\x03\x91\x20\x01' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x6f\x03\x91\x20\x01' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x05\x03\x91\x20\x01' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x04\x00\x75\x10\x00' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x04\x00\x76\x10\x01' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x8b\x00\x91\x20\x01' \
    -U '\xb5\x62\x06\x01\x03\x00\x01\x14\x01' \
    -W ''
