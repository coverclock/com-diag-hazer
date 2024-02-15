#!/bin/bash
# Copyright 2024 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

DEVICE=${1:-"/dev/ttyUSB0"}
# The baud rate is fixed on the WitMotion WT901BLECL5.0.

. $(readlink -e $(dirname ${0})/../bin)/setup

wt901setup | serialtool -D ${DEVICE} -b 115200 -8 -1 -n -T -P | wt901tool -E
