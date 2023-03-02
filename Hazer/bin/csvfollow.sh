#!/bin/bash
# Copyright 2021 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Follows a CSV file as it is updated, converts
# it line by line into Tesoro JSON, and sends it
# to the specified endpoint.
# usage: csvfollow ENDPOINT CSVFILE
# example: csvfollow channelhost:tesoro vehicle.csv

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}

PROGRAM=$(basename ${0})
OUTPUT=${1}
INPUT=${2-"${SAVDIR}/vehicle.csv"}

. $(readlink -e $(dirname ${0})/../bin)/setup

tail -n 0 -f ${INPUT} | csv2dgm -U ${OUTPUT} -j
