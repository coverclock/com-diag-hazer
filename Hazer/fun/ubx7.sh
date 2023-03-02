#!/bin/bash
# Copyright 2018-2021 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

# NMEA-PUBX-POSITION
# NMEA-PUBX-SVSTATUS
# NMEA-PUBX-TIME
# UBX-CFG-PRT [0] (all)
# UBX-CFG-MSG [3] UBX-NAV-TIMEGPS @1Hz
# UBX-CFG-MSG [3] UBX-NAV-TIMEUTC @1Hz
# UBX-CFG-MSG [3] UBX-NAV-CLOCK @1Hz
# UBX-CFG-MSG [3] UBX-TIM-TP @1Hz
# UBX-CFG-MSG [3] UBX-TIM-TM2 @1Hz
# UBX-MON-VER [0]
# UBX-CFG-DAT [0]
# UBX-CFG-TPS [0]
# UBX-CFG-GNSS [0]

COMMANDS='
    -W "\$PUBX,00"
    -W "\$PUBX,03"
    -W "\$PUBX,04"
    -A "\\xb5\\x62\\x06\\x00\\x00\\x00"
    -A "\\xb5\\x62\\x06\\x01\\x03\\x00\\x01\\x20\\x01"
    -A "\\xb5\\x62\\x06\\x01\\x03\\x00\\x01\\x21\\x01"
    -A "\\xb5\\x62\\x06\\x01\\x03\\x00\\x01\\x22\\x01"
    -A "\\xb5\\x62\\x06\\x01\\x03\\x00\\x0d\\x01\\x01"
    -A "\\xb5\\x62\\x06\\x01\\x03\\x00\\x0d\\x03\\x01"
    -U "\\xb5\\x62\\x0a\\x04\\x00\\x00"
    -A "\\xb5\\x62\\x06\\x06\\x00\\x00"
    -A "\\xb5\\x62\\x06\\x31\\x00\\x00"
    -A "\\xb5\\x62\\x06\\x3e\\x00\\x00"
'
