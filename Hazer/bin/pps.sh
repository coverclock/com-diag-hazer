#!/bin/bash

# Uses the Diminuto pintool to select(2) on the specified GPIO pin (24 on my
# lab fixture) to read the GPS device (MakerFocus USB-Port-GPS xQuectel L80-R
# on my lab fixture) PPS signal. This is just for testing. In production, the
# GPS NMEA stream and PPS signal is fed into gpsd which in turn feeds into
# ntpd.

. $(readlink -e $(dirname ${0})/../bin)/setup

PIN=${1:-"24"}

pintool -p ${PIN} -n 2> /dev/null || true
pintool -p ${PIN} -x -i -H -R
pintool -p ${PIN} -M
