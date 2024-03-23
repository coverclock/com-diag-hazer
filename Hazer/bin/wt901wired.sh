#!/bin/bash
# Copyright 2024 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Emit messages on standard output to set up the WitMotion WT901 IMU.
#
# EXAMPLE
#
# wt901wired | serialtool -D /dev/ttyUSB0 -T -b 115200 -8 -1 -n -P | dump
#

trap "trap '' SIGINT SIGPIPE SIGQUIT SIGTERM; echo $(basename $0): stop. 1>&2; kill -TERM -- -${SELF} 2> /dev/null; exit 0" SIGINT SIGPIPE SIGQUIT SIGTERM

echo "$(basename $0): start." 1>&2
sleep 1
#collapse '\xFF\xAA\x03\x03\x00' # Set Frequency 1Hz
collapse '\xFF\xAA\x03\x06\x00' # Set Frequency 10Hz (default)
sleep 1
collapse '\xFF\xAA\x27\x30\x00' # Get Year, Month
sleep 1
collapse '\xFF\xAA\x27\x31\x00' # Get Date, Hour
sleep 1
collapse '\xFF\xAA\x27\x32\x00' # Get Minute, Second
sleep 1
collapse '\xFF\xAA\x27\x33\x00' # Get Millisecond
sleep 1
while true; do
    collapse '\xFF\xAA\x27\x3A\x00' # Get Magnetic Field
    sleep 1
    collapse '\xFF\xAA\x27\x51\x00' # Get Quaternion
    sleep 1
    collapse '\xFF\xAA\x27\x40\x00' # Get Temperature
    sleep 1
done

exit 2
