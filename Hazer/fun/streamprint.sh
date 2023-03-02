#!/bin/bash
# Copyright 2021 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Crazily verbose test with minimal processing to look at the
# details of the stuff being emitted by a device.

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}
mkdir -p ${SAVDIR}

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-9600}
RAWFIL=${3-"${SAVDIR}/${PROGRAM}.raw"}
HEXFIL=${4-"${SAVDIR}/${PROGRAM}.hex"}
INPFIL=${5-"${SAVDIR}/${PROGRAM}.inp"}
LOGFIL=${5-"${SAVDIR}/${PROGRAM}.log"}
ERRFIL=${6-"${SAVDIR}/${PROGRAM}.err"}
DATFIL=${7-"${SAVDIR}/${PROGRAM}.dat"}
DMPFIL=${8-"${SAVDIR}/${PROGRAM}.dmp"}

. $(readlink -e $(dirname ${0})/../bin)/setup

# Raw binary output from socat.
mkdir -p $(dirname ${RAWFIL})
cp /dev/null ${RAWFIL}

# phex interpreted output from socat.
mkdir -p $(dirname ${HEXFIL})
cp /dev/null ${HEXFIL}

# Catenate input to gpstool.
mkdir -p $(dirname ${INPFIL})
cp /dev/null ${INPFIL}

# Pretty printed input to gpstool.
mkdir -p $(dirname ${LOGFIL})
cp /dev/null ${LOGFIL}

# Error log from gpstool.
mkdir -p $(dirname ${ERRFIL})
cp /dev/null ${ERRFIL}

# Hex dump of raw binary output from socat.
mkdir -p $(dirname ${DATFIL})
cp /dev/null ${DATFIL}

# Hex dum of Catenate input to gpstool.
mkdir -p $(dirname ${DMPFIL})
cp /dev/null ${DMPFIL}

socat -u OPEN:${DEVICE},b${RATE} - | tee ${RAWFIL} | phex -t 2> ${HEXFIL} | gpstool -P -v -d -C ${INPFIL} -L ${LOGFIL} 2>&1 | tee ${ERRFIL}
dump ${RAWFIL} > ${DATFIL}
dump ${INPFIL} > ${DMPFIL}
