#!/bin/bash
stty sane
clear
DEVICE=${1:-"/dev/ttyUSB0"}
SPEED=${2:-"4800"}
gpstool -D ${DEVICE} -b ${SPEED} -8 -n -1 -E
