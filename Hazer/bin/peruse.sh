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

PROGRAM=$(basename ${0})
TASK=${1}
FILE=${2:-"out"}
LIMIT=${3:-$(($(stty size | cut -d ' ' -f 1) - 2))}
DIRECTORY=${4:-${SAVDIR}}

mkdir -p ${SAVDIR}

. $(readlink -e $(dirname ${0})/../bin)/setup

if [[ "${TASK}" == "router" ]]; then
    cat ${DIRECTORY}/${TASK}.${FILE}
    test /var/log/syslog.1 && grep rtktool /var/log/syslog.1
    grep rtktool /var/log/syslog
    tail -n 0 -f /var/log/syslog | grep rtktool
elif [[ "${FILE}" == "err" ]]; then
    tail -n ${LIMIT} -f ${DIRECTORY}/${TASK}.${FILE}
elif [[ "${FILE}" == "out" ]]; then
    stdbuf -o0 headless ${DIRECTORY}/${TASK}.${FILE} ${DIRECTORY}/${TASK}.pid | \
    	stdbuf -o0 awk '
      		begin   { inp="INP [   ]"; out="OUT [   ]"; arm=1; }
      		/^INP / { inp=substr($0,0,79); arm=1; next; }
      		/^OUT / { out=substr($0,0,79); arm=1; next; }
              		{ if (arm!=0) { print inp; print out; arm=0; } print $0; next; }
      		end     { if (arm!=0) { print inp; print out; arm=0; } }
    	' | \
		stdbuf -o0 head -n ${LIMIT}
elif [[ "${FILE}" == "csv" ]]; then
    tail -n ${LIMIT} -f ${DIRECTORY}/${TASK}.${FILE}
else
    cat ${DIRECTORY}/${TASK}.${FILE}
fi
