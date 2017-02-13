#!/bin/bash

export PATH=${PATH}:${HOME}/src/com-diag-diminuto/Diminuto/out/host/bin/../sym:${HOME}/src/com-diag-diminuto/Diminuto/out/host/bin/../bin:${HOME}/src/com-diag-diminuto/Diminuto/out/host/bin/../tst:${HOME}/src/com-diag-hazer/Hazer/out/host/bin/../sym:${HOME}/src/com-diag-hazer/Hazer/out/host/bin/../bin:${HOME}/src/com-diag-hazer/Hazer/out/host/bin/../tst
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${HOME}/src/com-diag-diminuto/Diminuto/out/host/bin/../lib:${HOME}/src/com-diag-hazer/Hazer/out/host/bin/../lib

HOST=${1:-"consumer"}
PORT=${2:-"5555"}
DEVICE=${3:-"/dev/ttyUSB0"}
SPEED=${4:-"4800"}

exec gpstool -D ${DEVICE} -b ${SPEED} -8 -n -1 -E -6 -A ${HOST} -P ${PORT}
