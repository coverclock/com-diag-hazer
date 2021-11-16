#!/bin/bash
# Copyright 2021 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# This uses a Bluetooth-connected GNSS device like the Garmin GLO.

PGMDIR=$(dirname ${0})
BASDIR=$(readlink -e ${PGMDIR}/..)
PGMNAM=$(basename -s .sh ${0})
SAVDIR=${COM_DIAG_HAZER_SAVDIR:-${BASDIR}/tmp}
GPSDEV=${1:-"/dev/rfcomm0"}
ERRFIL=${2-"${SAVDIR}/${PGMNAM}.err"}
OUTFIL=${3-"${SAVDIR}/${PGMNAM}.out"}

mkdir -p $(dirname ${ERRFIL})
cp /dev/null ${ERRFIL}
exec 2>>${ERRFIL}

mkdir -p $(dirname ${OUTFIL})
cp /dev/null ${OUTFIL}

. ${BASDIR}/bin/setup

SELF=$$
trap "trap '' SIGINT SIGQUIT SIGTERM; kill -TERM -- -${SELF} 2> /dev/null; exit 0" SIGINT SIGQUIT SIGTERM

coreable gpstool -D ${GPSDEV} -t 10 -H ${OUTFIL} -E -F 1 &

peruse ${PGMNAM} out
