#!/bin/bash
# Copyright 2021 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Shutdown a Digi XBEE3 LTE-M radio module on a Digi XBIB-CU-TH board.
# This is strongly recommended prior to powering it off.
# usage: wheatstoneshutdown [ NETDEVICE [ NETRATE ] ]
# example: wheatstoneshutdown /dev/ttyUSB0

PROGRAM=$(basename ${0})
NETDEVICE=${2:-"/dev/ttyUSB0"}
NETRATE=${5:-9600}

echo ${PROGRAM}: DEVICE ${NETDEVICE} ${NETRATE} 1>&2

#coproc serialtool -D ${NETDEVICE} -b ${NETRATE} -8 -1 -n -r
#echo ${PROGRAM}: SERIALTOOL ${COPROC[0]} ${COPROC[1]} 1>&2
#exec 0<&${COPROC[0]}-
#exec 1>&${COPROC[1]}-

stty -F ${NETDEVICE} ${NETRATE} cs8 -cstopb cread clocal

echo ${PROGRAM}: PLUSPLUSPLUS 1>&2
echo -n -e '+++' > ${NETDEVICE}
read -s -n 2 REPLY < ${NETDEVICE}
echo ${PROGRAM}: PLUSPLUSPLUS \"${REPLY}\" 1>&2
echo ${PROGRAM}: SHUTDOWN 1>&2
echo -n -e 'ATSD0\r' > ${NETDEVICE}
read -s -n 2 REPLY < ${NETDEVICE}
echo ${PROGRAM}: SHUTDOWN \"${REPLY}\" 1>&2
