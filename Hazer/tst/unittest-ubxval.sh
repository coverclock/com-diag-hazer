#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

# The comparison of float values is extremely problematic.
# The best I can say is that it passes on the specific
# target on which I built and ran the unit test.

PROGRAM=$(basename ${0})

COMMAND="ubxval +1 65"
ACTUAL="$(${COMMAND})"
EXPECTED='\x41'
echo COMMAND=\"${COMMAND}\" EXPECTED=\"${EXPECTED}\" ACTUAL=\"${ACTUAL}\" 1>&2
test "${ACTUAL}" = "${EXPECTED}" || exit 1

COMMAND="ubxval +1 0x12"
ACTUAL="$(${COMMAND})"
EXPECTED='\x12'
echo COMMAND=\"${COMMAND}\" EXPECTED=\"${EXPECTED}\" ACTUAL=\"${ACTUAL}\" 1>&2
test "${ACTUAL}" = "${EXPECTED}" || exit 1
 
COMMAND="ubxval +2 65"
ACTUAL="$(${COMMAND})"
EXPECTED='\x41\x00'
echo COMMAND=\"${COMMAND}\" EXPECTED=\"${EXPECTED}\" ACTUAL=\"${ACTUAL}\" 1>&2
test "${ACTUAL}" = "${EXPECTED}" || exit 1
 
COMMAND="ubxval +2 0x1234"
ACTUAL="$(${COMMAND})"
EXPECTED='\x34\x12'
echo COMMAND=\"${COMMAND}\" EXPECTED=\"${EXPECTED}\" ACTUAL=\"${ACTUAL}\" 1>&2
test "${ACTUAL}" = "${EXPECTED}" || exit 1
  
COMMAND="ubxval +4 65"
ACTUAL="$(${COMMAND})"
EXPECTED='\x41\x00\x00\x00'
echo COMMAND=\"${COMMAND}\" EXPECTED=\"${EXPECTED}\" ACTUAL=\"${ACTUAL}\" 1>&2
test "${ACTUAL}" = "${EXPECTED}" || exit 1
  
COMMAND="ubxval +4 0x12345678"
ACTUAL="$(${COMMAND})"
EXPECTED='\x78\x56\x34\x12'
echo COMMAND=\"${COMMAND}\" EXPECTED=\"${EXPECTED}\" ACTUAL=\"${ACTUAL}\" 1>&2
test "${ACTUAL}" = "${EXPECTED}" || exit 1
 
COMMAND="ubxval +8 65"
ACTUAL="$(${COMMAND})"
EXPECTED='\x41\x00\x00\x00\x00\x00\x00\x00'
echo COMMAND=\"${COMMAND}\" EXPECTED=\"${EXPECTED}\" ACTUAL=\"${ACTUAL}\" 1>&2
test "${ACTUAL}" = "${EXPECTED}" || exit 1
 
COMMAND="ubxval +8 0x123456789abcdef0"
ACTUAL="$(${COMMAND})"
EXPECTED='\xf0\xde\xbc\x9a\x78\x56\x34\x12'
echo COMMAND=\"${COMMAND}\" EXPECTED=\"${EXPECTED}\" ACTUAL=\"${ACTUAL}\" 1>&2
test "${ACTUAL}" = "${EXPECTED}" || exit 1
  
COMMAND="ubxval +S 0.15625"
ACTUAL="$(${COMMAND})"
EXPECTED='\x00\x00\x20\x3e'
echo COMMAND=\"${COMMAND}\" EXPECTED=\"${EXPECTED}\" ACTUAL=\"${ACTUAL}\" 1>&2
test "${ACTUAL}" = "${EXPECTED}" || exit 1
  
COMMAND="ubxval +D 0.333333333333333314829616256247390992939472198486328125"
ACTUAL="$(${COMMAND})"
EXPECTED='\x55\x55\x55\x55\x55\x55\xd5\x3f'
echo COMMAND=\"${COMMAND}\" EXPECTED=\"${EXPECTED}\" ACTUAL=\"${ACTUAL}\" 1>&2
test "${ACTUAL}" = "${EXPECTED}" || exit 1

echo "PASSED" 1>&2 
exit 0
