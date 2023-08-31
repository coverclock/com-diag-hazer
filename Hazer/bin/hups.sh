#!/bin/bash
# Copyright 2020-2023 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Send a SIGHUP to all gpstool instances if any whenever
# user presses any key.
# USAGE hups [ PROGRAM [ SIGNAL ] ]
# EXAMPLES
#    hups
#    hups gpstool
#    hups gpstool HUP

PGM=${1:-"gpstool"}
SIG=${2:-"HUP"}
while read -p "${SIG}? " -N 1 INPUT; do
    pkill -${SIG} ${PGM}
done
echo
exit 0
