#!/bin/bash
# Copyright 2019 Digital Aggregates Corporation, Colorado, USA<BR>
# Licensed under the terms in README.h<BR>
# Chip Overclock (coverclock@diag.com)<BR>
# https://github.com/coverclock/com-diag-hazer
#
# This is a script that connects together two Ardusimple SimpleRTK2B
# boards using Digi XBee3 LTE-M cellular modems so that one of them, the
# Base - a stationary GNSS (Global Navigation Satellite System) receiver in
# Survey Mode - can send the other, the Rover - a mobile GNSS receiver -
# DGNSS (Differential GNSS) corrections derived from RTK (Real-Time Kinematic)
# measurements, in the form of RTCM (Radio Technical Commission for Maritime
# services) 3.3 messsages. This is simpler than it sounds. The script uses the
# socat utility to receive UDP (User Datagram Protocol) packets from the Base
# on a port - typically 2101 a.k.a. service "rtcm-sc104" (Special Committee 104)
# - and forwards them to the same port to the Rover. The forwarding is
# unidirectional (the Rover should never have anything to say to the Base).
#

ROOT=$(dirname ${0})
PROGRAM=$(basename ${0})
BASE=${1}
ROVER=${2}
PORT=${3:-2101}

. $(readlink -e ${ROOT}/../bin)/setup

exec socat -u UDP4-RECV:${PORT},range=${BASE}/32 UDP4-SENDTO:${ROVER}:${PORT} 2> >(log -S -N ${PROGRAM})
