#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
PROGRAM=${1:-base}
ARGUMENT=${2:-localhost:tumbleweed}
FILE=${3:-${PROGRAM}}
TYPE=${4:-out}
${PROGRAM} ${ARGUMENT} & sleep 5; exec peruse ${FILE} ${TYPE}
