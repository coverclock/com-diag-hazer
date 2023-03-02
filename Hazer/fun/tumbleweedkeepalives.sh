#!/bin/bash
# Copyright 2019 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

# This script is specific to the Ardusimple SimpleRTK2B.

PROGRAM=$(basename ${0})
PORT=${1:-"tumbleweed"}

. $(readlink -e $(dirname ${0})/../bin)/setup

exec coreable gpstool -Y ${PORT} -v 1> /dev/null
