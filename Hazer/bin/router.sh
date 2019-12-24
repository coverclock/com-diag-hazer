#!/bin/bash
# Copyright 2019 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

PROGRAM=$(basename ${0})
ROUTER=${1:-":tumbleweed"}
ERRFIL=${2-"./${PROGRAM}.err"}

cp /dev/null ${ERRFIL}
exec 2>>${ERRFIL}

. $(readlink -e $(dirname ${0})/../bin)/setup

export COM_DIAG_DIMINUTO_LOG_MASK=0xfe

exec coreable rtktool \
    -p ${ROUTER} -t 30 \
    < /dev/null 1> /dev/null
