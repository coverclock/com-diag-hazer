#!/bin/bash
stty sane
clear
DEVICE=${1:-"/dev/ttyUSB0"}
SPEED=${2:-"4800"}
serialtool -D ${DEVICE} -b ${SPEED} -8 -1 -n -l | gpstool  -e
