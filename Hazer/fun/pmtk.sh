#!/bin/bash
# Copyright 2021 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Used with the Adafruit Ultimate GPS which uses a GTop
# device based on an MTK3339 chipset with the PMTK message
# set.

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}
mkdir -p ${SAVDIR}

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyUSB0"}
RATE=${2:-9600}

ERRFIL="${SAVDIR}/${PROGRAM}.err"
mkdir -p $(dirname ${ERRFIL})

. $(readlink -e $(dirname ${0})/../bin)/setup

gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -F -W '$PMTK605' 2>> ${ERRFIL}
