#!/bin/bash
# Copyright 2019 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Configure and run the U-blox ZED-UBX-F9P as a stationary Base in fixed
# mode sending corrections to Rovers.
# IMPORTANT SAFETY TIP: when switching the F9P from FIXED back to SVIN mode,
# consider power cycling or otherwise resetting the device first.

SAVDIR=${COM_DIAG_HAZER_SAVE_DIR:-"./sav"}

PROGRAM=$(basename ${0})
ROUTER=${1:-"tumbleweed:tumbleweed"}
DEVICE=${2:-"/dev/tumbleweed"}
RATE=${3:-230400}
FIXFIL=${4-"./base.fix"}
ERRFIL=${5-"${SAVDIR}/${PROGRAM}.err"}
OUTFIL=${6-"${SAVDIR}/${PROGRAM}.out"}

mkdir -p $(dirname ${ERRFIL})
mkdir -p $(dirname ${OUTFIL})

cp /dev/null ${ERRFIL}
exec 2>>${ERRFIL}

. $(readlink -e $(dirname ${0})/../bin)/setup

export COM_DIAG_DIMINUTO_LOG_MASK=0xfe

test -f ${FIXFIL} || exit 2
exec 0<${FIXFIL}

read -r FIXED_POS_ACC
read -r LAT
read -r LATHP
read -r LON
read -r LONHP
read -r HEIGHT
read -r HEIGHTHP

log -I -N ${PROGRAM} -i FIXED_POS_ACC="\"${FIXED_POS_ACC}\""
log -I -N ${PROGRAM} -i LAT="\"${LAT}\""
log -I -N ${PROGRAM} -i LATHP="\"${LATHP}\""
log -I -N ${PROGRAM} -i LON="\"${LON}\""
log -I -N ${PROGRAM} -i LONHP="\"${LONHP}\""
log -I -N ${PROGRAM} -i HEIGHT="\"${HEIGHT}\""
log -I -N ${PROGRAM} -i HEIGHTHP="\"${HEIGHTHP}\""

test -z "${FIXED_POS_ACC}" && exit 3
test -z "${LAT}" && exit 3
test -z "${LATHP}" && exit 3
test -z "${LON}" && exit 3
test -z "${LONHP}" && exit 3
test -z "${HEIGHT}" && exit 3
test -z "${HEIGHTHP}" && exit 3

# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-TMODE-MODE DISABLED
# UBX-CFG-VALSET [12] V0 RAM 0 0 CFG-TMODE-LAT (read)
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-TMODE-LATHP (read)
# UBX-CFG-VALSET [12] V0 RAM 0 0 CFG-TMODE-LON (read)
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-TMODE-LONHP (read)
# UBX-CFG-VALSET [12] V0 RAM 0 0 CFG-TMODE-HEIGHT (read)
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-TMODE-HEIGHTHP (read)
# UBX-CFG-VALSET [12] V0 RAM 0 0 CFG-TMODE-FIXED_ACC_LIMIT (read)
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-TMODE-MODE FIXED
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-TMODE-POS_TYPE LLH
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-MSGOUT-RTCM_3X_TYPE1005_USB 1
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-MSGOUT-RTCM_3X_TYPE1074_USB 1
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-MSGOUT-RTCM_3X_TYPE1084_USB 1
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-MSGOUT-RTCM_3X_TYPE1094_USB 1
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-MSGOUT-RTCM_3X_TYPE1124_USB 1
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-MSGOUT-RTCM_3X_TYPE1230_USB 1
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-USBINPROT-RTCM3X 0
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-USBOUTPROT-RTCM3X 1
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-MSGOUT-UBX_NAV_SVIN_USB 1
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-I2C-ENABLED 0
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-SPI-ENABLED 0
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-UART1-ENABLED 0
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-UART2-ENABLED 0
# UBX-CFG-MSG [3] UBX-NAV-HPPOSLLH 1

exec coreable gpstool \
    -F -H ${OUTFIL} -t 10 \
    -D ${DEVICE} -b ${RATE} -8 -n -1 \
    -G ${ROUTER} -g 4 \
    -x \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x01\x00\x03\x20\x00' \
    -U '\xb5\x62\x06\x8a\x0c\x00\x00\x01\x00\x00\x09\x00\x03\x40'"${LAT}" \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x0c\x00\x03\x20'"${LATHP}" \
    -U '\xb5\x62\x06\x8a\x0c\x00\x00\x01\x00\x00\x0a\x00\x03\x40'"${LON}" \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x0d\x00\x03\x20'"${LONHP}" \
    -U '\xb5\x62\x06\x8a\x0c\x00\x00\x01\x00\x00\x0b\x00\x03\x40'"${HEIGHT}" \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x0e\x00\x03\x20'"${HEIGHTHP}" \
    -U '\xb5\x62\x06\x8a\x0c\x00\x00\x01\x00\x00\x0f\x00\x03\x40'"${FIXED_POS_ACC}" \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x01\x00\x03\x20\x02' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x02\x00\x03\x20\x01' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\xc0\x02\x91\x20\x01' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x61\x03\x91\x20\x01' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x66\x03\x91\x20\x01' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x6b\x03\x91\x20\x01' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x70\x03\x91\x20\x01' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x06\x03\x91\x20\x01' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x04\x00\x77\x10\x00' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x04\x00\x78\x10\x01' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x8b\x00\x91\x20\x01' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x03\x00\x51\x10\x00' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x06\x00\x64\x10\x00' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x05\x00\x52\x10\x00' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x05\x00\x53\x10\x00' \
    -U '\xb5\x62\x06\x01\x03\x00\x01\x14\x01' \
    < /dev/null 1> /dev/null
