#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Use a bunch of options just to test the command line parsing.
# Examples:
# kitchensink
# kitchensink disposal

PROGRAM=$(basename ${0})

if [[ "$1" != "" ]]; then
	PREFIX="valgrind --leak-check=full --show-leak-kinds=all"
else
	PREFIX="coreable"
fi

DEVICE="/dev/null"
RATE=115200

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}
mkdir -p ${SAVDIR}

CATFIL="${SAVDIR}/${PROGRAM}.cat"
CSVFIL="${SAVDIR}/${PROGRAM}.csv"
ERRFIL="${SAVDIR}/${PROGRAM}.err"
FIXFIL="${SAVDIR}/${PROGRAM}.fix"
LSTFIL="${SAVDIR}/${PROGRAM}.lst"
OUTFIL="${SAVDIR}/${PROGRAM}.out"
PIDFIL="${SAVDIR}/${PROGRAM}.pid"
QUEFIL="${SAVDIR}/${PROGRAM}.que"
SRCFIL="${SAVDIR}/${PROGRAM}.src"

mkdir -p $(dirname ${CATFIL})
mkdir -p $(dirname ${CSVFIL})
mkdir -p $(dirname ${ERRFIL})
mkdir -p $(dirname ${FIXFIL})
mkdir -p $(dirname ${LSTFIL})
mkdir -p $(dirname ${OUTFIL})
mkdir -p $(dirname ${PIDFIL})
mkdir -p $(dirname ${QUEFIL})
mkdir -p $(dirname ${SRCFIL})

cp /dev/null ${SRCFIL}

. $(readlink -e $(dirname ${0})/../bin)/setup

# -I 0
# -M
# -p 0

export COM_DIAG_DIMINUTO_LOG_MASK=0xff

exec ${PREFIX} gpstool \
	-1 \
	-2 \
	-4 \
	-6 \
	-7 \
	-8 \
	-A '' \
	-B 1048576 \
	-C ${CATFIL} \
	-D ${DEVICE} \
	-E \
	-F 1 \
	-G 127.0.0.1:21000 \
	-H ${OUTFIL} \
	-K \
	-L ${LSTFIL} \
	-N ${FIXFIL} \
	-O ${PIDFIL} \
	-P \
	-Q ${QUEFIL} \
	-R \
	-S ${SRCFIL} \
	-T ${CSVFIL} \
	-U '' \
	-V \
	-W '' \
	-X 1 \
	-Y 127.0.0.1:21001 \
	-Z '' \
	-a \
	-b ${RATE} \
	-c \
	-d \
	-e \
	-f 1 \
	-g 0x4 \
	-h \
	-i 5 \
	-k 0x3 \
	-l \
	-m \
	-n \
	-o \
	-q 0x2 \
	-s \
	-t 10 \
	-u 500 \
	-v \
	-w 5 \
	-x \
	-y 10 \
	< /dev/null
