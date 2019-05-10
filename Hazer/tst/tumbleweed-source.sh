#!/bin/bash
# Copyright 2019 Digital Aggregates Corporation, Colorado, USA<BR>
# Licensed under the terms in README.h<BR>
# Chip Overclock (coverclock@diag.com)<BR>
# https://github.com/coverclock/com-diag-hazer
#
# This script tests the source or send side of the Tumbleweed UDP channel by
# taking input from standard input, packaging it up in a datagram, and sending
# it to the specified host and RTCM port.

ROOT=$(dirname ${0})
PROGRAM=$(basename ${0})
HOST=${1-"diag.ddns.net"}
PORT=${2:-2101}

. $(readlink -e ${ROOT}/../bin)/setup

exec socat -u - UDP4-DATAGRAM:${HOST}:${PORT}
