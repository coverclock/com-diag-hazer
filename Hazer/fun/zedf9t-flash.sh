#!/bin/bash
# Copyright 2022 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Configure the U-blox ZED-UBX-F9T FLASH as a high precision
# time and frequency reference in "survey-in" mode, with TP1
# emitting a 1PPS, and TP2 emitting a 10MHz square wave. This
# is part of the "Metronome" sub-project.

SAVDIR=${COM_DIAG_HAZER_SAVDIR:-$(readlink -e $(dirname ${0})/..)/tmp}

PROGRAM=$(basename ${0})
DEVICE=${1:-"/dev/ttyACM0"}
RATE=${2:-38400}

. $(readlink -e $(dirname ${0})/../bin)/setup

# UBX-CFG-VALSET [4] V1 Flash Start 0x00

# UBX-CFG-VALSET [9] V1 Flash Continue 0x00 CFG-TMODE-MODE SURVEY_IN
# UBX-CFG-VALSET [12] V1 Flash Continue 0x00 CFG-TMODE-SVIN_MIN_DUR 300 (@sec = 5min)
# UBX-CFG-VALSET [12] V1 Flash Continue 0x00 CFG-TMODE-SVIN_ACC_LIMIT 250 (@0.1mm = 2.5cm = ~1in)

# UBX-CFG-VALSET [9] V1 Flash Continue 0x00 CFG-TP-PULSE_DEF 1 (frequency)
# UBX-CFG-VALSET [9] V1 Flash Continue 0x00 CFG-TP-PULSE_LENGTH_DEF 0 (ratio)

# UBX-CFG-VALSET [12] V1 Flash Continue 0x00 CFG-TP-FREQ_TP1 1 (Hz)
# UBX-CFG-VALSET [12] V1 Flash Continue 0x00 CFG-TP-FREQ_LOCK_TP1 1 (Hz)
# UBX-CFG-VALSET [12] V1 Flash Continue 0x00 CFG-TP-DUTY_TP1 10.0 (%)
# UBX-CFG-VALSET [12] V1 Flash Continue 0x00 CFG-TP-DUTY_LOCK_TP1 10.0 (%)
# UBX-CFG-VALSET [9] V1 Flash Continue 0x00 CFG-TP-TIMEGRID_TP1 1 (GPS)
# UBX-CFG-VALSET [9] V1 Flash Continue 0x00 CFG-TP-ALIGN_TO_TOW_TP1 1
# UBX-CFG-VALSET [9] V1 Flash Continue 0x00 CFG-TP-USE_LOCKED_TP1 1
# UBX-CFG-VALSET [9] V1 Flash Continue 0x00 CFG-TP-POL_TP1 1
# UBX-CFG-VALSET [9] V1 Flash Continue 0x00 CFG-TP-TP1_ENA 1 ("Must be set for frequency-time products.")

# UBX-CFG-VALSET [12] V1 Flash Continue 0x00 CFG-TP-FREQ_TP2 10 000 000 (Hz)
# UBX-CFG-VALSET [12] V1 Flash Continue 0x00 CFG-TP-FREQ_LOCK_TP2 10 000 000 (Hz)
# UBX-CFG-VALSET [12] V1 Flash Continue 0x00 CFG-TP-DUTY_TP2 50.0 (%)
# UBX-CFG-VALSET [12] V1 Flash Continue 0x00 CFG-TP-DUTY_LOCK_TP2 50.0 (%)
# UBX-CFG-VALSET [9] V1 Flash Continue 0x00 CFG-TP-TIMEGRID_TP2 1 (GPS)
# UBX-CFG-VALSET [9] V1 Flash Continue 0x00 CFG-TP-ALIGN_TO_TOW_TP2 1
# UBX-CFG-VALSET [9] V1 Flash Continue 0x00 CFG-TP-USE_LOCKED_TP2 1
# UBX-CFG-VALSET [9] V1 Flash Continue 0x00 CFG-TP-POL_TP2 1
# UBX-CFG-VALSET [9] V1 Flash Continue 0x00 CFG-TP-TP2_ENA 1

# UBX-CFG-VALSET [4] V1 Flash Apply 0x00

# UBX-CFG-RST [4] ColdStart ShutdownHardwareReset

# exit

exec gpstool \
    -D ${DEVICE} -b ${RATE} -8 -n -1 \
    -x \
    -A '\xb5\x62\x06\x8a'"$(ubxval -2  4)$(ubxval -1 1)$(ubxval -1 0x04)$(ubxval -1 1)"'\x00' \
    \
    -A '\xb5\x62\x06\x8a'"$(ubxval -2  9)$(ubxval -1 1)$(ubxval -1 0x04)$(ubxval -1 2)"'\x00'"$(ubxval -4 0x20030001)$(ubxval -1 1)" \
    -A '\xb5\x62\x06\x8a'"$(ubxval -2 12)$(ubxval -1 1)$(ubxval -1 0x04)$(ubxval -1 2)"'\x00'"$(ubxval -4 0x40030010)$(ubxval -4 300)" \
    -A '\xb5\x62\x06\x8a'"$(ubxval -2 12)$(ubxval -1 1)$(ubxval -1 0x04)$(ubxval -1 2)"'\x00'"$(ubxval -4 0x40030011)$(ubxval -4 250)" \
    \
    -A '\xb5\x62\x06\x8a'"$(ubxval -2  4)$(ubxval -1 1)$(ubxval -1 0x04)$(ubxval -1 3)"'\x00' \
    \
    -A '\xb5\x62\x06\x8a'"$(ubxval -2  4)$(ubxval -1 1)$(ubxval -1 0x04)$(ubxval -1 1)"'\x00' \
    \
    -A '\xb5\x62\x06\x8a'"$(ubxval -2  9)$(ubxval -1 1)$(ubxval -1 0x04)$(ubxval -1 2)"'\x00'"$(ubxval -4 0x20050023)$(ubxval -1 1)" \
    -A '\xb5\x62\x06\x8a'"$(ubxval -2  9)$(ubxval -1 1)$(ubxval -1 0x04)$(ubxval -1 2)"'\x00'"$(ubxval -4 0x20050030)$(ubxval -1 0)" \
    \
    -A '\xb5\x62\x06\x8a'"$(ubxval -2 12)$(ubxval -1 1)$(ubxval -1 0x04)$(ubxval -1 2)"'\x00'"$(ubxval -4 0x40050024)$(ubxval -4 1)" \
    -A '\xb5\x62\x06\x8a'"$(ubxval -2 12)$(ubxval -1 1)$(ubxval -1 0x04)$(ubxval -1 2)"'\x00'"$(ubxval -4 0x40050025)$(ubxval -4 1)" \
    -A '\xb5\x62\x06\x8a'"$(ubxval -2 16)$(ubxval -1 1)$(ubxval -1 0x04)$(ubxval -1 2)"'\x00'"$(ubxval -4 0x5005002a)$(ubxval -D 10.0)" \
    -A '\xb5\x62\x06\x8a'"$(ubxval -2 16)$(ubxval -1 1)$(ubxval -1 0x04)$(ubxval -1 2)"'\x00'"$(ubxval -4 0x5005002b)$(ubxval -D 10.0)" \
    -A '\xb5\x62\x06\x8a'"$(ubxval -2  9)$(ubxval -1 1)$(ubxval -1 0x04)$(ubxval -1 2)"'\x00'"$(ubxval -4 0x2005000c)$(ubxval -1 1)" \
    -A '\xb5\x62\x06\x8a'"$(ubxval -2  9)$(ubxval -1 1)$(ubxval -1 0x04)$(ubxval -1 2)"'\x00'"$(ubxval -4 0x1005000a)$(ubxval -1 1)" \
    -A '\xb5\x62\x06\x8a'"$(ubxval -2  9)$(ubxval -1 1)$(ubxval -1 0x04)$(ubxval -1 2)"'\x00'"$(ubxval -4 0x10050009)$(ubxval -1 1)" \
    -A '\xb5\x62\x06\x8a'"$(ubxval -2  9)$(ubxval -1 1)$(ubxval -1 0x04)$(ubxval -1 2)"'\x00'"$(ubxval -4 0x1005000b)$(ubxval -1 1)" \
    -A '\xb5\x62\x06\x8a'"$(ubxval -2  9)$(ubxval -1 1)$(ubxval -1 0x04)$(ubxval -1 2)"'\x00'"$(ubxval -4 0x10050007)$(ubxval -1 1)" \
    \
    -A '\xb5\x62\x06\x8a'"$(ubxval -2 12)$(ubxval -1 1)$(ubxval -1 0x04)$(ubxval -1 2)"'\x00'"$(ubxval -4 0x40050026)$(ubxval -4 10000000)" \
    -A '\xb5\x62\x06\x8a'"$(ubxval -2 12)$(ubxval -1 1)$(ubxval -1 0x04)$(ubxval -1 2)"'\x00'"$(ubxval -4 0x40050027)$(ubxval -4 10000000)" \
    -A '\xb5\x62\x06\x8a'"$(ubxval -2 16)$(ubxval -1 1)$(ubxval -1 0x04)$(ubxval -1 2)"'\x00'"$(ubxval -4 0x5005002c)$(ubxval -D 50.0)" \
    -A '\xb5\x62\x06\x8a'"$(ubxval -2 16)$(ubxval -1 1)$(ubxval -1 0x04)$(ubxval -1 2)"'\x00'"$(ubxval -4 0x5005002d)$(ubxval -D 50.0)" \
    -A '\xb5\x62\x06\x8a'"$(ubxval -2  9)$(ubxval -1 1)$(ubxval -1 0x04)$(ubxval -1 2)"'\x00'"$(ubxval -4 0x20050017)$(ubxval -1 1)" \
    -A '\xb5\x62\x06\x8a'"$(ubxval -2  9)$(ubxval -1 1)$(ubxval -1 0x04)$(ubxval -1 2)"'\x00'"$(ubxval -4 0x10050015)$(ubxval -1 1)" \
    -A '\xb5\x62\x06\x8a'"$(ubxval -2  9)$(ubxval -1 1)$(ubxval -1 0x04)$(ubxval -1 2)"'\x00'"$(ubxval -4 0x10050014)$(ubxval -1 1)" \
    -A '\xb5\x62\x06\x8a'"$(ubxval -2  9)$(ubxval -1 1)$(ubxval -1 0x04)$(ubxval -1 2)"'\x00'"$(ubxval -4 0x10050016)$(ubxval -1 1)" \
    -A '\xb5\x62\x06\x8a'"$(ubxval -2  9)$(ubxval -1 1)$(ubxval -1 0x04)$(ubxval -1 2)"'\x00'"$(ubxval -4 0x10050012)$(ubxval -1 1)" \
    \
    -A '\xb5\x62\x06\x8a'"$(ubxval -2  4)$(ubxval -1 1)$(ubxval -1 0x04)$(ubxval -1 3)"'\x00' \
    \
    -U '\xb5\x62\x06\x04'"$(ubxval -2 4)$(ubxval -2 0xffff)$(ubxval -1 0x04)"'0x00' \
    \
    -U ''
