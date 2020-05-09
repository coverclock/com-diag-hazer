#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Combines benchmark, hups, and peruse into a single script for use
# in field testing.

ROUTER=${1:-"tumbleweed"}
TASK=${2:-"benchmark"}

. $(readlink -e $(dirname ${0})/../bin)/setup

TASKPID=0
PERUSEPID=0
trap "kill -9 ${TASKPID} ${PERUSEPID}" 1 2 3 15

${TASK} ${ROUTER}:tumbleweed &
TASKPID=$!
sleep 5
peruse ${TASK} out < /dev/null &
PERUSEPID=$!
hups
