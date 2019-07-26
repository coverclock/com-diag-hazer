#!/bin/bash
# Copyright 2019 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

PROGRAM=$(basename ${0})
DELAY=${1:-"3"}

while true; do
	DATE=$(date -u +%Y-%m-%dT%H:%M:%S)
	TEMPERATURE=$(/opt/vc/bin/vcgencmd measure_temp | sed 's/^.*=//')
	echo ${PROGRAM} ${DATE} ${TEMPERATURE}
	sleep ${DELAY}
done
