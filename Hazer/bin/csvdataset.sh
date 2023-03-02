#!/bin/bash
# Copyright 2021 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Filter that reads CSV and outputs a JSON array to visualize a polyline.
# Because the array has to be imported and stored entirely in memory,
# a modulo value can be used to only select a sample of the incoming CSV.
# The default is modulo 1 (every sample) but for example modulo 10 would
# select every tenth sample. Regardless of the modulo, the first and last
# sample are selected so that the starting and ending points are included.
# To conserve memory, points that have no change in latitude and longitude
# are skipped.

# usage: csvdataset [ MODULO ] < CSVFILE > JSONFILE
# default: csvdataset 1 < CSVFILE > JSONFILE

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

	# Skip retrograde.

	NUM=${NUM%,}
	if [[ ${NUM} -le ${PRIOR} ]]; then
		continue
	fi
	PRIOR=${NUM}

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

	# Skip non-modulo.

	REMAINDER=$((${SEQUENCE} % ${MODULO}))
	if [[ ${REMAINDER} -eq 0 ]]; then

		if [[ -z "${INITIALIZED}" ]]; then
			echo '{'
			echo '  "PATH": [ '
			echo -n '      '
			INITIALIZED=Y
		else
			echo -n '    , '
		fi

		echo '[' "${LATITUDE}, ${LONGITUDE}" ']'

		LATITUDE=""
		LONGITUDE=""

	fi
	SEQUENCE=$((${SEQUENCE} + 1))

done

if [[ -z "${LATITUDE}" ]]; then
	:
elif [[ -z "${LONGITUDE}" ]]; then
	:
else
	echo '    , [' "${LATITUDE}, ${LONGITUDE}" ']'
fi

if [[ ${SEQUENCE} -gt 0 ]]; then
	echo '  ]'
	echo '}'
fi

exit 0
