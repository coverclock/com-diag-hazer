#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Combines benchmark, hups, and peruse into a single script for use
# in field testing.

SELF=$$

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}
ROUTER=${1:-"tumbleweed"}
TASK=${2:-"benchmark"}
LIMIT=${3:-$(($(stty size | cut -d ' ' -f 1) - 2))}

. $(readlink -e $(dirname ${0})/../bin)/setup

trap "trap '' SIGINT SIGQUIT SIGTERM; kill -TERM -- -${SELF}; exit 0" SIGINT SIGQUIT SIGTERM

${TASK} ${ROUTER}:tumbleweed &
sleep 5
peruse ${TASK} out ${LIMIT} < /dev/null &
hups $(cat ${SAVDIR}/${TASK}.pid)
