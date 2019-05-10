#!/bin/bash
# Copyright 2019 Digital Aggregates Corporation, Colorado, USA<BR>
# Licensed under the terms in README.h<BR>
# Chip Overclock (coverclock@diag.com)<BR>
# https://github.com/coverclock/com-diag-hazer
#
# This script tests the sink or receive side of the Tumbleweed UDP channel
# by receiving packets on the RTCM port and then using the Diminuto phex
# utility to display the received binary data on standard output in a
# printable form.

ROOT=$(dirname ${0})
PROGRAM=$(basename ${0})
PORT=${1:-2101}

. $(readlink -e ${ROOT}/../bin)/setup

exec socat -u UDP4-RECV:${PORT} - | phex
