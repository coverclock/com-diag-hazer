#!/bin/bash

export PATH=${PATH}:/home/pi/src/com-diag-diminuto/Diminuto/out/host/bin/../sym:/home/pi/src/com-diag-diminuto/Diminuto/out/host/bin/../bin:/home/pi/src/com-diag-diminuto/Diminuto/out/host/bin/../tst:/home/pi/src/com-diag-hazer/Hazer/out/host/bin/../sym:/home/pi/src/com-diag-hazer/Hazer/out/host/bin/../bin:/home/pi/src/com-diag-hazer/Hazer/out/host/bin/../tst
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/home/pi/src/com-diag-diminuto/Diminuto/out/host/bin/../lib:/home/pi/src/com-diag-hazer/Hazer/out/host/bin/../lib

HOST=${1:-"[::1]"}
PORT=${2:-"5555"}
DEVICE=${3:-"/dev/ttyUSB0"}
SPEED=${4:-"4800"}

serialtool -D ${DEVICE} -b ${SPEED} -8 -1 -n -l -i | gpstool | grep '^MAP ' | socat -u - TCP6:${HOST}:${PORT}
