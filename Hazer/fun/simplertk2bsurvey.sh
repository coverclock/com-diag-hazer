#!/bin/bash
# Copyright 2019 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

# This script is specific to the Ardusimple SimpleRTK2B in standalone survey
# mode in which the RTCM (Tumbleweed) messages are emitted in the same output
# stream as the NMEA (Hazer) sentences and the UBX (Yodel) packets, all of which
# can be recognized by gpstool.

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-9600}

. $(readlink -e $(dirname ${0})/../bin)/setup

# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-TMODE-MODE SURVEY_IN
# UBX-CFG-VALSET [12] V0 RAM 0 0 CFG-TMODE-SVIN-MIN-DUR 300 (seconds)
# UBX-CFG-VALSET [12] V0 RAM 0 0 CFG-TMODE-SVIN-ACC-LIMIT 10000 (x 0.1mm)
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-MSGOUT-RTCM_3X_TYPE1005_USB 1
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-MSGOUT-RTCM_3X_TYPE1074_USB 1
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-MSGOUT-RTCM_3X_TYPE1084_USB 1
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-MSGOUT-RTCM_3X_TYPE1094_USB 1
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-MSGOUT-RTCM_3X_TYPE1124_USB 1
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-MSGOUT-RTCM_3X_TYPE1230_USB 1
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-USBINPROT-RTCM3X 0
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-USBOUTPROT-RTCM3X 1
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-MSGOUT-UBX_NAV_SVIN_USB 1
# UBX-CFG-MSG [3] UBX-NAV-HPPOSLLH 1

exec coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x01\x00\x03\x20\x01' \
    -U '\xb5\x62\x06\x8a\x0c\x00\x00\x01\x00\x00\x10\x00\x03\x40\x2c\x01\x00\x00' \
    -U '\xb5\x62\x06\x8a\x0c\x00\x00\x01\x00\x00\x11\x00\x03\x40\x10\x27\x00\x00' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\xc0\x02\x91\x20\x01' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x61\x03\x91\x20\x01' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x66\x03\x91\x20\x01' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x6b\x03\x91\x20\x01' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x70\x03\x91\x20\x01' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x06\x03\x91\x20\x01' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x04\x00\x77\x10\x00' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x04\x00\x78\x10\x01' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x8b\x00\x91\x20\x01' \
    -U '\xb5\x62\x06\x01\x03\x00\x01\x14\x01' \
    -W ''
