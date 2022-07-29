#!/bin/bash
# Copyright 2022 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# WORK IN PROGRESS!

SSLDIR=${COM_DIAG_HAZER_SSLDIR:-$(readlink -e $(dirname ${0})/..)/ssl}

PROGRAM=$(basename ${0})
ROLE=${1:-""}
if [[ "${ROLE}" == "rover" ]]; then
	SOURCE=${2:-"tumbleweed"}
	HOST=${3:-"localhost"}
	SINK=${4:-"stagecoach"}
	PEMFIL=${5:-"${SSLDIR}/rover.pem"}
	CRTFIL=${6:-"${SSLDIR}/base.crt"}
	COMMON=${7:-"rover.prariethorn.org"}
elif [[ "${ROLE}" == "base" ]]; then
	HOST=${2:-"localhost"}
	SOURCE=${3:-"stagecoach"}
	SINK=${4:-"tumbleweed"}
	PEMFIL=${5:-"${SSLDIR}/base.pem"}
	CRTFIL=${6:-"${SSLDIR}/rover.crt"}
	COMMON=${7:-"base.prariethorn.org"}
else
	echo ${PROGRAM}: ${ROLE}? 1>&2
	exit 1
fi

if [[ "${ROLE}" == "rover" ]]; then
	COMMAND="socat UDP-DATAGRAM:localhost:${SOURCE} openssl-connect:${HOST}:${SINK},openssl-commonname=${COMMON},cert=${PEMFIL},cafile=${CRTFIL}"
	echo ${PROGRAM}: ${COMMAND} 1>&2
	${COMMAND}
elif [[ "${ROLE}" == "base" ]]; then
	COMMAND="socat openssl-listen:${SOURCE},reuseaddr,fork,cert=${PEMFIL},cafile=${CRTFIL},verify=1 UDP-DATAGRAM:${HOST}:${SINK}"
	echo ${PROGRAM}: ${COMMAND} 1>&2
	${COMMAND}
else
	echo ${PROGRAM}: ${ROLE}! 1>&2
	exit 4
fi

exit 0
