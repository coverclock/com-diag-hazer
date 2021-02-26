#!/bin/bash
# Copyright 2021 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Run a GNSS receiver, saving its data, and forwarding JSON datagrams.
# The DEVICE and RATE default to those of the BU-353S4 USB device.
# usage: witness [ ENDPOINT [ DEVICE [ RATE [ ERRFIL [ OUTFIL [ CSVFIL [ PIDFIL [ LIMIT ] ] ] ] ] ] ] ]
# example: witness tumbleweed:tesoro /dev/ttyUSB0 4800

##
## SETUP
##

SELF=$$

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}

PROGRAM=$(basename ${0})
ENDPOINT=${1:-"tumbleweed:tesoro"}
DEVICE=${2:-"/dev/ttyUSB0"}
RATE=${3:-4800}
ERRFIL=${4-"${SAVDIR}/${PROGRAM}.err"}
OUTFIL=${5-"${SAVDIR}/${PROGRAM}.out"}
CSVFIL=${6-"${SAVDIR}/${PROGRAM}.csv"}
PIDFIL=${7-"${SAVDIR}/${PROGRAM}.pid"}
LIMIT=${8:-$(($(stty size | cut -d ' ' -f 1) - 2))}

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
## FORWARD JSON DATAGRAMS
##

tail -n 0 -f ${CSVFIL} | csv2dgm -U ${ENDPOINT} -j &

##
## CAPTURE CSV GEOLOCATION
##

gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -H ${OUTFIL} -t 10 -T ${CSVFIL} -O ${PIDFIL} < /dev/null 1> /dev/null &

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
