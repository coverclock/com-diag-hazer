#!/bin/bash
# Copyright 2021 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# This script script mimics the messaging that is emitted by the
# U-blox CAM-M8Q in the Lynq Technologies Smart Compass.

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/log}
mkdir -p ${SAVDIR}

PROGRAM=$(basename -s .sh ${0})
DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-9600}
INTERVAL=${3:-1}
WRITE=${4:-1}
ERRFIL=${5-"${SAVDIR}/${PROGRAM}.err"}
CATFIL=${6-"${SAVDIR}/${PROGRAM}.cat"}

mkdir -p $(dirname ${ERRFIL})
mkdir -p $(dirname ${CATFIL})

cp /dev/null ${ERRFIL}
cp /dev/null ${CATFIL}

exec 2>>${ERRFIL}

. $(readlink -e $(dirname ${0})/../bin)/setup

export COM_DIAG_DIMINUTO_LOG_MASK=0xff

# UBX-CFG-RATE 1Hz
# UBX-CFG-MSG NMEA STD GGA ENABLE
# UBX-CFG-MSG NMEA STD GLL DISABLE
# UBX-CFG-MSG NMEA STD GSA DISABLE
# UBX-CFG-MSG NMEA STD GSV DISABLE
# UBX-CFG-MSG NMEA STD RMC ENABLE
# UBX-CFG-MSG NMEA STD VTG DISABLE
# UBX-CFG-MSG NMEA STD GRS DISABLE
# UBX-CFG-MSG NMEA STD GST DISABLE
# UBX-CFG-MSG NMEA STD ZDA DISABLE
# UBX-CFG-MSG NMEA STD GBS DISABLE
# UBX-CFG-MSG NMEA STD DTM DISABLE
# UBX-CFG-MSG NMEA STD GGA DISABLE
# UBX-CFG-MSG NMEA UBX PUBX 00 ENABLE
# UBX-CFG-MSG NMEA UBX PUBX 03 ENABLE
# UBX-CFG-MSG NMEA UBX PUBX 04 ENABLE
# UBX-CFG-GNSS GPS SBAS

# N.B. The u-blox module in the BU353W10 reports it uses Port 4 (one-based).
 
exec coreable gpstool \
	-D ${DEVICE} -b ${RATE} -8 -n -1 -i ${INTERVAL} \
	-C ${CATFIL} \
	-w ${WRITE} \
	-x \
	-A '\xB5\x62\x06\x08\x06\x00\xe8\x03\x01\x00\x01\x00' \
	-A '\xB5\x62\x06\x01\x08\x00\xF0\x00\x00\x00\x00\x01\x00\x00' \
	-A '\xB5\x62\x06\x01\x08\x00\xF0\x01\x00\x00\x00\x00\x00\x00' \
	-A '\xB5\x62\x06\x01\x08\x00\xF0\x02\x00\x00\x00\x00\x00\x00' \
	-A '\xB5\x62\x06\x01\x08\x00\xF0\x03\x00\x00\x00\x00\x00\x00' \
	-A '\xB5\x62\x06\x01\x08\x00\xF0\x04\x00\x00\x00\x01\x00\x00' \
	-A '\xB5\x62\x06\x01\x08\x00\xF0\x05\x00\x00\x00\x00\x00\x00' \
	-A '\xB5\x62\x06\x01\x08\x00\xF0\x06\x00\x00\x00\x00\x00\x00' \
	-A '\xB5\x62\x06\x01\x08\x00\xF0\x07\x00\x00\x00\x00\x00\x00' \
	-A '\xB5\x62\x06\x01\x08\x00\xF0\x08\x00\x00\x00\x00\x00\x00' \
	-A '\xB5\x62\x06\x01\x08\x00\xF0\x09\x00\x00\x00\x00\x00\x00' \
	-A '\xB5\x62\x06\x01\x08\x00\xF0\x0a\x00\x00\x00\x00\x00\x00' \
	-A '\xB5\x62\x06\x01\x08\x00\xF0\x00\x00\x00\x00\x00\x00\x00' \
	-A '\xB5\x62\x06\x01\x08\x00\xF1\x00\x00\x00\x00\x01\x00\x00' \
	-A '\xB5\x62\x06\x01\x08\x00\xF1\x03\x00\x00\x00\x01\x00\x00' \
	-A '\xB5\x62\x06\x01\x08\x00\xF1\x04\x00\x00\x00\x01\x00\x00' \
	-A '\xB5\x62\x06\x3E\x3C\x00\x00\x00\xFF\x07\x00\x08\x10\x00\x01\x00\x01\x00\x01\x01\x03\x00\x00\x00\x00\x00\x02\x04\x08\x00\x00\x00\x00\x00\x03\x08\x10\x00\x00\x00\x00\x00\x04\x00\x08\x00\x00\x00\x00\x00\x05\x00\x03\x00\x01\x00\x01\x00\x06\x08\x0e\x00\x01\x00\x01\x00' \
	-E -v
