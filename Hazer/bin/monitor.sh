#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Passively monitor a GNSS device.

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

DIRECTORY=$(dirname ${OUTFIL})
FILENAME=$(basename ${OUTFIL})
TASK=${FILENAME%%.*}
FILE=${FILENAME#*.}

mkdir -p $(dirname ${ERRFIL})
mkdir -p $(dirname ${OUTFIL})
mkdir -p $(dirname ${CSVFIL})
mkdir -p $(dirname ${PIDFIL})

cp /dev/null ${ERRFIL}
exec 2>>${ERRFIL}

. $(readlink -e $(dirname ${0})/../bin)/setup

export COM_DIAG_DIMINUTO_LOG_MASK=${COM_DIAG_DIMINUTO_LOG_MASK:=0xfc}

trap "trap '' SIGINT SIGQUIT SIGTERM; kill -TERM -- -${SELF} 2> /dev/null; exit 0" SIGINT SIGQUIT SIGTERM

coreable gpstool \
	-H ${OUTFIL} \
	-t 10 \
	-T ${CSVFIL} \
	-O ${PIDFIL} \
	-D ${DEVICE} -b ${RATE} -8 -n -1 \
	-x \
	< /dev/null 1> /dev/null &

sleep 5
peruse ${TASK} ${FILE} ${LIMIT} ${DIRECTORY} < /dev/null &
hups $(cat ${PIDFIL})
