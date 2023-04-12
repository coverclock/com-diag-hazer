#!/bin/bash
# Copyright 2023 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# In support of testing the Fothergill project, in which gpstool
# forwards CSV-based traces over LoRa radios using HDLC-like
# framing.

PROGRAM=$(basename ${0})
RADDEV=${1:-"/dev/ttyACM0"}
RADBPS=${2:-57600}

while [ -c ${RADDEV} ]; do
	framertool -D ${RADDEV} -b ${RADBPS} -8 -n -1 -d
	RADXIT=$?
	log -I -N ${PROGRAM} -n "framertool exited! (${RADXIT})"
done
