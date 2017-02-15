#!/bin/bash

# 1. Consume NMEA datagrams from the specified IPv6 port.
# 2. Report on standard output.

. $(readlink -e $(dirname ${0})/../bin)/setup

PORT=${1:-"5555"}

stty sane
clear

exec gpstool -6 -P ${PORT} -E
