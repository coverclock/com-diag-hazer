#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Combines mobile, hups, and peruse into a single script for use
# in field testing.

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}
TASK=${1:-"mobile"}
LIMIT=${2:-$(($(stty size | cut -d ' ' -f 1) - 2))}

. $(readlink -e $(dirname ${0})/../bin)/setup

${TASK} &
TASKPID=$!
sleep 5
peruse ${TASK} out ${LIMIT} < /dev/null &
PERUSEPID=$!
trap "kill ${TASKPID} ${PERUSEPID}" 0 1 2 3 15

hups
