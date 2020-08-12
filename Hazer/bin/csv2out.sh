#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Filter that reads CSV and outputs same with time converted to ISO8601.
# This makes it easier for humans to pick out intervals of interest.
# e.g. tail -f file.csv | csv2iso 

DEG="°"

while read NAM NUM FIX SYS CLK TIM LAT LON HAC MSL GEO VAC SOG COG ROL PIT YAW RAC PAC YAC; do

	if [[ "${NUM}" == "NUM," ]]; then
		continue
	fi

	POSITION=$(mapstool -P "${LAT%,} ${LON%,}")
	POSITION=${POSITION/,/}

	case "${FIX%,}" in
	0) TYPE="NONE";;
	1) TYPE="IMU";;
	2) TYPE="2D";;
	3) TYPE="3D";;
	4) TYPE="BOTH";;
	5) TYPE="TIME";;
	*) TYPE="OTHER";;
	esac

	case "${SYS%,}" in
	0) SYSTEM="GNSS";;
	1) SYSTEM="NAVSTAR";;
	2) SYSTEM="GLONASS";;
	3) SYSTEM="GALILEO";;
	4) SYSTEM="BEIDOU";;
	*) SYSTEM="OTHER";;
	esac

	TIME=$(date -d "@${TIM%,}" -u '+%Y-%m-%dT%H:%M:%S')

	COG=${COG%.*}
	if [[ ${COG} -lt 11 ]]; then
		COMPASS="N"
	elif [[ ${COG} -lt 34 ]]; then
		COMPASS="NNE"
	elif [[ ${COG} -lt 56 ]]; then
		COMPASS="NE"
	elif [[ ${COG} -lt 79 ]]; then
		COMPASS="ENE"
	elif [[ ${COG} -lt 101 ]]; then
		COMPASS="E"
	elif [[ ${COG} -lt 124 ]]; then
		COMPASS="ESE"
	elif [[ ${COG} -lt 146 ]]; then
		COMPASS="SE"
	elif [[ ${COG} -lt 169 ]]; then
		COMPASS="SSE"
	elif [[ ${COG} -lt 191 ]]; then
		COMPASS="S"
	elif [[ ${COG} -lt 213 ]]; then
		COMPASS="SSW"
	elif [[ ${COG} -lt 236 ]]; then
		COMPASS="SW"
	elif [[ ${COG} -lt 259 ]]; then
		COMPASS="WSW"
	elif [[ ${COG} -lt 281 ]]; then
		COMPASS="W"
	elif [[ ${COG} -lt 304 ]]; then
		COMPASS="WNW"
	elif [[ ${COG} -lt 326 ]]; then
		COMPASS="NW"
	elif [[ ${COG} -lt 349 ]]; then
		COMPASS="NNW"
	else
		COMPASS="?"
	fi

	echo "[" ${NUM%,} ${TYPE} ${SYSTEM} "] [" ${TIME} "] [" ${POSITION} "] [" ${MSL%,}m ${GEO%,}m "] [" ${SOG%,}kn "] [" ${COG}${DEG} ${COMPASS} "] ["  ${ROL%,}${DEG} ${PIT%,}${DEG} ${YAW%,}${DEG} "]"

done

exit 0
