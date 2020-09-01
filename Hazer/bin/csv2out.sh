#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Filter that reads CSV and outputs same with time converted to ISO8601.
# This makes it easier for humans to pick out intervals of interest.
# e.g. tail -f file.csv | csv2iso 

while read NAM NUM FIX SYS SAT CLK TIM LAT LON HAC MSL GEO VAC SOG COG ROL PIT YAW RAC PAC YAC OBS MAC; do

	if [[ "${NUM}" == "NUM," ]]; then
		continue
	fi

	# 39°47'39.190559"N, 105°09'12.088799"W

	POSITION=$(echo $(mapstool -P "${LAT%,} ${LON%,}") | sed 's/\.[0-9][0-9]*"/"/g')
	LATITUDE=${POSITION%%,*}
	LONGITUDE=${POSITION##*, }

	SYSTEM=$(csvsys2str ${SYS})

	TYPE=$(csvfix2str ${FIX})

	TIME=$(date -d "@${TIM%,}" -u '+%Y-%m-%dT%H:%M:%SZ')

	COMPASS=$(compasstool ${COG})

	printf "%-2s %-2s %-2s | %-20s | %12s, %12s | %5sm | %4skn | %3s° %-3s | %3s°, %3s°, %3s°\n" "${SYSTEM}" "${SAT%,}" "${TYPE}" "${TIME}" "${LATITUDE}" "${LONGITUDE}" "${MSL%.*}" "${SOG%.*}" "${COG%.*}" "${COMPASS}" "${ROL%.*}" "${PIT%.*}" "${YAW%.*}" 

done

exit 0
