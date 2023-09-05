#!/bin/bash
# Copyright 2023 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Configures the UBX-NEO-D9S to receive SPARTN messages on L-band
# from Inmarsat. Configures the UBX-ZED-F9P to decrypt the SPARTN
# corrections from the UBX-NEO-D9S. Queries the UBX-NEO-D9S mostly
# as a check. Then finally runs a headless gpstool against the
# UBX-ZED-F9P. Requires a configuration file with confidential
# Inmarsat parameters and SPARTN encryption keys. The file is
# assumed to be ${HOME}/com_diag_nicker_us.sh unless the
# environmental variable COM_DIAG_HAZER_CFGFIL says otherwise.
#
# REFERENCES
#
# "NEO-D9S and ZED-F9 configuration SPARTN L-band correction data reception
# Application Note", UBX-22008160-R02, U-blox AG, 2022-07-22

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}
CFGFIL=${COM_DIAG_HAZER_CFGFIL:-"${HOME}/com_diag_nicker_us.sh"}

PGMNAM=$(basename ${0})
FILNAM=${PGMNAM}
LOCDEV=${1:-"/dev/ttyACM0"}
LOCBPS=${2:-38400}
CORDEV=${3:-"/dev/ttyACM1"}
CORBPS=${4:-9600}
ERRFIL=${5:-"${SAVDIR}/${FILNAM}.err"}
OUTFIL=${6:-"${SAVDIR}/${FILNAM}.out"}
CSVFIL=${7:-"${SAVDIR}/${FILNAM}.csv"}
PIDFIL=${8:-"${SAVDIR}/${FILNAM}.pid"}

. $(readlink -e $(dirname ${0})/../bin)/setup

mkdir -p $(dirname ${ERRFIL})
mkdir -p $(dirname ${OUTFIL})
mkdir -p $(dirname ${CSVFIL})
mkdir -p $(dirname ${PIDFIL})

cp /dev/null ${ERRFIL}
cp /dev/null ${OUTFIL}
cp /dev/null ${CSVFIL}
cp /dev/null ${PIDFIL}
exec 2>>${ERRFIL}

tail -f ${ERRFIL} & ERRPID=$!
trap "kill ${ERRPID}" SIGINT SIGQUIT SIGTERM EXIT

MESSAGE="${PGMNAM}: Starting ${LOCDEV} ${CORDEV}"
log -I -N ${PGMNAM} -n "${MESSAGE}"

#####
# SOURCE THE CONFIGURATION SCRIPT.
#####

MESSAGE="${PGMNAM}: Sourcing ${CFGFIL}"
log -I -N ${PGMNAM} -n "${MESSAGE}"

if [[ ! -r ${CFGFIL} ]]; then
    echo "${PGMNAM}: ${CFGFIL} failed!" 1>&2
    exit 1
fi

. ${CFGFIL}

#####
# CONFIGURE THE UBX-NEO-D9S INMARSAT RECEIVER.
#####

MESSAGE="${PGMNAM}: Configuring UBX-NEO-D9S ${CORDEV} ${CORBPS}"
log -I -N ${PGMNAM} -n "${MESSAGE}"

OPTIONS=""
for OPTION in ${UBX_NEO_D9S}; do
    OPTIONS="${OPTIONS} ${OPTION}"
done

# UBX-MON-VER [ 0]
# UBX-VAL-SET [ 9] RAM: CFG-USART2-BAUDRATE=38400 CFG-USART2OUTPROT-UBX=1 CFG-MSGOUT-UBX_RXM_PMP_UART2=1
# : (imported options) :

eval gpstool \
    -R \
    -D ${CORDEV} -b ${CORBPS} -8 -n -1 \
    -O ${PIDFIL} \
    -w 5 -x \
    -U '\\xb5\\x62\\x0a\\x04\\x00\\x00' \
    -A '\\xb5\\x62\\x06\\x8a\\x16\\x00\\x00\\x01\\x00\\x00\\x01\\x00\\x53\\x40\\x00\\x96\\x00\\x00\\x01\\x00\\x76\\x10\\x01\\x1f\\x03\\x91\\x20\\x01' \
    ${OPTIONS} \
    -U \"\" \
    < /dev/null >> ${OUTFIL}

if (( $? != 0 )); then
    echo "${PGMNAM}: ${CORDEV} failed!" 1>&2
    exit 1
fi

#####
# CONFIGURE THE UBX-ZED-F9P GNSS RECEIVER.
#####

MESSAGE="${PGMNAM}: Configuring UBX-ZED-F9P ${LOCDEV} ${LOCBPS}"
log -I -N ${PGMNAM} -n "${MESSAGE}"

OPTIONS=""
for OPTION in ${UBX_ZED_F9P}; do
    OPTIONS="${OPTIONS} ${OPTION}"
done

# UBX-MON-VER [ 0]
# UBX-VAL-SET [ 9] v0 RAM: CFG-SPARTN-USE_SOURCE=1 (L-BAND)
# UBX-VAL-SET [ 9] v0 RAM: CFG-USART2INPROT-UBX=1
# : (imported options) :
# UBX-RXM-SPARTNKEY [ 0]

eval gpstool \
    -R \
    -D ${LOCDEV} -b ${LOCBPS} -8 -n -1 \
    -O ${PIDFIL} \
    -w 5 -x \
    -U '\\xb5\\x62\\x0a\\x04\\x00\\x00' \
    -A '\\xb5\\x62\\x06\\x8a\\x09\\x00\\x00\\x01\\x00\\x00\\x01\\x00\\xa7\\x20\\x01' \
    -A '\\xb5\\x62\\x06\\x8a\\x09\\x00\\x00\\x01\\x00\\x00\\x01\\x00\\x75\\x10\\x01' \
    ${OPTIONS} \
    -U '\\xb5\\x62\\x02\\x36\\x00\\x00' \
    -U \"\" \
    < /dev/null >> ${OUTFIL}

if (( $? != 0 )); then
    echo "${PGMNAM}: ${LOCDEV} failed!" 1>&2
    exit 1
fi

#####
# PROCESS THE UBX-ZED-F9P GNSS RECEIVER.
#####

MESSAGE="${PGMNAM}: Processing UBX-ZED-F9P ${LOCDEV} ${LOCBPS}"
log -I -N ${PGMNAM} -n "${MESSAGE}"

kill ${ERRPID}

# UBX-CFG-MSG [ 3] UBX-NAV-HPPOSLLH=1

gpstool \
    -D ${LOCDEV} -b ${LOCBPS} -8 -n -1 \
    -O ${PIDFIL} \
    -H ${OUTFIL} -F 1 -t 10 \
    -T ${CSVFIL} -f 1 \
    -w 5 -x \
    -A '\xb5\x62\x06\x01\x03\x00\x01\x14\x01' \
    < /dev/null

if (( $? != 0 )); then
    echo "${PGMNAM}: ${LOCDEV} failed!" 1>&2
    exit 1
fi

MESSAGE="${PGMNAM}: Ending ${LOCDEV} ${CORDEV}"
log -I -N ${PGMNAM} -n "${MESSAGE}"

exit 0
