#!/bin/bash
# Copyright 2023-2024 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# I'm using an Ardusimple SimpleGNSS board with a
# u-blox NEO-F10T module. This is the first GNSS
# device I've used that implements the NMEA 0183 4.11
# specification. N.B. the labeling of the PVT (1PPS)
# and POWER LEDs are reversed on this version of the
# SimpleGNSS board; later version may be fixed by
# Ardusimple. This script uses the device as is, in
# real-time instead of headless.

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}
mkdir -p ${SAVDIR}

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyUSB0"}
RATE=${2:-38400}
ONEPPS=${3:-"/dev/gpiochip4:18"} # pin 12 on Raspberry Pi 5
STROBE=${4:-"/dev/gpiochip4:16"} # pin 36 on Raspberry Pi 5

ERRFIL="${SAVDIR}/${PROGRAM}.err"
mkdir -p $(dirname ${ERRFIL})

# LSTFIL="${SAVDIR}/${PROGRAM}.lst"
# mkdir -p $(dirname ${LSTFIL})

. $(readlink -e $(dirname ${0})/../bin)/setup

eval coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -E -I ${ONEPPS} -p ${STROBE} 2>> ${ERRFIL}
