#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

[[ "$(csvfix2str 0)" == "NO" ]] || exit 1
[[ "$(csvfix2str 1)" == "IN" ]] || exit 1
[[ "$(csvfix2str 2)" == "2D" ]] || exit 1
[[ "$(csvfix2str 3)" == "3D" ]] || exit 1
[[ "$(csvfix2str 4)" == "GI" ]] || exit 1
[[ "$(csvfix2str 5)" == "TM" ]] || exit 1
[[ "$(csvfix2str 6)" == "OT" ]] || exit 1
[[ "$(csvfix2str  )" == "XX" ]] || exit 1

[[ "$(csvsys2str 0)" == "GN" ]] || exit 2
[[ "$(csvsys2str 1)" == "GP" ]] || exit 2
[[ "$(csvsys2str 2)" == "GL" ]] || exit 2
[[ "$(csvsys2str 3)" == "GA" ]] || exit 2
[[ "$(csvsys2str 4)" == "BD" ]] || exit 2
[[ "$(csvsys2str 5)" == "OT" ]] || exit 2
[[ "$(csvsys2str  )" == "XX" ]] || exit 2

BEARING=0
while [[ ${BEARING} -le 360 ]]; do
	COMPASSE=$(compasstool -e ${BEARING})
	COMPASSS=$(compasstool -s ${BEARING})
	COMPASST=$(compasstool -t ${BEARING})
	echo "COMPASS ${BEARING} ${COMPASSE} ${COMPASSS} ${COMPASST}"
	BEARING=$((${BEARING} + 1))
done

exit 0
