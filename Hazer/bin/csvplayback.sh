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

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}

PROGRAM=$(basename ${0})
OUTPUT=${1}
INPUT=${2-"${SAVDIR}/vehicle.csv"}

. $(readlink -e $(dirname ${0})/../bin)/setup

csvmeter < ${INPUT} | csv2dgm -U ${OUTPUT} -j
