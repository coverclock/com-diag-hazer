#!/bin/bash
# Copyright 2023 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Fire up a bunch of windows running the spartan field test.
# THIS IS A WORK IN PROGRESS.

BINDIR=$(readlink -e $(dirname ${0})/../bin)
SAVDIR=${COM_DIAG_HAZER_SAVDIR:-${BINDIR}/../tmp}

APPNAM=${1:-"spartan"}
BASNAM=${2:-"spartan"}

TERMINAL="xfce4-terminal"

mkdir -p ${SAVDIR}
touch ${SAVDIR}/${BASNAM}.csv
touch ${SAVDIR}/${BASNAM}.err
touch ${SAVDIR}/${BASNAM}.out

${TERMINAL} --geometry="80x25" --working-directory=${SAVDIR} --execute ${BINDIR}/peruse ${BASNAM} err &
${TERMINAL} --geometry="80x25" --working-directory=${SAVDIR} --execute ${BINDIR}/peruse ${BASNAM} out &
${TERMINAL} --geometry="80x25" --working-directory=${SAVDIR} --execute ${BINDIR}/peruse ${BASNAM} csv &
${TERMINAL} --geometry="80x25" --working-directory=${SAVDIR} --execute ${BINDIR}/${APPNAM}            &
${TERMINAL} --geometry="80x25" --working-directory=${SAVDIR} --execute ${BINDIR}/hups                 &

exit 0
