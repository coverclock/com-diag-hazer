#!/bin/bash
# Copyright 2019-2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Configure and run the U-blox ZED-UBX-F9P as a mobile Rover receiving
# corrections from a stationary Base.

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}
mkdir -p ${SAVDIR}

PROGRAM=$(basename ${0})
ROUTER=${1:-"tumbleweed:tumbleweed"}
DEVICE=${2:-"/dev/tumbleweed"}
RATE=${3:-230400}
ERRFIL=${4-"${SAVDIR}/${PROGRAM}.err"}
OUTFIL=${5-"${SAVDIR}/${PROGRAM}.out"}

mkdir -p $(dirname ${ERRFIL})
mkdir -p $(dirname ${OUTFIL})

cp /dev/null ${ERRFIL}
exec 2>>${ERRFIL}

. $(readlink -e $(dirname ${0})/../bin)/setup

# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-TMODE-MODE DISABLED
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-I2C-ENABLED 0
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-SPI-ENABLED 0
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-UART1-ENABLED 0
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-UART2-ENABLED 0
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-USBINPROT-RTCM3X 1
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-USBOUTPROT-RTCM3X 0
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-MSGOUT-UBX_RXM_RTCM_USB 1Hz
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-MSGOUT-NMEA_ID_RMC_USB 1Hz
# UBX-CFG-MSG [3] UBX-NAV-HPPOSLLH 1

# Add for debugging:
# UBX-CFG-MSG [3] UBX-MON-COMMS 240 (4min @ 1Hz)
#    -U '\xb5\x62\x06\x01\x03\x00\x0a\x36\xf0' \

exec coreable gpstool \
    -H ${OUTFIL} -t 10 \
    -D ${DEVICE} -b ${RATE} -8 -n -1 \
    -Y ${ROUTER} -y 20 \
    -x \
    -A '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x01\x00\x03\x20\x00' \
    -A '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x03\x00\x51\x10\x00' \
    -A '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x06\x00\x64\x10\x00' \
    -A '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x05\x00\x52\x10\x00' \
    -A '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x05\x00\x53\x10\x00' \
    -A '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x04\x00\x77\x10\x01' \
    -A '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x04\x00\x78\x10\x00' \
    -A '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x6b\x02\x91\x20\x01' \
    -A '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\xae\x00\x91\x20\x01' \
    -A '\xb5\x62\x06\x01\x03\x00\x01\x14\x01' \
    < /dev/null 1> /dev/null
