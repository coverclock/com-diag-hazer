#!/bin/bash
# Copyright 2021 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Follows a CSV file as it is updated, converts
# it line by line into Tesoro JSON, and sends it
# to the specified endpoint.
# usage: csvfollow CSVFILE ENDPOINT
# example: csvfollow vehicle.csv channelhost:tesoro

PROGRAM=$(basename ${0})
INPUT=${1}
OUTPUT=${2}

. $(readlink -e $(dirname ${0})/../bin)/setup

tail -f ${INPUT} | csv2dgm -U ${OUTPUT} -j
