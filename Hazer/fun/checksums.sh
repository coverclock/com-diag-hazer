#!/bin/bash
# Copyright 2018 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

XC=0

EXPECTED='$GLGSV,3,3,11,87,41,070,33,88,35,133,29,95,37,062,*58\r\n'
ACTUAL=$(checksum '$GLGSV,3,3,11,87,41,070,33,88,35,133,29,95,37,062,*58\r\n')
echo ${EXPECTED}
echo ${ACTUAL}
if [[ "${EXPECTED}" != "${ACTUAL}" ]]; then XC=1; fi

EXPECTED='$GLGSV,3,3,11,87,41,070,33,88,35,133,29,95,37,062,*58\r\n'
ACTUAL=$(checksum '$GLGSV,3,3,11,87,41,070,33,88,35,133,29,95,37,062,*')
echo ${EXPECTED}
echo ${ACTUAL}
if [[ "${EXPECTED}" != "${ACTUAL}" ]]; then XC=1; fi

EXPECTED='$GLGSV,3,3,11,87,41,070,33,88,35,133,29,95,37,062,*58\r\n'
ACTUAL=$(checksum '$GLGSV,3,3,11,87,41,070,33,88,35,133,29,95,37,062,')
echo ${EXPECTED}
echo ${ACTUAL}
if [[ "${EXPECTED}" != "${ACTUAL}" ]]; then XC=1; fi

EXPECTED='\xb5b\xa5Z\x04\x00\x01\x02\x03\x04\x0d\xca'
ACTUAL=$(checksum '\xb5\x62\xa5\x5a\x04\x00\x01\x02\x03\x04\x0d\xca')
echo ${EXPECTED}
echo ${ACTUAL}
if [[ "${EXPECTED}" != "${ACTUAL}" ]]; then XC=1; fi

EXPECTED='\xb5b\xa5Z\x04\x00\x01\x02\x03\x04\x0d\xca'
ACTUAL=$(checksum '\xb5\x62\xa5\x5a\x04\x00\x01\x02\x03\x04')
echo ${EXPECTED}
echo ${ACTUAL}
if [[ "${EXPECTED}" != "${ACTUAL}" ]]; then XC=1; fi

exit ${XC}
