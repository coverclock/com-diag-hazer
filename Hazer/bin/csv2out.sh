#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Filter that reads CSV and outputs same with time converted to ISO8601.
# This makes it easier for humans to pick out intervals of interest.
# e.g. tail -f file.csv | csv2iso 

DEG="°"

while read NAM NUM FIX SYS SAT CLK TIM LAT LON HAC MSL GEO VAC SOG COG ROL PIT YAW RAC PAC YAC OBS MAC; do

	if [[ "${NUM}" == "NUM," ]]; then
		continue
	fi

	# 39°47'39.190559"N, 105°09'12.088799"W

	POSITION=$(mapstool -P "${LAT%,} ${LON%,}")
	POSITION=$(echo ${POSITION} | sed 's/\.[0-9][0-9]*"/"/g')

	case "${SYS%,}" in
	0) SYSTEM="GN";;
	1) SYSTEM="GP";;
	2) SYSTEM="GL";;
	3) SYSTEM="GA";;
	4) SYSTEM="BD";;
	*) SYSTEM="XX";;
	esac

	case "${FIX%,}" in
	0) TYPE="NO";;
	1) TYPE="IN";;
	2) TYPE="2D";;
	3) TYPE="3D";;
	4) TYPE="GI";;
	5) TYPE="TM";;
	*) TYPE="XX";;
	esac

	TIME=$(date -d "@${TIM%,}" -u '+%Y-%m-%dT%H:%M:%SZ')

	DEGREES=${COG%.*}
	if [[ ${DEGREES} -lt 11 ]]; then
		COMPASS="N"
	elif [[ ${DEGREES} -lt 34 ]]; then
		COMPASS="NNE"
	elif [[ ${DEGREES} -lt 56 ]]; then
		COMPASS="NE"
	elif [[ ${DEGREES} -lt 79 ]]; then
		COMPASS="ENE"
	elif [[ ${DEGREES} -lt 101 ]]; then
		COMPASS="E"
	elif [[ ${DEGREES} -lt 124 ]]; then
		COMPASS="ESE"
	elif [[ ${DEGREES} -lt 146 ]]; then
		COMPASS="SE"
	elif [[ ${DEGREES} -lt 169 ]]; then
		COMPASS="SSE"
	elif [[ ${DEGREES} -lt 191 ]]; then
		COMPASS="S"
	elif [[ ${DEGREES} -lt 213 ]]; then
		COMPASS="SSW"
	elif [[ ${DEGREES} -lt 236 ]]; then
		COMPASS="SW"
	elif [[ ${DEGREES} -lt 259 ]]; then
		COMPASS="WSW"
	elif [[ ${DEGREES} -lt 281 ]]; then
		COMPASS="W"
	elif [[ ${DEGREES} -lt 304 ]]; then
		COMPASS="WNW"
	elif [[ ${DEGREES} -lt 326 ]]; then
		COMPASS="NW"
	elif [[ ${DEGREES} -lt 349 ]]; then
		COMPASS="NNW"
	else
		COMPASS="X"
	fi

	echo ${SYSTEM} ${SAT%,} ${TYPE} "|" ${TIME} "|" ${POSITION} "|" ${MSL%.*}m "|" ${SOG%.*}kn "|" ${COG%.*}${DEG} ${COMPASS} "|"  ${ROL%.*}${DEG}, ${PIT%.*}${DEG}, ${YAW%.*}${DEG}

done

exit 0
