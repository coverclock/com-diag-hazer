#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Added a column with the local interarrival
# time in seconds.
#

PREVIOUS=0
ELAPSED=0.0

while read NAM NUM CLK TIM LAT LON HOR MSL WGS VER SOG COG; do

    if [[ "${NUM}" == "OBSERVATION," ]]; then
    	echo ${NAM} ${NUM} ${CLK} ${TIM} ${LAT} ${LON} ${HOR} ${MSL} ${WGS} ${VER} ${SOG} ${COG}, ELAPSED
        continue
    fi

    CURRENT=${CLK%,}
    if [[ "${PREVIOUS}" != "0" ]]; then
        ELAPSED=$(echo "print ${CURRENT} - ${PREVIOUS}" | bc)
    fi
    PREVIOUS=${CURRENT}

    echo ${NAM} ${NUM} ${CLK} ${TIM} ${LAT} ${LON} ${HOR} ${MSL} ${WGS} ${VER} ${SOG} ${COG}, ${ELAPSED}

done

exit 0
