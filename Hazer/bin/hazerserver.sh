#!/bin/bash
export PATH=${PATH}:/home/pi/src/com-diag-diminuto/Diminuto/out/host/bin/../sym:/home/pi/src/com-diag-diminuto/Diminuto/out/host/bin/../bin:/home/pi/src/com-diag-diminuto/Diminuto/out/host/bin/../tst:/home/pi/src/com-diag-hazer/Hazer/out/host/bin/../sym:/home/pi/src/com-diag-hazer/Hazer/out/host/bin/../bin:/home/pi/src/com-diag-hazer/Hazer/out/host/bin/../tst
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/home/pi/src/com-diag-diminuto/Diminuto/out/host/bin/../lib:/home/pi/src/com-diag-hazer/Hazer/out/host/bin/../lib
GPS=${1:-"/dev/ttyUSB0"}
GPSSPEED=${2:-"4800"}
TTY=${3:-"/dev/ttyS0"}
TTYSPEED=${4:-"115200"}
serialtool -D ${GPS} -b ${GPSSPEED} -8 -1 -n -l | gpstool | serialtool -D ${TTY} -b ${TTYSPEED} -8 -n -1 -l
