#!/bin/bash
# Copyright 2023 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Used in support of the Fothergill LoraSerial project.
# THIS IS A WORK IN PROGRESS.

while read NAM NUM FIX SYS SAT CLK TIM LAT LON HAC MSL GEO VAC SOG COG ROL PIT YAW RAC PAC YAC OBS MAC; do

        if [[ "${NUM}" == "NUM," ]]; then
                continue
        fi

	if [[ "${TIM}" == "0.," ]]; then
		continue;
	fi

	NAM=${NAM##\"}
	NAM=${NAM%%\",}

        FIX=${FIX%,}
        FIX=$(csvfix2str ${FIX})

        SYS=${SYS%,}
        SYS=$(csvsys2str ${SYS})

        SAT=${SAT%,}

        TIM=${TIM%,}
        TIM=$(date -d "@${TIM}" -u '+%Y-%m-%dT%H:%M:%SZ')

        LAT=${LAT%,}
        LON=${LON%,}
        POS=$(mapstool -P "${LAT} ${LON}" | sed 's/\.[0-9][0-9]*"/"/g')
        LAT=${POS%%,*}
        LON=${POS##*, }

        MSL=${MSL%.*}

        SOG=${SOG%.*}

        COG=${COG%,}
        DIR=$(compasstool ${COG})
        COG=${COG%.*}

        printf "%s %-2s %2s %-2s %-20s %12s %12s %5sm %4skn %3s %-3s\n" "${NAM}" "${SYS}" "${SAT}" "${FIX}" "${TIM}" "${LAT}," "${LON}" "${MSL}" "${SOG}" "${COG}" "${DIR}"

done

exit 0
