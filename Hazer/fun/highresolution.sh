#!/bin/bash
# Copyright 2019 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Configure and run the U-blox UBX-ZED-F9P as a stationary Base sending
# corrections to Rovers with parameters for high resolution.

ROUTER=${1:-"tumbleweed:tumbleweed"}
DEVICE=${2:-"/dev/tumbleweed"}
RATE=${3:-230400}
DURATION=${4:-600}
ACCURACY=${5:-100}

exec $(readlink -e $(dirname ${0})/../bin)/base ${ROUTER} ${DEVICE} ${RATE} ${DURATION} ${ACCURACY}
