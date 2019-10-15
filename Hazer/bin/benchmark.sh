#!/bin/bash
# Copyright 2019 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Configure and run the U-blox ZED-UBX-F9P as a mobile Rover receiving
# corrections from a stationary Base and saving high precision solutions
# to a CSV file (especially useful for testing on survey benchmarks).

PROGRAM=$(basename ${0})
ROUTER=${1:-"tumbleweed:tumbleweed"}
DEVICE=${2:-"/dev/tumbleweed"}
RATE=${3:-230400}
ERRFIL=${4-"./${PROGRAM}.err"}
OUTFIL=${5-"./${PROGRAM}.out"}
CSVFIL=${6-"./${PROGRAM}.csv"}

cp /dev/null ${ERRFIL}
exec 2>>${ERRFIL}

. $(readlink -e $(dirname ${0})/../bin)/setup

export COM_DIAG_DIMINUTO_LOG_MASK=0xfe

# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-TMODE-MODE DISABLED
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-UART2-ENABLED 0
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-USBINPROT-RTCM3X 1
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-USBOUTPROT-RTCM3X 0
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-MSGOUT-UBX_RXM_RTCM_USB 1
# UBX-CFG-MSG [3] UBX-NAV-HPPOSLLH 1

exec coreable gpstool \
    -F -H ${OUTFIL} -t 10 \
    -T ${CSVFIL} \
    -D ${DEVICE} -b ${RATE} -8 -n -1 \
    -Y ${ROUTER} -y 20 \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x01\x00\x03\x20\x00' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x05\x00\x53\x10\x00' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x04\x00\x77\x10\x01' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x04\x00\x78\x10\x00' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x6b\x02\x91\x20\x01' \
    -U '\xb5\x62\x06\x01\x03\x00\x01\x14\x01' \
    < /dev/null 1> /dev/null
