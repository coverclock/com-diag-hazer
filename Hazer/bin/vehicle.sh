#!/bin/bash
# Copyright 2019-2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Configure and run the U-blox NEO-M8U, collecting the
# CSV dataset, and emitting just one line of output
# per interval.

##
## SETUP
##

SELF=$$

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}
mkdir -p ${SAVDIR}

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-9600}
ERRFIL=${3-"${SAVDIR}/${PROGRAM}.err"}
OUTFIL=${4-"${SAVDIR}/${PROGRAM}.out"}
CSVFIL=${5-"${SAVDIR}/${PROGRAM}.csv"}
PIDFIL=${6-"${SAVDIR}/${PROGRAM}.pid"}
LIMIT=${7:-$(($(stty size | cut -d ' ' -f 1) - 2))}

mkdir -p $(dirname ${ERRFIL})
mkdir -p $(dirname ${OUTFIL})
mkdir -p $(dirname ${CSVFIL})
mkdir -p $(dirname ${PIDFIL})

cp /dev/null ${ERRFIL}
cp /dev/null ${OUTFIL}
cp /dev/null ${CSVFIL}

exec 2>>${ERRFIL}

. $(readlink -e $(dirname ${0})/../bin)/setup

trap "trap '' SIGINT SIGQUIT SIGTERM; kill -TERM -- -${SELF} 2> /dev/null; exit 0" SIGINT SIGQUIT SIGTERM

##
## CAPTURE CSV GEOLOCATION
##

# NMEA-PUBX-POSITION
# NMEA-PUBX-SVSTATUS
# NMEA-PUBX-TIME
# NMEA-PUBX-RATE GSV @1Hz
# NMEA-PUBX-RATE VTG @1Hz
# UBX-CFG-PRT [0] (all)
# UBX-CFG-MSG [3] UBX-MON-HW @1Hz (ublox8 > fw18)
# UBX-CFG-MSG [3] UBX-NAV-STATUS @1Hz (ublox8 > fw18)
# UBX-CFG-MSG [3] UBX-NAV-ATT @1Hz (ublox8 > fw19)
# UBX-CFG-ODO [20] 0x01 3 1 50 153 76
# UBX-NAV-RESETODO [0]
# UBX-CFG-MSG [3] UBX-NAV-ODO @1Hz (ublox8 > fw15)
# UBX-CFG-MSG [3] UBX-NAV-PVT @1Hz (ublox8 > fw15)
# UBX-CFG-ITFM [8] LE(0x96b156YX) LE(0x0000631e) (X[4]=broadband threshold signed dB, Y[5]=continuous wave threshold signed dB)
# UBX-MON-VER [0]
# UBX-CFG-DAT [0]
# UBX-CFG-TPS [0]
# UBX-CFG-GNSS [0]

gpstool \
	-H ${OUTFIL} \
	-t 10 \
	-T ${CSVFIL} \
	-O ${PIDFIL} \
	-D ${DEVICE} -b ${RATE} -8 -n -1 \
	-x \
	-W '$PUBX,00' \
	-W '$PUBX,03' \
	-W '$PUBX,04' \
	-W '$PUBX,40,GSV,0,0,0,1,0,0' \
	-W '$PUBX,40,VTG,0,0,0,1,0,0' \
	-A '\xb5\x62\x06\x00\x00\x00' \
	-A '\xb5\x62\x06\x01\x03\x00\x0a\x09\x01' \
	-A '\xb5\x62\x06\x01\x03\x00\x01\x03\x01' \
	-A '\xb5\x62\x06\x01\x03\x00\x01\x05\x01' \
	-A '\xb5\x62\x06\x1e\x14\x00\x00\x00\x00\x00\x01\x03\x00\x00\x00\x00\x00\x00\x01\x32\x00\x00\x99\x4c\x00\x00' \
	-A '\xb5\x62\x01\x10\x00\x00' \
	-A '\xb5\x62\x06\x01\x03\x00\x01\x09\x01' \
	-A '\xb5\x62\x06\x01\x03\x00\x01\x07\x01' \
	-A '\xb5\x62\x06\x39\x08\x00\xf7\x56\xb1\x96\x1e\x63\x00\x00' \
	-U '\xb5\x62\x0a\x04\x00\x00' \
	-A '\xb5\x62\x06\x06\x00\x00' \
	-A '\xb5\x62\x06\x31\x00\x00' \
	-A '\xb5\x62\x06\x3e\x00\x00' \
	< /dev/null 1> /dev/null &

sleep 5

##
## OUTPUT DISPLAY
##

cat ${ERRFIL}

DIRECTORY=$(dirname ${CSVFIL})
FILENAME=$(basename ${CSVFIL})
TASK=${FILENAME%%.*}
FILE=${FILENAME#*.}

peruse ${TASK} ${FILE} ${LIMIT} ${DIRECTORY} < /dev/null &

##
## INPUT KEYBOARD
##

hups $(cat ${PIDFIL})
