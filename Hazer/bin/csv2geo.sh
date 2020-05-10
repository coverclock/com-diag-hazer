#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Compute the horizontal geodesic difference and vertical difference between
# successive points in the CSV file.

. $(readlink -e $(dirname ${0})/../bin)/setup

INIT=""
HEAD=""

while read NAM NUM CLK TIM LAT LON HOR MSL WGS VER SOG COG; do

	if [[ "${NUM}" == "OBSERVATION," ]]; then
		echo ${NAM} ${NUM} ${CLK} ${TIM} ${LAT} ${LON} ${HOR} ${MSL} ${WGS} ${VER} ${SOG} ${COG}, HDIFF, VDIFF
		continue
	fi

	if [[ -z "${INIT}" ]]; then
		LAT0=${LAT%,}
		LON0=${LON%,}
		MSL0=${MSL%,}
		WGS0=${WGS%,}
		echo ${NAM} ${NUM} ${CLK} ${TIM} ${LAT} ${LON} ${HOR} ${MSL} ${WGS} ${VER} ${SOG} ${COG}, 0.0, 0.0
		INIT=Y
		continue
	fi

	LAT1=${LAT%,}
	LON1=${LON%,}
	MSL1=${MSL%,}
	WGS1=${WGS%,}

        HORD=$(geodesic ${LAT0} ${LON0} ${LAT1} ${LON1})
	VERD=$(echo "print ${WGS1} - ${WGS0}" | bc)

	LAT0=${LAT1}
	LON0=${LON1}
	MSL0=${MSL1}
	WGS0=${WGS1}

	echo ${NAM} ${NUM} ${CLK} ${TIM} ${LAT} ${LON} ${HOR} ${MSL} ${WGS} ${VER} ${SOG} ${COG}, ${HORD}, ${VERD}

done

exit 0
