#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Combines benchmark, hups, and peruse into a single script for use
# in field testing.

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}
ROUTER=${1:-"tumbleweed"}
TASK=${2:-"benchmark"}

. $(readlink -e $(dirname ${0})/../bin)/setup

TASKPID=""
PERUSEPID=""
trap 'test -n "${TASKPID}" && kill -9 ${TASKPID}; test -n "${PERUSEPID}" && kill -9 ${PERUSEPID}' 0 1 2 3 15

${TASK} ${ROUTER}:tumbleweed &
TASKPID=$!
sleep 5
peruse ${TASK} out < /dev/null &
PERUSEPID=$!
hups
