#!/bin/bash
# Copyright 2019-2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Configure and run the U-blox ZED-UBX-F9P as a stationary Base in survey-in
# mode (if a fix file does not exist) or fixed mode (if it does) sending
# corrections to Rovers.
# IMPORTANT SAFETY TIP: when switching the F9P from FIXED back to SVIN mode,
# consider power cycling or otherwise resetting the device first.

FIXDIR=${COM_DIAG_HAZER_SAVDIR:-${HOME:-"/var/tmp"}/fix}
SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}

PROGRAM=$(basename ${0})
ROUTER=${1:-"tumbleweed:tumbleweed"}
DEVICE=${2:-"/dev/tumbleweed"}
RATE=${3:-230400}
DURATION=${4:-300}
ACCURACY=${5:-250}
FIXFIL=${6-"${FIXDIR}/${PROGRAM}.fix"}
ERRFIL=${7-"${SAVDIR}/${PROGRAM}.err"}
OUTFIL=${8-"${SAVDIR}/${PROGRAM}.out"}
CSVFIL=${9-"${SAVDIR}/${PROGRAM}.csv"}

mkdir -p $(dirname ${FIXFIL})
mkdir -p $(dirname ${ERRFIL})
mkdir -p $(dirname ${OUTFIL})
mkdir -p $(dirname ${CSVFIL})

. $(readlink -e $(dirname ${0})/../bin)/setup

if [[ ! -r ${FIXFIL} ]]; then
	exec survey ${ROUTER} ${DEVICE} ${RATE} ${DURATION} ${ACCURACY} ${FIXFIL} ${ERRFIL} ${OUTFIL} ${CSVFIL}
elif [[ $(wc -l ${FIXFIL}) < 7 ]]; then
	exec survey ${ROUTER} ${DEVICE} ${RATE} ${DURATION} ${ACCURACY} ${FIXFIL} ${ERRFIL} ${OUTFIL} ${CSVFIL}
else
	exec fixed ${ROUTER} ${DEVICE} ${RATE} ${FIXFIL} ${ERRFIL} ${OUTFIL}
fi
