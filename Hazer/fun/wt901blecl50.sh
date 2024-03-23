#!/bin/bash
# Copyright 2024 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

DEVICE=${1:-"/dev/ttyUSB0"}
# The baud rate is fixed on the WitMotion WT901BLECL5.0.

. $(readlink -e $(dirname ${0})/../bin)/setup

ERRDIR=$(readlink -e $(dirname ${0})/..)/tmp
mkdir -p ${ERRDIR}
ERRFIL=${ERRDIR}/$(basename ${0}).err
cp /dev/null ${ERRFIL}
exec 2>>${ERRFIL}

wt901wired | serialtool -D ${DEVICE} -b 115200 -8 -1 -n -P | wt901tool -E
