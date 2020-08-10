#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Split a potentially very large CSV file into many individual CSV
# files stored in individual directories.
# e.g. csvparts < file.csv

PROGRAM=$(basename ${0})
MAXIMUM=${1:-3600}
MODULO=${2:-24}

HEADER=""
COUNT=0
INDEX=0

BASDIR=${PROGRAM}.dir
mkdir ${BASDIR}

while read NAM NUM CLK TIM LAT LON HAC MSL WGS VAC SOG COG ROL PIT YAW RAC PAC YAC; do

	if [[ "${NUM}" == "OBSERVATION," ]]; then
		HEADER="$NAM $NUM $CLK $TIM $LAT $LON $HAC $MSL $WGS $VAC $SOG $COG $ROL $PIT $YAW $RAC $PAC $YAC"
		continue
	fi

	if [[ -z "${HEADER}" ]]; then
		continue
	fi

	if [[ ${COUNT} -eq 0 ]]; then
		TMPFIL=$(mktemp ${PROGRAM}-XXXXXXXXXX)
		exec 1>>${TMPFIL}
		echo ${HEADER}
	fi

	echo $NAM $NUM $CLK $TIM $LAT $LON $HAC $MSL $WGS $VAC $SOG $COG $ROL $PIT $YAC $RAC $PAC $YAC

	COUNT=$((${COUNT} + 1))
	if [[ ${COUNT} -ge ${MAXIMUM} ]]; then
		DATDIR=$(printf "%04d" $((${INDEX} / ${MODULO})))".dir"
		mkdir -p ${BASDIR}/${DATDIR}
		mv ${TMPFIL} ${BASDIR}/${DATDIR}/$(printf "%04d" ${INDEX}).csv
		INDEX=$((${INDEX} + 1))
		COUNT=0
	fi

done

if [[ ${COUNT} -gt 0 ]]; then
	mv ${TMPFIL} ${BASDIR}/${DATDIR}/$(printf "%04d" ${INDEX}).csv
fi

exit 0
