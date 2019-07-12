#!/bin/bash
# Copyright 2019 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# 
# Just a quick way to peruse the log or the output from one of the Tumbleweed
# scripts.
#
#     peruse base out
#     peruse rover out
#     peruse router err

PROGRAM=$(basename ${0})
TASK=${1}
FILE=${2}

. $(readlink -e $(dirname ${0})/../bin)/setup

LOG=$(readlink -e $(dirname ${0})/../log)

if [[ "${FILE}" == "err" ]]; then
	CMD="tail -f"
elif [[ "${FILE}" == "out" ]]; then
	CMD="headless"
else
	CMD="cat"
fi

exec ${CMD} ${LOG}/${TASK}.${FILE}
