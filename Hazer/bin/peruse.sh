#!/bin/bash
# Copyright 2019-2022 Digital Aggregates Corporation, Colorado, USA
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
# peruse TASK [ TYPE [ LIMIT [ DIRECTORY ] ] ]
# peruse TASK.TYPE [ LIMIT [ DIRECTORY ] ]
# peruse DIRECTORY/TASK [ TYPE [ LIMIT ] ]
# peruse DIRECTORY/TASK.TYPE [ LIMIT ]
#
# EXAMPLES
#
if false; then
peruse base
peruse base out
peruse base out 128
peruse base out 128 /tmp
peruse base err
peruse rover out
peruse rover err
peruse router err
peruse base.out
peruse base.out 128
peruse base.out 128 /tmp
peruse /tmp/base
peruse /tmp/base out
peruse /tmp/base out 128
peruse /tmp/base.out
peruse /tmp/base.out 128
fi

if [[ $# -eq 0 ]]; then
    exit 1
fi

SELF=$$

PROGRAM=$(basename ${0})
ROOT=$(readlink -e $(dirname ${0})/..)
SAVDIR=${COM_DIAG_HAZER_SAVDIR:-${ROOT}/tmp}

CASE="none"
case ${1} in
(*/*.*)
    CASE="path"
    DIRECTORY=$(dirname ${1})
    FILENAME=$(basename ${1})
    TASK=${FILENAME%%.*}
    TYPE=${FILENAME##*.}
    LIMIT=${2:-$(($(stty size | cut -d ' ' -f 1) - 2))}
    ;;
(*.*)
    CASE="file"
    FILENAME=${1}
    TASK=${FILENAME%%.*}
    TYPE=${FILENAME##*.}
    LIMIT=${2:-$(($(stty size | cut -d ' ' -f 1) - 2))}
    DIRECTORY=${3:-${SAVDIR}}
    ;;
(*/*)
    CASE="directory"
    DIRECTORY=$(dirname ${1})
    FILENAME=$(basename ${1})
    TASK=${FILENAME}
    TYPE=${2:-"out"}
    LIMIT=${3:-$(($(stty size | cut -d ' ' -f 1) - 2))}
    ;;
(*)
    CASE="default"
    TASK=${1}
    TYPE=${2:-"out"}
    LIMIT=${3:-$(($(stty size | cut -d ' ' -f 1) - 2))}
    DIRECTORY=${4:-${SAVDIR}}
    ;;
esac

echo ${PROGRAM}: ${CASE} ${DIRECTORY} '/' ${TASK} '.' ${TYPE} '@' ${LIMIT} 1>&2

mkdir -p ${DIRECTORY}
touch ${DIRECTORY}/${TASK}.${TYPE}

. ${ROOT}/bin/setup

trap "trap '' SIGINT SIGQUIT SIGTERM; kill -TERM -- -${SELF} 2> /dev/null; exit 0" SIGINT SIGQUIT SIGTERM

if [[ "${TASK}" == "router" ]]; then
    cat ${DIRECTORY}/${TASK}.${TYPE}
    test /var/log/syslog.1 && grep rtktool /var/log/syslog.1
    grep rtktool /var/log/syslog
    tail -n 0 -f /var/log/syslog | grep rtktool
elif [[ "${TYPE}" == "err" ]]; then
    cat ${DIRECTORY}/${TASK}.${TYPE}
    tail -n 0 -f ${DIRECTORY}/${TASK}.${TYPE}
elif [[ "${TYPE}" == "out" ]]; then
    stdbuf -oL observe ${DIRECTORY}/${TASK}.${TYPE} |
        while read FILENAME; do
            if [[ -f ${FILENAME} ]]; then
                clear
                awk -f ${ROOT}/bin/${PROGRAM}.awk < ${FILENAME} | head -n ${LIMIT}
            fi
        done
elif [[ "${TYPE}" == "csv" ]]; then
    tail -n 0 -f ${DIRECTORY}/${TASK}.${TYPE} | csv2tty
else
    cat ${DIRECTORY}/${TASK}.${TYPE}
    tail -n 0 -f ${DIRECTORY}/${TASK}.${TYPE}
fi
