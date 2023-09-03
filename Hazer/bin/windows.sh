#!/bin/bash
# Copyright 2023 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Fire up a bunch of windows running a field test. Currently this
# is specific to the Xfce desktop running under Linux MATE on my
# ancient HP Mini 110. But the Xfce terminal utility may be compatible
# enough with xterm to make it work with minor changes on other
# desktops.

BINDIR=$(readlink -e $(dirname ${0})/../bin)
SAVDIR=${COM_DIAG_HAZER_SAVDIR:-${BINDIR}/../tmp}

PGMNAM=$(basename ${0})
APPNAM=${1:-"spartan"}
BASNAM=${2:-"spartan"}

if true; then
    TERMINAL="x-terminal-emulator"
elif false; then
    TERMINAL="xfce4-terminal"
elif false; then
    TERMINAL="gnome-terminal"
elif false; then
    TERMINAL="mate-terminal"
elif false; then
    TERMINAL="lxterminal"
else
    TERMINAL="xterm"
fi

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
