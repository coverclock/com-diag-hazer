#!/bin/bash
# Copyright 2023 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Fire up a bunch of windows running a field test.
# Probably specific to Ubuntu-based systems.

BINDIR=$(readlink -e $(dirname ${0})/../bin)
SAVDIR=${COM_DIAG_HAZER_SAVDIR:-${BINDIR}/../tmp}

PGMNAM=$(basename ${0})
APPNAM=${1:-"spartan"}
BASNAM=${2:-"spartan"}

CSVFIL="${SAVDIR}/${BASNAM}.csv"
ERRFIL="${SAVDIR}/${BASNAM}.err"
OUTFIL="${SAVDIR}/${BASNAM}.out"
PIDFIL="${SAVDIR}/${BASNAM}.pid"

mkdir -p ${SAVDIR}
touch ${CSVFIL}
touch ${ERRFIL}
touch ${OUTFIL}
touch ${PIDFIL}
exec 2>>${ERRFIL}

cd ${SAVDIR}

if which x-terminal-emulator > /dev/null; then
    TERMINAL="x-terminal-emulator"
elif which xfce4-terminal > /dev/null; then
    TERMINAL="xfce4-terminal"
elif which mate-terminal > /dev/null; then
    TERMINAL="mate-terminal"
elif which lxterminal > /dev/null; then
    TERMINAL="lxterminal"
elif which gnome-terminal > /dev/null; then
    TERMINAL="gnome-terminal"
elif which xterm > /dev/null; then
    TERMINAL="xterm"
else
    echo "${PGMNAM}: TERMINAL failed!" 1>&2
    exit 1
fi

GEOMETRY="80x25"

${TERMINAL} --geometry=${GEOMETRY} -e "${BINDIR}/peruse ${BASNAM} err"
${TERMINAL} --geometry=${GEOMETRY} -e "${BINDIR}/peruse ${BASNAM} out"
${TERMINAL} --geometry=${GEOMETRY} -e "${BINDIR}/peruse ${BASNAM} csv"
${TERMINAL} --geometry=${GEOMETRY} -e "${BINDIR}/hups"
${TERMINAL} --geometry=${GEOMETRY} -e "${BINDIR}/${APPNAM}"

exit 0
