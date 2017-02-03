#!/bin/bash
stty sane
clear
serialtool -D /dev/ttyUSB0 -b 4800 -8 -1 -n -l | gpstool  -e
