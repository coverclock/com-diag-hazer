#!/bin/bash
# Copyright 2023 Digital Aggregates Corporation, Colorado, USA<BR>
# Licensed under the terms in LICENSE.txt<BR>
# Chip Overclock (coverclock@diag.com)<BR>
# https://github.com/coverclock/com-diag-hazer
#
# This script is like datagramsink except it just prints
# the presumable printable datagram on stdout.

ROOT=$(dirname ${0})
PROGRAM=$(basename ${0})
PORT=${1:-"tesoro"}

. $(readlink -e ${ROOT}/../bin)/setup

exec socat -u UDP4-RECV:${PORT} -
