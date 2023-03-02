#!/bin/bash
# Copyright 2019 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

# The comparison of float values is extremely problematic.
# The best I can say is that it passes on the specific
# target on which I built and ran the unit test.

# The mapstool utility and its unit test depends upon the
# appropriate interpretation of the Unicode encoding for
# the degree symbol This depends on the specific settings
# of the environmental variables that affect locale and
# internationalization. The defaults used in, for example,
# Ubuntu 19.10, worked just fine for me, but other values
# did not.

PROGRAM=$(basename ${0})

. $(readlink -e $(dirname ${0})/../bin)/setup

XC=0

ACTUAL=$(mapstool -d "39.794212196, -105.153349930")
EXPECTED="39.794212196, -105.153349930"
echo ${PROGRAM}: ACTUAL="\"${ACTUAL}\"" EXPECTED="\"${EXPECTED}\"" 1>&2
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo ${PROGRAM}: FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

ACTUAL=$(mapstool -d "39 47 39.16390(N) 105 09 12.05974(W)")
EXPECTED="39.794212194, -105.153349928"
echo ${PROGRAM}: ACTUAL="\"${ACTUAL}\"" EXPECTED="\"${EXPECTED}\"" 1>&2
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo ${PROGRAM}: FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

ACTUAL=$(mapstool -d "39°47'39.163\"N, 105°09'12.060\"W")
EXPECTED="39.794211944, -105.153350000"
echo ${PROGRAM}: ACTUAL="\"${ACTUAL}\"" EXPECTED="\"${EXPECTED}\"" 1>&2
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo ${PROGRAM}: FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

ACTUAL=$(mapstool -d -D "39.794212196, -105.153349930")
EXPECTED="39.794212196, -105.153349930"
echo ${PROGRAM}: ACTUAL="\"${ACTUAL}\"" EXPECTED="\"${EXPECTED}\"" 1>&2
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo ${PROGRAM}: FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

ACTUAL=$(mapstool -d -D "39 47 39.16390(N) 105 09 12.05974(W)")
EXPECTED="39.794212194, -105.153349928"
echo ${PROGRAM}: ACTUAL="\"${ACTUAL}\"" EXPECTED="\"${EXPECTED}\"" 1>&2
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo ${PROGRAM}: FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

ACTUAL=$(mapstool -d -D "39°47'39.163\"N, 105°09'12.060\"W")
EXPECTED="39.794211944, -105.153350000"
echo ${PROGRAM}: ACTUAL="\"${ACTUAL}\"" EXPECTED="\"${EXPECTED}\"" 1>&2
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo ${PROGRAM}: FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

ACTUAL=$(mapstool -d -P "39.794212196, -105.153349930")
EXPECTED="39°47'39.163905\"N, 105°09'12.059748\"W"
echo ${PROGRAM}: ACTUAL="\"${ACTUAL}\"" EXPECTED="\"${EXPECTED}\"" 1>&2
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo ${PROGRAM}: FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

ACTUAL=$(mapstool -d -P "39 47 39.16390(N) 105 09 12.05974(W)")
EXPECTED="39°47'39.163899\"N, 105°09'12.059740\"W"
echo ${PROGRAM}: ACTUAL="\"${ACTUAL}\"" EXPECTED="\"${EXPECTED}\"" 1>&2
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo ${PROGRAM}: FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

ACTUAL=$(mapstool -d -P "39°47'39.163\"N, 105°09'12.060\"W")
EXPECTED="39°47'39.162999\"N, 105°09'12.060000\"W"
echo ${PROGRAM}: ACTUAL="\"${ACTUAL}\"" EXPECTED="\"${EXPECTED}\"" 1>&2
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo ${PROGRAM}: FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

ACTUAL=$(mapstool -d "HPP   39.794212196, -105.153349930")
EXPECTED="39.794212196, -105.153349930"
echo ${PROGRAM}: ACTUAL="\"${ACTUAL}\"" EXPECTED="\"${EXPECTED}\"" 1>&2
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo ${PROGRAM}: FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

ACTUAL=$(mapstool -d "NGS  39 47 39.16390(N) 105 09 12.05974(W)")
EXPECTED="39.794212194, -105.153349928"
echo ${PROGRAM}: ACTUAL="\"${ACTUAL}\"" EXPECTED="\"${EXPECTED}\"" 1>&2
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo ${PROGRAM}: FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

ACTUAL=$(mapstool -d "POS 39°47'39.163\"N, 105°09'12.060\"W")
EXPECTED="39.794211944, -105.153350000"
echo ${PROGRAM}: ACTUAL="\"${ACTUAL}\"" EXPECTED="\"${EXPECTED}\"" 1>&2
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo ${PROGRAM}: FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

ACTUAL=$(mapstool -d -D "HPP   39.794212196, -105.153349930")
EXPECTED="39.794212196, -105.153349930"
echo ${PROGRAM}: ACTUAL="\"${ACTUAL}\"" EXPECTED="\"${EXPECTED}\"" 1>&2
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo ${PROGRAM}: FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

ACTUAL=$(mapstool -d -D "NGS  39 47 39.16390(N) 105 09 12.05974(W)")
EXPECTED="39.794212194, -105.153349928"
echo ${PROGRAM}: ACTUAL="\"${ACTUAL}\"" EXPECTED="\"${EXPECTED}\"" 1>&2
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo ${PROGRAM}: FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

ACTUAL=$(mapstool -d -D "POS 39°47'39.163\"N, 105°09'12.060\"W")
EXPECTED="39.794211944, -105.153350000"
echo ${PROGRAM}: ACTUAL="\"${ACTUAL}\"" EXPECTED="\"${EXPECTED}\"" 1>&2
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo ${PROGRAM}: FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

ACTUAL=$(mapstool -d -P "HPP    39.794212196, -105.153349930")
EXPECTED="39°47'39.163905\"N, 105°09'12.059748\"W"
echo ${PROGRAM}: ACTUAL="\"${ACTUAL}\"" EXPECTED="\"${EXPECTED}\"" 1>&2
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo ${PROGRAM}: FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

ACTUAL=$(mapstool -d -P "NGS  39 47 39.16390(N) 105 09 12.05974(W)")
EXPECTED="39°47'39.163899\"N, 105°09'12.059740\"W"
echo ${PROGRAM}: ACTUAL="\"${ACTUAL}\"" EXPECTED="\"${EXPECTED}\"" 1>&2
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo ${PROGRAM}: FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

ACTUAL=$(mapstool -d -P "POS 39°47'39.163\"N, 105°09'12.060\"W")
EXPECTED="39°47'39.162999\"N, 105°09'12.060000\"W"
echo ${PROGRAM}: ACTUAL="\"${ACTUAL}\"" EXPECTED="\"${EXPECTED}\"" 1>&2
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo ${PROGRAM}: FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

exit ${XC}
