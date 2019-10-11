#!/bin/bash
# Copyright 2019 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

# The comparison of float values is extremely problematic.
# The best I can say is that it passes on the specific
# target on which I built and ran the unit test.

PROGRAM=$(basename ${0})

. $(readlink -e $(dirname ${0})/../bin)/setup

XC=0

ACTUAL=$(googlemaps "39.794212196, -105.153349930")
EXPECTED="39.794212196, -105.153349930"
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

ACTUAL=$(googlemaps "39 47 39.16390(N) 105 09 12.05974(W)")
EXPECTED="39.794212194, -105.153349928"
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

ACTUAL=$(googlemaps "39°47'39.163\"N, 105°09'12.060\"W")
EXPECTED="39.794211944, -105.153350000"
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

ACTUAL=$(googlemaps -D "39.794212196, -105.153349930")
EXPECTED="39.794212196, -105.153349930"
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

ACTUAL=$(googlemaps -D "39 47 39.16390(N) 105 09 12.05974(W)")
EXPECTED="39.794212194, -105.153349928"
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

ACTUAL=$(googlemaps -D "39°47'39.163\"N, 105°09'12.060\"W")
EXPECTED="39.794211944, -105.153350000"
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

ACTUAL=$(googlemaps -P "39.794212196, -105.153349930")
EXPECTED="39°47'39.163905\"N, 105°09'12.059748\"W"
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

ACTUAL=$(googlemaps -P "39 47 39.16390(N) 105 09 12.05974(W)")
EXPECTED="39°47'39.163899\"N, 105°09'12.059740\"W"
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

ACTUAL=$(googlemaps -P "39°47'39.163\"N, 105°09'12.060\"W")
EXPECTED="39°47'39.162999\"N, 105°09'12.060000\"W"
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

ACTUAL=$(googlemaps "HPP   39.794212196, -105.153349930")
EXPECTED="39.794212196, -105.153349930"
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

ACTUAL=$(googlemaps "NGS  39 47 39.16390(N) 105 09 12.05974(W)")
EXPECTED="39.794212194, -105.153349928"
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

ACTUAL=$(googlemaps "POS 39°47'39.163\"N, 105°09'12.060\"W")
EXPECTED="39.794211944, -105.153350000"
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

ACTUAL=$(googlemaps -D "HPP   39.794212196, -105.153349930")
EXPECTED="39.794212196, -105.153349930"
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

ACTUAL=$(googlemaps -D "NGS  39 47 39.16390(N) 105 09 12.05974(W)")
EXPECTED="39.794212194, -105.153349928"
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

ACTUAL=$(googlemaps -D "POS 39°47'39.163\"N, 105°09'12.060\"W")
EXPECTED="39.794211944, -105.153350000"
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

ACTUAL=$(googlemaps -P "HPP    39.794212196, -105.153349930")
EXPECTED="39°47'39.163905\"N, 105°09'12.059748\"W"
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

ACTUAL=$(googlemaps -P "NGS  39 47 39.16390(N) 105 09 12.05974(W)")
EXPECTED="39°47'39.163899\"N, 105°09'12.059740\"W"
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

ACTUAL=$(googlemaps -P "POS 39°47'39.163\"N, 105°09'12.060\"W")
EXPECTED="39°47'39.162999\"N, 105°09'12.060000\"W"
if [[ "${ACTUAL}" != "${EXPECTED}" ]]; then
	echo FAIL! ${ACTUAL} != ${EXPECTED} 1>&2
	XC=1
fi

exit ${XC}
