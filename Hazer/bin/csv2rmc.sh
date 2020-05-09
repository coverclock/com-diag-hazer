#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Filter that reads CSV and outputs NMEA RMC.
# e.g. tail -f file.csv | csv2rmc | gpstool -E

while read NAM NUM CLK TIM LAT LON HOR MSL WGS VER SOG COG; do

	if [[ "${NUM}" == "OBSERVATION," ]]; then
		continue
	fi

	DAT=${TIM%,}
	TIME=$(date -d "@${DAT}" -u '+%H%M%S.00')
	DATE=$(date -d "@${DAT}" -u '+%d%m%y')

	DAT=${LAT%,}
	NUM=${DAT%.*}
	DEG=${NUM#-}
	if [[ "${DEG}" == "${NUM}" ]]; then
		NS="N"
	else
		NS="S"
	fi
	NAN=${DAT#*.}
	TMP=$((${NAN} * 60))
	MIN=$((${TMP} / 1000000000))
	MIL=$((${TMP} % 1000000000 / 1000))
	printf -v LATITUDE "%d%02d.%06d" ${DEG} ${MIN} ${MIL}
	
	DAT=${LON%,}
	NUM=${DAT%.*}
	DEG=${NUM#-}
	if [[ "${DEG}" == "${NUM}" ]]; then
		EW="E"
	else
		EW="W"
	fi
	NAN=${DAT#*.}
	TMP=$((${NAN} * 60))
	MIN=$((${TMP} / 1000000000))
	MIL=$((${TMP} % 1000000000 / 1000))
	printf -v LONGITUDE "%d%02d.%06d" ${DEG} ${MIN} ${MIL}

	SOG=${SOG%,}

	COG=${COG%,}

	RMC="\$GNRMC,${TIME},A,${LATITUDE},${NS},${LONGITUDE},${EW},${SOG},${COG},${DATE},,,A,V"
	NMEA=$(checksum ${RMC})
	echo -n -e ${NMEA}

done

exit 0
