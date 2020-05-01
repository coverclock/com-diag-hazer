#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Send a SIGHUP to all gpstool instances if any whenever
# user presses RETURN key.

SIG=${1:-"HUP"}
PGM=${2:-"gpstool"}
while read -p "${SIG}? "; do
	pkill -${SIG} ${PGM}
done
echo
exit 0
