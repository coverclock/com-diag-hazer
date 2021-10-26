#!/bin/bash
# Copyright 2019-2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}
mkdir -p ${SAVDIR}

PROGRAM=$(basename ${0})
ROUTER=${1:-":tumbleweed"}
ERRFIL=${2-"${SAVDIR}/${PROGRAM}.err"}

cp /dev/null ${ERRFIL}
exec 2>>${ERRFIL}

. $(readlink -e $(dirname ${0})/../bin)/setup

exec coreable rtktool \
    -p ${ROUTER} -t 30 -M \
    < /dev/null 1> /dev/null
