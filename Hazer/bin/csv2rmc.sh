#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

while read NUM CLK TIM LAT LON HOR MSL WGS VER; do
	if [[ "${NUM}" != "OBSERVATION" ]]; then

		TIM=${TIM%,}
		TIME=$(date -d "@${TIM}" -u '+%H%M%S.00')
		DATE=$(date -d "@${TIM}" -u '+%d%m%y')
		echo $TIME $DATE

		LAT=${LAT%,}
		NUM=${LAT%.*}
		DEG=${NUM#-}
		if [[ "${DEG}" == "${NUM}" ]]; then
			NORS="N"
		else
			NORS="S"
		fi
		NAN=${LAT#*.}
		MIN=$((${NAN} * 60 / 1000000000))
		echo $DEG $MIN $NORS
		
		LON=${LON%,}
		NUM=${LON%.*}
		DEG=${NUM#-}
		if [[ "${DEG}" == "${NUM}" ]]; then
			EORW="E"
		else
			EORW="W"
		fi
		NAN=${LON#*.}
		MIN=$((${NAN} * 60 / 1000000000))
		echo $DEG $MIN $EORW

	fi
done

exit 0
$GNRMC,185535.00,A,3947.66076,N,10509.19605,W,1.930,21.53,240420,,,A,V*27
