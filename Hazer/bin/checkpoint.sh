#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Renames the SAVDIR by adding a timestamp.
# N.B. Probably bad is someone is using SAVDIR at the time.


PROGRAM=$(basename ${0})
SAVDIR=${1:-${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}}
. $(readlink -e $(dirname ${0})/../bin)/setup
exec mvdate ${SAVDIR}
