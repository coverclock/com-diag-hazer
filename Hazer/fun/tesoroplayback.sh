#!/bin/bash
# Copyright 2021 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

PROGRAM=$(basename ${0})
INPUT=${1:-$(readlink -e $(dirname ${0})/../dat)/yodel/20200903/vehicle.csv}
ROOT=${2:-${HOME}/Desktop/Observations}

. $(readlink -e $(dirname ${0})/../bin)/setup

mkdir -P ${ROOT}

csv2meter < ${INPUT} | csv2dgm -F ${ROOT}/Observation.json -j
