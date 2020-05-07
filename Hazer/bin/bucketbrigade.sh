#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Use Diminuto's serialtool to read the output from a GNSS device on
# one serial port, then direct that output to another serial device,
# perhaps to be consumed by some other application or device.

SOURCE=${1:-"/dev/tumbleweed"}
SOURCING=${2:-230400}
SINK=${3:-"/dev/ttyUSB0"}
SINKING=${4:-4800}

. $(readlink -e $(dirname ${0})/../bin)/setup

serialtool -D ${SOURCE} -b ${SOURCING} -8 -n -1 -l | serialtool -D ${SINK} -b ${SINKING} -8 -n -1 -m

exit 0
