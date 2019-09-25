#!/bin/bash 
# Copyright 2019 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Configure and run the U-blox UBX-ZED-F9P as a surveying base with very low standards.

PROGRAM=$(basename ${0})
ROUTER=${1:-"localhost:21010"}
DEVICE=${2:-"/dev/ttyACM0"}
RATE=${3:-230400}
DURATION=${4:-300}
ACCURACY=${5:-150000}
FIXFIL=${6-"./${PROGRAM}.fix"}
ERRFIL=${7-"./${PROGRAM}.err"}
OUTFIL=${8-"./${PROGRAM}.out"}

. $(readlink -e $(dirname ${0})/../bin)/setup

exec base ${ROUTER} ${DEVICE} ${RATE} ${DURATION} ${ACCURACY} ${FIXFIL} ${ERRFIL} ${OUTFIL}
