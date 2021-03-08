#!/bin/bash
# Copyright 2021 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Filter that reads CSV and outputs a JSON array to visualize a polyline.
# usage: csv2json < CSVFILE > JSONFILE

PROGRAM=$(basename ${0})
MODULO=${1:-1}

PRIOR=0
SEQUENCE=0

while read NAM NUM FIX SYS SAT CLK TIM LAT LON HAC MSL GEO VAC SOG COG ROL PIT YAW RAC PAC YAC OBS MAC; do

	if [[ "${NUM}" == "NUM," ]]; then
		continue
	fi

	NUM=${NUM%,}

	if [[ ${NUM} -le ${PRIOR} ]]; then
		continue
	fi

	REMAINDER=$((${SEQUENCE} % ${MODULO}))
	if [[ ${REMAINDER} -eq 0 ]]; then

		if [[ ${SEQUENCE} -eq 0 ]]; then
			echo -n '{ "PATH": [ '
			INITIALIZED=Y
		else
			echo -n ', '
		fi

		LATITUDE=${LAT%,}
		LONGITUDE=${LON%,}

		echo -n '[' "${LATITUDE}, ${LONGITUDE}" ']'

	fi

	PRIOR=${NUM}
	SEQUENCE=$((${SEQUENCE} + 1))

done

echo
echo ' ] }'

exit 0
