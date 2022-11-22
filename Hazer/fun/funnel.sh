#!/bin/bash
# Copyright 2022 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# THIS IS A WORK IN PROGRESS.

SSLDIR=${COM_DIAG_HAZER_SSLDIR:-$(readlink -e $(dirname ${0})/..)/ssl}

# endpoint "localhost:tumbleweed" type IPV4 ipv4 127.0.0.1 ipv6 ::1 tcp 0 udp 21010 local ""
#        1                      2    3    4    5         6    7   8   910  11    12    13 14

PROGRAM=$(basename ${0})
ROLE=${1:-""}
if [[ "${ROLE}" == "rover" ]]; then
	SOURCE=${2:-"tumbleweed"}
	DGMHOST=$(endpoint "localhost:${SOURCE}" | cut -d ' ' -f 6)
	DGMPORT=$(endpoint "localhost:${SOURCE}" | cut -d ' ' -f 12)
	SINK=${3:-"localhost:stagecoach"}
	SSLHOST=$(endpoint "${SINK}" | cut -d ' ' -f 6)
	SSLPORT=$(endpoint "${SINK}" | cut -d ' ' -f 12)
	COMMON=${4:-"localhost"}
	PEMFIL=${5:-"${SSLDIR}/rover.pem"}
	CRTFIL=${6:-"${SSLDIR}/base.crt"}
	PROTOCOL=${7:-"4"}
elif [[ "${ROLE}" == "base" ]]; then
	SOURCE=${2:-"stagecoach"}
	SSLHOST=$(endpoint "localhost:${SOURCE}" | cut -d ' ' -f 6)
	SSLPORT=$(endpoint "localhost:${SOURCE}" | cut -d ' ' -f 12)
	SINK=${3:-"tumbleweed"}
	DGMHOST=$(endpoint "localhost:${SINK}" | cut -d ' ' -f 6)
	DGMPORT=$(endpoint "localhost:${SINK}" | cut -d ' ' -f 12)
	COMMON=${4:-"localhost"}
	PEMFIL=${5:-"${SSLDIR}/base.pem"}
	CRTFIL=${6:-"${SSLDIR}/rover.crt"}
	PROTOCOL=${7:-"4"}
else
	echo ${PROGRAM}: ${ROLE}? 1>&2
	exit 1
fi

if [[ "${ROLE}" == "rover" ]]; then

	# gpstool must be used on the rover side to remove the
	# header from each datagram and then pipe the concantenated
	# results to the SSL tunnel.

	COMMAND="socat UDP${PROTOCOL}-LISTEN:${DGMPORT} openssl-connect:${SSLHOST}:${SSLPORT},openssl-commonname=${COMMON},cert=${PEMFIL},cafile=${CRTFIL}"
	echo ${PROGRAM}: ${COMMAND} 1>&2
	eval exec ${COMMAND}

elif [[ "${ROLE}" == "base" ]]; then

	# gpstool must be used on the base side to recover the
	# message boundaries from the SSL tunnel and then forward
	# the results to the UDP socket.

	COMMAND="socat OPENSSL-LISTEN:${SSLPORT},reuseaddr,fork,openssl-commonname=${COMMON},cert=${PEMFIL},cafile=${CRTFIL},verify=1 UDP${PROTOCOL}-LISTEN:${DGMPORT}"
	echo ${PROGRAM}: ${COMMAND} 1>&2
	eval exec ${COMMAND}

else

	echo ${PROGRAM}: ${ROLE}! 1>&2
	exit 4
fi
