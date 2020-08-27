#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Converts a fix type to a string.

FIX=${1:-""}
TYPE="XX"

if [[ -n "${FIX}" ]]; then
	case "${FIX%,}" in
	0) TYPE="NO";;
	1) TYPE="IN";;
	2) TYPE="2D";;
	3) TYPE="3D";;
	4) TYPE="GI";;
	5) TYPE="TM";;
	*) TYPE="OT";;
	esac
fi

echo ${TYPE}

exit 0
