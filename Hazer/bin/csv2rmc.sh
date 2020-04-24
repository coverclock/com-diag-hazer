#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

while read NUM CLK TIM LAT LON HOR MSL WGS VER; do
	if [[ "${NUM}" != "OBSERVATION" ]]; then

		DAT=${TIM%,}
		TIME=$(date -d "@${DAT}" -u '+%H%M%S.00')
		DATE=$(date -d "@${DAT}" -u '+%d%m%y')
		echo $TIM $TIME $DATE

		DAT=${LAT%,}
		NUM=${DAT%.*}
		DEG=${NUM#-}
		if [[ "${DEG}" == "${NUM}" ]]; then
			NORS="N"
		else
			NORS="S"
		fi
		NAN=${DAT#*.}
		MIN=$((${NAN} * 60 / 1000000000))
		NAN=$((${NAN} / 60))
		MIL=$((${NAN} * 1000))
		printf -v LATITUDE "%d%02d.%06d" ${DEG} ${MIN} ${MIL}
		echo $LAT $LATITUDE $NORS
		
		DAT=${LON%,}
		NUM=${DAT%.*}
		DEG=${NUM#-}
		if [[ "${DEG}" == "${NUM}" ]]; then
			EORW="E"
		else
			EORW="W"
		fi
		NAN=${DAT#*.}
		MIN=$((${NAN} * 60 / 1000000000))
		NAN=$((${NAN} / 60))
		MIL=$((${NAN} / 1000))
		printf -v LONGITUDE "%d%02d.%06d" ${DEG} ${MIN} ${MIL}
		echo $LON $LONGITUDE $EORW

	fi
done

exit 0
$GNRMC,185535.00,A,3947.66076,N,10509.19605,W,1.930,21.53,240420,,,A,V*27
