#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Meters JSON entries from stdin to stdout based
# on the interarrival time between successive
# lines.
# usage: jsonmeter < JSONFILE | ...

PREVIOUS=0

while read LB KNAM NAM KNUM NUM KTIM TIM KLAT LAT KLON LON KMSL MSL KLBL LBL RB; do

    CURRENT=${TIM%,}
    if [[ "${PREVIOUS}" != "0" ]]; then
        ELAPSED=$(echo "print ${CURRENT} - ${PREVIOUS}" | bc)
	sleep ${ELAPSED}
    fi
    PREVIOUS=${CURRENT}

    echo $LB $KNAM $NAM $KNUM $NUM $KTIM $TIM $KLAT $LAT $KLON $LON $KMSL $MSL $KLBL $LBL $RB

done

exit 0

