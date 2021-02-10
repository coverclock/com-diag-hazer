#!/bin/bash
# Copyright 2021 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer


PROGRAM=$(basename ${0})
INPUT=${1:-$(readlink -e $(dirname ${0})/../../../dat/yodel)/20200917/vehicle.csv}
OUTPUT=${2:-/var/www/html/tesoro/channel/neon.json}
#OUTPUT=${2:-${HOME}/Desktop/Observations/Observation.json}

. $(readlink -e $(dirname ${0})/../bin)/setup

mkdir -p $(dirname ${OUTPUT})

rm -f ${OUTPUT}

csvmeter < ${INPUT} | csv2dgm -F ${OUTPUT} -j
