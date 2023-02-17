#!/bin/bash
# Copyright 2023 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# In support of testing the Stagecoach program in the Codex project.
# Codex is (or will be, if and when I get it working) an SSL tunnel
# and client and server proxy for Hazer applications. It is hosted
# in the Codex project because it is built on top of Codex, an SSL
# library.

PROGRAM=$(basename ${0})
INPUT=${1}
PORT=${2:-24040}
HOST=${3:-"localhost"}

. $(readlink -e $(dirname ${0})/../bin)/setup

csvmeter < ${INPUT} | tee /dev/tty | csv2dgm -U ${HOST}:${PORT} -c
