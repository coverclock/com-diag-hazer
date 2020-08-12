#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Filter that reads CSV and outputs same with time converted to ISO8601.
# This makes it easier for humans to pick out intervals of interest.
# e.g. tail -f file.csv | csv2iso 

while read NAM NUM FIX SYS CLK TIM LAT LON HAC MSL GEP VAC SOG COG ROL PIT YAW RAC PAC YAC; do

	if [[ "${NUM}" == "NUM," ]]; then
		echo $NAM $NUM $FIX $SYS $CLK $TIM $LAT $LON $HAC $MSL $GEO $VAC $SOG $COG $ROL $PIT $YAW $RAC $PAC $YAC
		continue
	fi

	CLOCK=$(date -d "@${CLK%,}" -u '+%Y-%m-%dT%H:%M:%S.%N+00:00,')

	TIME=$(date -d "@${TIM%,}" -u '+%Y-%m-%dT%H:%M:%S.%N+00:00,')

	echo $NAM $NUM $FIX $SYS ${CLOCK} ${TIME} $LAT $LON $HAC $MSL $GEO $VAC $SOG $COG $ROL $PIT $YAW $RAC $PAC $YAC

done

exit 0

