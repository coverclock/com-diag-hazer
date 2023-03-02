#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Renames the SAVDIR by adding a timestamp.
# N.B. Probably bad is someone is using SAVDIR at the time.

. $(readlink -e $(dirname ${0})/../bin)/setup
TIMESTAMP=$(date -u +%Y%m%dT%H%M%SZ%N)
SAVDIR=${1:-${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}}
OLDDIR="${SAVDIR}-${TIMESTAMP}"
exec mv -i ${SAVDIR} ${OLDDIR} && chmod u-wx,g-wx,o-wx ${OLDDIR}
