#!/bin/bash
# Copyright 2019 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

PROGRAM=$(basename ${0})
HOSTNAME=$(hostname)
DELAY=${1:-"3"}

while true; do
	DEGREE=$'\u00B0'
	TIMESTAMP=$(date -u +%Y-%m-%dT%H:%M:%S)
	TEMPERATURE=$(/opt/vc/bin/vcgencmd measure_temp | sed "s/'/${DEGREE}/; s/^.*=//")
	echo ${PROGRAM} ${TIMESTAMP}Z ${TEMPERATURE} ${HOSTNAME}
	sleep ${DELAY}
done
