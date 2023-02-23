#!/bin/bash
# Copyright 2023 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# In support of testing the Stagecoach program in the Codex project.

PROGRAM=$(basename ${0})
ENDPOINT=${1:-"localhost:tumbleweed"}
SECONDS=${2:-"1"}

. $(readlink -e $(dirname ${0})/../bin)/setup

rtk2dgm -Y ${ENDPOINT} -y ${SECONDS}
