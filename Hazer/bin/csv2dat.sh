#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Similar to csv2tty but IMO better for post-processing analysis.
# usage: csv2dat < CSVFILE

while read NAM NUM FIX SYS SAT CLK TIM LAT LON HAC MSL GEO VAC SOG COG ROL PIT YAW RAC PAC YAC OBS MAC; do

	if [[ "${NUM}" == "NUM," ]]; then
		continue
	fi

	NUM=${NUM%,}

	SYSTEM=$(csvsys2str ${SYS})

	SAT=${SAT%,}

	TYPE=$(csvfix2str ${FIX})

	CLOCK=$(date -d "@${CLK%,}" '+%Y-%m-%dT%H:%M:%SJ')

	TIME=$(date -d "@${TIM%,}" -u '+%Y-%m-%dT%H:%M:%SZ')

	POSITION=$(mapstool -P "${LAT%,} ${LON%,}")
	LATITUDE=${POSITION%%,*}
	LONGITUDE=${POSITION##*, }

	MSL=${MSL%,}
	FEET=$(echo "print ${MSL}" '*' "3.28084" | bc -l)
	FEET=${FEET%.*}
	if [ -z "${FEET}" ]; then FEET=0; fi
	MSL=${MSL%.*}

	SOG=${SOG%,}
	MPH=$(echo "print ${SOG}" '*' "1.15078" | bc -l)
	MPH=${MPH%.*}
	if [ -z "${MPH}" ]; then MPH=0; fi
	KPH=$(echo "print ${SOG}" '*' "1.852" | bc -l)
	KPH=${KPH%.*}
	if [ -z "${KPH}" ]; then KPH=0; fi
	SOG=${SOG%.*}

	COG=${COG%.*}
	COMPASS=$(compasstool ${COG})

	ROL=${ROL%.*}
	PIT=${PIT%.*}
	YAW=${YAW%.*}

	printf "%-2s %-2s %-2s | %6s | %-20s | %-20s | %19s, %19s | %5sm %6sft | %4skn %4smph %5skph | %3s° %-3s | %3s°, %3s°, %3s°\n" "${SYSTEM}" "${SAT}" "${TYPE}" "${NUM}" "${CLOCK}" "${TIME}" "${LATITUDE}" "${LONGITUDE}" "${MSL}" "${FEET}" "${SOG}" "${MPH}" "${KPH}" "${COG}" "${COMPASS}" "${ROL}" "${PIT}" "${YAW}" 

done

exit 0
