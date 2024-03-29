#!/bin/bash
# Copyright 2023 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Uses the socat utility to pipe date directly into dump with no
# interpretation or processing.
# serialtool -D /dev/ttyACM0 -b 9600 -8 -n -1 -i < /dev/null | dump
# also works.

DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-9600}

. $(readlink -e $(dirname ${0})/../bin)/setup

exec socat -u OPEN:${DEVICE},b${RATE} - | dump
