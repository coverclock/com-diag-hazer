#!/bin/bash
# Copyright 2021 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Like the bu353w10 script but uses one gpstool to read from the device
# and forward to a second gpstool over a datagram socket. Must have the
# service "hazer" defined in /etc/services.

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-9600}
HOST=${3:-"localhost"}
PORT=${4:-"hazer"}

. $(readlink -e $(dirname ${0})/../bin)/setup

. $(readlink -e $(dirname ${0})/../fun)/ubx8

LOG=$(readlink -e $(dirname ${0})/..)/log
mkdir -p ${LOG}

OPTIONS=""
for OPTION in ${COMMANDS}; do
    OPTIONS="${OPTIONS} -W ${OPTION}"
done

coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 ${OPTIONS} -G ${HOST}:${PORT} 2> ${LOG}/${PROGRAM}-producer.err 1> /dev/null &

coreable gpstool -G ${PORT} -E 2> ${LOG}/${PROGRAM}-consumer.err
