#!/bin/bash
# Copyright 2019 Digital Aggregates Corporation, Colorado, USA<BR>
# Licensed under the terms in LICENSE.txt<BR>
# Chip Overclock (coverclock@diag.com)<BR>
# https://github.com/coverclock/com-diag-hazer
#
# This script tests the source or send side of the Tumbleweed UDP channel by
# taking input from standard input, packaging it up in a datagram, then sending
# it to the specified host and RTCM port, and also receiving datagrams on the
# same port and displaying them on standard error.

ROOT=$(dirname ${0})
PROGRAM=$(basename ${0})
HOST=${1-"localhost"}
PORT=${2:-"tumbleweed"}

. $(readlink -e ${ROOT}/../bin)/setup

exec socat -u - UDP4-DATAGRAM:${HOST}:${PORT}
