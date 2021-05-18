#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Configure and run the U-blox ZED-UBX-F9P as a uncorrected independent
# mobile Rover.

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}
mkdir -p ${SAVDIR}

PROGRAM=$(basename ${0})

DEVICE="/dev/tty"
RATE=115200

CATFIL="${SAVDIR}/${PROGRAM}.cat"
CSVFIL="${SAVDIR}/${PROGRAM}.csv"
ERRFIL="${SAVDIR}/${PROGRAM}.err"
FD0FIL="${SAVDIR}/${PROGRAM}.fd0"
FD1FIL="${SAVDIR}/${PROGRAM}.fd1"
FD2FIL="${SAVDIR}/${PROGRAM}.fd2"
FIXFIL="${SAVDIR}/${PROGRAM}.fix"
LOGFIL="${SAVDIR}/${PROGRAM}.log"
OUTFIL="${SAVDIR}/${PROGRAM}.out"
SRCFIL="${SAVDIR}/${PROGRAM}.src"

mkdir -p $(dirname ${CATFIL})
mkdir -p $(dirname ${CSVFIL})
mkdir -p $(dirname ${ERRFIL})
mkdir -p $(dirname ${FIXFIL})
mkdir -p $(dirname ${LOGFIL})
mkdir -p $(dirname ${OUTFIL})
mkdir -p $(dirname ${SRCFIL})

cp /dev/null ${FD0FIL}
cp /dev/null ${SRCFIL}

. $(readlink -e $(dirname ${0})/../bin)/setup

coreable gpstool \
	-B 1024 \
	-C ${CATFIL} \
	-D ${DEVICE} -b ${RATE} -8 -n -1 \
	-H ${OUTFIL} -t 10 \
	-L ${LOGFIL} \
	-N ${FIXFIL} \
	-S ${SRCFIL} \
	-T ${CSVFIL} \
	-d \
	-g 0x7 \
	-k 0x7 \
	-t 10 \
	-u \
	-v \
	-y 10 \
	< ${FD0FIL} \
	> ${FD1FIL} \
	2> ${FD2FIL}

stty sane
