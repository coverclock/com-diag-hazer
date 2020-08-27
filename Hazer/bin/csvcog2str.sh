#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Converts a COG in degrees to a compass point.
# https://en.wikipedia.org/wiki/Points_of_the_compass

COG=${1:-""}
COMPASS="X"

if [[ -n "${COG}" ]]; then
	COG=$((${COG%.*} % 360))
	if [[ ${COG} -lt 11 ]]; then
		COMPASS="N"
	elif [[ ${COG} -lt 34 ]]; then
		COMPASS="NNE"
	elif [[ ${COG} -lt 56 ]]; then
		COMPASS="NE"
	elif [[ ${COG} -lt 79 ]]; then
		COMPASS="ENE"
	elif [[ ${COG} -lt 101 ]]; then
		COMPASS="E"
	elif [[ ${COG} -lt 124 ]]; then
		COMPASS="ESE"
	elif [[ ${COG} -lt 146 ]]; then
		COMPASS="SE"
	elif [[ ${COG} -lt 169 ]]; then
		COMPASS="SSE"
	elif [[ ${COG} -lt 191 ]]; then
		COMPASS="S"
	elif [[ ${COG} -lt 213 ]]; then
		COMPASS="SSW"
	elif [[ ${COG} -lt 236 ]]; then
		COMPASS="SW"
	elif [[ ${COG} -lt 259 ]]; then
		COMPASS="WSW"
	elif [[ ${COG} -lt 281 ]]; then
		COMPASS="W"
	elif [[ ${COG} -lt 304 ]]; then
		COMPASS="WNW"
	elif [[ ${COG} -lt 326 ]]; then
		COMPASS="NW"
	elif [[ ${COG} -lt 349 ]]; then
		COMPASS="NNW"
	elif [[ ${COG} -lt 361 ]]; then
		COMPASS="N"
	else
		COMPASS="O"
	fi
fi

echo ${COMPASS}

exit 0
