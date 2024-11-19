#!/bin/bash
# Copyright 2023-2024 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# I'm using an Ardusimple SimpleGNSS board with a
# u-blox NEO-F10T module. This is the first GNSS
# device I've used that implements the NMEA 0183 4.11
# specification. It is also the first generation 10
# u-blox device I've used. N.B. the labeling of the PVT
# (1PPS) and POWER LEDs are reversed on this version
# of the SimpleGNSS board. This script uses the device
# as it is.

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyUSB0"}
RATE=${2:-38400}
ONEPPS=${3:-"/dev/gpiochip4:18"} # pin 12 on Raspberry Pi 5
STROBE=${4:-"/dev/gpiochip4:16"} # pin 36 on Raspberry Pi 5

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}
mkdir -p ${SAVDIR}

ERRFIL="${SAVDIR}/${PROGRAM}.err"
mkdir -p $(dirname ${ERRFIL})
exec 2>>${ERRFIL}

OUTFIL="${SAVDIR}/${PROGRAM}.out"
mkdir -p $(dirname ${OUTFIL})

PIDFIL="${SAVDIR}/${PROGRAM}.pid"
mkdir -p $(dirname ${PIDFIL})

# LSTFIL="${SAVDIR}/${PROGRAM}.lst"
# mkdir -p $(dirname ${LSTFIL})

# DATFIL="${SAVDIR}/${PROGRAM}.dat"
# mkdir -p $(dirname ${DATFIL})

. $(readlink -e $(dirname ${0})/../bin)/setup

# UBX-MON-VER [0]

EUID=$(id -u)
if [[ ${EUID} == 0 ]]; then
    REALTIME="-r"
else
    REALTIME=""
fi

exec coreable gpstool \
	-D ${DEVICE} -b ${RATE} -8 -n -1 \
	-E -H ${OUTFIL} -a \
	-O ${PIDFIL} \
	-I ${ONEPPS} -p ${STROBE} ${REALTIME} \
	-t 10 -F 1 \
	-w 2 -x \
	-U '\xb5\x62\x0a\x04\x00\x00' \
	< /dev/null > /dev/null

# -L ${LSTFIL}
# -C ${DATFIL}
