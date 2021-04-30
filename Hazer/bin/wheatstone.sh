#!/bin/bash
# Copyright 2021 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Run a GNSS receiver, saving its data, and forwarding JSON datagrams via a
# serial-connected radio.
# The GPSDEVICE and GPSRATE default to those of the BU-353W10 USB device.
# The NETDEVICE and NETRATE default to those of the Digi XBEE3 LTE-M USB device.
# usage: wheatstone [ GPSDEVICE [ NETDEVICE [ GPSRATE [ NETRATE [ ERRFIL [ OUTFIL [ CSVFIL [ PIDFIL [ LIMIT ] ] ] ] ] ] ] ] ]
# example: wheatstone /dev/ttyACM0 /dev/ttyUSB0

##
## SETUP
##

SELF=$$

CSVDIR=${COM_DIAG_HAZER_SAVDIR:-${HOME:-"/var/tmp"}/csv}
SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}

PROGRAM=$(basename ${0})
GPSDEVICE=${1:-"/dev/ttyACM0"}
NETDEVICE=${2:-"/dev/ttyUSB0"}
GPSRATE=${3:-9600}
NETRATE=${4:-9600}
ERRFIL=${5-"${SAVDIR}/${PROGRAM}.err"}
OUTFIL=${6-"${SAVDIR}/${PROGRAM}.out"}
CSVFIL=${7-"${CSVDIR}/${PROGRAM}.csv"}
PIDFIL=${8-"${SAVDIR}/${PROGRAM}.pid"}
LIMIT=${9:-$(($(stty size | cut -d ' ' -f 1) - 2))}

mkdir -p $(dirname ${ERRFIL})
mkdir -p $(dirname ${OUTFIL})
mkdir -p $(dirname ${CSVFIL})
mkdir -p $(dirname ${PIDFIL})

cp /dev/null ${ERRFIL}
cp /dev/null ${OUTFIL}
touch ${CSVFIL}

exec 2>>${ERRFIL}

. $(readlink -e $(dirname ${0})/../bin)/setup

trap "trap '' SIGINT SIGQUIT SIGTERM; kill -TERM -- -${SELF} 2> /dev/null; exit 0" SIGINT SIGQUIT SIGTERM

##
## FORWARD JSON DATAGRAMS
##

tail -n 0 -f ${CSVFIL} | csv2dgm -D ${NETDEVICE} -b ${NETRATE} -8 -1 -n -j &

##
## CAPTURE CSV GEOLOCATION
##

gpstool -D ${GPSDEVICE} -b ${GPSRATE} -8 -n -1 -H ${OUTFIL} -t 10 -T ${CSVFIL} -O ${PIDFIL} < /dev/null 1> /dev/null &

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
