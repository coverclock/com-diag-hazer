#!/bin/bash
# Copyright 2021-2023 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# This script tests the Bad Elf GPS Pro+ via its USB connection.

PGMDIR=$(dirname ${0})
PGMNAM=$(basename -s .sh ${0})

BASDIR=$(readlink -e ${PGMDIR}/..)
SAVDIR=${COM_DIAG_HAZER_SAVDIR:-${BASDIR}/tmp}

GPSDEV=${1:-"/dev/ttyACM0"}
GPSBPS=${2:-"9600"}
ERRFIL=${3:-"${SAVDIR}/${PGMNAM}.err"}
OUTFIL=${4:-"${SAVDIR}/${PGMNAM}.out"}

mkdir -p $(dirname ${ERRFIL})
exec 2>>${ERRFIL}

mkdir -p $(dirname ${OUTFIL})
cp /dev/null ${OUTFIL}

exec coreable gpstool -D ${GPSDEV} -b ${GPSBPS} -8 -n -1 -t 10 -E -H ${OUTFIL} < /dev/null > /dev/null
