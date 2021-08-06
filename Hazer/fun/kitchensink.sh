#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Use a bunch of options just to test the command line parsing.

PROGRAM=$(basename ${0})

DEVICE="/dev/tty"
RATE=115200

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}
mkdir -p ${SAVDIR}

CATFIL="${SAVDIR}/${PROGRAM}.cat"
CSVFIL="${SAVDIR}/${PROGRAM}.csv"
ERRFIL="${SAVDIR}/${PROGRAM}.err"
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

cp /dev/null ${SRCFIL}

. $(readlink -e $(dirname ${0})/../bin)/setup

coreable gpstool \
	-1 \
	-2 \
	-7 \
	-8 \
	-B 1024 \
	-C ${CATFIL} \
	-D ${DEVICE} -b ${RATE} \
	-E \
	-F 1 \
	-G 127.0.0.1:21000 \
	-H ${OUTFIL} \
	-I 0 \
	-K \
	-L ${LOGFIL} \
	-M \
	-N ${FIXFIL} \
	-O ${PIDFIL} \
	-P \
	-R \
	-S ${SRCFIL} \
	-T ${CSVFIL} \
	-U '' \
	-V \
	-W '' \
	-X \
	-Y 127.0.0.1:21001 \
	-Z '' \
	-b 9600 \
	-c \
	-d \
	-e \
	-f 1 \
	-g 0x7 \
	-h \
	-i 5 \
	-k 0x7 \
	-l \
	-m \
	-n \
	-o \
	-p 0 \
	-s \
	-t 10 \
	-u \
	-v \
	-w 5 \
	-x \
	-y 10 \
	< /dev/null

stty sane

exit 0
