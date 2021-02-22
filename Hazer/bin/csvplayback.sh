#!/bin/bash
# Copyright 2021 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Plays back a CSV file at approximately the same
# rate it was generated, converts it into Tesoro
# JSON, and sends it to the specified endpoint.
# usage: csvplayback CSVFILE ENDPOINT
# example: csvplayback vehicle.csv channelhost:tesoro

PROGRAM=$(basename ${0})
INPUT=${1}
OUTPUT=${2}

. $(readlink -e $(dirname ${0})/../bin)/setup

csvmeter < ${INPUT} | csv2dgm -U ${OUTPUT} -j
