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

	CLOCK=$(date -d "@${CLK%,}" -u '+%Y-%m-%dT%H:%M:%S.%N+00:00,')

	TIME=$(date -d "@${TIM%,}" -u '+%Y-%m-%dT%H:%M:%S.%N+00:00,')

	echo ${NUM%,} ${TYPE} ${SYSTEM} ${CLOCK} ${TIME} ${POSITION} ${MSL%,}m ${GEO%,}m ${SOG%,}kn ${COG%,}${DEG} ${ROL%,}${DEG} ${PIT%,}${DEG} ${YAW%,}${DEG}

done

exit 0
