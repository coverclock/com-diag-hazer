#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Meters CSV entries from stdin to stdout based
# on the interarrival time between successive
# lines.
#

PREVIOUS=0

while read NUM CLK TIM LAT LON HOR MSL WGS VER SOG COG; do

    if [[ "${NUM}" == "OBSERVATION," ]]; then
        continue
    fi

    CURRENT=${CLK%,}
    if [[ "${PREVIOUS}" != "0" ]]; then
        ELAPSED=$(echo "print ${CURRENT} - ${PREVIOUS}" | bc)
	sleep ${ELAPSED}
    fi
    PREVIOUS=${CURRENT}

    echo ${NUM} ${CLK} ${TIM} ${LAT} ${LON} ${HOR} ${MSL} ${WGS} ${VER} ${SOG} ${COG}

done

exit 0

