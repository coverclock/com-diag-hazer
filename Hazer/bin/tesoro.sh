#!/bin/bash
# Copyright 2021 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Consumes datagrams from the Tesoro UDP port number, builds a
# URL for the tile server, and invokes a browser. EXPERIMENTAL
# I use UDP6 below because it should work with either IPv6 or
# IPv4 connections.

. $(readlink -e $(dirname ${0})/../bin)/setup

PROGRAM=$(basename $0)
PORT=${1:-"tesoro"}
HOST=${2:-"maps"}
if [[ "${TERM_PROGRAM}" == "Apple_Terminal" ]]; then
    BROWSER=${3:-"open -a Safari"}
else
    BROWSER=${3:-"firefox"}
if

EXP=0
PID=0

socat -u UDP6-RECV:${PORT} - | while read TIM LAT LON LBL; do

    if [[ ${EXP} -eq 0 ]]; then
	:
    elif [[ ${TIM} -lt ${EXP} ]]; then
	echo "${PROGRAM}: Reordered ${EXP} ${TIM}" 1>&2
	continue
    elif [[ ${TIM} -gt ${EXP} ]];then
	echo "${PROGRAM}: Dropped ${EXP} ${TIM}" 1>&2
    else
        :
    fi

    if [[ ${PID} -ne 0 ]]; then
        kill -0 ${PID} 2> /dev/null & kill ${PID}
    fi

    ${BROWSER} http://${HOST}/tesoro/tesoro.html?LAT=${LAT}\&LON=${LON}\&MSL=${MSL}\&LBL=${LBL} &
    PID=$!

    EXP=$((${TIM} + 1))

done
