#!/bin/bash
# Copyright 2022-2023 Digital Aggregates Corporation, Arvada Colorado USA.
# https://github.com/coverclock/com-diag-hazer
# mailto:coverclock@diag.com
# Convenience script to monitor instance of gpstool running headless.
FILE=${1:-"rover"}
TYPE=${2:-"out"}
ROOT=${HOME}/src/com-diag-hazer/Hazer
cd ${ROOT}
EXEC=out/host/bin
. ${EXEC}/setup
${EXEC}/peruse ${FILE} ${TYPE}
