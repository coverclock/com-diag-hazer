#!/bin/bash
# Copyright 2019 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

# Uses the inotify tools to watch a log file produced by gpstool running
# in "headless" (-H FILE) mode in which the latest full screen update
# is written to a file using the Diminuto Observation feature. See
# the bin/base.sh and bin/rover.sh for examples of this. So you might
# use the command "headless out/host/log/base" to watch the fill screen
# updates performed by the Base Station script.

PROGRAM=$(basename ${0})
HEADLESS=${1:-"/dev/null"}

CANONICAL=$(readlink -f ${HEADLESS})
DIRECTORY=$(dirname ${CANONICAL})
FILE=$(basename ${CANONICAL})

test -d ${DIRECTORY} || exit 1

clear

# sudo sudo apt-get install inotify-tools
while inotifywait -e moved_to ${DIRECTORY} 2> /dev/null | egrep "${FILE}"; do
	cat ${HEADLESS}
done
