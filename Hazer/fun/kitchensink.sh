#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Use a bunch of options.

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
PIDFIL="${SAVDIR}/${PROGRAM}.pid"
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
	-D ${DEVICE} -b ${RATE} -7 -8 -e -o -n -2 -1 \
	-F 1 \
	-R -E -P -H ${OUTFIL} -t 10 \
	-L ${LOGFIL} \
	-N ${FIXFIL} \
	-O ${PIDFIL} \
	-P ${PIDFIL} \
	-S ${SRCFIL} \
	-T ${CSVFIL} \
	-V \
	-d \
	-f 1 \
	-g 0x7 \
	-k 0x7 \
	-t 10 \
	-u \
	-v \
	-x \
	-y 10 \
	< ${FD0FIL} \
	> ${FD1FIL} \
	2> ${FD2FIL}

stty sane

cat ${OUTFIL}

cat ${FD2FIL}

cat ${FD1FIL}

exit 0
