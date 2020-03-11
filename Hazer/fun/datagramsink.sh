#!/bin/bash
# Copyright 2019 Digital Aggregates Corporation, Colorado, USA<BR>
# Licensed under the terms in LICENSE.txt<BR>
# Chip Overclock (coverclock@diag.com)<BR>
# https://github.com/coverclock/com-diag-hazer
#
# This script tests the sink or receive side of the Tumbleweed UDP channel
# by receiving packets on the RTCM port, displaying them on standard error,
# and then echoing them back to the sender.

ROOT=$(dirname ${0})
PROGRAM=$(basename ${0})
PORT=${1:-"21010"}

. $(readlink -e ${ROOT}/../bin)/setup

exec socat -u UDP4-RECV:${PORT} - | phex
