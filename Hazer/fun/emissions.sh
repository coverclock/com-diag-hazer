#!/bin/bash
# Copyright 2021-2025 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# This is a basic functional sanity test for those options of
# gpstool that emit sentences (NMEA), packets (UBX), and strings
# (other) to the device.

PROGRAM=$(basename ${0})

. $(readlink -e $(dirname ${0})/../bin)/setup

export LC_ALL=en_US.UTF-8

FIFO=${TMPDIR:-"/tmp"}/${PROGRAM}.fifo
trap "rm -f ${FIFO}" SIGINT SIGQUIT SIGTERM EXIT
mkfifo -m 600 ${FIFO}

coreable gpstool \
	-D ${FIFO} \
	-Z 'VERBATIM\r' \
	-Z 'VERBATIM\r' \
	-W '$PUBX,00' \
	-W '$PUBX,01' \
	-U '\xb5\x62\x05\x01\x02\x00\x05\x01' \
	-U '\xb5\x62\x05\x00\x02\x00\x05\x00' \
	-A '\xb5\x62\x05\x01\x02\x00\x05\x01' \
	-A '\xb5\x62\x05\x00\x02\x00\x05\x00' \
	-A '' \
	-v -R
