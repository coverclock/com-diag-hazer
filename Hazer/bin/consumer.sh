#!/bin/bash
# Copyright 2018 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

# 1. Consume NMEA datagrams from the specified IPv6 port.
# 2. Report on standard output.

# usage: consumer [ PORT ]

. $(readlink -e $(dirname ${0})/../bin)/setup

PORT=${1:-"29470"}

stty sane
clear

exec coreable gpstool -G :${PORT} -E
