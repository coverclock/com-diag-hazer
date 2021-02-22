#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Filter that reads CSV and outputs same with time converted to ISO8601.
# This makes it easier for humans to pick out intervals of interest.
# e.g. tail -f file.csv | csv2iso 
# usage: csv2iso < CSVFILE > CSVFILEWITHISO

while read NAM NUM FIX SYS SAT CLK TIM LAT LON HAC MSL GEP VAC SOG COG ROL PIT YAW RAC PAC YAC OBS MAC; do

	if [[ "${NUM}" == "NUM," ]]; then
		echo $NAM $NUM $FIX $SYS $SAT $CLK $TIM $LAT $LON $HAC $MSL $GEO $VAC $SOG $COG $ROL $PIT $YAW $RAC $PAC $YAC $OBS $MAC
		continue
	fi

	CLOCK=$(date -d "@${CLK%,}" '+%Y-%m-%dT%H:%M:%S.%N%:z,')

	TIME=$(date -d "@${TIM%,}" -u '+%Y-%m-%dT%H:%M:%S.%N%:z,')

	echo $NAM $NUM $FIX $SYS $SAT ${CLOCK} ${TIME} $LAT $LON $HAC $MSL $GEO $VAC $SOG $COG $ROL $PIT $YAW $RAC $PAC $YAC $OBS $MAC

done

exit 0

