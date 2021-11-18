#!/bin/bash
# Copyright 2021 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Test pitop4 in headless/wireless/battery mode with a bu353w10 GNSS dongle
# and an LTE modem.

SELF=$$

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}

# On the pi-top 4, try something like
#    pitop4 127.0.0.1:12345
# where 127.0.0.1 is the endpoint host name from /etc/hosts or IP address,
# and 12345 is the endpoint service name from /etc/services or port number.

PROGRAM=$(basename ${0})
ENDPOINT=${1:-"127.0.0.1:12345"}
DEVICE=${2:-"/dev/ttyACM0"}
RATE=${3:-9600}
ERRFIL=${4:-"${SAVDIR}/${PROGRAM}.err"}
OUTFIL=${5:-"${SAVDIR}/${PROGRAM}.out"}
PIDFIL=${6:-"${SAVDIR}/${PROGRAM}.pid"}
CSVFIO=${7:-"${SAVDIR}/${PROGRAM}.fio"}

mkdir -p $(dirname ${ERRFIL})
mkdir -p $(dirname ${OUTFIL})
mkdir -p $(dirname ${PIDFIL})

cp /dev/null ${OUTFIL}

cp /dev/null ${ERRFIL}
exec 2>>${ERRFIL}

# If you can login into the pitop4, try
#    peruse pitop4 out
# to display stdout, or
#    peruse pitop4 err
# to display stderr.

rm -f ${CSVFIO}
mkfifo ${CSVFIO}

trap "trap '' SIGINT SIGQUIT SIGTERM; kill -TERM -- -${SELF} 2> /dev/null; exit 0" SIGINT SIGQUIT SIGTERM

. $(readlink -e $(dirname ${0})/../bin)/setup

csv2dgm -U ${ENDPOINT} -j < ${CSVFIO} 1> /dev/null &

# On the endpoint host, try
#    socat -u UDP4-RECV:12345 -
# where 12345 is the endpoint port number.

exec coreable gpstool -D ${DEVICE} -b ${RATE} -8 -n -1 -H ${OUTFIL} -t 10 -T ${CSVFIO} -O ${PIDFIL} < /dev/null 1> /dev/null
