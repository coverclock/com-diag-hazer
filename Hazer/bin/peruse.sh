#!/bin/bash
# Copyright 2019-2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# 
# ABSTRACT
#
# Helper script used to follow the log or the screens from the Tumbleweed
# headless scripts.
#
# USAGE
#
# peruse TASK FILE
#
# EXAMPLES
#
# peruse base out
# peruse base err
# peruse rover out
# peruse rover err
# peruse router err

SELF=$$

ROOT=$(readlink -e $(dirname ${0})/..)
SAVDIR=${COM_DIAG_HAZER_SAVDIR:-${ROOT}/tmp}

PROGRAM=$(basename ${0})
TASK=${1}
FILE=${2:-"out"}
LIMIT=${3:-$(($(stty size | cut -d ' ' -f 1) - 2))}
DIRECTORY=${4:-${SAVDIR}}

mkdir -p ${SAVDIR}

. ${ROOT}/bin/setup

trap "trap '' SIGINT SIGQUIT SIGTERM; kill -TERM -- -${SELF} 2> /dev/null; exit 0" SIGINT SIGQUIT SIGTERM

if [[ "${TASK}" == "router" ]]; then
    cat ${DIRECTORY}/${TASK}.${FILE}
    test /var/log/syslog.1 && grep rtktool /var/log/syslog.1
    grep rtktool /var/log/syslog
    tail -n 0 -f /var/log/syslog | grep rtktool
elif [[ "${FILE}" == "err" ]]; then
    cat ${DIRECTORY}/${TASK}.${FILE}
    tail -n 0 -f ${DIRECTORY}/${TASK}.${FILE}
elif [[ "${FILE}" == "out" ]]; then
    stdbuf -oL observe ${DIRECTORY}/${TASK}.${FILE} |
        while read FILENAME; do
            if [[ -f ${FILENAME} ]]; then
                clear
                awk -f ${ROOT}/bin/${PROGRAM}.awk < ${FILENAME} | head -n ${LIMIT}
            fi
        done
elif [[ "${FILE}" == "csv" ]]; then
    # csv2tty < ${DIRECTORY}/${TASK}.${FILE}
    tail -n 0 -f ${DIRECTORY}/${TASK}.${FILE} | csv2tty
else
    cat ${DIRECTORY}/${TASK}.${FILE}
    tail -n 0 -f ${DIRECTORY}/${TASK}.${FILE}
fi
