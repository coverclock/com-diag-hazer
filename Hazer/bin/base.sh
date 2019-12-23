#!/bin/bash
# Copyright 2019 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Configure and run the U-blox ZED-UBX-F9P as a stationary Base in survey-in
# mode (if a fix file does not exist) or fixed mode (if it does) sending
# corrections to Rovers.
# IMPORTANT SAFETY TIP: when switching the F9P from FIXED back to SVIN mode,
# consider power cycling or otherwise resetting the device first.

PROGRAM=$(basename ${0})
ROUTER=${1:-"tumbleweed:tumbleweed"}
DEVICE=${2:-"/dev/tumbleweed"}
RATE=${3:-230400}
DURATION=${4:-300}
ACCURACY=${5:-250}
FIXFIL=${6-"./${PROGRAM}.fix"}
ERRFIL=${7-"./${PROGRAM}.err"}
OUTFIL=${8-"./${PROGRAM}.out"}

. $(readlink -e $(dirname ${0})/../bin)/setup

export LANG=en_US.UTF-8
export LANGUAGE=en_US.UTF-8
export LC_ALL=en_US.UTF-8

if [[ ! -r ${FIXFIL} ]]; then
	exec survey ${ROUTER} ${DEVICE} ${RATE} ${DURATION} ${ACCURACY} ${FIXFIL} ${ERRFIL} ${OUTFIL}
elif [[ $(wc -l ${FIXFIL}) < 7 ]]; then
	exec survey ${ROUTER} ${DEVICE} ${RATE} ${DURATION} ${ACCURACY} ${FIXFIL} ${ERRFIL} ${OUTFIL}
else
	exec fixed ${ROUTER} ${DEVICE} ${RATE} ${FIXFIL} ${ERRFIL} ${OUTFIL}
fi
