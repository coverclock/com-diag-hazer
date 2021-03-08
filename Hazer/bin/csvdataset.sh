#!/bin/bash
# Copyright 2021 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Filter that reads CSV and outputs a JSON array to visualize a polyline.
# usage: csv2json [ MODULO ] < CSVFILE > JSONFILE

PROGRAM=$(basename ${0})
MODULO=${1:-1}

INITIALIZED=""
PRIOR=0
SEQUENCE=0
LATITUDE=""
LONGITUDE=""
OLDLATITUDE=""
OLDLONGITUDE=""

while read NAM NUM FIX SYS SAT CLK TIM LAT LON HAC MSL GEO VAC SOG COG ROL PIT YAW RAC PAC YAC OBS MAC; do

	# Skip headings.

	if [[ "${NUM}" == "NUM," ]]; then
		continue
	fi

	NUM=${NUM%,}
	if [[ ${NUM} -le ${PRIOR} ]]; then
		continue
	fi

	# Skip stationary.

	LATITUDE=${LAT%,}
	LONGITUDE=${LON%,}
	if [[ "${LATITUDE}" != "${OLDLATITUDE}" ]]; then
		:
	elif [[ "${LONGITUDE}" != "${OLDLONGITUDE}" ]]; then
		:
	else
		continue
	fi
	OLDLATITUDE=${LATITUDE}
	OLDLONGITUDE=${LONGITUDE}

	# Skip unless modulo.

	REMAINDER=$((${SEQUENCE} % ${MODULO}))
	if [[ ${REMAINDER} -eq 0 ]]; then

		if [[ -z "${INITIALIZED}" ]]; then
			echo -n '{ "PATH": [ '
			INITIALIZED=Y
		else
			echo -n ', '
		fi

		echo -n '[' "${LATITUDE}, ${LONGITUDE}" ']'

		LATITUDE=""
		LONGITUDE=""

	fi

	PRIOR=${NUM}
	SEQUENCE=$((${SEQUENCE} + 1))

done

if [[ -z "${LATITUDE}" ]]; then
	:
elif [[ -z "${LONGITUDE}" ]]; then
	:
else
	echo -n ', [' "${LATITUDE}, ${LONGITUDE}" ']'
fi

if [[ ${SEQUENCE} -gt 0 ]]; then
	echo ' ] }'
fi

exit 0
