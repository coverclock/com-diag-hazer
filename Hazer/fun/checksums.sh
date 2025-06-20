#!/bin/bash
# Copyright 2018-2025 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

export LC_ALL=en_US.UTF-8

. $(readlink -e $(dirname ${0})/../bin)/setup

XC=0

ARGUMENT='$GLGSV,3,3,11,87,41,070,33,88,35,133,29,95,37,062,*58\r\n'
echo "ARGUMENT" ${ARGUMENT}
EXPECTED='$GLGSV,3,3,11,87,41,070,33,88,35,133,29,95,37,062,*58\r\n'
echo "EXPECTED" ${EXPECTED}
ACTUAL=$(checksum ${ARGUMENT})
echo "ACTUAL  " ${ACTUAL}
if [[ "${EXPECTED}" != "${ACTUAL}" ]]; then echo "ERROR   "; XC=1; fi
echo

ARGUMENT='$GLGSV,3,3,11,87,41,070,33,88,35,133,29,95,37,062,*'
echo "ARGUMENT" ${ARGUMENT}
EXPECTED='$GLGSV,3,3,11,87,41,070,33,88,35,133,29,95,37,062,*58\r\n'
echo "EXPECTED" ${EXPECTED}
ACTUAL=$(checksum ${ARGUMENT})
echo "ACTUAL  " ${ACTUAL}
if [[ "${EXPECTED}" != "${ACTUAL}" ]]; then echo "ERROR   "; XC=1; fi
echo

ARGUMENT='$GLGSV,3,3,11,87,41,070,33,88,35,133,29,95,37,062,'
echo "ARGUMENT" ${ARGUMENT}
EXPECTED='$GLGSV,3,3,11,87,41,070,33,88,35,133,29,95,37,062,*58\r\n'
echo "EXPECTED" ${EXPECTED}
ACTUAL=$(checksum ${ARGUMENT})
echo "ACTUAL  " ${ACTUAL}
if [[ "${EXPECTED}" != "${ACTUAL}" ]]; then echo "ERROR   "; XC=1; fi
echo

ARGUMENT='\xb5\x62\xa5\x5a\x04\x00\x01\x02\x03\x04\x0d\xca'
echo "ARGUMENT" ${ARGUMENT}
EXPECTED='\xb5b\xa5Z\x04\x00\x01\x02\x03\x04\x0d\xca'
echo "EXPECTED" ${EXPECTED}
ACTUAL=$(checksum ${ARGUMENT})
echo "ACTUAL  " ${ACTUAL}
if [[ "${EXPECTED}" != "${ACTUAL}" ]]; then echo "ERROR   "; XC=1; fi
echo

ARGUMENT='\xb5\x62\xa5\x5a\x04\x00\x01\x02\x03\x04'
echo "ARGUMENT" ${ARGUMENT}
EXPECTED='\xb5b\xa5Z\x04\x00\x01\x02\x03\x04\x0d\xca'
echo "EXPECTED" ${EXPECTED}
ACTUAL=$(checksum ${ARGUMENT})
echo "ACTUAL  " ${ACTUAL}
if [[ "${EXPECTED}" != "${ACTUAL}" ]]; then echo "ERROR   "; XC=1; fi
echo

ARGUMENT='\xb5\x62\xa5\x5a\x04\x00\x01\x02\x03\x04\xff\xff'
echo "ARGUMENT" ${ARGUMENT}
EXPECTED='\xb5b\xa5Z\x04\x00\x01\x02\x03\x04\x0d\xca'
echo "EXPECTED" ${EXPECTED}
ACTUAL=$(checksum ${ARGUMENT})
echo "ACTUAL  " ${ACTUAL}
if [[ "${EXPECTED}" != "${ACTUAL}" ]]; then echo "ERROR   "; XC=1; fi
echo

ARGUMENT='\xd3\x00\x08\x4c\xe0\x00\x8a\x00\x00\x00\x00\xa8\xf7\x2a';
echo "ARGUMENT" ${ARGUMENT}
EXPECTED='\xd3\x00\x08L\xe0\x00\x8a\x00\x00\x00\x00\xa8\xf7*';
echo "EXPECTED" ${EXPECTED}
ACTUAL=$(checksum ${ARGUMENT})
echo "ACTUAL  " ${ACTUAL}
if [[ "${EXPECTED}" != "${ACTUAL}" ]]; then echo "ERROR   "; XC=1; fi
echo

ARGUMENT='\xd3\x00\x08\x4c\xe0\x00\x8a\x00\x00\x00\x00\xff\xff\xff';
echo "ARGUMENT" ${ARGUMENT}
EXPECTED='\xd3\x00\x08L\xe0\x00\x8a\x00\x00\x00\x00\xa8\xf7*';
echo "EXPECTED" ${EXPECTED}
ACTUAL=$(checksum ${ARGUMENT})
echo "ACTUAL  " ${ACTUAL}
if [[ "${EXPECTED}" != "${ACTUAL}" ]]; then echo "ERROR   "; XC=1; fi
echo

ARGUMENT='\xd3\x00\x08\x4c\xe0\x00\x8a\x00\x00\x00\x00';
echo "ARGUMENT" ${ARGUMENT}
EXPECTED='\xd3\x00\x08L\xe0\x00\x8a\x00\x00\x00\x00\xa8\xf7*';
echo "EXPECTED" ${EXPECTED}
ACTUAL=$(checksum ${ARGUMENT})
echo "ACTUAL  " ${ACTUAL}
if [[ "${EXPECTED}" != "${ACTUAL}" ]]; then echo "ERROR   "; XC=1; fi
echo

ARGUMENT='P855w0rd!\n'
echo "ARGUMENT" ${ARGUMENT}
EXPECTED='P855w0rd!\n'
echo "EXPECTED" ${EXPECTED}
ACTUAL=$(checksum ${ARGUMENT})
echo "ACTUAL  " ${ACTUAL}
if [[ "${EXPECTED}" != "${ACTUAL}" ]]; then echo "ERROR   "; XC=1; fi
echo

if [[ ${XC} == 0 ]]; then echo "PASS    "; else echo "FAIL    "; fi

exit ${XC}
