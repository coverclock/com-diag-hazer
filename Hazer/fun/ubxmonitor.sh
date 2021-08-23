#!/bin/bash
# Copyright 2021 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

PROGRAM=$(basename -s .sh ${0})
DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-115200}
INTERVAL=${3:-1}
WRITE=${4:-1}

. $(readlink -e $(dirname ${0})/../bin)/setup

# Note that reverse of most other scripts in this project,
# this one saves STDOUT to the log file and emits
# STDERR.

LOG=$(readlink -e $(dirname ${0})/..)/log
mkdir -p ${LOG}

# UBX-MON-BATCH
# UBX-MON-GNSS
# UBX-MON-HW2
# UBX-MON-HW
# UBX-MON-IO
# UBX-MON-MSGPP
# UBX-MON-PATCH
# UBX-MON-RXBUF
# UBX-MON-RXR
# UBX-MON-SMGR
# UBX-MON-SPT
# UBX-MON-TXBUF
# UBX-MON-VER

exec coreable gpstool \
	-D ${DEVICE} -b ${RATE} -8 -n -1 -m -i ${INTERVAL} \
	-w ${WRITE} \
	-U '\xb5\x62\x0a\x32\x00\x00' \
	-U '\xb5\x62\x0a\x28\x00\x00' \
	-U '\xb5\x62\x0a\x0b\x00\x00' \
	-U '\xb5\x62\x0a\x09\x00\x00' \
	-U '\xb5\x62\x0a\x02\x00\x00' \
	-U '\xb5\x62\x0a\x06\x00\x00' \
	-U '\xb5\x62\x0a\x27\x00\x00' \
	-U '\xb5\x62\x0a\x07\x00\x00' \
	-U '\xb5\x62\x0a\x21\x00\x00' \
	-U '\xb5\x62\x0a\x2e\x00\x00' \
	-U '\xb5\x62\x0a\x2f\x00\x00' \
	-U '\xb5\x62\x0a\x08\x00\x00' \
	-U '\xb5\x62\x0a\x04\x00\x00' \
	-R -v > ${LOG}/${PROGRAM}.out
