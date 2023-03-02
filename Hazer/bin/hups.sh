#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Send a SIGHUP to all gpstool instances if any whenever
# user presses any key.

PID=${1:-$$}
SIG=${2:-"HUP"}
while read -p "${SIG}? " -N 1 INPUT; do
	kill -${SIG} ${PID}
done
echo
exit 0
