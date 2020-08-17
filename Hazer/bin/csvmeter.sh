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

while read NAM NUM FIX SYS SAT CLK TIM LAT LON HAC MSL GEO VAC SOG COG ROL PIT YAW RAC PAC YAC OBS MAC; do

    if [[ "${NUM}" == "NUM," ]]; then
        continue
    fi

    CURRENT=${CLK%,}
    if [[ "${PREVIOUS}" != "0" ]]; then
        ELAPSED=$(echo "print ${CURRENT} - ${PREVIOUS}" | bc)
	sleep ${ELAPSED}
    fi
    PREVIOUS=${CURRENT}

    echo $NAM $NUM $FIX $SYS $SAT $CLK $TIM $LAT $LON $HAC $MSL $GEO $VAC $SOG $COG $ROL $PIT $YAW $RAC $PAC $YAC $OBS $MAC

done

exit 0
