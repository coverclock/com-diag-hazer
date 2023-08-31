#!/bin/bash
# Copyright 2023 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

PROGRAM=$(basename $0}
APPLICATION=${1:-"spartan"}
BASE=${2:-"spartan"}

DIRECTORY="${HOME}/src/com-diag-hazer/Hazer"
TEMPORARY="${DIRECTORY}/out/host/tmp"
SETUP="${DIRECTORY}/out/host/bin/setup"
TERMINAL="xfce4-terminal"
DISPLAY=":0.0"

if [ ! -d ${DIRECTORY} ]; then
    echo "${PROGRAM: ${DIRECTORY} failed!" 1>&2
    exit 2
fi

cd ${DIRECTORY}

if [ ! -r ${SETUP} ]; then
    echo "${PROGRAM: ${SETUP} failed!" 1>&2
    exit 3
fi

mkdir -p ${TEMPORARY}
touch ${TEMPORARY}/${BASE}.csv
touch ${TEMPORARY}/${BASE}.err
touch ${TEMPORARY}/${BASE}.out

export DISPLAY

. ${SETUP}

${TERMINAL} --geometry="80x25" -x peruse ${BASE} err &
${TERMINAL} --geometry="80x25" -x peruse ${BASE} out &
${TERMINAL} --geometry="80x25" -x peruse ${BASE} csv &
${TERMINAL} --geometry="80x25" -x ${APPLICATION}     &
${TERMINAL} --geometry="80x25" -x hups               &

exit 0
