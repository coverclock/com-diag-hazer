
#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Configure and run the U-blox ZED-UBX-F9R as a mobile Rover receiving
# corrections from a stationary Base and saving high precision solutions
# to a CSV file.

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}
mkdir -p ${SAVDIR}

PROGRAM=$(basename ${0})
ROUTER=${1:-"tumbleweed:tumbleweed"}
DEVICE=${2:-"/dev/ttyACM0"}
RATE=${3:-230400}
ERRFIL=${4-"${SAVDIR}/${PROGRAM}.err"}
OUTFIL=${5-"${SAVDIR}/${PROGRAM}.out"}
CSVFIL=${6-"${SAVDIR}/${PROGRAM}.csv"}
PIDFIL=${7-"${SAVDIR}/${PROGRAM}.pid"}

mkdir -p $(dirname ${ERRFIL})
mkdir -p $(dirname ${OUTFIL})
mkdir -p $(dirname ${CSVFIL})
mkdir -p $(dirname ${PIDFIL})

cp /dev/null ${ERRFIL}
exec 2>>${ERRFIL}

. $(readlink -e $(dirname ${0})/../bin)/setup

export COM_DIAG_DIMINUTO_LOG_MASK=${COM_DIAG_DIMINUTO_LOG_MASK:=0xfe}

# UBX-MON-VER [0]
# NMEA-PUBX-POSITION
# NMEA-PUBX-SVSTATUS
# NMEA-PUBX-TIME
# NMEA-PUBX-RATE GSV @1Hz
# NMEA-PUBX-RATE VTG @1Hz
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-I2C-ENABLED 0
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-SPI-ENABLED 0
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-UART1-ENABLED 0
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-UART2-ENABLED 0
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-USBINPROT-RTCM3X 1
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-MSGOUT-UBX_RXM_RTCM_USB 1
# UBX-CFG-MSG [3] UBX-NAV-HPPOSLLH 1
# UBX-CFG-PRT [0] (all)
# UBX-CFG-MSG [3] UBX-MON-HW @1Hz (ublox8 > fw18)
# UBX-CFG-MSG [3] UBX-NAV-STATUS @1Hz (ublox8 > fw18)
# UBX-CFG-MSG [3] UBX-NAV-ATT @1Hz (ublox8 > fw19)
# UBX-CFG-ODO [20] 0x01 3 1 50 153 76
# UBX-NAV-RESETODO [0]
# UBX-CFG-MSG [3] UBX-NAV-ODO @1Hz (ublox8 > fw15)
# UBX-CFG-MSG [3] UBX-NAV-PVT @1Hz (ublox8 > fw15)
# UBX-CFG-ITFM [8] LE(0x96b156YX) LE(0x0000631e) (X[4]=broadband threshold signed dB, Y[5]=continuous wave threshold signed dB)
# UBX-CFG-DAT [0]
# UBX-CFG-TPS [0]
# UBX-CFG-GNSS [0]

exec coreable gpstool \
    -H ${OUTFIL} -t 10 \
    -T ${CSVFIL} \
    -O ${PIDFIL} \
    -D ${DEVICE} -b ${RATE} -8 -n -1 \
    -Y ${ROUTER} -y 20 \
    -x \
    -U '\xb5\x62\x0a\x04\x00\x00' \
    -W '$PUBX,00' \
    -W '$PUBX,03' \
    -W '$PUBX,04' \
    -W '$PUBX,40,GSV,0,0,0,1,0,0' \
    -W '$PUBX,40,VTG,0,0,0,1,0,0' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x03\x00\x51\x10\x00' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x06\x00\x64\x10\x00' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x05\x00\x52\x10\x00' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x05\x00\x53\x10\x00' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x04\x00\x77\x10\x01' \
    -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x6b\x02\x91\x20\x01' \
    -U '\xb5\x62\x06\x01\x03\x00\x01\x14\x01' \
    -U '\xb5\x62\x06\x00\x00\x00' \
    -U '\xb5\x62\x06\x01\x03\x00\x0a\x09\x01' \
    -U '\xb5\x62\x06\x01\x03\x00\x01\x03\x01' \
    -U '\xb5\x62\x06\x01\x03\x00\x01\x05\x01' \
    -U '\xb5\x62\x06\x1e\x14\x00\x00\x00\x00\x00\x01\x03\x00\x00\x00\x00\x00\x00\x01\x32\x00\x00\x99\x4c\x00\x00' \
    -U '\xb5\x62\x01\x10\x00\x00' \
    -U '\xb5\x62\x06\x01\x03\x00\x01\x09\x01' \
    -U '\xb5\x62\x06\x01\x03\x00\x01\x07\x01' \
    -U '\xb5\x62\x06\x39\x08\x00\xf7\x56\xb1\x96\x1e\x63\x00\x00' \
    -U '\xb5\x62\x06\x06\x00\x00' \
    -U '\xb5\x62\x06\x31\x00\x00' \
    -U '\xb5\x62\x06\x3e\x00\x00' \

    < /dev/null 1> /dev/null
