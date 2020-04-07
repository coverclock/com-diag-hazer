#!/bin/bash
# Copyright 2019-2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# 
# ABSTRACT
#
# Helper script used to follow the log or the screens from the Tumbleweed
# scripts.
#
# USAGE
#
#    peruse TASK FILE
#
# EXAMPLES
#
#    peruse base out
#    peruse base err
#    peruse rover out
#    peruse rover err
#    peruse router err

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}
mkdir -p ${SAVDIR}

PROGRAM=$(basename ${0})
TASK=${1}
FILE=${2}
DIRECTORY=${3:-${SAVDIR}}
LIMIT=${4:-$(($(stty size | cut -d ' ' -f 1) - 2))}
PID=""

. $(readlink -e $(dirname ${0})/../bin)/setup

if [[ "${TASK}" == "router" ]]; then
    cat ${DIRECTORY}/${TASK}.${FILE}
    exec tail -n 0 -f /var/log/syslog | grep rtktool
elif [[ "${FILE}" == "err" ]]; then
    exec tail -n ${LIMIT} -f ${DIRECTORY}/${TASK}.${FILE}
elif [[ "${FILE}" == "out" ]]; then
    exec headless ${DIRECTORY}/${TASK}.${FILE} ${DIRECTORY}/${TASK}.pid
elif [[ "${FILE}" == "csv" ]]; then
    exec tail -n ${LIMIT} -f ${DIRECTORY}/${TASK}.${FILE}
else
    exec cat ${DIRECTORY}/${TASK}.${FILE}
fi
