#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Filter that reads CSV and outputs same with time converted to ISO8601.
# This makes it easier for humans to pick out intervals of interest.
# e.g. tail -f file.csv | csv2tty 
# usage: csv2tty < CSVFILE

while read NAM NUM FIX SYS SAT CLK TIM LAT LON HAC MSL GEO VAC SOG COG ROL PIT YAW RAC PAC YAC OBS MAC; do

	if [[ "${NUM}" == "NUM," ]]; then
		continue
	fi

	FIX=${FIX%,}
	TYPE=$(csvfix2str ${FIX})

	SYS=${SYS%,}
	SYSTEM=$(csvsys2str ${SYS})

	SAT=${SAT%,}

	TIM=${TIM%,}
	TIME=$(date -d "@${TIM}" -u '+%Y-%m-%dT%H:%M:%SZ')

	LAT=${LAT%,}
	LON=${LON%,}
	POSITION=$(mapstool -P "${LAT} ${LON}" | sed 's/\.[0-9][0-9]*"/"/g')
	LATITUDE=${POSITION%%,*}
	LONGITUDE=${POSITION##*, }

	MSL=${MSL%.*}

	SOG=${SOG%.*}

	COG=${COG%,}
	COMPASS=$(compasstool ${COG})
	COG=${COG%.*}

	ROL=${ROL%.*}
	PIT=${PIT%.*}
	YAW=${YAW%.*}

	printf "%-2s %-2s %-2s | %-20s | %12s, %12s | %5sm | %4skn | %3s° %-3s | %3s°, %3s°, %3s°\n" "${SYSTEM}" "${SAT}" "${TYPE}" "${TIME}" "${LATITUDE}" "${LONGITUDE}" "${MSL}" "${SOG}" "${COG}" "${COMPASS}" "${ROL}" "${PIT}" "${YAW}" 

done

exit 0
