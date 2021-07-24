#!/bin/bash
# Copyright 2021 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# This is a basic functional sanity test for those options of
# gpstool that emit sentences (NMEA), packets (UBX), and strings
# (other) to the device.

PROGRAM=$(basename ${0})

. $(readlink -e $(dirname ${0})/../bin)/setup

FIFO=${TMPDIR:-"/tmp"}/${PROGRAM}.fifo
trap "rm -f ${FIFO}" SIGINT SIGQUIT SIGTERM EXIT
mkfifo -m 600 ${FIFO}

coreable gpstool \
	-D ${FIFO} \
	-Z 'LYNQ012ARHM\x0d' \
	-Z 'LYNQ012ARHM\x0d' \
	-W '$PUBX,00' \
	-U '\xb5b\x05\x01\x02\x00\x05\x01\x0e6' \
	-u -v
