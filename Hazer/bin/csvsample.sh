#!/bin/bash
# Copyright 2021 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Filter that reads CSV in a manner similar to csvdataset.sh but instead
# emitting JSON, emits the sampled CSV.  The default is modulo 1
# (every sample) but for example modulo 10 would select every tenth
# sample. Regardless of the modulo, the first and last sample are
# selected so that the starting and ending points are included. To
# conserve memory, points that have no change in latitude and longitude
# are skipped.

# usage: csv2sample [ MODULO ] < CSVFILE > JSONFILE
# default: csvsample 1 < CSVFILE > JSONFILE

PROGRAM=$(basename ${0})
MODULO=${1:-1}

PRIOR=0
SEQUENCE=0
LATITUDE=""
LONGITUDE=""
OLDLATITUDE=""
OLDLONGITUDE=""

while read NAM NUM FIX SYS SAT CLK TIM LAT LON HAC MSL GEO VAC SOG COG ROL PIT YAW RAC PAC YAC OBS MAC; do

	# Do headings.

	if [[ "${NUM}" == "NUM," ]]; then
		echo $NAM, $NUM, $FIX, $SYS, $SAT, $CLK, $TIM, $LAT, $LON, $HAC, $MSL, $GEO, $VAC, $SOG, $COG, $ROL, $PIT, $YAW, $RAC, $PAC, $YAC, $OBS, $MAC
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
		echo $NAM, $NUM, $FIX, $SYS, $SAT, $CLK, $TIM, $LAT, $LON, $HAC, $MSL, $GEO, $VAC, $SOG, $COG, $ROL, $PIT, $YAW, $RAC, $PAC, $YAC, $OBS, $MAC
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
	echo $NAM, $NUM, $FIX, $SYS, $SAT, $CLK, $TIM, $LAT, $LON, $HAC, $MSL, $GEO, $VAC, $SOG, $COG, $ROL, $PIT, $YAW, $RAC, $PAC, $YAC, $OBS, $MAC
fi

exit 0

