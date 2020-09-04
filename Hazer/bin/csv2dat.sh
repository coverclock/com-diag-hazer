#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Similar to csv2dat but IMO better for post-processing.

while read NAM NUM FIX SYS SAT CLK TIM LAT LON HAC MSL GEO VAC SOG COG ROL PIT YAW RAC PAC YAC OBS MAC; do

	if [[ "${NUM}" == "NUM," ]]; then
		continue
	fi

	SYSTEM=$(csvsys2str ${SYS})

	SAT=${SAT%,}

	TYPE=$(csvfix2str ${FIX})

	CLOCK=$(date -d "@${CLK%,}" '+%Y-%m-%dT%H:%M:%SJ')

	TIME=$(date -d "@${TIM%,}" -u '+%Y-%m-%dT%H:%M:%SZ')

	POSITION=$(mapstool -P "${LAT%,} ${LON%,}")
	LATITUDE=${POSITION%%,*}
	LONGITUDE=${POSITION##*, }

	MSL=${MSL%.*}

	SOG=${SOG%.*}

	COG=${COG%.*}
	COMPASS=$(compasstool ${COG})

	ROL=${ROL%.*}
	PIT=${PIT%.*}
	YAW=${YAW%.*}

	printf "%-2s %-2s %-2s | %-20s | %-20s | %19s, %19s | %5sm | %4skn | %3s° %-3s | %3s°, %3s°, %3s°\n" "${SYSTEM}" "${SAT}" "${TYPE}" "${CLOCK}" "${TIME}" "${LATITUDE}" "${LONGITUDE}" "${MSL}" "${SOG}" "${COG}" "${COMPASS}" "${ROL}" "${PIT}" "${YAW}" 

done

exit 0
