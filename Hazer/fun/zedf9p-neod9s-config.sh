#!/bin/bash
# Copyright 2023 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# This script configures the UBX-NEO-D9S and the UBX-ZED-F9P.
# By default it uses the configuration script for the Nicker
# project containing the appropriate commands for the U.S.
# region.

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}
CFGFIL=${COM_DIAG_HAZER_CFGFIL:-"${HOME}/com_diag_nicker_us.sh"}

PGMNAM=$(basename ${0})
LOCDEV=${1:-"/dev/ttyACM0"}
LOCBPS=${2:-38400}
CORDEV=${3:-"/dev/ttyACM1"}
CORBPS=${4:-9600}
ERRFIL=${5:-"${SAVDIR}/${PGMNAM}.err"}
OUTFIL=${6:-"${SAVDIR}/${PGMNAM}.out"}

. $(readlink -e $(dirname ${0})/../bin)/setup

mkdir -p $(dirname ${ERRFIL})
mkdir -p $(dirname ${OUTFIL})

cp /dev/null ${ERRFIL}
exec 2>>${ERRFIL}

#####
# SOURCE THE CONFIGURATION SCRIPT.
#####

. ${CFGFIL}

#####
# CONFIGURE THE UBX-NEO-D9S INMARSAT RECEIVER.
#####

OPTIONS=""
for OPTION in ${UBX_NEO_D9S}; do
    OPTIONS="${OPTIONS} ${OPTION}"
done

# UBX-VAL-SET [9] RAM: CFG-USART2OUTPROT-UBX=1
# UBX-VAL-SET [9] RAM: CFG-USART2-BAUDRATE=38400
# : (private options) :

eval gpstool \
    -R \
    -D ${CORDEV} -b ${CORBPS} -8 -n -1 \
    -x \
    -A '\\xb5\\x62\\x06\\x8a\\x09\\x00\\x00\\x01\\x00\\x00\\x01\\x00\\x76\\x10\\x01' \
    -A '\\xb5\\x62\\x06\\x8a\\x0c\\x00\\x00\\x01\\x00\\x00\\x01\\x00\\x53\\x40\\x00\\x96\\x00\\x00' \
    ${OPTIONS} \
    -A \"\" \
    < /dev/null

#####
# CONFIGURE THE UBX-ZED-F9P GNSS RECEIVER.
#####

OPTIONS=""
for OPTION in ${UBX_ZED_F9P}; do
    OPTIONS="${OPTIONS} ${OPTION}"
done

# UBX-MON-VER [0]
# UBX-CFG-MSG [3] UBX-NAV-HPPOSLLH 1
# UBX-VAL-SET [9] RAM: CFG-SPARTN-USE_SOURCE=1 (LBAND)
# UBX-VAL-SET [9] RAM: CFG-USART2INPROT-UBX=1
# : (private options) :

eval gpstool \
    -H ${OUTFIL} -F 1 -t 10 \
    -D ${LOCDEV} -b ${LOCBPS} -8 -n -1 \
    -x \
    -U '\\xb5\\x62\\x0a\\x04\\x00\\x00' \
    -A '\\xb5\\x62\\x06\\x01\\x03\\x00\\x01\\x14\\x01' \
    -A '\\xb5\\x62\\x06\\x8a\\x09\\x00\\x00\\x01\\x00\\x00\\x01\\x00\\xa7\\x20\\x01' \
    -A '\\xb5\\x62\\x06\\x8a\\x09\\x00\\x00\\x01\\x00\\x00\\x01\\x00\\x75\\x10\\x01' \
    ${OPTIONS} \
    < /dev/null
