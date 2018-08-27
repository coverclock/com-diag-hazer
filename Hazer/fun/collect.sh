#!/bin/bash
# Copyright 2018 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

# This script collects the raw output of a device into a file.
# I typically run this for about a minute to see what the device
# has to say. If it's a device I expect to use in the future, I
# save this data file in source control. I can use this file to
# test changes to Hazer and gpstool just by redirecting gpstool
# to read from this data file as if it were reading from the
# device.

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-9600}

. $(readlink -e $(dirname ${0})/../bin)/setup

DIR=$(readlink -e $(dirname ${0})/..)/dat
mkdir -p ${DIR}

exec gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -L ${DIR}/${PROGRAM}.dat -v
