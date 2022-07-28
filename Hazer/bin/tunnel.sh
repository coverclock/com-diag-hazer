#!/bin/bash
# Copyright 2022 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# WORK IN PROGRESS!

SSLDIR=${COM_DIAG_HAZER_SSLDIR:-$(readlink -e $(dirname ${0})/..)/ssl}
SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}
mkdir -p ${SAVDIR}

PROGRAM=$(basename ${0})
ROLE=${1:-""}
if [[ "${ROLE}" == "rover" ]]; then
	SOURCE=${2:-"tumbleweed"}
	HOST=${3:-"localhost"}
	SINK=${4:-"stagecoach"}
	PEMFIL=${5:-"${SSLDIR}/rover.pem"}
	CRTFIL=${6:-"${SSLDIR}/rover.crt"}
	COMMON=${7:-"stagecoach"}
elif [[ "${ROLE}" == "base" ]]; then
	HOST=${2:-"localhost"}
	SOURCE=${3:-"stagecoach"}
	SINK=${4:-"tumbleweed"}
	PEMFIL=${5:-"${SSLDIR}/base.pem"}
	CRTFIL=${6:-"${SSLDIR}/rover.crt"}
	COMMON=${7:-"stagecoach"}
else
	exit 1
fi
ERRFIL=${8:-"${SAVDIR}/${PROGRAM}.err"}

mkdir -p $(dirname ${ERRFIL})
cp /dev/null ${ERRFIL}
exec 2>>${ERRFIL}

. $(readlink -e $(dirname ${0})/../bin)/setup

if [[ "${ROLE}" == "rover" ]]; then
	exec socat UDP-DATAGRAM:localhost:${SOURCE} openssl-connect:${HOST}:${SINK},openssl-commonname=${COMMON},cert=${PEMFIL},cafile=${CRTFIL}
elif [[ "${ROLE}" == "base" ]]; then
	exec socat openssl-listen:${SOURCE},reuseaddr,fork,cert=${PEMFIL},cafile=${CRTFIL},verify=1 UDP-DATAGRAM:${HOST}:${SINK}
else
	exit 2
fi
