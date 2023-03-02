#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Converts a system number to a string.
# usage: SYSTEMSTRING=$(csvsys2str SYSTEMNUMBER)

SYS=${1:-""}
SYSTEM="XX"

if [[ -n "${SYS}" ]]; then
	case "${SYS%,}" in
	0) SYSTEM="GN";;
	1) SYSTEM="GP";;
	2) SYSTEM="GL";;
	3) SYSTEM="GA";;
	4) SYSTEM="BD";;
	*) SYSTEM="OT";;
	esac
fi

echo ${SYSTEM}

exit 0
