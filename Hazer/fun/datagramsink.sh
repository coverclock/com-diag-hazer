#!/bin/bash
# Copyright 2019-2023 Digital Aggregates Corporation, Colorado, USA<BR>
# Licensed under the terms in LICENSE.txt<BR>
# Chip Overclock (coverclock@diag.com)<BR>
# https://github.com/coverclock/com-diag-hazer
#
# This script receives datagrams and displays them on
# standard output using phex, which converts unprintable
# characters (including newlines) into C-style printable
# escape sequences.

ROOT=$(dirname ${0})
PROGRAM=$(basename ${0})
PORT=${1:-"tumbleweed"}

. $(readlink -e ${ROOT}/../bin)/setup

exec socat -u UDP4-RECV:${PORT} - | phex
