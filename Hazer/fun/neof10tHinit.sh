#!/bin/bash
# Copyright 2023 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# I'm using an Ardusimple SimpleGNSS board with a
# u-blox NEO-F10T module. This is the first GNSS
# device I've used that implements the NMEA 0183 4.11
# specification. It is also the first generation 10
# u-blox device I've used. N.B. the labeling of the PVT
# (1PPS) and POWER LEDs are reversed on this version
# of the SimpleGNSS board. This script uses my configuration.

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyUSB0"}
RATE=${2:-38400}

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}
mkdir -p ${SAVDIR}

ERRFIL="${SAVDIR}/${PROGRAM}.err"
mkdir -p $(dirname ${ERRFIL})
exec 2>>${ERRFIL}

OUTFIL="${SAVDIR}/${PROGRAM}.out"
mkdir -p $(dirname ${OUTFIL})

PIDFIL="${SAVDIR}/${PROGRAM}.pid"
mkdir -p $(dirname ${PIDFIL})

# LSTFIL="${SAVDIR}/${PROGRAM}.lst"
# mkdir -p $(dirname ${LSTFIL})
# -L ${LSTFIL}

# DATFIL="${SAVDIR}/${PROGRAM}.dat"
# mkdir -p $(dirname ${DATFIL})
# -C ${DATFIL}

. $(readlink -e $(dirname ${0})/../bin)/setup

# UBX-MON-VER [0]
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-SIGNAL-GPS_ENA 0x1031001f L - - GPS enable
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-SIGNAL-GPS_L1CA_ENA 0x10310001 L - - GPS L1C/A
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-SIGNAL-GPS_L5_ENA 0x10310004 L - - GPS L5
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-SIGNAL-SBAS_ENA 0x10310020 L - - SBAS enable
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-SIGNAL-SBAS_L1CA_ENA 0x10310005 L - - SBAS L1C/A
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-SIGNAL-GAL_ENA 0x10310021 L - - Galileo enable
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-SIGNAL-GAL_E1_ENA 0x10310007 L - - Galileo E1
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-SIGNAL-GAL_E5A_ENA 0x10310009 L - - Galileo E5a
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-SIGNAL-BDS_ENA 0x10310022 L - - BeiDou Enable
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-SIGNAL-BDS_B1C_ENA 0x1031000f L - - BeiDou B1C
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-SIGNAL-BDS_B2A_ENA 0x10310028 L - - BeiDou B2a
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-SIGNAL-QZSS_ENA 0x10310024 L - - QZSS enable
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-SIGNAL-QZSS_L1CA_ENA 0x10310012 L - - QZSS L1C/A
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-SIGNAL-QZSS_L5_ENA 0x10310017 L - - QZSS L5
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-SIGNAL-GLO_ENA 0x10310025 L - - GLONASS enable
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-SIGNAL-NAVIC_ENA 0x10310026 L - - NavIC enable
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-SIGNAL-NAVIC_L5_ENA 0x1031001d L - - NavIC L5

exec coreable gpstool \
	-D ${DEVICE} -b ${RATE} -8 -n -1 \
	-E -H ${OUTFIL} \
	-O ${PIDFIL} \
	-t 10 -F 1 \
	-w 2 -x \
	-U '\xb5\x62\x0a\x04\x00\x00' \
	-A '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x1f\x00\x31\x10\x01' \
	-A '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x01\x00\x31\x10\x01' \
	-A '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x04\x00\x31\x10\x01' \
	-A '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x20\x00\x31\x10\x01' \
	-A '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x05\x00\x31\x10\x01' \
	-A '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x21\x00\x31\x10\x01' \
	-A '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x07\x00\x31\x10\x01' \
	-A '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x09\x00\x31\x10\x01' \
	-A '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x22\x00\x31\x10\x01' \
	-A '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x0f\x00\x31\x10\x01' \
	-A '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x28\x00\x31\x10\x01' \
	-A '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x24\x00\x31\x10\x01' \
	-A '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x12\x00\x31\x10\x01' \
	-A '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x17\x00\x31\x10\x01' \
	-A '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x25\x00\x31\x10\x01' \
	-A '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x26\x00\x31\x10\x01' \
	-A '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x1d\x00\x31\x10\x01' \
	< /dev/null > /dev/null

# This command was NAKed by the NEO-F10T.
# UBX-CFG-VALSET [9] V0 RAM 0 0 CFG-SIGNAL-GLO_L1_ENA 0x10310018 L - - GLONASS L1
# -A '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x18\x00\x31\x10\x01' \

