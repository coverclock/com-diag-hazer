#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Added columns with tha local interarrival time
# and its running average.
#

PREVIOUS=0
ELAPSED=0.0
COUNT=0
TOTAL=0.0
MEAN=0.0

while read NAM NUM CLK TIM LAT LON HAC MSL WGS VAC SOG COG ROL PIT YAW RAC PAC YAC; do

    if [[ "${NUM}" == "OBSERVATION," ]]; then
    	echo ${NAM} ${NUM} ${CLK} ${TIM} ${LAT} ${LON} ${HAC} ${MSL} ${WGS} ${VAC} ${SOG} ${COG} ${ROL} ${PIT} ${YAW} ${RAC} ${PAC} ${YAC}, ELAPSED, MEAN
        continue
    fi

    CURRENT=${CLK%,}
    if [[ ${COUNT} -gt 0 ]]; then
        ELAPSED=$(echo "print ${CURRENT} - ${PREVIOUS}" | bc -l)
	TOTAL=$(echo "print ${TOTAL} + ${ELAPSED}" | bc -l)
	MEAN=$(echo "print ${TOTAL} / ${COUNT}" | bc -l)
    fi
    PREVIOUS=${CURRENT}
    COUNT=$((${COUNT} + 1))

    echo ${NAM} ${NUM} ${CLK} ${TIM} ${LAT} ${LON} ${HAC} ${MSL} ${WGS} ${VAC} ${SOG} ${COG} ${ROL} ${PIT} ${YAW} ${RAC} ${PAC} ${YAC}, ${ELAPSED}, ${MEAN}

done

exit 0
