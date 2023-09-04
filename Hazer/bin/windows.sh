#!/bin/bash
# Copyright 2023 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Fire up a bunch of windows running a field test.
# Probably specific to Ubuntu-based systems.

BINDIR=$(readlink -e $(dirname ${0})/../bin)
SAVDIR=${COM_DIAG_HAZER_SAVDIR:-${BINDIR}/../tmp}

mkdir -p ${SAVDIR}
touch ${SAVDIR}/${BASNAM}.csv
touch ${SAVDIR}/${BASNAM}.err
touch ${SAVDIR}/${BASNAM}.out
exec 2>>${SAVDIR}/${BASNAM}.err

cd ${SAVDIR}

PGMNAM=$(basename ${0})
APPNAM=${1:-"spartan"}
BASNAM=${2:-"spartan"}

if false; then
    TERMINAL="xfce4-terminal"
elif false; then
    TERMINAL="gnome-terminal"
elif false; then
    TERMINAL="mate-terminal"
elif false; then
    TERMINAL="lxterminal"
else
    TERMINAL="x-terminal-emulator"
fi

GEOMETRY="80x25"

${TERMINAL} --geometry=${GEOMETRY} -e "${BINDIR}/peruse ${BASNAM} err" & PE=$!
${TERMINAL} --geometry=${GEOMETRY} -e "${BINDIR}/peruse ${BASNAM} out" & PO=$!
${TERMINAL} --geometry=${GEOMETRY} -e "${BINDIR}/peruse ${BASNAM} csv" & PC=$!
${TERMINAL} --geometry=${GEOMETRY} -e "${BINDIR}/hups"                 & PH=$!
${TERMINAL} --geometry=${GEOMETRY} -e "${BINDIR}/${APPNAM}"            & PA=$!

wait ${PE} ${PO} ${PC} ${PH} ${PA}

exit 0
