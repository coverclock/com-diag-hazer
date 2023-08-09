#!/bin/bash
# Copyright 2023 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# This script configures the UBX-NEO-D9S and the UBX-ZED-F9P.

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}
CFGFIL=${COM_DIAG_HAZER_CFGFIL:-"${HOME}/com_diag_nicker.sh"}

PROGRAM=$(basename ${0})
LOCDEV=${1:-"/dev/ttyACM0"}
LOCBPS=${2:-38400}
CORDEV=${3:-"/dev/ttyACM1"}
CORBPS=${4:-9600}
ERRFIL=${5:-"${SAVDIR}/${PROGRAM}.err"}

. $(readlink -e $(dirname ${0})/../bin)/setup

mkdir -p $(dirname ${ERRFIL})

cp /dev/null ${ERRFIL}
exec 2>>${ERRFIL}

. ${CFGFIL}

OPTIONS=""
for OPTION in ${UBX_NEO_D9S}; do
    OPTIONS="${OPTIONS} ${OPTION}"
done

eval gpstool \
    -R \
    -D ${CORDEV} -b ${CORBPS} -8 -n -1 \
    ${OPTIONS} \
    -x \
    < /dev/null

OPTIONS=""
for OPTION in ${UBX_ZED_F9P}; do
    OPTIONS="${OPTIONS} ${OPTION}"
done

eval gpstool \
    -R \
    -D ${LOCDEV} -b ${LOCBPS} -8 -n -1 \
    ${OPTIONS} \
    -x \
    < /dev/null
