#!/bin/bash
# Copyright 2018 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

# Uses the Diminuto pintool to select(2) on the specified GPIO pin (18 on my
# lab fixture) to read the GPS device (MakerFocus USB-Port-GPS xQuectel L80-R
# on my lab fixture) PPS signal. This is just for testing. In production, the
# GPS NMEA stream and PPS signal is fed into gpsd which in turn feeds into
# ntpd.

# usage: pps [ PIN ]

. $(readlink -e $(dirname ${0})/../bin)/setup

PIN=${1:-"18"}

pintool -p ${PIN} -n 2> /dev/null || true
pintool -p ${PIN} -x -i -H -R
pintool -p ${PIN} -M
