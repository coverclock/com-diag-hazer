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

	POSITION=$(echo $(mapstool -P "${LAT%,} ${LON%,}") | sed 's/\.[0-9][0-9]*"/"/g')

	SYSTEM=$(csvsys2str ${SYS})

	TYPE=$(csvfix2str ${FIX})

	TIME=$(date -d "@${TIM%,}" -u '+%Y-%m-%dT%H:%M:%SZ')

	COMPASS=$(compasstool ${COG})

	echo ${SYSTEM} ${SAT%,} ${TYPE} "|" ${TIME} "|" ${POSITION} "|" ${MSL%.*}m "|" ${SOG%.*}kn "|" ${COG%.*}${DEG} ${COMPASS} "|"  ${ROL%.*}${DEG}, ${PIT%.*}${DEG}, ${YAW%.*}${DEG}

done

exit 0
