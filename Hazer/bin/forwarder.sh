#!/bin/bash
# Copyright 2021 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}
mkdir -p ${SAVDIR}

PROGRAM=$(basename ${0})
SOURCE=${1:-":tesoro"}
SINK=${2:-":tesoro"}
ERRFIL=${3-"${SAVDIR}/${PROGRAM}.err"}

cp /dev/null ${ERRFIL}
exec 2>>${ERRFIL}

. $(readlink -e $(dirname ${0})/../bin)/setup

exec coreable osmtool \
    -u ${SOURCE} -t ${SINK} \
    < /dev/null 1> /dev/null
