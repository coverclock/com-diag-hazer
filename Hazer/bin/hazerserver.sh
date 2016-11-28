#!/bin/bash
export PATH=${PATH}:/home/pi/src/com-diag-diminuto/Diminuto/out/host/bin/../sym:/home/pi/src/com-diag-diminuto/Diminuto/out/host/bin/../bin:/home/pi/src/com-diag-diminuto/Diminuto/out/host/bin/../tst:/home/pi/src/com-diag-hazer/Hazer/out/host/bin/../sym:/home/pi/src/com-diag-hazer/Hazer/out/host/bin/../bin:/home/pi/src/com-diag-hazer/Hazer/out/host/bin/../tst
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/home/pi/src/com-diag-diminuto/Diminuto/out/host/bin/../lib:/home/pi/src/com-diag-hazer/Hazer/out/host/bin/../lib
GPS=${1:-"/dev/ttyUSB0"}
GPSSPEED=${2:-"4800"}
HOST="[::1]"
PORT=5555
serialtool -D ${GPS} -b ${GPSSPEED} -8 -1 -n -l | gpstool | grep '^MAP '
#serialtool -D ${GPS} -b ${GPSSPEED} -8 -1 -n -l | gpstool | grep '^MAP ' | socat -u STDIN STDOUT
#serialtool -D ${GPS} -b ${GPSSPEED} -8 -1 -n -l | gpstool | grep '^MAP ' | socat -x -u STDIN UDP6-DATAGRAM:${HOST}:${PORT}
