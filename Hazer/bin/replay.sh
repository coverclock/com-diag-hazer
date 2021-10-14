#!/bin/bash
# Copyright 2021 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Script replays a catenate file (or any other input file).

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}

PROGRAM=$(basename ${0})
ERRFIL=${1-"${SAVDIR}/${PROGRAM}.err"}

mkdir -p $(dirname ${ERRFIL})
cp /dev/null ${ERRFIL}
exec 2>>${ERRFIL}

. $(readlink -e $(dirname ${0})/../bin)/setup

exec coreable gpstool -S - -E -i 0 -v
