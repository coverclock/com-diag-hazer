#!/bin/bash
# Copyright 2018 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

# This script is identical to bu353w10 except that full screen
# refreshes are limited to one Hertz (1Hz) by using the -F option
# instead of the -E option. This works better with slow terminals
# like the Raspberry Pi console which, even at 115200 baud, cannot
# match an xterm at 100Mbps Ethernet speeds.

# If you are using a Raspberry Pi console, you might find the command
# "stty rows N" useful to change the number of lines on your remote
# terminal from 24 to N. If you're using the vim editor, you might need
# to run vim again just to get it to take on your remote terminal (that
# was my experience using "screen" from my Mac laptop).

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-9600}

. $(readlink -e $(dirname ${0})/../bin)/setup

DIR=$(readlink -e $(dirname ${0})/..)/log
mkdir -p ${DIR}

eval coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -F -t 10 -W '\$PUBX,40,GSV,0,0,0,1,0,0' -W '\$PUBX,40,VTG,0,0,0,1,0,0' 2>> ${DIR}/${PROGRAM}.log