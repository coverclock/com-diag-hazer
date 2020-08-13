#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Compute the horizontal geodesic difference and vertical difference between
# successive points in the CSV file.

. $(readlink -e $(dirname ${0})/../bin)/setup

HEAD=""
INIT=""

while read NAM NUM FIX SYS SAT CLK TIM LAT LON HAC MSL GEO VAC SOG COG ROL PIT YAW RAC PAC YAC; do

	if [[ "${NUM}" == "NUM," ]]; then
		if [[ -z "${HEAD}" ]]; then
			echo $NAM $NUM $FIX $SYS $SAT $CLK $TIM $LAT $LON $HAC $MSL $GEO $VAC $SOG $COG $ROL $PIT $YAW $RAC $PAC $YAC, HDELTA, VDELTA
			HEAD=Y
		fi
		continue
	fi

	if [[ -z "${INIT}" ]]; then
		LAT0=${LAT%,}
		LON0=${LON%,}
		MSL0=${MSL%,}
		GEO0=${GEO%,}
		echo $NAM $NUM $FIX $SYS $SAT $CLK $TIM $LAT $LON $HAC $MSL $GEO $VAC $SOG $COG $ROL $PIT $YAW $RAC $PAC $YAC, 0.0, 0.0
		INIT=Y
		continue
	fi

	LAT1=${LAT%,}
	LON1=${LON%,}
	MSL1=${MSL%,}
	GEO1=${GEO%,}

        HDELTA=$(geodesic ${LAT0} ${LON0} ${LAT1} ${LON1})
	VDELTA=$(echo "print ${GEO1} - ${GEO0}" | bc)

	LAT0=${LAT1}
	LON0=${LON1}
	MSL0=${MSL1}
	GEO0=${GEO1}

	echo $NAM $NUM $FIX $SYS $SAT $CLK $TIM $LAT $LON $HAC $MSL $GEO $VAC $SOG $COG $ROL $PIT $YAW $RAC $PAC $YAC, ${HDELTA}, ${VDELTA}

done

exit 0
